#pragma once
using namespace System;
ref class Track
{
public:
	Track(String^ filename, String^ title){ this->Filename = filename; this->Title = title; }
	property String^ Filename;
	property String^ Title;
	String^ ToString() override { return this->Title; };
};

