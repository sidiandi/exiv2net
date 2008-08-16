#pragma once
#include "stdafx.h"
#include "Value.h"

using namespace System;

struct HIMAGE__;
typedef HIMAGE__* HIMAGE;

struct ImageWrapper;

namespace Exiv2Net
{
	public ref class Image
	{
	public:
		Image();
		Image(String^ file);
		~Image();
		void OpenFile(String^ file);
		void OpenMem(array<Byte>^ data);
		void Free();
		void Save();
		int ImageSize();
		array<Byte^>^ ImageData();
		Value^ ReadMeta(String^ key);
		typedef System::Collections::Generic::IEnumerable
		<
			System::Collections::Generic::KeyValuePair
			<
				String^, Value^
			>
		> MetaEnumerable;
		MetaEnumerable^ EnumMeta();
		void AddMeta(String^ key, Value^ value);
		void ModifyMeta(String^ key, Value^ value);
		void RemoveMeta(String^ key);
		bool ContainsKey(String^ key);
		property bool IsModified
		{
			bool get();
		}

		property String^ FileName
		{
			String^ get();
		}

		property DateTime DateTimeOriginal
		{
			DateTime get();
			void set(DateTime);
		}

		property String^ Artist
		{
			String^ get();
			void set(String^);
		}

		property double GPSLatitude
		{
			double get();
			void set(double);
		}

		property double GPSLongitude
		{
			double get();
			void set(double);
		}

		property DateTime GPSDateTime
		{
			DateTime get();
			void set(DateTime);
		}

		property bool HasGPSInformation
		{
			bool get();
		}

		HIMAGE hImage;
	private:
		void SetGPSVersion();
		ImageWrapper* GetImageWrapper();
		bool modified;
		String^ m_fileName;
	};
}
