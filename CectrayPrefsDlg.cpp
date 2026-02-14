/*
 * --- HDMI-CEC TRAY preferences dialog
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
#include "CectrayPrefsDlg.h"
#include "afxdialogex.h"


// CectrayPrefsDlg dialog

IMPLEMENT_DYNAMIC(CectrayPrefsDlg, CDialogEx)

CectrayPrefsDlg::CectrayPrefsDlg(CWnd* pParent /*=NULL*/) : CDialogEx(CectrayPrefsDlg::IDD, pParent)
{
	m_forcedHdmiAuto = FALSE;
}

CectrayPrefsDlg::~CectrayPrefsDlg()
{
	delete m_ToolTip;
}

BOOL CectrayPrefsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_hdmi_number.SetRangeMin( 1, TRUE );  m_hdmi_number.SetRangeMax( 4, TRUE );
	m_hdmi_number.SetPos( m_HdmiPos );
	m_standby_combobox.SetCurSel( (int)m_standbyPC );
	m_standbyTV_combobox.SetCurSel( (int)m_standbyTV );
	m_autoStartCheck.SetCheck( m_autoStart );
	m_launch_command_editbox_1.SetWindowTextA( m_launchCommand1 );
	m_launch_command_editbox_2.SetWindowTextA( m_launchCommand2 );
	m_launch_command_editbox_3.SetWindowTextA( m_launchCommand3 );
	m_launch_command_editbox_4.SetWindowTextA( m_launchCommand4 );
	m_ToolTip = new CToolTipCtrl();  m_ToolTip->Create(this);
	m_ToolTip->SetMaxTipWidth(500);	// Enforce multi-line.
	m_ToolTip->AddTool( &m_launch_command_editbox_1, "Put here the command you want to launch\nwhen the blue key is pressed" );
	m_ToolTip->AddTool( &m_launch_command_editbox_2, "Put here the command you want to launch\nwhen the red key is pressed" );
	m_ToolTip->AddTool( &m_launch_command_editbox_3, "Put here the command you want to launch\nwhen the green key is pressed" );
	m_ToolTip->AddTool( &m_launch_command_editbox_4, "Put here the command you want to launch\nwhen the yellow key is pressed" );
	m_ToolTip->AddTool( &m_fromHour_timebox, "Acceptable time for switching-on TV\nto avoid disturbing people at night" );
	m_ToolTip->AddTool( &m_uptoHour_timebox, "Acceptable time for switching-on TV\nto avoid disturbing people at night" );
	m_ToolTip->AddTool( &m_autoStartCheck, "Start when Windows starts" );
    m_ToolTip->Activate( TRUE );
	m_isAutomaticHdmiPort = ! m_HdmiAuto;
	CTime tt1(1999, 1, 1, m_fromHour_h, m_fromHour_m, 0), tt2(1999, 1, 1, m_uptoHour_h, m_uptoHour_m, 0);
	m_fromHour_timebox.SetTime( &tt1 );  m_uptoHour_timebox.SetTime( &tt2 );
	UpdateData( FALSE );
	refreshComponents();
	return TRUE;
}

BOOL CectrayPrefsDlg::PreTranslateMessage(MSG* pMsg)
{
   if( pMsg->message == WM_LBUTTONDOWN ||  pMsg->message == WM_LBUTTONUP ||  pMsg->message == WM_MOUSEMOVE )
   {
      m_ToolTip->RelayEvent( pMsg );
   }
   return CDialog::PreTranslateMessage( pMsg );
}

void CectrayPrefsDlg::OnOK() 
{
   m_HdmiPos = m_hdmi_number.GetPos();
   m_standbyPC = (m_standby_combobox.GetCurSel()==1);
   m_standbyTV = (m_standbyTV_combobox.GetCurSel()==1);
   m_autoStart = (m_autoStartCheck.GetCheck()==BST_CHECKED);
   m_launch_command_editbox_1.GetWindowText( m_launchCommand1 );
   m_launch_command_editbox_2.GetWindowText( m_launchCommand2 );
   m_launch_command_editbox_3.GetWindowText( m_launchCommand3 );
   m_launch_command_editbox_4.GetWindowText( m_launchCommand4 );
   m_HdmiAuto = (m_isAutomaticHdmiPort==FALSE);

   CTime tt;
   if( m_fromHour_timebox.GetTime(tt) == GDT_VALID )  { m_fromHour_h = tt.GetHour();  m_fromHour_m = tt.GetMinute(); }
   if( m_uptoHour_timebox.GetTime(tt) == GDT_VALID )  { m_uptoHour_h = tt.GetHour();  m_uptoHour_m = tt.GetMinute(); }

   CDialog::OnOK(); // This will close the dialog and DoModal will return.
}

void CectrayPrefsDlg::setHdmiAuto( bool b )
{
	m_HdmiAuto = b;
}

void CectrayPrefsDlg::setForcedHdmiAuto( bool b )
{
	m_forcedHdmiAuto = b;
	if( b )  setHdmiAuto(b);
}

bool CectrayPrefsDlg::getHdmiAuto()
{
	return m_HdmiAuto;
}

void CectrayPrefsDlg::setHdmiNumber( int k )
{
	m_HdmiPos = k;
}

int CectrayPrefsDlg::getHdmiNumber()
{
	return m_HdmiPos;
}

