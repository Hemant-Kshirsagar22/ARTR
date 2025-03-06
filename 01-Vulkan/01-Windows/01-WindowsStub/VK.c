// Windows Header File 
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "vk.h"

// Macros

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Global Function Declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable declarations
FILE *gpFile = NULL;
DWORD dwStyle = 0;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
BOOL gbFullScreen = FALSE;

HWND ghwnd;
BOOL gbActive = FALSE;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function declarations
	
	int initialize(void);
	void uninitialize(void);
	void display(void);
	void update(void);

	// Local Variable Declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("HGKWindow");

	int iResult = 0;

	BOOL bDone = FALSE;

	int SW, SH, WW, WH;

	// code
	gpFile = fopen("LOG.txt", "w");
	if (gpFile == NULL)
	{
		MessageBox(NULL, TEXT("LOG File Cant Create !!!"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
		exit(0);
	}

	fprintf(gpFile, "initialize()->Program Started Successfully !!!\n\n");

	// WNDCLASSEX initilization
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// Register WNDLCASSEX
	RegisterClassEx(&wndclass);

	// Create Window

	SW = GetSystemMetrics(SM_CXSCREEN);
	SH = GetSystemMetrics(SM_CYSCREEN);

	SW = SW / 2;
	SH = SH / 2;
	WW = WIN_WIDTH / 2;
	WH = WIN_HEIGHT / 2;

	WW = SW - WW;
	WH = SH - WH;
	
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("HEMANT GANESH KSHIRSAGAR : Vulkan"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		WW,
		WH,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	// initialization
	
	iResult = initialize();
	if (iResult != 0)
	{
		MessageBox(hwnd, TEXT("initialize() Failed !!!"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
		DestroyWindow(hwnd);
	}
	// Show The Window
	ShowWindow(hwnd, iCmdShow);

	SetForegroundWindow(hwnd);

	SetFocus(hwnd);
		
	// GameLoop

	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActive == TRUE)
			{
				// render
				display();

				// update
				update();
			}
		}
	}

	// uninitialization
	uninitialize();

	return((int)msg.wParam);

}

//CALLBACK Function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// Function Prototype / Declarations

	void ToggleFullScreen(void);

	void resize(int, int);

	// code

	switch (iMsg)
	{
	case WM_SETFOCUS: // When Window is in focus (Window is Active)
		gbActive = TRUE;
		break;

	case WM_KILLFOCUS: // When Window Is Not In Focus (Window is deactiveated)
		gbActive = FALSE;
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

//	case WM_ERASEBKGND:
//		return(0);

	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		default:
			break;
		}
		break;

	case WM_CHAR:
		switch (LOWORD(wParam))
		{
		case 'F':
		case 'f':
			if (gbFullScreen == FALSE)
			{
				ToggleFullScreen();
				gbFullScreen = TRUE;
			}
			else
			{
				ToggleFullScreen();
				gbFullScreen = FALSE;
			}
			break;
		default:
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
	// Local Variable declarations
	MONITORINFO mi = { sizeof(MONITORINFO) };

	// code
	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
	}
	else
	{
		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}

int initialize(void)
{
	// Function Declarations

	// code

	return(0);
}

void resize(int width, int height)
{
	// code

	if (height <= 0)
	{
		height = 1;
	}
}

void display(void)
{
	// code
}

void update(void)
{
	// code
}

void uninitialize(void)
{
	// Function Declarations

	void ToggleFullScreen(void);

	// code
	// if Application is exitting in fullscreen
	if (gbFullScreen == TRUE)
	{
		ToggleFullScreen();
		gbFullScreen = FALSE;
	}

	// Destroy Window
	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}

	// close the log file
	fprintf(gpFile, "uninitialize()->Program Ended Successfully !!!\n\n");
	if (gpFile)
	{
		fclose(gpFile);
		gpFile = NULL;
	}
}
