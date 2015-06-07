#include "stdafx.h"
#include "Settings.h"
#include "WinampControl.h"

static Ptr<Settings> settings = 0;

class NullOnFailSettings
{
public:
    static bool InitInstance()
    {
        WCHAR datadir[MAX_PATH + 1] = { 0 }, *settingPath = datadir, *pld;
        pld = GetPlayListDir();
        if (pld)
        {
            StringCbCopyW(datadir, sizeof(datadir), GetPlayListDir());
            PathAllocCombine(datadir, L"plugins/gen_naivesearch.setting", 0, &settingPath);
        }
        else
        {
            return false;
        }
        settings = new Settings(settingPath);
        LocalFree(settingPath);
        return true;
    }
};

void InitDefaultInstance()
{
    if (!settings && !NullOnFailSettings::InitInstance())
    {
        GetApplication()->DelayExecute(&InitDefaultInstance, 500);
    }
}

Settings& Settings::Default()
{
    return *settings.Obj();
}

Settings::Settings(const WString& path) : settingPath(path)
{

}

typedef _com_ptr_t <_com_IIID<IXmlWriter, &__uuidof(IXmlWriter)>> XmlWriterPtr;
typedef _com_ptr_t <_com_IIID<IXmlReader, &__uuidof(IXmlReader)>> XmlReaderPtr;
typedef _com_ptr_t <_com_IIID<IStream, &__uuidof(IStream)>> StreamPtr;

void Settings::Save() const
{
    HRESULT hr = S_OK;
    do
    {
        StreamPtr stream;
        hr = SHCreateStreamOnFile(
            settingPath.Buffer(),
            STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
            &stream);
        if (FAILED(hr)) break;
        XmlWriterPtr writer;
        hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&writer), NULL);
        if (FAILED(hr)) break;
        hr = writer->SetOutput(stream);
        if (FAILED(hr)) break;
        {
            writer->WriteStartDocument(XmlStandalone_Omit);
            writer->WriteStartElement(NULL, L"settings", NULL);
            writer->WriteElementString(NULL, L"Top", NULL, vl::itow(_windowBounds.Top()).Buffer());
            writer->WriteElementString(NULL, L"Left", NULL, vl::itow(_windowBounds.Left()).Buffer());
            writer->WriteElementString(NULL, L"Right", NULL, vl::itow(_windowBounds.Right()).Buffer());
            writer->WriteElementString(NULL, L"Bottom", NULL, vl::itow(_windowBounds.Bottom()).Buffer());
            writer->WriteEndElement();
            writer->WriteEndDocument();
            writer->Flush();
        }
        hr = stream->Commit(STGC_DEFAULT);
        if (FAILED(hr)) break;
    } while (false);
}

void Settings::Reload()
{
    HRESULT hr = S_OK;
    do
    {
        StreamPtr stream;
        hr = SHCreateStreamOnFile(
            settingPath.Buffer(),
            STGM_READ | STGM_SHARE_EXCLUSIVE,
            &stream);
        if (FAILED(hr)) break;
        XmlReaderPtr reader;
        hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&reader), NULL);
        if (FAILED(hr)) break;
        hr = reader->SetInput(stream);
        if (FAILED(hr)) break;
        {
            XmlNodeType nodeType;
            LPCWSTR nodeName = NULL, nodeValue;
            while (!reader->IsEOF())
            {
                reader->Read(&nodeType);
                switch (nodeType)
                {
                case XmlNodeType_Text:
                    if (!nodeName) break;
                    if (StrCmpW(nodeName, L"Left") == 0)
                    {
                        hr = reader->GetValue(&nodeValue, NULL);
                        if (FAILED(hr)) break;
                        _windowBounds.x1 = vl::wtoi(nodeValue);
                    }
                    else if (StrCmpW(nodeName, L"Top") == 0)
                    {
                        hr = reader->GetValue(&nodeValue, NULL);
                        if (FAILED(hr)) break;
                        _windowBounds.y1 = vl::wtoi(nodeValue);
                    }
                    else if (StrCmpW(nodeName, L"Right") == 0)
                    {
                        hr = reader->GetValue(&nodeValue, NULL);
                        if (FAILED(hr)) break;
                        _windowBounds.x2 = vl::wtoi(nodeValue);
                    }
                    else if (StrCmpW(nodeName, L"Bottom") == 0)
                    {
                        hr = reader->GetValue(&nodeValue, NULL);
                        if (FAILED(hr)) break;
                        _windowBounds.y2 = vl::wtoi(nodeValue);
                    }
                case XmlNodeType_Element:
                    hr = reader->GetLocalName(&nodeName, NULL);
                    if (FAILED(hr)) break;
                default:
                    break;
                }
            }
        }
    } while (false);
}

void Settings::Clear()
{
    _windowBounds = { 200, 200, 400, 600 };
}

WString& Settings::SettingFilePath()
{
    return settingPath;
}

const WString& Settings::SettingFilePath() const
{
    return settingPath;
}

Rect& Settings::WindowBounds()
{
    return _windowBounds;
}

const Rect& Settings::WindowBounds() const
{
    return _windowBounds;
}

typedef _com_ptr_t <_com_IIID<IShellFolder, &__uuidof(IShellFolder)>> ShellFolderPtr;

void OpenFolderAndSelectFile(const WString& filePath)
{
    LPWSTR sfdirpath = new WCHAR[filePath.Length() + 1];
    HRESULT hr = S_OK;
    if (sfdirpath)
    {
        do
        {
            hr = StringCchCopyW(sfdirpath, filePath.Length() + 1, filePath.Buffer());
            if (FAILED(hr)) break;
            ShellFolderPtr shell;
            hr = SHGetDesktopFolder(&shell);
            if (FAILED(hr)) break;
            LPITEMIDLIST idlist, idfolder;
            hr = shell->ParseDisplayName(NULL, NULL, sfdirpath, NULL, &idlist, NULL);
            if (FAILED(hr)) break;
            hr = PathCchRemoveFileSpec(sfdirpath, filePath.Length() + 1);
            if (FAILED(hr)) break;
            hr = shell->ParseDisplayName(NULL, NULL, sfdirpath, NULL, &idfolder, NULL);
            if (FAILED(hr)) break;
            CoInitialize(NULL);
            LPCITEMIDLIST cidlist = idlist;
            SHOpenFolderAndSelectItems(idfolder, 1, &cidlist, 0);
            CoUninitialize();
        } while (false);

        delete[] sfdirpath;
    }
}