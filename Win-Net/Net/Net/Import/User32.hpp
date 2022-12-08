/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once
#ifndef BUILD_LINUX
#include "ImportResolver.h"

#ifdef NET_DISABLE_IMPORT_USER32
#define User32
#else
#define IMPORT_NAME User32

RESOLVE_IMPORT_BEGIN;
DEFINE_IMPORT(BOOL, SetWindowTextA, const HWND handle, const LPCSTR lpName);
MAKE_IMPORT(handle, lpName);

DEFINE_IMPORT(BOOL, BlockInput, BOOL fBlockIt);
MAKE_IMPORT(fBlockIt);

DEFINE_IMPORT(BOOL, EnumDisplaySettingsA, LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode);
MAKE_IMPORT(lpszDeviceName, iModeNum, lpDevMode);

DEFINE_IMPORT(BOOL, GetMonitorInfoA, HMONITOR hMonitor, LPMONITORINFO lpmi);
MAKE_IMPORT(hMonitor, lpmi);

DEFINE_IMPORT(BOOL, GetMessageA, LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
MAKE_IMPORT(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

DEFINE_IMPORT(BOOL, TranslateMessage, const MSG* lpMsg);
MAKE_IMPORT(lpMsg);

DEFINE_IMPORT(LRESULT, DispatchMessageA, const MSG* lpMsg);
MAKE_IMPORT(lpMsg);

DEFINE_IMPORT(BOOL, IsWindowVisible, HWND hWnd);
MAKE_IMPORT(hWnd);

DEFINE_IMPORT(SHORT, GetKeyState, int nVirtKey);
MAKE_IMPORT(nVirtKey);

DEFINE_IMPORT(BOOL, GetCursorPos, LPPOINT lpPoint);
MAKE_IMPORT(lpPoint);

DEFINE_IMPORT(BOOL, MoveWindow, HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
MAKE_IMPORT(hWnd, X, Y, nWidth, nHeight, bRepaint);

DEFINE_IMPORT(BOOL, GetWindowRect, HWND hWnd, LPRECT lpRect);
MAKE_IMPORT(hWnd, lpRect);

DEFINE_IMPORT(HWND, GetActiveWindow);
MAKE_IMPORT();

DEFINE_IMPORT(BOOL, ShowWindow, HWND hWnd, int nCmdShow);
MAKE_IMPORT(hWnd, nCmdShow);

DEFINE_IMPORT(BOOL, PostMessageA, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
MAKE_IMPORT(hWnd, Msg, wParam, lParam);

DEFINE_IMPORT(LONG, GetWindowLongA, HWND hWnd, int nIndex);
MAKE_IMPORT(hWnd, nIndex);

DEFINE_IMPORT(LONG, SetWindowLongA, HWND hWnd, int nIndex, LONG dwNewLong);
MAKE_IMPORT(hWnd, nIndex, dwNewLong);

DEFINE_IMPORT(int, MessageBoxA, HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
MAKE_IMPORT(hWnd, lpText, lpCaption, uType);

DEFINE_IMPORT(int, MessageBoxW, HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
MAKE_IMPORT(hWnd, lpText, lpCaption, uType);
RESOLVE_IMPORT_END;
#endif
#endif
