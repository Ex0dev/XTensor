#pragma once

#include <stdio.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <map>
#include <array>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;

using Color = float[4];