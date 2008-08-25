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

public ref class Tags
{
public:
	static String^ Artist = "Exif.Image.Artist";
	static String^ DateTimeOriginal = "Exif.Photo.DateTimeOriginal";
	static String^ SubSecTimeOriginal = "Exif.Photo.SubSecTimeOriginal";

	static String^ GPSVersionID = "Exif.GPSInfo.GPSVersionID";
	static String^ GPSLatitudeRef = "Exif.GPSInfo.GPSLatitudeRef";
	static String^ GPSLatitude = "Exif.GPSInfo.GPSLatitude";
	static String^ GPSLongitudeRef = "Exif.GPSInfo.GPSLongitudeRef";
	static String^ GPSLongitude = "Exif.GPSInfo.GPSLongitude";

	static String^ GPSTimeStamp = "Exif.GPSInfo.GPSTimeStamp";
	static String^ GPSDateStamp = "Exif.GPSInfo.GPSDateStamp";
};

