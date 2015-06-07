#include "stdafx.h"
#include "SearchWindow.h"
#include "NaiveSearchWindow.h"
#include "DataSource.h"
#include "WinampControl.h"
#include "Settings.h"

class NaiveSearchWindow sealed :
    public vl::presentation::controls::GuiWindow,
    public IPlugin
{
private:
    DataSource * dataSource;
    GuiVirtualTextList* listBox;
    GuiSinglelineTextBox * searchBox;
    bool inputing = false;

    static inline bool IsContorlKeyClean(
        const NativeWindowKeyInfo& info,
        bool altstatus = false,
        bool ctrlstatus = false,
        bool shiftstatus = false)
    {
        return (info.alt == altstatus && info.ctrl == ctrlstatus && info.shift == shiftstatus);
    }

    vl::Ptr<vl::presentation::INativeDelay> lastChangeDelay, refreshPLDelay;
    SearchWindowStartUpParam* p;

    void NaiveHide()
    {
        auto thiswinf = vl::presentation::windows::GetWindowsForm(this->GetNativeWindow());
        ShowWindow(thiswinf->GetWindowHandle(), SW_HIDE);
    }

    void NaiveShow()
    {
        GuiWindow::Show();
        auto thiswinf = vl::presentation::windows::GetWindowsForm(this->GetNativeWindow());
        ShowWindow(thiswinf->GetWindowHandle(), SW_SHOWNORMAL);

        BringWindowToTop(thiswinf->GetWindowHandle());
        SetForegroundWindow(thiswinf->GetWindowHandle());

        this->SetActivated();
        searchBox->SelectAll();
        searchBox->SetFocus();
    }
public:

    void InitializeComponents()
    {
        this->SetTitleBar(false);
        this->SetTopMost(true);

        this->SetBounds(Settings::Default().WindowBounds());

        GuiTableComposition * table = new GuiTableComposition();
        table->SetRowsAndColumns(2, 1);
        table->SetCellPadding(3);
        table->SetAlignmentToParent({ 0, 0, 0, 0 });

        table->SetRowOption(0, GuiCellOption::AbsoluteOption(26));
        table->SetRowOption(1, GuiCellOption::PercentageOption(1.0));

        table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));


        this->GetContainerComposition()->AddChild(table);

        {
            GuiCellComposition* cell = new GuiCellComposition();
            table->AddChild(cell);
            cell->SetSite(0, 0, 1, 1);

            searchBox = g::NewTextBox();
            searchBox->GetBoundsComposition()->SetAlignmentToParent({ 0, 0, 0, 0 });
            searchBox->TextChanged.AttachMethod(this, &NaiveSearchWindow::searchBox_TextChanged);

            auto fc = searchBox->GetFocusableComposition()->GetEventReceiver();
            fc->gotFocus.AttachMethod(this, &NaiveSearchWindow::searchBox_gotFocus);
            fc->lostFocus.AttachMethod(this, &NaiveSearchWindow::searchBox_lostFocus);

            cell->AddChild(searchBox->GetBoundsComposition());
        }

        {
            GuiCellComposition* cell = new GuiCellComposition();
            table->AddChild(cell);
            cell->SetSite(1, 0, 1, 1);

            dataSource = new DataSource;
            this->OnListRefresh();

            listBox = new GuiVirtualTextList(GetCurrentTheme()->CreateTextListStyle(), GetCurrentTheme()->CreateTextListItemStyle(), dataSource);
            listBox->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
            listBox->SetHorizontalAlwaysVisible(false);
            listBox->SetMultiSelect(false);
            listBox->ItemLeftButtonDoubleClick.AttachMethod(this, &NaiveSearchWindow::listBox_ItemLeftButtonDoubleClick);
            listBox->SetVerticalAlwaysVisible(false);
            
            cell->AddChild(listBox->GetBoundsComposition());
        }

        this->GetNativeWindow()->HideInTaskBar();
        this->SetBorder(false);
        GetApplication()->DelayExecuteInMainThread([this]()
        {
            this->NaiveHide();
            this->Show();
            this->NaiveHide();
        }, 50);
#pragma region Window Events
        this->WindowLostFocus.AttachMethod(this, &NaiveSearchWindow::window_WindowLostFocus);
#pragma endregion End Window Events
    }

    NaiveSearchWindow(SearchWindowStartUpParam * param)
        : vl::presentation::controls::GuiWindow(GetCurrentTheme()->CreateWindowStyle()),
        p(param)
    {
        InitializeComponents();
        p->SearchPlugin = this;
    }

#pragma region IPlugin
    //
    //IPlugin
    //
    void OnConfig() override
    {
        //TODO:Add Config Window
        Show(true);
    }

    void OnExit() override
    {
        Settings::Default().WindowBounds() = this->GetBounds();
        try
        {
            Settings::Default().Save();

        }
        catch (...) { }
        GetApplication()->InvokeLambdaInMainThreadAndWait([=](){ this->Close(); }, 500);
    }

    void OnHotkeyPressed() override
    {
        this->Show();
    }

    void OnListRefresh() override
    {
        if (refreshPLDelay)
        {
            if (refreshPLDelay->GetStatus() == INativeDelay::Pending)
            {
                refreshPLDelay->Cancel();
            }
        }
        refreshPLDelay = GetApplication()->DelayExecute([=]()
        {
            dataSource->UpdatePlaylist(p->Plugin->hwndParent);
        }, 200);
    }
