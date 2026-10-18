#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <d3d.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <exception>
#include <wrl.h>
#include <iostream>
using Microsoft::WRL::ComPtr;
#include <vector>
inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw std::exception();
	}
};
struct Renderer {
	HWND m_Hwnd;
	Renderer(HWND hwnd, LONG width, LONG height);
	ComPtr<IDXGISwapChain3> mSwapChain=nullptr;
};