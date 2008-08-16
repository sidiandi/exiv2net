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
#include "Value.h"

using namespace System;
using namespace System::Text::RegularExpressions;

namespace Exiv2Net
{

ref class ExifUtil
{
public:
	static DateTime ParseDateTime(String^ s)
    {
        Match^ m = dateTimeParse->Match(s);
        return DateTime(
			Int32::Parse(m->Groups["year"]->Value),
			Int32::Parse(m->Groups["month"]->Value),
			Int32::Parse(m->Groups["day"]->Value),
			Int32::Parse(m->Groups["hour"]->Value),
			Int32::Parse(m->Groups["minute"]->Value),
			Int32::Parse(m->Groups["second"]->Value)
            );
    }

    static String^ FormatDateTime(DateTime time)
    {
        return time.ToString("yyyy:MM:dd HH:mm:ss");
    }

	static DateTime ParseDate(String^ s)
    {
        Match^ m = dateParse->Match(s);
        return DateTime(
			Int32::Parse(m->Groups["year"]->Value),
			Int32::Parse(m->Groups["month"]->Value),
			Int32::Parse(m->Groups["day"]->Value),
			0,0,0
            );
    }

    static String^ FormatDate(DateTime time)
    {
        return time.ToString("yyyy:MM:dd");
    }

	static UnsignedRational^ GPSDegreesToExif(double x)
	{
		array<UnsignedRational::Element>^ d = gcnew array<UnsignedRational::Element>(3);
		d[0].Nominator = (int)Math::Floor(x);
		d[0].Denominator = 1;
		x = (x - d[0].Nominator) * 60.0;
		d[1].Nominator = (int)Math::Floor(x);
		d[1].Denominator = 1;
		x = (x - d[1].Nominator) * 60.0;
		d[2].Nominator = (int)Math::Floor(x * 100.0);
		d[2].Denominator = 100;
		return gcnew UnsignedRational(d);
	}

	static double ExifToGPSDegrees(Value^ x)
	{
		array<UnsignedRational::Element>^ d = ((UnsignedRational^) x)->Value;
		return d[0].ToDouble() + d[1].ToDouble() / 60.0 + d[2].ToDouble() / 3600.0;
	}

private:
    static Regex^ dateTimeParse = gcnew Regex(
        "(?<year>\\d+)\\:(?<month>\\d+):(?<day>\\d+)\\ (?<hour>\\d+):(?<minute>\\d+):(?<second>\\d+)"
        );
    static Regex^ dateParse = gcnew Regex(
        "(?<year>\\d+)\\:(?<month>\\d+):(?<day>\\d+)"
        );
};

}
