/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "YourSynthesiser.h"
#include "SympathyStrings.h"
#include "SingleVoiceChorus.h"
#include <vector>


//==============================================================================
/**
*/
class CoupledMassAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CoupledMassAudioProcessor();
    ~CoupledMassAudioProcessor() override;
    //==============================================================================
    void stringReseter();
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:

    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float>* mass1Param;
    std::atomic<float>* dMassParam;
    std::atomic<float>* dSpringParam;
    std::atomic<float>* dampingParam;
    std::atomic<float>* massNumParam;
    std::atomic<float>* octaveSelectParam;
    std::atomic<float>* sustainDampingParam;
    std::atomic<float>* wetVolumeParam;
    std::atomic<float>* dryVolumeParam;
    std::atomic<float>* stringBuzzParam;
    std::atomic<float>* stringResetParam;
    std::atomic<float>* stringDampingParam;
    std::atomic<float>* lowPassFreqParam;
    std::atomic<float>* chorusVolParam;
    std::atomic<float>* chorusDepthParam;
    std::atomic<float>* chorusFreqParam;
    std::atomic<float>* stringTuningParam;
    std::atomic<float>* p4thTuningParam;

    //  instance of synthesiser class
    juce::Synthesiser synth;
    
    //  vector or strings
    std::vector<SympathyStrings*> sympathyStrings;

    //  instance of filter class
    juce::IIRFilter lowPass;

    //  vector of chorus voices
    std::vector<SingleVoiceChorus*> choruses;

    float sr;               

    int voiceCount = 32;
    int stringCount = 8;
    int chorusCount = 4;

    float stringResetCheck = 0.0f;

    float dryVolume = 1000.0f;
    float wetVolume = 10.0f;

    //  string parameters
    float tensions[8] = { 53.4, 53.4, 53.4f, 70.3f, 70.3f, 70.3f, 70.3f, 70.3f };
    float radiuses[8] = { 0.000415, 0.000415 ,0.000415, 0.000362, 0.000362, 0.000362 ,0.000362, 0.000362 };
    float stiffnesses[8] = { 0.00016, 0.00016, 0.00016, 0.00013, 0.00013, 0.00013, 0.00013, 0.00013 };
    float lengths[8] = { 0.5791, 0.5159, 0.4596, 0.5466,  0.5159, 0.4596, 0.4095, 0.3865 };
    float dampings[8] = { 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000 };
    float densities[8] = {959, 959, 959, 923.3, 923.3, 923.3, 923.3, 923.3 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoupledMassAudioProcessor)
};
