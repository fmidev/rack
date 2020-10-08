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

#include "drain/util/Log.h"
#include "drain/util/Time.h"
#include "drain/util/TreeXML.h"
#include "drain/image/AccumulatorGeo.h"
#include "drain/image/File.h"

#include "main/rack.h"

#include "FileGeoTIFF.h"


int rack::FileGeoTIFF::tileWidth(256);
int rack::FileGeoTIFF::tileHeight(256);


#ifdef GEOTIFF_NO

int rack::FileGeoTIFF::compression(1); // = NONE, but see below

#else

#include <proj_api.h>


#include <geotiff.h>
#include <geotiffio.h>
#include <xtiffio.h>
#include <geo_normalize.h>

// https://en.wikipedia.org/wiki/TIFF
// https://www.awaresystems.be/imaging/tiff/tifftags/gdal_nodata.html
#ifndef TIFFTAG_GDAL_METADATA //# ASCII tag (code 42113
#define TIFFTAG_GDAL_METADATA 42112 // 0xa481 // 42113
#endif

#ifndef TIFFTAG_GDAL_NODATA //# ASCII tag (code 42113
#define TIFFTAG_GDAL_NODATA 42113 // 0xa481 // 42113
#endif

namespace rack
{


int FileGeoTIFF::compression(COMPRESSION_LZW); // = tunable in fileio.cpp


const drain::Dictionary2<int, std::string> & FileGeoTIFF::getCompressionDict(){

	if (compressionDict.empty()){
		// Populate
		compressionDict.add(COMPRESSION_NONE,     "NONE");
		compressionDict.add(COMPRESSION_LZW,      "LZW");
		compressionDict.add(COMPRESSION_DEFLATE,  "DEFLATE");
		compressionDict.add(COMPRESSION_PACKBITS, "PACKBITS");
	}

	return compressionDict;
}


drain::Dictionary2<int, std::string> FileGeoTIFF::compressionDict;


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


typedef drain::Tree<std::string,NodeGDAL> TreeGDAL;

inline
std::ostream & operator<<(std::ostream &ostr, const TreeGDAL & tree){
	return drain::NodeXML::toOStr(ostr, tree);
}


//drain::Variable FileGeoTIFF::ties(typeid(double));


void SetUpTIFFDirectory(TIFF *tif, const drain::image::Image & src, int tileWidth=0, int tileHeight = 0){

	drain::Logger mout("FileGeoTIFF", __FUNCTION__);

	const drain::FlexVariableMap & prop = src.properties;

	const size_t width  = src.getWidth();
	const size_t height = src.getHeight();

	TIFFSetField(tif,TIFFTAG_IMAGEWIDTH,    width);
	TIFFSetField(tif,TIFFTAG_IMAGELENGTH,   height);
	//TIFFSetField(tif,TIFFTAG_COMPRESSION,   COMPRESSION_NONE);
	TIFFSetField(tif,TIFFTAG_COMPRESSION,   FileGeoTIFF::compression);
	TIFFSetField(tif,TIFFTAG_PHOTOMETRIC,   PHOTOMETRIC_MINISBLACK);
	TIFFSetField(tif,TIFFTAG_PLANARCONFIG,  PLANARCONFIG_CONTIG);

	const drain::Type t(src.getType());
	mout.debug() << " bytes=" << drain::Type::call<drain::sizeGetter>(t) << mout.endl;
	switch ((const char)t) {
		case 'C':
			// no break
		case 'S':
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8*drain::Type::call<drain::sizeGetter>(t));
			break;
		default:
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
			mout.warn() << "unsupported storage type=" <<  drain::Type::getTypeChar(t) << ", trying 8 bit mode"<< mout.endl;
	}

