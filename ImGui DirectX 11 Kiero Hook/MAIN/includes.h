#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "kiero/kiero.h"
#include "globals.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "../CppSDK/SDK/Engine_classes.hpp"
#include "../CppSDK/SDK/Basic.hpp"
#include "../CppSDK/SDK/Basic.cpp"
#include "../CppSDK/SDK/CoreUObject_functions.cpp"
#include "../CppSDK/SDK/Engine_functions.cpp"

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;