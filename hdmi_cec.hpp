/** HDMI-CEC TRAY low-level calls
    by Chirsoft, 2012
*/

#ifndef HDMI_CEC_HPP

#define HDMI_CEC_HPP 1

class CectrayMainDlg;

extern int open_hdmi_cec_pulse8( CectrayMainDlg*parent, int nPort, bool hasPCStandby, bool hasTVStandby, const char* launchCommand1, const char* launchCommand2, const char* launchCommand3, const char* launchCommand4 );
extern void close_hdmi_cec_pulse8();
extern void poweron_TV_pulse8();
extern void poweroff_TV_pulse8();
extern bool can_hdmi_detect_port_pulse8();

enum HDMI_CECTRAY_ERROR_CODES
{
	HDMI_CECTRAY_OK,
	HDMI_CECTRAY_LIBCEC_ERROR,
	HDMI_CECTRAY_PORT_ERROR,
	HDMI_CECTRAY_OPEN_ERROR
};

#endif	// HDMI_CEC_HPP