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


#include <drain/StringBuilder.h>
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

FileGeoTIFF::complianceFlagger FileGeoTIFF::compliancyFlagger(EPSG);
std::string FileGeoTIFF::compliancy; // = FileGeoTIFF::flagger.str()

// TODO: compare with
// https://gdal.org/drivers/raster/gtiff.html#creation-options :
// PROFILE=[GDALGeoTIFF/GeoTIFF/BASELINE]: Control what non-baseline tags are emitted by GDAL.

template <>
const drain::FlaggerDict drain::EnumDict<FileGeoTIFF::TiffCompliance>::dict = {
		{"UNDEFINED",   drain::image::FileGeoTIFF::TiffCompliance::UNDEFINED},
		{"TIFF",        drain::image::FileGeoTIFF::TiffCompliance::TIFF},
		{"GEOTIFF",     drain::image::FileGeoTIFF::TiffCompliance::GEOTIFF},  // consider PROJ4 ?
		{"EPSG",        drain::image::FileGeoTIFF::TiffCompliance::EPSG},
		{"STRICT",      drain::image::FileGeoTIFF::TiffCompliance::STRICT}
};

/*
const drain::FlaggerDict & FileGeoTIFF::getComplianceDict(){

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.error("design");

	return drain::EnumDict<FileGeoTIFF::TiffCompliance>::dict;
}
*/

//bool FileGeoTIFF::plainEPSG(false);
//bool FileGeoTIFF::plainEPSG(true);

// Save this for now. Future option: external conf.
/*
const FileGeoTIFF::tagtype_map_t FileGeoTIFF::tagtype_map = {
   {&typeid(unsigned short), TYPE_SHORT},
   {&typeid(  signed short), TYPE_SSHORT}
};
typedef enum {
   TYPE_BYTE=1,
   TYPE_SHORT=2,
   TYPE_LONG=3,
   TYPE_RATIONAL=4,
   TYPE_ASCII=5,
   TYPE_FLOAT=6,
   TYPE_DOUBLE=7,
   TYPE_SBYTE=8,
   TYPE_SSHORT=9,
   TYPE_SLONG=10,
   TYPE_UNKNOWN=11
} tagtype_t;
*/

/*
GTIFKeySet(gtif, ProjectedCSTypeGeoKey,   TYPE_SHORT,  1, 3067);
GTIFKeySet(gtif, GeogGeodeticDatumGeoKey, TYPE_SHORT,  1, Datum_European_Reference_System_1989);
*/

// Save this for now. Future option: external conf.
/*
FileGeoTIFF::epsg_map_t FileGeoTIFF::epsgConf = {
		{4326, { // https://epsg.io/4326
				{GTModelTypeGeoKey, ModelGeographic},
				{GeographicTypeGeoKey,GCS_WGS_84},
				{GeogCitationGeoKey, "WGS 84"},
				{GeogGeodeticDatumGeoKey, Datum_European_Reference_System_1989}
		}},
		{303500, { // https://epsg.io/3035
				{ProjectedCSTypeGeoKey, 3035},
				{GeogGeodeticDatumGeoKey, Datum_European_Reference_System_1989}
		}},
		{3067, { // https://epsg.io/3067
				{ProjectedCSTypeGeoKey, 3067},
				{GeogGeodeticDatumGeoKey, Datum_European_Reference_System_1989}
		}},
		{3035, {
				{ProjectedCSTypeGeoKey, 3067},
				{GeogGeodeticDatumGeoKey, "Test"}
		}
		}
};
*/

/*
GTIFKeySet(gtif, GTModelTypeGeoKey,       TYPE_SHORT,  1, ModelGeographic);
GTIFKeySet(gtif, GTRasterTypeGeoKey,      TYPE_SHORT,  1, RasterPixelIsArea); // Also in main function
//GTIFKeySet(gtif, GTRasterTypeGeoKey,      TYPE_SHORT,  1, RasterPixelIsPoint);

// GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT,  1, GCSE_WGS84); <= WRONG! GCS_WGS_84
GTIFKeySet(gtif, GeographicTypeGeoKey,    TYPE_SHORT,  1, GCS_WGS_84); // 4326 correct!
GTIFKeySet(gtif, GeogCitationGeoKey,      TYPE_ASCII,  7, "WGS 84");
GTIFKeySet(gtif, GeogAngularUnitsGeoKey,  TYPE_SHORT,  1, Angular_Degree);
*/
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
			//mout.error("failed creating GeoTIFF file from TIFF object, path=" , path );
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
		//mout.error("failed creating GeoTIFF file from TIFF object, path=" , path );
	}
	else {
		// gtif
	}
}
*/

