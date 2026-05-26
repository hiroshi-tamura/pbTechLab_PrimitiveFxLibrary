#include "PrimitiveFxImGuiEditor.h"
#include "PbPFLImGuiHeaderIconsData.h"

#define IMGUI_LFO_CHIP_TEXTURE_SLOT_BASE 20
#define wWinMain PbPFL_Disabled_ImGuiTemplate_wWinMain
#include PB_PLUGIN_GUI_IMGUI_MAIN
#undef wWinMain
#undef IMGUI_LFO_CHIP_TEXTURE_SLOT_BASE

#include <cwchar>

namespace pbpfl
{
namespace
{
constexpr int kMinEditorWidth = 740;
constexpr int kEditorHeight = 214;
constexpr float kMainKnobWidth = 88.0f;
constexpr float kMainKnobGap = 4.0f;
constexpr float kMainSideMargin = 8.0f;
constexpr float kHeaderControlsWidth = 427.0f;
constexpr float kHeaderLeftReserved = 236.0f;
constexpr float kHeaderRightReserved = 74.0f;
constexpr int kHeaderIconTextureSize = 64;
constexpr UINT_PTR renderTimerId = 4001;
constexpr int kFactoryPresetCount = 10;

enum HeaderSvgIcon
{
    kHeaderSvgBypassOn = 0,
    kHeaderSvgBypassOff,
    kHeaderSvgUndo,
    kHeaderSvgRedo,
    kHeaderSvgCount
};

HWND toHwnd(void* p)
{
    return static_cast<HWND>(p);
}

const char* headerIconName(HeaderSvgIcon icon)
{
    switch (icon)
    {
        case kHeaderSvgBypassOn: return "bypass_on_64_png";
        case kHeaderSvgBypassOff: return "bypass_off_64_png";
        case kHeaderSvgUndo: return "undo_64_png";
        case kHeaderSvgRedo: return "redo_64_png";
        default: break;
    }
    return "";
}

int editorWidthForCurrentPlugin()
{
    const int knobCount = static_cast<int>(getParameterSpecs().size());
    const float knobsWidth = kMainKnobWidth * static_cast<float>(knobCount)
        + kMainKnobGap * static_cast<float>(std::max(0, knobCount - 1))
        + kMainSideMargin * 2.0f;
    const float headerWidth = kHeaderLeftReserved + kHeaderControlsWidth + kHeaderRightReserved;
    return std::max(kMinEditorWidth, static_cast<int>(std::ceil(std::max(knobsWidth, headerWidth))));
}

HINSTANCE moduleHandleFromThisDll()
{
    HMODULE module = nullptr;
    ::GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&moduleHandleFromThisDll),
        &module);
    return reinterpret_cast<HINSTANCE>(module);
}

float syncLabelToHz(int index, float bpm)
{
    static constexpr float beats[] = {
        6.0f, 4.0f, 8.0f / 3.0f,
        3.0f, 2.0f, 4.0f / 3.0f,
        1.5f, 1.0f, 2.0f / 3.0f,
        0.75f, 0.5f, 1.0f / 3.0f,
        0.375f, 0.25f, 1.0f / 6.0f,
        0.1875f, 0.125f, 1.0f / 12.0f,
        0.09375f, 0.0625f, 1.0f / 24.0f
    };
    const auto safeBpm = juce::jlimit(20.0f, 300.0f, bpm);
    const auto beatSeconds = 60.0f / safeBpm;
    return juce::jlimit(0.03f, 8.0f, 1.0f / (beats[juce::jlimit(0, kSyncNoteCount - 1, index)] * beatSeconds));
}

