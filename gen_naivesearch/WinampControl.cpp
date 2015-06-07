#include "stdafx.h"
#include "WinampControl.h"

extern winampGeneralPurposePlugin plugin;

void PlayIndex(int index)
{
    if (SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING) != 1)
    {
        SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
    }
    SendMessage(plugin.hwndParent, WM_WA_IPC, static_cast<WPARAM>(index), IPC_SETPLAYLISTPOS);
    SendMessage(plugin.hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2, 0), 0);
}

void QueueIndex(api_queue * api, int index)
{
    if (api != NULL)
    {
        api->AddItemToQueue(index, 1, NULL);
    }
}

wchar_t * GetPlayListFile(int index)
{
    return reinterpret_cast<wchar_t*>(
        SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)index, IPC_GETPLAYLISTFILEW));
}

wchar_t * GetPlayListTitle(int index)
{
    return reinterpret_cast<wchar_t*>(
        SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)index, IPC_GETPLAYLISTTITLEW));
}

int GetListLength()
{
    return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
}

wchar_t * GetPlayListDir()
{
    return reinterpret_cast<wchar_t*>(
        SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETM3UDIRECTORYW));
}