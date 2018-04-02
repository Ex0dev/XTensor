#pragma once

#include "stdafx.h"
#include "Device.h"

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

	auto GetRenderTargetView() const { return m_rtv; }
	auto GetDepthStencilView() const { return m_dsv; }
	auto GetDeviceContext() const { return m_context; }

private:
	ComPtr<ID3D11RenderTargetView> m_rtv;
	ComPtr<ID3D11DepthStencilView> m_dsv;
	ComPtr<ID3D11DeviceContext> m_context;
};
