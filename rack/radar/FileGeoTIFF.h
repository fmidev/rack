/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
*/
#ifndef RACK_GEO_TIFF_H
#define RACK_GEO_TIFF_H

#include <drain/util/Log.h>
#include <drain/util/Dictionary.h>
#include <drain/image/Image.h>

namespace rack
{


/// For writing images in GeoTIFF format. Reading not supported currently.
/**
 */
class FileGeoTIFF
{
public:

	/// Writes image to a TIIF (GeoTIFF) file.
	/**
	 *  Writes drain::Image to a tif image.
	 *  Writes in 8 or 16 bits.
	 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
	*/
	//static void write(const Image &image,const std::string &path);
#ifdef GEOTIFF_NO // geotiff //RACKGEOTIFF

	static inline
	void write(const std::string & path, const drain::image::Image & src, int tileWidth, int tileHeight=0){
		drain::Logger mout("FileGeoTIFF", __FUNCTION__);
		mout.warn() << "binary compiled without TIFF/GeoTIFF support, skipping" << mout.endl;
	};

#else

	static
	void write(const std::string & path, const drain::image::Image & src, int tileWidth, int tileHeight=0);

	static
	const drain::Dictionary2<int, std::string> & getCompressionDict();

#endif

	static inline
	void write(const std::string & path, const drain::image::Image & src){
		write(path, src, tileWidth, tileHeight); // static defaults, see below
	};

	// Defaults
	static int tileWidth;
	static int tileHeight;
	// https://www.awaresystems.be/imaging/tiff/tifftags/compression.html
	static int compression; // COMPRESSION_NONE = 1; COMPRESSION_LZW = 5;

protected:

	static
	drain::Dictionary2<int, std::string> compressionDict;

private:

	//void SetUpTIFFDirectory

};


} // rack::

#endif
