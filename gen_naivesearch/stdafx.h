// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <Commdlg.h>
#include <Shlwapi.h>

// TODO:  在此处引用程序需要的其他头文件
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