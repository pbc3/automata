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
#include "automata_math.h"
#include <fstream>
inline std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	bool exists = (bool)file;

	if (!exists || !file.is_open()) {
		throw std::runtime_error("cant open vert shader");
	}
	size_t fileSize = size_t(file.tellg());
	std::vector<char> buffer(fileSize);


	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}


using u8 = uint8_t;

struct CBVS {
	Matrix4D projection = Matrix4D::identity();
	Matrix4D model = Matrix4D::identity() ;
	Matrix4D view = Matrix4D::identity();
};
struct Vec3 {
	float x, y, z;
};
struct Vertex {
	Vec3 pos;
	Vec3 colour;
};

constexpr UINT mBackBufferCount = 2;
inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw std::exception();
	}
};
struct Renderer {
	HWND m_Hwnd;
	Renderer(HWND hwnd, LONG width, LONG height);
	void setup_commands();
	void render();
	ComPtr<ID3D12PipelineState> mPipelineState;
	ComPtr<IDXGISwapChain3> mSwapChain=nullptr;
	ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	ComPtr<ID3D12DescriptorHeap> mConstantBufferHeap;
	ComPtr<ID3D12RootSignature> mRootSignature;

	UINT mCurrentBufferIndex=0;
	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12Resource> mRenderTargets[mBackBufferCount];
	UINT mRtvDescriptorSize;
	UINT mFrameIndex;
	HANDLE mFenceEvent = nullptr;
	ComPtr<ID3D12Fence> mFence;
	UINT64 mFenceValue = 1;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mSurfaceRect;

	ComPtr<ID3D12Resource> mVertexBuffer;
	ComPtr<ID3D12Resource> mIndexBuffer;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};



	ComPtr<ID3D12Resource> constantBuffer;
	u8* mappedConstantBuffer;
	CBVS cbVS;
};