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
	public ref class Image : public System::Collections::Generic::IDictionary<System::String^, Value^>
	{
	public:
		Image();
		Image(String^ file);
		~Image();
		void OpenFile(String^ file);
		void OpenMem(array<Byte>^ data);
		void Save();
		int ImageSize();
		array<Byte^>^ ImageData();
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

		property array<unsigned char>^ GPSVersionID
		{
			array<unsigned char>^ get();
			void set(array<unsigned char>^);
		}

		void SetDefaultGPSVersion();

		property bool HasGPSInformation
		{
			bool get();
		}

		HIMAGE hImage;

		// IDictionary
		typedef System::Collections::Generic::KeyValuePair<System::String ^,Exiv2Net::Value ^> T;

		virtual System::Collections::IEnumerator^ GetEnumerator(void)
			= System::Collections::IEnumerable::GetEnumerator
		{
			return EnumMeta()->GetEnumerator();
		}

		virtual System::Collections::Generic::IEnumerator<T>^ GetEnumeratorGeneric(void)
			= System::Collections::Generic::IEnumerable<T>::GetEnumerator
		{
			return EnumMeta()->GetEnumerator();
		}

		virtual property int Count
		{
			virtual int get(void)
			{
				int count = 0;
				System::Collections::Generic::IEnumerator<T>^ i = EnumMeta()->GetEnumerator();
				while (i->MoveNext())
				{
					++count;
				}
				return count;
			}
		}
        
		virtual property bool IsReadOnly
		{
			virtual bool get(void)
			{
				return false;
			}
		}

		virtual void Add(T t)
		{
			ModifyMeta(t.Key, t.Value);
		}

		virtual void Clear(void)
		{
			// todo
		}

		virtual bool Contains(T t)
		{
			return ContainsKey(t.Key);
		}

		virtual void CopyTo(cli::array<T, 1> ^,int)
		{
			// todo
		}

		virtual bool Remove(T t)
		{
			if (ContainsKey(t.Key))
			{
				RemoveMeta(t.Key);
				return true;
			}
			else
			{
				return false;
			}
		}

		virtual property Exiv2Net::Value^ default[System::String ^]
		{
			Exiv2Net::Value^ get(System::String ^ key)
			{
				return ReadMeta(key);
			}

			void set(System::String ^ key,Exiv2Net::Value ^ value)
			{
				ModifyMeta(key, value);
			}
		}

		virtual property System::Collections::Generic::ICollection<String^>^ Keys
		{
			virtual System::Collections::Generic::ICollection<String^>^ get(void)
			{
				System::Collections::Generic::IList<String^>^ keys = gcnew System::Collections::Generic::List<String^>();
				// todo
				return keys;
			}
		}

		virtual property System::Collections::Generic::ICollection<Value^>^ Values
		{
			virtual System::Collections::Generic::ICollection<Value^>^ get(void)
			{
				System::Collections::Generic::IList<Value^>^ values = gcnew System::Collections::Generic::List<Value^>();
				// todo
				return values;
			}

		}

		virtual bool ContainsKey(String^ key);
		virtual void Add(System::String ^ key,Exiv2Net::Value ^ value)
		{
			ModifyMeta(key, value);
		}

		virtual bool Remove(System::String ^ key)
		{
			if (ContainsKey(key))
			{
				RemoveMeta(key);
				return true;
			}
			else
			{
				return false;
			}
		}

		virtual bool TryGetValue(System::String ^ key,Exiv2Net::Value ^% value)
		{
			if (ContainsKey(key))
			{
				value = ReadMeta(key);
				return true;
			}
			else
			{
				value = nullptr;
				return false;
			}
		}

		// IDictionary

		typedef System::Collections::Generic::IEnumerable
		<
			System::Collections::Generic::KeyValuePair
			<
				String^, Value^
			>
		> MetaEnumerable;
	private:
		void Free();
		Value^ ReadMeta(String^ key);
		MetaEnumerable^ EnumMeta();
		void AddMeta(String^ key, Value^ value);
		void ModifyMeta(String^ key, Value^ value);
		void RemoveMeta(String^ key);

		ImageWrapper* GetImageWrapper();
		bool modified;
		String^ m_fileName;
	};
}
