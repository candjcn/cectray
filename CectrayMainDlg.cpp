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
#include "CectrayAboutDlg.h"
#include "CectrayPrefsDlg.h"
#include "hdmi_cec.hpp"
#include "Mmsystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define UM_TRAYNOTIFY	(WM_USER + 1)

#define REG_AUTOSTART_KEY "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REG_CECTRAY_KEY "Software\\Chirsoft\\HDMI CEC Tray"
#define REG_CECTRAY_AUTOPORT_KEY "HDMI AutoPort"
#define REG_CECTRAY_PORT_KEY "HDMI Port"
#define REG_CECTRAY_PCSTANDBY_KEY "Standby with TV"
#define REG_CECTRAY_TVSTANDBY_KEY "Standby with PC"
#define REG_CECTRAY_LAUNCH1_KEY "Launch Command 1"
#define REG_CECTRAY_LAUNCH2_KEY "Launch Command 2"
#define REG_CECTRAY_LAUNCH3_KEY "Launch Command 3"
#define REG_CECTRAY_LAUNCH4_KEY "Launch Command 4"
#define REG_CECTRAY_AUTOSTART_KEY "Autostart CEC Tray"
#define REG_CECTRAY_FROMHOUR_KEY "From hour"
#define REG_CECTRAY_FROMMIN_KEY "From minute"
#define REG_CECTRAY_UPTOHOUR_KEY "Up to hour"
#define REG_CECTRAY_UPTOMIN_KEY "Up to minute"

/////////////////////////////////////////////////////////////////////////////
// CectrayMainDlg dialog

CectrayMainDlg::CectrayMainDlg(CWnd* pParent /*=NULL*/)	: CDialog(CectrayMainDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);  m_hIcon2 = AfxGetApp()->LoadIcon(IDR_MAINFRAME2);		// Note that LoadIcon does not require a subsequent DestroyIcon.

	// Initialize NOTIFYICONDATA
	memset(&m_nid, 0 , sizeof(m_nid));
	m_nid.cbSize = sizeof(m_nid);
	m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;

	m_hasModal = false;				// Ergonomy (avoid multiple modal boxes on screen).
	m_hasHDMIAuto = true;
	m_portHDMI = 1;					// HDMI input.
	m_hasPCStandby = false;			// True if the HTPC must go into standby when TV powers off.
	m_hasTVStandby = false;			// True if the TV must go into standby when PC suspends.
	m_hasAutoStart = true;			// True if this app must start when Windows starts.
	m_launchCommand1 = "C:\\Windows\\notepad.exe";
	m_launchCommand2 = "C:\\Windows\\explorer.exe";
	m_launchCommand3 = "C:\\Windows\\ehome\\ehshell.exe";
	m_launchCommand4 = "C:\\Program Files\\XBMC\\XBMC.exe";
	m_fromHour_h = 8;  m_fromHour_m = 0;
	m_uptoHour_h = 23;  m_uptoHour_m = 59;
	m_isCecConnecting = false;
}

CectrayMainDlg::~CectrayMainDlg()
{
	close_hdmi_cec_pulse8();

	m_nid.hIcon = NULL;
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

void CectrayMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CectrayMainDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_APP_OPEN, OnAppOpen)
	ON_MESSAGE(UM_TRAYNOTIFY, OnTrayNotify)
	ON_COMMAND(ID_TRAY_PREFERENCES, &CectrayMainDlg::OnTrayPreferences)
	ON_WM_POWERBROADCAST()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CectrayMainDlg message handlers

BOOL CectrayMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	m_nid.hWnd = GetSafeHwnd();
	m_nid.uCallbackMessage = UM_TRAYNOTIFY;

	LoadPreferences();
	startHdmiCec();

	return TRUE;
}

void CectrayMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
	if( nID == SC_MONITORPOWER )
	{
		// The screensaver is powering off the screen.
		mciSendString("SET CDAUDIO DOOR CLOSED", NULL, 0, NULL);
		if( m_hasTVStandby )
		{
			poweroff_TV_pulse8();
			stopHdmiCec();
			startHdmiCec();
		}
	}
}

void CectrayMainDlg::OnPaint() 
{
	if( IsIconic() )
	{
		CPaintDC dc(this); // device context for painting
		SendMessage( WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0 );
		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2, y = (rect.Height() - cyIcon + 1) / 2;
		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		ShowWindow( SW_HIDE );
	}
}

HCURSOR CectrayMainDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

LRESULT CectrayMainDlg::OnTrayNotify(WPARAM /*wp*/, LPARAM lp)
{
	UINT uiMsg = (UINT) lp;

	switch (uiMsg)
	{
		case WM_RBUTTONUP:
			OnTrayContextMenu();
			return 1;
		case WM_LBUTTONDBLCLK:
			OnTrayPreferences();
			return 1;
	}

	return 0;
}

void CectrayMainDlg::OnTrayContextMenu()
{
	if( ! m_hasModal )
	{
		CPoint point;
		::GetCursorPos(&point);

		CMenu menu;
		menu.LoadMenu(IDR_MENU1);

		SetForegroundWindow();
		CMenu* popup = menu.GetSubMenu(0);
		popup->SetDefaultItem(0,TRUE);		// Make the first item appear in bold.
		popup->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
        SendMessage(WM_NULL, 0, 0);			// Send benign message to window to make sure the menu goes away.
	}
}

void CectrayMainDlg::OnAppAbout()
{
	if( ! m_hasModal )
	{
		m_hasModal = true;					// Ergonomy (avoid multiple About boxes on screen).
		CectrayAboutDlg d;
		d.DoModal();
		m_hasModal = false;
	}
}

void CectrayMainDlg::OnAppExit() 
{
	OnPowerBroadcast( PBT_APMSUSPEND, 0 );	// Power-off TV.
	closeHdmiCec();
	PostMessage( WM_QUIT );
}

void CectrayMainDlg::OnTrayPreferences()
{
	if( ! m_hasModal )
	{
		m_hasModal = true;					// Ergonomy (avoid multiple About boxes on screen).
		CectrayPrefsDlg d;
		d.setHdmiAuto( m_hasHDMIAuto );  d.setForcedHdmiAuto( can_hdmi_detect_port_pulse8() );
		d.setHdmiNumber( m_portHDMI );
		d.setStandbyPC( m_hasPCStandby );  d.setStandbyTV( m_hasTVStandby );
		d.setAutoStart( m_hasAutoStart );
		d.setLaunchCommand1( m_launchCommand1.c_str() );
		d.setLaunchCommand2( m_launchCommand2.c_str() );
		d.setLaunchCommand3( m_launchCommand3.c_str() );
		d.setLaunchCommand4( m_launchCommand4.c_str() );
		d.setFromHour( m_fromHour_h, m_fromHour_m );  d.setUptoHour( m_uptoHour_h, m_uptoHour_m );
		if( d.DoModal() == IDOK )
		{
			m_hasHDMIAuto = d.getHdmiAuto();
			m_portHDMI = d.getHdmiNumber();
			m_hasPCStandby = d.getStandbyPC();
			m_hasTVStandby = d.getStandbyTV();
			m_hasAutoStart = d.getAutoStart();
			m_launchCommand1 = d.getLaunchCommand1();
			m_launchCommand2 = d.getLaunchCommand2();
			m_launchCommand3 = d.getLaunchCommand3();
			m_launchCommand4 = d.getLaunchCommand4();
			d.getFromHour( &m_fromHour_h, &m_fromHour_m );
			d.getUptoHour( &m_uptoHour_h, &m_uptoHour_m );

			closeHdmiCec();  startHdmiCec();
			SavePreferences();
		}		
		m_hasModal = false;
	}
}

