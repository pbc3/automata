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

	//ComPtr<ID3D12CommandQueue> mCommandQueue;
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; 
	ThrowIfFailed(mDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&mCommandQueue)));

	// COMMAND ALLOCATOR - memory backing for command lists
	//ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));

	//// COMMAND LIST
	//ComPtr<ID3D12GraphicsCommandList> mCommandList;
	//ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), NULL, IID_PPV_ARGS(&mCommandList)));
	// SYNCHRONIZATION

	//Fence - once gpu has past this point you know the gpu has executed certain commmands, so its safe to touch the memory
	//UINT mFrameIndex;
	//HANDLE mFenceEvent;
	//ComPtr<ID3D12Fence> mFence;
	//UINT64 mFenceValue;
	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
	mFenceEvent = CreateEvent(
		nullptr,
		FALSE,
		FALSE,
		nullptr
	);

	if (!mFenceEvent) {
		throw std::runtime_error("Failed to create fence event");
	}
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
	//constexpr UINT mBackBufferCount = 2;
	//UINT mCurrentBufferIndex;
	//ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	//ComPtr<ID3D12Resource> mRenderTargets[mBackBufferCount];
	//UINT mRtvDescriptorSize;

	// swapcxhain
	//D3D12_VIEWPORT mViewport; D3D12_RECT mSurfaceRect;
	mSurfaceRect.left = 0;
	mSurfaceRect.top = 0;
	mSurfaceRect.right = width;
	mSurfaceRect.bottom = height;

	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = static_cast<float>(width);
	mViewport.Height = static_cast<float>(height);
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.f;

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

	//ComPtr<ID3D12RootSignature> mRootSignature;

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
	ComPtr<ID3D12Resource> uploadBuffer;
	std::vector<u8> sourceData;
	D3D12_HEAP_PROPERTIES uploadHeapProps = {
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1u, 1u
	};
	D3D12_RESOURCE_DESC uploadBufferDesc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		65536ull,
		65536ull,
		1u,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		{1u, 0u},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE
	};

	result = mDevice->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
	u8* data = nullptr;
	//D3D12_RANGE range{ 0, SIZE_T(chunkSize) };
	//HRESULT hrStaging = spStaging->Map(0, &range, reinterpret_cast<void**>(&data));
	//if (FAILED(hrStaging)) {
	//	std::cout << "failed to map resource";
	//}
	//if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
	//	memcpy(data, sourceData.data(), sourceData.size());
	//}
	ID3D12Resource* readbackBuffer;
	D3D12_HEAP_PROPERTIES readbackHeapProps =
		{ D3D12_HEAP_TYPE_READBACK,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN, 1u, 1u 
		};

	D3D12_RESOURCE_DESC resourceDescDimBuffer = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		2725888ull,
		1u,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		{1u, 0u},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
	};

	result = mDevice->CreateCommittedResource(
		&readbackHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDescDimBuffer,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, __uuidof(ID3D12Resource),
		((void**)&readbackBuffer));

	// VERTEX BUFFER
	Vertex vertexBufferData[3];
	vertexBufferData[0].pos = {1.0f,-1.0f, 0.0f};
	vertexBufferData[0].colour = { 1.0f,.0f, .0f };

	vertexBufferData[1].pos = { -1.0f,-1.0f, 0.0f };
	vertexBufferData[1].colour = { .0f,1.0f, .0f };

	vertexBufferData[2].pos = { .0f,1.0f, 0.0f };
	vertexBufferData[2].colour = { .0f,.0f, 1.0f };

	//ID3D12Resource* vertexBuffer;
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	constexpr UINT vertexBufSize = sizeof(vertexBufferData);

	D3D12_HEAP_PROPERTIES vertexHeapProps;
	vertexHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	vertexHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	vertexHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	vertexHeapProps.CreationNodeMask = 1;
	vertexHeapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC vertexBufferResourceDesc{};
	vertexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexBufferResourceDesc.Alignment = 0;
	vertexBufferResourceDesc.Width = vertexBufSize;
	vertexBufferResourceDesc.Height = 1;
	vertexBufferResourceDesc.MipLevels = 1;
	vertexBufferResourceDesc.DepthOrArraySize = 1;
	vertexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexBufferResourceDesc.SampleDesc.Count = 1;
	vertexBufferResourceDesc.SampleDesc.Quality = 0;
	vertexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(mDevice->CreateCommittedResource(&vertexHeapProps, D3D12_HEAP_FLAG_NONE, &vertexBufferResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mVertexBuffer)));

	// copy triangle data to vertex buffer
	u8* pVertexDataBegin;

	// not reading from resource on cpu
	D3D12_RANGE readRange; readRange.Begin = 0; readRange.End = 0;
	
	ThrowIfFailed(mVertexBuffer->Map(0, &readRange,
		reinterpret_cast<void**>(&pVertexDataBegin)));

	memcpy(pVertexDataBegin, vertexBufferData, vertexBufSize);
	mVertexBuffer->Unmap(0, nullptr);

	vertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufSize;

	using u32 = uint32_t;
	//INDEX BUFFER - TELL GPU HOW TO READ THE VERTEX BUFFER
	u32 indexBufferData[3] = { 0,1,2 };
	//ComPtr<ID3D12Resource> indexBuffer;
	//D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	const u32 indexBufSize = sizeof(indexBufferData);

	D3D12_HEAP_PROPERTIES indexBufferHeapProps;
	indexBufferHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	indexBufferHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	indexBufferHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	indexBufferHeapProps.CreationNodeMask = 1;
	indexBufferHeapProps.VisibleNodeMask = 1;

	//D3D12_RESOURCE_DESC vertexBufferResourceDesc{};
	//vertexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	//vertexBufferResourceDesc.Alignment = 0;
	//vertexBufferResourceDesc.Width = indexBufSize;
	//vertexBufferResourceDesc.Height = 1;
	//vertexBufferResourceDesc.DepthOrArraySize = 1;
	//vertexBufferResourceDesc.MipLevels = 1;
	//vertexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	//vertexBufferResourceDesc.SampleDesc.Count = 1;
	//vertexBufferResourceDesc.SampleDesc.Quality = 0;
	//vertexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//vertexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(mDevice->CreateCommittedResource(&indexBufferHeapProps, D3D12_HEAP_FLAG_NONE, &vertexBufferResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mIndexBuffer)));

	// copy data
	u8* pIndexBufferBegin;

	D3D12_RANGE iReadRange{ .Begin = 0,.End = 0 };

	ThrowIfFailed(mIndexBuffer->Map(0, &iReadRange, reinterpret_cast<void**>(&pIndexBufferBegin)));
	memcpy(pIndexBufferBegin, indexBufferData, sizeof(indexBufferData));
	mIndexBuffer->Unmap(0, nullptr);

	// index buffer view
	indexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = indexBufSize;

	// CONSTANT BUFFER - describes data we send to shaders when drawing, i.e. MVP matrix, colours etc
	//ComPtr<ID3D12Resource> constantBuffer;
	//ComPtr<ID3D12DescriptorHeap> constantBufferHeap;
	//u8* mappedConstantBuffer;

	D3D12_HEAP_PROPERTIES constantBufferHeapProps;
	constantBufferHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	constantBufferHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	constantBufferHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	constantBufferHeapProps.CreationNodeMask = 1;
	constantBufferHeapProps.VisibleNodeMask = 1;

	D3D12_DESCRIPTOR_HEAP_DESC constantBufferHeapDesc{};
	constantBufferHeapDesc.NumDescriptors = 1;
	constantBufferHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	constantBufferHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(mDevice->CreateDescriptorHeap(&constantBufferHeapDesc,
		IID_PPV_ARGS(&mConstantBufferHeap)));


	//CBVS cbVS;

	D3D12_RESOURCE_DESC constantBufferResourceDesc{};
	constantBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	constantBufferResourceDesc.Alignment = 0;
	constantBufferResourceDesc.Width = (sizeof(cbVS) + 255) & ~255; // TODO ???
	constantBufferResourceDesc.Height = 1;
	constantBufferResourceDesc.DepthOrArraySize = 1;
	constantBufferResourceDesc.MipLevels = 1;
	constantBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	constantBufferResourceDesc.SampleDesc.Count = 1;
	constantBufferResourceDesc.SampleDesc.Quality = 0;
	constantBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	constantBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Alignment = 0;
	cbResourceDesc.Width = (sizeof(cbVS) + 255) & ~255;
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.SampleDesc.Quality = 0;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	cbResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(mDevice->CreateCommittedResource(&constantBufferHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)));
	mConstantBufferHeap->SetName(L"Constant Buffer Upload Resource Heap");

	// CBV
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (sizeof(cbVS) + 255) & ~255; // CB size is required to be 256-byte aligned.

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle(mConstantBufferHeap->GetCPUDescriptorHandleForHeapStart());
	cbvHandle.ptr = cbvHandle.ptr +
		mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 0; // ???

	mDevice->CreateConstantBufferView(&cbvDesc, cbvHandle);

	D3D12_RANGE cReadRange{ 0,0 };
	ThrowIfFailed(constantBuffer->Map(0, &cReadRange, reinterpret_cast<void**>(&mappedConstantBuffer)));
	memcpy(mappedConstantBuffer, &cbVS, sizeof(cbVS));
	constantBuffer->Unmap(0, &readRange);

	// load vert shader bytecode

	D3D12_SHADER_BYTECODE vsByteCode;
	std::string vertPath = R"(C:\dev\automata engine\shaders\triangle_vert.dxil)";
	std::vector<char> vsByteData = readFile(vertPath);
	vsByteCode.pShaderBytecode = vsByteData.data();
	vsByteCode.BytecodeLength = vsByteData.size();


	D3D12_SHADER_BYTECODE fsByteCode;
	std::string fragPath = R"(C:\dev\automata engine\shaders\triangle_frag.dxil)";
	std::vector<char> fsByteData = readFile(fragPath);
	fsByteCode.pShaderBytecode = fsByteData.data();
	fsByteCode.BytecodeLength = fsByteData.size();

	std::cout << "VS size: " << vsByteData.size() << '\n';
	std::cout << "PS size: " << fsByteData.size() << '\n';

	std::cout << "VS magic: "
		<< std::string(vsByteData.data(), 4) << '\n';

	std::cout << "PS magic: "
		<< std::string(fsByteData.data(), 4) << '\n';
	
	// PIPELINE STATE - DESCRIBE EVERYTHING NEEDED TO EXECUTE A RASTER BASED DRAW CALL

	//ComPtr<ID3D12PipelineState> mPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

	// input assembly
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };

	psoDesc.pRootSignature = mRootSignature.Get();

	// if pre compiled shaders, do this, else if some runtime compilation idek set it dynamically
	psoDesc.PS = fsByteCode;
	psoDesc.VS = vsByteCode;

	// raster settings

	D3D12_RASTERIZER_DESC rasterDesc{};
	rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.AntialiasedLineEnable = FALSE;
	rasterDesc.ForcedSampleCount = 0;
	rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	
	psoDesc.RasterizerState = rasterDesc;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// colour / blend
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
		FALSE,
		FALSE,
		D3D12_BLEND_ONE,
		D3D12_BLEND_ZERO,
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE,
		D3D12_BLEND_ZERO,
		D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};

	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;
	psoDesc.BlendState = blendDesc;


	// depth / stencil test
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;

	// output desc
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	// create raster pipeline state
	ThrowIfFailed(mDevice->CreateGraphicsPipelineState(
		&psoDesc,
		IID_PPV_ARGS(&mPipelineState)
	));

	// encoding commands
	


	// COMMAND LIST
	ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCommandAllocator.Get(), mPipelineState.Get(), IID_PPV_ARGS(&mCommandList)));
	ThrowIfFailed(mCommandList->Close());

	return;
}	

