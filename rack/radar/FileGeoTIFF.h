/*

    Copyright 2016  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef RACK_GEO_TIFF_H
#define RACK_GEO_TIFF_H

#include <drain/image/AccumulationConverter.h>

#include <drain/image/AccumulatorGeo.h>
#include <drain/util/Log.h>
#include <drain/util/Rectangle.h>
#include <drain/util/Type.h>

#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"  // debugging
#include "data/ODIM.h"
#include "data/Data.h"
#include "data/DataSelector.h"
//#include "Coordinates.h"


namespace rack
{

using namespace drain::image;


// // using namespace std;

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
	static
	inline
	void write(const std::string & path, const drain::image::Image & src, int tileWidth, int tileHeight=0){
		drain::Logger mout("FileGeoTIFF", __FUNCTION__);
		mout.warn() << "binary compiled without TIFF/GeoTIFF support, skipping" << mout.endl;
	};
#else
	static
	void write(const std::string & path, const drain::image::Image & src, int tileWidth, int tileHeight=0);
#endif
	//void write(const std::string &filePath,  const HI5TREE & src, const std::list<std::string> & paths);

	inline
	static
	void write(const std::string & path, const drain::image::Image & src){
		write(path, src, tileWidth, tileHeight); // static defaults, see below
	};

	static int tileWidth;
	static int tileHeight;


private:

	//void SetUpTIFFDirectory

};


} // rack::

#endif //
