#pragma once
#define SingleVoiceChorus_h
#include <cmath>
#include "Oscillators.h"

/**
varying depth delay for use as an individual chorus voice
*/
class SingleVoiceChorus
{
public:

	/**
	Constructor
	*/
	SingleVoiceChorus()
	{

	}

	/**
	Destructor
	*/
	~SingleVoiceChorus()
	{
		delete [] delayLine;
	}

	/**
	initialise delay, calculates required interpolation curves
	@param float sample rate
	@param float depth modulating frequency
	*/
	void init(float sr, float f)
	{
		depth.setSampleRate(sr);	
		depth.setFrequency(f);

		delayLine = new float[maxDelay];								// initialise delay line 

		for (int i = 0; i < maxDelay; i++)
		{
			delayLine[i] = 0.0f;											// set all values to 0
		}

		for (int i = 0; i < fidelity; i++)									// for each point between whole samples
		{
			//	find 4 LeGrange curves for 3rd degree interpolation
			float a = (1.0f / fidelity) * i - 0.5f;						
			p1[i] = ((a + 0.5f) * (a - 0.5f) * (a - 1.5f)) / -6.0f;
			p2[i] = ((a + 1.5f) * (a - 0.5f) * (a - 1.5f)) / 2.0f;
			p3[i] = ((a + 0.5f) * (a - 1.5f) * (a + 1.5f)) / -2.0f;
			p4[i] = ((a + 0.5f) * (a - 0.5f) * (a + 1.5f)) / 6.0f;
		}

		writeHeadPos = 0;												// set intialial write position to 0
	}

	/**
	Process single sample.
	@param float: sample to be delayed
	@return float: delayed sample
	*/
	float process(float input)
	{
		writeHeadPos += 1;												// increment write position
		writeHeadPos = writeHeadPos % maxDelay;							// loop back to start if over the end
		float delayDepth = depthMean + depthRange * depth.process();	// find current delay length from sin term
		float frac = delayDepth - floor(delayDepth);					// find fraction part of delay length
		int stepNum = floor(frac * fidelity);							// find corrosponding LeGrange sample

		//	find sample numbers for use with interpolation
		int firstSample = floor(delayDepth) - 1;	
		int samples[4] = { firstSample, firstSample + 1, firstSample + 2, firstSample + 3 };

		float samplesToUse[4] = { 0.0f };								// initialise samples 

		for (int i = 0; i < 4; i++)											// for each LeGrange curve
		{
			//	read delayed value
			int readPos = (maxDelay + writeHeadPos - samples[i]) % maxDelay;	
			samplesToUse[i] = delayLine[readPos];
		}

		//	multiply by corrospnding curve and sum
		float output = samplesToUse[0] * p1[stepNum] + samplesToUse[1] * p2[stepNum] + samplesToUse[2] * p3[stepNum] + samplesToUse[3] * p4[stepNum];

		delayLine[writeHeadPos] = input;								// write incoming sample to write location on delay line

		return output;
	}

	/**
		* set depth of delay, sets mean depth and range.
		* @param float: depth in samples
		*/
	void setDepthMean(float dm)
	{
		depthMean = dm;
		depthRange = dm / 2;
	}

	/**
	* set modulation frequency
	* @param float: frequency (Hz)
	*/
	void setFreq(float f)
	{
		depth.setFrequency(f);
	}

private:

	SinOsc depth;
	float depthMean = 400.0f;
	float depthRange = 200.0f;

	int maxDelay = 1000;
	int writeHeadPos = 0;
	float fidelity = 100.0f;

	float p1[100] = { 0.0f };
	float p2[100] = { 0.0f };
	float p3[100] = { 0.0f };
	float p4[100] = { 0.0f };


	float* delayLine = nullptr;

};
