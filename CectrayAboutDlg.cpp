/*
 * --- HDMI-CEC TRAY About dialog
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

#include "stdafx.h"
#include "CectrayApp.h"
#include "CectrayAboutDlg.h"
#include "afxdialogex.h"


// CectrayAboutDlg dialog

IMPLEMENT_DYNAMIC(CectrayAboutDlg, CDialog)

CectrayAboutDlg::CectrayAboutDlg(CWnd* pParent /*=NULL*/) : CDialog(CectrayAboutDlg::IDD, pParent)
{
}

CectrayAboutDlg::~CectrayAboutDlg()
{
}

void CectrayAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CectrayAboutDlg, CDialog)
END_MESSAGE_MAP()


// CectrayAboutDlg message handlers
