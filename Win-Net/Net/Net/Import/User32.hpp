#pragma once
#include "ImportResolver.h"

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