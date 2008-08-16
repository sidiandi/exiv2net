// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2008 Andreas Grimme  <andreas.grimme@gmx.net>
 *
 * This program is part of the Exiv2Net distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

#include "stdafx.h"
#include "Image.h"
#include "Tags.h"
#include "exivsimple.h"
#include "ExifUtil.h"
#include "Image.h"
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/iptc.hpp>

using namespace Exiv2Net;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;
using namespace System::Collections::Generic;

String^ ToClrString(std::string const& x)
{
	return Marshal::PtrToStringAnsi((IntPtr)(char*)x.c_str());
}

std::string ToStdString(String^ x)
{
	return std::string(((const char*) Marshal::StringToHGlobalAnsi(x).ToPointer()));
}

template<class T>
array<T>^ ToArray(const Exiv2::Value& val)
{
	const Exiv2::ValueType<T>& vt = (const Exiv2::ValueType<T>&) val;
	return ToArray(vt.value_);
}

template<class T>
array<T>^ ToArray(const std::vector<T>& val)
{
	const int iend = val.size();
	array<T>^ a = gcnew array<T>(iend);
	for (int i=0; i< iend; ++i)
	{
		a[i] = val[i];
	}
	return a;
}

Exiv2Net::Value^ ToValue(const Exiv2::Value& val)
{
	switch (val.typeId())
	{
	case Exiv2::unsignedByte:
		{
			const Exiv2::DataValue& v = (const Exiv2::DataValue&) val;
			return gcnew Exiv2Net::UnsignedByte(ToArray(v.value_));
		}
		break;
	case Exiv2::asciiString:
		return gcnew Exiv2Net::AsciiString(ToClrString(val.toString()));
		break;
	case Exiv2::unsignedShort:
		return gcnew Exiv2Net::UnsignedShort(ToArray<unsigned short>(val));
		break;
	case Exiv2::unsignedLong:
		return gcnew Exiv2Net::UnsignedLong(ToArray<unsigned int>(val));
		break;
	case Exiv2::unsignedRational:
		{
			const Exiv2::ValueType<Exiv2::URational>& v = (const Exiv2::ValueType<Exiv2::URational>&) val;
			array<Exiv2Net::UnsignedRational::Element>^ a = gcnew array<Exiv2Net::UnsignedRational::Element>(v.value_.size());
			const int iend = v.value_.size();
			for (int i=0; i<iend; ++i)
			{
				a[i].Nominator = v.value_[i].first;
				a[i].Denominator = v.value_[i].second;
			}
			return gcnew Exiv2Net::UnsignedRational(a);
		}
		break;
	case Exiv2::signedByte:
		{
			const Exiv2::DataValue& v = (const Exiv2::DataValue&) val;
			const int iend = v.value_.size();
			array<signed char>^ a = gcnew array<signed char>(iend);
			for (int i=0; i<iend; ++i)
			{
				a[i] = v.value_[i];
			}
			return gcnew Exiv2Net::SignedByte(a);
		}
		break;
	case Exiv2::undefined:
		{
			try
			{
				const Exiv2::DataValue& v = (const Exiv2::DataValue&) val;
				return gcnew Exiv2Net::Undefined(ToArray(v.value_));
			}
			catch (...)
			{
				std::cout << val.toString();
				return gcnew Exiv2Net::Undefined(nullptr);
			}
		}
		break;
	case Exiv2::signedShort:
		return gcnew Exiv2Net::SignedShort(ToArray<signed short>(val));
		break;
	case Exiv2::signedLong:
		return gcnew Exiv2Net::SignedLong(ToArray<signed int>(val));
		break;
	case Exiv2::signedRational:
		{
			const Exiv2::ValueType<Exiv2::Rational>& v = (const Exiv2::ValueType<Exiv2::Rational>&) val;
			array<Exiv2Net::SignedRational::Element>^ a = gcnew array<Exiv2Net::SignedRational::Element>(v.value_.size());
			const int iend = v.value_.size();
			for (int i=0; i<iend; ++i)
			{
				a[i].Nominator = v.value_[i].first;
				a[i].Denominator = v.value_[i].second;
			}
			return gcnew Exiv2Net::SignedRational(a);
		}
		break;
	}
	throw gcnew IndexOutOfRangeException((gcnew System::Int32(val.typeId()))->ToString());
}

