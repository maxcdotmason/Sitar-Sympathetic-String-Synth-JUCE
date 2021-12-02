/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CoupledMassAudioProcessor::CoupledMassAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters(*this, nullptr, "ParamTreeId", {
    std::make_unique<juce::AudioParameterFloat>("massNum","Number of Masses",2.0f,20.0f, 10.0f),
     std::make_unique<juce::AudioParameterFloat>("dryVolume","Volume of Masses",0.0f,100.0f,25.0f),
    std::make_unique<juce::AudioParameterFloat>("wetVolume","Volume of Strings",0.0f,100.0f,30.0f),
    std::make_unique<juce::AudioParameterFloat>("chorusVol","Volume of Choruses",0.0f,100.0f,50.0f),
    std::make_unique<juce::AudioParameterFloat>("octaveSelect","Select Octave",-1.0f,2.0f,0.0f),
    std::make_unique<juce::AudioParameterFloat>("mass1","Mass of First Mass (kg)", 3.0f, 10.0f, 6.84f),
    std::make_unique<juce::AudioParameterFloat>("dMass","Increment Mass (kg)", 0.01f, 5.0f, 1.43f),
    std::make_unique<juce::AudioParameterFloat>("dSpring","Increment Spring Constant (N/m)", 0.0f,25000.0f, 1000.0f),
    std::make_unique<juce::AudioParameterFloat>("damping","Decay Time (s)", 0.1f,10.0f, 2.0f),
    std::make_unique<juce::AudioParameterFloat>("sustainDamping","Sustain Decay Time (s)",5.0f,40.0f,35.0f),
    std::make_unique<juce::AudioParameterFloat>("stringDamping","String Decay Time (s)",1.0f,50.0f,5.4f),
    std::make_unique<juce::AudioParameterFloat>("stringTuning","String Key Tuning (semitones)",0.0f,12.0f,0.0f),
    std::make_unique<juce::AudioParameterBool>("p4thTuning","Perfect 4th on",false),
    std::make_unique<juce::AudioParameterBool>("stringReset","String Reset",false),
    std::make_unique<juce::AudioParameterFloat>("lowPassFreq","Low Pass Cut-Off (Hz)",100.0f,10000.0f,10000.0f),
    std::make_unique<juce::AudioParameterFloat>("stringBuzz","String Buzz Reduction",0.0f,1.0f,0.36f),
    std::make_unique<juce::AudioParameterFloat>("chorusDepth","Chorus Depth (samples)",100.0f,500.0f,200.0f),
    std::make_unique<juce::AudioParameterFloat>("chorusFreq","Chorus Frequency (Hz)",0.1f,2.0f,0.5f)
    
    })

{
        // constructor//////////////////////////////////////
    ///////////////////////////////////////////////////////

    mass1Param = parameters.getRawParameterValue("mass1");
    dMassParam = parameters.getRawParameterValue("dMass");
    dSpringParam = parameters.getRawParameterValue("dSpring");
    dampingParam = parameters.getRawParameterValue("damping");
    octaveSelectParam = parameters.getRawParameterValue("octaveSelect");
    massNumParam = parameters.getRawParameterValue("massNum");
    sustainDampingParam = parameters.getRawParameterValue("sustainDamping");
    dryVolumeParam = parameters.getRawParameterValue("dryVolume");
    wetVolumeParam = parameters.getRawParameterValue("wetVolume");
    stringBuzzParam = parameters.getRawParameterValue("stringBuzz");
    stringResetParam = parameters.getRawParameterValue("stringReset");
    stringDampingParam = parameters.getRawParameterValue("stringDamping");
    lowPassFreqParam = parameters.getRawParameterValue("lowPassFreq");
    chorusVolParam = parameters.getRawParameterValue("chorusVol");
    chorusDepthParam = parameters.getRawParameterValue("chorusDepth");
    chorusFreqParam = parameters.getRawParameterValue("chorusFreq");
    stringTuningParam = parameters.getRawParameterValue("stringTuning");
    p4thTuningParam = parameters.getRawParameterValue("p4thTuning");

    //  for each voice add a voice
    for (int i = 0; i < voiceCount; i++)
    {
        synth.addVoice(new YourSynthVoice());
    }

    //  for each string add a string to the vector
    for (int i = 0; i < stringCount; i++)
    {                        
        sympathyStrings.push_back(new SympathyStrings());                            
    }

    //  for each chorus voice add a chorus voice to the vector
    for (int i = 0; i < chorusCount; i++)
    {
        choruses.push_back(new SingleVoiceChorus());
    }

    //  add a sound the the synth
    synth.addSound(new NewNameSound());

}

CoupledMassAudioProcessor::~CoupledMassAudioProcessor()
{
}

