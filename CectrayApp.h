/** HDMI-CEC TRAY application
    by Chirsoft, 2012
*/

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CectrayApp:
// See TrayMenu.cpp for the implementation of this class
//

class CectrayApp : public CWinAppEx
{
public:
	CectrayApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation
	DECLARE_MESSAGE_MAP()
};

extern CectrayApp theApp;