template<class ElementType>
void ToStdVector(array<ElementType>^ x, std::vector<ElementType>& y)
{
	int iend = x->Length;
	y.resize(iend);
	for (int i=0; i<iend; ++i)
	{
		y[i] = x[i];
	}
}

template<class SourceElementType, class DestElementType>
void ToStdVectorRational(array<SourceElementType>^ x, std::vector<DestElementType>& y)
{
	int iend = x->Length;
	y.resize(iend);
	for (int i=0; i<iend; ++i)
	{
		y[i].first = x[i].Nominator;
		y[i].second = x[i].Denominator;
	}
}

template<class ClrValueType, class ElementType>
Exiv2::Value::AutoPtr ToValueType(Value^value)
{
	ClrValueType^ x = (ClrValueType^) value;
	Exiv2::ValueType<ElementType>* vt = new Exiv2::ValueType<ElementType>();
	ToStdVector(x->Value, vt->value_);
	return Exiv2::Value::AutoPtr(vt);
}

template<class ClrValueType, class ElementType>
Exiv2::Value::AutoPtr ToValueTypeRational(Value^value)
{
	ClrValueType^ x = (ClrValueType^) value;
	Exiv2::ValueType<ElementType>* vt = new Exiv2::ValueType<ElementType>();
	ToStdVectorRational<ClrValueType::Element, ElementType>(x->Value, vt->value_);
	return Exiv2::Value::AutoPtr(vt);
}

Exiv2::Value::AutoPtr ToExif2Value(Value^ value)
{
	Type^ type = value->GetType();

	if (type == UnsignedByte::typeid)
	{
		UnsignedByte^ x = (UnsignedByte^) value;
		pin_ptr<unsigned char> buf = &x->Value[0];
		return Exiv2::Value::AutoPtr(new Exiv2::DataValue(buf , x->Value->Length, Exiv2::invalidByteOrder, Exiv2::unsignedByte));
	}
	if (type == AsciiString::typeid)
	{
		AsciiString^ x = (AsciiString^) value;
		return Exiv2::Value::AutoPtr(new Exiv2::AsciiValue(ToStdString(x->Value)));
	}
	if (type == UnsignedShort::typeid)
	{
		return ToValueType<UnsignedShort, unsigned short>(value);
	}
	if (type == UnsignedLong::typeid)
	{
		return ToValueType<UnsignedLong, unsigned int>(value);
	}
	if (type == UnsignedRational::typeid)
	{
		return ToValueTypeRational<UnsignedRational, Exiv2::URational>(value);
	}
	if (type == SignedByte::typeid)
	{
		SignedByte^ x = (SignedByte^) value;
		pin_ptr<signed char> buf = &x->Value[0];
		return Exiv2::Value::AutoPtr(new Exiv2::DataValue((unsigned char*) buf , x->Value->Length, Exiv2::invalidByteOrder, Exiv2::signedByte));
	}
	if (type == Undefined::typeid)
	{
		Undefined^ x = (Undefined^) value;
		pin_ptr<unsigned char> buf = &x->Value[0];
		return Exiv2::Value::AutoPtr(new Exiv2::DataValue(buf , x->Value->Length, Exiv2::invalidByteOrder, Exiv2::undefined));
	}
	if (type == SignedShort::typeid)
	{
		return ToValueType<SignedShort, signed short>(value);
	}
	if (type == SignedLong::typeid)
	{
		return ToValueType<SignedLong, signed int>(value);
	}
	if (type == SignedRational::typeid)
	{
		return ToValueTypeRational<SignedRational, Exiv2::Rational>(value);
	}
	throw gcnew ArgumentException(value->ToString());
}

struct ImageWrapper
{
    Exiv2::Image::AutoPtr image;
};

Image::Image(String^ file)
{
	hImage = 0;
	OpenFile(file);
}

String^ Image::FileName::get()
{
	return m_fileName;
}

Image::Image()
{
	hImage = 0;
}

Image::~Image()
{
	Free();
}

void Image::OpenFile(String^ file)
{
	Free();
	m_fileName = file;
	hImage = ::OpenFileImage((const char*) Marshal::StringToHGlobalAnsi(file).ToPointer());
	modified = false;
}

void Image::OpenMem(array<Byte>^ data)
{
	// todo
}

