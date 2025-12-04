#include "simulation/DashboardDigitalTwin.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cwchar>
#include <cwctype>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "driver/TouchDriver.hpp"
#include "logic/entities/Alert.hpp"

#if defined(_WIN32)
#include <windows.h>

namespace {
constexpr UINT_PTR kTimerId = 1U;
constexpr UINT kFrameIntervalMs = 100U;
constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 560;
constexpr int kPanelWidth = 320;
constexpr int kPanelHeight = 400;
constexpr wchar_t kMainWindowClass[] = L"MotorcycleTwinSimulatorWindow";
constexpr wchar_t kControlWindowClass[] = L"MotorcycleTwinControlsWindow";

logic::entities::AlertConfig makeSpeedAlert(float fLimit) {
    return {logic::entities::AlertMetadata{"speed_high", "Speed exceeds limit", logic::entities::AlertSeverity::kWarning},
            logic::entities::AlertThreshold{logic::entities::AlertType::kSpeed, fLimit, true},
            std::nullopt};
}

logic::entities::AlertConfig makeTempAlert(float fLimit) {
    return {logic::entities::AlertMetadata{"temp_high", "Engine temp critical", logic::entities::AlertSeverity::kCritical},
            logic::entities::AlertThreshold{logic::entities::AlertType::kEngineTemp, fLimit, true},
            std::nullopt};
}

std::wstring toWide(const std::string& sValue) {
    return {sValue.begin(), sValue.end()};
}

void adjustValue(float& fTarget, float fDelta, float fMin, float fMax) {
    fTarget = std::clamp(fTarget + fDelta, fMin, fMax);
}

float clampValue(float fValue, float fMin, float fMax) {
    return std::clamp(fValue, fMin, fMax);
}

std::wstring formatFloatingValue(double dValue, int iPrecision) {
    std::wostringstream oss;
    oss.setf(std::ios::fixed);
    oss << std::setprecision(static_cast<int>((std::max)(0, iPrecision))) << dValue;
    return oss.str();
}

std::wstring composeValueLine(const wchar_t* pwzPrefix, double dValue, int iPrecision, const wchar_t* pwzSuffix) {
    std::wstring sLine{pwzPrefix};
    sLine += formatFloatingValue(dValue, iPrecision);
    sLine += pwzSuffix;
    return sLine;
}

std::wstring composeAlertLine(const std::wstring& sMessage, bool bIsCritical) {
    std::wstring sLine = L" - ";
    sLine += sMessage;
    sLine += L" [";
    sLine += bIsCritical ? L"CRITICAL" : L"WARNING";
    sLine += L"]";
    return sLine;
}

std::wstring trim(const std::wstring& sValue) {
    const auto itStart = std::find_if_not(sValue.begin(), sValue.end(), [](wchar_t ch) { return std::iswspace(ch) != 0; });
    const auto itEnd = std::find_if_not(sValue.rbegin(), sValue.rend(), [](wchar_t ch) { return std::iswspace(ch) != 0; }).base();
    if (itStart >= itEnd) {
        return {};
    }
    return {itStart, itEnd};
}

std::optional<std::uint32_t> parseCanId(const std::wstring& sRaw) {
    if (sRaw.empty()) {
        return std::nullopt;
    }

    std::wstring sValue = trim(sRaw);
    if (sValue.empty()) {
        return std::nullopt;
    }

    int iBase = 10;
    if (sValue.rfind(L"0x", 0) == 0 || sValue.rfind(L"0X", 0) == 0) {
        sValue = sValue.substr(2);
        iBase = 16;
    } else if (!sValue.empty() && (sValue.back() == L'h' || sValue.back() == L'H')) {
        sValue.pop_back();
        iBase = 16;
    }

    if (sValue.empty()) {
        return std::nullopt;
    }

    wchar_t* pwzEnd = nullptr;
    const unsigned long ulValue = std::wcstoul(sValue.c_str(), &pwzEnd, iBase);
    if (pwzEnd == sValue.c_str() || *pwzEnd != L'\0') {
        return std::nullopt;
    }

    if (ulValue > 0x1FFFFFFFUL) {
        return std::nullopt;
    }

    return static_cast<std::uint32_t>(ulValue);
}

std::optional<std::uint8_t> parseByte(const std::wstring& sRaw) {
    if (sRaw.empty()) {
        return std::nullopt;
    }

    std::wstring sValue = trim(sRaw);
    if (sValue.empty()) {
        return std::nullopt;
    }

    int iBase = 16;
    if (sValue.rfind(L"0x", 0) == 0 || sValue.rfind(L"0X", 0) == 0) {
        sValue = sValue.substr(2);
    } else if (!sValue.empty() && (sValue.back() == L'h' || sValue.back() == L'H')) {
        sValue.pop_back();
    }

    if (sValue.empty()) {
        return std::nullopt;
    }

    wchar_t* pwzEnd = nullptr;
    const unsigned long ulValue = std::wcstoul(sValue.c_str(), &pwzEnd, iBase);
    if (pwzEnd == sValue.c_str() || *pwzEnd != L'\0' || ulValue > (std::numeric_limits<std::uint8_t>::max)()) {
        return std::nullopt;
    }

    return static_cast<std::uint8_t>(ulValue);
}

class TwinSimulatorApp;

class ControlPanelWindow {
public:
    explicit ControlPanelWindow(TwinSimulatorApp& rApp);

