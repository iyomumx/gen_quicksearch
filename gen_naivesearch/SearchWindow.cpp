#include "stdafx.h"
#include "SearchWindow.h"
#include "NaiveSearchWindow.h"
#include "Settings.h"

static SearchWindowStartUpParam * startp;

DWORD WINAPI SearchWindowStart(LPVOID param)
{
    startp = reinterpret_cast<SearchWindowStartUpParam *>(param);
    startp->Controller = CreateController(startp->Plugin->hwndParent);
    return SetupWindowsDirect2DRenderer();
}

void SetDefaultSetting()
{
    RECT ws = startp->Controller->GetPLWindowRect();
    if (ws.left != ws.right && ws.top != ws.bottom)
    {
        Settings::Default().WindowBounds() = vl::presentation::Rect(ws.left, ws.top, ws.right, ws.bottom);
    }
}

void GuiMain()
{
    InitDefaultInstance(startp->Controller);
    SetDefaultSetting();
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