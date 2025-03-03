/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class NeutralFaceCompressorAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    NeutralFaceCompressorAudioProcessor();
    ~NeutralFaceCompressorAudioProcessor() override;

    //==============================================================================
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
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeutralFaceCompressorAudioProcessor)

    double deltaTime;
    float lastProcessedSample = 0.f;
    float lastUnprocessedSample = 0.f;
    float gain = 1.f;

    float threshold_dB = -20.0f;
    float ratio = 4.0f;
    float attack = 50.0f;
    float release = 300.0f;

    // Threshold, Ratio, Attack, Release
    // Mix, Makeup Gain, Oversampling
    // Knee, Lookahead, Look behind, Max Reduction, RMS Size, Hold
    // Feedforward/Feedback, Attack Shape, Release Shape
    // Main input/Sidechain

    static inline float toDB(float sample) {
        return juce::Decibels::gainToDecibels(sample);
    }

    static inline float fromMilliseconds(float milliseconds) {
        return milliseconds / 1000.0f;
    }

    static inline float dBToGain(float dB) {
        return juce::Decibels::decibelsToGain(dB);
    }

    static inline float lerp(float a, float b, float t) {
        return t * (b - a);
    }
};
