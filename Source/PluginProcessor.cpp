/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>;

//==============================================================================
NeutralFaceCompressorAudioProcessor::NeutralFaceCompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

NeutralFaceCompressorAudioProcessor::~NeutralFaceCompressorAudioProcessor()
{
}

//==============================================================================
const juce::String NeutralFaceCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NeutralFaceCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NeutralFaceCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NeutralFaceCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NeutralFaceCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NeutralFaceCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NeutralFaceCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NeutralFaceCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NeutralFaceCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void NeutralFaceCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NeutralFaceCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    deltaTime = 1.0 / sampleRate;
}

void NeutralFaceCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NeutralFaceCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NeutralFaceCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int totalNumInputChannels  = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float threshold_dB = apvts.getRawParameterValue("threshold")->load();
    float ratio = apvts.getRawParameterValue("ratio")->load();
    float attack = apvts.getRawParameterValue("attack")->load();
    float release = apvts.getRawParameterValue("release")->load();
    float rms = apvts.getRawParameterValue("RMS")->load();
    bool feedback = apvts.getRawParameterValue("feedback")->load();

    float* const* samples = buffer.getArrayOfWritePointers();
        
    for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex)
    {
        float maxAmplitude = 0.f;

        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            maxAmplitude = std::fmax(std::fabs(samples[channel][sampleIndex]), maxAmplitude);
        }

        float rmsCoefficient = 1 - exp(-1000.f / (rms * getSampleRate()));
        lastUnprocessedSample = lerp(lastUnprocessedSample, maxAmplitude, rmsCoefficient);
        float controllingSample_dB = feedback ? toDB(lastProcessedSample) : toDB(lastUnprocessedSample);

        float target_dB = threshold_dB - (threshold_dB - controllingSample_dB) / ratio;
        float targetGain_dB = controllingSample_dB > threshold_dB ? target_dB - controllingSample_dB : 0.f;
        float envelopeTime = targetGain_dB > toDB(gain) ? release : attack;
        float coefficient = 1 - exp(-1000.f / (envelopeTime * getSampleRate()));
        float gain_dB = lerp(toDB(gain), targetGain_dB, coefficient);
        gain = dBToGain(gain_dB);
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            samples[channel][sampleIndex] = samples[channel][sampleIndex] * gain;
        }

        lastProcessedSample = maxAmplitude * gain;
    }
}

//==============================================================================
bool NeutralFaceCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NeutralFaceCompressorAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void NeutralFaceCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NeutralFaceCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeutralFaceCompressorAudioProcessor();
}
