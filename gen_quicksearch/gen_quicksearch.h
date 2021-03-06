#pragma once
#include <Windows.h>
#include <api\service\api_service.h>
#include <api\service\waservicefactory.h>
#include "..\Agave\Queue\api_queue.h"
#include <GEN.H>
#include <wa_ipc.h>

#define PLUGIN_SHORT_NAME "Search Playlist"

#pragma managed(push,off)

typedef struct {
	char *name;       // this is the name that will appear in the Global Hotkeys preferences panel
	DWORD flags;      // one or more HKF_* flags from above
	UINT uMsg;        // message that will be sent to winamp's main window (must always be !=NULL)
	WPARAM wParam;    // wParam that will be sent to winamp's main window
	LPARAM lParam;    // lParam that will be sent to winamp's main window
	char *id;         // unique string to identify this command (the string is case insensitive)
	HWND wnd;         // set the HWND to send message (or 0 for the main winamp window)


	int extended[6];  // for future extension - always set this to zero!
} genHotkeysAddStruct;

#pragma managed(pop)

extern int init();
extern void quit();
extern void config();

extern WNDPROC oldWndProc;
extern BOOL fUnicode;
extern LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern genHotkeysAddStruct hotkey;
extern api_service *WASABI_API_SVC;
extern waServiceFactory *factory;
extern api_queue * QueueApi;
extern winampGeneralPurposePlugin plugin;