void CectrayMainDlg::OnAppOpen() 
{
}

/** Change tray icon and tooltip */
void CectrayMainDlg::setTrayIcon( HICON h, char* s )
{
	m_nid.hIcon = h;
	CString strToolTip = _T(s);
	_tcsncpy_s( m_nid.szTip, strToolTip, strToolTip.GetLength() );
	Shell_NotifyIcon(NIM_DELETE, &m_nid);  Shell_NotifyIcon(NIM_ADD, &m_nid);
}

bool CectrayMainDlg::startHdmiCec()
{
	if( m_isCecConnecting )  return HDMI_CECTRAY_OK;

	setTrayIcon( m_hIcon2, "HDMI-CEC connecting..." );			// Change tray icon (ergonomy).
	m_isCecConnecting = true;
	int err_code = open_hdmi_cec_pulse8( this, (m_hasHDMIAuto ? -1: m_portHDMI), m_hasPCStandby, m_hasTVStandby, m_launchCommand1.c_str(), m_launchCommand2.c_str(), m_launchCommand3.c_str(), m_launchCommand4.c_str() );
	m_isCecConnecting = false;

	switch( err_code )
	{
		case HDMI_CECTRAY_LIBCEC_ERROR:
			::MessageBox(NULL, _T("Failed to launch LIBCEC.DLL\n\nCheck that the software is correctly installed."), _T("HDMI-CEC Tray"), MB_ICONERROR|MB_OK);
			exit(0);	// !!! Can't load library!
			break;
		case HDMI_CECTRAY_PORT_ERROR:
			::MessageBox(NULL, _T("Failed to find serial port\n\nCheck that the driver is correctly installed."), _T("HDMI-CEC Tray"), MB_ICONERROR|MB_OK);
			exit(0);	// !!! Can't load library!
			break;
		case HDMI_CECTRAY_OPEN_ERROR:
			::MessageBox(NULL, _T("Failed to open the CEC device on specified HDMI input\n\nCheck that the device is connected\nand that another application is not using it."), _T("HDMI-CEC Tray"), MB_ICONERROR|MB_OK);
			setTrayIcon( m_hIcon2, "HDMI-CEC not connected" );	// Change tray icon (ergonomy).
			break;
		case HDMI_CECTRAY_OK:
			setTrayIcon( m_hIcon, "HDMI-CEC connected" );		// Change tray icon (ergonomy).
			break;
		default:
			::MessageBox(NULL, _T("Failed to connect to the CEC device\n\nCheck that the device is connected\nand that another application is not using it."), _T("HDMI-CEC Tray"), MB_ICONERROR|MB_OK);
			setTrayIcon( m_hIcon2, "HDMI-CEC not connected" );	// Change tray icon (ergonomy).
			break;
	}
	return ( err_code == HDMI_CECTRAY_OK );
}

void CectrayMainDlg::stopHdmiCec()
{
	m_isCecConnecting = false;
	setTrayIcon( m_hIcon2, "HDMI-CEC not connected" );			// Change tray icon (ergonomy).
}

void CectrayMainDlg::closeHdmiCec()
{
	stopHdmiCec();
	if( ! m_isCecConnecting )
	{
		m_isCecConnecting = true;
		close_hdmi_cec_pulse8();
		m_isCecConnecting = false;
	}
}

void CectrayMainDlg::onHdmiCECLost()							// Call-back when CEC connection is lost.
{
	m_isCecConnecting = false;
	setTrayIcon( m_hIcon2, "HDMI-CEC not connected" );			// Change tray icon (ergonomy).
}

void CectrayMainDlg::onHdmiCECRestored()						// Call-back when CEC connection returns.
{
	m_isCecConnecting = false;
	setTrayIcon( m_hIcon, "HDMI-CEC connected" );				// Change tray icon (ergonomy).
}

