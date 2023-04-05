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
#ifndef DRAIN_GEO_TIFF
#define DRAIN_GEO_TIFF

#include "FileTIFF.h"

#ifndef USE_GEOTIFF_NO


#include <geotiff.h>
#include <geotiffio.h>
#include <geo_normalize.h>


namespace drain
{

namespace image
{

/** Extends TIFF by adding geo information in the metadata
 *
 *  https://gdal.org/drivers/raster/gtiff.html
 *
 */
class FileGeoTIFF : public FileTIFF {
public:

	//typedef drain::Dictionary<int, std::string> gtiff_epsg_dict_t;
	//typedef std::map<short, std::map<short, short> > epsg_map_t;
	typedef std::map<short, std::list<std::pair<geokey_t, short> > > epsg_map_t;

	static epsg_map_t epsgConf;

	FileGeoTIFF() : FileTIFF(), gtif(nullptr){
	}

	FileGeoTIFF(const std::string & path, const std::string & mode = "w") : FileTIFF(), gtif(nullptr){
		//open();
		open(path, mode);
	}

	virtual inline
	~FileGeoTIFF(){
		//gt_close();
		close();
	}

	/// Opens a GeoTIFF file.
	virtual
	void open(const std::string & path, const std::string & mode = "w");

	/// If EPSG is detected (currently by +init=epsg:EPSG) and support configured for EPSG code, set it directly.
	// Will replace latLon (EPSG=4326) as well?
	bool setProjectionEPSG(short epsg);

	/// "Opens" a GeoTIFF structure inside an opened TIFF file.
	//virtual void open();
	inline
	void writeMetadata(){
		if (isOpen()){
			GTIFWriteKeys(gtif);
		}
		else {
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.error("File not open");
		}
	}

	virtual
	void close();

	/// Todo: subclass
	inline virtual
	bool isOpen() const {
		return FileTIFF::isOpen() && (gtif != nullptr);
	}

	/// Require strict GeoTIFF compliance
	static
	bool strictCompliance;

	/// Use EPSG specific support only, if found. Else use also fromProj4Str().
	static
	bool plainEPSG;

	/// This is between Tiff and GeoTiff?
	//void setGdalMetaDataOLD(const std::string & nodata, double scale=1.0, double offset=0.0);
	//void setGdalMetaDataOLD(double nodata, double scale=1.0, double offset=0.0);

	/**
	 *  \param nodata - yes, string...
	 */
	void setGdalScale(double scale=1.0, double offset=0.0);

	//void setGdalMetaData(double scale=1.0, double offset=0.0);

	/// This is between Tiff and GeoTiff?
	/**
	 *  \param nodata - yes, string...
	 */
	void setGdalNoData(const std::string & nodata);

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

	//static
	//void write(const std::string & path, const drain::image::Image & src, int tileWidth, int tileHeight=0);

	/*
	static inline
	void write(const std::string & path, const drain::image::Image & src, int defaultTileWidth, int defaultTileHeight=0){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.warn("binary compiled without TIFF/GeoTIFF support, skipping");
	};
	*/




	//void setUpTIFFDirectory_rack(const drain::image::Image & src); //, int tileWidth=0, int tileHeight = 0);

	//void adjustGeoFrame_rack(const drain::image::Image & src, drain::image::GeoFrame & frame);


protected:



	GTIF *gtif;
	// TIFF *tif = XTIFFOpen(path.c_str(), "w");
	// GTIF *gtif = GTIFNew(tif);

};

} // image::

} // drain::
#endif

#endif
