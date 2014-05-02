#include "stdafx.h"

using namespace System;
using namespace System::Reflection;

[assembly:AssemblyVersionAttribute("1.0.0.2")];
[assembly:AssemblyCultureAttribute("zh")];
[assembly:AssemblyTitleAttribute("WinampQuickSearch")];
[assembly:AssemblyCopyrightAttribute("iyomumx @ 2014")];
[assembly:AssemblyDescriptionAttribute("为Winamp提供简易的搜索功能的插件")];
[assembly:System::Runtime::CompilerServices::CompilationRelaxationsAttribute(System::Runtime::CompilerServices::CompilationRelaxations::NoStringInterning)];


WNDPROC oldWndProc;
BOOL fUnicode;
LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
genHotkeysAddStruct hotkey;
api_service *WASABI_API_SVC;
waServiceFactory *factory;
api_queue * QueueApi;
winampGeneralPurposePlugin plugin = { PLUGIN_VER, PLUGIN_NAME, init, config, quit, 0, 0 };