const char* factoryPresetName(int index)
{
    const int safeIndex = juce::jlimit(0, kFactoryPresetCount - 1, index);
    switch (getPluginKind())
    {
        case PluginKind::Reverb:
        {
            static constexpr const char* names[] = { "Reverb Init", "Small Room", "Bright Room", "Wide Hall", "Dark Hall", "Huge Space", "Plate Shine", "Short Ambience", "Dream Wash", "Tight Verb" };
            return names[safeIndex];
        }
        case PluginKind::Delay:
        {
            static constexpr const char* names[] = { "Delay Init", "Slap Echo", "Tape Echo", "Stereo Echo", "Dub Throw", "Short Room", "Ping Delay", "Long Trail", "Filtered Repeat", "Tight Double" };
            return names[safeIndex];
        }
        case PluginKind::Distortion:
        {
            static constexpr const char* names[] = { "Drive Init", "Warm Drive", "Edge Bite", "Heavy Clip", "Fuzz Push", "Low Grind", "Bright Crunch", "Parallel Heat", "Soft Saturate", "Hard Shred" };
            return names[safeIndex];
        }
        case PluginKind::Compressor:
        {
            static constexpr const char* names[] = { "Comp Init", "Gentle Level", "Vocal Control", "Punch Clamp", "Drum Glue", "Bass Hold", "Fast Catch", "Slow Smooth", "Parallel Smash", "Clean Trim" };
            return names[safeIndex];
        }
        case PluginKind::Limiter:
        {
            static constexpr const char* names[] = { "Limiter Init", "Clean Ceiling", "Soft Catch", "Hot Limit", "Mix Guard", "Loud Push", "Peak Tamer", "Smooth Wall", "Clip Safety", "Final Trim" };
            return names[safeIndex];
        }
        case PluginKind::ThreeBandEQ:
        {
            static constexpr const char* names[] = { "3Band Init", "Low Lift", "Mid Focus", "Air Boost", "Warm Scoop", "Phone Tone", "Bass Cut", "Presence Lift", "Soft Smile", "Tight Mix" };
            return names[safeIndex];
        }
        case PluginKind::FourBandEQ:
        {
            static constexpr const char* names[] = { "4Band Init", "Clean Shape", "Presence", "Wide Polish", "Low Control", "Vocal Shine", "Guitar Edge", "Drum Tone", "Sub Tidy", "Mix Sweet" };
            return names[safeIndex];
        }
        case PluginKind::PitchShifter:
        {
            static constexpr const char* names[] = { "Pitch Init", "Octave Down", "Octave Up", "Fifth Up", "Thick Shift", "Robot Low", "Fine Detune", "Wide Double", "Chip Voice", "Dark Layer" };
            return names[safeIndex];
        }
        case PluginKind::Chorus:
        {
            static constexpr const char* names[] = { "Chorus Init", "Soft Chorus", "Wide Chorus", "Deep Swim", "Slow Ensemble", "Bright Wash", "Bass Chorus", "Vintage Mod", "Shimmer Pad", "Tight Double" };
            return names[safeIndex];
        }
        case PluginKind::Phaser:
        {
            static constexpr const char* names[] = { "Phaser Init", "Slow Sweep", "Vocal Notch", "Deep Phase", "Fast Swirl", "Vintage Phase", "Wide Notch", "Dark Sweep", "Light Motion", "Reso Spin" };
            return names[safeIndex];
        }
        case PluginKind::Flanger:
        {
            static constexpr const char* names[] = { "Flanger Init", "Subtle Jet", "Wide Sweep", "Deep Flange", "Metallic Spin", "Zero Pass", "Slow Comb", "Bright Jet", "Dark Tunnel", "Stereo Blade" };
            return names[safeIndex];
        }
        case PluginKind::StereoEnhancer:
        {
            static constexpr const char* names[] = { "Enhancer Init", "Soft Enhance", "Focused Wide", "Shimmer Wide", "Vocal Space", "Bass Safe", "Air Extend", "Narrow Focus", "Mix Open", "Edge Wide" };
            return names[safeIndex];
        }
        case PluginKind::StereoWidth:
        {
            static constexpr const char* names[] = { "Width Init", "Narrow Check", "Wide Image", "Rotated Space", "Mono Utility", "Center Lock", "Side Push", "Sub Mono", "Mix Spread", "Balance Fix" };
            return names[safeIndex];
        }
        case PluginKind::MidSideProcessor:
        {
            static constexpr const char* names[] = { "M/S Init", "Mid Forward", "Side Lift", "Wide Balance", "Mono Check", "Vocal Center", "Side Trim", "Bass Center", "Open Sides", "Tight M/S" };
            return names[safeIndex];
        }
        case PluginKind::AutoPan:
        {
            static constexpr const char* names[] = { "Pan Init", "Slow Drift", "Wide Motion", "Fast Chop", "Soft Swing", "Hard Alternate", "Quarter Pulse", "Subtle Move", "Stereo Circle", "Trem Pan" };
            return names[safeIndex];
        }
        case PluginKind::Tremolo:
        {
            static constexpr const char* names[] = { "Trem Init", "Soft Pulse", "Stereo Pulse", "Hard Gate", "Slow Fade", "Fast Trem", "Vintage Amp", "Deep Chop", "Gentle Waver", "Square Cut" };
            return names[safeIndex];
        }
        case PluginKind::Vibrato:
        {
            static constexpr const char* names[] = { "Vibrato Init", "Light Wobble", "Tape Drift", "Deep Warble", "Fast Shake", "Slow Flutter", "Voice Bend", "Stereo Swim", "Subtle Pitch", "Extreme Dive" };
            return names[safeIndex];
        }
    }
    return index == 0 ? "Init" : "Preset";
}

void drawHostText(ImDrawList* drawList, ImVec2 pos, const char* text, float size = 13.0f, ImU32 color = IM_COL32(132, 145, 154, 255))
{
    drawList->AddText(ImGui::GetFont(), size, pos, color, text);
}

void drawHeaderText(ImDrawList* drawList, ImFont* font, ImVec2 pos, const char* text, float size, ImU32 color)
{
    if (font == nullptr)
        font = ImGui::GetFont();
    pos.x = std::round(pos.x);
    pos.y = std::round(pos.y);
    drawList->AddText(font, size, pos, color, text);
}

void drawMockButton(ImDrawList* drawList, ImVec2 min, ImVec2 max, const char* label, bool active = false, float fontSize = 12.0f)
{
    const ImU32 bg = active ? IM_COL32(66, 150, 250, 255) : IM_COL32(30, 30, 30, 255);
    const ImU32 border = active ? IM_COL32(66, 150, 250, 255) : IM_COL32(45, 60, 94, 255);
    const ImU32 fg = active ? IM_COL32(255, 255, 255, 255) : IM_COL32(154, 166, 192, 255);
    drawList->AddRectFilled(min, max, bg, 4.0f);
    drawList->AddRect(min, max, border, 4.0f);
    const ImVec2 size = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label);
    drawHostText(drawList, ImVec2((min.x + max.x - size.x) * 0.5f, (min.y + max.y - size.y) * 0.5f - 1.0f), label, fontSize, fg);
}

void drawMockIconButton(ImDrawList* drawList, ImVec2 min, ImVec2 max, const char* label, bool active = false)
{
    drawMockButton(drawList, min, max, label, active, 14.0f);
}

void drawBrandLogo(ImDrawList* drawList, ImVec2 pos)
{
    const ImVec2 min = pos;
    const ImVec2 max(pos.x + 34.0f, pos.y + 34.0f);
    drawList->AddRectFilled(min, max, IM_COL32(24, 39, 68, 255), 5.0f);
    drawList->AddRect(min, max, IM_COL32(45, 60, 94, 255), 5.0f, 0, 1.0f);
    drawList->AddRectFilled(ImVec2(pos.x + 6.0f, pos.y + 6.0f), ImVec2(pos.x + 28.0f, pos.y + 28.0f), IM_COL32(18, 28, 48, 255), 3.0f);
    ImVector<ImVec2> p;
    p.push_back(ImVec2(pos.x + 11.0f, pos.y + 24.0f));
    p.push_back(ImVec2(pos.x + 11.0f, pos.y + 10.0f));
    p.push_back(ImVec2(pos.x + 21.0f, pos.y + 10.0f));
    p.push_back(ImVec2(pos.x + 25.0f, pos.y + 13.0f));
    p.push_back(ImVec2(pos.x + 25.0f, pos.y + 18.0f));
    p.push_back(ImVec2(pos.x + 21.0f, pos.y + 21.0f));
    p.push_back(ImVec2(pos.x + 16.0f, pos.y + 21.0f));
    p.push_back(ImVec2(pos.x + 16.0f, pos.y + 24.0f));
    drawList->AddConvexPolyFilled(p.Data, p.Size, IM_COL32(45, 212, 168, 240));
    drawList->AddRectFilled(ImVec2(pos.x + 16.0f, pos.y + 14.0f), ImVec2(pos.x + 21.0f, pos.y + 17.0f), IM_COL32(66, 150, 250, 230), 1.0f);
}
}

