// DirectX11Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <minwinbase.h>
#include <fstream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct Device
{
	Device(const int width, const int height)
		: width(width), height(height)
	{
	}

	void InitDevice(const HWND window, std::vector<D3D_FEATURE_LEVEL> levels)
	{
		DXGI_MODE_DESC dmd{};
		dmd.Width = width;
		dmd.Height = height;
		dmd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dmd.RefreshRate.Numerator = 60;
		dmd.RefreshRate.Denominator = 1;
		dmd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		dmd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SWAP_CHAIN_DESC scd{};
		scd.BufferCount = 1;
		scd.BufferDesc = dmd;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = window;
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.Windowed = TRUE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		UINT creationFlags = 0;
#ifdef _DEBUG
		creationFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

		// TODO: Create an IDXGISwapChain1 rather than IDXGISwapChain
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
		                              levels.data(), levels.size(), D3D11_SDK_VERSION,
		                              &scd, m_swapChain.GetAddressOf(), m_device.GetAddressOf(), nullptr,
		                              m_deviceContext.GetAddressOf());
	}

	void Release()
	{
		m_device.Reset();
		m_deviceContext.Reset();
		m_swapChain.Reset();
	}

	const auto& GetDevice() const { return m_device; }
	const auto& GetDeviceContext() const { return m_deviceContext; }
	const auto& GetSwapChain() const { return m_swapChain; }

	int width;
	int height;

private:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	ComPtr<IDXGISwapChain> m_swapChain;
};

struct VertexShader
{
	VertexShader() = default;

	static VertexShader CreateShader(const Device& device, std::wstring fileName, std::string version,
	                                       std::vector<D3D_SHADER_MACRO> defines,
	                                       std::vector<ID3DInclude> includes)
	{
		VertexShader shader{};
		D3DCompileFromFile(fileName.c_str(), defines.data(), includes.data(), "main", version.c_str(), 0, 0,
		                   shader.m_shaderBlob.GetAddressOf(), nullptr);
		device.GetDevice()->CreateVertexShader(shader.m_shaderBlob->GetBufferPointer(), shader.m_shaderBlob->GetBufferSize(),
		                                       nullptr,
		                                       shader.m_shader.GetAddressOf());
		device.GetDeviceContext()->VSSetShader(shader.m_shader.Get(), nullptr, 0);
		return shader;
	}

	ComPtr<ID3D11InputLayout> CreateInputLayout(const Device& device,
	                                            const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
	                                            const D3D11_PRIMITIVE_TOPOLOGY topology) const
	{
		ComPtr<ID3D11InputLayout> iaLayout;
		device.GetDevice()->CreateInputLayout(inputElements.data(), inputElements.size(),
		                                      m_shaderBlob->GetBufferPointer(), m_shaderBlob->GetBufferSize(),
		                                      iaLayout.GetAddressOf());
		device.GetDeviceContext()->IASetInputLayout(iaLayout.Get());
		device.GetDeviceContext()->IASetPrimitiveTopology(topology);
		return iaLayout;
	}

	void Release()
	{
		m_shaderBlob.Reset();
		m_shader.Reset();
	}

private:
	ComPtr<ID3DBlob> m_shaderBlob;
	ComPtr<ID3D11VertexShader> m_shader;
};

struct PixelShader
{
	PixelShader() = default;

	static PixelShader CreateShader(const Device& device, std::wstring fileName, std::string version,
	                                     std::vector<D3D_SHADER_MACRO> defines,
	                                     std::vector<ID3DInclude> includes)
	{
		PixelShader shader{};
		D3DCompileFromFile(fileName.c_str(), defines.data(), includes.data(), "main", version.c_str(), 0, 0,
		                   shader.m_shaderBlob.GetAddressOf(), nullptr);
		device.GetDevice()->CreatePixelShader(shader.m_shaderBlob->GetBufferPointer(), shader.m_shaderBlob->GetBufferSize(),
		                                      nullptr,
		                                      shader.m_shader.GetAddressOf());
		device.GetDeviceContext()->PSSetShader(shader.m_shader.Get(), nullptr, 0);
		return shader;
	}

