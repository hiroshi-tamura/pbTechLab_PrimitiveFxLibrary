#include "PrimitiveFxProcessor.h"
#include "PrimitiveFxEditor.h"

namespace pbpfl
{
namespace
{
constexpr float pi = juce::MathConstants<float>::pi;

PluginKind parseKind()
{
    const juce::String kind(PB_PLUGIN_KIND);
    if (kind == "Reverb") return PluginKind::Reverb;
    if (kind == "Delay") return PluginKind::Delay;
    if (kind == "Distortion") return PluginKind::Distortion;
    if (kind == "Compressor") return PluginKind::Compressor;
    if (kind == "Limiter") return PluginKind::Limiter;
    if (kind == "ThreeBandEQ") return PluginKind::ThreeBandEQ;
    if (kind == "FourBandEQ") return PluginKind::FourBandEQ;
    if (kind == "PitchShifter") return PluginKind::PitchShifter;
    if (kind == "Chorus") return PluginKind::Chorus;
    if (kind == "Phaser") return PluginKind::Phaser;
    if (kind == "Flanger") return PluginKind::Flanger;
    if (kind == "StereoEnhancer") return PluginKind::StereoEnhancer;
    if (kind == "StereoWidth") return PluginKind::StereoWidth;
    if (kind == "MidSideProcessor") return PluginKind::MidSideProcessor;
    if (kind == "AutoPan") return PluginKind::AutoPan;
    if (kind == "Tremolo") return PluginKind::Tremolo;
    return PluginKind::Vibrato;
}

float peakOf(const juce::AudioBuffer<float>& buffer)
{
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        peak = std::max(peak, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    return juce::jlimit(0.0f, 1.5f, peak);
}

float smooth(float current, float target, float coeff)
{
    return current + (target - current) * coeff;
}
}

PluginKind getPluginKind() { return parseKind(); }
juce::String getPluginName() { return PB_PLUGIN_NAME; }
juce::String getPluginDescription() { return PB_PLUGIN_DESCRIPTION; }

std::vector<ParameterSpec> getParameterSpecs()
{
    switch (getPluginKind())
    {
        case PluginKind::Reverb:
            return {{"size","Size",0,100,58,1,"%"},{"damping","Damping",0,100,42,1,"%"},{"diffusion","Diffusion",0,100,72,1,"%"},{"mod","Mod",0,100,18,1,"%"},{"width","Width",0,200,120,1,"%"},{"dryWet","Dry/Wet",0,100,35,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::Delay:
            return {{"time","Time",1,2000,360,1,"ms"},{"feedback","Feedback",0,95,38,1,"%"},{"tone","Tone",0,100,54,1,"%"},{"spread","Spread",0,100,28,1,"%"},{"dryWet","Dry/Wet",0,100,32,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::Distortion:
            return {{"drive","Drive",0,36,12,0.1,"dB"},{"bias","Bias",-1,1,0,0.01,""},{"tone","Tone",0,100,55,1,"%"},{"lowCut","Low Cut",20,1000,80,1,"Hz"},{"shape","Shape",0,100,45,1,"%"},{"dryWet","Dry/Wet",0,100,70,1,"%"},{"output","Output",-36,12,-6,0.1,"dB"}};
        case PluginKind::Compressor:
            return {{"threshold","Threshold",-60,0,-18,0.1,"dB"},{"ratio","Ratio",1,20,4,0.1,":1"},{"attack","Attack",0.1,100,12,0.1,"ms"},{"release","Release",10,1000,160,1,"ms"},{"makeup","Makeup",0,24,4,0.1,"dB"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::Limiter:
            return {{"input","Input",0,24,4,0.1,"dB"},{"ceiling","Ceiling",-12,0,-1,0.1,"dB"},{"release","Release",10,1000,90,1,"ms"},{"softClip","Soft Clip",0,100,25,1,"%"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::ThreeBandEQ:
            return {{"lowGain","Low",-18,18,0,0.1,"dB"},{"lowFreq","Low Freq",40,400,120,1,"Hz"},{"lowQ","Low Q",0.2,8,0.7,0.01,"Q"},{"midGain","Mid",-18,18,0,0.1,"dB"},{"midFreq","Mid Freq",200,5000,1200,1,"Hz"},{"midQ","Mid Q",0.2,8,1.1,0.01,"Q"},{"highGain","High",-18,18,0,0.1,"dB"},{"highQ","High Q",0.2,8,0.7,0.01,"Q"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::FourBandEQ:
            return {{"lowGain","Low",-18,18,0,0.1,"dB"},{"lowFreq","Low Freq",40,400,100,1,"Hz"},{"lowQ","Low Q",0.2,8,0.7,0.01,"Q"},{"mid1Gain","Low Mid",-18,18,0,0.1,"dB"},{"mid1Freq","Low Mid Freq",150,3000,650,1,"Hz"},{"mid1Q","Low Mid Q",0.2,8,1.0,0.01,"Q"},{"mid2Gain","High Mid",-18,18,0,0.1,"dB"},{"mid2Freq","High Mid Freq",800,9000,2800,1,"Hz"},{"mid2Q","High Mid Q",0.2,8,1.0,0.01,"Q"},{"highGain","High",-18,18,0,0.1,"dB"},{"highFreq","High Freq",3000,14000,7000,1,"Hz"},{"highQ","High Q",0.2,8,0.7,0.01,"Q"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::PitchShifter:
            return {{"semitones","Semitone",-24,24,0,1,"st"},{"cents","Cents",-100,100,0,1,"ct"},{"grain","Grain",20,180,80,1,"ms"},{"dryWet","Dry/Wet",0,100,50,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::Chorus:
            return {{"rate","Rate",0.05,8,0.8,0.01,"Hz"},{"depth","Depth",0,100,45,1,"%"},{"delay","Delay",1,35,14,0.1,"ms"},{"feedback","Feedback",-90,90,8,1,"%"},{"spread","Spread",0,180,110,1,"deg"},{"dryWet","Dry/Wet",0,100,45,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::Phaser:
            return {{"rate","Rate",0.03,8,0.55,0.01,"Hz"},{"depth","Depth",0,100,62,1,"%"},{"center","Center",200,5000,950,1,"Hz"},{"feedback","Feedback",-95,95,35,1,"%"},{"dryWet","Dry/Wet",0,100,55,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::Flanger:
            return {{"rate","Rate",0.03,8,0.32,0.01,"Hz"},{"depth","Depth",0,100,55,1,"%"},{"delay","Delay",0.1,15,4.5,0.1,"ms"},{"feedback","Feedback",-95,95,42,1,"%"},{"spread","Spread",0,180,135,1,"deg"},{"dryWet","Dry/Wet",0,100,50,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::StereoEnhancer:
            return {{"width","Width",0,200,130,1,"%"},{"enhance","Enhance",0,100,35,1,"%"},{"focus","Focus",0,100,55,1,"%"},{"bassMono","Bass Mono",0,100,30,1,"%"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::StereoWidth:
            return {{"width","Width",0,200,125,1,"%"},{"mono","Mono",0,100,0,1,"%"},{"balance","Balance",-100,100,0,1,"%"},{"rotation","Rotation",-45,45,0,1,"deg"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::MidSideProcessor:
            return {{"midGain","Mid Gain",-24,24,0,0.1,"dB"},{"sideGain","Side Gain",-24,24,0,0.1,"dB"},{"width","Width",0,200,100,1,"%"},{"balance","Balance",-100,100,0,1,"%"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::AutoPan:
            return {{"rate","Rate",0.03,120,1.2,0.01,"Hz"},{"depth","Depth",0,100,70,1,"%"},{"phase","Phase",0,180,180,1,"deg"},{"shape","Shape",0,100,50,1,"%"},{"offset","Offset",-100,100,0,1,"%"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::Tremolo:
            return {{"rate","Rate",0.03,120,4.0,0.01,"Hz"},{"depth","Depth",0,100,65,1,"%"},{"shape","Shape",0,100,45,1,"%"},{"stereo","Stereo",0,180,0,1,"deg"},{"smooth","Smooth",0,100,20,1,"%"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
        case PluginKind::Vibrato:
            return {{"rate","Rate",0.03,120,5.2,0.01,"Hz"},{"depth","Depth",0,100,35,1,"%"},{"delay","Delay",1,20,7,0.1,"ms"},{"shape","Shape",0,100,50,1,"%"},{"stereo","Stereo",0,180,90,1,"deg"},{"dryWet","Dry/Wet",0,100,100,1,"%"},{"output","Output",-24,12,0,0.1,"dB"}};
    }
    return {};
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    for (const auto& spec : getParameterSpecs())
    {
        auto range = juce::NormalisableRange<float>(
            static_cast<float>(spec.minValue),
            static_cast<float>(spec.maxValue),
            static_cast<float>(spec.step));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { spec.id, 1 }, spec.name, range, static_cast<float>(spec.defaultValue),
            juce::AudioParameterFloatAttributes().withLabel(spec.unit)));
    }
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "bypass", 1 }, "Bypass", false));
    return { params.begin(), params.end() };
}

PrimitiveFxAudioProcessor::PrimitiveFxAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

void PrimitiveFxAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    maxBlockSize = samplesPerBlock;
    ensureWorkBufferSize(getTotalNumOutputChannels(), samplesPerBlock);

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(std::max(1, getTotalNumOutputChannels())) };
    reverb.reset();
    delayLine.reset();
    delayLine.prepare(spec);
    modulationDelay.reset();
    modulationDelay.prepare(spec);
    compressor.prepare(spec);
    limiter.prepare(spec);
    chorus.prepare(spec);
    phaser.prepare(spec);
    eqLow.prepare(spec);
    eqMidA.prepare(spec);
    eqMidB.prepare(spec);
    eqHigh.prepare(spec);
    delayFeedbackL = delayFeedbackR = 0.0f;
    limiterEnv = 1.0f;
    smoothingStateL = smoothingStateR = 0.0f;
    lfoPhase = 0.0f;
    pitchWritePos = 0;
}

void PrimitiveFxAudioProcessor::releaseResources() {}

bool PrimitiveFxAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainIn = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();
    return (mainIn == mainOut) && (mainOut == juce::AudioChannelSet::mono() || mainOut == juce::AudioChannelSet::stereo());
}

double PrimitiveFxAudioProcessor::getTailLengthSeconds() const
{
    if (getPluginKind() == PluginKind::Reverb) return 4.0;
    if (getPluginKind() == PluginKind::Delay) return 2.0;
    return 0.0;
}

float PrimitiveFxAudioProcessor::param(const char* id) const
{
    if (auto* value = apvts.getRawParameterValue(id))
        return value->load();
    return 0.0f;
}

void PrimitiveFxAudioProcessor::ensureWorkBufferSize(int channels, int samples)
{
    dryBuffer.setSize(channels, samples, false, false, true);
    wetBuffer.setSize(channels, samples, false, false, true);
}

void PrimitiveFxAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (auto* currentPlayHead = getPlayHead())
        if (auto position = currentPlayHead->getPosition())
            if (auto bpm = position->getBpm())
                hostBpm.store(static_cast<float>(*bpm), std::memory_order_relaxed);

    ensureWorkBufferSize(buffer.getNumChannels(), buffer.getNumSamples());

    dryBuffer.makeCopyOf(buffer, true);
    wetBuffer.makeCopyOf(buffer, true);
    inputPeak.store(peakOf(buffer), std::memory_order_relaxed);

    if (param("bypass") >= 0.5f)
    {
        outputPeak.store(peakOf(buffer), std::memory_order_relaxed);
        return;
    }

    switch (getPluginKind())
    {
        case PluginKind::Reverb: processReverb(wetBuffer); break;
        case PluginKind::Delay: processDelay(wetBuffer); break;
        case PluginKind::Distortion: processDistortion(wetBuffer); break;
        case PluginKind::Compressor: processCompressor(wetBuffer); break;
        case PluginKind::Limiter: processLimiter(wetBuffer); break;
        case PluginKind::ThreeBandEQ: processThreeBandEQ(wetBuffer); break;
        case PluginKind::FourBandEQ: processFourBandEQ(wetBuffer); break;
        case PluginKind::PitchShifter: processPitchShifter(wetBuffer); break;
        case PluginKind::Chorus: processChorus(wetBuffer); break;
        case PluginKind::Phaser: processPhaser(wetBuffer); break;
        case PluginKind::Flanger: processFlanger(wetBuffer); break;
        case PluginKind::StereoEnhancer: processStereoEnhancer(wetBuffer); break;
        case PluginKind::StereoWidth: processStereoWidth(wetBuffer); break;
        case PluginKind::MidSideProcessor: processMidSideProcessor(wetBuffer); break;
        case PluginKind::AutoPan: processAutoPan(wetBuffer); break;
        case PluginKind::Tremolo: processTremolo(wetBuffer); break;
        case PluginKind::Vibrato: processVibrato(wetBuffer); break;
    }

    mixDryWet(buffer, dryBuffer, param("dryWet") * 0.01f, param("output"));
    outputPeak.store(peakOf(buffer), std::memory_order_relaxed);
}

void PrimitiveFxAudioProcessor::mixDryWet(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>& dry, float dryWet, float outputDb)
{
    const auto wetGain = juce::jlimit(0.0f, 1.0f, dryWet);
    const auto dryGain = 1.0f - wetGain;
    const auto outGain = juce::Decibels::decibelsToGain(outputDb);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* dst = buffer.getWritePointer(ch);
        const auto* d = dry.getReadPointer(ch);
        const auto* w = wetBuffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            dst[i] = (d[i] * dryGain + w[i] * wetGain) * outGain;
    }
}

void PrimitiveFxAudioProcessor::processReverb(juce::AudioBuffer<float>& wet)
{
    juce::dsp::Reverb::Parameters p;
    p.roomSize = param("size") * 0.01f;
    p.damping = param("damping") * 0.01f;
    p.wetLevel = 1.0f;
    p.dryLevel = 0.0f;
    p.width = juce::jlimit(0.0f, 1.0f, param("width") * 0.005f);
    p.freezeMode = 0.0f;
    reverb.setParameters(p);
    juce::dsp::AudioBlock<float> block(wet);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    reverb.process(ctx);

    const float diffusion = 0.35f + param("diffusion") * 0.0065f;
    const float modAmount = param("mod") * 0.00008f;
    for (int ch = 0; ch < wet.getNumChannels(); ++ch)
    {
        auto* x = wet.getWritePointer(ch);
        float z = 0.0f;
        for (int i = 0; i < wet.getNumSamples(); ++i)
        {
            z = smooth(z, x[i], diffusion);
            lfoPhase += modAmount;
            if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
            x[i] = z + 0.04f * std::sin(lfoPhase * juce::MathConstants<float>::twoPi) * x[i];
        }
    }
}

void PrimitiveFxAudioProcessor::processDelay(juce::AudioBuffer<float>& wet)
{
    const int n = wet.getNumSamples();
    const float sr = static_cast<float>(currentSampleRate);
    const float baseDelay = param("time") * sr * 0.001f;
    const float fb = juce::jlimit(0.0f, 0.95f, param("feedback") * 0.01f);
    const float spread = param("spread") * 0.005f;
    const float modRate = param("modRate");
    const float modDepth = param("modDepth") * sr * 0.001f;
    const float tone = 0.05f + param("tone") * 0.009f;

    for (int i = 0; i < n; ++i)
    {
        lfoPhase += modRate / sr;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
        const float mod = std::sin(lfoPhase * juce::MathConstants<float>::twoPi) * modDepth;
        for (int ch = 0; ch < wet.getNumChannels(); ++ch)
        {
            const float chSpread = (ch == 0 ? -spread : spread) * baseDelay;
            const float d = juce::jlimit(1.0f, 191999.0f, baseDelay + chSpread + mod);
            auto delayed = delayLine.popSample(ch, d);
            const float in = wet.getSample(ch, i);
            const float filtered = smooth(ch == 0 ? delayFeedbackL : delayFeedbackR, delayed, tone);
            if (ch == 0) delayFeedbackL = filtered; else delayFeedbackR = filtered;
            delayLine.pushSample(ch, in + filtered * fb);
            wet.setSample(ch, i, delayed);
        }
    }
}

void PrimitiveFxAudioProcessor::processDistortion(juce::AudioBuffer<float>& wet)
{
    const float drive = juce::Decibels::decibelsToGain(param("drive"));
    const float bias = param("bias");
    const float shape = 1.0f + param("shape") * 0.06f;
    const float presence = juce::Decibels::decibelsToGain(param("presence"));
    const float tone = param("tone") * 0.01f;
    const float lowCut = std::exp(-2.0f * pi * param("lowCut") / static_cast<float>(currentSampleRate));

    for (int ch = 0; ch < wet.getNumChannels(); ++ch)
    {
        auto* x = wet.getWritePointer(ch);
        float hp = 0.0f, prev = 0.0f;
        for (int i = 0; i < wet.getNumSamples(); ++i)
        {
            const float driven = x[i] * drive + bias;
            float y = std::tanh(driven * shape) / std::tanh(shape);
            hp = lowCut * (hp + y - prev);
            prev = y;
            y = hp * (0.7f + presence * 0.3f);
            x[i] = smooth(y, std::tanh(y * 1.8f), tone);
        }
    }
}

void PrimitiveFxAudioProcessor::processCompressor(juce::AudioBuffer<float>& wet)
{
    compressor.setThreshold(param("threshold"));
    compressor.setRatio(param("ratio"));
    compressor.setAttack(param("attack"));
    compressor.setRelease(param("release"));
    juce::dsp::AudioBlock<float> block(wet);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    compressor.process(ctx);
    wet.applyGain(juce::Decibels::decibelsToGain(param("makeup")));
}

void PrimitiveFxAudioProcessor::processLimiter(juce::AudioBuffer<float>& wet)
{
    wet.applyGain(juce::Decibels::decibelsToGain(param("input")));
    const float ceiling = juce::Decibels::decibelsToGain(param("ceiling"));
    const float releaseCoeff = std::exp(-1.0f / (0.001f * param("release") * static_cast<float>(currentSampleRate)));
    const float clip = param("softClip") * 0.01f;

    for (int i = 0; i < wet.getNumSamples(); ++i)
    {
        float peak = 0.0f;
        for (int ch = 0; ch < wet.getNumChannels(); ++ch)
            peak = std::max(peak, std::abs(wet.getSample(ch, i)));
        const float target = peak > ceiling ? ceiling / std::max(peak, 0.000001f) : 1.0f;
        limiterEnv = target < limiterEnv ? target : smooth(limiterEnv, target, 1.0f - releaseCoeff);
        for (int ch = 0; ch < wet.getNumChannels(); ++ch)
        {
            float y = wet.getSample(ch, i) * limiterEnv;
            y = y * (1.0f - clip) + std::tanh(y / std::max(ceiling, 0.1f)) * ceiling * clip;
            wet.setSample(ch, i, juce::jlimit(-ceiling, ceiling, y));
        }
    }
}

void PrimitiveFxAudioProcessor::processThreeBandEQ(juce::AudioBuffer<float>& wet)
{
    *eqLow.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, param("lowFreq"), param("lowQ"), juce::Decibels::decibelsToGain(param("lowGain")));
    *eqMidA.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, param("midFreq"), param("midQ"), juce::Decibels::decibelsToGain(param("midGain")));
    *eqHigh.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, 6000.0, param("highQ"), juce::Decibels::decibelsToGain(param("highGain")));
    juce::dsp::AudioBlock<float> block(wet);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    eqLow.process(ctx); eqMidA.process(ctx); eqHigh.process(ctx);
}

void PrimitiveFxAudioProcessor::processFourBandEQ(juce::AudioBuffer<float>& wet)
{
    *eqLow.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, param("lowFreq"), param("lowQ"), juce::Decibels::decibelsToGain(param("lowGain")));
    *eqMidA.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, param("mid1Freq"), param("mid1Q"), juce::Decibels::decibelsToGain(param("mid1Gain")));
    *eqMidB.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, param("mid2Freq"), param("mid2Q"), juce::Decibels::decibelsToGain(param("mid2Gain")));
    *eqHigh.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, param("highFreq"), param("highQ"), juce::Decibels::decibelsToGain(param("highGain")));
    juce::dsp::AudioBlock<float> block(wet);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    eqLow.process(ctx); eqMidA.process(ctx); eqMidB.process(ctx);
}

void PrimitiveFxAudioProcessor::processPitchShifter(juce::AudioBuffer<float>& wet)
{
    const float ratio = std::pow(2.0f, (param("semitones") + param("cents") / 100.0f) / 12.0f);
    const int n = wet.getNumSamples();
    const int grain = juce::jlimit(32, 8192, static_cast<int>(param("grain") * currentSampleRate * 0.001));
    juce::AudioBuffer<float> src;
    src.makeCopyOf(wet, true);

    for (int ch = 0; ch < wet.getNumChannels(); ++ch)
    {
        auto* out = wet.getWritePointer(ch);
        const auto* in = src.getReadPointer(ch);
        for (int i = 0; i < n; ++i)
        {
            const float read = std::fmod(static_cast<float>(i) * ratio, static_cast<float>(std::max(1, n - 1)));
            const int i0 = static_cast<int>(read);
            const int i1 = std::min(i0 + 1, n - 1);
            const float frac = read - static_cast<float>(i0);
            const float shifted = in[i0] + (in[i1] - in[i0]) * frac;
            const float window = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * static_cast<float>(i % grain) / static_cast<float>(grain));
            out[i] = shifted * (0.65f + window * 0.35f);
        }
    }
}

void PrimitiveFxAudioProcessor::processChorus(juce::AudioBuffer<float>& wet)
{
    chorus.setRate(param("rate"));
    chorus.setDepth(param("depth") * 0.01f);
    chorus.setCentreDelay(param("delay"));
    chorus.setFeedback(param("feedback") * 0.01f);
    chorus.setMix(1.0f);
    juce::dsp::AudioBlock<float> block(wet);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    chorus.process(ctx);
}

void PrimitiveFxAudioProcessor::processPhaser(juce::AudioBuffer<float>& wet)
{
    phaser.setRate(param("rate"));
    phaser.setDepth(param("depth") * 0.01f);
    phaser.setCentreFrequency(param("center"));
    phaser.setFeedback(param("feedback") * 0.01f);
    phaser.setMix(1.0f);
    juce::dsp::AudioBlock<float> block(wet);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    phaser.process(ctx);
}

void PrimitiveFxAudioProcessor::processFlanger(juce::AudioBuffer<float>& wet)
{
    const float sr = static_cast<float>(currentSampleRate);
    const float base = param("delay") * sr * 0.001f;
    const float depth = param("depth") * 0.01f * base;
    const float fb = param("feedback") * 0.01f;
    const float spread = param("spread") / 180.0f;
    for (int i = 0; i < wet.getNumSamples(); ++i)
    {
        lfoPhase += param("rate") / sr;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
        for (int ch = 0; ch < wet.getNumChannels(); ++ch)
        {
            const float phase = lfoPhase + (ch == 0 ? 0.0f : spread * 0.5f);
            const float d = juce::jlimit(1.0f, 191999.0f, base + std::sin(phase * juce::MathConstants<float>::twoPi) * depth);
            const float delayed = modulationDelay.popSample(ch, d);
            const float in = wet.getSample(ch, i);
            modulationDelay.pushSample(ch, in + delayed * fb);
            wet.setSample(ch, i, delayed);
        }
    }
}

void PrimitiveFxAudioProcessor::processStereoEnhancer(juce::AudioBuffer<float>& wet)
{
    if (wet.getNumChannels() < 2)
        return;

    const float width = param("width") * 0.01f;
    const float enhance = param("enhance") * 0.01f;
    const float focusCoeff = 0.02f + param("focus") * 0.008f;
    const float bassMono = param("bassMono") * 0.01f;

    auto* left = wet.getWritePointer(0);
    auto* right = wet.getWritePointer(1);
    for (int i = 0; i < wet.getNumSamples(); ++i)
    {
        const float mid = (left[i] + right[i]) * 0.5f;
        float side = (left[i] - right[i]) * 0.5f;
        smoothingStateL = smooth(smoothingStateL, side, focusCoeff);
        side = side * width + (side - smoothingStateL) * enhance;
        side *= (1.0f - bassMono * 0.45f);
        left[i] = mid + side;
        right[i] = mid - side;
    }
}

void PrimitiveFxAudioProcessor::processStereoWidth(juce::AudioBuffer<float>& wet)
{
    if (wet.getNumChannels() < 2)
        return;

    const float width = param("width") * 0.01f;
    const float mono = param("mono") * 0.01f;
    const float balance = param("balance") * 0.01f;
    const float rotation = juce::degreesToRadians(param("rotation"));
    const float leftGain = juce::jlimit(0.0f, 2.0f, 1.0f - std::max(0.0f, balance));
    const float rightGain = juce::jlimit(0.0f, 2.0f, 1.0f + std::min(0.0f, balance));

    auto* left = wet.getWritePointer(0);
    auto* right = wet.getWritePointer(1);
    for (int i = 0; i < wet.getNumSamples(); ++i)
    {
        float mid = (left[i] + right[i]) * 0.5f;
        float side = (left[i] - right[i]) * 0.5f * width * (1.0f - mono);
        const float rotatedMid = mid * std::cos(rotation) - side * std::sin(rotation);
        const float rotatedSide = mid * std::sin(rotation) + side * std::cos(rotation);
        left[i] = (rotatedMid + rotatedSide) * leftGain;
        right[i] = (rotatedMid - rotatedSide) * rightGain;
    }
}

void PrimitiveFxAudioProcessor::processMidSideProcessor(juce::AudioBuffer<float>& wet)
{
    if (wet.getNumChannels() < 2)
        return;

    const float midGain = juce::Decibels::decibelsToGain(param("midGain"));
    const float sideGain = juce::Decibels::decibelsToGain(param("sideGain")) * param("width") * 0.01f;
    const float balance = param("balance") * 0.01f;
    const float leftGain = juce::jlimit(0.0f, 2.0f, 1.0f - std::max(0.0f, balance));
    const float rightGain = juce::jlimit(0.0f, 2.0f, 1.0f + std::min(0.0f, balance));

    auto* left = wet.getWritePointer(0);
    auto* right = wet.getWritePointer(1);
    for (int i = 0; i < wet.getNumSamples(); ++i)
    {
        const float mid = (left[i] + right[i]) * 0.5f * midGain;
        const float side = (left[i] - right[i]) * 0.5f * sideGain;
        left[i] = (mid + side) * leftGain;
        right[i] = (mid - side) * rightGain;
    }
}

void PrimitiveFxAudioProcessor::processAutoPan(juce::AudioBuffer<float>& wet)
{
    const float sr = static_cast<float>(currentSampleRate);
    const float depth = param("depth") * 0.01f;
    const float phaseOffset = param("phase") / 360.0f;
    const float shape = param("shape") * 0.01f;
    const float offset = param("offset") * 0.01f;

    for (int i = 0; i < wet.getNumSamples(); ++i)
    {
        lfoPhase += param("rate") / sr;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        const float sine = std::sin(lfoPhase * juce::MathConstants<float>::twoPi);
        const float shaped = sine * (1.0f - shape) + (sine >= 0.0f ? 1.0f : -1.0f) * shape;
        const float pan = juce::jlimit(-1.0f, 1.0f, offset + shaped * depth);
        const float angle = (pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;

        wet.applyGain(0, i, 1, std::cos(angle));
        if (wet.getNumChannels() > 1)
        {
            const float rightSine = std::sin((lfoPhase + phaseOffset) * juce::MathConstants<float>::twoPi);
            const float rightPan = juce::jlimit(-1.0f, 1.0f, offset - rightSine * depth);
            const float rightAngle = (rightPan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
            wet.applyGain(1, i, 1, std::sin(rightAngle));
        }
    }
}

void PrimitiveFxAudioProcessor::processTremolo(juce::AudioBuffer<float>& wet)
{
    const float sr = static_cast<float>(currentSampleRate);
    const float depth = param("depth") * 0.01f;
    const float shape = param("shape") * 0.01f;
    const float stereoOffset = param("stereo") / 360.0f;
    const float smoothCoeff = 0.04f + param("smooth") * 0.008f;

    for (int i = 0; i < wet.getNumSamples(); ++i)
    {
        lfoPhase += param("rate") / sr;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        for (int ch = 0; ch < wet.getNumChannels(); ++ch)
        {
            const float phase = lfoPhase + (ch == 0 ? 0.0f : stereoOffset);
            const float sine = 0.5f + 0.5f * std::sin(phase * juce::MathConstants<float>::twoPi);
            const float square = sine > 0.5f ? 1.0f : 0.0f;
            const float lfo = sine * (1.0f - shape) + square * shape;
            const float target = 1.0f - depth * lfo;
            if (ch == 0) smoothingStateL = smooth(smoothingStateL, target, smoothCoeff);
            else smoothingStateR = smooth(smoothingStateR, target, smoothCoeff);
            wet.applyGain(ch, i, 1, ch == 0 ? smoothingStateL : smoothingStateR);
        }
    }
}

void PrimitiveFxAudioProcessor::processVibrato(juce::AudioBuffer<float>& wet)
{
    const float sr = static_cast<float>(currentSampleRate);
    const float baseDelay = param("delay") * sr * 0.001f;
    const float depth = param("depth") * sr * 0.00008f;
    const float shape = param("shape") * 0.01f;
    const float stereoOffset = param("stereo") / 360.0f;

    for (int i = 0; i < wet.getNumSamples(); ++i)
    {
        lfoPhase += param("rate") / sr;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        for (int ch = 0; ch < wet.getNumChannels(); ++ch)
        {
            const float phase = lfoPhase + (ch == 0 ? 0.0f : stereoOffset);
            const float sine = std::sin(phase * juce::MathConstants<float>::twoPi);
            const float triangle = 2.0f * std::abs(2.0f * (phase - std::floor(phase + 0.5f))) - 1.0f;
            const float lfo = sine * (1.0f - shape) + triangle * shape;
            const float delaySamples = juce::jlimit(1.0f, 191999.0f, baseDelay + depth * lfo);
            const float in = wet.getSample(ch, i);
            const float delayed = modulationDelay.popSample(ch, delaySamples);
            modulationDelay.pushSample(ch, in);
            wet.setSample(ch, i, delayed);
        }
    }
}

void PrimitiveFxAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void PrimitiveFxAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

void PrimitiveFxAudioProcessor::setParameterFromWeb(const juce::String& id, float value)
{
    if (auto* p = apvts.getParameter(id))
    {
        const auto normalised = p->convertTo0to1(value);
        p->beginChangeGesture();
        p->setValueNotifyingHost(normalised);
        p->endChangeGesture();
    }
}

juce::var PrimitiveFxAudioProcessor::makeParameterStateForWeb() const
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    juce::Array<juce::var> items;
    for (const auto& spec : getParameterSpecs())
    {
        juce::DynamicObject::Ptr item = new juce::DynamicObject();
        item->setProperty("id", spec.id);
        item->setProperty("name", spec.name);
        item->setProperty("min", spec.minValue);
        item->setProperty("max", spec.maxValue);
        item->setProperty("value", param(spec.id));
        item->setProperty("step", spec.step);
        item->setProperty("unit", spec.unit);
        items.add(juce::var(item.get()));
    }
    root->setProperty("params", items);
    root->setProperty("bypass", param("bypass") >= 0.5f);
    return juce::var(root.get());
}
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new pbpfl::PrimitiveFxAudioProcessor();
}
