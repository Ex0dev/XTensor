#pragma once

#include "stdafx.h"
#include "Device.h"
#include "Camera.h"

// TODO: Add Buffer management by ID
struct Buffer
{
	Buffer() = default;

	template <typename T>
	static Buffer CreateVertexBuffer(const Device& device, const std::vector<T>& vertices,
	                                 UINT stride = sizeof(T), UINT offset = 0)
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

	// TODO: Change default T from Camera::CBPerObject to something else
	static Buffer CreateConstantBuffer(const Device& device, const size_t byteWidth)
	{
		Buffer buffer{};

		D3D11_BUFFER_DESC cbbd{};
		cbbd.Usage = D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = byteWidth;
		cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		device.GetDevice()->CreateBuffer(&cbbd, nullptr, buffer.m_buffer.GetAddressOf());
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
