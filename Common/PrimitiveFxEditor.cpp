#include "PrimitiveFxEditor.h"
#include "BinaryData.h"

namespace pbpfl
{
namespace
{
juce::Rectangle<int> getEditorBoundsForParameterCount(int parameterCount)
{
    constexpr int minWidth = 680;
    constexpr int sidePadding = 40;
    constexpr int knobWidth = 98;
    constexpr int editorHeight = 235;

    return { std::max(minWidth, sidePadding + parameterCount * knobWidth), editorHeight };
}
}

PrimitiveFxAudioProcessorEditor::PrimitiveFxAudioProcessorEditor(PrimitiveFxAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    const auto bounds = getEditorBoundsForParameterCount(static_cast<int>(getParameterSpecs().size()));
    setResizable(false, false);
    setSize(bounds.getWidth(), bounds.getHeight());
    fallback.setJustificationType(juce::Justification::centred);
    fallback.setColour(juce::Label::textColourId, juce::Colours::white);
    fallback.setText(getPluginName() + "\nWebView is not available.", juce::dontSendNotification);
    addChildComponent(fallback);
    initialiseWebView();
    startTimerHz(30);
}

PrimitiveFxAudioProcessorEditor::~PrimitiveFxAudioProcessorEditor()
{
    stopTimer();
    webView = nullptr;
}

void PrimitiveFxAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff101820));
}

void PrimitiveFxAudioProcessorEditor::resized()
{
    if (webView) webView->setBounds(getLocalBounds());
    fallback.setBounds(getLocalBounds());
}

juce::File PrimitiveFxAudioProcessorEditor::extractWebUiFiles()
{
    auto safeName = getPluginName().replaceCharacter(' ', '_');
    auto userDataDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                           .getChildFile(safeName + "_WebView2_v" + juce::String(JucePlugin_VersionString) + "_ui2");
    userDataDir.createDirectory();

    webUiDir = userDataDir.getChildFile("ui");
    webUiDir.createDirectory();

    const auto html = juce::String::fromUTF8(BinaryData::index_html, BinaryData::index_htmlSize);
    const auto css = juce::String::fromUTF8(BinaryData::style_css, BinaryData::style_cssSize);
    const auto js = juce::String::fromUTF8(BinaryData::script_js, BinaryData::script_jsSize);

    auto bundledHtml = html
        .replace("<link rel=\"stylesheet\" href=\"style.css\">", "<style>\n" + css + "\n</style>")
        .replace("<script src=\"script.js\"></script>", "<script>\n" + js + "\n</script>");

    webUiDir.getChildFile("index.html").replaceWithText(bundledHtml, false, false, "\n");
    webUiDir.getChildFile("style.css").replaceWithText(css, false, false, "\n");
    webUiDir.getChildFile("script.js").replaceWithText(js, false, false, "\n");

    return userDataDir;
}

void PrimitiveFxAudioProcessorEditor::initialiseWebView()
{
    try
    {
        const auto userDataDir = extractWebUiFiles();

        using Options = juce::WebBrowserComponent::Options;
        auto opts = Options()
            .withBackend(Options::Backend::webview2)
            .withKeepPageLoadedWhenBrowserIsHidden()
            .withNativeIntegrationEnabled()
            .withWinWebView2Options(Options::WinWebView2{}
                .withUserDataFolder(userDataDir)
                .withBackgroundColour(juce::Colour(0xff17223a))
                .withStatusBarDisabled()
                .withBuiltInErrorPageDisabled())
            .withEventListener("uiEvent", [this](const juce::var& payload)
            {
                juce::MessageManager::callAsync([this, payload] { handleUiEvent(payload); });
            });

        webView = std::make_unique<juce::WebBrowserComponent>(opts);
        addAndMakeVisible(*webView);
        resized();
        auto indexPath = webUiDir.getChildFile("index.html").getFullPathName().replace("\\", "/");
        webView->goToURL("file:///" + indexPath);
        fallback.setVisible(false);
    }
    catch (...)
    {
        webView.reset();
        fallback.setVisible(true);
    }
}

void PrimitiveFxAudioProcessorEditor::handleUiEvent(const juce::var& payload)
{
    const auto event = payload.getProperty("event", {}).toString();
    if (event == "uiReady")
    {
        webReady = true;
        sendStateToWeb();
        return;
    }
    if (event == "paramChange")
        processor.setParameterFromWeb(payload.getProperty("id", {}).toString(), static_cast<float>(payload.getProperty("value", 0.0)));
    if (event == "bypass")
        processor.setParameterFromWeb("bypass", static_cast<bool>(payload.getProperty("value", false)) ? 1.0f : 0.0f);
    if (event == "savePreset")
    {
        processor.updateHostDisplay();
        return;
    }
    if (event == "help")
    {
        processor.updateHostDisplay();
        return;
    }
    if (event == "openUrl")
    {
        juce::URL(payload.getProperty("url", {}).toString()).launchInDefaultBrowser();
        return;
    }
    if (event == "requestState")
        sendStateToWeb();
}

void PrimitiveFxAudioProcessorEditor::sendStateToWeb()
{
    if (!webView || !webReady) return;
    const auto json = juce::JSON::toString(processor.makeParameterStateForWeb());
    webView->evaluateJavascript("window.handleHostMessage && window.handleHostMessage({type:'state', payload:" + json + "});");
}

void PrimitiveFxAudioProcessorEditor::sendMetersToWeb()
{
    if (!webView || !webReady) return;
    juce::DynamicObject::Ptr meter = new juce::DynamicObject();
    meter->setProperty("input", processor.inputPeak.load(std::memory_order_relaxed));
    meter->setProperty("output", processor.outputPeak.load(std::memory_order_relaxed));
    meter->setProperty("bpm", processor.hostBpm.load(std::memory_order_relaxed));
    webView->evaluateJavascript("window.handleHostMessage && window.handleHostMessage({type:'meters', payload:" + juce::JSON::toString(juce::var(meter.get())) + "});");
}

void PrimitiveFxAudioProcessorEditor::timerCallback()
{
    sendMetersToWeb();
}
}

#ifndef PB_USE_IMGUI_EDITOR
juce::AudioProcessorEditor* pbpfl::PrimitiveFxAudioProcessor::createEditor()
{
    return new PrimitiveFxAudioProcessorEditor(*this);
}
#endif
