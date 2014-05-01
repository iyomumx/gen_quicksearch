#include "stdafx.h"

WNDPROC oldWndProc;
BOOL fUnicode;
LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
genHotkeysAddStruct hotkey;
api_service *WASABI_API_SVC;
waServiceFactory *factory;
api_queue * QueueApi;
winampGeneralPurposePlugin plugin = { PLUGIN_VER, PLUGIN_NAME, init, config, quit, 0, 0 };