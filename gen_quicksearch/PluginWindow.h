#pragma once

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Controls;
using namespace System::Windows::Data;
using namespace System::Collections::ObjectModel;

ref class PluginWindow sealed :
public Window
{
public:
	PluginWindow();
	static PluginWindow^ MainWindow;
	static void Init();
	void Invoke(Action^ callback);
	void AsyncInvoke(Action^ callback);
	void ShowAndFocus();
	Action^ SAFcallback;
	void RefreshList();
	Action^ RLcallback;
	volatile bool IsClosing;
private:
	static String^ GetPlayListFile(int index);
	static String^ GetPlayListTitle(int index);
	static int GetListLength();
	static void PlayIndex(int index);
	static void QueueIndex(int index);
	//窗体初始化与事件处理函数
	void InitializeComponent();
	TextBox^ txtFilter;
	ListBox^ lstPlaylist;
	ObservableCollection<Track^>^ Playlist;
	CollectionView^ PlaylistView;
	void OnTextChanged(System::Object ^sender, System::Windows::Controls::TextChangedEventArgs ^e);
	bool Filter(Object^ obj);
protected:
	void OnDeactivated(EventArgs^ e) override;
	void OnKeyUp(System::Object ^sender, System::Windows::Input::KeyEventArgs ^e);
	void OnKeyDown(System::Object ^sender, System::Windows::Input::KeyEventArgs ^e);
	void OnMouseDoubleClick(System::Object ^sender, System::Windows::Input::MouseButtonEventArgs ^e);
	void OnGotFocus(System::Object ^sender, System::Windows::RoutedEventArgs ^e);
	void OnClosing(System::ComponentModel::CancelEventArgs^ e) override;
	void OnSourceInitialized(EventArgs^ e) override;
	void OnMouseLeftButtonDown(System::Windows::Input::MouseButtonEventArgs^ e) override;
};

