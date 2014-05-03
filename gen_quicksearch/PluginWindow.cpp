#include "stdafx.h"

#define HIDE(window) (window)->Visibility = System::Windows::Visibility::Collapsed
#define ISVISIBLE(window) ((window)->Visibility == System::Windows::Visibility::Visible)

PluginWindow::PluginWindow()
{
	InitializeComponent();
}

void PluginWindow::Init()
{
	MainWindow = gcnew PluginWindow();
	Application ^ app = Application::Current;
	if (app == nullptr)
	{
		app = gcnew Application();
	}
	app->Run(MainWindow);
}

void PluginWindow::InitializeComponent()
{
	//初始化与组件设定
	//窗体属性
	this->Height = SystemParameters::WorkArea.Height / 2;
	this->Width = SystemParameters::WorkArea.Width / 4;
	this->Topmost = true;
	this->ShowInTaskbar = false;
	this->Visibility = System::Windows::Visibility::Hidden;
	this->WindowStyle = System::Windows::WindowStyle::None;
	this->ResizeMode = System::Windows::ResizeMode::NoResize;

	this->IsClosing = false;
	//子控件
	Grid ^ grid = gcnew Grid();
	this->Content = grid;
	grid->Margin = *gcnew Thickness(2);

	RowDefinition ^ rd;
	grid->RowDefinitions->Add(rd = gcnew RowDefinition());
	rd->Height = GridLength::Auto;
	grid->RowDefinitions->Add(rd = gcnew RowDefinition());
	rd->Height = *gcnew GridLength(1, GridUnitType::Star);

	txtFilter = gcnew TextBox();
	lstPlaylist = gcnew ListBox();
	Playlist = gcnew ObservableCollection<Track^>();
	PlaylistView = (CollectionView^)CollectionViewSource::GetDefaultView(Playlist);

	grid->Children->Add(txtFilter);
	txtFilter->SetValue(Grid::RowProperty, (Object^)0);
	txtFilter->Margin = *gcnew Thickness(1);
	txtFilter->Height = txtFilter->FontSize + 10;
	txtFilter->TextWrapping = TextWrapping::NoWrap;
	txtFilter->TabIndex = 0;
	txtFilter->TextChanged += gcnew TextChangedEventHandler(this, &PluginWindow::OnTextChanged);
	txtFilter->KeyUp += gcnew System::Windows::Input::KeyEventHandler(this, &PluginWindow::OnKeyUp);

	grid->Children->Add(lstPlaylist);
	lstPlaylist->SetValue(Grid::RowProperty, (Object^)1);
	lstPlaylist->Margin = *gcnew Thickness(1);
	lstPlaylist->ItemsSource = this->Playlist;
	lstPlaylist->TabIndex = 1;
	lstPlaylist->KeyDown += gcnew System::Windows::Input::KeyEventHandler(this, &PluginWindow::OnKeyDown);
	lstPlaylist->MouseDoubleClick += gcnew System::Windows::Input::MouseButtonEventHandler(this, &PluginWindow::OnMouseDoubleClick);
	lstPlaylist->GotFocus += gcnew System::Windows::RoutedEventHandler(this, &PluginWindow::OnGotFocus);
	PlaylistView->Filter = gcnew Predicate<Object^>(this, &PluginWindow::Filter);

	this->RefreshList();
	//非托管项设置，延迟到此处避免JTFE未加载而无法获得API接口
	factory = WASABI_API_SVC->service_getServiceByGuid(QueueManagerApiGUID);
	if (factory)
	{
		QueueApi = (api_queue *)factory->getInterface();
	}
	else
	{
		QueueApi = NULL;
	}
	SAFcallback = gcnew Action(this, &PluginWindow::ShowAndFocus);
	RLcallback = gcnew Action(this, &PluginWindow::RefreshList);
}

bool PluginWindow::Filter(Object^ obj)
{
	if ((obj != nullptr) && (obj->GetType() == Track::typeid))
	{
		Track^ t = (Track^)obj;
		return	t->Filename->ToUpper()->Contains(txtFilter->Text->ToUpper()) ||
			t->Title->ToUpper()->Contains(txtFilter->Text->ToUpper());
	}
	return obj->ToString()->Contains(txtFilter->Text);
}

void PluginWindow::RefreshList()
{
	int i, length = GetListLength();
	Playlist->Clear();
	for (i = 0; i < length; i++)
	{
		Playlist->Add(gcnew Track(GetPlayListFile(i), GetPlayListTitle(i)));
	}
}

void PluginWindow::OnSourceInitialized(EventArgs^ e)
{
	Window::OnSourceInitialized(e);
	HIDE(this);
}

void PluginWindow::OnDeactivated(EventArgs^ e)
{
	Window::OnActivated(e);
	HIDE(this);
}

void PluginWindow::OnMouseLeftButtonDown(System::Windows::Input::MouseButtonEventArgs^ e)
{
	Window::OnMouseLeftButtonDown(e);
	DragMove();
}