PrimitiveFxImGuiEditor::PrimitiveFxImGuiEditor(PrimitiveFxAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setOpaque(true);
    setResizable(false, false);
    setSize(editorWidthForCurrentPlugin(), kEditorHeight);
    initialiseImGuiHost();
    startTimerHz(30);
}

PrimitiveFxImGuiEditor::~PrimitiveFxImGuiEditor()
{
    stopTimer();
    shutdownImGuiHost();
}

void PrimitiveFxImGuiEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121922));
}

void PrimitiveFxImGuiEditor::resized()
{
    if (auto hwnd = toHwnd(childWindow))
        ::SetWindowPos(hwnd, nullptr, 0, 0, getWidth(), getHeight(), SWP_NOZORDER | SWP_NOACTIVATE);
    resizeWidth = static_cast<UINT>(getWidth());
    resizeHeight = static_cast<UINT>(getHeight());
}

void PrimitiveFxImGuiEditor::timerCallback()
{
    if (!imguiReady)
        initialiseImGuiHost();
    renderImGuiFrame();
}

void PrimitiveFxImGuiEditor::initialiseImGuiHost()
{
    auto* peer = getPeer();
    if (peer == nullptr)
    {
        juce::MessageManager::callAsync([safe = juce::Component::SafePointer<PrimitiveFxImGuiEditor>(this)]
        {
            if (safe != nullptr)
                safe->initialiseImGuiHost();
        });
        return;
    }

    auto parent = static_cast<HWND>(peer->getNativeHandle());
    if (parent == nullptr || childWindow != nullptr)
        return;

    childWindowInstance = moduleHandleFromThisDll();
    wchar_t className[96] {};
    std::swprintf(className, 96, L"PbPFLImGuiChild_%p", this);
    childWindowClassName = className;
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, PrimitiveFxImGuiEditor::childWndProc, 0L, 0L, childWindowInstance, nullptr, nullptr, nullptr, nullptr, childWindowClassName.c_str(), nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowExW(0, wc.lpszClassName, L"pbPFL ImGui", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                 0, 0, getWidth(), getHeight(), parent, nullptr, childWindowInstance, this);
    childWindow = hwnd;

    bindTemplateGlobals();
    if (hwnd == nullptr || !CreateDeviceD3D(hwnd))
    {
        captureTemplateGlobals();
        return;
    }
    captureTemplateGlobals();

    IMGUI_CHECKVERSION();
    imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    ImFontConfig titleConfig;
    titleConfig.OversampleH = 3;
    titleConfig.OversampleV = 3;
    titleConfig.PixelSnapH = false;
    ImFontConfig metaConfig = titleConfig;
    headerTitleFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeuib.ttf", 17.0f, &titleConfig);
    headerMetaFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 10.5f, &metaConfig);

    ApplyAudioPluginStyle();
    bindTemplateGlobals();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    for (int i = 0; i < kKnobTextureSlotCount; ++i)
        CreateProceduralKnobTexture(i, true);
    captureTemplateGlobals();
    loadHeaderSvgIconTextures();

    imguiReady = true;
    resized();
}

void PrimitiveFxImGuiEditor::shutdownImGuiHost()
{
    if (imguiReady)
    {
        bindTemplateGlobals();
        ImGui::SetCurrentContext(imguiContext);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        captureTemplateGlobals();
        ImGui::DestroyContext(imguiContext);
        imguiContext = nullptr;
        imguiReady = false;
    }

    bindTemplateGlobals();
    releaseHeaderSvgIconTextures();
    for (int i = 0; i < kKnobTextureSlotCount; ++i)
    {
        if (g_knobTexture[i]) { g_knobTexture[i]->Release(); g_knobTexture[i] = nullptr; }
        if (g_knobTexture2D[i]) { g_knobTexture2D[i]->Release(); g_knobTexture2D[i] = nullptr; }
    }
    CleanupDeviceD3D();
    captureTemplateGlobals();

    if (auto hwnd = toHwnd(childWindow))
    {
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        ::DestroyWindow(hwnd);
        childWindow = nullptr;
    }
    if (!childWindowClassName.empty() && childWindowInstance != nullptr)
    {
        ::UnregisterClassW(childWindowClassName.c_str(), childWindowInstance);
        childWindowClassName.clear();
        childWindowInstance = nullptr;
    }
}

void PrimitiveFxImGuiEditor::renderImGuiFrame()
{
    if (!imguiReady || swapChain == nullptr || imguiContext == nullptr)
        return;

    bindTemplateGlobals();
    ImGui::SetCurrentContext(imguiContext);

    if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
    {
        captureTemplateGlobals();
        return;
    }
    g_SwapChainOccluded = false;

    if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
    {
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }

    syncFromParameters();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    renderFlangerPanel();
    ImGui::Render();

    const float clearColor[4] = { 0.075f, 0.082f, 0.090f, 1.0f };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    const HRESULT hr = g_pSwapChain->Present(1, 0);
    g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    captureTemplateGlobals();
}

void PrimitiveFxImGuiEditor::bindTemplateGlobals()
{
    g_pd3dDevice = d3dDevice;
    g_pd3dDeviceContext = d3dDeviceContext;
    g_pSwapChain = swapChain;
    g_SwapChainOccluded = swapChainOccluded;
    g_ResizeWidth = resizeWidth;
    g_ResizeHeight = resizeHeight;
    g_mainRenderTargetView = mainRenderTargetView;
    for (int i = 0; i < kKnobTextureSlotCount; ++i)
    {
        g_knobTexture2D[i] = knobTexture2D[static_cast<size_t>(i)];
        g_knobTexture[i] = knobTexture[static_cast<size_t>(i)];
    }
    g_texturedKnobInputBlocked = texturedKnobInputBlocked;
}

void PrimitiveFxImGuiEditor::captureTemplateGlobals()
{
    d3dDevice = g_pd3dDevice;
    d3dDeviceContext = g_pd3dDeviceContext;
    swapChain = g_pSwapChain;
    swapChainOccluded = g_SwapChainOccluded;
    resizeWidth = g_ResizeWidth;
    resizeHeight = g_ResizeHeight;
    mainRenderTargetView = g_mainRenderTargetView;
    for (int i = 0; i < kKnobTextureSlotCount; ++i)
    {
        knobTexture2D[static_cast<size_t>(i)] = g_knobTexture2D[i];
        knobTexture[static_cast<size_t>(i)] = g_knobTexture[i];
    }
    texturedKnobInputBlocked = g_texturedKnobInputBlocked;
}