//==============================================================================
const juce::String CoupledMassAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CoupledMassAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CoupledMassAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CoupledMassAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CoupledMassAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CoupledMassAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CoupledMassAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CoupledMassAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CoupledMassAudioProcessor::getProgramName (int index)
{
    return {};
}

void CoupledMassAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void CoupledMassAudioProcessor::stringReseter()
{
    for (int i = 0; i < stringCount; i++)
    {
        sympathyStrings[i]->setDamping(*stringDampingParam);
        sympathyStrings[i]->reseter();
    }
}

//==============================================================================
void CoupledMassAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //  set current sample rate
    synth.setCurrentPlaybackSampleRate(sampleRate);

    //  initialise each string
    for (int i = 0; i < stringCount; i++)
    {
        sympathyStrings[i]->init(sampleRate, tensions[i], radiuses[i], stiffnesses[i], lengths[i], dampings[i], densities[i]);
    }

    //  set up and reset filter
    lowPass.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, 1000.0));
    lowPass.reset();

    //  initialise each chorus
    for (int i = 0; i < chorusCount; i++)
    {
        choruses[i]->init(sampleRate, float(i+1)*0.2f);
    }
   
    // sample rate for use elsewhere
    sr = sampleRate;
}

void CoupledMassAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CoupledMassAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CoupledMassAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
     // for each voice send current user settings
    for (int i = 0; i < voiceCount; i++)
    {
        YourSynthVoice* q = dynamic_cast<YourSynthVoice*>(synth.getVoice(i));
        q->setMassNum(*massNumParam);
        q->setMass1(*mass1Param);
        q->setDMass(*dMassParam);
        q->setDSpring(*dSpringParam);
        q->setDamping(*dampingParam);
        q->setOctave(*octaveSelectParam);
        q->setSustainDamping(*sustainDampingParam);
    }
    
    //  if string reset has been pressed
    if (*stringResetParam != stringResetCheck)
    {
        //  for each string set global tuning
        for (int i = 0; i < stringCount; i++)
        {
            sympathyStrings[i]->setGlobalTuning(*stringTuningParam);
        }

        //  if perfect fourth tuning enabled
        if (*p4thTuningParam > 0.5)
        {
            //  set string length to p4
           sympathyStrings[3]->setLength(0.5791f);
        }
        else
        {
            //  else set to #4
            sympathyStrings[3]->setLength(0.5466f);
        }

        //  reset the stings and the check
        stringReseter();
        stringResetCheck = *stringResetParam;
    }

    //  for each string send the current buzz setting
    for (int i = 0; i < stringCount; i++)
    {
        sympathyStrings[i]->setStringBuzz(*stringBuzzParam);
    }

    //  for each chorus voice send the current depths and frequencies
    for (int i = 0; i < chorusCount; i++)
    {
        choruses[i]->setDepthMean(*chorusDepthParam);
        choruses[i]->setFreq(*chorusFreqParam);
    }

    //  set the current low pass coefficients
    lowPass.setCoefficients(juce::IIRCoefficients::makeLowPass(sr, *lowPassFreqParam));
 
    //  voices are calculated
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    //  get locations of audio buffers
    auto* leftChannel = buffer.getWritePointer(0);      
    auto* rightChannel = buffer.getWritePointer(1);
     
    //  for each sample in block
    for (int i = 0; i < buffer.getNumSamples(); i++)                      
    {
        //  set outputs to 0
        float output = 0.0f;
        float output2 = 0.0f;
        float output3 = 0.0f;
        float output4 = 0.0f;

        //  for each string
        for (int j = 0; j < stringCount; j++)
        {   
            //  process the strings based on current sample and adjust volume
            output = sympathyStrings[j]->process(leftChannel[i]) * *wetVolumeParam + output;
        }

        //  sum strings and dry and pass through filter
        output4 = lowPass.processSingleSampleRaw(output + (leftChannel[i] * *dryVolumeParam * 100.0f))*0.1;
         
        //  for each chorus voice
        for (int j = 0; j < chorusCount/2; j++)
        {
            //  proccess voice and add to output
            output2 = choruses[j]->process(output4) + output2;
            output3 = choruses[j+1]->process(output4) + output3;
        }

        //  mix dry with chorus and send to output
        leftChannel[i] = (output2**chorusVolParam/100.0f  + output4) * 0.1f;
        rightChannel[i] = (output3**chorusVolParam/100.0f + output4) * 0.1f;
        
    }
    
}

//==============================================================================
bool CoupledMassAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CoupledMassAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void CoupledMassAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // getStateInformation
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CoupledMassAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // setStateInformation
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CoupledMassAudioProcessor();
}