#pragma endregion

#pragma region GuiWindow

    //
    //GuiWindow
    //
    void Show()
    {
        Show(false);
    }

    void Show(bool showBorder)
    {
        auto app = GetApplication();
        if (app->IsInMainThread())
        {
            this->NaiveShow();
            this->SetBorder(showBorder);
        }
        else
        {
            app->InvokeLambdaInMainThread([=](){ this->Show(); });
        }
    }

    void KeyDown(const NativeWindowKeyInfo& info) override
    {
        if (info.code == VKEY_RETURN && IsContorlKeyClean(info)) //Enter
        {
            auto index = listBox->GetSelectedItemIndex();
            if (index == -1)
            {
                index = 0;
            }
            PlayIndex(dataSource->TranslateIndex(index));
            this->NaiveHide();
        }
        else if (info.code == VKEY_TAB && IsContorlKeyClean(info)) //Tab
        {
            auto index = listBox->GetSelectedItemIndex();
            auto count = dataSource->Count();
            if (index != -1)
            {
                listBox->SetSelected(index, false);
            }
            index = (index + 1) % count;
            listBox->SetSelected(index, true);
            listBox->EnsureItemVisible(index);
            listBox->SetFocus();
        }
        else if (info.code == VKEY_Q && IsContorlKeyClean(info)) //Q
        {
            if (p->QueueApi && !inputing)
            {
                auto index = listBox->GetSelectedItemIndex();
                if (index == -1)
                {
                    index = 0;
                }
                QueueIndex(p->QueueApi, dataSource->TranslateIndex(index));
                this->NaiveHide();
            }
        }
#ifdef _DEBUG
        else if (info.code == VKEY_S && IsContorlKeyClean(info, false, true)) //Ctrl+S
        {
            OpenFolderAndSelectFile(Settings::Default().SettingFilePath());
            this->SetBorder(!this->GetBorder());
        }
#endif
        else if (info.code == VKEY_ESCAPE) //ESC
        {
            this->NaiveHide();
        }
    }

    template <int r>
    static bool IsNear(const int& x1, const int& y1, const int& x2, const int& y2)
    {
        const int dx = x1 - x2;
        const int dy = y1 - y2;
        return dx * dx + dy * dy <= r * r;
    }

    HitTestResult HitTest(Point location) override
    {
        HitTestResult ret = GuiWindow::HitTest(location);
        if (ret == HitTestResult::NoDecision)
        {
            auto bounds = this->GetNativeWindow()->GetClientBoundsInScreen();
            bool isNear =
                IsNear<16>(location.x, location.y, 0, 0) ||
                IsNear<16>(location.x, location.y, bounds.Width(), 0) ||
                IsNear<16>(location.x, location.y, 0, bounds.Height()) ||
                IsNear<16>(location.x, location.y, bounds.Width(), bounds.Height());
            if (isNear)
            {
                ret = HitTestResult::Title;
            }
        }
        else if (ret == HitTestResult::Client)
        {
            ret = HitTestResult::Title;
        }
        return ret;
    }
#pragma endregion

#pragma region Listeners

    //Listeners

    void listBox_ItemLeftButtonDoubleClick(GuiGraphicsComposition* sender, GuiItemMouseEventArgs& e)
    {
        PlayIndex(dataSource->TranslateIndex(listBox->GetSelectedItemIndex()));
    }

    void searchBox_TextChanged(GuiGraphicsComposition* sender, GuiEventArgs& e)
    {
        const auto & text = searchBox->GetText();
        if (StrStrW(text.Buffer(), L"\t") != NULL)
        {
            WString extab(text);
            for (int i = extab.Length() - 1; i >= 0; --i)
            {
                if (extab[i] == L'\t')
                {
                    extab.Remove(i, 1);
                }
            }
            searchBox->SetText(extab);
        }
        else
        {
            if (lastChangeDelay)
            {
                if (lastChangeDelay->GetStatus() == INativeDelay::Pending)
                {
                    lastChangeDelay->Cancel();
                }
            }
            lastChangeDelay = GetApplication()->DelayExecuteInMainThread([=]()
            {
                dataSource->UpdateFilter(searchBox->GetText());
            }, 700);
        }
    }

    void searchBox_gotFocus(GuiGraphicsComposition* sender, GuiEventArgs& e)
    {
        inputing = true;
    }

    void searchBox_lostFocus(GuiGraphicsComposition* sender, GuiEventArgs& e)
    {
        inputing = false;
    }

    void window_WindowLostFocus(GuiGraphicsComposition* sender, GuiEventArgs& e)
    {
        this->NaiveHide();
    }
#pragma endregion

    ~NaiveSearchWindow()
    {

    }
};

GuiWindow * CreateNaiveSearchWindow(SearchWindowStartUpParam * param)
{
    return new NaiveSearchWindow(param);
}
