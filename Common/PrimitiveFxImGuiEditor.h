#pragma once

#include "PrimitiveFxProcessor.h"
#include "imgui.h"
#include <array>
#include <string>
#include <vector>
#include <windows.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace pbpfl
{
class PrimitiveFxImGuiEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PrimitiveFxImGuiEditor(PrimitiveFxAudioProcessor&);
    ~PrimitiveFxImGuiEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    static LRESULT CALLBACK childWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void timerCallback() override;
    void initialiseImGuiHost();
    void shutdownImGuiHost();
    void renderImGuiFrame();
    void renderFlangerPanel();
    void syncFromParameters();
    void pushToParameter(const char* id, float value);
    float parameterValue(const char* id) const;
    int noteIndexFromRate(float rate) const;
    void renderHeaderControls(ImDrawList* drawList, float centerY, float centerW);
    bool headerButton(ImDrawList* drawList, const char* id, ImVec2 min, ImVec2 max, const char* label, bool active = false, float fontSize = 12.0f);
    bool headerIconButton(ImDrawList* drawList, const char* id, ImVec2 min, ImVec2 max, const char* icon, bool active = false);
    std::vector<float> captureSnapshot() const;
    void applySnapshot(const std::vector<float>& values);
    void pushHistory();
    void applyPreset(int index);
    std::vector<float> factoryPresetValues(int index) const;
    void initialiseHeaderState();
    void bindTemplateGlobals();
    void captureTemplateGlobals();
    void loadHeaderSvgIconTextures();
    void releaseHeaderSvgIconTextures();
    bool drawSvgHeaderIcon(ImDrawList* drawList, ImVec2 min, ImVec2 max, const char* icon, bool active);

    PrimitiveFxAudioProcessor& processor;
    void* childWindow {};
    HINSTANCE childWindowInstance {};
    std::wstring childWindowClassName;
    bool imguiReady {};
    ImGuiContext* imguiContext {};
    ID3D11Device* d3dDevice {};
    ID3D11DeviceContext* d3dDeviceContext {};
    IDXGISwapChain* swapChain {};
    bool swapChainOccluded {};
    unsigned int resizeWidth {};
    unsigned int resizeHeight {};
    ID3D11RenderTargetView* mainRenderTargetView {};
    std::array<ID3D11Texture2D*, 32> knobTexture2D {};
    std::array<ID3D11ShaderResourceView*, 32> knobTexture {};
    std::array<ID3D11Texture2D*, 4> headerIconTexture2D {};
    std::array<ID3D11ShaderResourceView*, 4> headerIconTexture {};
    bool texturedKnobInputBlocked {};
    ImFont* headerTitleFont {};
    ImFont* headerMetaFont {};
    bool rateTempoSync {};
    int rateSyncIndex { 7 };
    bool lfoChipOpen {};
    bool lfoChipVisible {};
    float lfoType {};
    float lfoRate { 1.0f };
    float lfoDepth { 0.44f };
    bool lfoRateTempoSync {};
    int lfoRateSyncIndex { 7 };
    bool headerStateReady {};
    int activeAB {};
    std::array<std::vector<float>, 2> abSnapshots {};
    int currentPresetIndex {};
    std::vector<std::vector<float>> userPresets;
    std::vector<std::vector<float>> undoStack;
    std::vector<std::vector<float>> redoStack;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PrimitiveFxImGuiEditor)
};
}