void Image::Free()
{
	if (hImage)
	{
		FreeImage(hImage);
		hImage = 0;
		m_fileName = String::Empty;
	}
}

void Image::Save()
{
	SaveImage(hImage);
	modified = false;
}

int Image::ImageSize()
{
	return ::ImageSize(hImage);
}

array<Byte^>^ Image::ImageData()
{
	int size = ImageSize();
	array<Byte^>^ data = gcnew array<Byte^>(size);
	// todo ::ImageData(hImage, 0, size);
	return data;
}

class NativeIterator
{
public:
	int phase;
	Exiv2::IptcData::const_iterator iend;
    Exiv2::IptcData::const_iterator i;
    Exiv2::ExifData::const_iterator eend;
	Exiv2::ExifData::const_iterator e;
};

ref class Enumerator : IEnumerator<KeyValuePair<String^, Value^>>
{
public:
	NativeIterator* n;
	
	Enumerator(Image^ a_image)
	{
		image = a_image;
		n = new NativeIterator();
		Reset();
	}

	virtual ~Enumerator()
	{
		delete n;
	}

	virtual property Object ^ CurrentObject
	{
        Object ^ get () = System::Collections::IEnumerator::Current::get
        {
           return Current::get();
        }
    }

	virtual bool MoveNextObject(void)
		= System::Collections::IEnumerator::MoveNext
	{
		switch (n->phase)
		{
		case 0:
			if (n->i != n->iend)
			{
				n->phase = 1;
				return true;
			}
			else
			{
				n->phase = 2;
				return MoveNextObject();
			}
			return true;
		case 1:
			++n->i;
			if (n->i != n->iend)
			{
				return true;
			}
			else
			{
				n->phase = 2;
				return MoveNextObject();
			}
			break;
		case 2:
			if (n->e != n->eend)
			{
				n->phase = 3;
				return true;
			}
			else
			{
				n->phase = 4;
				return MoveNextObject();
			}
			break;
		case 3:
			++n->e;
			if (n->e != n->eend)
			{
				return true;
			}
			else
			{
				n->phase = 4;
				return MoveNextObject();
			}
			break;
		case 4:
			return false;
			break;
		}
		return false;
	}


	virtual void Reset(void)
	{
		ImageWrapper *imgWrap = (ImageWrapper*)image->hImage;
		Exiv2::IptcData &iptcData = imgWrap->image->iptcData();
		Exiv2::ExifData &exifData = imgWrap->image->exifData();

		n->phase = 0;
		n->i = iptcData.begin();
		n->iend = iptcData.end();
		n->e = exifData.begin();
		n->eend = exifData.end();
    }

	virtual property System::Collections::Generic::KeyValuePair<String^, Value^> Current
	{
		virtual System::Collections::Generic::KeyValuePair<String^, Value^> Current::get(void)
		{
			switch (n->phase)
			{
			case 1:
				return KeyValuePair<String^, Value^>(
					Marshal::PtrToStringAnsi((IntPtr)(char*) n->i->key().c_str()),
					ToValue(n->i->value())
					);
				break;
			case 3:
				return KeyValuePair<String^, Value^>(
					Marshal::PtrToStringAnsi((IntPtr)(char*) n->e->key().c_str()),
					ToValue(n->e->value())
					);
				break;
			}
			return KeyValuePair<String^, Value^>(String::Empty, nullptr);
		}
	}

private:
	Image^ image;
};

ref class Enumerable : Image::MetaEnumerable
{
public:
	Enumerable(Image^ a_image)
	{
		image = a_image;
	}

	virtual IEnumerator<KeyValuePair<String^,Value^>>^ GetEnumerator(void) 
		= IEnumerable<KeyValuePair<String ^,Value ^>>::GetEnumerator
	{
		return gcnew Enumerator(image);
	}

	virtual System::Collections::IEnumerator^ GetEnumerator2(void)
		= System::Collections::IEnumerable::GetEnumerator
	{
		return gcnew Enumerator(image);
	}

private:
	Image^ image;
};

Image::MetaEnumerable^ Image::EnumMeta()
{
	return gcnew Enumerable(this);
}

ImageWrapper* Image::GetImageWrapper()
{
	return (ImageWrapper*)hImage;
}