void CectrayMainDlg::LoadPreferences()
{
	// If the app was just installed, give the user the opportunity to set his preferences (ergonomy).
	{
		CRegKey key0;
		if( key0.Open(HKEY_CURRENT_USER,REG_CECTRAY_KEY,KEY_READ) != ERROR_SUCCESS )
		{
			OnTrayPreferences();
		}
	}
	// Read general preferences.
	CRegKey key;
	DWORD dw;
    if( key.Open(HKEY_CURRENT_USER,REG_CECTRAY_KEY,KEY_READ) == ERROR_SUCCESS )
    {
		dw = m_hasHDMIAuto;
	    if( key.QueryDWORDValue( REG_CECTRAY_AUTOPORT_KEY, (DWORD&)dw ) == ERROR_SUCCESS )
		{
			m_hasHDMIAuto = (dw==1);
		}
		dw = m_portHDMI;
	    if( key.QueryDWORDValue( REG_CECTRAY_PORT_KEY, (DWORD&)dw ) == ERROR_SUCCESS )
		{
			m_portHDMI = dw;
		}
		dw = m_hasPCStandby;
	    if( key.QueryDWORDValue( REG_CECTRAY_PCSTANDBY_KEY, (DWORD&)dw ) == ERROR_SUCCESS )
		{
			m_hasPCStandby = (dw==1);
		}
		dw = m_hasTVStandby;
	    if( key.QueryDWORDValue( REG_CECTRAY_TVSTANDBY_KEY, (DWORD&)dw ) == ERROR_SUCCESS )
		{
			m_hasTVStandby = (dw==1);
		}
		char buff[80+1];
		ULONG buffsize;
		buffsize = 80;
		sprintf( buff, "%s", m_launchCommand1.c_str() );
	    if( key.QueryStringValue( REG_CECTRAY_LAUNCH1_KEY, buff, &buffsize ) == ERROR_SUCCESS )
		{
			buff[buffsize] = '\0';
			m_launchCommand1 = buff;
		}
		buffsize = 80;
		sprintf( buff, "%s", m_launchCommand2.c_str() );
	    if( key.QueryStringValue( REG_CECTRAY_LAUNCH2_KEY, buff, &buffsize ) == ERROR_SUCCESS )
		{
			buff[buffsize] = '\0';
			m_launchCommand2 = buff;
		}
		buffsize = 80;
		sprintf( buff, "%s", m_launchCommand3.c_str() );
	    if( key.QueryStringValue( REG_CECTRAY_LAUNCH3_KEY, buff, &buffsize ) == ERROR_SUCCESS )
		{
			buff[buffsize] = '\0';
			m_launchCommand3 = buff;
		}
		buffsize = 80;
		sprintf( buff, "%s", m_launchCommand4.c_str() );
	    if( key.QueryStringValue( REG_CECTRAY_LAUNCH4_KEY, buff, &buffsize ) == ERROR_SUCCESS )
		{
			buff[buffsize] = '\0';
			m_launchCommand4 = buff;
		}
		dw = m_fromHour_h;
	    if( key.QueryDWORDValue( REG_CECTRAY_FROMHOUR_KEY, (DWORD&)dw ) == ERROR_SUCCESS )
		{
			m_fromHour_h = dw;
		}
		dw = m_fromHour_m;
	    if( key.QueryDWORDValue( REG_CECTRAY_FROMMIN_KEY, (DWORD&)dw ) == ERROR_SUCCESS )
		{
			m_fromHour_m = dw;
		}
		dw = m_uptoHour_h;
	    if( key.QueryDWORDValue( REG_CECTRAY_UPTOHOUR_KEY, (DWORD&)dw ) == ERROR_SUCCESS )
		{
			m_uptoHour_h = dw;
		}
		dw = m_uptoHour_m;
	    if( key.QueryDWORDValue( REG_CECTRAY_UPTOMIN_KEY, (DWORD&)dw ) == ERROR_SUCCESS )
		{
			m_uptoHour_m = dw;
		}
    }
	// Read autostart preference.
	CRegKey key2;
    if( key2.Open(HKEY_CURRENT_USER,REG_AUTOSTART_KEY,KEY_READ) == ERROR_SUCCESS )
    {
		char buff[80+1];
		ULONG buffsize = 80;
		m_hasAutoStart = false;
	    if( key.QueryStringValue( REG_CECTRAY_AUTOSTART_KEY, buff, &buffsize ) == ERROR_SUCCESS )
		{
			m_hasAutoStart = (buffsize>0);
		}
	}
}

