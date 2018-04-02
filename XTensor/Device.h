#pragma once

#include "stdafx.h"

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

	auto GetDevice() const { return m_device; }
	auto GetDeviceContext() const { return m_deviceContext; }
	auto GetSwapChain() const { return m_swapChain; }

	int width;
	int height;

private:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	ComPtr<IDXGISwapChain> m_swapChain;
};