Value^ Image::ReadMeta(String^ clrKey)
{
	ImageWrapper* imgWrap = GetImageWrapper();
    Exiv2::IptcData &iptcData = imgWrap->image->iptcData();
    Exiv2::ExifData &exifData = imgWrap->image->exifData();

	std::string key = ToStdString(clrKey);

    // First try iptc
	try
	{
		Exiv2::IptcKey iptcKey(key);
		Exiv2::IptcData::const_iterator iter = iptcData.findKey(iptcKey);
		if (iter != iptcData.end())
		{
			return ToValue(iter->value());
		}
	}
	catch (...)
	{
	}

    // No iptc value, so try exif
	try
	{
		Exiv2::ExifKey exifKey(key);
		Exiv2::ExifData::const_iterator iter = exifData.findKey(exifKey);
		if (iter != exifData.end())
		{
			return ToValue(iter->value());
		}
	}
	catch (...)
	{
	}

	throw gcnew IndexOutOfRangeException(clrKey);
}

void Image::AddMeta(String^ key, Value^ value)
{
	// todo
	modified = true;
}

void Image::ModifyMeta(String^ clrKey, Value^ clrValue)
{
    ImageWrapper *imgWrap = GetImageWrapper();

    Exiv2::IptcData &iptcData = imgWrap->image->iptcData();
    Exiv2::ExifData &exifData = imgWrap->image->exifData();

	std::string key = ToStdString(clrKey);
	Exiv2::Value::AutoPtr value = ToExif2Value(clrValue);

	try {
        Exiv2::IptcKey iptcKey(key);

        Exiv2::IptcData::iterator iter = iptcData.findKey(iptcKey);
        if (iter != iptcData.end()) {
            iter->setValue(value.get());
        }
        else {
            iptcData.add(iptcKey, value.get());
        }
        modified = true;
		return;
    } 
    catch(const Exiv2::AnyError&) {
    }

    // Failed with iptc, so try exif
    try {
        Exiv2::ExifKey exifKey(key);

        Exiv2::ExifData::iterator iter = exifData.findKey(exifKey);
        if (iter != exifData.end()) {
            iter->setValue(value.get());
        }
        else {
            exifData.add(exifKey, value.get());
        }
		modified = true;
		return;
    }
    catch(const Exiv2::AnyError&) {
    }

    throw gcnew IndexOutOfRangeException(clrKey);
}

bool Image::IsModified::get()
{
	return modified;
}

void Image::RemoveMeta(String^ key)
{
	::RemoveMeta(
		hImage,
		(const char*) Marshal::StringToHGlobalAnsi(key).ToPointer());
}

bool Image::ContainsKey(String^ clrKey)
{
    ImageWrapper *imgWrap = GetImageWrapper();

    Exiv2::IptcData &iptcData = imgWrap->image->iptcData();
    Exiv2::ExifData &exifData = imgWrap->image->exifData();

	std::string key = ToStdString(clrKey);

	try {
        Exiv2::IptcKey iptcKey(key);

        Exiv2::IptcData::iterator iter = iptcData.findKey(iptcKey);
        if (iter != iptcData.end())
		{
            return true;
        }
    } 
    catch(const Exiv2::AnyError&) {
    }

    // Failed with iptc, so try exif
    try {
        Exiv2::ExifKey exifKey(key);

        Exiv2::ExifData::iterator iter = exifData.findKey(exifKey);
        if (iter != exifData.end()) {
            return true;
        }
    }
    catch(const Exiv2::AnyError&) {
    }

    return false;
}

/*
DateTimeOriginal
The date and time when the original image data was generated. For a DSC the date and time the picture was taken
are recorded. The format is "YYYY:MM:DD HH:MM:SS" with time shown in 24-hour format, and the date and time
separated by one blank character [20.H]. When the date and time are unknown, all the character spaces except
colons (":") may be filled with blank characters, or else the Interoperability field may be filled with blank characters.
The character string length is 20 bytes including NULL for termination. When the field is left blank, it is treated as
unknown.
Tag = 36867 (9003.H)
Type = ASCII
Count = 20
Default = none
*/

DateTime Image::DateTimeOriginal::get()
{
	String^ s = ((AsciiString^) ReadMeta(Tags::DateTimeOriginal))->Value;
	return ExifUtil::ParseDateTime(s);
}

