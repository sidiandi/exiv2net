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
