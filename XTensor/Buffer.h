#pragma once

#include "stdafx.h"
#include "Device.h"

// TODO: Add Buffer management by ID
using BufferId = size_t;

// TODO: Allow for different usage types on buffers
// https://msdn.microsoft.com/en-us/library/windows/desktop/ff476899(v=vs.85).aspx#Remarks
// TODO: Check for duplicate buffers
struct Buffer
{
	Buffer() = delete;

	template <typename T>
	static BufferId CreateVertexBuffer(const Device& device, const std::vector<T>& vertices,
	                                   UINT stride = sizeof(T), UINT offset = 0)
	{
		ComPtr<ID3D11Buffer> buffer;

		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(T) * vertices.size();
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = vertices.data();

		device.GetDevice()->CreateBuffer(&bufferDesc, &data, buffer.GetAddressOf());
		
		const auto id = GetNextId();
		m_buffers[id] = buffer;
		BindVertexBuffer<T>(device.GetDeviceContext(), id);
		return id;
	}

	static BufferId CreateIndexBuffer(const Device& device, const std::vector<UINT32>& indices, const UINT offset = 0)
	{
		ComPtr<ID3D11Buffer> buffer;

		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(UINT32) * indices.size();
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = indices.data();

		device.GetDevice()->CreateBuffer(&bufferDesc, &data, buffer.GetAddressOf());
		
		const auto id = GetNextId();
		m_buffers[id] = buffer;
		BindIndexBuffer(device.GetDeviceContext(), id);
		return id;
	}

	static BufferId CreateConstantBuffer(const Device& device, const size_t byteWidth)
	{
		ComPtr<ID3D11Buffer> buffer;

		D3D11_BUFFER_DESC cbbd{};
		cbbd.Usage = D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = byteWidth;
		cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		
		device.GetDevice()->CreateBuffer(&cbbd, nullptr, buffer.GetAddressOf());
		
		const auto id = GetNextId();
		m_buffers[id] = buffer;
		BindConstantBuffer(device.GetDeviceContext(), id);
		return id;
	}

	// TODO: Encapsulate all binds/unbinds to 1 function each
	// void ID3D11Buffer::GetDesc(D3D11_BUFFER_DESC*)
	template <typename T>
	static void BindVertexBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id)
	{
		assert(m_buffers.find(id) != m_buffers.end(), "Could not bind vertex buffer: " + std::string(id) +
			". Buffer does not exist.");
		assert(m_vertexBinds.size() <= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, "Can not bind vertex buffer: " +
			std::string(id) + ". Too many buffers bound.");
		if (m_vertexBinds.find(id) == m_vertexBinds.end())
		{
			UINT stride = sizeof(T);
			UINT offset = 0;
			m_vertexBinds[id] = m_buffers[id].Get();
			context->IASetVertexBuffers(m_vertexBinds.size(), 1, m_buffers[id].GetAddressOf(), &stride, &offset);
		}
	}

	static void UnbindVertexBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id)
	{
		const auto iter = m_vertexBinds.find(id);
		if (iter != m_vertexBinds.end())
		{
			const auto slot = std::distance(m_vertexBinds.begin(), iter);
			context->IASetVertexBuffers(slot, 1, nullptr, nullptr, nullptr);
			m_vertexBinds.erase(id);
		}
	}

	static void BindIndexBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id)
	{
		assert(m_buffers.find(id) != m_buffers.end(), "Could not bind index buffer: " + std::string(id) +
			". Buffer does not exist.");
		if (id != m_indexBind)
		{
			m_indexBind = id;
			context->IASetIndexBuffer(m_buffers[id].Get(), DXGI_FORMAT_R32_UINT, 0);
		}
	}

	static void UnbindIndexBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id) throw()
	{
		if (id == m_indexBind)
		{
			context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
			m_indexBind = 0;
		}
	}

	static void BindConstantBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id)
	{
		assert(m_buffers.find(id) != m_buffers.end(), "Could not bind constant buffer: " + std::string(id) +
			". Buffer does not exist.");
		assert(m_constantBinds.size() <= D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, "Can not bind constant buffer: " +
			std::string(id) + ". Too many buffers bound.");
		if (m_constantBinds.find(id) == m_constantBinds.end())
		{
			m_constantBinds[id] = m_buffers[id].Get();
			context->VSSetConstantBuffers(m_constantBinds.size(), 1, m_buffers[id].GetAddressOf());
		}
	}


	static void UnbindConstantBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id)
	{
		const auto iter = m_constantBinds.find(id);
		if (iter != m_constantBinds.end())
		{
			const auto slot = std::distance(m_constantBinds.begin(), iter);
			context->IASetVertexBuffers(slot, 1, nullptr, nullptr, nullptr);
			m_constantBinds.erase(iter);
		}
	}

	// TODO: Make it unbind buffer before deleting
	static void DeleteBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id)
	{
		UnbindVertexBuffer(context, id);
		UnbindConstantBuffer(context, id);
		UnbindIndexBuffer(context, id);

		if (m_buffers.find(id) != m_buffers.end())
		{
			m_buffers[id].Reset();
			m_buffers.erase(id);
		}
	}

	static auto GetBuffer(const BufferId id)
	{
		if (m_buffers.find(id) == m_buffers.end())
			return ComPtr<ID3D11Buffer>{};
		return m_buffers[id];
	}

private:
	static BufferId GetNextId()
	{
		if (m_buffers.find(++m_currentId) != m_buffers.end())
			m_currentId = m_buffers.rbegin()->first + 1;
		return m_currentId;
	}

private:
	/*
	 * TODO: Consider staging buffers:
	 * All of the buffers in m_vertexBinds and m_constantBinds are
	 * D3D11_USAGE_DEFAULT and all other buffers are D3D11_USAGE_STAGING.
	 * Mark each DEFAULT buffer if it's in use (map<ComPtr<ID3D11Buffer>, bool>).
	 * When binding, find the first unused buffer and ID3D11DeviceContext::CopyResource().
	 * When unbinding, mark as unused and clear buffer with CopyResource().
	 * 
	 * REMARK: Staging buffer must have identical dimensions
	 * https://msdn.microsoft.com/en-us/library/windows/desktop/ff476899(v=vs.85).aspx#Remarks 
	 */
	static BufferId m_currentId;
	static std::map<BufferId, ComPtr<ID3D11Buffer>> m_buffers;
	static std::map<BufferId, ID3D11Buffer*> m_vertexBinds;
	static BufferId m_indexBind;
	static std::map<BufferId, ID3D11Buffer*> m_constantBinds;
};

// NEVER HAVE 0 AS A VALUE
BufferId Buffer::m_currentId = 0;
std::map<BufferId, ComPtr<ID3D11Buffer>> Buffer::m_buffers = {};
std::map<BufferId, ID3D11Buffer*> Buffer::m_vertexBinds = {};
std::map<BufferId, ID3D11Buffer*> Buffer::m_constantBinds = {};
BufferId Buffer::m_indexBind = 0;