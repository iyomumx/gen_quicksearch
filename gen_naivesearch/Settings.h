#pragma once
#include "WinampControl.h"

void InitDefaultInstance(IWinampController * wactrl);

class Settings
{
    friend class NullOnFailSettings;
private:
    Rect _windowBounds = { 200, 200, 400, 600 };
    Settings(const vl::WString&);
    WString settingPath;
public:
    static Settings& Default();
    void Save() const;
    void Reload();
    void Clear();

    vl::WString& SettingFilePath();
    const vl::WString& SettingFilePath() const;

    Rect& WindowBounds();
    const Rect& WindowBounds() const;
};

void OpenFolderAndSelectFile(const vl::WString& filePath);