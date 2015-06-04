#pragma once
DWORD WINAPI SearchWindowStart(LPVOID);

struct genHotkeysAddStruct {
    char *name;       // this is the name that will appear in the Global Hotkeys preferences panel
    DWORD flags;      // one or more HKF_* flags from above
    UINT uMsg;        // message that will be sent to winamp's main window (must always be !=NULL)
    WPARAM wParam;    // wParam that will be sent to winamp's main window
    LPARAM lParam;    // lParam that will be sent to winamp's main window
    char *id;         // unique string to identify this command (the string is case insensitive)
    HWND wnd;         // set the HWND to send message (or 0 for the main winamp window)


    int extended[6];  // for future extension - always set this to zero!
    genHotkeysAddStruct(char * name, char* id, WPARAM wp = 0, LPARAM lp = 0)
        : name(name), id(id), uMsg(0), wnd(0), flags(0), wParam(wp), lParam(lp)
    {

    }

    void RegisterHotkey(HWND wa_window)
    {
        this->uMsg = static_cast<UINT>(
            SendMessage(wa_window, WM_WA_IPC,
            reinterpret_cast<WPARAM>(this->id),
            IPC_REGISTER_WINAMP_IPCMESSAGE));
        LRESULT genhotkeys_add_ipc =
            SendMessage(wa_window, WM_WA_IPC,
            reinterpret_cast<WPARAM>(&"GenHotkeysAdd"),
            IPC_REGISTER_WINAMP_IPCMESSAGE);
        PostMessage(wa_window, WM_WA_IPC, reinterpret_cast<WPARAM>(this), genhotkeys_add_ipc);
    }
};

class IPlugin
{
public:
    virtual void OnConfig() { }
    virtual void OnExit() { }
    virtual void OnHotkeyPressed() { }
    virtual void OnListRefresh() { }
};

struct SearchWindowStartUpParam
{
    winampGeneralPurposePlugin * Plugin;
    HANDLE ProcessHeap;
    HWND SearchWindow;
    IPlugin * SearchPlugin;
    api_queue * QueueApi;
    LPVOID preserved;
};