    bool create(HINSTANCE hInstance);
    void destroy();
    void updateFromInputs(const simulation::DashboardDigitalTwin::TwinInputs& stInputs);
    void updateAlerts(const std::vector<logic::entities::AlertState>& vAlerts);
    void show();
    void hide();
    void toggleVisibility();
    [[nodiscard]] bool isVisible() const;

private:
    bool registerClass(HINSTANCE hInstance);
    void createControls();
    void layoutControls(int iClientWidth, int iClientHeight);
    void onCommand(WPARAM wParam, LPARAM lParam);
    void handleApply();
    void handleReset();
    void handleTouch();
    void handleAbsToggle();
    void handleSendCan();
    float readFloat(HWND hwndEdit, float fDefault) const;
    void setEditValue(HWND hwndEdit, float fValue, int iPrecision);
    std::wstring readText(HWND hwndEdit) const;
    void clearControls();

    static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    enum : WORD {
        kIdSpeedEdit = 100,
        kIdRpmEdit = 101,
        kIdThrottleEdit = 102,
        kIdTempEdit = 103,
        kIdBatteryEdit = 104,
        kIdAbsCheck = 110,
        kIdApplyButton = 120,
        kIdResetButton = 121,
        kIdTouchButton = 122,
        kIdCanIdEdit = 130,
        kIdCanPayloadEdit = 131,
        kIdSendCanButton = 132
    };

    TwinSimulatorApp& m_rApp;
    HINSTANCE m_hInstance{nullptr};
    HWND m_hwnd{nullptr};
    HWND m_hwndSpeedEdit{nullptr};
    HWND m_hwndRpmEdit{nullptr};
    HWND m_hwndThrottleEdit{nullptr};
    HWND m_hwndTempEdit{nullptr};
    HWND m_hwndBatteryEdit{nullptr};
    HWND m_hwndAbsCheck{nullptr};
    HWND m_hwndApplyButton{nullptr};
    HWND m_hwndResetButton{nullptr};
    HWND m_hwndTouchButton{nullptr};
    HWND m_hwndAlertsList{nullptr};
    HWND m_hwndCanIdEdit{nullptr};
    HWND m_hwndCanPayloadEdit{nullptr};
    HWND m_hwndSendCanButton{nullptr};
    bool m_bUpdating{false};
};

class TwinSimulatorApp {
public:
    TwinSimulatorApp();

    int run(HINSTANCE hInstance);
    void onPanelApply(float fSpeed, float fRpm, float fThrottle, float fTemp, float fBattery, bool bAbsActive);
    void onPanelReset();
    void onPanelTouch();
    void onPanelAbsToggled(bool bAbsActive);
    void onPanelSendCan(std::uint32_t u32Id, const std::vector<std::uint8_t>& vBytes);
    [[nodiscard]] const simulation::DashboardDigitalTwin::TwinInputs& inputs() const noexcept;

private:
    bool registerWindowClass(HINSTANCE hInstance);
    bool createWindow(HINSTANCE hInstance);
    void onCreate();
    void onDestroy();
    void onTimer();
    void onKeyDown(WPARAM wParam);
    void resetInputs();
    void updateTwin();
    void onPaint();