void Image::DateTimeOriginal::set(DateTime value)
{
	ModifyMeta(Tags::DateTimeOriginal, gcnew AsciiString(ExifUtil::FormatDateTime(value)));
}

double Image::GPSLatitude::get()
{
	String^ r = ((AsciiString^)ReadMeta(Tags::GPSLatitudeRef))->Value;
	double sign = 0.0;
	if (r == "N")
	{
		sign = 1.0;
	}
	else if (r == "S")
	{
		sign = -1.0;
	}
	else
	{
		throw gcnew ArgumentOutOfRangeException(Tags::GPSLatitudeRef);
	}
	return sign * ExifUtil::ExifToGPSDegrees(ReadMeta(Tags::GPSLatitude));
}

void Image::GPSLatitude::set(double x)
{
	SetGPSVersion();

	String^ r = x >= 0 ? "N" : "S";
	ModifyMeta(Tags::GPSLatitudeRef, gcnew AsciiString(r));
	x = Math::Abs(x);
	ModifyMeta(Tags::GPSLatitude, ExifUtil::GPSDegreesToExif(x));
}

double Image::GPSLongitude::get()
{
	String^ r = ((AsciiString^)ReadMeta(Tags::GPSLongitudeRef))->Value;
	double sign = 0.0;
	if (r == "E")
	{
		sign = 1.0;
	}
	else if (r == "W")
	{
		sign = -1.0;
	}
	else
	{
		throw gcnew ArgumentOutOfRangeException(Tags::GPSLongitudeRef);
	}
	return sign * ExifUtil::ExifToGPSDegrees(ReadMeta(Tags::GPSLongitude));
}

void Image::GPSLongitude::set(double x)
{
	SetGPSVersion();

	String^ r = x >= 0 ? "E" : "W";
	ModifyMeta(Tags::GPSLongitudeRef, gcnew AsciiString(r));
	x = Math::Abs(x);
	ModifyMeta(Tags::GPSLongitude, ExifUtil::GPSDegreesToExif(x));
}

void Image::SetGPSVersion()
{
	if (!ContainsKey(Tags::GPSVersionID))
	{
		array<unsigned char>^ a = gcnew array<unsigned char>(4);
		a[0] = 2;
		a[1] = 2;
		a[2] = 0;
		a[3] = 0;
		ModifyMeta(Tags::GPSVersionID, gcnew UnsignedByte(a));
	}
}

DateTime Image::GPSDateTime::get()
{
	UnsignedRational^ t = ((UnsignedRational^) ReadMeta(Tags::GPSTimeStamp));
	DateTime d = ExifUtil::ParseDate(((AsciiString^)ReadMeta(Tags::GPSDateStamp))->Value);
	d = d.Add(TimeSpan(
		(int)Math::Floor(t->Value[0].ToDouble()),
		(int)Math::Floor(t->Value[1].ToDouble()),
		(int)Math::Floor(t->Value[2].ToDouble())));
	return d;
}

void Image::GPSDateTime::set(DateTime dateTime)
{
	SetGPSVersion();

	array<UnsignedRational::Element>^ d = gcnew array<UnsignedRational::Element>(3);
	d[0].Nominator = dateTime.Hour;
	d[0].Denominator = 1;
	d[1].Nominator = dateTime.Minute;
	d[1].Denominator = 1;
	d[2].Nominator = dateTime.Second;
	d[2].Denominator = 1;
	ModifyMeta(Tags::GPSTimeStamp, gcnew UnsignedRational(d));
	ModifyMeta(Tags::GPSDateStamp, gcnew AsciiString(ExifUtil::FormatDate(dateTime)));
}

bool Image::HasGPSInformation::get()
{
	return
		ContainsKey(Tags::GPSVersionID) &&
		ContainsKey(Tags::GPSLatitude) &&
		ContainsKey(Tags::GPSLatitudeRef) &&
		ContainsKey(Tags::GPSLongitude) &&
		ContainsKey(Tags::GPSLongitudeRef) &&
		ContainsKey(Tags::GPSTimeStamp) &&
		ContainsKey(Tags::GPSDateStamp);
}

String^ Image::Artist::get()
{
	return ((AsciiString^)ReadMeta(Tags::Artist))->Value;
}

void Image::Artist::set(String^ value)
{
	ModifyMeta(Tags::Artist, gcnew AsciiString(value));
}
