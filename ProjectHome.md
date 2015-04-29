This is a .NET wrapper for Exiv2 (http://www.exiv2.org/). It compiles with Visual Studio 2008.

Since Microsoft itself now offers decent support for EXIF tags, **I have stopped maintaining exiv2net**. I would recommend to use the Microsoft API. I have added a piece of [source code](http://code.google.com/p/exiv2net/source/browse/trunk/example/MetaInfo.cs) of [TrackUtil](http://andreas-grimme.gmxhome.de/trackutil/), so that you get the idea of how to use the API (as usual with Microsoft, it's not straight forward).

Download the binaries for the latest version: http://exiv2net.googlecode.com/files/exiv2net-trunk.zip

[Contact the author](http://andreas-grimme.gmxhome.de/)

Synopsis:
```
// Construct an Exiv2Net.Image instance to access EXIF information of an image.
Exiv2Net.Image image = new Exiv2Net.Image(TestDataFile("test1.jpeg"));

// Display all properties
foreach (KeyValuePair<string, Exiv2Net.Value> i in image)
{
    Console.WriteLine(i);
}

// read a property
string dateTimeOriginalString = (image["Exif.Photo.DateTimeOriginal"] as Exiv2Net.AsciiString).Value;
Console.WriteLine(dateTimeOriginalString);

// write a property
image["Exif.Photo.DateTimeOriginal"] = new Exiv2Net.AsciiString(dateTimeOriginalString);

// Use the hard-coded properties to set and get values the easy way
Console.WriteLine(image.DateTimeOriginal);
Console.WriteLine(image.FileName);

// The Artist property is not set on the example image.
// This is how we can catch this exception:
try
{
    Console.WriteLine(image.Artist);
}
catch (IndexOutOfRangeException e)
{
    Console.WriteLine(String.Format("The EXIF tag {0} is not available in the image.", e.Message));
}

// write the Artist property
image.Artist = "cool@photographer.com";

// Read geo-tag information
if (image.HasGPSInformation)
{
    Console.WriteLine(image.GPSDateTime);
    Console.WriteLine(image.GPSLatitude);
    Console.WriteLine(image.GPSLongitude);
}

// save the modification, if any
if (image.IsModified)
{
    image.Save();
}
```