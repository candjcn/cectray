/** HDMI-CEC TRAY application
    by Chirsoft, 2012
*/

#ifndef TRAY_MENU_DLG_H

#define TRAY_MENU_DLG_H 1

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CectrayMainDlg dialog

#define CDialog CDialogEx

class CectrayMainDlg : public CDialog
{
// Construction
public:
	CectrayMainDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CectrayMainDlg();
	void onHdmiCECLost();				// Call-back when CEC connection is lost.
	void onHdmiCECRestored();			// Call-back when CEC connection returns.

// Dialog Data
	enum { IDD = IDD_TRAYMENU_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAppAbout();
	afx_msg void OnAppExit();
	afx_msg void OnAppOpen();
	afx_msg void OnTrayPreferences();
	afx_msg LRESULT OnTrayNotify(WPARAM wp, LPARAM lp);
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, UINT nEventData);

	DECLARE_MESSAGE_MAP()

	NOTIFYICONDATA	m_nid;			// struct for Shell_NotifyIcon args

	void OnTrayContextMenu();
private:
	HICON m_hIcon, m_hIcon2;
	void LoadPreferences();
	void SavePreferences();
	void setTrayIcon( HICON h, char* s );
	bool startHdmiCec();
	void stopHdmiCec(), closeHdmiCec();
	bool m_hasModal;				// Ergonomy (avoid multiple modal boxes on screen).
	bool m_hasHDMIAuto;				// True if HDMI port must be ignored.
	int m_portHDMI;					// HDMI input
	bool m_hasPCStandby;			// True if the HTPC must go into standby when TV powers off.
	bool m_hasTVStandby;			// True if the TV must go into standby when PC suspends.
	bool m_hasAutoStart;			// True if this app must start when Windows starts.
	std::string m_launchCommand1;	// Program to launch when pressing the blue button.
	std::string m_launchCommand2;	// Program to launch when pressing the red button.
	std::string m_launchCommand3;	// Program to launch when pressing the green button.
	std::string m_launchCommand4;	// Program to launch when pressing the yellow button.
	int m_fromHour_h, m_fromHour_m;
	int m_uptoHour_h, m_uptoHour_m;
	bool m_isCecConnecting;
};

#endif	// TRAY_MENU_DLG_H