void FileGeoTIFF::writeMetadata(){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (isOpen()){

		static
		const TIFFFieldInfo xtiffFieldInfo[] = {
				{ TIFFTAG_GDAL_METADATA, -1, -1, TIFF_ASCII,  FIELD_CUSTOM, true, 0, const_cast<char*>("GDAL_METADATA") },
		};
		TIFFMergeFieldInfo(tif, xtiffFieldInfo, 1);

		// Write metadata in XML format
		std::stringstream sstr;
		NodeGDAL::toStream(sstr, gdalMetadata);
		//sstr << gdalMetadata;
		mout.special<LOG_INFO>("GDAL XML metadata:\n", sstr.str());
		if (mout.isDebug()){
			mout.special<LOG_INFO>("GDAL XML tree:\n");
			TreeUtils::dump(gdalMetadata, std::cerr);
		}
		setField(TIFFTAG_GDAL_METADATA, sstr.str());

		GTIFWriteKeys(gtif);
	}
	else {
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.error("File not open");
	}
}

void FileGeoTIFF::close(){
	if (isOpen()){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.debug("Closing GeoTIFF...");
		//GTIFWriteKeys(gtif); // moved to writeMetadata() for cloud optimized GeoTIFF, COG.


		enum {VERSION=0,MAJOR,MINOR};
		int version[3] = {0,0,0};
		int keycount = 0;
		GTIFDirectoryInfo(gtif, version, &keycount);
		mout.info("GTIFDirectoryInfo: version:", version[VERSION], '.', version[MAJOR], '.', version[MINOR], " keycount:", keycount);
		GTIFFree(gtif);
		gtif = nullptr;
	}
	FileTIFF::close(); // ?
}


void FileGeoTIFF::setGdalScale(double scale, double offset){
	// TODO: separate code without nodata marker?
	// void FileGeoTIFF::setGdalMetaData(double nodata, double scale, double offset){
	// drain::Logger mout(__FILE__, __FUNCTION__);
	setGdal("SCALE", scale, 0);
	setGdal("OFFSET", offset, 0);
	// gdalMetadata["SCALE"]->setGDAL(scale, 0, "scale");
	// gdalMetadata["OFFSET"]->setGDAL(offset, 0, "offset");
}

void FileGeoTIFF::setGdalNoData(const std::string & nodata){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (nodata.empty()){
		return;
	}

	static
	const TIFFFieldInfo xtiffFieldInfo[] = {
			{TIFFTAG_GDAL_NODATA,    1,  1, TIFF_ASCII,  FIELD_CUSTOM, true, 0, const_cast<char*>("nodata-marker") },
	};
	TIFFMergeFieldInfo(tif, xtiffFieldInfo, 1);

	// std::string nodata = prop["what:nodata"];
	// http://stackoverflow.com/questions/24059421/adding-custom-tags-to-a-tiff-file
	mout.info("registering what:nodata => nodata=", nodata);
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

	// mout.special("NOW");

	// Image coords
	drain::Point2D<int> imageCoord;

	// Geographical coords (degrees or meters)
	drain::Point2D<double> mapCoord;

	imageCoord.setLocation(0, -1); //int(height));
	//frame.pix2m(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
	frame.pix2LLm(imageCoord.x, imageCoord.y, mapCoord.x, mapCoord.y);
	imageCoord.setLocation(0, 0);
	setProjection(frame.projGeo2Native); // GeoTIFF, FIXED 2023/02

	/*
	//if (frame.isLongLat()){
	if (false){
		//imagePos.setLocation(width/2, height/2);
		imageCoord.setLocation(frame.getFrameWidth()/2, frame.getFrameHeight()/2); // TODO: CHECK
		frame.pix2LLdeg(imageCoord.x, imageCoord.y, mapCoord.x, mapCoord.y);
		setProjectionLongLat(); // GeoTIFF, FIXED 2023/02
	}
	else { // metric
		imageCoord.setLocation(0, -1); //int(height));
		//frame.pix2m(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
		frame.pix2LLm(imageCoord.x, imageCoord.y, mapCoord.x, mapCoord.y);
		imageCoord.setLocation(0, 0);
		setProjection(frame.projGeo2Native); // GeoTIFF, FIXED 2023/02
	}
	*/

	std::vector<double> tiepoints(6);
	tiepoints[0] = static_cast<double>(imageCoord.x);
	tiepoints[1] = static_cast<double>(imageCoord.y);
	tiepoints[2] = 0.0;
	tiepoints[3] = mapCoord.x;
	tiepoints[4] = mapCoord.y;
	tiepoints[5] = 0.0;
	mout.debug("Tiepoint: ", imageCoord, " => ", mapCoord);
	setField(TIFFTAG_GEOTIEPOINTS, tiepoints);

	std::vector<double> pixscale(3);

	const drain::Rectangle<double> & bbox = frame.getBoundingBoxM();
	//const drain::Rectangle<double> & bbox = frame.isLongLat() ? frame.getBoundingBoxD() : frame.getBoundingBoxM();
	// mout.attention("Bbox: ", bbox, " Nat:", frame.getBoundingBoxM());

	mout.debug("Scale: ", frame.getXScale(), ", ", frame.getYScale());

	pixscale[0] = bbox.getWidth()  / static_cast<double>(frame.getFrameWidth()); // upperRight.x - bbox.lowerLeft.x
	pixscale[1] = bbox.getHeight() / static_cast<double>(frame.getFrameHeight()); // bbox.upperRight.y - bbox.lowerLeft.y
	pixscale[2] = 0.0;

	mout.debug("BBox: ", bbox);
	mout.debug("ScaleX: ", pixscale[0], ' ', bbox.getWidth(), ' ', frame.getFrameWidth()); // << ' ' << width
	mout.debug("ScaleY: ", pixscale[1]);
	// printf("$(( %.10f - %.10f )) = %.10f", bbox.upperRight.x, bbox.lowerLeft.x, bbox.getWidth());
	// mout.debug("Noh: " , (static_cast<double>(1280000) / static_cast<double>(1280)) );
	// mout.debug("Noh: " , (static_cast<double>(bbox.getWidth()) / static_cast<double>(frame.getFrameWidth())) );

	setField(TIFFTAG_GEOPIXELSCALE, pixscale);

}


