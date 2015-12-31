#pragma once

struct NOVTABLE IWinampController
{
    virtual void PlayIndex(int index) = 0;
    virtual void QueueIndex(int index) = 0;
    virtual wchar_t * GetPlayListFile(int index) = 0;
    virtual wchar_t * GetPlayListTitle(int index) = 0;
    virtual int GetPlayListLength() = 0;
    virtual wchar_t * GetPlayListDir() = 0;
    virtual RECT GetPLWindowRect() = 0;
};

IWinampController * CreateController(HWND wa_hwnd);
void ReleaseController(IWinampController * controller);