	// write as tiles
	if (tileWidth > 0){

		if (tileHeight <= 0)
			tileHeight = tileWidth;

		if (TIFFSetField(tif, TIFFTAG_TILEWIDTH,  tileWidth)==0){
			mout.warn() << "invalid tileWidth=" << tileWidth << ", using 256"<< mout.endl;
			TIFFSetField(tif, TIFFTAG_TILEWIDTH, 256);
		}

		if (TIFFSetField(tif,TIFFTAG_TILELENGTH,  tileHeight)==0){
			mout.warn() << "invalid tileWidth=" << tileHeight << ", using 256"<< mout.endl;
			TIFFSetField(tif, TIFFTAG_TILELENGTH, 256);
		}

	}
	else {
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,  20L);
	}

	// http://www.gdal.org/frmt_gtiff.html
	// Optional
	const std::string software = std::string(__RACK__) + " " + std::string(__RACK_VERSION__);
	TIFFSetField(tif,TIFFTAG_SOFTWARE, software.c_str());


	drain::Time datetime;
	datetime.setTime(prop.get("what:date", "19700101"), "%Y%m%d");
	datetime.setTime(prop.get("what:time", "000000"), "%H%M%S");

	TIFFSetField(tif, TIFFTAG_DATETIME, datetime.str("%Y:%m:%d %H:%M:%S").c_str() );
	// const std::string datetime = prop.get("what:date", "") + prop.get("what:time", "");
	//TIFFSetField(tif, TIFFTAG_DATETIME, datetime.c_str() );


	const std::string desc = prop.get("what:object", "") + ":"+ prop.get("what:product", "") + ":" + prop.get("what:prodpar", "") + ":" + prop.get("what:quantity", "");
	TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, desc.c_str());


	// GDALMetadata etc
	// usr/include/gdal/rawdataset.h
	// Non-standard http://www.gdal.org/frmt_gtiff.html

	//const int TIFFTAG_KOE = 65001;
	// { TIFFTAG_KOE,           -1, -1, TIFF_DOUBLE, FIELD_CUSTOM, true, 0, const_cast<char*>("koe") },
	// "gdal-metadata"
	static const TIFFFieldInfo xtiffFieldInfo[] = {
			{ TIFFTAG_GDAL_METADATA, -1, -1, TIFF_ASCII,  FIELD_CUSTOM, true, 0, const_cast<char*>("GDAL_METADATA") },
			{ TIFFTAG_GDAL_NODATA,    1,  1, TIFF_ASCII,  FIELD_CUSTOM, true, 0, const_cast<char*>("nodata-marker") },
	};
	TIFFMergeFieldInfo(tif, xtiffFieldInfo, 2);

	TreeGDAL gdalInfo;
	gdalInfo["SCALE"]->set(prop.get("what:gain", 1.0),    0, "scale");
	gdalInfo["OFFSET"]->set(prop.get("what:offset", 0.0), 0, "offset");
	/*
	<GDALMetadata >
	<Item name="OFFSET" role="offset" sample="0" >-32</Item>
	<Item name="SCALE" role="scale" sample="0" >0.5</Item>
	</GDALMetadata>
	*/

	std::stringstream gdal;
	gdal << gdalInfo;
	mout.debug() << gdal.str() << mout.endl;
	TIFFSetField(tif, TIFFTAG_GDAL_METADATA, gdal.str().c_str());

	std::string nodata = prop["what:nodata"];
	if (!nodata.empty()){
		// http://stackoverflow.com/questions/24059421/adding-custom-tags-to-a-tiff-file
		mout.info() << "registering what:nodata => nodata=" << nodata << mout.endl;
		TIFFSetField(tif, TIFFTAG_GDAL_NODATA, nodata.c_str());
	}





	/// Projection
	std::string projdef = prop["where:projdef"];
	if (projdef.empty()){
		mout.note() << "where:projdef missing, no GeoTIFF projection info written" << mout.endl;
		return;
	}



	drain::image::GeoFrame frame;
	frame.setGeometry(width, height);
	frame.setProjection(projdef);
	const drain::Rectangle<double> bboxD(prop["where:LL_lon"], prop["where:LL_lat"], prop["where:UR_lon"], prop["where:UR_lat"]);
	if (frame.isLongLat()){
		frame.setBoundingBoxD(bboxD);
	}
	else {
		const drain::Variable & p = prop["where:BBOX_native"];
		std::vector<double> v;
		p.toContainer(v);
		if (v.size() == 4){
			frame.setBoundingBoxM(v[0], v[1], v[2], v[3]);
			mout.note() << "Setting exact (metric) BBOX=";
			char sep = ' ';
			v = frame.getBoundingBoxM().toVector(); // Back!
			char buffer[256];
			for (size_t i=0; i<v.size(); ++i){
				mout << sep;
				sep = ',';
				snprintf(buffer, sizeof(buffer), "%.2f", v[i]);
				mout << buffer;
			}
			mout << mout.endl;
		}
		else {
			mout.warn() << "where:BBOX_native (" << p << ") missing or invalid, using bbox in degrees (approximative)" << mout.endl;
			frame.setBoundingBoxD(bboxD);
		}
	}

	//frame.getBoundingBoxM().toStream();

	double tiepoints[6]; // = {0,0,0,0,0,0};

	// Image coords
	// const int i = width/2;
	// const int j = height/2;
	drain::Point2D<int>    imagePos;

	// Geographical coords (degrees or meters)
	drain::Point2D<double>   geoPos;
	//double x, y;

	if (frame.isLongLat()){
		imagePos.setLocation(width/2, height/2);
		frame.pix2LLdeg(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
	}
	else { // metric
		imagePos.setLocation(0, 0); //int(height));
		frame.pix2m(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
		//frame.pix2m(imagePos.x, imagePos.y, geoPos.x, geoPos.y);
	}

	tiepoints[0] = static_cast<double>(imagePos.x);
	tiepoints[1] = static_cast<double>(imagePos.y);
	tiepoints[2] = 0.0;
	tiepoints[3] = geoPos.x;
	tiepoints[4] = geoPos.y;
	tiepoints[5] = 0.0;
	mout.debug() << "Tiepoint: " << imagePos << " => " << geoPos << mout.endl;

	TIFFSetField(tif,TIFFTAG_GEOTIEPOINTS, 6,tiepoints);

	double pixscale[3]; // = {1,1,0};
	//std::cerr << "frame: " << frame.getProjection() << '\n';
	const drain::Rectangle<double> & bbox = frame.isLongLat() ? bboxD : frame.getBoundingBoxM();
	//frame.getXScale()?
	mout.debug() << "Scale: " << frame.getXScale() << ", " << frame.getYScale() << mout.endl;

	pixscale[0] = bbox.getWidth()   / static_cast<double>(frame.getFrameWidth()); // upperRight.x - bbox.lowerLeft.x
	pixscale[1] = bbox.getHeight() / static_cast<double>(frame.getFrameHeight()); // bbox.upperRight.y - bbox.lowerLeft.y
	pixscale[2] = 0.0;

	mout.debug() << "BBox: " << bbox << mout.endl;
	mout.debug() << "ScaleX: " << pixscale[0] << ' ' << bbox.getWidth() << ' ' << frame.getFrameWidth() << ' ' << width << mout.endl;
	mout.debug() << "ScaleY: " << pixscale[1] << mout.endl;

	//printf("$(( %.10f - %.10f )) = %.10f", bbox.upperRight.x, bbox.lowerLeft.x, bbox.getWidth());

	// mout.debug() << "Noh: " << (static_cast<double>(1280000) / static_cast<double>(1280)) << mout.endl;
	// mout.debug() << "Noh: " << (static_cast<double>(bbox.getWidth()) / static_cast<double>(frame.getFrameWidth())) << mout.endl;

	TIFFSetField(tif,TIFFTAG_GEOPIXELSCALE, 3, pixscale);


}

void SetUpGeoKeys_4326_LongLat(GTIF *gtif){
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


void WriteImage(TIFF *tif, const drain::image::Image & src) //, int tileWidth = 0, int tileHeight = 0)
{

	drain::Logger mout("FileGeoTIFF", __FUNCTION__);

	const int width  = src.getWidth();
	const int height = src.getHeight();

	unsigned char *buffer = NULL;
	//unsigned short int *buffer16b = NULL;

	int tileWidth = 0;
	int tileHeight = 0;
	int bitspersample = 8;

	TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &tileWidth);
	TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileHeight);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);

	if (tileWidth > 0){

		if (tileHeight == 0)
			tileHeight = tileWidth;

		const int W = (width/tileWidth);
		const int H = (height/tileHeight);

		const bool UCHAR8 = (bitspersample==8); // ? 1 : 2; // 8 or 16 bit data

		drain::image::Image tile;
		if (UCHAR8)
			tile.setType<unsigned char>();
		else
			tile.setType<unsigned short int>();
		tile.setGeometry(tileWidth, tileHeight);

		mout.info() << "tiled mode:"  << tile << ", bits=" << bitspersample << mout.endl;

		if ((!UCHAR8) && (width % tileWidth)){
			mout.warn() << "16bit image, width != N*tileWidth (" << tileWidth <<"), errors may occur (libgeotiff problem?)" << mout.endl;
		}

		/// current tile-widths
		int w;
		int h;
		const int wPartial = width  % tileWidth;
		const int hPartial = height % tileHeight;
		int iOffset;
		int jOffset;
		for (int l=0; l<=H; ++l){

			jOffset = l*tileHeight;
			if (l<H)
				h = tileHeight;
			else
				h = hPartial;

			for (int k=0; k<=W; ++k){

				iOffset = k*tileWidth;
				if (k<W)
					w = tileWidth;
				else // last tile is partial
					w = wPartial;


				// Copy image data to tile
				if ((w>0) && (h>0)){
					//if (!UCHAR8){tile.setGeometry(w, h);}
					mout.debug(1) << "TILE:" << k << ',' <<  l << '\t' << w << 'x' << h << mout.endl;
					for (int j=0; j<h; ++j){
						for (int i=0; i<w; ++i){
							//buffer[j*tileWidth + i] = src.get<int>(iOffset+i, jOffset+j);
							tile.put(i,j, src.get<int>(iOffset+i, jOffset+j));
						}
					}
					/*
					if ((k==W) || (l==H)){
						std::stringstream s;
						s << "tile" << l << k << ".png";
						drain::image::File::write(tile, s.str());
					}
					*/
					if(!TIFFWriteTile(tif, tile.getBuffer(), iOffset, jOffset, 0, 0)){
						TIFFError("WriteImage", "TIFFWriteTile failed \n");
					}

					//if (!UCHAR8){tile.setGeometry(tileHeight, tileHeight);}

				}
			}
		}

	}
	else {

		const drain::Type t(src.getType());
		if ((t == 'C') || (t=='S')){
			/// Address each ŕow directly
			const int rowBytes = width * src.getEncoding().getElementSize();
			buffer = (unsigned char *)src.getBuffer();
			for (int j=0; j<height; ++j){
				if (!TIFFWriteScanline(tif, &(buffer[j * rowBytes]), j, 0))
					TIFFError(__FUNCTION__, "failure in direct WriteScanline\n");
			}
		}
		else {
			/// Copy each ŕow to buffer
			buffer = new unsigned char[width*1]; // 8 bits
			for (int j=0; j<height; ++j){
				for (int i=0; i<width; ++i)
					buffer[i] = src.get<int>(i,j);
				if (!TIFFWriteScanline(tif, buffer, j, 0))
					TIFFError(__FUNCTION__, "failure in buffered WriteScanline\n");
			}
			delete buffer;
		}

	}


}




