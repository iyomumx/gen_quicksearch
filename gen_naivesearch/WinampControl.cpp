#include "stdafx.h"
#include "WinampControl.h"

class WinampController sealed
    : public IWinampController
{
private:
    HWND hwnd;
    api_queue * api = NULL;
    api_service * wasabisvc = NULL;
    waServiceFactory * factory = NULL;

    void ReleaseAPIPtr()
    {
        if (this->api) this->api->Release();
        if (this->factory) this->factory->Release();
        if (this->wasabisvc) this->wasabisvc->Release();
    }

    void InitliazeQueueAPI()
    {
        this->ReleaseAPIPtr();
        wasabisvc = reinterpret_cast<api_service*>(
            SendMessage(this->hwnd, WM_WA_IPC, 0, IPC_GET_API_SERVICE));
        if (wasabisvc && (factory = wasabisvc->service_getServiceByGuid(QueueManagerApiGUID)))
        {
            this->api = reinterpret_cast<api_queue *>(factory->getInterface());
        }
        else
        {
            this->api = NULL;
        }
    }
public:
    WinampController(HWND wa_hwnd) : hwnd(wa_hwnd)
    {
        this->InitliazeQueueAPI();
    }
    ~WinampController()
    {
        this->ReleaseAPIPtr();
    }

    void PlayIndex(int index) override
    {
        if (SendMessage(this->hwnd, WM_WA_IPC, 0, IPC_ISPLAYING) != 1)
        {
            SendMessage(this->hwnd, WM_WA_IPC, 0, IPC_STARTPLAY);
        }
        SendMessage(this->hwnd, WM_WA_IPC, static_cast<WPARAM>(index), IPC_SETPLAYLISTPOS);
        SendMessage(this->hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2, 0), 0);
    }

    void QueueIndex(int index) override
    {
        if (this->api != NULL)
        {
            this->api->AddItemToQueue(index, 1, NULL);
        }
        else
        {
            int retry = 5;
            while (!this->api && retry--)
            {
                this->InitliazeQueueAPI();
            }
            if (this->api) this->QueueIndex(index);
        }
    }

    wchar_t * GetPlayListFile(int index) override
    {
        return reinterpret_cast<wchar_t*>(
            SendMessage(this->hwnd, WM_WA_IPC, (WPARAM)index, IPC_GETPLAYLISTFILEW));
    }

    wchar_t * GetPlayListTitle(int index) override
    {
        return reinterpret_cast<wchar_t*>(
            SendMessage(this->hwnd, WM_WA_IPC, (WPARAM)index, IPC_GETPLAYLISTTITLEW));
    }

    int GetPlayListLength() override
    {
        return SendMessage(this->hwnd, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
    }

    wchar_t * GetPlayListDir() override
    {
        return reinterpret_cast<wchar_t*>(
            SendMessage(this->hwnd, WM_WA_IPC, 0, IPC_GETM3UDIRECTORYW));
    }

    RECT GetPLWindowRect() override
    {
        HWND h = reinterpret_cast<HWND>(SendMessage(this->hwnd, WM_WA_IPC, IPC_GETWND_PE, IPC_GETWND));
        if (IsWindow(h) == FALSE)
        {
            h = this->hwnd;
        }
        //now h should be a vaild window
        RECT ret = { 0 };
        GetWindowRect(h, &ret);
        return ret;
    }
};

IWinampController * CreateController(HWND wa_hwnd)
{
    return new WinampController(wa_hwnd);
}

void ReleaseController(IWinampController * controller)
{
    delete controller;
}