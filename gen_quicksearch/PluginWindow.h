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
	static void Init(HWND);
	void Invoke(Action^ callback);
	void ShowAndFocus();
	void RefreshList();
	static HWND hwnd_winamp;
private:
	HWND MainHandle;
	//WINAMP API部分
	static String ^ GetPlayListFile(int index)
	{
		return gcnew String((wchar_t*)SendMessage(hwnd_winamp, WM_WA_IPC, index, IPC_GETPLAYLISTFILEW));
	};
	static String ^ GetPlayListTitle(int index)
	{
		return gcnew String((wchar_t*)SendMessage(hwnd_winamp, WM_WA_IPC, index, IPC_GETPLAYLISTTITLEW));
	};
	static int GetListLength()
	{
		return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
	};
	static void PlayIndex(int index)
	{
		SendMessage(hwnd_winamp, WM_WA_IPC, index, IPC_SETPLAYLISTPOS);
		SendMessage(hwnd_winamp, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2, 0), 0);
	};
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
public:
	void AsyncInvoke(Action^ callback);
};

