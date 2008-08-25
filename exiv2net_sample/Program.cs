using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.IO;

namespace exiv2net_sample
{
    class Program
    {
        static string TestDataFile(string name)
        {
            string p = Assembly.GetExecutingAssembly().Location;
            p = Directory.GetParent(p).FullName;
            p = Path.Combine(p, "test-data");
            return Path.Combine(p, name);
        }

        static void Main(string[] args)
        {
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
        }
    }
}
