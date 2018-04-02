#pragma once

#include "Shader.h"
#include "Device.h"

// TODO: Add Shader management by ID
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
