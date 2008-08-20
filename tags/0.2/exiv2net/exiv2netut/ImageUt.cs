// ***************************************************************** -*- C# -*-
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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;
using Exiv2Net;
using System.Reflection;
using System.IO;

namespace exiv2netut
{
    [TestFixture]
    public class ImageUt
    {
        static string TestDataFile(string name)
        {
            string p = Assembly.GetExecutingAssembly().Location;
            p = Directory.GetParent(p).FullName;
            p = Path.Combine(p, "test-data");
            return Path.Combine(p, name);
        }
        
        Image image;
        string imageFileName;
        string DateTimeOriginal = "Exif.Photo.DateTimeOriginal";

        [SetUp]
        public void Setup()
        {
            imageFileName = TestDataFile("test1_copy.jpeg");
            File.Copy(TestDataFile("test1.jpeg"), imageFileName, true);
            image = new Image();
            image.OpenFile(imageFileName);
        }

        [TearDown]
        public void TearDown()
        {
            image.Dispose();
        }

        [Test]
        public void ImageEnumMeta()
        {
            bool foundDateTimeOriginal = false;
            
            foreach (KeyValuePair<String, Value> i in image)
            {
                Console.WriteLine(i);
                if (i.Key == DateTimeOriginal)
                {
                    foundDateTimeOriginal = true;
                }
            }

            Assert.IsTrue(foundDateTimeOriginal);
        }

        [Test]
        public void Modify()
        {
            string oldValue = (image[DateTimeOriginal] as AsciiString).Value;
            string newValue = oldValue + " added something!";
            image[DateTimeOriginal] = new AsciiString(newValue);

            image.Save();
            image.Dispose();
            Image imageRead = new Image(imageFileName);

            Assert.AreEqual(newValue, ((AsciiString)imageRead[DateTimeOriginal]).Value);
        }

        [Test]
        public void ReadWrite()
        {
            string oldValue = (image[DateTimeOriginal] as AsciiString).Value;
            string newValue = oldValue + " added something!";
            image[DateTimeOriginal] = new AsciiString(newValue);
            Assert.AreEqual(true, image.IsModified);

            image.Save();
            image.Dispose();
            Image imageRead = new Image(imageFileName);

            Assert.AreEqual(newValue, ((AsciiString)imageRead[DateTimeOriginal]).Value);
        }

        [Test]
        public void ModifyUnsignedByte()
        {
            string key = "Exif.Canon.0x0018";
            byte[] data = { 0, 1, 2, 3, 4 };
            image[key] = new UnsignedByte(data);

            image.Save();
            image.Dispose();
            Image imageRead = new Image(imageFileName);

            byte[] readData = (imageRead[key] as UnsignedByte).Value;
            Assert.AreEqual(data, readData);
        }

        [Test]
        public void ModifyUnsignedShort()
        {
            string key = "Exif.Canon.0x0018";
            UInt16[] data = { 0, 1, 2, 3, 4 };
            image[key] = new UnsignedShort(data);

            image.Save();
            image.Dispose();
            Image imageRead = new Image(imageFileName);
            
            ushort[] readData = (imageRead[key] as UnsignedShort).Value;
            Assert.AreEqual(data, readData);
        }

        [Test]
        public void ModifyUnsignedRational()
        {
            string key = "Exif.Canon.0x0018";
            UnsignedRational.Element[] data = new UnsignedRational.Element[2];
            data[0].Nominator = 1;
            data[0].Denominator = 2;
            data[1].Nominator = 3;
            data[1].Denominator = 4;
            image[key] = new UnsignedRational(data);

            image.Save();
            image.Dispose();
            Image imageRead = new Image(imageFileName);
            
            UnsignedRational.Element[] readData = (imageRead[key] as UnsignedRational).Value;
            Assert.AreEqual(data, readData);
        }

        [Test]
        public void Properties()
        {
            DateTime d = DateTime.Now;
            d = new DateTime(d.Year, d.Month, d.Day, d.Hour, d.Minute, d.Second);
            image.DateTimeOriginal = d;
            Assert.AreEqual(d, image.DateTimeOriginal);

            double delta = 0.0001;
            
            // GPS
            double lat = 49.47055;
            image.GPSLatitude = lat;
            Assert.AreEqual(lat, image.GPSLatitude, delta);
            
            double lon = 11.4080444444444;
            image.GPSLongitude = lon;
            Assert.AreEqual(lon, image.GPSLongitude, delta);

            // GPS (negative latitude and longitude values)
            lat = - 49.47055;
            image.GPSLatitude = lat;
            Assert.AreEqual(lat, image.GPSLatitude, delta);

            lon = - 11.4080444444444;
            image.GPSLongitude = lon;
            Assert.AreEqual(lon, image.GPSLongitude, delta);

            image.GPSDateTime = d;
            Assert.AreEqual(d, image.GPSDateTime);
        }

        // IDictionary
    }
}
