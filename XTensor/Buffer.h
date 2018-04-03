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

		const auto id = m_nextId;
		m_buffers[id] = buffer;
		m_nextId = GetNextId();
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

		const auto id = m_nextId;
		m_buffers[id] = buffer;
		m_nextId = GetNextId();
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

		const auto id = m_nextId;
		m_buffers[id] = buffer;
		m_nextId = GetNextId();
		return id;
	}

	// TODO: Encapsulate all binds/unbinds to 1 function each
	// void ID3D11Buffer::GetDesc(D3D11_BUFFER_DESC*)
	template <typename T>
	static void BindBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id)
	{
		if (m_buffers.find(id) != m_buffers.end())
		{
			D3D11_BUFFER_DESC desc{};
			m_buffers[id]->GetDesc(&desc);
			if (desc.BindFlags == D3D11_BIND_VERTEX_BUFFER)
			{
				const auto iter = std::find(m_vertexBinds.begin(), m_vertexBinds.end(), id);
				if (iter == m_vertexBinds.end())
				{
					static UINT stride = sizeof(T);
					static UINT offset = 0;
					context->IASetVertexBuffers(m_vertexBinds.size(), 1, m_buffers[id].GetAddressOf(), &stride, &offset);
					m_vertexBinds.push_back(id);
				}
			}
			else
			{
				BindBuffer(context, id);
			}
		}
	}

	static void BindBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id)
	{
		if (m_buffers.find(id) != m_buffers.end())
		{
			D3D11_BUFFER_DESC desc{};
			m_buffers[id]->GetDesc(&desc);
			if (desc.BindFlags == D3D11_BIND_VERTEX_BUFFER)
			{
				assert(false, "Need to specify per vertex type.");
			}
			else if (desc.BindFlags == D3D11_BIND_INDEX_BUFFER)
			{
				if (m_indexBind != id)
				{
					m_indexBind = id;
					context->IASetIndexBuffer(m_buffers[id].Get(), DXGI_FORMAT_R32_UINT, 0);
				}
			}
			else if (desc.BindFlags == D3D11_BIND_CONSTANT_BUFFER)
			{
				const auto iter = std::find(m_constantBinds.begin(), m_constantBinds.end(), id);
				if (iter == m_constantBinds.end())
				{
					context->VSSetConstantBuffers(m_constantBinds.size(), 1, m_buffers[id].GetAddressOf());
					m_constantBinds.push_back(id);
				}
			}
			else
			{
				assert(false, "Invalid buffer.");
			}
		}
	}

	static void UnbindBuffer(const ComPtr<ID3D11DeviceContext>& context, const BufferId id)
	{
		if (m_buffers.find(id) == m_buffers.end())
		{
			D3D11_BUFFER_DESC desc{};
			m_buffers[id]->GetDesc(&desc);
			if (desc.BindFlags == D3D11_BIND_VERTEX_BUFFER)
			{
				const auto iter = std::find(m_constantBinds.begin(), m_constantBinds.end(), id);
				if (iter != m_vertexBinds.end())
				{
					const auto slot = std::distance(m_constantBinds.begin(), iter);
					// TODO: Figure out how to unbind vertex buffers efficiently
					m_vertexBinds.erase(iter);
				}
			}
			else if (desc.BindFlags == D3D11_BIND_INDEX_BUFFER)
			{
				if (m_indexBind == id)
				{
					m_indexBind = 0;
					context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
				}
			}
			else if (desc.BindFlags == D3D11_BIND_CONSTANT_BUFFER)
			{
				const auto iter = std::find(m_constantBinds.begin(), m_constantBinds.end(), id);
				if (iter != m_constantBinds.end())
				{
					const auto slot = std::distance(m_constantBinds.begin(), iter);
					context->VSSetConstantBuffers(slot, 1, nullptr);
					m_constantBinds.erase(iter);
				}
			}
			else
			{
				assert(false, "Invalid buffer.");
			}
		}
	}

	// TODO: Make it unbind buffer before deleting
	static void DeleteBuffer(const ComPtr<ID3D11DeviceContext>& context, BufferId& id)
	{
		UnbindBuffer(context, id);

		if (m_buffers.find(id) != m_buffers.end())
		{
			m_buffers[id].Reset();
			m_buffers.erase(id);

			if (m_buffers.find(m_nextId) != m_buffers.end() ||
				m_nextId > id)
				m_nextId = id;
			id = 0;
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
		auto id = m_nextId + 1;
		while (m_buffers.find(id) != m_buffers.end())
			++id;
		return id;
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
	static BufferId m_nextId;
	static std::map<BufferId, ComPtr<ID3D11Buffer>> m_buffers;
	static std::vector<BufferId> m_vertexBinds;
	static std::vector<BufferId> m_constantBinds;
	static BufferId m_indexBind;
};

// NEVER HAVE 0 AS A VALUE
BufferId Buffer::m_nextId = 1;
std::map<BufferId, ComPtr<ID3D11Buffer>> Buffer::m_buffers = {};
std::vector<BufferId> Buffer::m_vertexBinds = {};
std::vector<BufferId> Buffer::m_constantBinds = {};
BufferId Buffer::m_indexBind = 0;
