#include "renderer.h"



/*
 DX12 FLOW

 1) Initalise API:
  - init factory, adapter, device, commadn queue, command alloc, command list
 2) frame backings
 - setup swap chain, descriptor heap for back buffer, resource for RTV, fence for frame detection
 3) init resources
 - send graphics data, i.e. a triangle via resource vertex buffers, resource index buffers, fences to detect when uploads are done
 - shader blobs, constant buffer resources, more descriptor heaps, describe resources with root sig, build PSO
 4) encode commands
 - write commands to commandlist, using resource barriers when needed
 5) render - upload constant buffer data (uniforms), submit commands to queue, present, await
 6) destroy - release per frame structures via ::Release() or use Comptrs
*/

/*
	write most of it in 1 func, then extrapolate out
*/


// understand
IDXGISwapChain1* create_swapchain(HWND hwnd, IDXGIFactory4* factory, ID3D12CommandQueue* queue, DXGI_SWAP_CHAIN_DESC1* swapChainDesc,
IDXGIOutput* output=nullptr, DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDesc = nullptr) {
	IDXGISwapChain1* swapchain=nullptr;
	HRESULT result = factory->CreateSwapChainForHwnd(queue, hwnd, swapChainDesc, fullscreenDesc, output, &swapchain);
	if (!FAILED(result)) {
		return swapchain;
	}
	return nullptr;
}

Renderer::Renderer(HWND hwnd, LONG width, LONG height) {
	//m_Hwnd = hwnd;
	// create factory
	ComPtr<IDXGIFactory4> mFactory;
	ComPtr<ID3D12Debug> mDebugController;

	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* dc;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&dc)));
	ThrowIfFailed(dc->QueryInterface(IID_PPV_ARGS(&mDebugController)));
	mDebugController->EnableDebugLayer();

	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	dc->Release();
	dc = nullptr;
#endif

	HRESULT result = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory));

	// create adapter

	// adapter gives info on physical properties of GPU/integrated graphics
	// query gpu name, number, memory, feature level etc
	// need to loop over them and do some shit, etc, for later though. not important now
	//ComPtr<IDXGIAdapter1> m_Adapter; // TODO - udpate this and actually dynamically select an adapter, fail if not supported
	


	// DEVICE
	ComPtr<ID3D12Device> mDevice;

	// passing null just selects the first adapter, which is correct for my system
	ThrowIfFailed(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice)));
	ComPtr<ID3D12DebugDevice> mDebugDevice;
	#if defined(DEBUG) || defined(_DEBUG) 
		ThrowIfFailed(mDevice->QueryInterface(IID_PPV_ARGS(&mDebugDevice)));
	#endif // 


	// COMMAND QUEUE
	// submits draw calls and other commands here

	ComPtr<ID3D12CommandQueue> mCommandQueue;
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; 
	ThrowIfFailed(mDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&mCommandQueue)));

	// COMMAND ALLOCATOR - memory backing for command lists
	ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));

	// COMMAND LIST
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), NULL, IID_PPV_ARGS(&mCommandList)));
	// SYNCHRONIZATION

	//Fence - once gpu has past this point you know the gpu has executed certain commmands, so its safe to touch the memory
	UINT mFrameIndex;
	HANDLE mFenceEvent;
	ComPtr<ID3D12Fence> mFence;
	UINT64 mFenceValue;
	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

	//Barrier - defines how a resource should be used in the upcoming commands
	D3D12_RESOURCE_BARRIER mBarrier{};
	mBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	mBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//mBarrier.Transition.pResource = texResource; // TODO? 
	mBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	mBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	mBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	//mCommandList->ResourceBarrier(1, &mBarrier);

	// SWAPCHAIN - swap backbuffers

	// handles
	constexpr UINT mBackBufferCount = 2;
	UINT mCurrentBufferIndex;
	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12Resource> mRenderTargets[mBackBufferCount];
	UINT mRtvDescriptorSize;

	// swapcxhain
	D3D12_VIEWPORT mViewport; D3D12_RECT mSurfaceRect;
	mSurfaceRect.left = 0;
	mSurfaceRect.top = 0;
	mSurfaceRect.right = width;
	mSurfaceRect.bottom = height;

	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = static_cast<float>(width);
	mViewport.Height = static_cast<float>(height);
	mViewport.MinDepth = 0.1f;
	mViewport.MaxDepth = 1000.f;

	if (mSwapChain != nullptr) {
		// per frame? create render target attachments(?)
		mSwapChain->ResizeBuffers(mBackBufferCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	}
	else {
		// create swap
		DXGI_SWAP_CHAIN_DESC1 mSwapChainDesc{};
		mSwapChainDesc.BufferCount = mBackBufferCount;
		mSwapChainDesc.Width = width;
		mSwapChainDesc.Height = height;
		mSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		mSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		mSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		mSwapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> newSwap = create_swapchain(hwnd, mFactory.Get(), mCommandQueue.Get(),
			&mSwapChainDesc);
		HRESULT swapchainSupported = newSwap->QueryInterface(IID_PPV_ARGS(&newSwap));
		if (SUCCEEDED(swapchainSupported)) {
			newSwap.As(&mSwapChain);
		}
		mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
	}
		// INIT RESOURCES

		// DESCRIPTOR HEAPS - handle memory alloc for heap descriptors




		// describe & create RTV
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
		rtvHeapDesc.NumDescriptors = mBackBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

		mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


		D3D12_CPU_DESCRIPTOR_HANDLE mRtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < mBackBufferCount; n++) {
			ThrowIfFailed(mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mRenderTargets[n])));
			mDevice->CreateRenderTargetView(mRenderTargets[n].Get(), nullptr, mRtvHandle);
			mRtvHandle.ptr += (1 * mRtvDescriptorSize);
		}
	


	// ROOT SIGNATURE - defines what resources shaders can access / resources that are bound to the graphics pipeline



	ComPtr<ID3D12RootSignature> mRootSignature;

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1; // 1.2?

	// TODO - CHECK HERE FOR FALL BACK TO VERSION 1_0
	// TODO - UNDERSTAND ROOT SIGNATURES
	D3D12_DESCRIPTOR_RANGE1 ranges[1];
	ranges[0].BaseShaderRegister = 0;
	ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	ranges[0].NumDescriptors = 1;
	ranges[0].RegisterSpace = 0;
	ranges[0].OffsetInDescriptorsFromTableStart = 0;
	ranges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

	D3D12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = ranges;

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.Desc_1_1.NumParameters = 1;
	rootSignatureDesc.Desc_1_1.pParameters = rootParameters;
	rootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
	rootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;


	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	try
	{
		ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
			signature.GetAddressOf(), error.GetAddressOf()));
		ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)));
		mRootSignature->SetName(L"automata root signature");
	}
	catch (const std::exception&)
	{
		const char* errStr = (const char*)error->GetBufferPointer();
		std::cout << errStr;
		error->Release();
		error = nullptr;
	}
	if (signature)
	{
		signature->Release();
		signature = nullptr;
	}
	// TODO - BINDLESS RESOURCES INSTEAD?

	//HEAPS - chunks of gpu memory for buffers & textures
	return;
}	