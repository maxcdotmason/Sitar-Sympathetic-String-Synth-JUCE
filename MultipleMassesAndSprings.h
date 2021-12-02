#pragma once
#define MultipleMassesAndSprings_h
#include <cmath>

/**
A mass string system of variable masses, 
a velocity is imparted on all of the masses
their positions over time summed for the output
*/
class MultipleMassesAndSprings
{
public:

	/**
	Constructor
	*/
	MultipleMassesAndSprings()
	{
		massPoss = new float[21];
		massPossPrevious1 = new float[21];
		massPossPrevious2 = new float[21];
		schemeParameter = new float[450];
		sustainSchemeParameter = new float[450];
	}

	/**
	Destructor
	*/
	~MultipleMassesAndSprings()
	{
		delete[] massPoss;
		delete[] massPossPrevious1;
		delete[] massPossPrevious2;
		delete[] schemeParameter;
		delete[] sustainSchemeParameter;
	}

	/**
	initialise the system with variables and calculate scheme parameters

	@param float sample rate
	@param int number of masses
	@param float damping coefficient
	@param float mass of first mass
	@param float increment of mass between each mass
	@param float spring constant of first spring
	@param float increment of spring constant between each spring
	@param float velocity of first mass at t = 0
	@param float increment of initial velocity between each mass
	@param float damping of system which sustain held
	*/
	void init(float sampleRateI, int massNumberI, float dampingI, float mass1I, float dMassI, float spring1I, float dSpringI, float velocity1I, float dVelocityI, float sustainDampingI)
	{
		//	set member variables to incoming values
		setTimeStep(sampleRateI);
		setMassNum(massNumberI);
		setDamping(dampingI);
		setMass1(mass1I);
		setDMass(dMassI);
		setSpring1(spring1I);
		setDSpring(dSpringI);
		setVelocity1(velocity1I);
		setDVelocity(dVelocityI);
		setSustainDamping(sustainDampingI);

		//	initialise incrementing values
		float massSum = mass1;
		float springSum = spring1;
		float velocitySum = velocity1;

		//	set to 0
		for (int i = 0; i < 21; i++)
		{
			masses[i] = 0.0f;
			springs[i] = 0.0f;
			velocitys[i] = 0.0f;
		}

		//	for each mass/spring set value based upon previous value and increment
		for (int i = 0; i < massNum; i++)
		{
			masses[i] = massSum;
			massSum = massSum + dMass;

			velocitys[i] = velocitySum;
			velocitySum = velocitySum + dVelocity;

			springs[i] = springSum;
			springSum = springSum + dSpring;
		}
		springs[massNum] = springSum;					//	set final spring (more springs then masses)

		//	calculate scheme parameters
		dampingCoefficient = (6 * log(10)) / damping;
		sustainDampingCoefficient = (6 * log(10)) / sustainDamping;
		dampingParameter = (1 - (dampingCoefficient * timeStep)) / (1 + (dampingCoefficient * timeStep));
		sustainDampingParameter = (1 - (sustainDampingCoefficient * timeStep)) / (1 + (sustainDampingCoefficient * timeStep));

		//	initialise size of matrix
		int numSquared = pow(massNum, 2);

		//	for each potition in matrix
		for (int i = 0; i < numSquared; i++)
		{
			//	find location reference of current position
			int down = fmod(i , massNum) + 1;
			int across = ceil(i / massNum) + 1;

			//	set to 0
			schemeParameter[i] = 0;

			//	if current position is on diagonal
			if (down == across)
				{
				schemeParameter[i] = (2 + ((-springs[across] - springs[(across - 1)]) * pow(timeStep, 2) / masses[across-1])) / (1 + (dampingCoefficient * timeStep));
				}

			//	if current position is on superdiagonal
			if (down == across + 1)
				{
				schemeParameter[i] = (springs[down - 1] * pow(timeStep, 2) / masses[across - 1]) / (1 + (dampingCoefficient * timeStep));
				}

			//	if current position is on subdiagonal
			if (down == across - 1)
				{
				schemeParameter[i] = (springs[across - 1] * pow(timeStep, 2) / masses[down]) / (1 + (dampingCoefficient * timeStep));
				}	
		}

		//	for each position in matrix
		for (int i = 0; i < numSquared; i++)
		{
			//	find position
			int down = fmod(i, massNum) + 1;
			int across = ceil(i / massNum) + 1;

			//	set to 0
			sustainSchemeParameter[i] = 0;
			
			//	if position is on diagonal
			if (down == across)
			{
				sustainSchemeParameter[i] = (2 + ((-springs[across] - springs[(across - 1)]) * pow(timeStep, 2) / masses[across - 1])) / (1 + (sustainDampingCoefficient * timeStep));
			}

			//	if position is on superdiagonal
			if (down == across + 1)
			{
				sustainSchemeParameter[i] = (springs[down - 1] * pow(timeStep, 2) / masses[across - 1]) / (1 + (sustainDampingCoefficient * timeStep));
			}

			//	if position is on subdiagonal
			if (down == across - 1)
			{
				sustainSchemeParameter[i] = (springs[across - 1] * pow(timeStep, 2) / masses[down]) / (1 + (sustainDampingCoefficient * timeStep));
			}
		}

		//	set to initial conditions
		for (int i = 0; i < massNum; i++)
		{
			massPossPrevious2[i] = 0.0f;
			massPossPrevious1[i] = timeStep*velocitys[i];
		}

		//	find number of samples for which output will be audible
		countMax = massNum * damping * sampleRateI;
	}

