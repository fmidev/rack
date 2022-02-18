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

/*

   Based on the example code by Niles D. Ritter,
   http://svn.osgeo.org/metacrs/geotiff/trunk/libgeotiff/bin/makegeo.c

 */

#include "FileGeoTIFF.h"

#ifndef USE_GEOTIFF_NO


//#include "drain/util/Log.h"
#include "drain/util/StringBuilder.h"
#include "drain/util/TreeXML.h"
#include "drain/image/AccumulatorGeo.h"

//int rack::FileGeoTIFF::compression(1); // = NONE, but see below

// https://en.wikipedia.org/wiki/TIFF
// https://www.awaresystems.be/imaging/tiff/tifftags/gdal_nodata.html
#ifndef TIFFTAG_GDAL_METADATA //# ASCII tag (code 42113
#define TIFFTAG_GDAL_METADATA 42112 // 0xa481 // 42113
#endif

#ifndef TIFFTAG_GDAL_NODATA //# ASCII tag (code 42113
#define TIFFTAG_GDAL_NODATA 42113 // 0xa481 // 42113
#endif

namespace drain
{

namespace image
{




// https://www.awaresystems.be/imaging/tiff/tifftags/gdal_metadata.html
class NodeGDAL: public drain::NodeXML {
public:

	enum type { ROOT, ITEM }; // check CTEXT, maybe implement in XML

	NodeGDAL(type t = ROOT);

	void set(const drain::Variable & ctext, int sample=0, const std::string & role = "");

protected:

	void setType(type t);

	int sample;

