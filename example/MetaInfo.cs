// copyright (c) 2009, andreas.grimme@gmx.net. All rights reserved.

using System;
using System.Collections.Generic;
using System.Text;
using System.Collections.Specialized;
using System.Windows.Media.Imaging;
using System.IO;
using System.Collections.ObjectModel;
using NUnit.Framework;
using Sidi.IO;
using System.Linq;
using System.Text.RegularExpressions;
using Sidi.Util;
using System.Globalization;

namespace Sidi.Tu
{
    public class PictureMetaInformation
    {
        public string Caption { get; set; }
        public string FileName { get { return fileName; } }

        public DateTime? FileDateTime;
        public string ImageDescription;
        public string Make;
        public string Model;
        public string Software;
        public string Artist;
        public string Copyright;

        public DateTime? DateTimeOriginal;
        public DateTime? DateTimeDigitized;

        public DateTime? GPSDateTime { get; set; }
        public double? GPSLatitude { get; set; }
        public double? GPSLongitude { get; set; }

        public string CameraID
        {
            get
            {
                return new string[] { Make, Model }.Where(x => x != null).Select(x => x.Trim()).Join(" ");
            }
        }

        string fileName;

        public PictureMetaInformation(string filename)
        {
            this.fileName = filename;
            Read();
        }

        public static bool TryGet(string filename, out PictureMetaInformation instance)
        {
            try
            {
                instance = new PictureMetaInformation(filename);
                return true;
            }
            catch (Exception)
            {
                instance = null;
                return false;
            }
        }

        BitmapMetadata bitmapMetadata;

        public BitmapMetadata BitmapMetadata
        {
            get
            {
                return bitmapMetadata;
            }
        }

        string GetString(string query)
        {
            return (string)bitmapMetadata.GetQuery(query);
        }

        /// <summary>
        /// "YYYY:MM:DD HH:MM:SS"
        /// </summary>
        /// <param name="query"></param>
        /// <returns></returns>
        DateTime? GetDate(string query)
        {
            string ds = GetString(query);
            if (ds == null)
            {
                return null;
            }
            else
            {
                return ParseDate(ds);
            }
        }

        DateTime? GetDate(string query, string subsecQuery)
        {
            DateTime? d = GetDate(query);
            if (d == null)
            {
                return d;
            }

            string sub = GetString(subsecQuery);
            if (sub != null)
            {
                d = d.Value + TimeSpan.FromSeconds(Double.Parse("0." + sub, CultureInfo.InvariantCulture));
            }
            return d;
        }

        DateTime ParseDate(string d)
        {
            string[] p = d.Split(new char[] { ':', ' ', '.' });
            return new DateTime(
                Int32.Parse(p[0]),
                Int32.Parse(p[1]),
                Int32.Parse(p[2]),
                Int32.Parse(p[3]),
                Int32.Parse(p[4]),
                Int32.Parse(p[5]));
        }