	void Release()
	{
		m_shaderBlob.Reset();
		m_shader.Reset();
	}

private:
	ComPtr<ID3DBlob> m_shaderBlob;
	ComPtr<ID3D11PixelShader> m_shader;
};

struct Camera
{
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR target;
	DirectX::XMVECTOR up;

	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;

	struct CBPerObject
	{
		DirectX::XMMATRIX wvp;
	} cbPerObject;

	void UpdateWVPMatrix(DirectX::XMMATRIX world)
	{
		auto wvp = world * view * projection;
		cbPerObject.wvp = DirectX::XMMatrixTranspose(wvp);
	}
};

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

struct Buffer
{
	Buffer() = default;

	static Buffer CreateVertexBuffer(const Device& device, const std::vector<Vertex>& vertices,
	                                 UINT stride = sizeof(Vertex), UINT offset = 0)
	{
		Buffer buffer{};

		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(Vertex) * vertices.size();
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = vertices.data();

		device.GetDevice()->CreateBuffer(&bufferDesc, &data, buffer.m_buffer.GetAddressOf());
		device.GetDeviceContext()->IASetVertexBuffers(0, 1, buffer.m_buffer.GetAddressOf(), &stride, &offset);
		return buffer;
	}

	static Buffer CreateIndexBuffer(const Device& device, const std::vector<UINT32>& indices, const UINT offset = 0)
	{
		Buffer buffer{};

		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(UINT32) * indices.size();
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = indices.data();

		device.GetDevice()->CreateBuffer(&bufferDesc, &data, buffer.m_buffer.GetAddressOf());
		device.GetDeviceContext()->IASetIndexBuffer(buffer.m_buffer.Get(), DXGI_FORMAT_R32_UINT, offset);
		return buffer;
	}

	template <typename T = Camera::CBPerObject>
	static Buffer CreateConstantBuffer(const Device& device, T* cbPerObj = nullptr)
	{
		Buffer buffer{};

		D3D11_BUFFER_DESC cbbd{};
		cbbd.Usage = D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = sizeof(Camera::CBPerObject);
		cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		device.GetDevice()->CreateBuffer(&cbbd, nullptr, buffer.m_buffer.GetAddressOf());

		if (cbPerObj)
		{
			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = cbPerObj;

			device.GetDevice()->CreateBuffer(&cbbd, &data, buffer.m_buffer.GetAddressOf());
		}

		device.GetDeviceContext()->VSSetConstantBuffers(0, 1, buffer.m_buffer.GetAddressOf());
		return buffer;
	}

	const auto& GetBuffer() const { return m_buffer; }

	void Release()
	{
		m_buffer.Reset();
	}

private:
	ComPtr<ID3D11Buffer> m_buffer;
};

using Color = float[4];

struct Renderer
{
	explicit Renderer(const Device& device)
	{
		CreateRenderTargetView(device);
		CreateDepthStencilView(device);
		m_context = device.GetDeviceContext();
		m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get());
	}

	void CreateRenderTargetView(const Device& device)
	{
		ComPtr<ID3D11Texture2D> backBuffer;
		device.GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		device.GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.GetAddressOf());

		backBuffer.Reset();
	}

	void CreateDepthStencilView(const Device& device)
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = device.width;
		desc.Height = device.height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ComPtr<ID3D11Texture2D> dsb;
		device.GetDevice()->CreateTexture2D(&desc, nullptr, dsb.GetAddressOf());
		device.GetDevice()->CreateDepthStencilView(dsb.Get(), nullptr, m_dsv.GetAddressOf());
		dsb.Reset();
	}

	void RenderClear(Color clearColor) const
	{
		m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);
		m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}

	void Release()
	{
		m_rtv.Reset();
		m_dsv.Reset();
	}

	const auto& GetRenderTargetView() const { return m_rtv; }
	const auto& GetDepthStencilView() const { return m_dsv; }

private:
	ComPtr<ID3D11RenderTargetView> m_rtv;
	ComPtr<ID3D11DepthStencilView> m_dsv;
	ComPtr<ID3D11DeviceContext> m_context;
};

