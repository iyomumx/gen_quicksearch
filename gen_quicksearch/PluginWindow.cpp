#include "stdafx.h"
#include "PluginWindow.h"

#define SETSETTING(window,b) window->_viewModel->OnSetting = b
#define HIDE(window) (SETSETTING(window, false)),((window)->Visibility = System::Windows::Visibility::Collapsed)
#define ISVISIBLE(window) ((window)->Visibility == System::Windows::Visibility::Visible)
#define INIT_BINDING(BINDING,PATH) BINDING=gcnew System::Windows::Data::Binding(PATH);BINDING->Mode = System::Windows::Data::BindingMode::TwoWay;BINDING->NotifyOnSourceUpdated = true;BINDING->NotifyOnTargetUpdated = true

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
	this->_viewModel = ViewModel::Load(System::IO::Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData), "Winamp", "gen_quicksearch.xml"));
	this->DataContext = this->_viewModel;
	auto INIT_BINDING(b, "WindowHeight");
	this->SetBinding(Window::HeightProperty, b);
	INIT_BINDING(b, "WindowWidth");
	this->SetBinding(Window::WidthProperty, b);
	INIT_BINDING(b, "WindowTop");
	this->SetBinding(Window::TopProperty, b);
	INIT_BINDING(b, "WindowLeft");
	this->SetBinding(Window::LeftProperty, b);
	this->Topmost = true;
	this->ShowInTaskbar = false;
	this->Visibility = System::Windows::Visibility::Hidden;
	this->WindowStyle = System::Windows::WindowStyle::None;
    this->ResizeMode = System::Windows::ResizeMode::NoResize;
	this->PreviewKeyDown += gcnew System::Windows::Input::KeyEventHandler(this, &PluginWindow::window_PreviewKeyDown);
	this->IsClosing = false;

	Grid ^ grid;
	{
		using System::Resources::ResourceManager;
		auto rm = gcnew ResourceManager(this->GetType());
		auto xr = dynamic_cast<ResourceDictionary^>(System::Windows::Markup::XamlReader::Parse(rm->GetObject("Resource.xaml")->ToString()));
		grid = dynamic_cast<Grid^>(xr["grid"]);
	}
	for each (Object^ o in grid->Children)
	{
		if (o->GetType() == TextBox::typeid)
		{
			txtFilter = dynamic_cast<TextBox^>(o);
		}
		else if (o->GetType() == ListBox::typeid)
		{
			lstPlaylist = dynamic_cast<ListBox^>(o);
		}
	}

	//子控件

	this->Content = grid;
	Playlist = gcnew ObservableCollection<Track^>();
	PlaylistView = safe_cast<CollectionView^>(CollectionViewSource::GetDefaultView(Playlist));

	txtFilter->Height = txtFilter->FontSize * txtFilter->FontFamily->LineSpacing * 1.25;
	txtFilter->TextChanged += gcnew TextChangedEventHandler(this, &PluginWindow::txtFilter_TextChanged);
	txtFilter->KeyDown += gcnew System::Windows::Input::KeyEventHandler(this, &PluginWindow::txtFilter_KeyDown);

	lstPlaylist->ItemsSource = this->Playlist;
	lstPlaylist->KeyDown += gcnew System::Windows::Input::KeyEventHandler(this, &PluginWindow::lstPlaylist_KeyDown);
	lstPlaylist->MouseDoubleClick += gcnew System::Windows::Input::MouseButtonEventHandler(this, &PluginWindow::lstPlaylist_MouseDoubleClick);
	PlaylistView->Filter = gcnew Predicate<Object^>(this, &PluginWindow::Filter);

	SAFcallback = gcnew Action(this, &PluginWindow::ShowAndFocus);
	RLcallback = gcnew Action(this, &PluginWindow::RefreshList);
	this->PlaylistLock = gcnew Object();
	this->EndInit();
	this->ResetRegex();
	this->RefreshList();
	//非托管项设置，延迟到此处避免JTFE未加载而无法获得API接口
	factory = WASABI_API_SVC->service_getServiceByGuid(QueueManagerApiGUID);
	if (factory)
	{
        QueueApi = reinterpret_cast<api_queue *>(factory->getInterface());
	}
	else
	{
		QueueApi = NULL;
	}
}