    static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd{nullptr};
    HINSTANCE m_hInstance{nullptr};
    simulation::DashboardDigitalTwin m_twin;
    simulation::DashboardDigitalTwin::TwinInputs m_inputs{};
    std::uint64_t m_u64NowMs{0ULL};
    ControlPanelWindow m_controlPanel;
};

ControlPanelWindow::ControlPanelWindow(TwinSimulatorApp& rApp) : m_rApp(rApp) {}

TwinSimulatorApp::TwinSimulatorApp() : m_controlPanel(*this) {
    m_twin.vSetManualContent("manual/simulator_manual.md",
                             "## Simulator Manual\n"
                             "Use the control panel or keyboard shortcuts to adjust metrics and observe alerts in real time.\n");
    m_twin.vConfigureAlerts({makeSpeedAlert(120.F), makeTempAlert(110.F)});
    m_inputs.fSpeedKph = 65.F;
    m_inputs.fEngineRpm = 3200.F;
    m_inputs.fThrottlePercent = 35.F;
    m_inputs.fEngineTempC = 85.F;
    m_inputs.fBatteryVoltage = 12.4F;
}

int TwinSimulatorApp::run(HINSTANCE hInstance) {
    if (!registerWindowClass(hInstance)) {
        MessageBoxW(nullptr, L"Failed to register simulator window class", L"Motorcycle Simulator", MB_ICONERROR);
        return 1;
    }

    if (!createWindow(hInstance)) {
        MessageBoxW(nullptr, L"Failed to create simulator window", L"Motorcycle Simulator", MB_ICONERROR);
        return 1;
    }

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void TwinSimulatorApp::onPanelApply(float fSpeed, float fRpm, float fThrottle, float fTemp, float fBattery, bool bAbsActive) {
    m_inputs.fSpeedKph = clampValue(fSpeed, 0.F, 240.F);
    m_inputs.fEngineRpm = clampValue(fRpm, 0.F, 14000.F);
    m_inputs.fThrottlePercent = clampValue(fThrottle, 0.F, 100.F);
    m_inputs.fEngineTempC = clampValue(fTemp, -20.F, 160.F);
    m_inputs.fBatteryVoltage = clampValue(fBattery, 6.F, 16.F);
    m_inputs.bAbsActive = bAbsActive;
    updateTwin();
    m_controlPanel.updateFromInputs(m_inputs);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void TwinSimulatorApp::onPanelReset() {
    resetInputs();
    updateTwin();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void TwinSimulatorApp::onPanelTouch() {
    driver::TouchEvent stEvent{};
    stEvent.eType = driver::TouchEvent::Type::kTap;
    m_twin.vEnqueueTouch(stEvent);
    m_twin.vProcessOnce();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void TwinSimulatorApp::onPanelAbsToggled(bool bAbsActive) {
    m_inputs.bAbsActive = bAbsActive;
    updateTwin();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void TwinSimulatorApp::onPanelSendCan(std::uint32_t u32Id, const std::vector<std::uint8_t>& vBytes) {
    logic::ports::RawCanFrame stFrame{};
    stFrame.u32Id = u32Id;
    stFrame.u64TimestampMs = m_u64NowMs;
    const std::size_t uLength = (std::min)(vBytes.size(), stFrame.au8Data.size());
    stFrame.u8Dlc = static_cast<std::uint8_t>(uLength);
    for (std::size_t uIndex = 0; uIndex < uLength; ++uIndex) {
        stFrame.au8Data[uIndex] = vBytes[uIndex];
    }

    m_twin.vEnqueueFrame(stFrame);
    m_twin.vProcessOnce();
    m_controlPanel.updateAlerts(m_twin.rDisplayDriver().vLastAlerts());
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

const simulation::DashboardDigitalTwin::TwinInputs& TwinSimulatorApp::inputs() const noexcept {
    return m_inputs;
}

bool TwinSimulatorApp::registerWindowClass(HINSTANCE hInstance) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &TwinSimulatorApp::sWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kMainWindowClass;
    return RegisterClassExW(&wc) != 0;
}

bool TwinSimulatorApp::createWindow(HINSTANCE hInstance) {
    m_hInstance = hInstance;
    m_hwnd = CreateWindowExW(0, kMainWindowClass, L"Motorcycle Digital Twin Simulator", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, kWindowWidth, kWindowHeight, nullptr, nullptr, hInstance, this);
    return m_hwnd != nullptr;
}

void TwinSimulatorApp::onCreate() {
    SetTimer(m_hwnd, kTimerId, kFrameIntervalMs, nullptr);
    updateTwin();

    if (!m_controlPanel.create(m_hInstance)) {
        MessageBoxW(m_hwnd, L"Failed to create control panel window", L"Motorcycle Simulator", MB_ICONWARNING);
    } else {
        m_controlPanel.updateFromInputs(m_inputs);
        m_controlPanel.updateAlerts(m_twin.rDisplayDriver().vLastAlerts());
        m_controlPanel.show();
    }
}

void TwinSimulatorApp::onDestroy() {
    KillTimer(m_hwnd, kTimerId);
    m_controlPanel.destroy();
    PostQuitMessage(0);
}

void TwinSimulatorApp::onTimer() {
    m_u64NowMs += kFrameIntervalMs;
    m_twin.vSetClockNow(m_u64NowMs);
    m_twin.vApplyInputs(m_inputs);
    m_twin.vProcessOnce();
    m_controlPanel.updateAlerts(m_twin.rDisplayDriver().vLastAlerts());
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void TwinSimulatorApp::onKeyDown(WPARAM wParam) {
    switch (wParam) {
        case VK_ESCAPE:
            PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
            return;
        case VK_UP:
            adjustValue(m_inputs.fSpeedKph, 5.F, 0.F, 240.F);
            break;
        case VK_DOWN:
            adjustValue(m_inputs.fSpeedKph, -5.F, 0.F, 240.F);
            break;
        case 'W':
            adjustValue(m_inputs.fEngineRpm, 250.F, 0.F, 14000.F);
            break;
        case 'S':
            adjustValue(m_inputs.fEngineRpm, -250.F, 0.F, 14000.F);
            break;
        case 'E':
            adjustValue(m_inputs.fThrottlePercent, 5.F, 0.F, 100.F);
            break;
        case 'D':
            adjustValue(m_inputs.fThrottlePercent, -5.F, 0.F, 100.F);
            break;
        case 'R':
            adjustValue(m_inputs.fEngineTempC, 2.F, -20.F, 160.F);
            break;
        case 'F':
            adjustValue(m_inputs.fEngineTempC, -2.F, -20.F, 160.F);
            break;
        case 'T':
            adjustValue(m_inputs.fBatteryVoltage, 0.2F, 6.F, 16.F);
            break;
        case 'G':
            adjustValue(m_inputs.fBatteryVoltage, -0.2F, 6.F, 16.F);
            break;
        case 'A':
            m_inputs.bAbsActive = !m_inputs.bAbsActive;
            break;
        case 'Z':
            resetInputs();
            break;
        case 'M': {
            driver::TouchEvent stEvent{};
            stEvent.eType = driver::TouchEvent::Type::kTap;
            m_twin.vEnqueueTouch(stEvent);
            break;
        }
        case 'H':
            m_twin.vManualGoHome();
            return;
        case 'B':
            m_twin.vManualGoBack();
            return;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            const auto optManual = m_twin.rDisplayDriver().optLastManual();
            if (optManual.has_value()) {
                const auto uIndex = static_cast<std::size_t>(wParam - '1');
                if (uIndex < optManual->vChildren.size()) {
                    m_twin.vManualOpenTopic(optManual->vChildren[uIndex].sId);
                }
            }
            return;
        }
        case 'P':
            m_controlPanel.toggleVisibility();
            return;
        default:
            break;
    }

    m_controlPanel.updateFromInputs(m_inputs);
    updateTwin();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void TwinSimulatorApp::resetInputs() {
    m_inputs.fSpeedKph = 65.F;
    m_inputs.fEngineRpm = 3200.F;
    m_inputs.fThrottlePercent = 35.F;
    m_inputs.bAbsActive = false;
    m_inputs.fEngineTempC = 85.F;
    m_inputs.fBatteryVoltage = 12.4F;
    m_controlPanel.updateFromInputs(m_inputs);
}

void TwinSimulatorApp::updateTwin() {
    m_twin.vSetClockNow(m_u64NowMs);
    m_twin.vApplyInputs(m_inputs);
    m_twin.vProcessOnce();
    m_controlPanel.updateAlerts(m_twin.rDisplayDriver().vLastAlerts());
}

void TwinSimulatorApp::onPaint() {
    PAINTSTRUCT ps{};
    const HDC hdc = BeginPaint(m_hwnd, &ps);
    if (!hdc) {
        return;
    }

    SetBkMode(hdc, TRANSPARENT);

    const auto stData = m_twin.rDisplayDriver().stLastData();
    const auto vAlerts = m_twin.rDisplayDriver().vLastAlerts();
    const auto optManual = m_twin.rDisplayDriver().optLastManual();
    const auto& stApp = m_twin.stAppMetadata();

    std::vector<std::wstring> vLines;
    const std::string sHeader = stApp.sAppName + " v" + stApp.sVersion + " (" + stApp.sBuildId + ")";
    vLines.push_back(toWide(sHeader));
    vLines.push_back(toWide(stApp.sCopyright));
    vLines.push_back(L"Motorcycle Digital Twin Simulator");
    vLines.push_back(L"--------------------------------------------------");
    vLines.push_back(L"Adjust telemetry via the control panel window (P) or the keyboard shortcuts below.");
    vLines.push_back(composeValueLine(L"Speed [Up/Down]: ", stData.fSpeedKph, 1, L" km/h"));
    vLines.push_back(composeValueLine(L"Engine RPM [W/S]: ", stData.fEngineRpm, 0, L" rpm"));
    vLines.push_back(composeValueLine(L"Throttle [E/D]: ", stData.fThrottlePercent, 1, L"%"));
    vLines.push_back(composeValueLine(L"Engine Temp [R/F]: ", stData.fEngineTempC, 1, L" C"));
    vLines.push_back(composeValueLine(L"Battery [T/G]: ", stData.fBatteryVoltage, 1, L" V"));
    std::wstring sAbsLine = L"ABS [A]: ";
    sAbsLine += stData.bAbsActive ? L"ENGAGED" : L"OFF";
    vLines.push_back(std::move(sAbsLine));
    vLines.push_back(L"");

    if (!vAlerts.empty()) {
        vLines.push_back(L"Active Alerts:");
        for (const auto& stAlert : vAlerts) {
            if (!stAlert.bActive) {
                continue;
            }
            vLines.push_back(composeAlertLine(toWide(stAlert.stMetadata.sMessage),
                                              stAlert.stMetadata.eSeverity == logic::entities::AlertSeverity::kCritical));
        }
    } else {
        vLines.push_back(L"No active alerts");
    }

    vLines.push_back(L"");
    vLines.push_back(
        L"Controls: ↑/↓ speed, W/S rpm, E/D throttle, R/F temp, T/G battery, "
        L"A ABS toggle, M touch tap, Z reset, P panel, Esc exit");

    if (optManual.has_value()) {
        vLines.push_back(L"");
        vLines.push_back(L"Manual: " + toWide(optManual->sTitle));
        if (!optManual->sBody.empty()) {
            const auto sPreview = optManual->sBody.substr(0, std::min<std::size_t>(optManual->sBody.size(), 200U));
            vLines.push_back(toWide(sPreview));
        }
        if (!optManual->vChildren.empty()) {
            vLines.push_back(L"Topics:");
            for (const auto& stChild : optManual->vChildren) {
                const std::wstring sTopic = L" - " + toWide(stChild.sTitle) + L" [" + toWide(stChild.sId) + L"]";
                vLines.push_back(sTopic);
            }
            std::wstring sNav = L"Use Back=";
            sNav += optManual->bCanGoBack ? L"Yes" : L"No";
            sNav += L" Home=";
            sNav += optManual->bCanGoHome ? L"Yes" : L"No";
            vLines.push_back(std::move(sNav));
        }
    }

    int iY = 10;
    for (const auto& sLine : vLines) {
        TextOutW(hdc, 10, iY, sLine.c_str(), static_cast<int>(sLine.length()));
        iY += 18;
    }

    EndPaint(m_hwnd, &ps);
}

LRESULT CALLBACK TwinSimulatorApp::sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        const auto pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* pApp = static_cast<TwinSimulatorApp*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    auto* pApp = reinterpret_cast<TwinSimulatorApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (!pApp) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    switch (msg) {
        case WM_CREATE:
            pApp->m_hwnd = hwnd;
            pApp->onCreate();
            return 0;
        case WM_DESTROY:
            pApp->onDestroy();
            return 0;
        case WM_TIMER:
            if (wParam == kTimerId) {
                pApp->onTimer();
            }
            return 0;
        case WM_KEYDOWN:
            pApp->onKeyDown(wParam);
            return 0;
        case WM_PAINT:
            pApp->onPaint();
            return 0;
        default:
            break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool ControlPanelWindow::registerClass(HINSTANCE hInstance) {
    static bool bRegistered = false;
    if (bRegistered) {
        return true;
    }

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &ControlPanelWindow::sWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kControlWindowClass;

    bRegistered = RegisterClassExW(&wc) != 0;
    return bRegistered;
}

bool ControlPanelWindow::create(HINSTANCE hInstance) {
    if (!registerClass(hInstance)) {
        return false;
    }

    m_hInstance = hInstance;
    m_hwnd = CreateWindowExW(
        0,
        kControlWindowClass,
        L"Twin Control Panel",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        kPanelWidth,
        kPanelHeight,
        nullptr,
        nullptr,
        hInstance,
        this);
    if (!m_hwnd) {
        return false;
    }

    ShowWindow(m_hwnd, SW_HIDE);
    UpdateWindow(m_hwnd);
    return true;
}

void ControlPanelWindow::destroy() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

void ControlPanelWindow::updateFromInputs(const simulation::DashboardDigitalTwin::TwinInputs& stInputs) {
    if (!m_hwnd) {
        return;
    }

    m_bUpdating = true;
    setEditValue(m_hwndSpeedEdit, stInputs.fSpeedKph, 1);
    setEditValue(m_hwndRpmEdit, stInputs.fEngineRpm, 0);
    setEditValue(m_hwndThrottleEdit, stInputs.fThrottlePercent, 1);
    setEditValue(m_hwndTempEdit, stInputs.fEngineTempC, 1);
    setEditValue(m_hwndBatteryEdit, stInputs.fBatteryVoltage, 2);
    SendMessageW(
        m_hwndAbsCheck,
        BM_SETCHECK,
        stInputs.bAbsActive ? BST_CHECKED : BST_UNCHECKED,
        0);
    m_bUpdating = false;
}

void ControlPanelWindow::updateAlerts(const std::vector<logic::entities::AlertState>& vAlerts) {
    if (!m_hwndAlertsList) {
        return;
    }

    SendMessageW(m_hwndAlertsList, LB_RESETCONTENT, 0, 0);

    bool bAdded = false;
    for (const auto& stAlert : vAlerts) {
        if (!stAlert.bActive) {
            continue;
        }

        const std::wstring sLine = composeAlertLine(
            toWide(stAlert.stMetadata.sMessage),
            stAlert.stMetadata.eSeverity == logic::entities::AlertSeverity::kCritical);
        SendMessageW(
            m_hwndAlertsList,
            LB_ADDSTRING,
            0,
            reinterpret_cast<LPARAM>(sLine.c_str()));
        bAdded = true;
    }

    if (!bAdded) {
        static const wchar_t* pszNone = L"No active alerts";
        SendMessageW(
            m_hwndAlertsList,
            LB_ADDSTRING,
            0,
            reinterpret_cast<LPARAM>(pszNone));
    }
}

void ControlPanelWindow::show() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        SetForegroundWindow(m_hwnd);
    }
}

void ControlPanelWindow::hide() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
    }
}

void ControlPanelWindow::toggleVisibility() {
    if (!m_hwnd) {
        return;
    }

    if (isVisible()) {
        hide();
    } else {
        show();
    }
}

bool ControlPanelWindow::isVisible() const {
    return m_hwnd != nullptr && IsWindowVisible(m_hwnd) != 0;
}

void ControlPanelWindow::createControls() {
    const int iLeft = 16;
    const int iLabelWidth = 150;
    const int iEditWidth = 110;
    int iTop = 16;

    auto createLabel = [&](const wchar_t* pszText) {
        CreateWindowExW(
            0,
            L"STATIC",
            pszText,
            WS_CHILD | WS_VISIBLE,
            iLeft,
            iTop,
            iLabelWidth,
            20,
            m_hwnd,
            nullptr,
            m_hInstance,
            nullptr);
    };

    auto createEdit = [&](WORD wId) {
        return CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
            iLeft + iLabelWidth,
            iTop - 2,
            iEditWidth,
            24,
            m_hwnd,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(wId)),
            m_hInstance,
            nullptr);
    };

    createLabel(L"Speed (km/h)");
    m_hwndSpeedEdit = createEdit(kIdSpeedEdit);
    iTop += 32;

    createLabel(L"Engine RPM");
    m_hwndRpmEdit = createEdit(kIdRpmEdit);
    iTop += 32;

    createLabel(L"Throttle (%)");
    m_hwndThrottleEdit = createEdit(kIdThrottleEdit);
    iTop += 32;

    createLabel(L"Engine Temp (C)");
    m_hwndTempEdit = createEdit(kIdTempEdit);
    iTop += 32;

    createLabel(L"Battery (V)");
    m_hwndBatteryEdit = createEdit(kIdBatteryEdit);
    iTop += 32;

    m_hwndAbsCheck = CreateWindowExW(
        0,
        L"BUTTON",
        L"ABS Active",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
        iLeft,
        iTop,
        iLabelWidth + iEditWidth,
        24,
        m_hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdAbsCheck)),
        m_hInstance,
        nullptr);
    iTop += 36;

