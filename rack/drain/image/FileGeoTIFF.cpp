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


typedef drain::UnorderedMultiTree<NodeGDAL> TreeGDAL;

inline
std::ostream & operator<<(std::ostream &ostr, const TreeGDAL & tree){
	return drain::NodeXML::toOStr(ostr, tree);
}

bool FileGeoTIFF::strictCompliance(false);

// bool FileGeoTIFF::plainEPSG(false);
bool FileGeoTIFF::plainEPSG(true);


/*
GTIFKeySet(gtif, ProjectedCSTypeGeoKey,   TYPE_SHORT,  1, 3067);
GTIFKeySet(gtif, GeogGeodeticDatumGeoKey, TYPE_SHORT,  1, Datum_European_Reference_System_1989);
*/
FileGeoTIFF::epsg_map_t FileGeoTIFF::epsgConf = {
		{3067, {
				{ProjectedCSTypeGeoKey, 3067},
				{GeogGeodeticDatumGeoKey, Datum_European_Reference_System_1989}
		}},
		{0000, {
				{GeogGeodeticDatumGeoKey, Datum_European_Reference_System_1989}
		}
		}
};

/*
typedef std::map<int, std::string > mydict_t;

const mydict_t MD = {
		{3067, "MIKA"}
};
*/

void FileGeoTIFF::open(const std::string & path, const std::string & mode){
	drain::Logger mout(__FILE__, __FUNCTION__);
	if (isOpen()){
		// drain::Logger mout(__FILE__, __FUNCTION__);
		mout.warn("GeoTIFF already open?");
	}
	else {
		FileTIFF::open(path, mode);
		gtif = GTIFNew(tif);
		if (!isOpen()){
			mout.error("failed creating GeoTIFF file from TIFF object");
			//mout.error() << "failed creating GeoTIFF file from TIFF object, path=" << path << mout.endl;
		}
		//mout.attention("GeoTIFF version: ", gtif->hdr_version, '.', gtif->hdr_rev_major,  '.', gtif->hdr_rev_minor,  '.', gtif->hdr_num_keys);
		//open();
	}
}

/// "Opens" a GeoTIFF structure inside an opened TIFF file.
/*
virtual
void FileGeoTIFF::open(){
	gtif = GTIFNew(tif);
	if (!isOpen()){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.error("failed creating GeoTIFF file from TIFF object");
		//mout.error() << "failed creating GeoTIFF file from TIFF object, path=" << path << mout.endl;
	}
	else {
		// gtif
	}
}
*/

void FileGeoTIFF::close(){
	if (isOpen()){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.debug("Closing GeoTIFF...");
		//GTIFWriteKeys(gtif); // moved to writeMetadata() for cloud optimized GeoTIFF, COG.
		enum {VERSION=0,MAJOR,MINOR};
		int version[3] = {0,0,0};
		int keycount = 0;
		GTIFDirectoryInfo(gtif, version, &keycount);
		mout.special("GTIFDirectoryInfo: version:", version[VERSION], '.', version[MAJOR], '.', version[MINOR], " keycount:", keycount);
		GTIFFree(gtif);
		gtif = nullptr;
	}
	FileTIFF::close(); // ?
}


void FileGeoTIFF::setGdalScale(double scale, double offset){

	// TODO: separate code without nodata marker?
	// void FileGeoTIFF::setGdalMetaData(double nodata, double scale, double offset){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// const int TIFFTAG_KOE = 65001;
	// { TIFFTAG_KOE,           -1, -1, TIFF_DOUBLE, FIELD_CUSTOM, true, 0, const_cast<char*>("koe") },
	// "gdal-metadata"
	static
	const TIFFFieldInfo xtiffFieldInfo[] = {
			{ TIFFTAG_GDAL_METADATA, -1, -1, TIFF_ASCII,  FIELD_CUSTOM, true, 0, const_cast<char*>("GDAL_METADATA") },
	};
	TIFFMergeFieldInfo(tif, xtiffFieldInfo, 1);

	TreeGDAL gdalInfo;
	gdalInfo["SCALE"]->set(scale, 0, "scale");
	gdalInfo["OFFSET"]->set(offset, 0, "offset");
	/*
	    <GDALMetadata >
	    <Item name="OFFSET" role="offset" sample="0" >-32</Item>
	    <Item name="SCALE"  role="scale" sample="0" >0.5</Item>
	    </GDALMetadata>
	*/

	std::stringstream gdal;
	gdal << gdalInfo;
	mout.debug(gdal.str());
	setField(TIFFTAG_GDAL_METADATA, gdal.str());

}