void PrimitiveFxImGuiEditor::releaseHeaderSvgIconTextures()
{
    for (auto*& texture : headerIconTexture)
    {
        if (texture != nullptr)
        {
            texture->Release();
            texture = nullptr;
        }
    }
    for (auto*& texture : headerIconTexture2D)
    {
        if (texture != nullptr)
        {
            texture->Release();
            texture = nullptr;
        }
    }
}

void PrimitiveFxImGuiEditor::loadHeaderSvgIconTextures()
{
    if (d3dDevice == nullptr)
        return;

    releaseHeaderSvgIconTextures();

    for (int icon = 0; icon < kHeaderSvgCount; ++icon)
    {
        int imageDataSize = 0;
        const auto* imageData = PbPFLImGuiHeaderIcons::getNamedResource(headerIconName(static_cast<HeaderSvgIcon>(icon)), imageDataSize);
        if (imageData == nullptr || imageDataSize <= 0)
            continue;

        auto image = juce::ImageFileFormat::loadFrom(imageData, static_cast<size_t>(imageDataSize));
        if (!image.isValid())
            continue;

        std::vector<unsigned char> rgba(static_cast<size_t>(kHeaderIconTextureSize * kHeaderIconTextureSize * 4));
        for (int y = 0; y < kHeaderIconTextureSize; ++y)
        {
            for (int x = 0; x < kHeaderIconTextureSize; ++x)
            {
                const auto color = image.getPixelAt(x, y);
                const size_t offset = static_cast<size_t>((y * kHeaderIconTextureSize + x) * 4);
                rgba[offset + 0] = color.getRed();
                rgba[offset + 1] = color.getGreen();
                rgba[offset + 2] = color.getBlue();
                rgba[offset + 3] = color.getAlpha();
            }
        }

        D3D11_TEXTURE2D_DESC textureDesc {};
        textureDesc.Width = kHeaderIconTextureSize;
        textureDesc.Height = kHeaderIconTextureSize;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA subResource {};
        subResource.pSysMem = rgba.data();
        subResource.SysMemPitch = kHeaderIconTextureSize * 4;

        ID3D11Texture2D* texture2D = nullptr;
        if (FAILED(d3dDevice->CreateTexture2D(&textureDesc, &subResource, &texture2D)))
            continue;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        ID3D11ShaderResourceView* srv = nullptr;
        if (FAILED(d3dDevice->CreateShaderResourceView(texture2D, &srvDesc, &srv)))
        {
            texture2D->Release();
            continue;
        }

        headerIconTexture2D[static_cast<size_t>(icon)] = texture2D;
        headerIconTexture[static_cast<size_t>(icon)] = srv;
    }
}

bool PrimitiveFxImGuiEditor::drawSvgHeaderIcon(ImDrawList* drawList, ImVec2 min, ImVec2 max, const char* icon, bool active)
{
    HeaderSvgIcon svgIcon = kHeaderSvgCount;
    if (std::strcmp(icon, "undo") == 0)
        svgIcon = kHeaderSvgUndo;
    else if (std::strcmp(icon, "redo") == 0)
        svgIcon = kHeaderSvgRedo;
    else if (std::strcmp(icon, "power") == 0)
        svgIcon = active ? kHeaderSvgBypassOn : kHeaderSvgBypassOff;
    else
        return false;

    auto* texture = headerIconTexture[static_cast<size_t>(svgIcon)];
    if (texture == nullptr)
        return false;

    const float buttonW = max.x - min.x;
    const float buttonH = max.y - min.y;
    const float iconSize = std::min(buttonW, buttonH) - 7.0f;
    const ImVec2 iconMin(min.x + (buttonW - iconSize) * 0.5f, min.y + (buttonH - iconSize) * 0.5f);
    const ImVec2 iconMax(iconMin.x + iconSize, iconMin.y + iconSize);
    drawList->AddImage(reinterpret_cast<ImTextureID>(texture), iconMin, iconMax);
    return true;
}