	std::string role;

};

NodeGDAL::NodeGDAL(type t){
	setType(t);
	this->id = -1;
	//this->name = "~";
}

void NodeGDAL::setType(type t){

	if (t == ROOT){
		tag = "GDALMetadata";
	}
	else {
		tag = "Item";
		link("sample", sample = 0);
		link("role",   role = "");
	}

}

void NodeGDAL::set(const drain::Variable & ctext, int sample, const std::string & role){
	setType(ITEM);
	this->ctext  = ctext.toStr();
	this->sample = sample;
	this->role   = role;

	/*
	for (drain::ReferenceMap::const_iterator it = this->begin(); it != this->end(); it++){
		std::cerr << tag << '=' << it->first << ':' << it->second << '\n';
	};
	 */

}


typedef drain::Tree<NodeGDAL> TreeGDAL;

inline
std::ostream & operator<<(std::ostream &ostr, const TreeGDAL & tree){
	return drain::NodeXML::toOStr(ostr, tree);
}


void FileGeoTIFF::setGdalMetaData(const std::string & nodata, double scale, double offset){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//const int TIFFTAG_KOE = 65001;
	// { TIFFTAG_KOE,           -1, -1, TIFF_DOUBLE, FIELD_CUSTOM, true, 0, const_cast<char*>("koe") },
	// "gdal-metadata"
	static
	const TIFFFieldInfo xtiffFieldInfo[] = {
			{ TIFFTAG_GDAL_METADATA, -1, -1, TIFF_ASCII,  FIELD_CUSTOM, true, 0, const_cast<char*>("GDAL_METADATA") },
			{ TIFFTAG_GDAL_NODATA,    1,  1, TIFF_ASCII,  FIELD_CUSTOM, true, 0, const_cast<char*>("nodata-marker") },
	};
	TIFFMergeFieldInfo(tif, xtiffFieldInfo, 2);

	TreeGDAL gdalInfo;
	gdalInfo["SCALE"]->set(scale, 0, "scale");
	gdalInfo["OFFSET"]->set(offset, 0, "offset");
	/*
	<GDALMetadata >
	<Item name="OFFSET" role="offset" sample="0" >-32</Item>
	<Item name="SCALE" role="scale" sample="0" >0.5</Item>
	</GDALMetadata>
	 */

	std::stringstream gdal;
	gdal << gdalInfo;
	mout.debug() << gdal.str() << mout.endl;
	setField(TIFFTAG_GDAL_METADATA,gdal.str());

	//std::string nodata = prop["what:nodata"];
	if (!nodata.empty()){
		// http://stackoverflow.com/questions/24059421/adding-custom-tags-to-a-tiff-file
		mout.info() << "registering what:nodata => nodata=" << nodata << mout.endl;
		setField(TIFFTAG_GDAL_NODATA,nodata);
	}
	// usr/include/gdal/rawdataset.h

	// Non-standard http://www.gdal.org/frmt_gtiff.html
	/*
	std::string nodata = src.properties["what:nodata"];
	if (!nodata.empty()){
		mout.toOStr() << "registering what:nodata => nodata=" << nodata << mout.endl;
		GTIFKeySet(gtif, (geokey_t)TIFFTAG_GDAL_NODATA, TYPE_ASCII, nodata.length()+1, nodata.c_str());  // yes, ascii
	}
	 */


}

/*
void FileGeoTIFF::adjustGeoFrame_rack(const drain::image::Image & src, drain::image::GeoFrame & frame){

	drain::Logger mout(__FILE__, __FUNCTION__);

	const drain::FlexVariableMap & prop = src.properties;

	std::string projdef = prop["where:projdef"];

	if (!projdef.empty()){

		//drain::image::GeoFrame frame;
		frame.setGeometry(src.getWidth(), src.getHeight());
		frame.setProjection(projdef);

	    drain::Rectangle<double> bboxD(prop["where:LL_lon"], prop["where:LL_lat"], prop["where:UR_lon"], prop["where:UR_lat"] );
		if (frame.isLongLat()){
			frame.setBoundingBoxD(bboxD);
		}
		else {
			// Debug
			frame.setBoundingBoxD(bboxD);
			mout.special("BBOX deg: ", frame.getBoundingBoxD());
			mout.special() << "BBOX m  : ";
			mout.precision(20);
			mout << frame.getBoundingBoxM() << mout;


			const drain::Variable & p = prop["where:BBOX_native"];
			std::vector<double> v;
			//drain::Rectangle<double> v;
			p.toSequence(v);
			drain::Rectangle<double> bboxM;
			bboxM.fromSequence(v);
			if (v.size() == 4){
				//frame.setBoundingBoxM(v[0], v[1], v[2], v[3]);
				frame.setBoundingBoxM(bboxM);
				mout.note() << "Setting exact (metric) BBOX=";
				//char sep = ' ';
				//v = frame.getBoundingBoxM().toVector(); // Back!
				mout.special() << "BBOX m  : ";
				mout.precision(20);
				mout << frame.getBoundingBoxM() << mout;
			}
			else {
				mout.warn() << "where:BBOX_native (" << p << ") missing or invalid, using bbox in degrees (approximative)" << mout.endl;
				frame.setBoundingBoxD(bboxD);
			}
		}
	}
	else {
		mout.note("where:projdef missing, no GeoTIFF projection info written");
	}

}
 */

/*
void FileGeoTIFF::setUpTIFFDirectory_rack(const drain::image::Image & src){ // int tileWidth, int tileHeight){

	drain::Logger mout(__FILE__, __FUNCTION__);

	const drain::FlexVariableMap & prop = src.properties;

	// http://www.gdal.org/frmt_gtiff.html
	// Optional
	//const std::string software = std::string(__RACK__) + " " + std::string(__RACK_VERSION__);
	//TIFFSetField(tif,TIFFTAG_SOFTWARE, software.c_str());
	//std::string s = drain::StringBuilder(__RACK__," ",__RACK_VERSION__);
	setField(TIFFTAG_SOFTWARE,(const std::string &) drain::StringBuilder(__RACK__," ",__RACK_VERSION__));

	drain::Time datetime;
	datetime.setTime(prop.get("what:date", "19700101"), "%Y%m%d");
	datetime.setTime(prop.get("what:time", "000000"), "%H%M%S");
	setTime(datetime);

	//TIFFSetField(tif, TIFFTAG_DATETIME, datetime.str("%Y:%m:%d %H:%M:%S").c_str() );
	// const std::string datetime = prop.get("what:date", "") + prop.get("what:time", "");
	//TIFFSetField(tif, TIFFTAG_DATETIME, datetime.c_str() );


	const std::string desc = prop.get("what:object", "") + ":"+ prop.get("what:product", "") + ":" + prop.get("what:prodpar", "") + ":" + prop.get("what:quantity", "");
	//TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, desc.c_str());
	setField(TIFFTAG_IMAGEDESCRIPTION,desc);

	// GDALMetadata etc
	// usr/include/gdal/rawdataset.h
	// Non-standard http://www.gdal.org/frmt_gtiff.html

	setGdalMetaData(prop["what:nodata"], prop.get("what:gain", 1.0), prop.get("what:offset", 0.0));

	drain::image::GeoFrame frame;
	adjustGeoFrame_rack(src, frame);
	setGeoMetaData(frame);

}
 */

void FileGeoTIFF::setGeoMetaData(const drain::image::GeoFrame & frame){

	drain::Logger mout(__FILE__, __FUNCTION__);
	//const int width/2;
	//const int j = height/2;

	// Image coords
	drain::Point2D<int>    imagePos;

	// Geographical coords (degrees or meters)
	drain::Point2D<double>   geoPos;

	if (frame.isLongLat()){
		//imagePos.setLocation(width/2, height/2);
		imagePos.setLocation(frame.getFrameWidth()/2, frame.getFrameHeight()/2); // TODO: CHECK
		frame.pix2LLdeg(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
	}
	else { // metric
		imagePos.setLocation(0, 0); //int(height));
		frame.pix2m(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
		//frame.pix2m(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
	}

	//double tiepoints[6]; // = {0,0,0,0,0,0};
	std::vector<double> tiepoints(6); // = {0,0,0,0,0,0};
	tiepoints[0] = static_cast<double>(imagePos.x);
	tiepoints[1] = static_cast<double>(imagePos.y);
	tiepoints[2] = 0.0;
	tiepoints[3] = geoPos.x;
	tiepoints[4] = geoPos.y;
	tiepoints[5] = 0.0;
	mout.debug() << "Tiepoint: " << imagePos << " => " << geoPos << mout.endl;

	// consider
	//TIFFSetField(tif,TIFFTAG_GEOTIEPOINTS, 6,tiepoints);
	setField(TIFFTAG_GEOTIEPOINTS,tiepoints);

	//double pixscale[3]; // = {1,1,0};
	std::vector<double> pixscale(3);
	//std::cerr << "frame: " << frame.getProjection() << '\n';
	//const drain::Rectangle<double> & bbox = frame.isLongLat() ? bboxD : frame.getBoundingBoxM();
	const drain::Rectangle<double> & bbox = frame.isLongLat() ? frame.getBoundingBoxD() : frame.getBoundingBoxM();
	//mout.experimental("check", )
	//frame.getXScale()?
	mout.debug() << "Scale: " << frame.getXScale() << ", " << frame.getYScale() << mout.endl;

	pixscale[0] = bbox.getWidth()  / static_cast<double>(frame.getFrameWidth()); // upperRight.x - bbox.lowerLeft.x
	pixscale[1] = bbox.getHeight() / static_cast<double>(frame.getFrameHeight()); // bbox.upperRight.y - bbox.lowerLeft.y
	pixscale[2] = 0.0;

	mout.debug() << "BBox: " << bbox << mout.endl;
	mout.debug() << "ScaleX: " << pixscale[0] << ' ' << bbox.getWidth() << ' ' << frame.getFrameWidth()  << mout.endl; // << ' ' << width
	mout.debug() << "ScaleY: " << pixscale[1] << mout.endl;

	//printf("$(( %.10f - %.10f )) = %.10f", bbox.upperRight.x, bbox.lowerLeft.x, bbox.getWidth());

	// mout.debug() << "Noh: " << (static_cast<double>(1280000) / static_cast<double>(1280)) << mout.endl;
	// mout.debug() << "Noh: " << (static_cast<double>(bbox.getWidth()) / static_cast<double>(frame.getFrameWidth())) << mout.endl;

	//TIFFSetField(tif,TIFFTAG_GEOPIXELSCALE, 3, pixscale);
	setField(TIFFTAG_GEOPIXELSCALE,pixscale);


}





void FileGeoTIFF::setProjection(const std::string & projstr){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!projstr.empty()){
		mout.info() << "projstr= " << projstr << mout.endl;
		drain::Proj4 proj;
		proj.setProjectionDst(projstr);
		setProjection(proj);
	}
	else {
		mout.warn() << "where:projdef empty, skipping (writing plain TIFF)" << mout.endl;
	}
}


void FileGeoTIFF::setProjection(const drain::Proj4 & proj){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (proj.isLongLat()){
		mout.info() << "Writing EPSG:4326 (longlat)" << mout.endl;
		//SetUpGeoKeys_4326_LongLat(gtif);
		setProjectionLongLat();
	}
	else {
		mout.info() << "Writing metric projection" << mout.endl;
		//GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT,  1, GCSE_WGS84);
		//int projOK = GTIFSetFromProj4(gtif, projstr.c_str());
		if (!GTIFSetFromProj4(gtif, proj.getProjectionDst().c_str())){ // CHECK!!
			mout.warn() << "Failed in setting GeoTIFF projection: " << proj << mout.endl;
			mout.note() << "Consider: gdal_translate -a_srs '" << proj.getProjectionDst() << "' file.tif out.tif" << mout.endl;
		}
	}


	// NEW
	GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT,  1, RasterPixelIsArea);
	//GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT,  1, RasterPixelIsPoint);
	/*
	 */

}

void FileGeoTIFF::setProjectionLongLat(){
	GTIFKeySet(gtif, GTModelTypeGeoKey,       TYPE_SHORT,  1, ModelGeographic);
	GTIFKeySet(gtif, GTRasterTypeGeoKey,      TYPE_SHORT,  1, RasterPixelIsArea); // Also in main function
	//GTIFKeySet(gtif, GTRasterTypeGeoKey,      TYPE_SHORT,  1, RasterPixelIsPoint);

	// GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT,  1, GCSE_WGS84); <= WRONG! GCS_WGS_84
	GTIFKeySet(gtif, GeographicTypeGeoKey,    TYPE_SHORT,  1, GCS_WGS_84); // 4326 correct!
	GTIFKeySet(gtif, GeogCitationGeoKey,      TYPE_ASCII,  7, "WGS 84");
	GTIFKeySet(gtif, GeogAngularUnitsGeoKey,  TYPE_SHORT,  1, Angular_Degree);
	//GTIFKeySet(gtif, GeogSemiMajorAxisGeoKey, TYPE_DOUBLE, 1, 6378137.0);  //6377298.556);
	//GTIFKeySet(gtif, GeogInvFlatteningGeoKey, TYPE_DOUBLE, 1, 298.257223563);// 300.8017);
}



} // image::

} // drain::

#endif