void FileGeoTIFF::setGdalNoData(const std::string & nodata){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (nodata.empty()){
		return;
	}

	// const int TIFFTAG_KOE = 65001;
	// { TIFFTAG_KOE,           -1, -1, TIFF_DOUBLE, FIELD_CUSTOM, true, 0, const_cast<char*>("koe") },
	// "gdal-metadata"
	static
	const TIFFFieldInfo xtiffFieldInfo[] = {
			{ TIFFTAG_GDAL_NODATA,    1,  1, TIFF_ASCII,  FIELD_CUSTOM, true, 0, const_cast<char*>("nodata-marker") },
	};
	TIFFMergeFieldInfo(tif, xtiffFieldInfo, 1);


	// std::string nodata = prop["what:nodata"];
	// http://stackoverflow.com/questions/24059421/adding-custom-tags-to-a-tiff-file
	mout.info() << "registering what:nodata => nodata=" << nodata << mout.endl;
	// TODO: separate code without nodata marker?
	setField(TIFFTAG_GDAL_NODATA, nodata); // or should be string?
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
		setProjectionLongLat(); // GeoTIFF, FIXED 2023/02
	}
	else { // metric
		imagePos.setLocation(0, 0); //int(height));
		frame.pix2m(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
		//frame.pix2m(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
		setProjection(frame.projR2M); // GeoTIFF, FIXED 2023/02
	}

	//double tiepoints[6]; // = {0,0,0,0,0,0};
	std::vector<double> tiepoints(6); // = {0,0,0,0,0,0};
	tiepoints[0] = static_cast<double>(imagePos.x);
	tiepoints[1] = static_cast<double>(imagePos.y);
	tiepoints[2] = 0.0;
	tiepoints[3] = geoPos.x;
	tiepoints[4] = geoPos.y;
	tiepoints[5] = 0.0;
	mout.debug("Tiepoint: ", imagePos, " => ", geoPos);

	// consider
	//TIFFSetField(tif,TIFFTAG_GEOTIEPOINTS, 6,tiepoints);
	setField(TIFFTAG_GEOTIEPOINTS,tiepoints);

	//double pixscale[3]; // = {1,1,0};
	std::vector<double> pixscale(3);

	const drain::Rectangle<double> & bbox = frame.isLongLat() ? frame.getBoundingBoxD() : frame.getBoundingBoxM();

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

	setField(TIFFTAG_GEOPIXELSCALE, pixscale);


}





void FileGeoTIFF::setProjection(const std::string & projstr){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!projstr.empty()){
		mout.info("projstr= ", projstr);
		drain::Proj4 proj;
		proj.setProjectionDst(projstr);
		setProjection(proj);
	}
	else {
		mout.warn("projstr empty: skipping GeoTIFF and writing plain TIFF)");
	}
}


bool FileGeoTIFF::setProjectionEPSG(short epsg){

	drain::Logger mout(__FILE__, __FUNCTION__);

	epsg_map_t::const_iterator it = epsgConf.find(epsg);
	if (it !=  epsgConf.end()){
		//mout.experimental("detected epsg:", epsg, ", using additional support");

		for (const auto & entry: it->second){
			mout.experimental("setting (EPSG: ", epsg, "): ", entry.first, '=' , entry.second);
			GTIFKeySet(gtif, entry.first,   TYPE_SHORT,  1, entry.second);
			//GTIFKeySet(gtif, ProjectedCSTypeGeoKey,   TYPE_SHORT,  1, 3067);
		}

		return true;
	}
	else {
		mout.warn("No support configured for epsg:", epsg);
		return false;
	}

	/*
	typedef std::map<std::string, std::string> map_t;
	map_t projArgMap;
	for (const std::string & arg: projArgs){
		drain::MapReader<std::string, std::string>::read(arg, projArgMap);
	}

	for (const auto & entry: projArgMap){
		mout.warn(entry.first, entry.second);
	}

	map_t::const_iterator it = projArgMap.find("+init");
	if (it != projArgMap)
	*/

}


void FileGeoTIFF::setProjection(const drain::Proj4 & proj){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//mout.attention("start");
	//mout.attention(proj);

	if (proj.isLongLat()){
		mout.info("Writing EPSG:4326 (longlat)");
		//SetUpGeoKeys_4326_LongLat(gtif);
		setProjectionLongLat();
	}
	else {
		const std::string & dstProj = proj.getProjectionDst();
		//const std::string dstProj = "+init=epsg:3067";

		const short epsg = drain::Proj4::pickEpsgCode(dstProj);

		bool USE_EPSG = (epsg>0) && (epsgConf.find(epsg) != epsgConf.end());

		mout.info("Writing metric projection: ", dstProj);
		//ProjectedCSTypeGeoKey();
		//GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT,  1, GCSE_WGS84);
		// https://github.com/OSGeo/libgeotiff/issues/53
		if (plainEPSG && USE_EPSG){
			mout.experimental("using only: EPSG:", epsg);
			setProjectionEPSG(epsg);
		}
		else if (GTIFSetFromProj4(gtif, dstProj.c_str())){ // CHECK!!

			mout.ok("GTIFSetFromProj4: ", dstProj);

			if (USE_EPSG){
				mout.experimental("complementing with EPSG:", epsg);
				setProjectionEPSG(epsg);
			}


			/*
			GTIFKeySet(gtif, ProjectedCSTypeGeoKey,   TYPE_SHORT,  1, 3067);
			GTIFKeySet(gtif, GeogGeodeticDatumGeoKey, TYPE_SHORT,  1, Datum_European_Reference_System_1989);
			*/
		}
		else {
			mout.warn("Failed in setting GeoTIFF projection: ", dstProj);
			mout.note("Consider: gdal_translate -a_srs '", dstProj, "' file.tif out.tif");
			if (FileGeoTIFF::strictCompliance){
				mout.error("GeoTIFF error under strict compliance (requested by user)");
			}

		}
	}


	// NEW
	GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT,  1, RasterPixelIsArea); // Repeated? Also in setProjectionLongLat?
	//GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT,  1, RasterPixelIsPoint);
	/*
	 */

}

void FileGeoTIFF::setProjectionLongLat(){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.info("Setting Long-Lat projection");

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