void FileGeoTIFF::setProjection(const std::string & projstr){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!projstr.empty()){
		mout.info("projstr= ", projstr);
		drain::Proj6 proj;
		proj.setProjectionDst(projstr);
		setProjection(proj);
	}
	else {
		mout.warn("projstr empty: skipping GeoTIFF and writing plain TIFF)");
	}
}


void FileGeoTIFF::setProjection(const drain::Proj6 & proj){

	drain::Logger mout(__FILE__, __FUNCTION__);

	setGeoTiffField(GTRasterTypeGeoKey, RasterPixelIsArea);
	// GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT,  1, RasterPixelIsArea); // 2023/03 moved here
	//mout.attention(proj);

	if (proj.isLongLat()){
		mout.info("Writing EPSG:4326 (longlat)");
		//SetUpGeoKeys_4326_LongLat(gtif);
		//setProjectionLongLat();
		setProjectionEPSG(4326);
	}
	else {

		const std::string & dstProj = proj.getProjectionDst();
		//const std::string dstProj = "+init=epsg:3067";

		mout.info("Writing metric projection: ", dstProj);

		//const short epsg = drain::Proj6::pickEpsgCode(dstProj);
		const short epsg = proj.getDst().getEPSG(); //rain::Proj6::extractEPSG(dstProj);
		mout.debug("Read EPSG:", epsg);

		if (epsg > 0){
			mout.info("Using EPSG:", epsg, " 'directly'");
			setProjectionEPSG(epsg);
		}
		else if (GTIFSetFromProj4(gtif, dstProj.c_str())){ // CHECK!!
			mout.ok("GTIFSetFromProj4: ", dstProj);
		}
		else {
			mout.warn("Failed in setting GeoTIFF projection: ", dstProj);
			//mout.advice("Consider EPSG codes for projections in GeoTIFF, eg. PROJ.4 string '+init=epsg:3035'");
			mout.advice("For GeoTIFF outputs, try setting EPSG codes '--cProj 3035' instead of of PROJ4 strings.");
			mout.advice("Consider: gdal_translate -a_srs '", dstProj, "' file.tif out.tif");
			if (FileGeoTIFF::compliancyFlagger.isSet(FileGeoTIFF::STRICT)){
				mout.error("GeoTIFF error under strict compliance (requested by user)");
			}
		}
	}

	//GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT,  1, RasterPixelIsArea); // Repeated? Also in setProjectionLongLat?
	//GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT,  1, RasterPixelIsPoint);
}

