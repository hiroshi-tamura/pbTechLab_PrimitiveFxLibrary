#pragma once

#include "PrimitiveFxProcessor.h"

namespace pbpfl
{
class PrimitiveFxAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PrimitiveFxAudioProcessorEditor(PrimitiveFxAudioProcessor&);
    ~PrimitiveFxAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void initialiseWebView();
    void sendStateToWeb();
    void sendMetersToWeb();
    void handleUiEvent(const juce::var& payload);
    juce::File extractWebUiFiles();

    PrimitiveFxAudioProcessor& processor;
    std::unique_ptr<juce::WebBrowserComponent> webView;
    juce::Label fallback;
    juce::File webUiDir;
    bool webReady { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PrimitiveFxAudioProcessorEditor)
};
}
