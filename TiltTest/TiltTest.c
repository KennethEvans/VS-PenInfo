
/*----------------------------------------------------------------------------

	NAME
		TiltTest.c

	PURPOSE
		Tests pressure, eraser and tilt output

	AUTHORS
		Based on prstest from LCS Telegraphics  RICO 4/1/92
		Modified by Ken Loftus @ WACOM Technology Corp 2/15/95
		Project file created by Kim Ritchie @ WACOM Technology Corp 8/2004
		Modified by Robert Cohn @ WACOM Technology Corp 2/26/2010

		The text and information contained in this file may be freely used,
		copied, or distributed without compensation or licensing restrictions.

	COPYRIGHT
		Copyright (C) 1998  LCS/Telegraphics
		Copyright (c) Wacom Company, Ltd. 2010 All Rights Reserved
		All rights reserved.

---------------------------------------------------------------------------- */
#define DEBUG 0	/* Use rcInfogen for information rather than default*/
#define explicitlyLinkWinTab 1

#include <windows.h>
#include <math.h>
#include <string.h>

#include "wintab.h"			// NOTE: get from wactab header package
#define PACKETDATA      (PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | \
						 PK_ORIENTATION | PK_CURSOR)
#define PACKETMODE      0
#include "pktdef.h"			// NOTE: get from wactab header package

#include "msgpack.h"
#include "Utils.h"
#include "TiltTest.h"

/* converts FIX32 to double */
#define FIX_DOUBLE(x)   ((double)(INT(x))+((double)FRAC(x)/65536))
#define pi 3.14159265359

#ifdef WIN32                                
#define MoveTo(h,x,y)   MoveToEx(h,x,y,NULL)
#endif

char*	gpszProgramName = "TiltTest";

HANDLE          hInst;               /* Handle for instance */
HCTX            hTab = NULL;         /* Handle for Tablet Context */
POINT           ptNew;               /* XY value storage */
UINT            prsNew;              /* Pressure value storage */
UINT            curNew;              /* Cursor number storage */
ORIENTATION     ortNew;              /* Tilt value storage */
RECT            rcClient;            /* Size of current Client */
RECT            rcVirtual;           /* Size of virtual screen */
RECT            rcInfo1;          /* Size of tilt info box */
RECT            rcInfo2;          /* Size of cursor name box */
RECT            rcInfo3;           /* Size of testing box */
RECT            rcDraw;              /* Size of draw area */
double          aziFactor = 1;       /* Azimuth factor */
double          altFactor = 1;       /* Altitude factor */
double          altAdjust = 1;       /* Altitude zero adjust */
BOOL            tilt_support = TRUE; /* Is tilt supported */

LONG				xMin = 0;			/* Maximum pkX */
LONG				xMax = 0;			/* Maximum pkY */
LONG				yMin = 0;			/* Maximum pkX */
LONG				yMax = 0;			/* Maximum pkY */
int				tabletXMin = 0;		/* Tablet x min size */
int				tabletYMin = 0;		/* Tablet y min size */
int				tabletXMax = 0;		/* Tablet x max size */
int				tabletYMax = 0;		/* Tablet y max size */

/* ------------------------------------------------------------------------- */
int PASCAL WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HINSTANCE  hInstance;
HINSTANCE  hPrevInstance;
LPSTR   lpCmdLine;
int     nCmdShow;
{
	MSG msg;

	if (!hPrevInstance)
		if (!InitApplication(hInstance))
			return (FALSE);

	/* Perform initializations that apply to a specific instance */
	if (!InitInstance(hInstance, nCmdShow))
		return (FALSE);

	/* Acquire and dispatch messages until a WM_QUIT message is received. */
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Return Wintab resources.
	Cleanup();

	return (msg.wParam);
}

/* ------------------------------------------------------------------------- */
BOOL InitApplication(hInstance)
HANDLE  hInstance;
{
	WNDCLASS wc;

	/* Fill in window class structure with parameters that describe the */
	/* main window. */
	wc.style = 0;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
	wc.lpszMenuName =  "TiltTestMenu";
	wc.lpszClassName = "TiltTestWClass";

	/* Register the window class and return success/failure code. */
	return (RegisterClass(&wc));

}

