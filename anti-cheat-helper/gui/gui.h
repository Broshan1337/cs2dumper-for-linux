#pragma once
#include <windows.h>
#include <d3d11.h>
#include <atomic>
#include <mutex>

namespace gui {
	void run(HINSTANCE hInstance);

	struct Win32Context {
		// Device / D3D globals
		inline static ID3D11Device* device = nullptr;
		inline static ID3D11DeviceContext* context = nullptr;
		inline static IDXGISwapChain* swap_chain = nullptr;
		inline static ID3D11RenderTargetView* render_target = nullptr;

		// Win32 window globals
		inline static HWND window = nullptr;
		inline static WNDCLASSEX window_class = {};
	};

}// namespace gui