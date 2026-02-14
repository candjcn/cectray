/*
 * --- HDMI-CEC TRAY application
 *
 * Copyright (C) 2012 Chirsoft
 *
 * DESCRIPTION: this program provides support for the Pulse-8 HDMI-CEC device in Windows applications.
 *
 * THIS PROGRAM IS FREE SOFTWARE; YOU CAN REDISTRIBUTE IT AND/OR MODIFY
 * IT UNDER THE TERMS OF THE GNU GENERAL PUBLIC LICENSE AS PUBLISHED BY
 * THE FREE SOFTWARE FOUNDATION; EITHER VERSION 2 OF THE LICENSE, OR
 * (AT YOUR OPTION) ANY LATER VERSION.
 *
 * THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
 * BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
 * GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF THE GNU GENERAL PUBLIC LICENSE
 * ALONG WITH THIS PROGRAM; IF NOT, WRITE TO THE FREE SOFTWARE
 * FOUNDATION, INC., 59 TEMPLE PLACE, SUITE 330, BOSTON, MA 02111-1307 USA
 */

#include <string>
#include "stdafx.h"
#include "CectrayApp.h"
#include "CectrayMainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CectrayApp

BEGIN_MESSAGE_MAP(CectrayApp, CWinAppEx)
	ON_COMMAND(ID_HELP, CWinAppEx::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CectrayApp construction

CectrayApp::CectrayApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CectrayApp object

CectrayApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CectrayApp initialization

BOOL CectrayApp::InitInstance()
{
	AfxEnableControlContainer();

	InitContextMenuManager ();

//	CMFCVisualManager::SetDefaultManager (RUNTIME_CLASS (CMFCVisualManagerOffice2003));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	CectrayMainDlg dlg;
	m_pMainWnd = &dlg;

	int nResponse = (int) dlg.DoModal();
	if( nResponse == IDOK )
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if( nResponse == IDCANCEL )
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