/* ------------------------------------------------------------------------- */
BOOL InitInstance(hInstance, nCmdShow)
HANDLE  hInstance;
int     nCmdShow;
{
	HWND            hWnd;             /* Handle for window */
	HDC             hDC;              /* Handle for Device Context */
	TEXTMETRIC      textmetric;       /* Structure for font info */
	int             nLineH;           /* Holds the text height */
	int             Xinch, Yinch;     /* Holds the number of pixels per inch */
	int             Hres, Vres;       /* Holds the screen resolution */
	char            WName[50];        /* String to hold window name */
	struct          tagAXIS TpOri[3]; /* The capabilities of tilt */
	double          tpvar;            /* A temp for converting fix to double */

	/* Save the instance handle in static variable, which will be used in  */
	/* many subsequence calls from this application to Windows.            */
	hInst = hInstance;

#if explicitlyLinkWinTab
	// Explicitly load wintab32.dll
	if ( !LoadWintab( ) )
	{
		ShowError( "Wintab not available" );
		return FALSE;
	}

	/* check if WinTab available. */
	if (!gpWTInfoA(0, 0, NULL)) {
		MessageBox(NULL, "WinTab Services Not Available.", gpszProgramName, 
			   MB_OK | MB_ICONHAND);
		return FALSE;
	}

	/* check if WACOM available. */
    gpWTInfoA(WTI_DEVICES, DVC_NAME, WName);
    if (strncmp(WName,"WACOM",5)) {
		MessageBox(NULL, "Wacom Tablet Not Installed.", gpszProgramName, 
		    	   MB_OK | MB_ICONHAND);
      return FALSE;
    }

	/* get info about tilt */
	tilt_support = gpWTInfoA(WTI_DEVICES,DVC_ORIENTATION,&TpOri);
	if (tilt_support) {
		/* does the tablet support azimuth and altitude */
		if (TpOri[0].axResolution && TpOri[1].axResolution) {

			/* convert azimuth resulution to double */
			tpvar = FIX_DOUBLE(TpOri[0].axResolution);
			/* convert from resolution to radians */
			aziFactor = tpvar/(2*pi);  
			
			/* convert altitude resolution to double */
			tpvar = FIX_DOUBLE(TpOri[1].axResolution);
			/* scale to arbitrary value to get decent line length */ 
			altFactor = tpvar/1000; 
			 /* adjust for maximum value at vertical */
			altAdjust = (double)TpOri[1].axMax/altFactor;
		}
		else {  /* no so dont do tilt stuff */
			tilt_support = FALSE;
		}
	}
#endif
	/* Create a main window for this application instance.  */
	wsprintf(WName, "TiltTest:%x", hInst);
	hWnd = CreateWindow(
		"TiltTestWClass",
		WName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	/* If window could not be created, return "failure" */
	if (!hWnd)
		return (FALSE);

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
    rcInfo1.left   = Xinch / 8;
    rcInfo1.top    = Yinch / 8;
    rcInfo1.bottom = rcInfo1.top + nLineH;
    rcInfo2 = rcInfo1;
    rcInfo2.top    += nLineH;
    rcInfo2.bottom += nLineH;
    rcInfo3 = rcInfo2;
    rcInfo3.top    += nLineH;
    rcInfo3.bottom += nLineH;
    rcDraw = rcInfo3;
    rcDraw.left   = 0;
    rcDraw.top   += nLineH;
    rcDraw.bottom = rcClient.bottom;
	rcVirtual.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	rcVirtual.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	rcVirtual.right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	rcVirtual.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	/* Make the window visible; update its client area; and return "success" */
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return (TRUE);

}

/* ------------------------------------------------------------------------- */
HCTX static NEAR TabletInit(HWND hWnd)
{
	LOGCONTEXT      lcMine;           /* The context of the tablet */
	AXIS            TabletX, TabletY; /* The maximum tablet size */

	/* get default region */
	gpWTInfoA(WTI_DEFCONTEXT, 0, &lcMine);

	/* modify the digitizing region */
	wsprintf(lcMine.lcName, "TiltTest Digitizing %x", hInst);
	lcMine.lcOptions |= CXO_MESSAGES;
	lcMine.lcPktData = PACKETDATA;
	lcMine.lcPktMode = PACKETMODE;
	lcMine.lcMoveMask = PACKETDATA;
	lcMine.lcBtnUpMask = lcMine.lcBtnDnMask;

    /* Set the entire tablet as active */
	gpWTInfoA(WTI_DEVICES,DVC_X,&TabletX);
	gpWTInfoA(WTI_DEVICES,DVC_Y,&TabletY);
	lcMine.lcInOrgX = 0;
	lcMine.lcInOrgY = 0;
	lcMine.lcInExtX = TabletX.axMax;
	lcMine.lcInExtY = TabletY.axMax;
	tabletXMin = TabletX.axMin;
	tabletYMin = TabletY.axMin;
	tabletXMax = TabletX.axMax;
	tabletYMax = TabletY.axMax;

    /* output the data in screen coords */
//HACK_STUBOUT	lcMine.lcOutOrgX = lcMine.lcOutOrgY = 0;
//HACK_STUBOUT	lcMine.lcOutExtX = GetSystemMetrics(SM_CXSCREEN);
//HACK_STUBOUT    /* move origin to upper left */
//HACK_STUBOUT	lcMine.lcOutExtY = -GetSystemMetrics(SM_CYSCREEN);

	/* open the region */
	return gpWTOpenA(hWnd, &lcMine, TRUE);

}

/* ------------------------------------------------------------------------- */
LRESULT FAR PASCAL MainWndProc(hWnd, message, wParam, lParam)
HWND            hWnd;
unsigned        message;
WPARAM          wParam;
LPARAM          lParam;
{
	FARPROC         lpProcAbout;     /* pointer to the about function */
	HDC             hDC;             /* handle for Device Context */
	PAINTSTRUCT     psPaint;         /* the paint structure */
	PACKET          pkt;             /* the current packet */
	BOOL            fHandled = TRUE; /* whether the message was handled or not */
	LRESULT         lResult = 0L;    /* the result of the message */

	switch (message) {
		case WM_COMMAND: /* Respond to a menu selection */
			if (GET_WM_COMMAND_ID(wParam, lParam) == IDM_ABOUT) {
				lpProcAbout = MakeProcInstance(About, hInst);
				DialogBoxA(hInst, "AboutBox", hWnd, (DLGPROC)lpProcAbout);
				FreeProcInstance(lpProcAbout);
			}
			else
				fHandled = FALSE;
			break;

		case WM_CREATE: /* The window was created so open a context */
			hTab = TabletInit(hWnd);
			if (!hTab) {
				MessageBox(NULL, " Could Not Open Tablet Context.", "WinTab", 
					   MB_OK | MB_ICONHAND);
				SendMessage(hWnd, WM_DESTROY, 0, 0L);
			}
			break;

		case WM_SIZE: /* The window size changed so adjust the output boxes */
			GetClientRect(hWnd, &rcClient);
			rcInfo1.right = rcClient.right;
		    rcInfo2.right = rcClient.right;
		    rcInfo3.right  = rcClient.right;
		    rcDraw.right     = rcClient.right;
		    rcDraw.bottom    = rcClient.bottom;
		    /* redraw the entire window */
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case WM_ACTIVATE: /* The window is activated or deactivated */
			if (GET_WM_ACTIVATE_STATE(wParam, lParam))
				InvalidateRect(hWnd, NULL, TRUE);
			/* if switching in the middle, disable the region */
			if (hTab) {
				gpWTEnable(hTab, GET_WM_ACTIVATE_STATE(wParam, lParam));
				if (hTab && GET_WM_ACTIVATE_STATE(wParam, lParam))
					gpWTOverlap(hTab, TRUE);
			}
			break;

		case WM_DESTROY: /* The window was destroyed */
			if (hTab)
				gpWTClose(hTab);
			PostQuitMessage(0);
			break;

		case WM_PAINT: { /* Paint the window */
			int ZAngle;         /* Raw Altitude */
			UINT Theta;         /* Raw Azimuth */
			UINT Twist;         /* Raw Twist */
			double ZAngle2;     /* Adjusted Altitude */
			double Theta2;      /* Adjusted Azimuth */
			POINT Z1Angle;      /* Rect coords from polar coords */
			char szOutput[256]; /* String for outputs */
			char szLabeledOutput[256];  /* String for out put with a label*/

			if (tilt_support) {                             
				/* 
				   wintab.h defines .orAltitude 
				   as a UINT but documents .orAltitude 
				   as positive for upward angles 
				   and negative for downward angles.
				   WACOM uses negative altitude values to 
				   show that the pen is inverted; 
				   therefore we cast .orAltitude as an 
				   (int) and then use the absolute value. 
				*/
				ZAngle  = (int)ortNew.orAltitude;
				ZAngle2 = altAdjust - (double)abs(ZAngle)/altFactor;
				/* adjust azimuth */
				Theta  = ortNew.orAzimuth;
				Theta2 = (double)Theta/aziFactor;
				/* twist */
				Twist = ortNew.orTwist;
				/* get the position of the diagnal to draw */  
				Z1Angle.x = (int)(ZAngle2*sin(Theta2));
				Z1Angle.y = (int)(ZAngle2*cos(Theta2));
			}
			else {
				Z1Angle.x = 0;
				Z1Angle.y = 0;
			}

			if (hDC = BeginPaint(hWnd, &psPaint)) {
				// rescale
				int x, y;
				UINT prsNewScaled;
				if (tabletXMax != tabletXMin &&
					tabletYMax != tabletYMin) {
					x = ptNew.x * (rcClient.right - rcClient.left) /
						(tabletXMax - tabletXMin);
					// Need to invert y
					y = rcClient.bottom - ptNew.y * (rcClient.bottom - rcClient.top) /
						(tabletYMax - tabletYMin);
					// TEMP
					//x = (rcClient.right + rcClient.left) / 4;
					//y = (rcClient.bottom + rcClient.top) / 4;
					if ((rcClient.right - rcClient.left) >
						(rcClient.bottom - rcClient.top)) {
						prsNewScaled = prsNew *
							(rcClient.bottom - rcClient.top) / 8192;
					} else {
						prsNewScaled = prsNew *
							(rcClient.right - rcClient.left) / 8192;
					}
					// TEMP
					//prsNewScaled = 256;
				} else {
					x = (rcClient.right + rcClient.left) / 2;
					y = (rcClient.bottom + rcClient.top) / 2;
					prsNewScaled = 0;
				}

				
				/* write raw tilt info */ 
				if (tilt_support) {
					wsprintf((LPSTR)szOutput, "Tilt (Altitude): %03i"
						", Theta (Azimuth): %04u"
						", Twist %04u, Pressure: %04u"
						" @ %08u,%08u"
						"\0",
						ZAngle, Theta, Twist, prsNew, ptNew.x, ptNew.y);
				}
				else {
				    strcpy(szOutput,"Tilt not supported.");
				}
				DrawText(hDC,szOutput,strlen(szOutput),&rcInfo1,DT_LEFT);

				/* write current cursor name */ 
				gpWTInfoA(WTI_CURSORS + curNew, CSR_NAME, szOutput);
				wsprintf((LPSTR)szLabeledOutput, "Cursor Name: %s", szOutput);
				DrawText(hDC, szLabeledOutput,strlen(szLabeledOutput),&rcInfo2,DT_LEFT);
				
				/* write tablet name */
				gpWTInfoA(WTI_DEVICES, DVC_NAME, szOutput);
#if DEBUG
				//wsprintf((LPSTR)szLabeledOutput, "prsNew=%d presNewScaled=%d", prsNew, prsNewScaled);
				//wsprintf((LPSTR)szLabeledOutput, "rcVirtual(left,top,right,bottom)=(%d,%d,%d,%d)",
				//	rcVirtual.left, rcVirtual.top, rcVirtual.right, rcVirtual.bottom);
				//wsprintf((LPSTR)szLabeledOutput, "xMin=%ld xMax=%ld yMin=%ld yMax=%ld", xMin, xMax, yMin, yMax);
				wsprintf((LPSTR)szLabeledOutput, "tabletXMin=%d tabletXmax=%d tabletYMin=%d tabletXmax=%d",
					tabletXMin, tabletXMax, tabletYMin, tabletYMax);
#else
				wsprintf((LPSTR)szLabeledOutput, "Tablet Name: %s", szOutput);
#endif
				DrawText(hDC, szLabeledOutput,strlen(szLabeledOutput),&rcInfo3,DT_LEFT);

				/* draw circle based on tablet pressure */
				Ellipse(hDC, x - prsNewScaled, y - prsNewScaled,
					x + prsNewScaled, y + prsNewScaled);

				/* draw a line based on tablet tilt */
				MoveTo(hDC, x, y);
				LineTo(hDC, x + Z1Angle.x, y - Z1Angle.y);

				/* draw CROSS based on tablet position */
				MoveTo(hDC, x - 20, y);
				LineTo(hDC, x + 20, y);
				MoveTo(hDC, x, y - 20);
				LineTo(hDC, x, y + 20);
				EndPaint(hWnd, &psPaint);
			}
			break;
		}

		case WT_PACKET: /* A packet is waiting from WINTAB */
			if (gpWTPacket((HCTX)lParam, wParam, &pkt))
			{
				/* old co-ordinates used for comparisons */
				POINT 		ptOld = ptNew;
				UINT  		prsOld = prsNew;
				UINT  		curOld = curNew;
				ORIENTATION ortOld = ortNew;

				/* save new co-ordinates */
				ptNew.x = (UINT)pkt.pkX;
				ptNew.y = (UINT)pkt.pkY;
				if (pkt.pkX > xMax) xMax = pkt.pkX;
				if (pkt.pkY > yMax) yMax = pkt.pkY;
				if (pkt.pkX < xMin) xMin = pkt.pkX;
				if (pkt.pkY < yMin) yMin = pkt.pkY;

				curNew = pkt.pkCursor;
				prsNew = pkt.pkNormalPressure;
				ortNew = pkt.pkOrientation;

				WacomTrace("pkX: %i, pkY: %i\n", pkt.pkX, pkt.pkY);
				
				/* If the visual changes update the main graphic */
				if (ptNew.x != ptOld.x ||
					ptNew.y != ptOld.y ||
					prsNew != prsOld ||
					ortNew.orAzimuth != ortOld.orAzimuth ||
					ortNew.orAltitude != ortOld.orAltitude ||
					ortNew.orTwist != ortOld.orTwist) {                                     
					InvalidateRect(hWnd, &rcDraw, TRUE);
				}
				/* if the displayed data changes update the text */
				if (ortNew.orAzimuth != ortOld.orAzimuth ||
					ortNew.orAltitude != ortOld.orAltitude ||
					ortNew.orTwist != ortOld.orTwist) {
					InvalidateRect(hWnd, &rcInfo1, TRUE);
#if DEBUG
					InvalidateRect(hWnd, &rcInfoGen, TRUE);
#endif
				}
				/* if the cursor changes update the cursor name */
				if (curNew != curOld) {
					InvalidateRect(hWnd, &rcInfo2, TRUE);
				}
			}
			break;
			
		default:
			fHandled = FALSE;
			break;
	}
	if (fHandled)
		return (lResult);
	else
		return (DefWindowProc(hWnd, message, wParam, lParam));
}

/* -------------------------------------------------------------------------- */
BOOL FAR PASCAL About(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WPARAM wParam;
LPARAM lParam;
{
	switch (message) {
		case WM_INITDIALOG:
			return (TRUE);

		case WM_COMMAND:
			if (GET_WM_COMMAND_ID(wParam, lParam) == IDOK || 
				GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) {
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;
	}
	return (FALSE);
}
/* -------------------------------------------------------------------------- */



//////////////////////////////////////////////////////////////////////////////
// Purpose
//		Release resources we used in this example.
//
void Cleanup( void )
{
	WACOM_TRACE( "Cleanup()\n" );
#if explicitlyLinkWinTab
	UnloadWintab( );
#endif
}
