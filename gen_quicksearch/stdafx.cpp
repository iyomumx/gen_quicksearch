#include "stdafx.h"

using namespace System;
using namespace System::Reflection;

#define PLUGIN_VER 0x11
#define PLUGIN_NAME "Quick Search In Current Playlist v0.0.6"

[assembly:AssemblyVersionAttribute("1.0.0.6")];
[assembly:AssemblyCultureAttribute("zh")];
[assembly:AssemblyTitleAttribute("WinampQuickSearch")];
[assembly:AssemblyCopyrightAttribute("iyomumx @ 2014")];
[assembly:AssemblyDescriptionAttribute("为Winamp提供简易的搜索功能的插件")];
[assembly:System::Runtime::CompilerServices::CompilationRelaxationsAttribute(System::Runtime::CompilerServices::CompilationRelaxations::NoStringInterning)];

#pragma unmanaged

WNDPROC oldWndProc;
BOOL fUnicode;
LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
genHotkeysAddStruct hotkey;
api_service *WASABI_API_SVC;
waServiceFactory *factory;
api_queue * QueueApi;
winampGeneralPurposePlugin plugin = { PLUGIN_VER, (char*)(void*)_T(PLUGIN_NAME), init, config, quit, 0, 0 };