void CectrayMainDlg::SavePreferences()
{
	// Write general preferences.
	CRegKey key;
    if( key.Create(HKEY_CURRENT_USER, REG_CECTRAY_KEY) == ERROR_SUCCESS )
	{
		key.SetDWORDValue( REG_CECTRAY_AUTOPORT_KEY, m_hasHDMIAuto );
		key.SetDWORDValue( REG_CECTRAY_PORT_KEY, m_portHDMI );
		key.SetDWORDValue( REG_CECTRAY_PCSTANDBY_KEY, m_hasPCStandby );
		key.SetDWORDValue( REG_CECTRAY_TVSTANDBY_KEY, m_hasTVStandby );
		key.SetStringValue( REG_CECTRAY_LAUNCH1_KEY, m_launchCommand1.c_str() );
		key.SetStringValue( REG_CECTRAY_LAUNCH2_KEY, m_launchCommand2.c_str() );
		key.SetStringValue( REG_CECTRAY_LAUNCH3_KEY, m_launchCommand3.c_str() );
		key.SetStringValue( REG_CECTRAY_LAUNCH4_KEY, m_launchCommand4.c_str() );
		key.SetDWORDValue( REG_CECTRAY_FROMHOUR_KEY, m_fromHour_h );
		key.SetDWORDValue( REG_CECTRAY_FROMMIN_KEY, m_fromHour_m );
		key.SetDWORDValue( REG_CECTRAY_UPTOHOUR_KEY, m_uptoHour_h );
		key.SetDWORDValue( REG_CECTRAY_UPTOMIN_KEY, m_uptoHour_m );
	}
	key.Close();
	// Write autostart preference.
	CRegKey key2;
    if( key2.Create(HKEY_CURRENT_USER, REG_AUTOSTART_KEY) == ERROR_SUCCESS )
	{
		if( m_hasAutoStart )
		{	
			key2.SetStringValue( REG_CECTRAY_AUTOSTART_KEY, "\"C:\\Program Files\\HDMI-CEC Tray\\CecTray.exe\"" );
		}
		else
		{
			key2.DeleteValue( REG_CECTRAY_AUTOSTART_KEY );
		}
	}
	key2.Close();
}

UINT CectrayMainDlg::OnPowerBroadcast(UINT nPowerEvent, UINT nEventData)
{
	switch( nPowerEvent )
	{
		case PBT_APMSUSPEND:
			poweroff_TV_pulse8();
			stopHdmiCec();
			mciSendString("SET CDAUDIO DOOR CLOSED", NULL, 0, NULL);
			break;
		case PBT_APMRESUMESUSPEND:  case PBT_APMRESUMECRITICAL:  case PBT_APMRESUMESTANDBY:  case PBTF_APMRESUMEFROMFAILURE:
			CTime tt = CTime::GetCurrentTime();
			int h = tt.GetHour(), m = tt.GetMinute();

			if( (h>m_fromHour_h || (h==m_fromHour_h && m>m_fromHour_m))  &&  (h<m_uptoHour_h || (h==m_uptoHour_h && m<m_uptoHour_m)) )
			{
				closeHdmiCec();  startHdmiCec();
				poweron_TV_pulse8();
			}
			break;
	}
	return CDialog::OnPowerBroadcast(nPowerEvent, nEventData);
}