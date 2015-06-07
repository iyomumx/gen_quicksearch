#pragma once

void InitDefaultInstance();

class Settings
{
    friend class NullOnFailSettings;
private:
    Rect _windowBounds = { 200, 200, 400, 600 };
    Settings(const WString&);
    WString settingPath;
public:
    static Settings& Default();
    void Save() const;
    void Reload();
    void Clear();

    WString& SettingFilePath();
    const WString& SettingFilePath() const;

    Rect& WindowBounds();
    const Rect& WindowBounds() const;
};

void OpenFolderAndSelectFile(const WString& filePath);