void Renderer::setup_commands() {
	ThrowIfFailed(mCommandAllocator->Reset());
	ThrowIfFailed(mCommandList->Reset(
		mCommandAllocator.Get(),
		mPipelineState.Get()
	));

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	ID3D12DescriptorHeap* pDescriptorHeaps[] = {mConstantBufferHeap.Get()};
	mCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);
	D3D12_GPU_DESCRIPTOR_HANDLE cbvHandle(mConstantBufferHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);

	// set back buffer
	D3D12_RESOURCE_BARRIER renderTargetBarrier;
	renderTargetBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	renderTargetBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	renderTargetBarrier.Transition.pResource = mRenderTargets[mFrameIndex].Get();
	renderTargetBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	renderTargetBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	renderTargetBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	mCommandList->ResourceBarrier(1, &renderTargetBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.ptr = rtvHandle.ptr + (mFrameIndex * mRtvDescriptorSize);
	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// actual raster commands
	constexpr Vector4 clearColour = { 0.2f,.2f,.2f,1.0f };
	mCommandList->RSSetViewports(1, &mViewport);
	mCommandList->RSSetScissorRects(1, &mSurfaceRect);
	mCommandList->ClearRenderTargetView(rtvHandle, &clearColour.data[0], 0, nullptr);
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	mCommandList->IASetIndexBuffer(&indexBufferView);

	mCommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);

	// transition to present?
	D3D12_RESOURCE_BARRIER presentBarrier;
	presentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	presentBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	presentBarrier.Transition.pResource = mRenderTargets[mFrameIndex].Get();
	presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	presentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	presentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	mCommandList->ResourceBarrier(1, &presentBarrier);

	ThrowIfFailed(mCommandList->Close());

}
void Renderer::render() {
	// rotate tri

	D3D12_RANGE readRange;
	readRange.Begin = 0; readRange.End = 0;
	ThrowIfFailed(constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedConstantBuffer)));
	memcpy(mappedConstantBuffer, &cbVS, sizeof(cbVS));

	setup_commands();
	ID3D12CommandList * ppCommandLists[] = { mCommandList.Get() };

	mCommandQueue->ExecuteCommandLists(
		_countof(ppCommandLists),
		ppCommandLists
	);

	mSwapChain->Present(1, 0);
	UINT64 fence = mFenceValue;
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fence));
	mFenceValue++;

	if (mFence->GetCompletedValue() < fence) {
		ThrowIfFailed(mFence->SetEventOnCompletion(fence, mFenceEvent));
		WaitForSingleObject(mFenceEvent, INFINITE);
	}
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}

