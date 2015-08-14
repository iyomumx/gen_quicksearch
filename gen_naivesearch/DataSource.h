#pragma once
#include "WinampControl.h"

struct PlaylistItem
{
    vl::WString Title;
    vl::WString Filepath;

    PlaylistItem() : Title(L"--"), Filepath(L"") { }
    PlaylistItem(const vl::WString& title, const vl::WString& path)
        : Title(title), Filepath(path) { }
    PlaylistItem(const wchar_t * title, const wchar_t* path)
        : Title(title), Filepath(path) { }

    bool Match(const vl::regex::Regex* regex) const
    {
        return regex->Match(Title) || regex->Match(Filepath);
    }

    bool Match(const vl::WString & par) const
    {
        return
            StrStrIW(Title.Buffer(), par.Buffer()) != NULL ||
            StrStrIW(Filepath.Buffer(), par.Buffer()) != NULL;
    }

    bool operator==(const PlaylistItem& other)
    {
        return Title == other.Title && Filepath == other.Filepath;
    }
};

class DataSource :
    public vl::presentation::controls::list::ItemProviderBase,
    private vl::presentation::controls::list::TextItemStyleProvider::ITextItemView
{
protected:
    vl::Ptr<vl::collections::List<PlaylistItem>> playlist;
    vl::collections::List<PlaylistItem> viewlist;
    struct
    {
        bool UseStringFilter = true;
        vl::WString StringFilter = L"";
        vl::regex::Regex * RegexFilter = nullptr;
    };
    IWinampController * wactrl;
public:
    DataSource(IWinampController * controller);
    ~DataSource();

    DataSource(const DataSource&) = delete;
    DataSource& operator=(const DataSource&) = delete;

    int Count() override;
    IDescriptable* RequestView(const vl::WString& identifier) override;
    void ReleaseView(vl::reflection::IDescriptable* view) override;
    vl::WString GetText(int itemIndex) override;
    bool GetChecked(int itemIndex) override;
    void SetCheckedSilently(int itemIndex, bool value) override;
    vl::WString GetPrimaryTextViewText(int itemIndex) override;
    bool ContainsPrimaryText(int itemIndex) override;

    int TranslateIndex(int index);
    void UpdatePlaylist();
    void UpdateFilter(vl::regex::Regex* regex);
    void UpdateFilter(const vl::WString& par);
    void UpdateView();
};

