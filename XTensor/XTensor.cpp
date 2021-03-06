// DirectX11Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Window.h"
#include "Device.h"
#include "Shader.h"
#include "Buffer.h"
#include "Camera.h"
#include "Renderer.h"
#include <functional>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
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

struct WVP
{
	struct CBPerObject
	{
		DirectX::XMMATRIX wvp;
	} cbPerObject;

	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX world;

	explicit WVP(const Camera& camera)
	{
		view = DirectX::XMMatrixLookAtLH(camera.position, camera.target, camera.up);
		projection = DirectX::XMMatrixPerspectiveFovLH(0.4f * 3.14f,
		                                               static_cast<float>(camera.width) / camera.height,
		                                               1.f, 1000.f);
		world = DirectX::XMMatrixIdentity();
	}

	void UpdateWVPMatrix()
	{
		cbPerObject.wvp = DirectX::XMMatrixTranspose(world * view * projection);
	}
};

struct Cube
{
	explicit Cube(const Device& device, const Camera& camera)
		: m_wvp(camera)
	{
		CreateVertexBuffer(device);
		CreateIndexBuffer(device);
		CreateConstantBuffer(device);
	}

	Cube(const Device& device, const Camera& camera,
	     const std::vector<Vertex>& vertices)
		: m_cubeVertices(vertices), m_cubeIndices({}), m_wvp(camera)
	{
		CreateVertexBuffer(device);
		CreateConstantBuffer(device);
	}

	Cube(const Device& device, const Camera& camera,
	     const std::vector<Vertex>& vertices, const std::vector<UINT32>& indices)
		: m_cubeVertices(vertices), m_cubeIndices(indices), m_wvp(camera)
	{
		CreateVertexBuffer(device);
		CreateIndexBuffer(device);
		CreateConstantBuffer(device);
	}

	virtual ~Cube() = default;

	void Destroy(const ComPtr<ID3D11DeviceContext>& context)
	{
		Buffer::DeleteBuffer(context, m_vertexBuffer);
		Buffer::DeleteBuffer(context, m_indexBuffer);
		Buffer::DeleteBuffer(context, m_constBuffer);
	}

	// TODO: Temporary Update function
	template <class Pred>
	void Update(const Device& device, Pred pred)
	{
		pred(m_wvp);
		Update(device);
	}

	virtual void Update(const Device& device)
	{
		Buffer::BindBuffer(device.GetDeviceContext(), m_indexBuffer);
		Buffer::BindBuffer<Vertex>(device.GetDeviceContext(), m_vertexBuffer);
		const auto cb = Buffer::GetBuffer(m_constBuffer);
		device.GetDeviceContext()->UpdateSubresource(cb.Get(), 0, nullptr, &m_wvp.cbPerObject, 0, 0);
		Buffer::BindBuffer(device.GetDeviceContext(), m_constBuffer);
	}

	void Render(const Renderer& renderer) const
	{
		renderer.GetDeviceContext()->DrawIndexed(m_cubeIndices.size(), 0, 0);
	}

private:
	void CreateVertexBuffer(const Device& device)
	{
		m_vertexBuffer = Buffer::CreateVertexBuffer(device, m_cubeVertices);
	}

	void CreateIndexBuffer(const Device& device)
	{
		m_indexBuffer = Buffer::CreateIndexBuffer(device, m_cubeIndices);
	}

	void CreateConstantBuffer(const Device& device)
	{
		m_constBuffer = Buffer::CreateConstantBuffer(device, sizeof(WVP::CBPerObject));
	}

private:
	std::vector<Vertex> m_cubeVertices = {
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
	std::vector<UINT32> m_cubeIndices = {
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

	BufferId m_vertexBuffer;
	BufferId m_indexBuffer;
	BufferId m_constBuffer;

	WVP m_wvp;
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

	Cube cube1{mainDevice, camera};
	Cube cube2{mainDevice, camera};

	// Create a vertex and pixel shader
	// Binds to Vertex Shader and Pixel Shader stages (resp.)
	auto vertShader = VertexShader::CreateShader(mainDevice, L"VertexShader.hlsl", "vs_5_0", {}, {});
	auto pixShader = PixelShader::CreateShader(mainDevice, L"PixelShader.hlsl", "ps_5_0", {}, {});

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

			renderer.RenderClear(Color{0.f, 0.f, 0.f, 0.f});

			cube1.Update(mainDevice, [&](WVP& wvp)
			{
				const auto rotation = DirectX::XMMatrixRotationAxis(rotaxis, rot);
				const auto translation = DirectX::XMMatrixTranslation(0.f, 0.f, 4.f);
				wvp.world = translation * rotation;
				wvp.UpdateWVPMatrix();
			});
			cube2.Update(mainDevice, [&](WVP& wvp)
			{
				auto rotation = DirectX::XMMatrixRotationAxis(rotaxis, -rot);
				const auto scaling = DirectX::XMMatrixScaling(1.3f, 1.3f, 1.3f);
				wvp.world = rotation * scaling;
				wvp.UpdateWVPMatrix();
			});

			cube1.Render(renderer);
			cube2.Render(renderer);

			// Present back buffer to screen
			mainDevice.GetSwapChain()->Present(0, 0);
		}
	}

	cube1.Destroy(mainDevice.GetDeviceContext());
	cube2.Destroy(mainDevice.GetDeviceContext());

	mainDevice.Release();
	vertShader.Release();
	pixShader.Release();
	rasterState.Reset();
	inputLayout.Reset();
	return message.wParam;
}