        public void Read()
        {
            using (Stream pictureFileStream = new FileStream(fileName, FileMode.Open, FileAccess.Read))
            {
                // Disable caching to prevent excessive memory usage.
                JpegBitmapDecoder decoder = new JpegBitmapDecoder(pictureFileStream, BitmapCreateOptions.None, BitmapCacheOption.None);
                bitmapMetadata = (BitmapMetadata)decoder.Frames[0].Metadata;

                if (bitmapMetadata != null)
                {
                    FileDateTime = GetDate(DateTimeQuery);
                    ImageDescription = GetString(ImageDescriptionQuery);
                    Make = GetString(MakeQuery);
                    Model = GetString(ModelQuery);
                    Software = GetString(SoftwareQuery);
                    Artist = GetString(ArtistQuery);
                    Copyright = GetString(CopyrightQuery);

                    DateTimeOriginal = GetDate(DateTimeOriginalQuery, SubSecTimeOriginalQuery);
                    DateTimeDigitized = GetDate(DateTimeDigitizedQuery, SubSecTimeDigitizedQuery);

                    //the rest is all gps stuff
                    byte[] gpsVersionNumbers = bitmapMetadata.GetQuery(GPSVersionIDQuery) as byte[];
                    bool strangeVersion = (gpsVersionNumbers != null && gpsVersionNumbers[0] == 2);

                    ulong[] latitudes = bitmapMetadata.GetQuery(GPSLatitudeQuery) as ulong[];
                    if (latitudes != null)
                    {
                        double latitude = ConvertCoordinate(latitudes, strangeVersion);

                        // N or S
                        string northOrSouth = (string)bitmapMetadata.GetQuery(GPSLatitudeRefQuery);
                        if (northOrSouth == "S")
                        {
                            // South means negative latitude.
                            latitude = -latitude;
                        }
                        this.GPSLatitude = latitude;
                    }

                    ulong[] longitudes = bitmapMetadata.GetQuery(GPSLongitudeQuery) as ulong[];
                    if (longitudes != null)
                    {
                        double longitude = ConvertCoordinate(longitudes, strangeVersion);

                        // E or W
                        string eastOrWest = (string)bitmapMetadata.GetQuery(GPSLongitudeRefQuery);
                        if (eastOrWest == "W")
                        {
                            // West means negative longitude.
                            longitude = -longitude;
                        }
                        this.GPSLongitude = longitude;
                    }

                    var gpsTimeStamp = ToRational(bitmapMetadata.GetQuery(GPSTimeStampQuery));
                    var gpsDate = (string)bitmapMetadata.GetQuery(GPSDateStampQuery);
                    if (gpsTimeStamp != null && gpsDate != null)
                    {
                        var dateParts = gpsDate.Split(new char[] { ':' });
                        GPSDateTime = new DateTime(
                            Int32.Parse(dateParts[0]),
                            Int32.Parse(dateParts[1]),
                            Int32.Parse(dateParts[2])) + TimeSpan.FromMilliseconds(
                                (((gpsTimeStamp[0] * 60) + gpsTimeStamp[1]) * 60 + gpsTimeStamp[2]) * 1000);
                    }
                }
            }
        }

        public void Write()
        {
            bool inPlaceSuccessful = false;

            using (Stream savedFile = File.Open(fileName, FileMode.Open, FileAccess.ReadWrite))
            {
                BitmapDecoder output = BitmapDecoder.Create(savedFile, BitmapCreateOptions.None, BitmapCacheOption.Default);
                InPlaceBitmapMetadataWriter bitmapMetadata = output.Frames[0].CreateInPlaceBitmapMetadataWriter();

                SetMetadata(bitmapMetadata);

                if (bitmapMetadata.TrySave())
                {
                    //if it was able to save it in place, then...
                    //Great! We're done...
                    inPlaceSuccessful = true;
                }
            }

            //if the in place save wasn't successful, try to save another way.
            if (!inPlaceSuccessful)
            {
                string tempFileName = fileName + Path.GetRandomFileName();
                string origFileName = fileName + ".orig";
                try
                {
                    WriteCopyOfPictureUsingWic(fileName, tempFileName);
                    File.Move(fileName, fileName + ".orig");
                    File.Move(tempFileName, fileName);
                    File.Delete(origFileName);
                }
                catch
                {
                    if (File.Exists(origFileName))
                    {
                        File.Delete(fileName);
                        File.Move(origFileName, fileName);
                    }
                    if (File.Exists(tempFileName))
                    {
                        File.Delete(tempFileName);
                    }
                }
            }
        }

