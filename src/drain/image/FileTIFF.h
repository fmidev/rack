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
#ifndef DRAIN_TIFF_H_EXPERIMENTAL
#define DRAIN_TIFF_H_EXPERIMENTAL

#include <drain/Log.h>
#include "drain/util/FileInfo.h"
#include "drain/Dictionary.h"
#include "drain/util/Time.h"
#include "GeoFrame.h"
#include "Image.h"

#ifndef USE_GEOTIFF_NO

#include <xtiffio.h>
#endif

namespace drain
{

namespace image
{



/// For writing images in basic TIFF format. Reading not supported currently.
/**
 */
class FileTIFF { //: public FileHandler{
public:


	static
	const drain::FileInfo fileInfo;

	// OLD (Weak?)
	typedef drain::SingleFlagger<unsigned int>::dict_t dict_t;

	static const dict_t compressionDict;
	/*
	static
	const dict_t & getCompressionDict();
	*/

	// https://www.awaresystems.be/imaging/tiff/tifftags/compression.html
	// https://gdal.org/drivers/raster/cog.html
	static dict_t::value_t defaultCompression;      // COMPRESSION_NONE = 1; COMPRESSION_LZW = 5;
	// static int defaultCompressionLevel; // COMPRESSION_NONE = 1; COMPRESSION_LZW = 5;

	static drain::Frame2D<int> defaultTile;


#ifndef USE_GEOTIFF_NO


	inline
	FileTIFF(const std::string & path = "", const std::string & mode = "w") : tif(nullptr), tile(defaultTile){ // FileHandler(__FUNCTION__),
		if (!path.empty())
			open(path, mode);
		//tif = XTIFFOpen(path.c_str(), mode);
	}

	virtual inline
	~FileTIFF(){
		//mout.special("Closing FileTIFF");
		close();
	}

	virtual inline
	void open(const std::string & path, const std::string & mode = "w"){
		tif = XTIFFOpen(path.c_str(), mode.c_str());
	}

	virtual inline
	bool isOpen() const {
		return (tif != nullptr);
	}

	virtual	inline
	void close(){
		if (isOpen()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.debug("Closing TIFF...");
			XTIFFClose(tif);
			tif = nullptr;
		}
	}

	inline
	int setField(int tag, const std::string & value){
		if (!isOpen()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.error("TIFF file not open");
		}
		return TIFFSetField(tif, tag, value.c_str());
	}

	template <class T>
	inline
	int setField(int tag, const std::vector<T> & value){
		if (!isOpen()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.error("TIFF file not open");
			//return 0;
		}
		return TIFFSetField(tif, tag, value.size(), &value.at(0));
	}

	template <class T>
	inline
	int setField(int tag, T value){
		if (!isOpen()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.error("TIFF file not open");
			//return 0;
		}
		return TIFFSetField(tif, tag, value);
	}


	inline
	void useDefaultTileSize(){
		this->tile = defaultTile;
	}

	inline
	void setTileSize(int tileWidth, int tileHeight = 0){
		if (tileWidth == 0){
			tile = {0,0};
			//this->tile = defaultTile;
		}
		else {
			tile.setWidth(tileWidth);
			if (tileHeight == 0){
				tileHeight = tileWidth;
			}
			tile.setHeight(tileHeight);
		}
	}

	/**
	 */
	void setTime(const drain::Time & time);

	/**
	 *
	 */
	void setDefaults(); //, int tileWidth=0, int tileHeight = 0);
	//void setDefaults(const drain::image::ImageConf & src); //, int tileWidth=0, int tileHeight = 0);

	void writeImageData(const drain::image::Image & src);

	/// Default implementation.
	/**
	 *  Practically, on should develop own method for adding metadata.
	 */
	static
	void write(const std::string & path, const drain::image::Image & src);


protected:

	TIFF *tif;

	drain::Frame2D<int> tile;


#endif

};

} // image::

} // drain::



#endif
