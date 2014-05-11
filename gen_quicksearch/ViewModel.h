#pragma once

#define NOTIFY_CHANGED_PROPERTY(TYPE,NAME,INNER) property TYPE NAME \
												 { \
												 TYPE get() { return INNER; }\
												 void set(TYPE value) \
													{\
													if (INNER != value)\
														{\
														INNER = value;\
														OnPropertyChanged(#NAME);\
														}\
													}\
												 }

using System::Xml::Serialization::XmlRootAttribute;
using System::Xml::Serialization::XmlIgnoreAttribute;

[XmlRootAttribute("QuickSearchSettings")]
public ref class ViewModel : System::ComponentModel::INotifyPropertyChanged
{
public:
	void Save();
	static ViewModel ^ Load(String^ path);
	virtual event System::ComponentModel::PropertyChangedEventHandler^ PropertyChanged;

	NOTIFY_CHANGED_PROPERTY(double, WindowHeight, _windowHeight);
	NOTIFY_CHANGED_PROPERTY(double, WindowWidth, _windowWidth);
	NOTIFY_CHANGED_PROPERTY(double, WindowLeft, _windowLeft);
	NOTIFY_CHANGED_PROPERTY(double, WindowTop, _windowTop);
	NOTIFY_CHANGED_PROPERTY(bool, UseRegex, _useRegex);
	//Runtime Only
	[XmlIgnoreAttribute()]
	NOTIFY_CHANGED_PROPERTY(bool, OnSetting, _onSetting);
	[XmlIgnoreAttribute()]
	NOTIFY_CHANGED_PROPERTY(String^, FilterString, _filterString);
private:
	ViewModel();
	double _windowHeight;
	double _windowWidth;
	String ^ _savePath;
	System::Xml::Serialization::XmlSerializer ^ _xs;
	double _windowLeft;
	double _windowTop;
	bool _useRegex;
	void OnPropertyChanged(String ^ propertyName);
	void OnUnknown(System::Object^, EventArgs^);
	bool _onSetting;
	String^ _filterString;
};
#undef NOTIFY_CHANGED_PROPERTY
