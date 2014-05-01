#include "stdafx.h"


PluginWindow::PluginWindow()
{
	InitializeComponent();
}

void PluginWindow::Init(HWND handle)
{
	MainWindow = gcnew PluginWindow();
	MainWindow->MainHandle = handle;
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
	//txtFilter->Height = System::Double::NaN;
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
}

void PluginWindow::OnTextChanged(System::Object ^sender, TextChangedEventArgs ^e)
{
	PlaylistView->Refresh();
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


void PluginWindow::Invoke(Action^ callback)
{
	this->Dispatcher->Invoke(System::Windows::Threading::DispatcherPriority::Normal, callback);
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

void PluginWindow::OnDeactivated(EventArgs^ e)
{
	this->Hide();
}

void PluginWindow::OnKeyUp(System::Object ^sender, System::Windows::Input::KeyEventArgs ^e)
{
	if (e->Key == System::Windows::Input::Key::Enter)
	{
		PlaylistView->MoveCurrentToFirst();
		PlayIndex(Playlist->IndexOf((Track^)PlaylistView->CurrentItem));
		this->Hide();
	}
}


void PluginWindow::OnKeyDown(System::Object ^sender, System::Windows::Input::KeyEventArgs ^e)
{
	if (e->Key == System::Windows::Input::Key::Enter)
	{
		PlayIndex(Playlist->IndexOf((Track^)lstPlaylist->SelectedItem));
		this->Hide();
	}
	else if (e->Key == System::Windows::Input::Key::Tab)
	{
		if (!(PlaylistView->MoveCurrentToNext()))
		{
			PlaylistView->MoveCurrentToFirst();
		}
		lstPlaylist->SelectedItem = PlaylistView->CurrentItem;
		e->Handled = true;
	}
	else if (e->Key == System::Windows::Input::Key::Q)
	{
		QueueIndex(Playlist->IndexOf((Track^)lstPlaylist->SelectedItem));
		this->Hide();
	}
}


void PluginWindow::OnMouseDoubleClick(System::Object ^sender, System::Windows::Input::MouseButtonEventArgs ^e)
{
	PlayIndex(Playlist->IndexOf((Track^)lstPlaylist->SelectedItem));
	this->Hide();
}


void PluginWindow::OnGotFocus(System::Object ^sender, System::Windows::RoutedEventArgs ^e)
{
	if (lstPlaylist->SelectedIndex == -1)
	{
		if (PlaylistView->CurrentPosition == -1)
		{
			PlaylistView->MoveCurrentToFirst();
		}
		lstPlaylist->SelectedItem = PlaylistView->CurrentItem;
	}
}

void PluginWindow::ShowAndFocus()
{
	Show();
	Activate();
	txtFilter->Text = String::Empty;
	txtFilter->Focus();
	PlaylistView->MoveCurrentToFirst();
	lstPlaylist->SelectedItem = PlaylistView->CurrentItem;
}

void PluginWindow::AsyncInvoke(Action^ callback)
{
	this->Dispatcher->BeginInvoke(System::Windows::Threading::DispatcherPriority::Normal, callback);
}

void PluginWindow::QueueIndex(int index)
{
	if (QueueApi != NULL)
	{
		QueueApi->AddItemToQueue(index, 1, NULL);
	}
}