/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 */
//void FileGeoTIFF::write(const std::string &filePath, const Hi5Tree & src, const ODIMPathList & paths){
void FileGeoTIFF::write(const std::string &path, const drain::image::Image & src, int tileWidth, int tileHeight){

	drain::Logger mout("FileGeoTIFF", __FUNCTION__);
	//mout.note() << src.properties << mout.endl;

	/// Open TIFF file for writing
	TIFF *tif = XTIFFOpen(path.c_str(), "w");
	if (tif){

		GTIF *gtif = GTIFNew(tif);
		if (gtif){

			//int tileSize = 256;

			//const drain::Type t(src.getType());
			//SetUpTIFFDirectory(tif, src.getWidth(), src.getHeight());
			SetUpTIFFDirectory(tif, src, tileWidth, tileHeight);
			WriteImage(tif, src); //, tileSize, tileSize/2);

			//mout.note() << src.properties << mout.endl;
			std::string projstr = src.properties["where:projdef"];

			if (!projstr.empty()){
				mout.info() << "where:projdef= " << projstr << mout.endl;
			}
			else
				mout.warn() << "where:projdef empty" << mout.endl;

			drain::Proj4 proj;
			proj.setProjectionDst(projstr);
			if (proj.isLongLat()){
				mout.info() << "writing 4326 longlat" << mout.endl;
				SetUpGeoKeys_4326_LongLat(gtif);
			}
			else {
				mout.info() << "writing metric projection" << mout.endl;
				//GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT,  1, GCSE_WGS84);
				//int projOK = GTIFSetFromProj4(gtif, projstr.c_str());
				if (!GTIFSetFromProj4(gtif, projstr.c_str())){
					mout.warn() << "failed in setting GeoTIFF projection, where:projdef='" << projstr << "'" << mout.endl;
					mout.note() << "consider: gdal_translate -a_srs '" << projstr << "' " << path << ' ' << path << 'f' << mout.endl;
				}
			}

			// NEW
			GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT,  1, RasterPixelIsArea);
			//GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT,  1, RasterPixelIsPoint);
			/*
			// usr/include/gdal/rawdataset.h
			// Non-standard http://www.gdal.org/frmt_gtiff.html
			std::string nodata = src.properties["what:nodata"];
			if (!nodata.empty()){
				mout.toOStr() << "registering what:nodata => nodata=" << nodata << mout.endl;
				GTIFKeySet(gtif, (geokey_t)TIFFTAG_GDAL_NODATA, TYPE_ASCII, nodata.length()+1, nodata.c_str());  // yes, ascii
			}
			 */
			GTIFWriteKeys(gtif);

			GTIFFree(gtif);

		}
		else {
			mout.error() << "failed creating GeoTIFF file from TIFF object, path=" << path << mout.endl;
		}

		XTIFFClose(tif);

	}
	else {
		mout.error() << "file open error, path=" << path << mout.endl;
	}

	return ; //-1;

}

}

#endif


// Rack
 // REP
