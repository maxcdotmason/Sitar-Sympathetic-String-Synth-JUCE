#pragma once
#define SympathyStrings_h
#include <cmath>

/**
A single string which vibrates symapthetically with an incoming signal
*/
class SympathyStrings
{
public:


	/**
	Constructor
	*/
	SympathyStrings()
	{
		massPossPrevious2 = new float[159];
		massPossPrevious1 = new float[159];
		massPoss = new float[159];	
	}

	/**
	Destructor
	*/
	~SympathyStrings()
	{
		delete[] massPossPrevious2;
		delete[] massPossPrevious1;
		delete[] massPoss;
	}

	/**
	initialise string with variables. Calls reseter to use values
	@param float sample rate
	@param float string tension
	@param float string radius
	@param float string stiffness
	@param float string length
	@param float damping on string
	@param float density of string
	*/
	void init(float sampleRateI, float tensionI, float radiusI, float stiffnessI, float lengthI, float dampingI, float densityI)
	{
		//	set member variables to incoming values
		setTimeStep(sampleRateI);
		setTension(tensionI);
		setRadius(radiusI);
		setStiffness(stiffnessI);
		setLength(lengthI);
		setDamping(dampingI);
		setDensity(densityI);

		reseter();
	}

	/**
	resets and calculates scheme parameters using currently stored variables
	*/
	void reseter()
	{
		//	calulate string length to use based on standard lengths and any tuning effects
		float length = defaultLength - (0.5f * defaultLength * ((1.0f/ 12.0f) * globalTuning));

		//	calculate physical parameters of string
		float area = 3.141592653589793238 *pow(radius,2)  ;
		float waveSpeed = sqrt(tension / (density * area));
		float loss = 6 * log(10) / damping;
		float stiffnessConstant = sqrt(stiffness / (density * area));

		//	calculate minimum spacial fidelity to ensure stability
		float minSpacing = sqrt( 0.5f * ( ( pow(waveSpeed,2) * pow(timeStep,2)) + sqrt((pow(waveSpeed,4) * pow(timeStep,4)) + (16 * pow(timeStep,2) * pow(stiffnessConstant,2)))));
		segmentNumber = floor(length / minSpacing);
		float spacing = length / segmentNumber;

		//	calculate second spacial derivative "matrix"
		float dXX[2];						
		dXX[0] = -2/(pow(spacing,2));		
		dXX[1] = 1 / (pow(spacing, 2));
		
		//	calculate fourth spacial derivative "matrix"
		float dXXXX[4];
		dXXXX[1] = 6 / (pow(spacing, 4));
		dXXXX[2] = -4 / (pow(spacing, 4));
		dXXXX[3] = 1 / (pow(spacing, 4));
		dXXXX[0] = 5 / (pow(spacing, 4));

		//	calculate scheme paramater "matrix"
		schemeParameterB[0] = (1 / (1 + loss*timeStep)) *   (  2   +   pow(waveSpeed,2)*pow(timeStep,2)*dXX[0]   -   pow(timeStep,2)*pow(stiffnessConstant,2)*dXXXX[0]  );
		schemeParameterB[1] = (1 / (1 + loss * timeStep)) * (2 + pow(waveSpeed, 2) * pow(timeStep, 2) * dXX[0] - pow(timeStep, 2) * pow(stiffnessConstant, 2) * dXXXX[1]);
		schemeParameterB[2] = (1 / (1 + loss * timeStep)) * (pow(waveSpeed, 2)* pow(timeStep, 2)* dXX[1] - pow(timeStep, 2) * pow(stiffnessConstant, 2) * dXXXX[2]);
		schemeParameterB[3] = (1 / (1 + loss * timeStep)) * (pow(waveSpeed, 2) * pow(timeStep, 2) * 0.0f - pow(timeStep, 2) * pow(stiffnessConstant, 2) * dXXXX[3]);
		
		//	claculate damping parameter
		schemeParameterC = (1 - loss * timeStep) / (1 + loss * timeStep);

		//	set all to 0
		for (int i = 0; i < (segmentNumber); i++)
		{
			massPossPrevious2[i] = 0.0f;
			massPossPrevious1[i] = 0.0f;
			massPoss[i] = 0.0f;
		}
	}

