/** HDMI-CEC TRAY about dialog
    by Chirsoft, 2012
*/

#pragma once


// CectrayAboutDlg dialog

class CectrayAboutDlg : public CDialog
{
	DECLARE_DYNAMIC(CectrayAboutDlg)

// Dialog Data
	enum { IDD = IDD_ABOUT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedStatic2();
	CectrayAboutDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CectrayAboutDlg();
};
