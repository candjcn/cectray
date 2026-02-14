/*
 * --- HDMI-CEC TRAY low-level calls
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
#include <PowrProf.h> 

#undef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#undef _MFC_VER

#include "CectrayApp.h"
#include "CectrayMainDlg.h"
using namespace std;
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include "../lib/platform/os.h"
#include "../lib/implementations/CECCommandHandler.h"
#include "../../include/cec.h"
using namespace CEC;
#include "../../include/cecloader.h"
using namespace PLATFORM;
#include "hdmi_cec.hpp"


ICECAdapter *m_parser = NULL;
ICECCallbacks g_callbacks;
libcec_configuration g_config;
std::string g_strPort;
bool m_lastReleased = FALSE;		// To avoid autorepeat.
bool m_hasPCStandby = FALSE;		// True if PC must suspend after TV powers down.
bool m_hasTVStandby = FALSE;		// True if the TV must go into standby when PC suspends.
std::string m_launchCommand1 = "";	// Program to launch when pressing the blue button.
std::string m_launchCommand2 = "";	// Program to launch when pressing the red button.
std::string m_launchCommand3 = "";	// Program to launch when pressing the green button.
std::string m_launchCommand4 = "";	// Program to launch when pressing the yelow button.
CectrayMainDlg*	m_parent = NULL;

std::map<cec_user_control_code,int> m_hdmiKeymap, m_hdmiMediaportalKeymap, m_hdmiCtrlKeymap, m_hdmiMediaportalCtrlKeymap;

void CBCecSourceActivated( void *param, const cec_logical_address, const uint8_t bActivated );
void stop_mce_playback_pulse8();

// -------------- Win32 calls --------------

void send_key_win32( WPARAM c, bool is_ctrl )
{
    vector<INPUT> EventQueue;
	INPUT ev;

	if( c == 0 )  return;
	if( ! is_ctrl )
	{
        ::ZeroMemory( &ev, sizeof( ev ));
		ev.type = INPUT_KEYBOARD;
 		ev.ki.wVk = (WORD)c;
		ev.ki.dwFlags = 0;
		ev.ki.time = 0;
		ev.ki.dwExtraInfo = 0;
	    EventQueue.push_back( ev );
 
        ::ZeroMemory( &ev, sizeof( ev ));
		ev.type = INPUT_KEYBOARD;
		ev.ki.wVk = (WORD)c;
		ev.ki.dwFlags = KEYEVENTF_KEYUP;
		ev.ki.time = 0;
		ev.ki.dwExtraInfo = 0;
	    EventQueue.push_back( ev );
	}
	else
	{
 	  char Buff[120] = { 0 };
	  GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_ILANGUAGE, Buff, sizeof(Buff) );
	  HKL hKeyboardLayout = ::LoadKeyboardLayout( Buff, KLF_ACTIVATE );
      const SHORT Vk = VkKeyScanEx( (char)c, hKeyboardLayout );
      const UINT VKey = ::MapVirtualKey( LOBYTE( Vk ), 0 );
      UnloadKeyboardLayout( hKeyboardLayout );

	  // Press CONTROL key
      ::ZeroMemory( &ev, sizeof( ev ));
      ev.type = INPUT_KEYBOARD;
      ev.ki.dwFlags = KEYEVENTF_SCANCODE;
      ev.ki.wScan = (WORD) ::MapVirtualKey( VK_LCONTROL, 0 );
      EventQueue.push_back( ev );

	  if( HIBYTE( Vk ) == 1 ) // Check if SHIFT key also needs to be pressed.
      {
          // Press SHIFT key
          ::ZeroMemory( &ev, sizeof( ev ));
          ev.type = INPUT_KEYBOARD;
          ev.ki.dwFlags = KEYEVENTF_SCANCODE;
          ev.ki.wScan = (WORD) ::MapVirtualKey( VK_LSHIFT, 0 );
          EventQueue.push_back( ev );
      }
 
      // Keydown
      ::ZeroMemory( &ev, sizeof( ev ));
      ev.type = INPUT_KEYBOARD;
      ev.ki.dwFlags = KEYEVENTF_SCANCODE;
      ev.ki.wScan = (WORD) VKey;
      EventQueue.push_back( ev );
 
      // Keyup
      ::ZeroMemory( &ev, sizeof( ev ));
      ev.type = INPUT_KEYBOARD;
      ev.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
      ev.ki.wScan = (WORD) VKey;
      EventQueue.push_back( ev );
 
      if( HIBYTE( Vk ) == 1 )	// Release if previouly pressed
      {
          // Release SHIFT key
          ::ZeroMemory( &ev, sizeof( ev ));
          ev.type = INPUT_KEYBOARD;
          ev.ki.dwFlags = KEYEVENTF_SCANCODE| KEYEVENTF_KEYUP;
          ev.ki.wScan = (WORD) ::MapVirtualKey( VK_LSHIFT, 0 );
          EventQueue.push_back( ev );
      }
	  // Release CONTROL key
	  ::ZeroMemory( &ev, sizeof( ev ));
	  ev.type = INPUT_KEYBOARD;
	  ev.ki.dwFlags = KEYEVENTF_SCANCODE| KEYEVENTF_KEYUP;
	  ev.ki.wScan = (WORD) ::MapVirtualKey( VK_LCONTROL, 0 );
	  EventQueue.push_back( ev );  
	}
    SendInput( EventQueue.size(), &EventQueue[0], sizeof(INPUT) );
}

void launch_program_win32( LPCTSTR lpCmdLine )
{
    PROCESS_INFORMATION pi;
	STARTUPINFO si;

    ZeroMemory( &pi, sizeof(pi) );  ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

    CreateProcess( NULL, (LPSTR)lpCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );
 } 

// -------------- Pulse8 call-backs --------------

int CecKeyPress( void *UNUSED(cbParam), const cec_keypress key )
{
	if( m_lastReleased )  return 0;	// Avoid auto-repeat

	cec_user_control_code k = key.keycode;
	TCHAR szBuf[80];
	HWND h = GetForegroundWindow();
	GetWindowText( h, szBuf, 80 );
	bool is_mediaportal = ( strncmp(szBuf, "MediaPortal",11) == 0 );	// Mediaportal support.

	WPARAM l = 0;
	bool is_ctrl = false;

	switch( k )
	{
		case CEC_USER_CONTROL_CODE_F1_BLUE:
			launch_program_win32( m_launchCommand1.c_str() );
			break;
		case CEC_USER_CONTROL_CODE_F2_RED:
			launch_program_win32( m_launchCommand2.c_str() );
			break;
		case CEC_USER_CONTROL_CODE_F3_GREEN:
			launch_program_win32( m_launchCommand3.c_str() );
			break;
		case CEC_USER_CONTROL_CODE_F4_YELLOW:
			launch_program_win32( m_launchCommand4.c_str() );
			break;
		case CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION:
			stop_mce_playback_pulse8();
			if( m_hasPCStandby )  SetSuspendState( FALSE, FALSE, FALSE );
			break;
		default:
			map<cec_user_control_code,int>::iterator it;
			if( is_mediaportal )
			{
				it = m_hdmiMediaportalKeymap.find( k );
				if( it != m_hdmiMediaportalKeymap.end() )
				{
					l = (*it).second;
				}
				else
				{
					it = m_hdmiMediaportalCtrlKeymap.find( k );
					if( it != m_hdmiMediaportalCtrlKeymap.end() )
					{
						l = (*it).second;  is_ctrl = true;
					}
				}
			}
			else
			{
				it = m_hdmiKeymap.find( k );
				if( it != m_hdmiKeymap.end() )
				{
					l = (*it).second;
				}
				else
				{
					it = m_hdmiCtrlKeymap.find( k );
					if( it != m_hdmiCtrlKeymap.end() )
					{
						l = (*it).second;  is_ctrl = true;
					}
				}
			}
			break;
	}

	send_key_win32( l, is_ctrl );

	return 0;
}

int CecCommand( void *UNUSED(cbParam), const cec_command command )
{
	if( m_parser != NULL )
	{
		cec_keypress key;
		switch( command.opcode )
		{
			case CEC_OPCODE_PLAY:
				key.keycode = CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION;
				m_lastReleased = FALSE;
				CecKeyPress( NULL, key );
				break;
			case CEC_OPCODE_STANDBY:
				key.keycode = CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION;
				m_lastReleased = FALSE;
				CecKeyPress( NULL, key );
				break;
			case CEC_OPCODE_USER_CONTROL_PRESSED:
				m_lastReleased = FALSE;
				break;
			case CEC_OPCODE_USER_CONTROL_RELEASE:
				m_lastReleased = TRUE;
				break;
			case CEC_OPCODE_DECK_CONTROL:
				key.keycode = CEC_USER_CONTROL_CODE_STOP;
				m_lastReleased = FALSE;
				CecKeyPress( NULL, key );
				break;
		}
	}
	return 0;
}

int CecAlert( void *UNUSED(cbParam), const libcec_alert type, const libcec_parameter UNUSED(param) )
{
	switch( type )
	{
		case CEC_ALERT_CONNECTION_LOST:
			if( m_parent != NULL )  m_parent->onHdmiCECLost();		// Callback mainframe to change the icon.
			break;
	}
	return 0;
}

void CBCecSourceActivated( void *param, const cec_logical_address, const uint8_t bActivated )
{
	if( bActivated == 0 )
	{
		if( m_parent != NULL )  m_parent->onHdmiCECLost();			// Callback mainframe to change the icon.
	}
	else
	{
		if( m_parent != NULL )  m_parent->onHdmiCECRestored();		// Callback mainframe to change the icon.
	}
}

// -------------- Hardware abstraction calls --------------

/** Open the CEC adapter */
int open_hdmi_cec_pulse8( CectrayMainDlg*parent, int nPort, bool hasPCStandby, bool hasTVStandby, const char* launchCommand1, const char* launchCommand2, const char* launchCommand3, const char* launchCommand4 )
{
	if( m_parser != NULL )  return HDMI_CECTRAY_OK;	// Already initialized.

	ICECAdapter* parser = NULL;
	if( nPort != -1 )
	{
		g_config.iPhysicalAddress = 0;  g_config.bAutodetectAddress = 0;	// Disable auto detection, to enforce an HDMI port.
	}
	g_callbacks.Clear();
	g_callbacks.CBCecKeyPress    = &CecKeyPress;
	g_callbacks.CBCecCommand     = &CecCommand;
	g_callbacks.CBCecAlert       = &CecAlert;
	g_callbacks.CBCecSourceActivated = &CBCecSourceActivated;
	g_config.Clear();
	snprintf( g_config.strDeviceName, 4, "HTPC" );
	g_config.callbackParam       = NULL;
	g_config.clientVersion       = CEC_CLIENT_VERSION_2_0_3;
	g_config.callbacks           = &g_callbacks;
	g_config.deviceTypes.Add( CEC_DEVICE_TYPE_RECORDING_DEVICE );
	g_config.iHDMIPort = (uint8_t)nPort;
	g_config.bPowerOffOnStandby = 1;
	g_config.bPowerOffScreensaver = 1;
	g_config.bActivateSource = 1;
	g_config.bSendInactiveSource = 1;
	g_config.powerOffDevices.Set( CECDEVICE_BROADCAST );
	g_config.wakeDevices.Set( CECDEVICE_BROADCAST );
	m_hasPCStandby = hasPCStandby;
	m_hasTVStandby = hasTVStandby;
	m_launchCommand1 = launchCommand1;
	m_launchCommand2 = launchCommand2;
	m_launchCommand3 = launchCommand3;
	m_launchCommand4 = launchCommand4;

	parser = LibCecInitialise( &g_config );
	if( parser == NULL )
	{
		return HDMI_CECTRAY_LIBCEC_ERROR;
	}

	if( g_strPort.length() == 0 )
	{
		cec_adapter devices[10];
		uint8_t iDevicesFound = parser->FindAdapters( devices, 10, NULL );
		if( iDevicesFound <= 0 )
		{
			if( parser != NULL )  { UnloadLibCec(parser); }
			return HDMI_CECTRAY_PORT_ERROR;
		}
		else
		{
			g_strPort = devices[0].comm;
		}
	}

	m_hdmiKeymap.clear();
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_SELECT] = VK_RETURN;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_ENTER] = VK_RETURN;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_UP] = VK_UP;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_DOWN] = VK_DOWN;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_LEFT] = VK_LEFT;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_RIGHT] = VK_RIGHT;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_DOT] = VK_DECIMAL;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_CHANNEL_UP] = VK_ADD;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_CHANNEL_DOWN] = VK_SUBTRACT;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_VOLUME_UP] = VK_VOLUME_UP;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_VOLUME_DOWN] = VK_VOLUME_DOWN;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER0] = '0';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER1] = '1';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER2] = '2';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER3] = '3';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER4] = '4';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER5] = '5';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER6] = '6';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER7] = '7';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER8] = '8';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_NUMBER9] = '9';
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_MUTE] = VK_VOLUME_MUTE;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_EXIT] = VK_BACK;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_ROOT_MENU] = VK_BROWSER_HOME;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION] = VK_RBUTTON;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_PLAY] = VK_MEDIA_PLAY_PAUSE;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION] = VK_MEDIA_PLAY_PAUSE;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_PAUSE] = VK_MEDIA_PLAY_PAUSE;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_STOP] = VK_MEDIA_STOP;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_REWIND] = VK_MEDIA_PREV_TRACK;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_FAST_FORWARD] = VK_MEDIA_NEXT_TRACK;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_BACKWARD] = VK_MEDIA_PREV_TRACK;
	m_hdmiKeymap[CEC_USER_CONTROL_CODE_FORWARD] = VK_MEDIA_NEXT_TRACK;

	m_hdmiCtrlKeymap.clear();
	m_hdmiCtrlKeymap[CEC_USER_CONTROL_CODE_RECORD] = 'r';
	m_hdmiCtrlKeymap[CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE] = 'g';

	m_hdmiMediaportalKeymap.clear();
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_SELECT] = VK_RETURN;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_ENTER] = VK_RETURN;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_UP] = VK_UP;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_DOWN] = VK_DOWN;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_LEFT] = VK_LEFT;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_RIGHT] = VK_RIGHT;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_DOT] = VK_DECIMAL;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_CHANNEL_UP] = VK_ADD;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_CHANNEL_DOWN] = VK_SUBTRACT;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_VOLUME_UP] = VK_VOLUME_UP;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_VOLUME_DOWN] = VK_VOLUME_DOWN;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER0] = '0';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER1] = '1';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER2] = '2';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER3] = '3';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER4] = '4';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER5] = '5';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER6] = '6';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER7] = '7';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER8] = '8';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_NUMBER9] = '9';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_MUTE] = 'M';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_EXIT] = VK_ESCAPE;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_ROOT_MENU] = 'H';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION] = VK_F3;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_PLAY] = 'P';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION] = VK_SPACE;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_PAUSE] = VK_MEDIA_PLAY_PAUSE;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_STOP] = 'B';
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_REWIND] = VK_F5;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_FAST_FORWARD] = VK_F6;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_BACKWARD] = VK_F7;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_FORWARD] = VK_F8;
	m_hdmiMediaportalKeymap[CEC_USER_CONTROL_CODE_RECORD] = 'R';

	m_parser = parser;	// Semaphore.

    if( ! m_parser->Open(g_strPort.c_str()) )
    {
		if( m_parser != NULL )  { UnloadLibCec(m_parser); m_parser=NULL; }
		return HDMI_CECTRAY_OPEN_ERROR;
    }

	m_parent = parent;

	return HDMI_CECTRAY_OK;
}