void PluginWindow::OnClosing(System::ComponentModel::CancelEventArgs ^e)
{
	Window::OnClosing(e);
	if (!IsClosing)
	{
		e->Cancel = true;
		HIDE(this);
	}
}

void PluginWindow::OnTextChanged(System::Object ^sender, TextChangedEventArgs ^e)
{
	if (Visibility != System::Windows::Visibility::Visible) return;
	PlaylistView->Refresh();
}

//处理txtFilter中的按键事件
void PluginWindow::OnKeyUp(System::Object ^sender, System::Windows::Input::KeyEventArgs ^e)
{
	if (!ISVISIBLE(this)) return;
	if (e->Key == System::Windows::Input::Key::Enter)
	{
		if (PlaylistView->Count <= 0) return;
		PlaylistView->MoveCurrentToFirst();
		PlayIndex(Playlist->IndexOf((Track^)PlaylistView->CurrentItem));
		HIDE(this);
	}
	else if (e->Key == System::Windows::Input::Key::Escape)
	{
		HIDE(this);
	}
}

//处理lstPlaylist中的按键事件
void PluginWindow::OnKeyDown(System::Object ^sender, System::Windows::Input::KeyEventArgs ^e)
{
	if (!ISVISIBLE(this)) return;
	if (e->Key == System::Windows::Input::Key::Enter)
	{
		PlayIndex(Playlist->IndexOf((Track^)lstPlaylist->SelectedItem));
		HIDE(this);
	}
	else if (e->Key == System::Windows::Input::Key::Tab)
	{
		if (!(PlaylistView->MoveCurrentToNext()))
		{
			PlaylistView->MoveCurrentToFirst();
		}
		lstPlaylist->SelectedItem = PlaylistView->CurrentItem;
		lstPlaylist->ScrollIntoView(lstPlaylist->SelectedItem);
		e->Handled = true;
	}
	else if (e->Key == System::Windows::Input::Key::Q)
	{
		QueueIndex(Playlist->IndexOf((Track^)lstPlaylist->SelectedItem));
		HIDE(this);
	}
	else if (e->Key == System::Windows::Input::Key::Escape)
	{
		HIDE(this);
	}
}

//鼠标双击时，播放选定项。由于鼠标第一次按下会选中其位置所在项，此处不确保选中。
void PluginWindow::OnMouseDoubleClick(System::Object ^sender, System::Windows::Input::MouseButtonEventArgs ^e)
{
	PlayIndex(Playlist->IndexOf((Track^)lstPlaylist->SelectedItem));
	HIDE(this);
}

//焦点切换到lstPlaylist时的事件处理
void PluginWindow::OnGotFocus(System::Object ^sender, System::Windows::RoutedEventArgs ^e)
{
	if (!ISVISIBLE(this)) return;
	if (lstPlaylist->SelectedIndex == -1)
	{
		if (PlaylistView->CurrentPosition == -1)
		{
			PlaylistView->MoveCurrentToFirst();
		}
		if (PlaylistView->CurrentItem != nullptr)
		{
			lstPlaylist->SelectedItem = PlaylistView->CurrentItem;
		}
		else
		{
			return;
		}
	}
	lstPlaylist->ScrollIntoView(lstPlaylist->SelectedItem);
}

void PluginWindow::ShowAndFocus()
{
	if (ISVISIBLE(this)) return;
	this->Visibility = System::Windows::Visibility::Visible;
	Activate();
	txtFilter->Focus();
	txtFilter->Text = String::Empty;
	PlaylistView->MoveCurrentToFirst();
	lstPlaylist->SelectedItem = PlaylistView->CurrentItem;
}

void PluginWindow::AsyncInvoke(Action^ callback)
{
	if (callback == nullptr) return;
	this->Dispatcher->BeginInvoke(System::Windows::Threading::DispatcherPriority::Normal, callback);
}

void PluginWindow::Invoke(Action^ callback)
{
	if (callback == nullptr) return;
	this->Dispatcher->Invoke(System::Windows::Threading::DispatcherPriority::Normal, callback);
}

//WINAMP API部分
void PluginWindow::QueueIndex(int index)
{
	if (QueueApi != NULL)
	{
		QueueApi->AddItemToQueue(index, 1, NULL);
	}
}

String ^ PluginWindow::GetPlayListFile(int index)
{
	return gcnew String((wchar_t*)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)index, IPC_GETPLAYLISTFILEW));
}

String ^ PluginWindow::GetPlayListTitle(int index)
{
	return gcnew String((wchar_t*)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)index, IPC_GETPLAYLISTTITLEW));
}

int PluginWindow::GetListLength()
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
}

void PluginWindow::PlayIndex(int index)
{
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)index, IPC_SETPLAYLISTPOS);
	SendMessage(plugin.hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2, 0), 0);
}