using CubeWorld = DirectX::XMMATRIX;
static CubeWorld g_cube1 = DirectX::XMMatrixIdentity();
static CubeWorld g_cube2 = DirectX::XMMatrixIdentity();

struct Window
{
	Window(const HINSTANCE instance, const int showCmd, const int width, const int height)
		: width(width), height(height), m_device(width, height)
	{
		WNDCLASSEX wndClass;
		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = WindowProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = instance;
		wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
		wndClass.lpszMenuName = nullptr;
		wndClass.lpszClassName = "MainWindowClass";
		wndClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
		RegisterClassEx(&wndClass);

		m_window = CreateWindowEx(0, "MainWindowClass", "DirectX 11", WS_OVERLAPPEDWINDOW,
		                          CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		                          nullptr, nullptr, instance, nullptr);

		m_device.InitDevice(m_window, {D3D_FEATURE_LEVEL_11_1});

		ShowWindow(m_window, showCmd);
		UpdateWindow(m_window);
	}

	void SetLayout() const
	{
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MaxDepth = 1.f;
		m_device.GetDeviceContext()->RSSetViewports(1, &viewport);
	}

	HWND GetWindow() const { return m_window; }
	const Device& GetDevice() const { return m_device; }

	int width;
	int height;

private:
	static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_CLOSE:
			DestroyWindow(window);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(window, message, wParam, lParam);
	}

private:
	HWND m_window;
	Device m_device;
};

static std::vector<Vertex> g_squareVertices = {
	{
		DirectX::XMFLOAT3{-0.5f, -0.5f, 0.5f},
		DirectX::XMFLOAT4{1.f, 0.f, 0.f, 1.f}
	},
	{
		DirectX::XMFLOAT3{-0.5f, 0.5f, 0.5f},
		DirectX::XMFLOAT4{0.f, 1.f, 0.f, 1.f}
	},
	{
		DirectX::XMFLOAT3{0.5f, 0.5f, 0.5f},
		DirectX::XMFLOAT4{0.f, 0.f, 1.f, 1.f}
	},
	{
		DirectX::XMFLOAT3{0.5f, -0.5f, 0.5f},
		DirectX::XMFLOAT4{0.f, 1.f, 0.f, 1.f}
	}
};
static std::vector<UINT32> g_squareIndices = {
	0, 1, 2,
	0, 2, 3
};

