// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <Commdlg.h>
#include <Shlwapi.h>

// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include <api\service\api_service.h>
#include <api\service\waservicefactory.h>
#include "..\Agave\Queue\api_queue.h"
#include <..\Agave\Config\api_config.h>
#include <GEN.H>
#include <wa_ipc.h>
#include <..\gen_tray\WINAMPCMD.H>

//GUI
#include "..\gac\Public\Source\Vlpp.h"
#include "..\gac\Public\Source\GacUI.h"
#include "..\gac\Public\Source\GacUIWindows.h"