#include "stdafx.h"
#include "PluginWindow.h"

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
	this->BeginInit();
	this->Height = SystemParameters::WorkArea.Height / 2;
	this->Width = SystemParameters::WorkArea.Width / 4;
	this->Topmost = true;
	this->ShowInTaskbar = false;
	this->Visibility = System::Windows::Visibility::Hidden;
	this->WindowStyle = System::Windows::WindowStyle::None;
	this->ResizeMode = System::Windows::ResizeMode::NoResize;
	this->PreviewKeyDown += gcnew System::Windows::Input::KeyEventHandler(this, &PluginWindow::window_PreviewKeyDown);
	this->IsClosing = false;
	//子控件
	Grid ^ grid = gcnew Grid();
	grid->BeginInit();
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

	txtFilter->BeginInit();
	grid->Children->Add(txtFilter);
	txtFilter->SetValue(Grid::RowProperty, (Object^)0);
	txtFilter->Margin = *gcnew Thickness(1);
	txtFilter->Height = txtFilter->FontSize * txtFilter->FontFamily->LineSpacing * 1.25;
	txtFilter->TextWrapping = TextWrapping::NoWrap;
	txtFilter->TabIndex = 0;
	txtFilter->TextChanged += gcnew TextChangedEventHandler(this, &PluginWindow::txtFilter_TextChanged);
	txtFilter->KeyDown += gcnew System::Windows::Input::KeyEventHandler(this, &PluginWindow::txtFilter_KeyDown);
	txtFilter->EndInit();

	lstPlaylist->BeginInit();
	grid->Children->Add(lstPlaylist);
	lstPlaylist->SetValue(Grid::RowProperty, (Object^)1);
	lstPlaylist->Margin = *gcnew Thickness(1);
	lstPlaylist->ItemsSource = this->Playlist;
	lstPlaylist->TabIndex = 1;
	lstPlaylist->KeyDown += gcnew System::Windows::Input::KeyEventHandler(this, &PluginWindow::lstPlaylist_KeyDown);
	lstPlaylist->MouseDoubleClick += gcnew System::Windows::Input::MouseButtonEventHandler(this, &PluginWindow::lstPlaylist_MouseDoubleClick);
	PlaylistView->Filter = gcnew Predicate<Object^>(this, &PluginWindow::Filter);
	lstPlaylist->EndInit();

	grid->EndInit();

	SAFcallback = gcnew Action(this, &PluginWindow::ShowAndFocus);
	RLcallback = gcnew Action(this, &PluginWindow::RefreshList);
	this->PlaylistLock = gcnew Object();
	this->EndInit();
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
	using System::Threading::Monitor;
	if ((PlaylistLock != nullptr) && (Monitor::TryEnter(PlaylistLock, 0)))
	{
		try
		{
			int i, length = GetListLength();
			Action<Track^>^ add = gcnew Action<Track^>(Playlist, &ObservableCollection<Track^>::Add);
			this->Invoke(gcnew Action(Playlist, &ObservableCollection<Track^>::Clear));
			for (i = 0; i < length; i++)
			{
				this->AsyncInvoke(add, gcnew Track(GetPlayListFile(i), GetPlayListTitle(i)));
			}
		}
		finally
		{
			Monitor::Exit(PlaylistLock);
		}
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

void PluginWindow::txtFilter_TextChanged(System::Object ^sender, TextChangedEventArgs ^e)
{
	if (Visibility != System::Windows::Visibility::Visible) return;
	PlaylistView->Refresh();
}

//处理txtFilter中的按键事件
void PluginWindow::txtFilter_KeyDown(System::Object ^sender, System::Windows::Input::KeyEventArgs ^e)
{
	if (!ISVISIBLE(this)) return;
	if (e->Key == System::Windows::Input::Key::Enter)
	{
		if (PlaylistView->Count <= 0) return;
		PlaylistView->MoveCurrentToFirst();
		PlayIndex(Playlist->IndexOf((Track^)PlaylistView->CurrentItem));
		HIDE(this);
	}
	else if (e->Key == System::Windows::Input::Key::Tab)
	{
		if (PlaylistView->Count == 0)
		{
			e->Handled = true;
			System::Media::SystemSounds::Question->Play();
			return;
		}
		PlaylistView->MoveCurrentToFirst();
		if (PlaylistView->CurrentItem != nullptr)
		{
			lstPlaylist->SelectedItem = PlaylistView->CurrentItem;
		}
		else
		{
			return;
		}
		lstPlaylist->ScrollIntoView(lstPlaylist->SelectedItem);
	}
}

//处理lstPlaylist中的按键事件
void PluginWindow::lstPlaylist_KeyDown(System::Object ^sender, System::Windows::Input::KeyEventArgs ^e)
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
}

//鼠标双击时，播放选定项。
void PluginWindow::lstPlaylist_MouseDoubleClick(System::Object ^sender, System::Windows::Input::MouseButtonEventArgs ^e)
{
	DependencyObject^ obj = dynamic_cast<DependencyObject^>(e->OriginalSource);
	while ((obj != nullptr) && (obj != lstPlaylist))
	{
		if (obj->GetType() == ListBoxItem::typeid)
		{
			Object^ t = (dynamic_cast<ListBoxItem^>(obj))->DataContext;
			if (t->GetType() == Track::typeid)
			{
				PlayIndex(Playlist->IndexOf(dynamic_cast<Track^>(t)));
				HIDE(this);
			}
			break;
		}
		obj = System::Windows::Media::VisualTreeHelper::GetParent(obj);
	}
}

void PluginWindow::window_PreviewKeyDown(System::Object ^sender, System::Windows::Input::KeyEventArgs ^e)
{
	if (e->Key == System::Windows::Input::Key::Escape)
	{
		e->Handled = true;
		HIDE(this);
	}
	else if (e->Key == System::Windows::Input::Key::F5)
	{
		e->Handled = true;
		RLcallback->BeginInvoke(nullptr, nullptr);
	}
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

generic <typename T>void PluginWindow::AsyncInvoke(Action<T>^ callback, T arg)
{
	if (callback == nullptr) return;
	this->Dispatcher->BeginInvoke(System::Windows::Threading::DispatcherPriority::Normal, callback, arg);
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