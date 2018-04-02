#pragma once

#include "stdafx.h"

struct Camera
{
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR target;
	DirectX::XMVECTOR up;

	int width = 1280;
	int height = 720;
};
