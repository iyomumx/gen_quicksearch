#pragma once

#include <Windows.h>
#include <tchar.h>
#include <wa_ipc.h>
#include <..\gen_tray\WINAMPCMD.H>
#include <GEN.H>
#include <api\service\api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager
#include <api\service\waServiceFactory.h>
#include <..\Agave\Config\api_config.h>

#include "Track.h"
#include "PluginWindow.h"
#include "gen_quicksearch.h"