#include <Winamp\wa_ipc.h>
#include <Winamp\GEN.H>
#include <Windows.h>

#include "PluginWindow.h"

using namespace System;

#define PLUGIN_VER 0x10
#define PLUGIN_NAME "Quick Search In Current Playlist v0.0.1"

int init();
void quit();
void config();

winampGeneralPurposePlugin plugin = { PLUGIN_VER, PLUGIN_NAME, init, config, quit };
extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() { return &plugin; }

void Init()
{
	PluginWindow::Init(plugin.hwndParent);
}

int init()
{
	auto t = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(&Init));
	t->Start();
	return 0;
}

void quit()
{
	PluginWindow::MainWindow->Close();
}

void config()
{
	
}
