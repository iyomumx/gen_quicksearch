#include "stdafx.h"
#include "DataSource.h"
#include "WinampControl.h"

DataSource::DataSource()
    : playlist(MakePtr<vl::collections::List<PlaylistItem>>())
{
    
}


DataSource::~DataSource()
{
}

int DataSource::Count()
{
    return viewlist.Count();
}


IDescriptable* DataSource::RequestView(const WString& identifier)
{
    if (identifier == list::TextItemStyleProvider::ITextItemView::Identifier)
    {
        return this;
    }
    else if (identifier == GuiListControl::IItemPrimaryTextView::Identifier)
    {
        return this;
    }
    else
    {
        return 0;
    }
}


void DataSource::ReleaseView(IDescriptable* view)
{

}


WString DataSource::GetText(int itemIndex)
{
    return viewlist[itemIndex].Title;
}

bool DataSource::GetChecked(int itemIndex)
{
    return false;
}


void DataSource::SetCheckedSilently(int, bool)
{
}


WString DataSource::GetPrimaryTextViewText(int itemIndex)
{
    return GetText(itemIndex);
}


bool DataSource::ContainsPrimaryText(int itemIndex)
{
    return true;
}

int DataSource::TranslateIndex(int index)
{
    //TODO:Translate View Index to Playlist Index
    return playlist->IndexOf(viewlist[index]);
}

void DataSource::UpdatePlaylist(HWND wa_window)
{
    auto app = GetApplication();
    if (app->IsInMainThread())
    {
        app->InvokeAsync([=]() { this->UpdatePlaylist(wa_window); });
    }
    else
    {
        Ptr<vl::collections::List<PlaylistItem>> temp =
            vl::MakePtr<vl::collections::List<PlaylistItem>>();
        int count = GetListLength();
        for (int i = 0; i < count; i++)
        {
            PlaylistItem item(
                GetPlayListTitle(i),
                GetPlayListFile(i));
            temp->Add(item);
        }
        app->InvokeLambdaInMainThreadAndWait([this, temp]()
        {
            playlist = temp;
            UpdateView();
        });
    }
}

void DataSource::UpdateFilter(vl::regex::Regex* regex)
{
    UseStringFilter = false;
    RegexFilter = regex;
    UpdateView();
}

void DataSource::UpdateFilter(const vl::WString& par)
{
    UseStringFilter = true;
    StringFilter = par;
    UpdateView();
}

void DataSource::UpdateView()
{
    int count = viewlist.Count();
    viewlist.Clear();
    if (UseStringFilter)
    {
        for (int i = 0, c = playlist->Count(); i < c; i++)
        {
            const PlaylistItem& obj = playlist->Get(i);
            if (StringFilter.Length() == 0 || obj.Match(StringFilter))
            {
                viewlist.Add(obj);
            }
        }
    }
    else
    {
        for (int i = 0, c = playlist->Count(); i < c; i++)
        {
            const PlaylistItem& obj = playlist->Get(i);
            if (obj.Match(RegexFilter))
            {
                viewlist.Add(obj);
            }
        }
    }
    int newcount = viewlist.Count();
    InvokeOnItemModified(0, count, newcount);
}