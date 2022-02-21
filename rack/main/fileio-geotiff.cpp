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

//#include <exception>
#include <drain/image/ImageFile.h>
#include <fstream>
#include <iostream>
/*
#include <algorithm>
#include <limits>
#include <list>
#include <map>
#include <sstream>
#include <utility>
#include <vector>
 */
//#include <regex.h>
// #include <stddef.h>

#include "drain/util/Log.h"
#include "drain/util/StringBuilder.h"
/*
#include "drain/util/FilePath.h"
#include "drain/util/Output.h"
#include "drain/util/StringMapper.h"
#include "drain/util/Tree.h"
#include "drain/util/Variable.h"
#include "drain/image/FilePng.h"
#include "drain/image/FilePnm.h"
*/
//#include "drain/image/FileGeoTIFF.h"
//#include "radar/FileGeoTIFF.h"

//#include "drain/image/Image.h"
//#include "drain/image/Sampler.h"
//#include "drain/imageops/ImageModifierPack.h"

//#include "drain/prog/Command.h"
/*
#include "drain/prog/CommandBankUtils.h"
#include "drain/prog/CommandInstaller.h"

#include "data/Data.h"
//#include "data/DataOutput.h"
#include "data/DataSelector.h"
#include "data/DataTools.h"
#include "data/ODIMPath.h"
#include "data/PolarODIM.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"
#include "product/ProductOp.h"
//#include "radar/RadarDataPicker.h"
*/
//#include "resources.h"
#include "fileio-geotiff.h"

namespace rack {

const drain::image::FileTIFF::dict_t & CmdGeoTiff::compressionDict(drain::image::FileTIFF::getCompressionDict());


void CmdGeoTiff::exec() const {
	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	/// TODO. develop FileTIFF::dict_t as Single-Flagger etc

	drain::image::FileTIFF::dict_t::value_t value = drain::FlagResolver::getValue(compressionDict, compression, '\0');
	mout.special("Resolved: ", compression, " => ", value, " == ", drain::FlagResolver::getKeys(compressionDict, value));
	if (compressionDict.hasValue(value)){
		drain::image::FileTIFF::defaultCompression = value;
	}
	else {
		mout.fail("Multiple compression method: ", compression , " == ", value, " not supported");
		mout.warn("Keeping:  <= '", compressionDict.getKey(drain::image::FileTIFF::defaultCompression), "' (", drain::image::FileTIFF::defaultCompression, ')');
	}
}


void CmdGeoTiff::write(const drain::image::Image & src, const std::string & filename) {

	//void FileGeoTIFF::adjustGeoFrame_rack(const drain::image::Image & src, drain::image::GeoFrame & frame){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::image::FileGeoTIFF file(filename, "w");

	const drain::FlexVariableMap & prop = src.properties;

	drain::Time t;
	t.setTime(prop.get("what:date", "19700101"), "%Y%m%d");
	t.setTime(prop.get("what:time", "000000"), "%H%M%S");
	file.setTime(t);

	//const std::string desc = prop.get("what:object", "") + ":"+ prop.get("what:product", "") + ":" + prop.get("what:prodpar", "") + ":" + prop.get("what:quantity", "");
	const std::string desc = drain::StringBuilder(
			prop["what:object"],':',
			prop["what:product"],':',
			prop["what:prodpar"],':',
			prop["what:quantity"],':',
			prop["how:angles"]);
	//TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, desc.c_str());
	file.setField(TIFFTAG_IMAGEDESCRIPTION, desc);

	// http://www.gdal.org/frmt_gtiff.html
	// Optional
	file.setField(TIFFTAG_SOFTWARE,(const std::string &) drain::StringBuilder(__RACK__," ",__RACK_VERSION__));

	file.setDefaults();
	//file.setField(, value)
	// file.setField(TIFFTAG_COMPRESSION, FileTIFF::getCompressionDict().getKey(value));
	file.useDefaultTileSize();

	// GDALMetadata etc
	// usr/include/gdal/rawdataset.h
	// Non-standard http://www.gdal.org/frmt_gtiff.html

	file.setGdalMetaData(prop["what:nodata"], prop.get("what:gain", 1.0), prop.get("what:offset", 0.0));


	drain::image::GeoFrame frame;

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
			/*
			frame.setBoundingBoxD(bboxD);
			mout.special("BBOX deg: ", frame.getBoundingBoxD());
			mout.special() << "BBOX m  : ";
			mout.precision(20);
			mout << frame.getBoundingBoxM() << mout;
			*/

			const drain::Variable & p = prop["where:BBOX_native"];
			std::vector<double> v;
			p.toSequence(v);
			drain::Rectangle<double> bboxM;
			bboxM.fromSequence(v);
			if (v.size() == 4){
				//frame.setBoundingBoxM(v[0], v[1], v[2], v[3]);
				frame.setBoundingBoxM(bboxM);
				mout.special() << "Setting exact (metric) BBOX=";
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


	file.setGeoMetaData(frame);
	//file.setUpTIFFDirectory_rack(src); // <-- check if could be added finally
	file.writeImageData(src);

}




} // namespace rack
