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

using namespace System;

namespace Exiv2Net
{
public enum class TypeId { invalidTypeId, unsignedByte, asciiString, unsignedShort, 
            unsignedLong, unsignedRational, invalid6, undefined, 
            signedShort, signedLong, signedRational, 
            string, isoDate, isoTime,
            lastTypeId };

	public ref class Value
	{
	protected:
		generic<class T>
		String^ ArrayToString(array<T>^ a)
		{
			if (a == nullptr)
			{
				return String::Empty;
			}
			System::Text::StringBuilder^ s = gcnew System::Text::StringBuilder();
			const int iend = a->Length;
			for (int i=0; i<iend;++i)
			{
				s->Append(a[i]);
				s->Append(" ");
			}
			return s->ToString();
		}
	};

	public ref class UnsignedByte : public Value
	{
	public:
		array<unsigned char>^ Value;
		UnsignedByte(array<unsigned char>^ val)
		{
			Value = val;
		}
		
		virtual String^ ToString() override
		{
			return ArrayToString(Value);
		}
	};

	public ref class AsciiString : public Value
	{
	public:
		String^ Value;
		AsciiString(String^ val)
		{
			Value = val;
		}
		virtual String^ ToString() override
		{
			return Value;
		}
	};

	public ref class UnsignedShort : public Value
	{
	public:
		array<unsigned short>^ Value;
		UnsignedShort(array<unsigned short>^ val)
		{
			Value = val;
		}
		virtual String^ ToString() override
		{
			return ArrayToString(Value);
		}
	};

	public ref class UnsignedLong : public Value
	{
	public:
		array<unsigned int>^ Value;
		UnsignedLong(array<unsigned int>^ val)
		{
			Value = val;
		}
		virtual String^ ToString() override
		{
			return ArrayToString(Value);
		}
	};

	public ref class UnsignedRational : public Value
	{
	public:
		value struct Element
		{
		public:
			unsigned int Nominator;
			unsigned int Denominator;
			virtual String^ ToString() override
			{
				return String::Format("{0}/{1}", Nominator, Denominator);
			}
			double ToDouble()
			{
				return (double)Nominator/(double)Denominator;
			}
		};

		array<Element>^ Value;
		UnsignedRational(array<Element>^ val)
		{
			Value = val;
		}
		virtual String^ ToString() override
		{
			return ArrayToString(Value);
		}
	};

	public ref class SignedByte : public Value
	{
	public:
		array<signed char>^ Value;
		SignedByte(array<signed char>^ val)
		{
			Value = val;
		}
		virtual String^ ToString() override
		{
			return ArrayToString(Value);
		}
	};

	public ref class Undefined : public Value
	{
	public:
		array<unsigned char>^ Value;
		Undefined(array<unsigned char>^ val)
		{
			Value = val;
		}
		virtual String^ ToString() override
		{
			return ArrayToString(Value);
		}
	};

	public ref class SignedShort : public Value
	{
	public:
		array<signed short>^ Value;
		SignedShort (array<signed short>^ val)
		{
			Value = val;
		}
		virtual String^ ToString() override
		{
			return ArrayToString(Value);
		}
	};

	public ref class SignedLong : public Value
	{
	public:
		array<signed int>^ Value;
		SignedLong(array<signed int>^ val)
		{
			Value = val;
		}
		virtual String^ ToString() override
		{
			return ArrayToString(Value);
		}
	};

	public ref class SignedRational : public Value
	{
	public:
		value struct Element
		{
		public:
			signed int Nominator;
			signed int Denominator;
			virtual String^ ToString() override
			{
				return String::Format("{0}/{1}", Nominator, Denominator);
			}
			double ToDouble()
			{
				return (double)Nominator/(double)Denominator;
			}
		};

		array<Element>^ Value;
		SignedRational(array<Element>^ val)
		{
			Value = val;
		}
		virtual String^ ToString() override
		{
			return ArrayToString(Value);
		}
	};

}