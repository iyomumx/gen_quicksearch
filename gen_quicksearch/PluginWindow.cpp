#include "PluginWindow.h"


PluginWindow::PluginWindow()
{
}

void PluginWindow::Init(HWND handle)
{
	MainWindow = gcnew PluginWindow();
	MainWindow->MainHandle = handle;
	Application ^ app = gcnew Application();
	app->Run(MainWindow);
}

void PluginWindow::InitializeComponent()
{
	//��ʼ��������趨
}