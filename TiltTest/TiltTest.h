/*----------------------------------------------------------------------------

	NAME
		TitlTest.h

	PURPOSE
		Tests pressure, eraser and tilt output

	COPYRIGHT
		Copyright (C) 1998  LCS/Telegraphics
		Copyright (c) Wacom Company, Ltd. 2010 All Rights Reserved		
		All rights reserved.

		The text and information contained in this file may be freely used,
		copied, or distributed without compensation or licensing restrictions.

---------------------------------------------------------------------------- */

#pragma once

//////////////////////////////////////////////////////////////////////////////

#define IDM_ABOUT 100

//////////////////////////////////////////////////////////////////////////////

int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
LRESULT FAR PASCAL MainWndProc(HWND, unsigned, WPARAM, LPARAM);
BOOL FAR PASCAL About(HWND, unsigned, WPARAM, LPARAM);
void Cleanup( void );
