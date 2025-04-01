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

    juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "threshold",
            "threshold",
            juce::NormalisableRange<float>(-50.f, 0.f, 0.1f, 1.0f),
            -20.f
        ));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "ratio",
            "ratio",
            juce::NormalisableRange<float>(1.0f, 16.0f, 0.1f, 0.6f),
            4.f
        ));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "attack",
            "attack",
            juce::NormalisableRange<float>(0.f, 500.f, 0.1f, 0.2f),
            10.0f
        ));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "release",
            "release",
            juce::NormalisableRange<float>(0.f, 500.f, 0.1f, 0.2f),
            200.0f
        ));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "RMS",
            "RMS",
            juce::NormalisableRange<float>(0.f, 500.f, 0.1f, 0.2f),
            5.0f
        ));

        layout.add(std::make_unique<juce::AudioParameterBool>(
            "feedback",
            "feedback",
            false
        ));

        return layout;
    }

    juce::AudioProcessorValueTreeState apvts = juce::AudioProcessorValueTreeState (
        *this,
        nullptr,
        "Parameters",
        getParameterLayout()
    );

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeutralFaceCompressorAudioProcessor)

    double deltaTime;
    float lastUnprocessedSample = 0.f;
    float lastProcessedSample = 0.f;
    float gain = 1.f;

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
        return a + t * (b - a);
    }

    static inline float getCoefficient(float deltaTime_ms, float sampleRate) {
        // Equivalent to 1 - exp((1/sampleRate) / deltaTime)
        return 1 - exp(-1000.f / (deltaTime_ms * sampleRate));
    }
    
};
