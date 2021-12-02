#pragma once
#define Oscillators_h
#include <cmath>

/**
Building block oscilators based on parent Phasor object
*/
class Phasor
{
public:

	/**
	* step phase and output 
	* @return phase
	*/
	float process()
	{
		phase += phaseDelta;								// increment phase

		if (phase > 1.0f)									// loop phase
			phase -= 1.0f;

		return output(phase);								// output phase to oscillators
	}

	/**
	* turn phase into useful wave
	* @param float: phase
	* @return float: oscilator
	*/
	virtual float output(float p)
	{
		return p;
	}

	/**
	* set sample rate
	* @param float: sample rate (Hz)
	*/
	void setSampleRate(float SR)
	{
		sampleRate = SR;
	}

	/**
	* set osc freqency
	* @param int: frequency (Hz)
	*/
	void setFrequency(float freq)
	{
		frequency = freq;
		phaseDelta = frequency / sampleRate;
	}

	/**
	* set phase
	* @param float: phase (0-10)
	*/
	void setPhase(float p)
	{
		phase = p ;
	}

private:
	float frequency;
	float sampleRate;
	float phase = 0.0f;
	float phaseDelta;
};

//=======================================

//CHILD class
/**
simple sin, returns value between -1:1 to be scaled outside of class
*/
class SinOsc: public Phasor
{
public:

	/**
	* turn phase into sin wave
	*
	* @param float: phase (0-1)
	* @return float: sin wave
	*/
	float output(float p) override
	{
		return sin(p * 2.0f * 3.14159f);
	}
			
private:
			
};