void FileGeoTIFF::setProjectionEPSG(short epsg){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (epsg == 4326){
		setProjectionLongLat();
		return;
	}

	//  GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, epsg);
	setGeoTiffField(ProjectedCSTypeGeoKey, epsg);
	// =GTIFKeySet(gtif, tag, TYPE_SHORT, 1, static_cast<short int>(value));

	if (FileGeoTIFF::compliancyFlagger.isSet(FileGeoTIFF::EPSG)){

		mout.special("Applying extended configuration for EPSG:", epsg);

		setGeoTiffField(GTModelTypeGeoKey, ModelTypeProjected);
		setGeoTiffField(GeogAngularUnitsGeoKey, Angular_Degree);
		setGeoTiffField(ProjLinearUnitsGeoKey, Linear_Meter);

		switch (epsg) {
		case 3035:
			setGeoTiffField(GTCitationGeoKey,   "ETRS89-extended / LAEA Europe");
			setGeoTiffField(GeogCitationGeoKey, "ETRS89");
			break;
		case 2393:
			setGeoTiffField(GTCitationGeoKey,   "KKJ / Finland Uniform Coordinate System");
			setGeoTiffField(GeogCitationGeoKey, "KKJ");
			break;
		case 3067:
			setGeoTiffField(GTCitationGeoKey,   "ETRS89 / TM35FIN(E,N)");
			setGeoTiffField(GeogCitationGeoKey, "ETRS89");
			break;
		case 3844:
			setGeoTiffField(GTCitationGeoKey,   "Pulkovo 1942(58) / Stereo70");
			setGeoTiffField(GeogCitationGeoKey, "Pulkovo 1942(58)");
			break;
		case 3995:
			setGeoTiffField(GTCitationGeoKey,   "WGS 84 / Arctic Polar Stereographic");
			setGeoTiffField(GeogCitationGeoKey, "WGS 84");
			break;
		case 5125:
			setGeoTiffField(GTCitationGeoKey,   "ETRS89 / NTM zone 25");
			setGeoTiffField(GeogCitationGeoKey, "ETRS89");
			break;
		default:
			/*
			setGeoTiffField(GTCitationGeoKey,   "");
			setGeoTiffField(GeogCitationGeoKey, "");
			*/
			mout.info("No extended configuration found for EPSG:", epsg);
		}

	}


}


void FileGeoTIFF::setProjectionLongLat(){

	drain::Logger mout(__FILE__, __FUNCTION__);

	/* compare with:
	setGeoTiffField(GTModelTypeGeoKey, ModelTypeProjected);
	setGeoTiffField(GeogAngularUnitsGeoKey, Angular_Degree);
	setGeoTiffField(ProjLinearUnitsGeoKey, Linear_Meter);
	setGeoTiffField(GTCitationGeoKey,   "ETRS89-extended / LAEA Europe");
	setGeoTiffField(GeogCitationGeoKey, "ETRS89");
	*/

	mout.info("Setting Long-Lat projection");
	// GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, 4326);
	// setGeoTiffField(ProjectedCSTypeGeoKey,   4326); // ?
	// setGeoTiffField(GTRasterTypeGeoKey,      RasterPixelIsArea);
	setGeoTiffField(GTModelTypeGeoKey,       ModelTypeGeographic);
	setGeoTiffField(GeographicTypeGeoKey,    GCS_WGS_84);
	setGeoTiffField(GeogCitationGeoKey,      "WGS 84");
	setGeoTiffField(GeogAngularUnitsGeoKey,  Angular_Degree);
	/* GeogSemiMajorAxisGeoKey (Double,1): 6378137
	 * GeogInvFlatteningGeoKey (Double,1): 298.257223563
	 *
	*/
	setGeoTiffField(GeogSemiMajorAxisGeoKey, 6378137.0);
	setGeoTiffField(GeogInvFlatteningGeoKey, 298.257223563);

	/*
	GTIFKeySet(gtif, GTModelTypeGeoKey,       TYPE_SHORT,  1, ModelGeographic);
	GTIFKeySet(gtif, GTRasterTypeGeoKey,      TYPE_SHORT,  1, RasterPixelIsArea); // Also in main function
	//GTIFKeySet(gtif, GTRasterTypeGeoKey,      TYPE_SHORT,  1, RasterPixelIsPoint);

	// GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT,  1, GCSE_WGS84); <= WRONG! GCS_WGS_84
	GTIFKeySet(gtif, GeographicTypeGeoKey,    TYPE_SHORT,  1, GCS_WGS_84); // 4326 correct!
	GTIFKeySet(gtif, GeogCitationGeoKey,      TYPE_ASCII,  7, "WGS 84");
	GTIFKeySet(gtif, GeogAngularUnitsGeoKey,  TYPE_SHORT,  1, Angular_Degree);
	//GTIFKeySet(gtif, GeogSemiMajorAxisGeoKey, TYPE_DOUBLE, 1, 6378137.0);  //6377298.556);
	//GTIFKeySet(gtif, GeogInvFlatteningGeoKey, TYPE_DOUBLE, 1, 298.257223563);// 300.8017);
	 */
}


} // image::



} // drain::

#endif
