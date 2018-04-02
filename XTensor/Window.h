#pragma once

#include "stdafx.h"
#include "Device.h"

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
