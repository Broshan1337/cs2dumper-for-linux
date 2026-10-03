#include <windows.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <string_view>
#include <mutex>
#include <thread>
#include <iostream>
#include <Psapi.h>

#include <d3d11.h>
#include <windowsx.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_internal.h"

#include "../utils/module_utils.h"
#include "../scanner/scanner.h"
#include "../utils/reporting.h"
#include "gui.h"

// Link necessary d3d11 lib
#pragma comment(lib, "d3d11.lib")

// Globals

static int active_tab = 0;

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace gui
{
    Win32Context ctx;

    constexpr wchar_t target_process_name[] = L"cs2.exe";

    [[nodiscard]] DWORD wait_for_process(const wchar_t* process_name) noexcept {
        std::wcout << L"Waiting for process " << process_name << L" ...\n";

        DWORD pid = 0;
        while ((pid = utils::get_process_id_by_name(process_name)) == 0) {
            Sleep(1000);
        }

        std::wcout << L"Found PID: " << pid << L"\n";
        return pid;
    }

    void ToggleClickThrough(bool enable) {
        LONG exStyle = GetWindowLong(ctx.window, GWL_EXSTYLE);
        if (enable) {
            SetWindowLong(ctx.window, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
        }
        else {
            SetWindowLong(ctx.window, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
        }
    }

    // Globals to track window size for ImGui window position/size inside Win32 window
    static int g_window_width = 500;
    static int g_window_height = 400;

    void RenderGui() {
        static int g_window_width = 500;
        static int g_window_height = 400;

        static bool show_hooks = true;
        static bool show_manual = true;
        static bool show_shellcode = true;

        ImGuiWindow* window = ImGui::FindWindowByName("VAC3 Anti-Cheat Helper");

        // If window not found yet (early frame), use cached size
        int current_width = g_window_width;
        int current_height = g_window_height;

        if (window != nullptr) {
            current_width = (int)window->Size.x;
            current_height = (int)window->Size.y;
        }

        // Manual drag support: if left mouse is dragging ImGui window, move Win32 window
        if (window && ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

            RECT wndRect;
            GetWindowRect(ctx.window, &wndRect);

            int new_x = wndRect.left + (int)drag_delta.x;
            int new_y = wndRect.top + (int)drag_delta.y;

            RECT work_area;
            SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);

            if (new_x < work_area.left) new_x = work_area.left;
            if (new_y < work_area.top) new_y = work_area.top;
            if (new_x + current_width > work_area.right) new_x = work_area.right - current_width;
            if (new_y + current_height > work_area.bottom) new_y = work_area.bottom - current_height;

            SetWindowPos(ctx.window, HWND_TOPMOST, new_x, new_y, current_width, current_height, SWP_NOZORDER | SWP_NOACTIVATE);
        }

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_MenuBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |      // ImGui window fixed inside Win32 window
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

        ImGui::Begin("VAC3 Anti-Cheat Helper", nullptr, flags);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::MenuItem("Dashboard", nullptr, active_tab == 0))
                active_tab = 0;
            if (ImGui::MenuItem("Scanner", nullptr, active_tab == 1))
                active_tab = 1;
            if (ImGui::MenuItem("Settings", nullptr, active_tab == 2))
                active_tab = 2;
            if (ImGui::MenuItem("About", nullptr, active_tab == 3))
                active_tab = 3;
            ImGui::Separator();
            if (ImGui::MenuItem("Close"))
                PostQuitMessage(0);
            ImGui::EndMenuBar();
        }

        if (active_tab == 0) {
            ImGui::Text("Welcome to your VAC3-style scanner.");
            ImGui::Text("This tool is designed for detecting cheats like Vac3");
        }
        else if (active_tab == 1) {
            ImGui::Text("Scanner:");

            if (ImGui::Button(scanner::is_scanning ? "Stop Scan" : "Start Scan", ImVec2(120, 0))) {
                if (!scanner::is_scanning) {
                    scanner::is_scanning = true;
                    scanner::stop_scanning = false;
                    {
                        std::lock_guard<std::mutex> lock(utils::log_mutex);
                        utils::log_lines.clear();
                    }

                    std::thread([] {
                        const DWORD pid = wait_for_process(target_process_name);
                        if (pid == 0) {
                            std::lock_guard<std::mutex> lock(utils::log_mutex);
                            utils::log_lines.push_back(L"Failed to find process.");
                            scanner::is_scanning = false;
                            return;
                        }
                        scanner::do_scan(pid);
                        scanner::is_scanning = false;
                        std::lock_guard<std::mutex> lock(utils::log_mutex);
                        utils::log_lines.push_back(L"Scan completed.");
                        }).detach();
                }
                else {
                    scanner::stop_scanning = true;
                }
            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Memory Hooks", &show_hooks)) {
                std::lock_guard<std::mutex> lock(utils::log_mutex);
                for (const auto& line : utils::log_lines)
                    if (wcsstr(line.c_str(), L"[hook]")) ImGui::TextWrapped("%ws", line.c_str());
            }

            if (ImGui::CollapsingHeader("Manual Map Modules", &show_manual)) {
                std::lock_guard<std::mutex> lock(utils::log_mutex);
                for (const auto& line : utils::log_lines)
                    if (wcsstr(line.c_str(), L"[manual]")) ImGui::TextWrapped("%ws", line.c_str());
            }

            if (ImGui::CollapsingHeader("Shellcode Injections", &show_shellcode)) {
                std::lock_guard<std::mutex> lock(utils::log_mutex);
                for (const auto& line : utils::log_lines)
                    if (wcsstr(line.c_str(), L"[shellcode]")) ImGui::TextWrapped("%ws", line.c_str());
            }
        }
        else if (active_tab == 2) {
            ImGui::Text("Settings placeholder.");
        }
        else if (active_tab == 3) {
            ImGui::Text("VAC AC Helper");
         /*   ImGui::Text("Version 0.1");
            ImGui::Text("Author: Tectonicwave");*/
        }

        ImGui::End();

        // Update cached window size only if window exists
        if (window) {
            g_window_width = current_width;
            g_window_height = current_height;
        }

        // Hover detection on entire Win32 window
        POINT cursor;
        GetCursorPos(&cursor);
        RECT wndRect;
        GetWindowRect(ctx.window, &wndRect);

        bool hovering = cursor.x >= wndRect.left && cursor.x <= wndRect.right &&
            cursor.y >= wndRect.top && cursor.y <= wndRect.bottom;

        static bool last_hovered = false;
        if (hovering != last_hovered) {
            ToggleClickThrough(!hovering);
            last_hovered = hovering;
        }
    }

    // win32

    bool CreateDeviceD3D(HWND hWnd) {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            0, nullptr, 0, D3D11_SDK_VERSION, &sd, &ctx.swap_chain,
            &ctx.device, nullptr, &ctx.context) != S_OK)
            return false;

        ID3D11Texture2D* pBackBuffer = nullptr;
        ctx.swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        ctx.device->CreateRenderTargetView(pBackBuffer, nullptr, &ctx.render_target);
        pBackBuffer->Release();
        return true;
    }

    void CleanupDeviceD3D() {
        if (ctx.render_target) { ctx.render_target->Release(); ctx.render_target = nullptr; }
        if (ctx.swap_chain) { ctx.swap_chain->Release(); ctx.swap_chain = nullptr; }
        if (ctx.context) { ctx.context->Release(); ctx.context = nullptr; }
        if (ctx.device) { ctx.device->Release(); ctx.device = nullptr; }
    }

    void CleanupRenderTarget() {
        if (ctx.render_target) { ctx.render_target->Release(); ctx.render_target = nullptr; }
    }

    void CreateRenderTarget() {
        ID3D11Texture2D* pBackBuffer = nullptr;
        ctx.swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        ctx.device->CreateRenderTargetView(pBackBuffer, nullptr, &ctx.render_target);
        pBackBuffer->Release();
    }

    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        static bool dragging = false;
        static POINT drag_start = { 0, 0 };
        static POINT window_start = { 0, 0 };

        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_DELETE) {
                PostQuitMessage(0);
                return 0;
            }
            break;

        case WM_LBUTTONDOWN:
        {
            ImGuiWindow* window = ImGui::FindWindowByName("VAC3 Anti-Cheat Helper");
            if (window) {
                POINT ptClient = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                // Check if click is inside ImGui window client area
                if (ptClient.x >= window->Pos.x && ptClient.x < window->Pos.x + window->Size.x &&
                    ptClient.y >= window->Pos.y && ptClient.y < window->Pos.y + window->Size.y) {
                    dragging = true;

                    // Capture starting mouse position in screen coords
                    GetCursorPos(&drag_start);

                    // Get current window position in screen coords
                    RECT rc;
                    GetWindowRect(hWnd, &rc);
                    window_start.x = rc.left;
                    window_start.y = rc.top;

                    SetCapture(hWnd);
                    return 0; // handled
                }
            }
            break;
        }

        case WM_MOUSEMOVE:
        {
            if (dragging) {
                POINT pt;
                GetCursorPos(&pt);

                int dx = pt.x - drag_start.x;
                int dy = pt.y - drag_start.y;

                SetWindowPos(hWnd, nullptr,
                    window_start.x + dx,
                    window_start.y + dy,
                    0, 0,
                    SWP_NOZORDER | SWP_NOSIZE);
                return 0;
            }
            break;
        }

        case WM_LBUTTONUP:
        {
            if (dragging) {
                dragging = false;
                ReleaseCapture();
                return 0;
            }
            break;
        }

        case WM_NCHITTEST:
        {
            // Let clicks outside ImGui pass through
            POINT ptScreen = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            POINT ptClient = ptScreen;
            ScreenToClient(hWnd, &ptClient);

            ImGuiWindow* window = ImGui::FindWindowByName("VAC3 Anti-Cheat Helper");
            if (window) {
                ImVec2 pos = window->Pos;
                ImVec2 size = window->Size;

                if (ptClient.x >= pos.x && ptClient.x < pos.x + size.x &&
                    ptClient.y >= pos.y && ptClient.y < pos.y + size.y) {
                    ImGuiIO& io = ImGui::GetIO();
                    if (io.WantCaptureMouse)
                        return HTCLIENT;  // let ImGui handle mouse input
                    else
                        return HTCLIENT;  // treat as client area to avoid default dragging
                }
            }
            return HTTRANSPARENT; // clicks outside ImGui window pass through
        }

        case WM_SIZE:
            if (ctx.device != nullptr && wParam != SIZE_MINIMIZED) {
                CleanupRenderTarget();
                HRESULT hr = ctx.swap_chain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                if (FAILED(hr)) {
                    // handle failure here if needed
                }
                CreateRenderTarget();
            }
            return 0;
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    void run(HINSTANCE hInstance) {
        // Register window class
        ctx.window_class = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
               hInstance, nullptr, nullptr, nullptr, nullptr,
               _T("VAC3Menu"), nullptr };
        RegisterClassEx(&ctx.window_class);

        // Create layered window with no title bar, no borders

        ctx.window = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            ctx.window_class.lpszClassName,
            _T("VAC3 Anti-Cheat Helper"),
            WS_POPUP,
            100, 100, 400, 300,
            nullptr, nullptr, ctx.window_class.hInstance, nullptr);

        SetLayeredWindowAttributes(ctx.window, RGB(0, 0, 0), 0, LWA_COLORKEY);

        if (!CreateDeviceD3D(ctx.window)) {
            CleanupDeviceD3D();
            UnregisterClass(ctx.window_class.lpszClassName, ctx.window_class.hInstance);
            return;
        }

        ShowWindow(ctx.window, SW_SHOWDEFAULT);
        UpdateWindow(ctx.window);

        // Setup ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        // Initialize ImGui Win32 + DX11 implementations
        ImGui_ImplWin32_Init(ctx.window);
        ImGui_ImplDX11_Init(ctx.device, ctx.context);

        bool last_hovered = false;

        // Main loop
        MSG msg;
        while (true) {
            while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                    goto cleanup;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            RenderGui();

            // Detect if mouse is hovering the ImGui window (for toggling click-through)
            ImGuiWindow* window = ImGui::FindWindowByName("VAC3 Anti-Cheat Helper");
            bool hovering = false;
            if (window) {
                POINT cursor;
                GetCursorPos(&cursor);

                RECT wndRect;
                GetWindowRect(ctx.window, &wndRect);

                POINT topLeft = { wndRect.left + (LONG)window->Pos.x, wndRect.top + (LONG)window->Pos.y };
                POINT bottomRight = { topLeft.x + (LONG)window->Size.x, topLeft.y + (LONG)window->Size.y };

                hovering = cursor.x >= topLeft.x && cursor.x <= bottomRight.x &&
                    cursor.y >= topLeft.y && cursor.y <= bottomRight.y;
            }

            if (hovering != last_hovered) {
                ToggleClickThrough(!hovering);
                last_hovered = hovering;
            }

            ImGui::Render();

            ctx.context->OMSetRenderTargets(1, &ctx.render_target, nullptr);
            const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // RGBA with alpha=0 = fully transparent
            ctx.context->ClearRenderTargetView(ctx.render_target, clear_color);

            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            ctx.swap_chain->Present(1, 0);
        }

    cleanup:
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        UnregisterClass(ctx.window_class.lpszClassName, ctx.window_class.hInstance);
    }
}