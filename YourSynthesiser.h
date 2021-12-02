/*
  ==============================================================================

    YourSynthesiser.h
    Created: 7 Mar 2020 4:27:57pm
    Author:  Tom Mudd && Max Mason

  ==============================================================================

*/

#pragma once
#include <JuceHeader.h>
#include "MultipleMassesAndSprings.h"

// ===========================
// ===========================
// SOUND
class NewNameSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    //--------------------------------------------------------------------------
    bool appliesToChannel(int) override { return true; }
};

// =================================
// =================================
// Synthesiser Voice - your synth code goes in here

/*!
 @class YourSynthVoice
 @abstract struct defining the DSP associated with a specific voice.
 @discussion multiple YourSynthVoice objects will be created by the Synthesiser so that it can be played polyphicially

 @namespace none
 @updated 2019-06-18
 */
class YourSynthVoice : public juce::SynthesiserVoice
{
public:
    YourSynthVoice() {}

    /**
    * set number of massed
    * @param int: number of masses
    */
    void setMassNum(float m)
    {
        massNumber = round(m);
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
    * set spring contant of spring 1
    * @param float: spring constant of spring 1
    */
    void setSpring1(float s)
    {
        spring1 = s;
    }

    /**
    * set spring constant increment
    * @param float: spring constant increment
    */
    void setDSpring(float s)
    {
        dSpring = s;
    }

    /**
    * set damping
    * @param float: damping
    */
    void setDamping(float d)
    {
        damping = d;
    }

    /**
    * set octave
    * @param float: octave
    */
    void setOctave(float o)
    {
        octave = o;
    }

    /**
    * set sustain damping
    * @param float: sustain damping
    */
    void setSustainDamping(float sd)
    {
        sustainDamping = sd;
    }


    //--------------------------------------------------------------------------
    /**
     What should be done when a note starts

     @param midiNoteNumber
     @param velocity
     @param SynthesiserSound unused variable
     @param / unused variable
     */
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        //  voice should be sounding
        playing = true;

        //  if chosen octave is 2, add 36 to midi in
        if (octave == 2)
        {
            midiNoteNumber = midiNoteNumber + 36;
        }

        //  if chosen ocatve is 1, add 24 to midi in
        if (octave == 1)
        {
            midiNoteNumber = midiNoteNumber + 24;
        }

        //  if chosen octave is 0, add 12 to midi in
        if (octave == 0)
        {
            midiNoteNumber = midiNoteNumber + 12;
        }

        //  calculate masses and spring constants based on user settings and midi information
        float keyMass = pow(mass1, 2);
        float keyDMass =  pow(dMass,2);
        float keySpring = pow((juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber)*2.0*juce::float_Pi),2.0f)*keyMass;
        float keyDSpring = pow(dSpring,2);
        float vel = velocity * 0.5;
        float dVel = velocity * 0.1;

        //  initialise the coupled mass sytem 
        firstCouple.init(getSampleRate(), massNumber, damping, keyMass, keyDMass, keySpring, keyDSpring, vel, dVel, sustainDamping);

        //  set attack counter to 0 and set key to down
        attackCount = 0;
        keyDown = true;

        //  calculate numeber of samples required for attack duration
        attackDurationSamples = attackDuration * getSampleRate();
    }

    //--------------------------------------------------------------------------
    /// Called when a MIDI noteOff message is received
    /**
     What should be done when a note stops

     @param / unused variable
     @param allowTailOff bool to decie if the should be any volume decay
     */
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        //  key has been released
        keyDown = false;
    }


    //--------------------------------------------------------------------------
    /// Called when a MIDI noteOff message is received
    /**
     What should be done when a note stops

    @param / unused variable
    @param allowTailOff bool to decie if the should be any volume decay
    */


    //--------------------------------------------------------------------------
    /**
     The Main DSP Block: Put your DSP code in here

     If the sound that the voice is playing finishes during the course of this rendered block, it must call clearCurrentNote(), to tell the synthesiser that it has finished

     @param outputBuffer pointer to output
     @param startSample position of first sample in buffer
     @param numSamples number of smaples in output buffer
     */
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (playing) // check to see if this voice should be playing
        {
            // iterate through the necessary number of samples (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
            {
                //  process coupled mass system
                float currentSample = firstCouple.process(isSustainPedalDown(), keyDown);

                //  if during attack period
                if (attackCount < attackDurationSamples)
                {   
                    // linearly increase the volume ove rthe attack period
                    currentSample = currentSample * (attackCount/attackDurationSamples);

                    //  increment attack counter
                    attackCount = attackCount + 1;
                }

                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan < outputBuffer.getNumChannels(); chan++)
                {
                    // The output sample is scaled by 0.2 so that it is not too loud by default
                    outputBuffer.addSample(chan, sampleIndex, currentSample);
                }
                 
                //  check if the sprung masses have become inaudible
                if (firstCouple.isTimeToStop())
                {
                    //  if they have then clear the note and tell everything it is done
                    clearCurrentNote();
                    playing = false;
                    firstCouple.setTimeToStop(false);
                }
            }
        }
    }
    //--------------------------------------------------------------------------
    void pitchWheelMoved(int) override {}
    //--------------------------------------------------------------------------
    void controllerMoved(int, int) override {}
    //--------------------------------------------------------------------------
    /**
     Can this voice play a sound. I wouldn't worry about this for the time being

     @param sound a juce::SynthesiserSound* base class pointer
     @return sound cast as a pointer to an instance of YourSynthSound
     */
    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<NewNameSound*> (sound) != nullptr;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    // Set up any necessary variables here
    /// Should the voice be playing?
    bool playing = false;

    MultipleMassesAndSprings firstCouple;
 
    float massNumber = 8;
    float damping = 10;
    float mass1 = 0.01;
    float dMass = 0.02;
    float spring1 = 50000;
    float dSpring = 5000;
    int octave = 0;
    float sustainDamping = 15;

    int attackCount = 0;
    float attackDuration = 0.01f;
    float attackDurationSamples = 0.0f;

    float velocity1 = 0.1;
    float dVelocity = 0.00;
    float vel = 0.0f;
    bool keyDown = false;

};