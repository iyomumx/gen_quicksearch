#include "stdafx.h"
#include "ViewModel.h"

ViewModel::ViewModel()
{
	using namespace System::Xml::Serialization;
	_windowHeight = SystemParameters::WorkArea.Height / 2;
	_windowWidth = SystemParameters::WorkArea.Width / 4;
	_windowLeft = SystemParameters::WorkArea.Width / 4;
	_windowTop = SystemParameters::WorkArea.Height / 4;
	_xs = gcnew XmlSerializer(ViewModel::typeid);
	_xs->UnknownAttribute += dynamic_cast<XmlAttributeEventHandler^>(gcnew EventHandler(this, &ViewModel::OnUnknown));
	_xs->UnknownElement += dynamic_cast<XmlElementEventHandler^>(gcnew EventHandler(this, &ViewModel::OnUnknown));
	_xs->UnknownNode += dynamic_cast<XmlNodeEventHandler^>(gcnew EventHandler(this, &ViewModel::OnUnknown));
}


void ViewModel::Save()
{
	using namespace System::Xml::Serialization;
	using namespace System::IO;
	FileStream^ fs = nullptr;
	try
	{
		fs = File::OpenWrite(this->_savePath);
		_xs->Serialize(fs, this);
	}
	catch (Exception^){}
	finally
	{
		if (fs != nullptr)
		{
			fs->Close();
		}
	}
}


ViewModel ^ ViewModel::Load(String^ path)
{
	using namespace System::Xml::Serialization;
	using namespace System::IO;
	auto result = gcnew ViewModel();
	FileStream^ fs = nullptr;
	try
	{
		fs = File::OpenRead(path);
		result = dynamic_cast<ViewModel^>(result->_xs->Deserialize(fs));
	}
	catch (Exception^){}
	finally
	{
		if (fs != nullptr)
		{
			fs->Close();
		}
		result->_savePath = path;
	}
	return result;
}


void ViewModel::OnPropertyChanged(String ^ propertyName)
{
	using System::Threading::Monitor;
	try
	{
		Monitor::Enter(this);
		this->PropertyChanged(this, gcnew System::ComponentModel::PropertyChangedEventArgs(propertyName));
	}
	finally
	{
		Monitor::Exit(this);
	}
}


void ViewModel::OnUnknown(Object^, EventArgs^) { }