static std::vector<Vertex> g_cubeVertices = {
	{
		{-1.f, -1.f, -1.f},
		{1.f, 0.f, 0.f, 1.f}
	},
	{
		{-1.f, 1.f, -1.f},
		{0.f, 1.f, 0.f, 1.f}
	},
	{
		{1.f, 1.f, -1.f},
		{0.f, 0.f, 1.f, 1.f}
	},
	{
		{1.f, -1.f, -1.f},
		{0.f, 1.f, 0.f, 1.f}
	},
	{
		{-1.f, -1.f, 1.f},
		{1.f, 0.f, 0.f, 1.f}
	},
	{
		{-1.f, 1.f, 1.f},
		{0.f, 1.f, 0.f, 1.f}
	},
	{
		{1.f, 1.f, 1.f},
		{0.f, 0.f, 1.f, 1.f}
	},
	{
		{1.f, -1.f, 1.f},
		{0.f, 1.f, 0.f, 1.f}
	},
};
static std::vector<UINT32> g_cubeIndices = {
	// Front face
	0, 1, 2,
	0, 2, 3,
	// Back face
	4, 6, 5,
	4, 7, 6,
	// Left face
	4, 5, 1,
	4, 1, 0,
	// Right face
	3, 2, 6,
	3, 6, 7,
	// Top face
	1, 5, 6,
	1, 6, 2,
	// Bottom face
	4, 0, 3,
	4, 3, 7,
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCmd)
{
	// Create a window with a device, device context, and swapchain
	Window mainWindow(instance, showCmd, 1280, 720);
	auto mainDevice = mainWindow.GetDevice();
	auto device = mainDevice.GetDevice();
	auto deviceContext = mainDevice.GetDeviceContext();
	auto renderer = Renderer{mainDevice};

	// Create a global camera with a WVP constant buffer object
	Camera camera{};
	camera.position = DirectX::XMVectorSet(0.f, 3.f, -8.f, 0.f);
	camera.target = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f);
	camera.up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);

	camera.view = DirectX::XMMatrixLookAtLH(camera.position, camera.target, camera.up);
	camera.projection = DirectX::XMMatrixPerspectiveFovLH(0.4f * 3.14f,
	                                                      static_cast<float>(mainWindow.width) / mainWindow.height,
	                                                      1.f, 1000.f);

	// Create a vertex and pixel shader
	// Binds to Vertex Shader and Pixel Shader stages (resp.)
	auto vertShader = VertexShader::CreateShader(mainDevice, L"VertexShader.hlsl", "vs_5_0", {}, {});
	auto pixShader = PixelShader::CreateShader(mainDevice, L"PixelShader.hlsl", "ps_5_0", {}, {});

	// Create a vertex, index, and constant buffer
	// Binds to Input Assembly, Input Assembly, and Vertex Shader stages (resp.)
	auto vertBuffer = Buffer::CreateVertexBuffer(mainDevice, g_cubeVertices);
	auto indexBuffer = Buffer::CreateIndexBuffer(mainDevice, g_cubeIndices);
	auto constBuffer = Buffer::CreateConstantBuffer(mainDevice);

	// Create a render state
	// Binds to Rasterizer stage
	D3D11_RASTERIZER_DESC renderDesc{};
	renderDesc.FillMode = D3D11_FILL_WIREFRAME;
	renderDesc.CullMode = D3D11_CULL_NONE;
	ComPtr<ID3D11RasterizerState> rasterState;
	device->CreateRasterizerState(&renderDesc, rasterState.GetAddressOf());
	deviceContext->RSSetState(rasterState.Get());

	// Create an input layout
	// Binds to Input Assembly stage
	const std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	auto inputLayout = vertShader.CreateInputLayout(mainDevice, inputElements, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the window's viewport
	// Binds to Rasterizer stage
	mainWindow.SetLayout();

	// Event/Main loop
	MSG message{};
	while (message.message != WM_QUIT)
	{
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else
		{
			static auto rot = 0.f;
			rot += 0.0005f;
			if (rot > 6.28f)
				rot = 0.f;

			const auto rotaxis = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
			auto rotation = DirectX::XMMatrixRotationAxis(rotaxis, rot);
			const auto translation = DirectX::XMMatrixTranslation(0.f, 0.f, 4.f);
			g_cube1 = translation * rotation;

			rotation = DirectX::XMMatrixRotationAxis(rotaxis, -rot);
			const auto scaling = DirectX::XMMatrixScaling(1.3f, 1.3f, 1.3f);
			g_cube2 = rotation * scaling;

			renderer.RenderClear(Color{0.f, 0.f, 0.f, 0.f});

			// Updates WVP matrix and constant buffer data
			// Updates vertex shader stage
			camera.UpdateWVPMatrix(g_cube1);
			deviceContext->UpdateSubresource(constBuffer.GetBuffer().Get(), 0, nullptr, &camera.cbPerObject, 0, 0);
			deviceContext->VSSetConstantBuffers(0, 1, constBuffer.GetBuffer().GetAddressOf());
			deviceContext->DrawIndexed(36, 0, 0);

			camera.UpdateWVPMatrix(g_cube2);
			deviceContext->UpdateSubresource(constBuffer.GetBuffer().Get(), 0, nullptr, &camera.cbPerObject, 0, 0);
			deviceContext->VSSetConstantBuffers(0, 1, constBuffer.GetBuffer().GetAddressOf());
			deviceContext->DrawIndexed(36, 0, 0);

			// Present back buffer to screen
			mainDevice.GetSwapChain()->Present(0, 0);
		}
	}

	mainDevice.Release();
	vertShader.Release();
	pixShader.Release();
	vertBuffer.Release();
	indexBuffer.Release();
	constBuffer.Release();
	rasterState.Reset();
	inputLayout.Reset();
	return message.wParam;
}
