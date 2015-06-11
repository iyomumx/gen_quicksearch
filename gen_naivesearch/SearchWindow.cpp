#include "stdafx.h"
#include "SearchWindow.h"
#include "NaiveSearchWindow.h"
#include "Settings.h"

static SearchWindowStartUpParam * startp;

DWORD WINAPI SearchWindowStart(LPVOID param)
{
    vl::Thread::Sleep(1000);
    startp = reinterpret_cast<SearchWindowStartUpParam *>(param);
    startp->Controller = CreateController(startp->Plugin->hwndParent);
    return SetupWindowsDirect2DRenderer();
}

void GuiMain()
{
    InitDefaultInstance(startp->Controller);
    try
    {
        Settings::Default().Reload();
    }
    catch (...)
    {
        Settings::Default().Clear();
    }
    GuiWindow * sw = CreateNaiveSearchWindow(startp);
    GetApplication()->Run(sw);
    delete sw;
}