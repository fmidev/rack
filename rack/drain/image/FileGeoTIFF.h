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
#ifndef DRAIN_GEO_TIFF_H
#define DRAIN_GEO_TIFF_H

#include "FileTIFF.h"

#ifdef USE_GEOTIFF_YES

#include <geotiff.h>
#include <geotiffio.h>
#include <geo_normalize.h>

#include "drain/util/Dictionary.h"
#include "drain/util/FileInfo.h"
#include "drain/util/Log.h"
#include "drain/image/Image.h"



namespace drain
{

	namespace image
	{


class FileGeoTIFF : public FileTIFF{
public:


	FileGeoTIFF(const std::string & path = "", const char *mode = "w") : FileTIFF(path, mode), gtif(nullptr){
		open();
	}

	inline
	~FileGeoTIFF(){
		//gt_close();
		close();
	}

	inline
	virtual
	void open(const std::string & path, const char *mode = "w"){
		if (isOpen()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.warn("GeoTIFF already open?");
		}
		else {
			FileTIFF::open(path, mode);
			open();
		}
	}


	/// "Opens" a GeoTIFF structure inside an opened TIFF file.
	virtual inline
	void open(){
		gtif = GTIFNew(tif);
		if (!isOpen()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.error("failed creating GeoTIFF file from TIFF object");
			//mout.error() << "failed creating GeoTIFF file from TIFF object, path=" << path << mout.endl;
		}
	}

	inline virtual
	void close(){
		if (isOpen()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.experimental("Closing GeoTIFF...");
			GTIFWriteKeys(gtif);
			GTIFFree(gtif);
			gtif = nullptr;
		}
		FileTIFF::close(); // ?
	}

	/// Todo: subclass
	inline virtual
	bool isOpen() const {
		return FileTIFF::isOpen() && (gtif != nullptr);
	}

	/// This is between Tiff and GeoTiff?
	/**
	 *  \param nodata - yes, string...
	 */
	void setGdalMetaData(const std::string & nodata, double scale=1.0, double offset=0.0);

	/// Sets projection and bounding box. Adjusts spatial resolution accordingly.
	/**
	 *
	 */
	void setGeoMetaData(const drain::image::GeoFrame & frame);

	void setProjection(const std::string & proj);

	void setProjection(const drain::Proj4 & proj);

	void setProjectionLongLat();


	/// Writes image to a TIIF (GeoTIFF) file.
	/**
	 *  Writes drain::Image to a tif image.
	 *  Writes in 8 or 16 bits.
	 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
	*/
	//static void write(const Image &image,const std::string &path);

	static
	void write(const std::string & path, const drain::image::Image & src, int tileWidth, int tileHeight=0);

	/*
	static inline
	void write(const std::string & path, const drain::image::Image & src, int defaultTileWidth, int defaultTileHeight=0){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.warn("binary compiled without TIFF/GeoTIFF support, skipping");
	};
	*/


	static inline
	void write(const std::string & path, const drain::image::Image & src){
		write(path, src, FileTIFF::defaultTile.width, FileTIFF::defaultTile.getHeight()); // static defaults, see below
	};


	void setUpTIFFDirectory_rack(const drain::image::Image & src); //, int tileWidth=0, int tileHeight = 0);

	void adjustGeoFrame_rack(const drain::image::Image & src, drain::image::GeoFrame & frame);


protected:

	GTIF *gtif;
	// TIFF *tif = XTIFFOpen(path.c_str(), "w");
	// GTIF *gtif = GTIFNew(tif);

};

} // image::

} // drain::
#endif

#endif
