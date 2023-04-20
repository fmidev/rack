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

#include "drain/util/Flags.h"
#include "drain/util/TreeXML.h"


#include <geotiff.h>
#include <geotiffio.h>
#include <geo_normalize.h>


namespace drain
{

namespace image
{

// https://www.awaresystems.be/imaging/tiff/tifftags/gdal_metadata.html
class NodeGDAL: public drain::NodeXML {
public:

	enum type { UNDEFINED, ROOT, ITEM, USER }; // check CTEXT, maybe implement in XML

	NodeGDAL(type t = USER);

	void setGDAL(const drain::Variable & ctext, int sample=0, const std::string & role = "");

	// Set user attribute
	void setText(const drain::Variable & value);

	// Set user attribute
	void setText(const std::string & value);

	void setType(type t);

protected:

	/// Standard GDAL attribute
	int sample;

	/// Standard GDAL attribute
	std::string role;

	// Multi-purpose key
	// std::string value;


};



typedef drain::UnorderedMultiTree<NodeGDAL> TreeGDAL;

inline
std::ostream & operator<<(std::ostream &ostr, const TreeGDAL & tree){
	return drain::NodeXML::toOStr(ostr, tree);
}


/** Extends TIFF by adding geo information in the metadata
 *
 *  https://gdal.org/drivers/raster/gtiff.html
 *
 */
class FileGeoTIFF : public FileTIFF {
public:

	// typedef std::map<short, std::list<std::pair<geokey_t, drain::Variable> > > epsg_map_t;
	// static epsg_map_t epsgConf;

	FileGeoTIFF() : FileTIFF(), gtif(nullptr){
		gdalInfo.data.setType(NodeGDAL::ROOT);
	}

	FileGeoTIFF(const std::string & path, const std::string & mode = "w") : FileTIFF(), gtif(nullptr){
		gdalInfo.data.setType(NodeGDAL::ROOT);
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

	/*
	inline
	void setGeoTiffField(geokey_t tag, short int value){
		GTIFKeySet(gtif, tag, TYPE_SHORT, 1, value);
	}

	inline
	void setGeoTiffField(geokey_t tag, int value){
		GTIFKeySet(gtif, tag, TYPE_SHORT, 1, value);
	}
	*/

	inline
	void setGeoTiffField(geokey_t tag, const char *value){
		GTIFKeySet(gtif, tag, TYPE_ASCII, strlen(value)+1, value);
	}

	inline
	void setGeoTiffField(geokey_t tag, const std::string & value){
		GTIFKeySet(gtif, tag, TYPE_ASCII, value.size()+1, value.c_str());
	}

	template <typename T>
	void setGeoTiffField(geokey_t tag, T value){
		if ((std::is_enum<T>::value) || (std::is_integral<T>::value)){
			GTIFKeySet(gtif, tag, TYPE_SHORT, 1, static_cast<short int>(value));
		}
		else {
			drain::Logger mout(__FUNCTION__, __FILE__);
			mout.warn("TAG: ", tag, ", value:", value, ", type=", typeid(T).name()); // , drain::Type::call<drain::nameGetter>(typeid(T)));
			mout.error("Not implemented");
		}
	}

	/// Specialize only for strings
	/*
	template <typename T>
	int setGeoTiffField(int tag, T value){

		if (!isOpen()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.error("GeoTIFF file not open");
			return -1;
		}

		tagtype_t tag_type = getTagType<T>();
		switch (tag_type) {
			case TYPE_ASCII:
				// GTIFKeySet(gtif, tag, TYPE_ASCII, 7, "WGS 84");
				break;
			default:
				break;
		}
		return 0;
	}
	*/


	/// Completes GeoTIFF structure
	void writeMetadata();

	virtual
	void close();

	/// Todo: subclass
	inline virtual
	bool isOpen() const {
		return FileTIFF::isOpen() && (gtif != nullptr);
	}

	/// Require strict GeoTIFF compliance
	/**
	 *  TODO: consider TIFF,default,GTIFF,StrictGTIFF
	 */
	static
	bool strictCompliance;

	typedef enum {TIFF=1, GEOTIFF=2, GEOTIFF_EXT=3} TiffCompliance;

	typedef drain::EnumFlagger<drain::SingleFlagger<TiffCompliance> > tiffComplianceFlagger;

	static
	const drain::FlaggerDict & getComplianceDict();

	static
	int compliance;
	//TiffCompliance compliance;
	/// Use EPSG specific support only, if found. Else use also fromProj4Str().
	// static	bool plainEPSG;

	TreeGDAL gdalInfo;
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

	//inline
	void setProjectionLongLat();

	/// If EPSG is detected (currently by +init=epsg:EPSG) and support configured for EPSG code, set it directly.
	// Will replace latLon (EPSG=4326) as well?
	//inline
	void setProjectionEPSG(short epsg);

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

	/*
	template <typename T>
	inline
	tagtype_t getTagType(){
		const typename tagtype_map_t::const_iterator it = tagtype_map.find(&typeid(T));
		if (it != tagtype_map.end()){
			return it->second;
		}
		else {
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.warn("unknown/unsupported GTIFF tag basetype: ", typeid(T).name());
			return TYPE_UNKNOWN;
		}
	}
	*/

protected:

	/*
	typedef std::map<const std::type_info *,tagtype_t> tagtype_map_t;

	static
	const tagtype_map_t tagtype_map;
	*/

	GTIF *gtif;
	// TIFF *tif = XTIFFOpen(path.c_str(), "w");
	// GTIF *gtif = GTIFNew(tif);

};

/*

template <>
inline
tagtype_t  FileGeoTIFF::getTagType<unsigned char>(){ return TYPE_BYTE;};

template <>
inline
tagtype_t  FileGeoTIFF::getTagType<unsigned short>(){ return TYPE_SHORT;}

template <>
inline
tagtype_t  FileGeoTIFF::getTagType<unsigned long>(){ return TYPE_LONG;};

template <>
inline
tagtype_t  FileGeoTIFF::getTagType<std::string>(){ return TYPE_ASCII;};

template  <>
inline
tagtype_t FileGeoTIFF::getTagType<float>(){ return TYPE_FLOAT;};

template <>
inline
tagtype_t  FileGeoTIFF::getTagType<double>(){ return TYPE_DOUBLE;};

template <>
inline
tagtype_t  FileGeoTIFF::getTagType<signed char>(){ return TYPE_SBYTE;};

template <>
inline
tagtype_t  FileGeoTIFF::getTagType<signed short>(){ return TYPE_SSHORT;};

template <>
inline
tagtype_t  FileGeoTIFF::getTagType<signed long>(){ return TYPE_SLONG;};
//inline getTagType<>(){ return TYPE_UNKNOWN);
//inline getTagType<>(){ return TYPE_RATIONAL);
*/

} // image::

} // drain::
#endif

#endif