LRESULT CALLBACK PrimitiveFxImGuiEditor::childWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PrimitiveFxImGuiEditor* editor = reinterpret_cast<PrimitiveFxImGuiEditor*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE)
    {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        editor = static_cast<PrimitiveFxImGuiEditor*>(create != nullptr ? create->lpCreateParams : nullptr);
        if (editor != nullptr)
        {
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(editor));
            editor->childWindow = hWnd;
        }
    }

    if (editor != nullptr)
    {
        if (editor->imguiContext != nullptr)
            ImGui::SetCurrentContext(editor->imguiContext);
        editor->bindTemplateGlobals();

        if (editor->imguiContext != nullptr && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        {
            editor->captureTemplateGlobals();
            return true;
        }

        switch (msg)
        {
            case WM_SIZE:
                if (wParam == SIZE_MINIMIZED)
                {
                    editor->captureTemplateGlobals();
                    return 0;
                }
                g_ResizeWidth = static_cast<UINT>(LOWORD(lParam));
                g_ResizeHeight = static_cast<UINT>(HIWORD(lParam));
                editor->captureTemplateGlobals();
                return 0;
            case WM_SYSCOMMAND:
                if ((wParam & 0xfff0) == SC_KEYMENU)
                {
                    editor->captureTemplateGlobals();
                    return 0;
                }
                break;
            case WM_NCDESTROY:
                ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
                break;
            default:
                break;
        }
        editor->captureTemplateGlobals();
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool PrimitiveFxImGuiEditor::headerButton(ImDrawList* drawList, const char* id, ImVec2 min, ImVec2 max, const char* label, bool active, float fontSize)
{
    ImGui::SetCursorScreenPos(min);
    ImGui::PushID(id);
    ImGui::InvisibleButton("##button", ImVec2(max.x - min.x, max.y - min.y));
    const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    const bool hovered = ImGui::IsItemHovered();
    ImGui::PopID();
    drawMockButton(drawList, min, max, label, active || hovered, fontSize);
    return clicked;
}

bool PrimitiveFxImGuiEditor::headerIconButton(ImDrawList* drawList, const char* id, ImVec2 min, ImVec2 max, const char* icon, bool active)
{
    ImGui::SetCursorScreenPos(min);
    ImGui::PushID(id);
    ImGui::InvisibleButton("##icon_button", ImVec2(max.x - min.x, max.y - min.y));
    const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    const bool hovered = ImGui::IsItemHovered();
    ImGui::PopID();

    const bool on = active || hovered;
    const ImU32 bg = on ? IM_COL32(66, 150, 250, 255) : IM_COL32(30, 30, 30, 255);
    const ImU32 border = on ? IM_COL32(66, 150, 250, 255) : IM_COL32(45, 60, 94, 255);
    const ImU32 fg = on ? IM_COL32(255, 255, 255, 255) : IM_COL32(154, 166, 192, 255);
    drawList->AddRectFilled(min, max, bg, 4.0f);
    drawList->AddRect(min, max, border, 4.0f);

    if (drawSvgHeaderIcon(drawList, min, max, icon, active))
        return clicked;

    const ImVec2 c((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
    const float w = max.x - min.x;
    const float h = max.y - min.y;
    const float stroke = 1.8f;
    auto line = [&](ImVec2 a, ImVec2 b, float thick = 1.8f) { drawList->AddLine(a, b, fg, thick); };
    auto circle = [&](ImVec2 p, float r, int seg = 18) { drawList->AddCircle(p, r, fg, seg, stroke); };

    if (std::strcmp(icon, "undo") == 0 || std::strcmp(icon, "redo") == 0)
    {
        const bool undo = std::strcmp(icon, "undo") == 0;
        const float dir = undo ? -1.0f : 1.0f;
        drawList->PathClear();
        drawList->PathArcTo(ImVec2(c.x + dir * 1.5f, c.y + 1.5f), 7.0f, undo ? -0.30f : 3.44f, undo ? 3.65f : 7.40f, 18);
        drawList->PathStroke(fg, 0, stroke);
        const ImVec2 tip(c.x + dir * -7.0f, c.y - 5.0f);
        line(tip, ImVec2(tip.x + dir * 5.0f, tip.y - 1.0f), stroke);
        line(tip, ImVec2(tip.x + dir * 1.0f, tip.y + 5.0f), stroke);
    }
    else if (std::strcmp(icon, "prev") == 0 || std::strcmp(icon, "next") == 0)
    {
        const bool prev = std::strcmp(icon, "prev") == 0;
        const float dir = prev ? 1.0f : -1.0f;
        line(ImVec2(c.x + dir * 3.5f, c.y - 6.0f), ImVec2(c.x - dir * 3.5f, c.y), 2.1f);
        line(ImVec2(c.x - dir * 3.5f, c.y), ImVec2(c.x + dir * 3.5f, c.y + 6.0f), 2.1f);
    }
    else if (std::strcmp(icon, "save") == 0)
    {
        const ImVec2 a(c.x - 7.0f, c.y - 7.0f);
        const ImVec2 b(c.x + 7.0f, c.y + 7.0f);
        drawList->AddRect(a, b, fg, 1.5f, 0, stroke);
        line(ImVec2(a.x + 3.0f, a.y), ImVec2(a.x + 3.0f, a.y + 5.0f), stroke);
        line(ImVec2(a.x + 3.0f, a.y + 5.0f), ImVec2(b.x - 3.0f, a.y + 5.0f), stroke);
        drawList->AddRect(ImVec2(a.x + 3.0f, c.y + 2.0f), ImVec2(b.x - 3.0f, b.y), fg, 1.0f, 0, stroke);
    }
    else if (std::strcmp(icon, "help") == 0)
    {
        circle(c, 7.0f);
        drawHeaderText(drawList, headerMetaFont, ImVec2(c.x - 2.6f, c.y - 7.8f), "?", 14.0f, fg);
    }
    else if (std::strcmp(icon, "power") == 0)
    {
        drawList->PathClear();
        drawList->PathArcTo(c, 7.0f, 0.72f, 5.56f, 24);
        drawList->PathStroke(fg, 0, stroke);
        line(ImVec2(c.x, c.y - 9.0f), ImVec2(c.x, c.y - 1.0f), 2.0f);
    }
    juce::ignoreUnused(w, h);
    return clicked;
}

void PrimitiveFxImGuiEditor::initialiseHeaderState()
{
    if (headerStateReady)
        return;
    abSnapshots[0] = captureSnapshot();
    abSnapshots[1] = factoryPresetValues(0);
    headerStateReady = true;
}

std::vector<float> PrimitiveFxImGuiEditor::captureSnapshot() const
{
    const auto specs = getParameterSpecs();
    std::vector<float> values(specs.size(), 0.0f);
    for (size_t i = 0; i < specs.size(); ++i)
        values[i] = parameterValue(specs[i].id);
    return values;
}

void PrimitiveFxImGuiEditor::applySnapshot(const std::vector<float>& values)
{
    const auto specs = getParameterSpecs();
    for (size_t i = 0; i < specs.size() && i < values.size(); ++i)
        pushToParameter(specs[i].id, values[i]);
}

void PrimitiveFxImGuiEditor::pushHistory()
{
    const auto current = captureSnapshot();
    if (!undoStack.empty() && undoStack.back() == current)
        return;
    undoStack.push_back(current);
    if (undoStack.size() > 64)
        undoStack.erase(undoStack.begin());
    redoStack.clear();
}

std::vector<float> PrimitiveFxImGuiEditor::factoryPresetValues(int index) const
{
    const auto specs = getParameterSpecs();
    const int preset = juce::jlimit(0, kFactoryPresetCount - 1, index);
    std::vector<float> values;
    values.reserve(specs.size());

    static constexpr float generalShape[] = {
        0.50f, 0.30f, 0.70f, 0.86f, 0.18f,
        0.62f, 0.40f, 0.78f, 0.24f, 0.92f
    };
    static constexpr float mixShape[] = {
        0.50f, 0.24f, 0.40f, 0.58f, 0.78f,
        0.32f, 0.66f, 0.90f, 0.46f, 0.70f
    };
    static constexpr float motionShape[] = {
        0.28f, 0.18f, 0.38f, 0.56f, 0.78f,
        0.08f, 0.48f, 0.68f, 0.30f, 0.88f
    };
    static constexpr float outputShape[] = {
        0.67f, 0.62f, 0.64f, 0.60f, 0.56f,
        0.68f, 0.58f, 0.54f, 0.66f, 0.50f
    };

    auto valueAt = [](const ParameterSpec& spec, float normalised)
    {
        const auto n = juce::jlimit(0.0f, 1.0f, normalised);
        return static_cast<float>(spec.minValue + (spec.maxValue - spec.minValue) * n);
    };

    for (const auto& spec : specs)
    {
        const juce::String id(spec.id);
        float target = static_cast<float>(spec.defaultValue);

        if (preset == 0)
        {
            values.push_back(target);
            continue;
        }

        if (id == "output")
            target = valueAt(spec, outputShape[preset]);
        else if (id == "dryWet")
            target = valueAt(spec, mixShape[preset]);
        else if (id == "rate" || id == "time" || id == "attack" || id == "release")
            target = valueAt(spec, motionShape[preset]);
        else if (id == "width" || id == "spread" || id == "stereo" || id == "phase" || id == "rotation")
            target = valueAt(spec, juce::jlimit(0.0f, 1.0f, generalShape[preset] + 0.10f));
        else if (id == "depth" || id == "feedback" || id == "drive" || id == "ratio" || id == "input")
            target = valueAt(spec, juce::jlimit(0.0f, 1.0f, generalShape[preset] + 0.05f));
        else if (id.containsIgnoreCase("gain") || id == "bias" || id == "balance" || id == "offset" || id == "semitones" || id == "cents")
            target = valueAt(spec, 1.0f - generalShape[preset]);
        else if (id.containsIgnoreCase("freq") || id == "center" || id == "tone" || id == "focus")
            target = valueAt(spec, juce::jlimit(0.0f, 1.0f, generalShape[preset] * 0.82f + 0.08f));
        else if (id == "ceiling")
            target = valueAt(spec, 0.82f - generalShape[preset] * 0.22f);
        else if (id == "lowCut" || id == "bassMono" || id == "mono")
            target = valueAt(spec, 1.0f - mixShape[preset]);
        else
            target = valueAt(spec, generalShape[preset]);

        values.push_back(juce::jlimit(static_cast<float>(spec.minValue), static_cast<float>(spec.maxValue), target));
    }
    return values;
}

void PrimitiveFxImGuiEditor::applyPreset(int index)
{
    const int total = kFactoryPresetCount + static_cast<int>(userPresets.size());
    if (total <= 0)
        return;
    currentPresetIndex = (index % total + total) % total;
    pushHistory();
    if (currentPresetIndex < kFactoryPresetCount)
        applySnapshot(factoryPresetValues(currentPresetIndex));
    else
        applySnapshot(userPresets[static_cast<size_t>(currentPresetIndex - kFactoryPresetCount)]);
}

void PrimitiveFxImGuiEditor::renderHeaderControls(ImDrawList* drawList, float centerY, float centerW)
{
    initialiseHeaderState();
    ImGuiIO& io = ImGui::GetIO();
    const float centeredX = (io.DisplaySize.x - centerW) * 0.5f;
    const float rightLimitX = io.DisplaySize.x - kHeaderRightReserved - centerW;
    float x = std::clamp(centeredX, kHeaderLeftReserved, std::max(kHeaderLeftReserved, rightLimitX));
    if (headerIconButton(drawList, "undo", ImVec2(x, centerY), ImVec2(x + 28.0f, centerY + 28.0f), "undo") && !undoStack.empty())
    {
        redoStack.push_back(captureSnapshot());
        const auto previous = undoStack.back();
        undoStack.pop_back();
        applySnapshot(previous);
    }
    x += 36.0f;
    if (headerIconButton(drawList, "redo", ImVec2(x, centerY), ImVec2(x + 28.0f, centerY + 28.0f), "redo") && !redoStack.empty())
    {
        undoStack.push_back(captureSnapshot());
        const auto next = redoStack.back();
        redoStack.pop_back();
        applySnapshot(next);
    }
    x += 39.0f;
    drawList->AddRectFilled(ImVec2(x, centerY), ImVec2(x + 58.0f, centerY + 28.0f), IM_COL32(20, 20, 20, 255), 4.0f);
    drawList->AddRect(ImVec2(x, centerY), ImVec2(x + 58.0f, centerY + 28.0f), IM_COL32(45, 60, 94, 255), 4.0f);
    if (headerButton(drawList, "ab_a", ImVec2(x + 3.0f, centerY + 3.0f), ImVec2(x + 29.0f, centerY + 25.0f), "A", activeAB == 0, 12.0f) && activeAB != 0)
    {
        abSnapshots[activeAB] = captureSnapshot();
        activeAB = 0;
        pushHistory();
        applySnapshot(abSnapshots[0]);
    }
    if (headerButton(drawList, "ab_b", ImVec2(x + 31.0f, centerY + 3.0f), ImVec2(x + 55.0f, centerY + 25.0f), "B", activeAB == 1, 12.0f) && activeAB != 1)
    {
        abSnapshots[activeAB] = captureSnapshot();
        activeAB = 1;
        pushHistory();
        applySnapshot(abSnapshots[1]);
    }
    x += 66.0f;
    if (headerIconButton(drawList, "preset_prev", ImVec2(x, centerY), ImVec2(x + 24.0f, centerY + 28.0f), "prev"))
        applyPreset(currentPresetIndex - 1);
    x += 32.0f;

    const ImVec2 presetMin(x, centerY);
    const ImVec2 presetMax(x + 140.0f, centerY + 28.0f);
    ImGui::SetCursorScreenPos(presetMin);
    ImGui::InvisibleButton("preset_combo", ImVec2(presetMax.x - presetMin.x, presetMax.y - presetMin.y));
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        ImGui::OpenPopup("preset_popup");
    drawList->AddRectFilled(presetMin, presetMax, IM_COL32(30, 30, 30, 255), 4.0f);
    drawList->AddRect(presetMin, presetMax, IM_COL32(45, 60, 94, 255), 4.0f);
    juce::String presetName = currentPresetIndex < kFactoryPresetCount
        ? juce::String(factoryPresetName(currentPresetIndex))
        : "User " + juce::String(currentPresetIndex - kFactoryPresetCount + 1);
    drawHostText(drawList, ImVec2(x + 12.0f, centerY + 6.0f), presetName.toRawUTF8(), 13.0f, IM_COL32(224, 230, 240, 255));
    drawHostText(drawList, ImVec2(x + 121.0f, centerY + 7.0f), "v", 12.0f, IM_COL32(154, 166, 192, 255));
    if (ImGui::BeginPopup("preset_popup"))
    {
        for (int i = 0; i < kFactoryPresetCount; ++i)
            if (ImGui::MenuItem(factoryPresetName(i), nullptr, currentPresetIndex == i))
                applyPreset(i);
        if (!userPresets.empty())
            ImGui::Separator();
        for (int i = 0; i < static_cast<int>(userPresets.size()); ++i)
            if (ImGui::MenuItem(("User " + juce::String(i + 1)).toRawUTF8(), nullptr, currentPresetIndex == kFactoryPresetCount + i))
                applyPreset(kFactoryPresetCount + i);
        if (currentPresetIndex >= kFactoryPresetCount)
        {
            ImGui::Separator();
            if (ImGui::MenuItem("Delete Current"))
            {
                pushHistory();
                userPresets.erase(userPresets.begin() + (currentPresetIndex - kFactoryPresetCount));
                currentPresetIndex = 0;
                applySnapshot(factoryPresetValues(0));
            }
        }
        ImGui::EndPopup();
    }
    x += 148.0f;
    if (headerIconButton(drawList, "preset_next", ImVec2(x, centerY), ImVec2(x + 24.0f, centerY + 28.0f), "next"))
        applyPreset(currentPresetIndex + 1);
    x += 32.0f;
    if (headerIconButton(drawList, "preset_save", ImVec2(x, centerY), ImVec2(x + 28.0f, centerY + 28.0f), "save"))
    {
        if (currentPresetIndex >= kFactoryPresetCount)
            userPresets[static_cast<size_t>(currentPresetIndex - kFactoryPresetCount)] = captureSnapshot();
        else
        {
            userPresets.push_back(captureSnapshot());
            currentPresetIndex = kFactoryPresetCount + static_cast<int>(userPresets.size()) - 1;
        }
    }

    if (headerIconButton(drawList, "help", ImVec2(io.DisplaySize.x - 66.0f, centerY), ImVec2(io.DisplaySize.x - 38.0f, centerY + 28.0f), "help"))
        juce::URL("https://pbtechlab.com/").launchInDefaultBrowser();
    const bool bypassed = parameterValue("bypass") >= 0.5f;
    if (headerIconButton(drawList, "power", ImVec2(io.DisplaySize.x - 34.0f, centerY), ImVec2(io.DisplaySize.x - 6.0f, centerY + 28.0f), "power", bypassed))
        pushToParameter("bypass", bypassed ? 0.0f : 1.0f);
}

void PrimitiveFxImGuiEditor::renderFlangerPanel()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
    ImGui::Begin("RootCanvas", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBackground);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(ImVec2(0.0f, 0.0f), ImVec2(io.DisplaySize.x, 58.0f), IM_COL32(19, 28, 50, 255));
    drawList->AddLine(ImVec2(0.0f, 57.0f), ImVec2(io.DisplaySize.x, 57.0f), IM_COL32(45, 60, 94, 255), 1.0f);
    drawBrandLogo(drawList, ImVec2(10.0f, 12.0f));
    const auto pluginName = getPluginName();
    drawHeaderText(drawList, headerTitleFont, ImVec2(54.0f, 12.0f), pluginName.toRawUTF8(), 17.0f, IM_COL32(224, 230, 240, 248));
    drawHeaderText(drawList, headerMetaFont, ImVec2(55.0f, 35.0f), "pbTechLab PrimitiveFxLibrary", 10.5f, IM_COL32(154, 166, 192, 155));

    const float centerY = 15.0f;
    const float centerW = kHeaderControlsWidth;
    renderHeaderControls(drawList, centerY, centerW);

    const auto specs = getParameterSpecs();
    const int knobCount = static_cast<int>(specs.size());
    const float sideMargin = kMainSideMargin;
    const float knobWidth = kMainKnobWidth;
    const float availableKnobGap = knobCount > 1
        ? (io.DisplaySize.x - sideMargin * 2.0f - knobWidth * static_cast<float>(knobCount)) / static_cast<float>(knobCount - 1)
        : 0.0f;
    const float knobGap = knobCount > 1 ? std::max(kMainKnobGap, availableKnobGap) : 0.0f;
    const float groupX = sideMargin;
    const float groupY = 58.0f;
    int rateIndex = -1;
    for (int i = 0; i < knobCount; ++i)
        if (juce::String(specs[static_cast<size_t>(i)].id) == "rate")
            rateIndex = i;

    const float rateX = rateIndex >= 0 ? groupX + (knobWidth + knobGap) * static_cast<float>(rateIndex) : groupX;
    const ImVec2 lButtonTarget(rateX + knobWidth * 0.865f, groupY + 49.0f);
    const ImVec2 chipSize(174.0f, 96.0f);
    const float margin = 8.0f;
    const bool placeRight = lButtonTarget.x + 18.0f + chipSize.x <= io.DisplaySize.x - margin;
    float chipX = placeRight ? lButtonTarget.x + 18.0f : rateX - chipSize.x + 4.0f;
    float chipY = lButtonTarget.y - 16.0f;
    chipX = std::clamp(chipX, margin, io.DisplaySize.x - chipSize.x - margin);
    chipY = std::clamp(chipY, margin, io.DisplaySize.y - chipSize.y - margin);
    const ImVec2 chipMin(chipX, chipY);
    const ImVec2 chipMax(chipX + chipSize.x, chipY + chipSize.y);
    const bool chipBlocksMainInput = rateIndex >= 0 && lfoChipOpen && lfoChipVisible && ImGui::IsMouseHoveringRect(chipMin, chipMax, false);
    g_texturedKnobInputBlocked = chipBlocksMainInput;
    texturedKnobInputBlocked = chipBlocksMainInput;

    const bool wasLfoChipOpen = lfoChipOpen;
    const bool lfoEnabled = lfoChipOpen;
    const float visibleLfoDepth = lfoEnabled ? lfoDepth : 0.0f;

    for (int i = 0; i < knobCount; ++i)
    {
        const auto& spec = specs[static_cast<size_t>(i)];
        const float minValue = static_cast<float>(spec.minValue);
        const float maxValue = static_cast<float>(spec.maxValue);
        const float defaultValue = static_cast<float>(spec.defaultValue);
        float value = parameterValue(spec.id);
        const bool isRate = juce::String(spec.id) == "rate";
        const bool bipolar = minValue < 0.0f && maxValue > 0.0f;
        const bool hasDecimals = std::abs(static_cast<float>(spec.step)) < 1.0f;
        const juce::String format = juce::String(hasDecimals ? "%.1f" : "%.0f") + spec.unit;
        ImGui::SetCursorScreenPos(ImVec2(groupX + (knobWidth + knobGap) * static_cast<float>(i), groupY));
        if (isRate && rateTempoSync)
        {
            float displayValue = static_cast<float>(rateSyncIndex);
            TexturedKnobFloat(spec.name, &displayValue, 0.0f, static_cast<float>(kSyncNoteCount - 1), 7.0f, "%.0f", knobWidth, i, true, false, 0.78f, lfoEnabled, static_cast<float>(ImGui::GetTime()) * std::max(0.01f, lfoRate), bipolar, kSyncNoteLabels[rateSyncIndex], visibleLfoDepth, static_cast<int>(std::round(lfoType)), "S", &rateTempoSync, "L", &lfoChipOpen);
            const int nextIndex = std::clamp(static_cast<int>(std::round(displayValue)), 0, kSyncNoteCount - 1);
            if (nextIndex != rateSyncIndex)
            {
                rateSyncIndex = nextIndex;
                pushToParameter(spec.id, syncLabelToHz(rateSyncIndex, processor.hostBpm.load(std::memory_order_relaxed)));
            }
        }
        else
        {
            TexturedKnobFloat(spec.name, &value, minValue, maxValue, defaultValue, format.toRawUTF8(), knobWidth, i, true, false, 0.78f, isRate && lfoEnabled, static_cast<float>(ImGui::GetTime()) * std::max(0.01f, lfoRate), bipolar, nullptr, isRate ? visibleLfoDepth : 0.44f, isRate ? static_cast<int>(std::round(lfoType)) : 0, isRate ? "S" : nullptr, isRate ? &rateTempoSync : nullptr, isRate ? "L" : nullptr, isRate ? &lfoChipOpen : nullptr);
            pushToParameter(spec.id, value);
            if (isRate)
                rateSyncIndex = noteIndexFromRate(value);
        }
    }
    if (wasLfoChipOpen != lfoChipOpen)
        lfoChipVisible = lfoChipOpen;

    g_texturedKnobInputBlocked = false;
    texturedKnobInputBlocked = false;

    if (rateIndex >= 0 && lfoChipOpen && lfoChipVisible)
    {
        ImGui::PushID("lfo_chip");
        DrawLfoChip(chipX, chipY, lButtonTarget, &lfoType, &lfoRate, &lfoDepth, &lfoRateTempoSync, &lfoRateSyncIndex);
        ImGui::PopID();

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            const ImVec2 mouse = io.MousePos;
            const ImVec2 lButtonMin(rateX + knobWidth * 0.8525f - (knobWidth * 0.180f - 2.0f), groupY);
            const ImVec2 lButtonMax(rateX + knobWidth * 0.8525f + 3.0f, groupY + 78.0f);
            const bool inChip = mouse.x >= chipMin.x && mouse.x <= chipMax.x && mouse.y >= chipMin.y && mouse.y <= chipMax.y;
            const bool inPointer = mouse.x >= std::min(lButtonTarget.x, chipX) - 4.0f &&
                mouse.x <= std::max(lButtonTarget.x, chipX + chipSize.x) + 4.0f &&
                mouse.y >= chipY - 4.0f && mouse.y <= chipY + chipSize.y + 4.0f;
            const bool inLButton = mouse.x >= lButtonMin.x && mouse.x <= lButtonMax.x && mouse.y >= lButtonMin.y && mouse.y <= lButtonMax.y;
            if (!inChip && !inPointer && !inLButton)
                lfoChipVisible = false;
        }
    }

    drawList->AddRectFilled(ImVec2(0.0f, io.DisplaySize.y - 21.0f), ImVec2(io.DisplaySize.x, io.DisplaySize.y), IM_COL32(19, 28, 50, 255));
    drawList->AddLine(ImVec2(0.0f, io.DisplaySize.y - 21.0f), ImVec2(io.DisplaySize.x, io.DisplaySize.y - 21.0f), IM_COL32(45, 60, 94, 255), 1.0f);
    drawHostText(drawList, ImVec2(io.DisplaySize.x - 58.0f, io.DisplaySize.y - 17.0f), "v1.0.0", 11.0f, IM_COL32(154, 166, 192, 255));

    ImGui::End();
}

void PrimitiveFxImGuiEditor::syncFromParameters()
{
    if (!rateTempoSync)
        rateSyncIndex = noteIndexFromRate(parameterValue("rate"));
}

void PrimitiveFxImGuiEditor::pushToParameter(const char* id, float value)
{
    if (auto* p = processor.apvts.getParameter(id))
    {
        const auto current = parameterValue(id);
        if (std::abs(current - value) < 0.0001f)
            return;
        const auto normalised = p->convertTo0to1(value);
        p->beginChangeGesture();
        p->setValueNotifyingHost(normalised);
        p->endChangeGesture();
    }
}

float PrimitiveFxImGuiEditor::parameterValue(const char* id) const
{
    if (auto* v = processor.apvts.getRawParameterValue(id))
        return v->load();
    return 0.0f;
}

int PrimitiveFxImGuiEditor::noteIndexFromRate(float rate) const
{
    const auto bpm = processor.hostBpm.load(std::memory_order_relaxed);
    int best = 0;
    float bestDistance = std::numeric_limits<float>::max();
    for (int i = 0; i < kSyncNoteCount; ++i)
    {
        const auto hz = syncLabelToHz(i, bpm);
        const auto distance = std::abs(hz - rate);
        if (distance < bestDistance)
        {
            best = i;
            bestDistance = distance;
        }
    }
    return best;
}
}

juce::AudioProcessorEditor* pbpfl::PrimitiveFxAudioProcessor::createEditor()
{
    return new PrimitiveFxImGuiEditor(*this);
}