	/**
	inputs audio into sympathetic string 
	Process 1 sample of audio and return 1 sample.
	@param float: Sample to be processed
	@return float: processed Sample
	*/
	float process(float input)
	{
		//	calculate position of point 1
		massPoss[0] = massPossPrevious1[(0)] * schemeParameterB[0] + massPossPrevious1[(1)] * schemeParameterB[2] + massPossPrevious1[(2)] * schemeParameterB[3] - schemeParameterC * massPossPrevious2[0];

		//	confine to create string buzz akin to flat bridge
		if (massPoss[0] < 0.0f)
		{
			massPoss[0] = stringBuzz * massPoss[0];
		}

		//	calculate postion of point 2
		massPoss[1] = massPossPrevious1[(0)] * schemeParameterB[2] + massPossPrevious1[(1)] * schemeParameterB[1] + massPossPrevious1[(2)] * schemeParameterB[2] + massPossPrevious1[(3)] * schemeParameterB[3] - schemeParameterC * massPossPrevious2[1];
		
		//	confine to create string buzz akin to flat bridge
		if (massPoss[1] < 0.0f)
		{
			massPoss[1]= stringBuzz * massPoss[1];
		}

		//	calculate positions of middle points
		for (int i = 2; i < (segmentNumber - 3); i++)
		{
			massPoss[i] = massPossPrevious1[(i - 2)] * schemeParameterB[3] + massPossPrevious1[(i - 1)] * schemeParameterB[2] + massPossPrevious1[(i)] * schemeParameterB[1] + massPossPrevious1[(i + 1)] * schemeParameterB[2] + +massPossPrevious1[(i + 2)] * schemeParameterB[3] - schemeParameterC * massPossPrevious2[i];
		}

		//	calculate positions of end points
		massPoss[(segmentNumber - 2)] = massPossPrevious1[(segmentNumber - 1)] * schemeParameterB[2] + massPossPrevious1[(segmentNumber - 2)] * schemeParameterB[1] + massPossPrevious1[(segmentNumber - 3)] * schemeParameterB[2] + massPossPrevious1[(segmentNumber - 4)] * schemeParameterB[3] - schemeParameterC * massPossPrevious2[(segmentNumber - 2)];
		massPoss[(segmentNumber - 1)] = massPossPrevious1[(segmentNumber - 1)] * schemeParameterB[0] + massPossPrevious1[(segmentNumber - 2)] * schemeParameterB[2] + massPossPrevious1[(segmentNumber - 3)] * schemeParameterB[3] - schemeParameterC * massPossPrevious2[(segmentNumber - 1)];
		
		//	input sample to string
		massPoss[4] = massPoss[4] + input;

		//	output sample from near end
		output = massPossPrevious2[(segmentNumber - 10)];
		
		//	pass state
		float* tempPtr;
		tempPtr = massPossPrevious2;
		massPossPrevious2 = massPossPrevious1;
		massPossPrevious1 = massPoss;
		massPoss = tempPtr;

		return output;
	}

	/**
	* set string tension
	* @param float: tension
	*/
	void setTension(float t )
	{
		tension = t;
	}

	/**
	* set string radius
	* @param float: radius
	*/
	void setRadius(float r)
	{
		radius = r;
	}

	/**
	* set string stiffness
	* @param float: stiffness
	*/
	void setStiffness(float s)
	{
		stiffness = s;
	}

	/**
	* set string nominal length
	* @param float: length
	*/
	void setLength(float l)
	{
		defaultLength = l;
	}

	/**
	* set string damping
	* @param float: damping
	*/
	void setDamping(float d)
	{
		damping = d;
	}

	/**
	* set string density
	* @param float: density
	*/
	void setDensity(float d)
	{
		density = d;
	}
		
	/**
	* set time step size
	* @param float: sample rate
	*/
	void setTimeStep(float sr)
	{
		timeStep = 1 / sr;
	}

	/**
	* set amount of desired string buzz
	* @param float: string buzz parameter (0-1)
	*/
	void setStringBuzz(float sb)
	{
		stringBuzz = sb;
	}

	/**
	* set tuning offset from default
	* @param float: tuning offset in semitones
	*/
	void setGlobalTuning(float t)
	{
		globalTuning = t;
	}

private:

	
	float tension = 60;
	float radius = 0.0004;
	float stiffness = 0.0040212386;
	float defaultLength = 0.1;
	float damping = 50;
	float density = 7850;

	float globalTuning = 0.0f;
	
	float stringBuzz = 0.9;

	int segmentNumber;
	float timeStep;

	float output = 0.0f;

	float schemeParameterB[4];
	float schemeParameterC;

	float* massPoss = nullptr;
	float* massPossPrevious1 = nullptr;
	float* massPossPrevious2 = nullptr;
	

};