/** Kill the CEC adapter */
void close_hdmi_cec_pulse8()
{
    if( m_parser != NULL )
	{
		ICECAdapter* temp = m_parser;
		m_parser = NULL;			// Semafore.
		temp->Close();
		UnloadLibCec(temp);
	}
}

/** On power resuming */
void poweron_TV_pulse8()
{
	send_key_win32( VK_SHIFT, false );		// Press SHIFT key to make screensaver go away.
	if( m_parser != NULL )
	{
		if( m_hasTVStandby )
		{
			// Wake up HDMI devices.
			m_parser->PowerOnDevices( CECDEVICE_BROADCAST );
			m_parser->SetActiveSource( CEC_DEVICE_TYPE_TV );
		}
	}
}

/** On power suspending */
void poweroff_TV_pulse8()
{
    if( m_parser != NULL )
	{
		stop_mce_playback_pulse8();
		if( m_hasTVStandby )
		{
			// Put HDMI devices to sleep.
			m_parser->StandbyDevices( CECDEVICE_BROADCAST );
		}
		else
		{
			m_parser->SetInactiveView();	// Unset active source.
		}
	}
}

void stop_mce_playback_pulse8()
{
	// Press "stop" before sleeping.
	cec_keypress key;
	key.keycode = CEC_USER_CONTROL_CODE_STOP;
	m_lastReleased = FALSE;
	CecKeyPress( NULL, key );
}

bool can_hdmi_detect_port_pulse8()
{
	return (g_config.bAutodetectAddress==TRUE);
}
