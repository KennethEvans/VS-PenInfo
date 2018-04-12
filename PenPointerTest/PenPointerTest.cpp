// TestTilt2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "PenPointerTest.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
RECT            rcClient;            /* Size of current Client */
RECT            rcVirtual;           /* Size of virtual screen */
RECT            rcInfo1;          /* Size of tilt info box */
RECT            rcInfo2;          /* Size of cursor name box */
RECT            rcInfo3;           /* Size of testing box */
RECT            rcDraw;              /* Size of draw area */

UINT oldPressure, newPressure;
UINT oldRotation, newRotation;
INT32 oldTiltX, newTiltX;
INT32 oldTiltY, newTiltY;
POINT oldLocation, newLocation;
long nPointer = 0;
long nPointerPen = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTTILT2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTTILT2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTTILT2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TESTTILT2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   HDC             hDC;              /* Handle for Device Context */
   TEXTMETRIC      textmetric;       /* Structure for font info */
   int             nLineH;           /* Holds the text height */
   int             Xinch, Yinch;     /* Holds the number of pixels per inch */
   int             Hres, Vres;       /* Holds the screen resolution */

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   /* Get Device Context and setup a rects  to write packet info */
   hDC = GetDC(hWnd);
   if (!hDC)
	   return FALSE;
   GetTextMetrics(hDC, &textmetric);
   nLineH = textmetric.tmExternalLeading + textmetric.tmHeight;
   Xinch = GetDeviceCaps(hDC, LOGPIXELSX);
   Yinch = GetDeviceCaps(hDC, LOGPIXELSY);
   Hres = GetDeviceCaps(hDC, HORZRES);
   Vres = GetDeviceCaps(hDC, VERTRES);
   ReleaseDC(hWnd, hDC);

   GetClientRect(hWnd, &rcClient);
   rcInfo1 = rcClient;
   rcInfo1.left = Xinch / 8;
   rcInfo1.top = Yinch / 8;
   rcInfo1.bottom = rcInfo1.top + nLineH;
   rcInfo2 = rcInfo1;
   rcInfo2.top += nLineH;
   rcInfo2.bottom += nLineH;
   rcInfo3 = rcInfo2;
   rcInfo3.top += nLineH;
   rcInfo3.bottom += nLineH;
   rcDraw = rcInfo3;
   rcDraw.left = 0;
   rcDraw.top += nLineH;
   rcDraw.bottom = rcClient.bottom;
   rcVirtual.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
   rcVirtual.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
   rcVirtual.right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
   rcVirtual.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
   
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	case WM_NCPOINTERUPDATE:
	case WM_NCPOINTERUP:
	case WM_NCPOINTERDOWN:
	{
		nPointer++;
		const WORD wid = GET_POINTERID_WPARAM(wParam);
		POINTER_INFO piTemp = { NULL };
		GetPointerInfo(wid, &piTemp);
		if (piTemp.pointerType == PT_PEN) {
			nPointerPen++;
			UINT32 entries = 0;
			UINT32 pointers = 0;
			UINT32 pointerID = piTemp.pointerId;

			POINTER_PEN_INFO ppi = { NULL };
			BOOL res = GetPointerPenInfo(pointerID, &ppi);
			if (!res) { break; }
			newPressure = ppi.pressure;
			newRotation = ppi.rotation;
			newTiltX = ppi.tiltX;
			newTiltY = ppi.tiltY;
			if (oldPressure != newPressure && oldRotation != newRotation &&
				oldTiltX != newTiltX && oldTiltY != newTiltY &&
				oldLocation.x != newLocation.x &&
				oldLocation.y != newLocation.y) {
				InvalidateRect(hWnd, &rcInfo1, TRUE);
			}
			oldLocation = newLocation;
			oldPressure = newPressure;
			oldRotation = newRotation;
			oldTiltX = newTiltX;
			oldTiltY = newTiltY;

			//GetPointerFramePenInfoHistory(wid, &entries, &pointers, NULL); 
		}
		InvalidateRect(hWnd, &rcInfo3, TRUE);
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);
		TCHAR szOutput[1024]; /* String for outputs */

		wsprintf(szOutput, _T("Pressure=%04u Rotation=%03u TiltX=%03i TiltY=%03i"),
			newPressure, newRotation, newTiltX, newTiltY);
		DrawText(hDC, szOutput, wcslen(szOutput), &rcInfo1, DT_LEFT);
		wsprintf(szOutput, _T("Line 2"));
		DrawText(hDC, szOutput, wcslen(szOutput), &rcInfo2, DT_LEFT);
		wsprintf(szOutput, _T("nPointer=%ld nPointerPen=%ld"),
			nPointer, nPointerPen);
		DrawText(hDC, szOutput, wcslen(szOutput), &rcInfo3, DT_LEFT);

		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_SIZE: /* The window size changed so adjust the output boxes */
		GetClientRect(hWnd, &rcClient);
		rcInfo1.right = rcClient.right;
		rcInfo2.right = rcClient.right;
		rcInfo3.right = rcClient.right;
		rcDraw.right = rcClient.right;
		rcDraw.bottom = rcClient.bottom;
		/* redraw the entire window */
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
