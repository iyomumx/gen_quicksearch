#include "stdafx.h"
#include "SearchWindow.h"
#include "NaiveSearchWindow.h"
#include "Settings.h"

static SearchWindowStartUpParam * startp;

DWORD WINAPI SearchWindowStart(LPVOID param)
{
    api_service * wasabisvc = NULL;
    waServiceFactory * factory = NULL;

    vl::Thread::Sleep(500);

    startp = reinterpret_cast<SearchWindowStartUpParam*>(param);

    wasabisvc = reinterpret_cast<api_service*>(
        SendMessage(startp->Plugin->hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE));

    factory = wasabisvc->service_getServiceByGuid(QueueManagerApiGUID);
    if (factory != NULL)
    {
        startp->QueueApi = reinterpret_cast<api_queue*>(factory->getInterface());
    }

    return SetupWindowsDirect2DRenderer();
}

void GuiMain()
{
    InitDefaultInstance();
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