/** HDMI-CEC TRAY preference dialog
    by Chirsoft, 2012
*/

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CectrayPrefsDlg dialog

class CectrayPrefsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CectrayPrefsDlg)

public:
	CectrayPrefsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CectrayPrefsDlg();
	BOOL OnInitDialog();
	void OnOK();
	bool getHdmiAuto();
	void setHdmiAuto( bool );
	void setForcedHdmiAuto( bool );
	void setHdmiNumber( int );
	int getHdmiNumber();
	void setStandbyPC( bool );
	bool getStandbyPC();
	void setStandbyTV( bool );
	bool getStandbyTV();
	void setAutoStart( bool );
	bool getAutoStart();
	CString getLaunchCommand1();
	CString getLaunchCommand2();
	CString getLaunchCommand3();
	CString getLaunchCommand4();
	void setLaunchCommand1( const char * );
	void setLaunchCommand2( const char * );
	void setLaunchCommand3( const char * );
	void setLaunchCommand4( const char * );
	void getFromHour( int *h, int* m );
	void setFromHour( int h, int m );
	void getUptoHour( int *h, int* m );
	void setUptoHour( int h, int m );
	BOOL PreTranslateMessage(MSG* pMsg);

// Dialog Data
	enum { IDD = IDD_PREFS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void OnBnClickedButton( CEdit *e );
	void refreshComponents();
	BOOL m_isAutomaticHdmiPort;
	CSliderCtrl m_hdmi_number;
	CComboBox m_standby_combobox, m_standbyTV_combobox;
	CEdit m_launch_command_editbox_1, m_launch_command_editbox_2, m_launch_command_editbox_3, m_launch_command_editbox_4;
	CButton m_autoStartCheck;
	CDateTimeCtrl m_fromHour_timebox, m_uptoHour_timebox;
	bool m_forcedHdmiAuto;
	bool m_HdmiAuto;
	int m_HdmiPos;
	bool m_standbyPC;
	bool m_standbyTV;
	bool m_autoStart;
	CString m_launchCommand1;
	CString m_launchCommand2;
	CString m_launchCommand3;
	CString m_launchCommand4;
	int m_fromHour_h, m_fromHour_m;
	int m_uptoHour_h, m_uptoHour_m;
	CToolTipCtrl *m_ToolTip;
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnClickedRadio();
	afx_msg void OnSelchangeCombo();
};