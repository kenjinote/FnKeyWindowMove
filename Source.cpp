#include <windows.h>
#include "resource.h"

#define TRAY_ID 1
#define MYWM_NOTIFYICON (WM_APP+100)

TCHAR szClassName[] = TEXT("FnKeyWindowMove");
HHOOK hHook;
HWND hMainWnd;
HWND hTargetWnd;
POINT pt;
POINT ptOffset;

void AddTaskbarIcon(HWND hWnd, NOTIFYICONDATA* lpTnd)
{
	lpTnd->cbSize = sizeof(NOTIFYICONDATA);
	lpTnd->hWnd = hWnd;
	lpTnd->uID = TRAY_ID;
	lpTnd->uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	lpTnd->uCallbackMessage = MYWM_NOTIFYICON;
	lpTnd->hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1));
	lstrcpy(lpTnd->szTip, szClassName);
	Shell_NotifyIcon(NIM_ADD, lpTnd);
}

void DelTaskbarIcon(HWND hWnd, NOTIFYICONDATA* lpTnd)
{
	lpTnd->cbSize = sizeof(NOTIFYICONDATA);
	lpTnd->hWnd = hWnd;
	lpTnd->uID = TRAY_ID;
	lpTnd->uFlags = 0;
	lpTnd->uCallbackMessage = 0;
	lpTnd->hIcon = 0;
	lpTnd->szTip[0] = 0;
	Shell_NotifyIcon(NIM_DELETE, lpTnd);
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0) {
		return CallNextHookEx(hHook, nCode, wParam, lParam);
	}
	MSLLHOOKSTRUCT* pMS = (MSLLHOOKSTRUCT*)lParam;
	switch (wParam) {
	case WM_LBUTTONDOWN:
		if (GetKeyState(0xFF) < 0) {
			HWND hWnd = GetAncestor(WindowFromPoint(pMS->pt), GA_ROOT);
			if (IsWindow(hWnd)) {
				HMENU hMenu = GetSystemMenu(hWnd, FALSE);
				if (hMenu) {
					MENUITEMINFO info = { sizeof(MENUITEMINFO) };
					info.fMask = MIIM_ID;
					if (GetMenuItemInfo(hMenu, SC_MOVE, FALSE, &info) && !(info.fState & MFS_DISABLED)) {
						RECT rect;
						GetWindowRect(hWnd, &rect);
						ptOffset.x = pMS->pt.x - rect.left;
						ptOffset.y = pMS->pt.y - rect.top;
						hTargetWnd = hWnd;
						return 1;
					}
				}
			}
		}
		hTargetWnd = 0;
		break;
	case WM_MOUSEMOVE:
		if (hTargetWnd) {
			if (IsZoomed(hTargetWnd)) {
				ShowWindow(hTargetWnd, SW_RESTORE);
			}
			pt.x = pMS->pt.x - ptOffset.x;
			pt.y = pMS->pt.y - ptOffset.y;
			SetWindowPos(hTargetWnd, 0, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			InvalidateRect(hMainWnd, 0, TRUE);
		}
		break;
	case WM_LBUTTONUP:
		if (hTargetWnd) {
			hTargetWnd = 0;
			return 1;
		}
		break;
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static NOTIFYICONDATA tnd;
	switch (msg)
	{
	case WM_CREATE:
		hMainWnd = hWnd;
		AddTaskbarIcon(hWnd, &tnd);
		hHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
		break;
	case MYWM_NOTIFYICON:
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
		{
			POINT point;
			GetCursorPos((LPPOINT)&point);
			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, 1, L"終了(&X)");
			TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, point.x, point.y, 0, hWnd, 0);
			DestroyMenu(hMenu);
		}
		break;
		}
		break;
	case WM_COMMAND:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		DelTaskbarIcon(hWnd, &tnd);
		UnhookWindowsHookEx(hHook);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd)
{
	MSG msg;
	WNDCLASS wndclass = {
		0,
		WndProc,
		0,
		0,
		hInstance,
		0,
		0,
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		hInstance,
		0
	);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}