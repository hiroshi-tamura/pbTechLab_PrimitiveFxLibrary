#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <memory>
#include <vector>

namespace pbpfl
{
enum class PluginKind
{
    Reverb,
    Delay,
    Distortion,
    Compressor,
    Limiter,
    ThreeBandEQ,
    FourBandEQ,
    PitchShifter,
    Chorus,
    Phaser,
    Flanger,
    StereoEnhancer,
    StereoWidth,
    MidSideProcessor,
    AutoPan,
    Tremolo,
    Vibrato
};

struct ParameterSpec
{
    const char* id;
    const char* name;
    double minValue;
    double maxValue;
    double defaultValue;
    double step;
    const char* unit;
};

PluginKind getPluginKind();
juce::String getPluginName();
juce::String getPluginDescription();
std::vector<ParameterSpec> getParameterSpecs();
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

class PrimitiveFxAudioProcessor final : public juce::AudioProcessor
{
public:
    PrimitiveFxAudioProcessor();
    ~PrimitiveFxAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return getPluginName(); }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override;

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void setParameterFromWeb(const juce::String& id, float value);
    juce::var makeParameterStateForWeb() const;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> inputPeak { 0.0f };
    std::atomic<float> outputPeak { 0.0f };
    std::atomic<float> hostBpm { 120.0f };

private:
    float param(const char* id) const;
    void updateInputOutputMeters(const juce::AudioBuffer<float>& input, const juce::AudioBuffer<float>& output);
    void ensureWorkBufferSize(int channels, int samples);
    void processReverb(juce::AudioBuffer<float>& wet);
    void processDelay(juce::AudioBuffer<float>& wet);
    void processDistortion(juce::AudioBuffer<float>& wet);
    void processCompressor(juce::AudioBuffer<float>& wet);
    void processLimiter(juce::AudioBuffer<float>& wet);
    void processThreeBandEQ(juce::AudioBuffer<float>& wet);
    void processFourBandEQ(juce::AudioBuffer<float>& wet);
    void processPitchShifter(juce::AudioBuffer<float>& wet);
    void processChorus(juce::AudioBuffer<float>& wet);
    void processPhaser(juce::AudioBuffer<float>& wet);
    void processFlanger(juce::AudioBuffer<float>& wet);
    void processStereoEnhancer(juce::AudioBuffer<float>& wet);
    void processStereoWidth(juce::AudioBuffer<float>& wet);
    void processMidSideProcessor(juce::AudioBuffer<float>& wet);
    void processAutoPan(juce::AudioBuffer<float>& wet);
    void processTremolo(juce::AudioBuffer<float>& wet);
    void processVibrato(juce::AudioBuffer<float>& wet);
    void mixDryWet(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>& dry, float dryWet, float outputDb);

    double currentSampleRate { 44100.0 };
    int maxBlockSize { 512 };
    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> wetBuffer;

    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 192000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> modulationDelay { 192000 };
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Limiter<float> limiter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> eqLow;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> eqMidA;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> eqMidB;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> eqHigh;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Phaser<float> phaser;

    float delayFeedbackL { 0.0f }, delayFeedbackR { 0.0f };
    float limiterEnv { 1.0f };
    float lfoPhase { 0.0f };
    float smoothingStateL { 0.0f };
    float smoothingStateR { 0.0f };
    int pitchWritePos { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PrimitiveFxAudioProcessor)
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