void CectrayPrefsDlg::setStandbyPC( bool b )
{
	m_standbyPC = b;
}

bool CectrayPrefsDlg::getStandbyPC()
{
	return m_standbyPC;
}

void CectrayPrefsDlg::setStandbyTV( bool b )
{
	m_standbyTV = b;
}

bool CectrayPrefsDlg::getStandbyTV()
{
	return m_standbyTV;
}

void CectrayPrefsDlg::setAutoStart( bool b )
{
	m_autoStart = b;
}

bool CectrayPrefsDlg::getAutoStart()
{
	return m_autoStart;
}

CString CectrayPrefsDlg::getLaunchCommand1()
{
	return m_launchCommand1;
}

void CectrayPrefsDlg::setLaunchCommand1( const char *s )
{
	m_launchCommand1 = s;
}

CString CectrayPrefsDlg::getLaunchCommand2()
{
	return m_launchCommand2;
}

void CectrayPrefsDlg::setLaunchCommand2( const char *s )
{
	m_launchCommand2 = s;
}

CString CectrayPrefsDlg::getLaunchCommand3()
{
	return m_launchCommand3;
}

void CectrayPrefsDlg::setLaunchCommand3( const char *s )
{
	m_launchCommand3 = s;
}

CString CectrayPrefsDlg::getLaunchCommand4()
{
	return m_launchCommand4;
}

void CectrayPrefsDlg::setLaunchCommand4( const char *s )
{
	m_launchCommand4 = s;
}

void CectrayPrefsDlg::getFromHour( int *h, int* m )
{
	*h = m_fromHour_h;  *m = m_fromHour_m;
}

void CectrayPrefsDlg::setFromHour( int h, int m )
{
	m_fromHour_h = h;  m_fromHour_m = m;
}

void CectrayPrefsDlg::getUptoHour( int *h, int* m )
{
	*h = m_uptoHour_h;  *m = m_uptoHour_m;
}

void CectrayPrefsDlg::setUptoHour( int h, int m )
{
	m_uptoHour_h = h;  m_uptoHour_m = m;
}

void CectrayPrefsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, m_hdmi_number);
	DDX_Control(pDX, IDC_COMBO1, m_standby_combobox);
	DDX_Control(pDX, IDC_COMBO2, m_standbyTV_combobox);
	DDX_Control(pDX, IDC_EDIT1, m_launch_command_editbox_1);
	DDX_Control(pDX, IDC_EDIT2, m_launch_command_editbox_2);
	DDX_Control(pDX, IDC_EDIT3, m_launch_command_editbox_3);
	DDX_Control(pDX, IDC_EDIT4, m_launch_command_editbox_4);
	DDX_Radio(pDX, IDC_RADIO1, m_isAutomaticHdmiPort);
	DDX_Control(pDX, IDC_CHECK1, m_autoStartCheck);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_fromHour_timebox);
	DDX_Control(pDX, IDC_DATETIMEPICKER2, m_uptoHour_timebox);
}

BEGIN_MESSAGE_MAP(CectrayPrefsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CectrayPrefsDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CectrayPrefsDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CectrayPrefsDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CectrayPrefsDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_RADIO1, &CectrayPrefsDlg::OnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO2, &CectrayPrefsDlg::OnClickedRadio)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CectrayPrefsDlg::OnSelchangeCombo)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CectrayPrefsDlg::OnSelchangeCombo)
END_MESSAGE_MAP()

void CectrayPrefsDlg::refreshComponents()	// private
{
	UpdateData(TRUE);		// Download data from components into DataExchange variables.
	// Enable or disable HDMI Port.
	if( m_forcedHdmiAuto )
	{
		m_hdmi_number.EnableWindow( FALSE );
		m_isAutomaticHdmiPort = 0;
		UpdateData(FALSE);	// Upload data from DataExchange variables into components.
	}
	else
	{
		m_hdmi_number.EnableWindow( m_isAutomaticHdmiPort==1 );
	}
	// Enable or disable hour interval.
	m_fromHour_timebox.EnableWindow( m_standbyTV_combobox.GetCurSel() == 1 );
	m_uptoHour_timebox.EnableWindow( m_standbyTV_combobox.GetCurSel() == 1 );
}

void CectrayPrefsDlg::OnBnClickedButton( CEdit *e )	// private
{
	CFileDialog fileDlg( TRUE, "EXE", "*.EXE", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, "Program Files (*.EXE)|*.EXE|", this );

	if( fileDlg.DoModal() == IDOK )
	{
        CString m_strPathname = fileDlg.GetPathName();
		e->SetWindowText(m_strPathname);
	}
}

// CectrayPrefsDlg message handlers
void CectrayPrefsDlg::OnBnClickedButton1()
{
	OnBnClickedButton( &m_launch_command_editbox_1 );
}

void CectrayPrefsDlg::OnBnClickedButton2()
{
	OnBnClickedButton( &m_launch_command_editbox_2 );
}

void CectrayPrefsDlg::OnBnClickedButton3()
{
	OnBnClickedButton( &m_launch_command_editbox_3 );
}

void CectrayPrefsDlg::OnBnClickedButton4()
{
	OnBnClickedButton( &m_launch_command_editbox_4 );
}

void CectrayPrefsDlg::OnClickedRadio()
{
	refreshComponents();
}

void CectrayPrefsDlg::OnSelchangeCombo()
{
	refreshComponents();
}