	/**
	step the simulation and output current positions

	@param bool is sustain pedal down
	@param bool is key held down
	*/
	float process(bool sustain, bool keyDown)
	{
		//	set the output to zero
		output = 0.0f;

		// for each mass
		for (int i = 0; i < massNum; i++)
		{
			//	set mass position to 0
			massPoss[i] = 0.0f;

			//	if the note should be held and decay with the "sustain damping"
			if (sustain || keyDown)
			{	
				// for the adjacent masses
				for (int j = (i-1); j < (i+2); j++)
				{	
					//	if the adjacent masses are outside of range ignore them
					if ((j >= 0) && (j < massNum))
					{
						//	calculate position of current mass based on adjacent masses
						massPoss[i] = sustainSchemeParameter[i + (massNum * j)] * massPossPrevious1[j] + massPoss[i];
					}
				}

				//	subtract effects of damping
				massPoss[i] = massPoss[i] - massPossPrevious2[i] * sustainDampingParameter;
			}

			//	if the note is not held and will deacy quickly
			else
			{
				//	for the adjacent masses
				for (int j = (i - 1); j < (i + 2); j++)
				{
					//	if the adjacent masses are outside of range ignore them
					if ((j >= 0) && (j < massNum))
					{
						//	calculate position of current mass based on adjacent masses
						massPoss[i] = schemeParameter[i + (massNum * j)] * massPossPrevious1[j] + massPoss[i];
					}
				}

				//	subtract effects of damping
				massPoss[i] = massPoss[i] - massPossPrevious2[i] *dampingParameter;

				//	count samples during which the sound is decaying
				count = count + 1;
			}			
					
			//	add position of mass to output
			output = massPossPrevious2[i] + output;				
		}

		//	if the count has reached the point where the sound is inaudible
		if (count > countMax)
		{
			//	update time to stop bool and reset counter
			timeToStop = true;
			count = 0;
		}

		//	pass state
		float* tempPtr;

		tempPtr = massPossPrevious2;
		massPossPrevious2 = massPossPrevious1;
		massPossPrevious1 = massPoss;
		massPoss = tempPtr;

		return output;
	}

	/**
	returns whether it is time to stop this voice when queried
	*/
	bool isTimeToStop()
	{
		return timeToStop;
		
	}
	/**
	sets whether it is time to stop from outside class
	@param bool whether is time to stop true/false
	*/
	void setTimeToStop(bool i)
	{
		timeToStop = i;
	}

	/**
	* set number of masses
	* @param int: number of masses
	*/
	void setMassNum(int m)
	{
		massNum = m;
	}

	/**
	* set damping parameter
	* @param float: damping
	*/
	void setDamping(float d)
	{
		damping = d;
	}

	/**
	* set mass of mass 1
	* @param float: mass of mass 1
	*/
	void setMass1(float m)
	{
		mass1 = m;
	}

	/**
	* set mass increment
	* @param float: mass increment
	*/
	void setDMass(float m)
	{
		dMass = m;
	}

	/**
	* set spring canstant of spring 1
	* @param float: spring constant of spring 1
	*/
	void setSpring1(float s)
	{
		spring1 = s;
	}

	/**
	* set increment of spring constants
	* @param float: spring increments
	*/
	void setDSpring(float s)
	{
		dSpring = s;
	}

	/**
	* set initial velocity
	* @param float: initial velocity
	*/
	void setVelocity1(float v)
	{
		velocity1 = v;
	}

	/**
	* set initial velocity increment
	* @param float: velocity increment
	*/
	void setDVelocity(float v)
	{
		dVelocity = v;
	}

	/**
	* set time step
	* @param float: sample rate
	*/
	void setTimeStep(float sr)
	{
		timeStep = 1/sr;
	}

	/**
	* set sustain damping
	* @param float: sustain damping
	*/
	void setSustainDamping(float sd)
	{
		sustainDamping = sd;
	}

private:

	int massNum = 3;
	float damping = 5.0f;
	float mass1;
	float dMass;
	float spring1;
	float dSpring;
	float velocity1;
	float dVelocity;
	float timeStep;
	float output = 0.0f;

	int count = 0;
	int countMax;
	bool timeToStop = false;

	float sustainDamping = 15;

	float dampingCoefficient;
	float sustainDampingCoefficient;
	float dampingParameter;
	float sustainDampingParameter;
		
	float* schemeParameter = nullptr;
	float* sustainSchemeParameter = nullptr;

	float* massPoss = nullptr;
	float* massPossPrevious1 = nullptr;
	float* massPossPrevious2 = nullptr;

	float masses[21];
	float springs[21];
	float velocitys[21];
};