        private void SetMetadata(BitmapMetadata bitmapMetadata)
        {
            if (Caption != null)
            {
                bitmapMetadata.Comment = Caption;
            }

            //the rest is gps stuff
            if (GPSLatitude.HasValue && GPSLongitude.HasValue)
            {
                bitmapMetadata.SetQuery(GPSLatitudeQuery, ConvertCoordinate(GPSLatitude.Value));
                bitmapMetadata.SetQuery(GPSLongitudeQuery, ConvertCoordinate(GPSLongitude.Value));

                byte[] gpsVersionNumbers = new byte[4];
                gpsVersionNumbers[0] = 0;
                gpsVersionNumbers[1] = 0;
                gpsVersionNumbers[2] = 2;
                gpsVersionNumbers[3] = 2;
                bitmapMetadata.SetQuery(GPSVersionIDQuery, gpsVersionNumbers);

                string northOrSouth = (GPSLatitude.Value >= 0) ? "N" : "S";
                bitmapMetadata.SetQuery(GPSLatitudeRefQuery, northOrSouth);

                string eastOrWest = (GPSLongitude.Value >= 0) ? "E" : "W";
                bitmapMetadata.SetQuery(GPSLongitudeRefQuery, eastOrWest);

                string gpsTime = GPSDateTime.Value.ToString("yyyy:MM:dd HH:mm:ss");
            }
            else
            {
                bitmapMetadata.RemoveQuery(GPSLatitudeQuery);
                bitmapMetadata.RemoveQuery(GPSLongitudeQuery);
                bitmapMetadata.RemoveQuery(GPSLatitudeRefQuery);
                bitmapMetadata.RemoveQuery(GPSLongitudeRefQuery);
            }

            if (GPSDateTime.HasValue)
            {
                bitmapMetadata.SetQuery(GPSTimeStampQuery, ConvertGpsTime(GPSDateTime.Value));
                bitmapMetadata.SetQuery(GPSDateStampQuery, ConvertGpsDate(GPSDateTime.Value));
            }
        }

        ulong[] ConvertGpsTime(DateTime t)
        {
            return new ulong[]
            {
                PackRational(t.Hour, 1),
                PackRational(t.Minute, 1),
                PackRational(t.Second * 1000 + t.Millisecond, 1000)
            };
        }

        string ConvertGpsDate(DateTime t)
        {
            return t.ToString("yyyy:MM:dd");
        }

        private static ulong[] ConvertCoordinate(double coordinate)
        {
            ulong[] coordinates = new ulong[3];

            // Make sure coordinate is positive.
            coordinate = Math.Abs(coordinate);

            double degrees = Math.Floor(coordinate);

            coordinate -= degrees;

            double minutes = Math.Floor(coordinate * 60.0);

            coordinate -= (minutes / 60.0);

            double seconds = Math.Floor(coordinate * 3600.0);

            coordinates[0] = Convert.ToUInt64(degrees + DEGREES_OFFSET);
            coordinates[1] = Convert.ToUInt64(minutes + MINUTES_OFFSET);
            coordinates[2] = Convert.ToUInt64((seconds * 100.0) + SECONDS_OFFSET);

            return coordinates;
        }

