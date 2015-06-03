#include "stdafx.h"
#include "SearchWindow.h"
#include "NaiveSearchWindow.h"
#include "DataSource.h"
#include "WinampControl.h"

class NaiveSearchWindow sealed :
    public vl::presentation::controls::GuiWindow,
    public IPlugin
{
private:
    DataSource * dataSource;
    GuiVirtualTextList* listBox;
    GuiSinglelineTextBox * searchBox;

    static inline bool IsContorlKeyClean(const NativeWindowKeyInfo& info)
    {
        return !(info.alt || info.ctrl || info.shift);
    }

    vl::Ptr<vl::presentation::INativeDelay> lastChangeDelay;
    SearchWindowStartUpParam* p;

    void NaiveHide()
    {
        auto thiswinf = vl::presentation::windows::GetWindowsForm(this->GetNativeWindow());
        ShowWindow(thiswinf->GetWindowHandle(), SW_HIDE);
    }
public:

    void InitializeComponents()
    {
        this->SetTitleBar(false);
        this->SetTopMost(true);

        this->SetBounds({200, 200, 400, 600});

        GuiTableComposition * table = new GuiTableComposition();
        table->SetRowsAndColumns(2, 1);
        table->SetCellPadding(3);
        table->SetAlignmentToParent({ 0, 0, 0, 0 });

        table->SetRowOption(0, GuiCellOption::AbsoluteOption(30));
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

            cell->AddChild(listBox->GetBoundsComposition());
        }

        this->GetNativeWindow()->HideInTaskBar();
        GetApplication()->DelayExecuteInMainThread([this]()
        {
            this->NaiveHide();
        }, 200);
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
        Show();
    }

    void OnExit() override
    {
        GetApplication()->InvokeLambdaInMainThreadAndWait([=](){ this->Close(); }, 500);
    }

    void OnHotkeyPressed() override
    {
        this->Show();
    }

    void OnListRefresh() override
    {
        GetApplication()->InvokeAsync([=]()
        {
            dataSource->UpdatePlaylist(p->Plugin->hwndParent);
        });
    }
#pragma endregion

#pragma region GuiWindow

    //
    //GuiWindow
    //
    void Show()
    {
        auto app = GetApplication();
        if (app->IsInMainThread())
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
        else
        {
            app->InvokeLambdaInMainThread([=](){ this->Show(); });
        }
    }

    void KeyDown(const NativeWindowKeyInfo& info) override
    {
        if (info.code == VKEY_RETURN && IsContorlKeyClean(info))
        {
            auto index = listBox->GetSelectedItemIndex();
            if (index == -1)
            {
                listBox->SetFocus();
            }
            else
            {
                PlayIndex(dataSource->TranslateIndex(listBox->GetSelectedItemIndex()));
            }
        }
        else if (info.code == VKEY_TAB && IsContorlKeyClean(info))
        {
            auto index = listBox->GetSelectedItemIndex();
            auto count = dataSource->Count();
            if (index != -1)
            {
                listBox->SetSelected(index, false);
            }
            listBox->SetSelected((index + 1) % count, true);
            listBox->SetFocus();
        }
        else if (info.code == VKEY_ESCAPE)
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
                IsNear<16>(location.x, location.y, bounds.x2 - bounds.x1, 0) ||
                IsNear<16>(location.x, location.y, 0, bounds.y2 - bounds.y1) ||
                IsNear<16>(location.x, location.y, bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
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