    m_hwndApplyButton = CreateWindowExW(
        0,
        L"BUTTON",
        L"Apply",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
        iLeft,
        iTop,
        90,
        26,
        m_hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdApplyButton)),
        m_hInstance,
        nullptr);
    m_hwndResetButton = CreateWindowExW(
        0,
        L"BUTTON",
        L"Reset",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        iLeft + 100,
        iTop,
        90,
        26,
        m_hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdResetButton)),
        m_hInstance,
        nullptr);
    m_hwndTouchButton = CreateWindowExW(
        0,
        L"BUTTON",
        L"Touch Tap",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        iLeft + 200,
        iTop,
        90,
        26,
        m_hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdTouchButton)),
        m_hInstance,
        nullptr);
    iTop += 40;

    CreateWindowExW(
        0,
        L"STATIC",
        L"Active alerts",
        WS_CHILD | WS_VISIBLE,
        iLeft,
        iTop,
        iLabelWidth + iEditWidth,
        20,
        m_hwnd,
        nullptr,
        m_hInstance,
        nullptr);
    iTop += 24;

    m_hwndAlertsList = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"LISTBOX",
        L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
        iLeft,
        iTop,
        iLabelWidth + iEditWidth + 30,
        120,
        m_hwnd,
        nullptr,
        m_hInstance,
        nullptr);

    iTop += 136;

    CreateWindowExW(
        0,
        L"STATIC",
        L"CAN ID (hex/dec)",
        WS_CHILD | WS_VISIBLE,
        iLeft,
        iTop,
        iLabelWidth + iEditWidth,
        20,
        m_hwnd,
        nullptr,
        m_hInstance,
        nullptr);
    iTop += 22;
    m_hwndCanIdEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
        iLeft,
        iTop,
        iLabelWidth + iEditWidth + 30,
        24,
        m_hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdCanIdEdit)),
        m_hInstance,
        nullptr);

    iTop += 32;
    CreateWindowExW(
        0,
        L"STATIC",
        L"Payload bytes (hex)",
        WS_CHILD | WS_VISIBLE,
        iLeft,
        iTop,
        iLabelWidth + iEditWidth,
        20,
        m_hwnd,
        nullptr,
        m_hInstance,
        nullptr);
    iTop += 22;
    m_hwndCanPayloadEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
        iLeft,
        iTop,
        iLabelWidth + iEditWidth + 30,
        24,
        m_hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdCanPayloadEdit)),
        m_hInstance,
        nullptr);

    iTop += 36;
    m_hwndSendCanButton = CreateWindowExW(
        0,
        L"BUTTON",
        L"Send CAN Frame",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        iLeft,
        iTop,
        160,
        26,
        m_hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdSendCanButton)),
        m_hInstance,
        nullptr);
}

