/* Hey, I know this file looks ugly and scary, this simply means you're a human.
   this file basically hides away all the ugly shit you need to do on Windows to create a window
   using Win32 API, i could've used GLFW or SDL2 but i think as for now you just need to try ImGui
   so it fits to it's purpose, i decided to put all of this into a separate file so you can just ignore
   all of this code since it's really unneccesary.

   using Win32 API means you don't need to install anything extra, you can compile this code,
   and everything will run as it should since it only depends on the system libraries.

   most of the code in this file is LITERALLY useless and is abstracted away in GLFW3 or SDL2.

   and yes, WINDOWS API IS SHIT
*/

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>

// Data stored per platform window
struct WGL_WindowData { HDC hDC; };

// Data
static HGLRC            g_hRC;
static WGL_WindowData   g_MainWindow;
static int              g_Width;
static int              g_Height;
static HWND             g_HWND;
static WNDCLASSEXW      g_wc;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data) {
	HDC hDc = ::GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	const int pf = ::ChoosePixelFormat(hDc, &pfd);
	if (pf == 0) return false;
	if (::SetPixelFormat(hDc, pf, &pfd) == FALSE) return false;
	::ReleaseDC(hWnd, hDc);

	data->hDC = ::GetDC(hWnd);
	if (!g_hRC) g_hRC = wglCreateContext(data->hDC);
	return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data) {
	wglMakeCurrent(NULL, NULL);
	::ReleaseDC(hWnd, data->hDC);
}

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			g_Width = LOWORD(lParam);
			g_Height = HIWORD(lParam);
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool Win_Init(const char* title, int x, int y, int width, int height) {
	// Create application window
	ImGui_ImplWin32_EnableDpiAwareness();
	g_wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
	::RegisterClassExW(&wc);
	g_HWND = ::CreateWindowW(wc.lpszClassName, (LPCWSTR)title, WS_OVERLAPPEDWINDOW, x, y, width, height, NULL, NULL, wc.hInstance, NULL);

	// Initialize OpenGL
	if (!CreateDeviceWGL(g_HWND, &g_MainWindow)) {
		CleanupDeviceWGL(g_HWND, &g_MainWindow);
		::DestroyWindow(g_HWND);
		::UnregisterClassW(g_wc.lpszClassName, wc.hInstance);
		return false;
	}
	wglMakeCurrent(g_MainWindow.hDC, g_hRC);

	// Show the window
	::ShowWindow(g_HWND, SW_SHOWDEFAULT);
	::UpdateWindow(g_HWND);

	return true;
}

bool Win_Process() {
	// Poll and handle messages (inputs, window resize, etc.)
	// See the WndProc() function below for our to dispatch events to the Win32 backend.
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT) return true;
	}
}

HWND& Win_GetHWND() {
	return g_HWND;
}

void Win_Update() {
	// Present
	::SwapBuffers(g_MainWindow.hDC);
}

void Win_Release() {
	CleanupDeviceWGL(g_HWND, &g_MainWindow);
	wglDeleteContext(g_hRC);
	::DestroyWindow(g_HWND);
	::UnregisterClassW(g_wc.lpszClassName, g_wc.hInstance);
}