bool PluginWindow::Filter(Object^ obj)
{
	Track^ t = dynamic_cast<Track^>(obj);
	if (this->_viewModel->UseRegex)
	{
        if (t != nullptr)
        {
            if (filterRegex == nullptr || filterRegex->Match(t->Filename)->Success || filterRegex->Match(t->Title)->Success)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (filterRegex == nullptr || filterRegex->Match((obj ? obj : String::Empty)->ToString())->Success)
            {
                return true;
            }
            else
            {
                return false;
            }
        }		
	}
	else
	{
		if (t != nullptr)
		{
			return
				t->Filename->ToUpper()->Contains(this->_viewModel->FilterString->ToUpper()) ||
				t->Title->ToUpper()->Contains(this->_viewModel->FilterString->ToUpper());
		}
		else
		{
			return (obj ? obj : String::Empty)->ToString()->Contains(txtFilter->Text);
		}
	}
}

void PluginWindow::RefreshList()
{
	using System::Threading::Monitor;
	bool acquried = false;
	try
	{
		if (PlaylistLock != nullptr)
		{
			Monitor::TryEnter(PlaylistLock, 0, acquried);
		}
		if (acquried == true)
		{
			int i, length = GetListLength();
			Action<Track^>^ add = gcnew Action<Track^>(Playlist, &ObservableCollection<Track^>::Add);
			this->Invoke(gcnew Action(Playlist, &ObservableCollection<Track^>::Clear));
			for (i = 0; i < length; i++)
			{
				this->AsyncInvoke(add, gcnew Track(GetPlayListFile(i), GetPlayListTitle(i)));
			}
		}
	}
	finally
	{
		if (acquried == true)
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
	else
	{
		this->_viewModel->Save();
	}
}

void PluginWindow::txtFilter_TextChanged(System::Object ^sender, TextChangedEventArgs ^e)
{
	if (Visibility != System::Windows::Visibility::Visible) return;
	ResetRegex();
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
        PlayIndex(Playlist->IndexOf(safe_cast<Track^>(PlaylistView->CurrentItem)));
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
        PlayIndex(Playlist->IndexOf(safe_cast<Track^>(lstPlaylist->SelectedItem)));
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
		QueueIndex(Playlist->IndexOf(safe_cast<Track^>(lstPlaylist->SelectedItem)));
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
			if (Track^ t = dynamic_cast<Track^>((dynamic_cast<ListBoxItem^>(obj))->DataContext))
			{
				PlayIndex(Playlist->IndexOf(t));
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
	lstPlaylist->ScrollIntoView(lstPlaylist->SelectedItem);
}

void PluginWindow::ShowSetting()
{
	SETSETTING(this, true);
	ShowAndFocus();
	this->Focus();
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
	return gcnew String(reinterpret_cast<wchar_t*>(SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)index, IPC_GETPLAYLISTFILEW)));
}

String ^ PluginWindow::GetPlayListTitle(int index)
{
    return gcnew String(reinterpret_cast<wchar_t*>(SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)index, IPC_GETPLAYLISTTITLEW)));
}

int PluginWindow::GetListLength()
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
}

void PluginWindow::PlayIndex(int index)
{
	if (SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING) != 1)
	{
		SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
	}
	SendMessage(plugin.hwndParent, WM_WA_IPC, static_cast<WPARAM>( index ), IPC_SETPLAYLISTPOS);
	SendMessage(plugin.hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2, 0), 0);
}


void PluginWindow::ResetRegex()
{
	using namespace System::Text::RegularExpressions;
	try
	{
		this->filterRegex = gcnew Regex(this->_viewModel->FilterString, RegexOptions::IgnoreCase);
	}
	catch (Exception^)
	{
		this->filterRegex = nullptr;
	}
}