        private void WriteCopyOfPictureUsingWic(string originalFileName, string outputFileName)
        {
            bool tryOneLastMethod = false;

            using (Stream originalFile = new FileStream(originalFileName, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                BitmapCreateOptions createOptions = BitmapCreateOptions.PreservePixelFormat | BitmapCreateOptions.IgnoreColorProfile;
                BitmapDecoder original = BitmapDecoder.Create(originalFile, createOptions, BitmapCacheOption.None);

                JpegBitmapEncoder output = new JpegBitmapEncoder();

                if (original.Frames[0] != null && original.Frames[0].Metadata != null)
                {
                    BitmapMetadata bitmapMetadata = original.Frames[0].Metadata.Clone() as BitmapMetadata;
                    bitmapMetadata.SetQuery("/app1/ifd/PaddingSchema:Padding", METADATA_PADDING_IN_BYTES);
                    bitmapMetadata.SetQuery("/app1/ifd/exif/PaddingSchema:Padding", METADATA_PADDING_IN_BYTES);
                    bitmapMetadata.SetQuery("/xmp/PaddingSchema:Padding", METADATA_PADDING_IN_BYTES);

                    SetMetadata(bitmapMetadata);

                    output.Frames.Add(BitmapFrame.Create(original.Frames[0], original.Frames[0].Thumbnail, bitmapMetadata, original.Frames[0].ColorContexts));
                }

                try
                {
                    using (Stream outputFile = File.Open(outputFileName, FileMode.Create, FileAccess.ReadWrite))
                    {
                        output.Save(outputFile);
                    }
                }
                catch (NotSupportedException e)
                {
                    System.Diagnostics.Debug.Print(e.Message);

                    output = new JpegBitmapEncoder();

                    output.Frames.Add(BitmapFrame.Create(original.Frames[0], original.Frames[0].Thumbnail, original.Metadata, original.Frames[0].ColorContexts));

                    using (Stream outputFile = File.Open(outputFileName, FileMode.Create, FileAccess.ReadWrite))
                    {
                        output.Save(outputFile);
                    }

                    tryOneLastMethod = true;
                }
            }

            if (tryOneLastMethod)
            {
                File.Move(outputFileName, outputFileName + "tmp");

                using (Stream recentlyOutputFile = new FileStream(outputFileName + "tmp", FileMode.Open, FileAccess.Read, FileShare.Read))
                {
                    BitmapCreateOptions createOptions = BitmapCreateOptions.PreservePixelFormat | BitmapCreateOptions.IgnoreColorProfile;
                    BitmapDecoder original = BitmapDecoder.Create(recentlyOutputFile, createOptions, BitmapCacheOption.None);
                    JpegBitmapEncoder output = new JpegBitmapEncoder();
                    if (original.Frames[0] != null && original.Frames[0].Metadata != null)
                    {
                        BitmapMetadata bitmapMetadata = original.Frames[0].Metadata.Clone() as BitmapMetadata;
                        bitmapMetadata.SetQuery("/app1/ifd/PaddingSchema:Padding", METADATA_PADDING_IN_BYTES);
                        bitmapMetadata.SetQuery("/app1/ifd/exif/PaddingSchema:Padding", METADATA_PADDING_IN_BYTES);
                        bitmapMetadata.SetQuery("/xmp/PaddingSchema:Padding", METADATA_PADDING_IN_BYTES);

                        SetMetadata(bitmapMetadata);

                        output.Frames.Add(BitmapFrame.Create(original.Frames[0], original.Frames[0].Thumbnail, bitmapMetadata, original.Frames[0].ColorContexts));
                    }

                    using (Stream outputFile = File.Open(outputFileName, FileMode.Create, FileAccess.ReadWrite))
                    {
                        output.Save(outputFile);
                    }
                }
                File.Delete(outputFileName + "tmp");
            }
        }

        public static double[] ToRational(object obj)
        {
            ulong[] data = obj as ulong[];
            if (data == null)
            {
                return null;
            }
            else
            {
                return data.Select(x => splitLongAndDivide(x)).ToArray();
            }
        }

        private double ConvertCoordinate(ulong[] coordinates, bool strangeVersion)
        {
            int degrees;
            int minutes;
            double seconds;

            if (strangeVersion)
            {
                degrees = (int)splitLongAndDivide(coordinates[0]);
                minutes = (int)splitLongAndDivide(coordinates[1]);
                seconds = splitLongAndDivide(coordinates[2]);
            }
            else
            {
                degrees = (int)(coordinates[0] - DEGREES_OFFSET);
                minutes = (int)(coordinates[1] - MINUTES_OFFSET);
                seconds = (double)(coordinates[2] - SECONDS_OFFSET) / 100.0;
            }

            double coordinate = degrees + (minutes / 60.0) + (seconds / 3600);

            double roundedCoordinate = Math.Floor(coordinate * COORDINATE_ROUNDING_FACTOR) / COORDINATE_ROUNDING_FACTOR;

            return roundedCoordinate;
        }

        private static double splitLongAndDivide(ulong number)
        {
            byte[] bytes = BitConverter.GetBytes(number);
            int int1 = BitConverter.ToInt32(bytes, 0);
            int int2 = BitConverter.ToInt32(bytes, 4);
            return ((double)int1 / (double)int2);
        }

        ulong PackRational(int n, int d)
        {
            byte[] bytes = new byte[8];
            BitConverter.GetBytes(n).CopyTo(bytes, 0);
            BitConverter.GetBytes(d).CopyTo(bytes, 4);
            return BitConverter.ToUInt64(bytes, 0);
        }

        #region Private Constants
        private const uint METADATA_PADDING_IN_BYTES = 2048;
        private const string ORIGINAL_DATE_QUERY = "/app1/ifd/exif/{ushort=36867}";
        private const string DIGITIZED_DATE_QUERY = "/app1/ifd/exif/{ushort=36868}";
        private const string IPTC_KEYWORDS_QUERY = "/app13/irb/8bimiptc/iptc/keywords";
        private const long DEGREES_OFFSET = 0x100000000;
        private const long MINUTES_OFFSET = 0x100000000;
        private const long SECONDS_OFFSET = 0x6400000000;
        private const double COORDINATE_ROUNDING_FACTOR = 1000000.0;
        #endregion

        // A. Tags relating to image data structure

        // Image width 
        private const string ImageWidthQuery = "/app1/ifd/{ushort=256}"; // SHORT or LONG 1
        // Image height 
        private const string ImageLengthQuery = "/app1/ifd/{ushort=257}"; // SHORT or LONG 1
        // Number of bits per component 
        private const string BitsPerSampleQuery = "/app1/ifd/{ushort=258}"; // SHORT 3
        // Compression scheme 
        private const string CompressionQuery = "/app1/ifd/{ushort=259}"; // SHORT 1
        // Pixel composition 
        private const string PhotometricInterpretationQuery = "/app1/ifd/{ushort=262}"; // SHORT 1
        // Orientation of image 
        private const string OrientationQuery = "/app1/ifd/{ushort=274}"; // SHORT 1
        // Number of components 
        private const string SamplesPerPixelQuery = "/app1/ifd/{ushort=277}"; // SHORT 1
        // Image data arrangement 
        private const string PlanarConfigurationQuery = "/app1/ifd/{ushort=284}"; // SHORT 1
        // Subsampling ratio of Y to C 
        private const string YCbCrSubSamplingQuery = "/app1/ifd/{ushort=530}"; // SHORT 2
        // Y and C positioning 
        private const string YCbCrPositioningQuery = "/app1/ifd/{ushort=531}"; // SHORT 1
        // Image resolution in width direction 
        private const string XResolutionQuery = "/app1/ifd/{ushort=282}"; // RATIONAL 1
        // Image resolution in height direction 
        private const string YResolutionQuery = "/app1/ifd/{ushort=283}"; // RATIONAL 1
        // Unit of X and Y resolution 
        private const string ResolutionUnitQuery = "/app1/ifd/{ushort=296}"; // SHORT 1

        // B. Tags relating to recording offset
        // Image data location 
        private const string StripOffsetsQuery = "/app1/ifd/{ushort=273}"; // SHORT or LONG *S
        // Number of rows per strip 
        private const string RowsPerStripQuery = "/app1/ifd/{ushort=278}"; // SHORT or LONG 1
        // Bytes per compressed strip 
        private const string StripByteCountsQuery = "/app1/ifd/{ushort=279}"; // SHORT or LONG *S
        // Offset to JPEG SOI 
        private const string JPEGInterchangeFormatQuery = "/app1/ifd/{ushort=513}"; // LONG 1
        // Bytes of JPEG data 
        private const string JPEGInterchangeFormatLengthQuery = "/app1/ifd/{ushort=514}"; // LONG 1

        // C. Tags relating to image data characteristics

        // Transfer function 
        private const string TransferFunctionQuery = "/app1/ifd/{ushort=301}"; // SHORT 3 * 256
        // White point chromaticity 
        private const string WhitePointQuery = "/app1/ifd/{ushort=318}"; // RATIONAL 2
        // Chromaticities of primaries 
        private const string PrimaryChromaticitiesQuery = "/app1/ifd/{ushort=319}"; // RATIONAL 6
        // Color space transformation matrix coefficients 
        private const string YCbCrCoefficientsQuery = "/app1/ifd/{ushort=529}"; // RATIONAL 3
        // Pair of black and white reference values 
        private const string ReferenceBlackWhiteQuery = "/app1/ifd/{ushort=532}"; // RATIONAL 6

        // D. Other tags
        // File change date and time 
        private const string DateTimeQuery = "/app1/ifd/{ushort=306}"; // ASCII 20
        // Image title 
        private const string ImageDescriptionQuery = "/app1/ifd/{ushort=270}"; // ASCII Any
        // Image input equipment manufacturer 
        private const string MakeQuery = "/app1/ifd/{ushort=271}"; // ASCII Any
        // Image input equipment model 
        private const string ModelQuery = "/app1/ifd/{ushort=272}"; // ASCII Any
        // Software used 
        private const string SoftwareQuery = "/app1/ifd/{ushort=305}"; // ASCII Any
        // Person who created the image 
        private const string ArtistQuery = "/app1/ifd/{ushort=315}"; // ASCII Any
        // Copyright holder 
        private const string CopyrightQuery = "/app1/ifd/{ushort=33432}"; // ASCII Any

        // F. Tags Relating to Date and Time
        // Date and time of original data generation 
        private const string DateTimeOriginalQuery = "/app1/ifd/exif/{ushort=36867}"; // ASCII 20
        // Date and time of digital data generation 
        private const string DateTimeDigitizedQuery = "/app1/ifd/exif/{ushort=36868}"; // ASCII 20
        // DateTime subseconds 
        private const string SubSecTimeQuery = "/app1/ifd/exif/{ushort=37520}"; // ASCII Any
        // DateTimeOriginal subseconds 
        private const string SubSecTimeOriginalQuery = "/app1/ifd/exif/{ushort=37521}"; // ASCII Any
        // DateTimeDigitized subseconds 
        private const string SubSecTimeDigitizedQuery = "/app1/ifd/exif/{ushort=37522}"; // ASCII Any

        // GPS tag version 
        private const string GPSVersionIDQuery = "/app1/ifd/gps/subifd:{ulong=0}"; // BYTE 4
        // North or South Latitude 
        private const string GPSLatitudeRefQuery = "/app1/ifd/gps/subifd:{ulong=1}"; // ASCII 2
        // Latitude        
        private const string GPSLatitudeQuery = "/app1/ifd/gps/subifd:{ulong=2}"; // RATIONAL 3
        // East or West Longitude 
        private const string GPSLongitudeRefQuery = "/app1/ifd/gps/subifd:{ulong=3}"; // ASCII 2
        // Longitude 
        private const string GPSLongitudeQuery = "/app1/ifd/gps/subifd:{ulong=4}"; // RATIONAL 3
        // Altitude reference 
        private const string GPSAltitudeRefQuery = "/app1/ifd/gps/subifd:{ulong=5}"; // BYTE 1
        // Altitude 
        private const string GPSAltitudeQuery = "/app1/ifd/gps/subifd:{ulong=6}"; // RATIONAL 1
        // GPS time (atomic clock) 
        private const string GPSTimeStampQuery = "/app1/ifd/gps/subifd:{ulong=7}"; // RATIONAL 3
        // GPS satellites used for measurement 
        private const string GPSSatellitesQuery = "/app1/ifd/gps/subifd:{ulong=8}"; // ASCII Any
        // GPS receiver status 
        private const string GPSStatusQuery = "/app1/ifd/gps/subifd:{ulong=9}"; // ASCII 2
        // GPS measurement mode 
        private const string GPSMeasureModeQuery = "/app1/ifd/gps/subifd:{ulong=10}"; // ASCII 2
        // Measurement precision 
        private const string GPSDOPQuery = "/app1/ifd/gps/subifd:{ulong=11}"; // RATIONAL 1
        // Speed unit 
        private const string GPSSpeedRefQuery = "/app1/ifd/gps/subifd:{ulong=12}"; // ASCII 2
        // Speed of GPS receiver 
        private const string GPSSpeedQuery = "/app1/ifd/gps/subifd:{ulong=13}"; // RATIONAL 1
        // Reference for direction of movement 
        private const string GPSTrackRefQuery = "/app1/ifd/gps/subifd:{ulong=14}"; // ASCII 2
        // Direction of movement 
        private const string GPSTrackQuery = "/app1/ifd/gps/subifd:{ulong=15}"; // RATIONAL 1
        // Reference for direction of image 
        private const string GPSImgDirectionRefQuery = "/app1/ifd/gps/subifd:{ulong=16}"; // ASCII 2
        // Direction of image 
        private const string GPSImgDirectionQuery = "/app1/ifd/gps/subifd:{ulong=17}"; // RATIONAL 1
        // Geodetic survey data used 
        private const string GPSMapDatumQuery = "/app1/ifd/gps/subifd:{ulong=18}"; // ASCII Any
        // Reference for latitude of destination 
        private const string GPSDestLatitudeRefQuery = "/app1/ifd/gps/subifd:{ulong=19}"; // ASCII 2
        // Latitude of destination 
        private const string GPSDestLatitudeQuery = "/app1/ifd/gps/subifd:{ulong=20}"; // RATIONAL 3
        // Reference for longitude of destination 
        private const string GPSDestLongitudeRefQuery = "/app1/ifd/gps/subifd:{ulong=21}"; // ASCII 2
        // Longitude of destination 
        private const string GPSDestLongitudeQuery = "/app1/ifd/gps/subifd:{ulong=22}"; // RATIONAL 3
        // Reference for bearing of destination 
        private const string GPSDestBearingRefQuery = "/app1/ifd/gps/subifd:{ulong=23}"; // ASCII 2
        // Bearing of destination 
        private const string GPSDestBearingQuery = "/app1/ifd/gps/subifd:{ulong=24}"; // RATIONAL 1
        // Reference for distance to destination 
        private const string GPSDestDistanceRefQuery = "/app1/ifd/gps/subifd:{ulong=25}"; // ASCII 2
        // Distance to destination 
        private const string GPSDestDistanceQuery = "/app1/ifd/gps/subifd:{ulong=26}"; // RATIONAL 1
        // Name of GPS processing method 
        private const string GPSProcessingMethodQuery = "/app1/ifd/gps/subifd:{ulong=27}"; // UNDEFINED Any
        // Name of GPS area 
        private const string GPSAreaInformationQuery = "/app1/ifd/gps/subifd:{ulong=28}"; // UNDEFINED Any
        // GPS date 
        private const string GPSDateStampQuery = "/app1/ifd/gps/subifd:{ulong=29}"; // ASCII 11
        // GPS differential correction 
        private const string GPSDifferentialQuery = "/app1/ifd/gps/subifd:{ulong=30}"; // SHORT 1

        [TestFixture]
        public class Test
        {
            private static readonly log4net.ILog log = log4net.LogManager.GetLogger(System.Reflection.MethodBase.GetCurrentMethod().DeclaringType);

            string origFile;
            string copyFile;

            public Test()
            {
                log4net.Config.BasicConfigurator.Configure();
            }

            [SetUp]
            public void Setup()
            {
                origFile = FileUtil.BinFile(@"Test\test1.jpeg");
                copyFile = origFile + ".copy.jpeg";
                File.Copy(origFile, copyFile, true);
            }

            [Test]
            public void Read()
            {
                PictureMetaInformation mi = new PictureMetaInformation(copyFile);
                Assert.IsTrue(mi.GPSLatitude.HasValue);
                Assert.IsTrue(mi.GPSLongitude.HasValue);
                log.Info(mi.GPSDateTime);
            }

            byte[] GetPixelData(BitmapSource src)
            {
                int octetsPerPixel = src.Format.BitsPerPixel / 8;
                byte[] d = new byte[src.PixelWidth * src.PixelHeight * octetsPerPixel];
                src.CopyPixels(d, src.PixelWidth * octetsPerPixel, 0);
                return d;
            }

            void AssertPixelDataEqual()
            {
                AssertPixelDataEqual(origFile, copyFile);
            }

            void AssertPixelDataEqual(string i1, string i2)
            {
                JpegBitmapDecoder d1 = new JpegBitmapDecoder(File.OpenRead(i1), BitmapCreateOptions.None, BitmapCacheOption.None);
                JpegBitmapDecoder d2 = new JpegBitmapDecoder(File.OpenRead(i2), BitmapCreateOptions.None, BitmapCacheOption.None);

                var pd1 = GetPixelData(d1.Frames[0]);
                var pd2 = GetPixelData(d2.Frames[0]);

                Assert.IsTrue(pd1.SequenceEqual(pd2));
            }

            [Test]
            public void ComparePixelData()
            {
                AssertPixelDataEqual(origFile, copyFile);
            }

            [Test, ExpectedException(ExceptionType = typeof(AssertionException))]
            public void ComparePixelData2()
            {
                AssertPixelDataEqual(origFile, FileUtil.BinFile(@"test\CIMG8451.JPG"));
            }

            [Test]
            public void TestRead()
            {
                PictureMetaInformation mi = new PictureMetaInformation(origFile);
                Assert.IsTrue(mi.GPSLatitude.HasValue);
                Assert.IsTrue(mi.GPSLongitude.HasValue);
                log.Info(mi.GPSDateTime);
            }

            [Test]
            public void TestWrite()
            {
                PictureMetaInformation mi = new PictureMetaInformation(copyFile);

                mi.GPSLongitude = 123.456;
                mi.GPSLatitude = -89.01;
                mi.GPSDateTime = DateTime.Now;

                mi.Write();

                AssertPixelDataEqual();

                var mi2 = new PictureMetaInformation(copyFile);
                Assert.AreEqual(mi.GPSLongitude.Value, mi2.GPSLongitude.Value, 0.01);
                Assert.AreEqual(mi.GPSLatitude.Value, mi2.GPSLatitude.Value, 0.01);
                Assert.AreEqual((double)mi.GPSDateTime.Value.Ticks, (double)mi2.GPSDateTime.Value.Ticks, TimeSpan.FromMilliseconds(1).Ticks);
            }

            [Test]
            public void ReadAttributes()
            {
                PictureMetaInformation mi = new PictureMetaInformation(copyFile);
                mi.DumpProperties(Console.Out);
            }

            [Test]
            public void ReadAttributes2()
            {
                PictureMetaInformation mi = new PictureMetaInformation(FileUtil.BinFile(@"test\MEMO0016.JPG"));
                mi.DumpProperties(Console.Out);
            }

            [Test]
            public void ReadAttributes3()
            {
                PictureMetaInformation mi = new PictureMetaInformation(FileUtil.BinFile(@"C:\Dokumente und Einstellungen\test\Eigene Dateien\Eigene Bilder\ref\IMG_0182.JPG"));
                mi.DumpProperties(Console.Out);
            }
        }
    }
}