void ControlPanelWindow::onCommand(WPARAM wParam, LPARAM) {
    const WORD wId = LOWORD(wParam);
    const WORD wCode = HIWORD(wParam);

    if (wId == kIdApplyButton && wCode == BN_CLICKED) {
        handleApply();
        return;
    }

    if (wId == kIdResetButton && wCode == BN_CLICKED) {
        handleReset();
        return;
    }

    if (wId == kIdTouchButton && wCode == BN_CLICKED) {
        handleTouch();
        return;
    }

    if (wId == kIdSendCanButton && wCode == BN_CLICKED) {
        handleSendCan();
        return;
    }

    if (wId == kIdAbsCheck && wCode == BN_CLICKED) {
        handleAbsToggle();
    }
}

void ControlPanelWindow::handleApply() {
    const auto& stInputs = m_rApp.inputs();
    const float fSpeed = readFloat(m_hwndSpeedEdit, stInputs.fSpeedKph);
    const float fRpm = readFloat(m_hwndRpmEdit, stInputs.fEngineRpm);
    const float fThrottle = readFloat(m_hwndThrottleEdit, stInputs.fThrottlePercent);
    const float fTemp = readFloat(m_hwndTempEdit, stInputs.fEngineTempC);
    const float fBattery = readFloat(m_hwndBatteryEdit, stInputs.fBatteryVoltage);
    const bool bAbsActive =
        SendMessageW(m_hwndAbsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
    m_rApp.onPanelApply(fSpeed, fRpm, fThrottle, fTemp, fBattery, bAbsActive);
}

void ControlPanelWindow::handleReset() {
    m_rApp.onPanelReset();
}

void ControlPanelWindow::handleTouch() {
    m_rApp.onPanelTouch();
}

void ControlPanelWindow::handleAbsToggle() {
    if (m_bUpdating) {
        return;
    }

    const bool bAbsActive =
        SendMessageW(m_hwndAbsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
    m_rApp.onPanelAbsToggled(bAbsActive);
}

void ControlPanelWindow::handleSendCan() {
    const std::wstring sIdText = readText(m_hwndCanIdEdit);
    const auto optId = parseCanId(sIdText);
    if (!optId.has_value()) {
        MessageBoxW(
            m_hwnd,
            L"Enter a valid CAN identifier (0x0 - 0x1FFFFFFF).",
            L"Invalid CAN ID",
            MB_ICONERROR | MB_OK);
        return;
    }

    const std::wstring sPayloadText = readText(m_hwndCanPayloadEdit);
    std::wstringstream ss{sPayloadText};
    std::wstring sToken;
    std::vector<std::uint8_t> vBytes;
    while (ss >> sToken) {
        if (vBytes.size() >= 8U) {
            MessageBoxW(
                m_hwnd,
                L"CAN payload may include up to 8 bytes.",
                L"Payload Too Long",
                MB_ICONERROR | MB_OK);
            return;
        }

        const auto optByte = parseByte(sToken);
        if (!optByte.has_value()) {
            MessageBoxW(
                m_hwnd,
                L"Provide payload bytes as hex values (e.g., 0A FF 2C).",
                L"Invalid Payload",
                MB_ICONERROR | MB_OK);
            return;
        }

        vBytes.push_back(*optByte);
    }

    m_rApp.onPanelSendCan(*optId, vBytes);
}

float ControlPanelWindow::readFloat(HWND hwndEdit, float fDefault) const {
    if (!hwndEdit) {
        return fDefault;
    }

    std::array<wchar_t, 64> awzBuffer{};
    const int iLen = GetWindowTextW(hwndEdit, awzBuffer.data(), static_cast<int>(awzBuffer.size()));
    if (iLen <= 0) {
        return fDefault;
    }

    wchar_t* pwzEnd = nullptr;
    const float fValue = std::wcstof(awzBuffer.data(), &pwzEnd);
    if (pwzEnd == awzBuffer.data()) {
        return fDefault;
    }

    return fValue;
}

void ControlPanelWindow::setEditValue(HWND hwndEdit, float fValue, int iPrecision) {
    if (!hwndEdit) {
        return;
    }

    const std::wstring sValue = formatFloatingValue(fValue, iPrecision);
    SetWindowTextW(hwndEdit, sValue.c_str());
}

std::wstring ControlPanelWindow::readText(HWND hwndEdit) const {
    if (!hwndEdit) {
        return {};
    }

    const int iLen = GetWindowTextLengthW(hwndEdit);
    if (iLen <= 0) {
        return {};
    }

    std::wstring sBuffer(static_cast<std::size_t>(iLen) + 1ULL, L'\0');
    const int iWritten = GetWindowTextW(hwndEdit, sBuffer.data(), iLen + 1);
    if (iWritten <= 0) {
        return {};
    }

    sBuffer.resize(static_cast<std::size_t>(iWritten));
    return trim(sBuffer);
}

void ControlPanelWindow::clearControls() {
    m_hwndSpeedEdit = nullptr;
    m_hwndRpmEdit = nullptr;
    m_hwndThrottleEdit = nullptr;
    m_hwndTempEdit = nullptr;
    m_hwndBatteryEdit = nullptr;
    m_hwndAbsCheck = nullptr;
    m_hwndApplyButton = nullptr;
    m_hwndResetButton = nullptr;
    m_hwndTouchButton = nullptr;
    m_hwndAlertsList = nullptr;
    m_hwndCanIdEdit = nullptr;
    m_hwndCanPayloadEdit = nullptr;
    m_hwndSendCanButton = nullptr;
}

void ControlPanelWindow::layoutControls(int iClientWidth, int iClientHeight) {
    if (!m_hwnd) {
        return;
    }

    const int iMargin = 16;
    const int iLabelWidth = 150;
    const int iEditHeight = 24;
    const int iRowDelta = iEditHeight + 8;
    const int iEffectiveWidth = (std::max)(iClientWidth - (iMargin * 2), iLabelWidth + 120);
    const int iEditLeft = iMargin + iLabelWidth;
    const int iEditWidth = (std::max)(110, iEffectiveWidth - iLabelWidth);

    int iTop = iMargin;
    auto placeEdit = [&](HWND hwnd) {
        if (!hwnd) {
            return;
        }
        MoveWindow(hwnd, iEditLeft, iTop - 2, iEditWidth, iEditHeight, TRUE);
        iTop += iRowDelta;
    };

    placeEdit(m_hwndSpeedEdit);
    placeEdit(m_hwndRpmEdit);
    placeEdit(m_hwndThrottleEdit);
    placeEdit(m_hwndTempEdit);
    placeEdit(m_hwndBatteryEdit);

    if (m_hwndAbsCheck) {
        MoveWindow(m_hwndAbsCheck, iMargin, iTop, iEffectiveWidth, iEditHeight, TRUE);
    }
    iTop += 36;

    const int iButtonTop = iTop;
    if (m_hwndApplyButton && m_hwndResetButton && m_hwndTouchButton) {
        const int iButtonWidth = (std::max)(90, (iEffectiveWidth - 20) / 3);
        MoveWindow(m_hwndApplyButton, iMargin, iButtonTop, iButtonWidth, 26, TRUE);
        MoveWindow(m_hwndResetButton, iMargin + iButtonWidth + 10, iButtonTop, iButtonWidth, 26, TRUE);
        MoveWindow(m_hwndTouchButton, iMargin + 2 * (iButtonWidth + 10), iButtonTop, iButtonWidth, 26, TRUE);
    }

    iTop = iButtonTop + 40;
    const int iAlertsTop = iTop + 24;
    const int iAlertsHeight = (std::max)(80, iClientHeight - iAlertsTop - 160);
    if (m_hwndAlertsList) {
        MoveWindow(m_hwndAlertsList, iMargin, iAlertsTop, iEffectiveWidth, iAlertsHeight, TRUE);
    }

    const int iCanSectionTop = iAlertsTop + iAlertsHeight + 16;
    if (m_hwndCanIdEdit) {
        MoveWindow(m_hwndCanIdEdit, iMargin, iCanSectionTop + 20, iEffectiveWidth, iEditHeight, TRUE);
    }

    if (m_hwndCanPayloadEdit) {
        MoveWindow(m_hwndCanPayloadEdit, iMargin, iCanSectionTop + 20 + iEditHeight + 20, iEffectiveWidth, iEditHeight, TRUE);
    }

    if (m_hwndSendCanButton) {
        const int iRequiredTop = iCanSectionTop + 20 + (iEditHeight + 20) * 2;
        const int iMaxTop = (std::max)(iMargin, iClientHeight - iMargin - 30);
        const int iSendButtonTop = (std::min)(iMaxTop, (std::max)(iRequiredTop, iMargin));
        MoveWindow(m_hwndSendCanButton, iMargin, iSendButtonTop, 180, 26, TRUE);
    }
}

LRESULT ControlPanelWindow::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            m_hwnd = hwnd;
            createControls();
            if (RECT rc; GetClientRect(hwnd, &rc)) {
                layoutControls(rc.right - rc.left, rc.bottom - rc.top);
            }
            return 0;
        case WM_COMMAND:
            onCommand(wParam, lParam);
            return 0;
        case WM_SIZE:
            layoutControls(LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_GETMINMAXINFO: {
            auto* pInfo = reinterpret_cast<MINMAXINFO*>(lParam);
            pInfo->ptMinTrackSize.x = 360;
            pInfo->ptMinTrackSize.y = 500;
            return 0;
        }
        case WM_CLOSE:
            hide();
            return 0;
        case WM_DESTROY:
            clearControls();
            m_hwnd = nullptr;
            return 0;
        default:
            break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ControlPanelWindow::sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        const auto pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* pWindow = static_cast<ControlPanelWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    auto* pWindow = reinterpret_cast<ControlPanelWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (!pWindow) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    return pWindow->wndProc(hwnd, msg, wParam, lParam);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    TwinSimulatorApp app;
    return app.run(hInstance);
}

#else

#include <iostream>

int main() {
    std::cout << "The interactive twin simulator is currently supported on Windows only." << std::endl;
    return 1;
}

#endif
