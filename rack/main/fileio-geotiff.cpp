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

#include "drain/util/Log.h"
#include "drain/util/StringBuilder.h"
#include "main/rack.h"

#include "fileio-geotiff.h"

namespace rack {

const drain::image::FileTIFF::dict_t & CmdGeoTiff::compressionDict(drain::image::FileTIFF::getCompressionDict());


void CmdGeoTiff::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	mout.deprecating("Use --outputConf instead");

	/// TODO. develop FileTIFF::dict_t as Single-Flagger etc
	if (compression.empty()){

		const drain::image::FileTIFF::dict_t::value_t value = drain::FlagResolver::getValue(compressionDict, compression, '\0');
		mout.special("Resolved: '", compression, "' => ", value, " == ", drain::FlagResolver::getKeys(compressionDict, value));
		if (compressionDict.hasValue(value)){
			drain::image::FileTIFF::defaultCompression = value;
		}
		else {
			mout.fail("Multiple compression method: ", compression , " == ", value, " not supported, use: ", compressionDict);
			mout.warn("Keeping:  <= '", compressionDict.getKey(drain::image::FileTIFF::defaultCompression), "' (", drain::image::FileTIFF::defaultCompression, ')');
		}

	}

}


void CmdGeoTiff::write(const drain::image::Image & src, const std::string & filename) {

	//void FileGeoTIFF::adjustGeoFrame_rack(const drain::image::Image & src, drain::image::GeoFrame & frame){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::image::FileGeoTIFF file(filename, "w");

	CartesianODIM odim(src);
	src.properties["how:angles"].toSequence(odim.angles); // std::vector cannot be linked

	drain::Time t;
	odim.getTime(t);
	// t.setTime(prop.get("what:date", "19700101"), "%Y%m%d");
	// t.setTime(prop.get("what:time", "000000"), "%H%M%S");
	file.setTime(t);

	//mout.attention("orig ODIM angles: ",   drain::sprinter(odim.angles) );
	//mout.attention("src.properties map: ", src.properties);

	//const std::string desc = prop.get("what:object", "") + ":"+ prop.get("what:product", "") + ":" + prop.get("what:prodpar", "") + ":" + prop.get("what:quantity", "");
	//const std::string desc = drain::StringBuilder(
	std::stringstream desc;
	desc << odim.object << ':' << odim.quantity << ':' << odim.product;
	if (!odim.prodpar.empty()){
		desc << '(' << odim.prodpar << ')';
	}
	if (!src.properties["how:angles"].empty())
		desc << ':' << "elangles(" << src.properties["how:angles"] << ')';
	file.setField(TIFFTAG_IMAGEDESCRIPTION, desc.str());

	// http://www.gdal.org/frmt_gtiff.html
	// Optional
	file.setField(TIFFTAG_SOFTWARE,(const std::string &) drain::StringBuilder(__RACK__," ",__RACK_VERSION__));
	//file.setField(TIFFTAG_SOFTWARE, drain::StringBuilder(__RACK__," ",__RACK_VERSION__));

	file.setDefaults();
	file.useDefaultTileSize();

	// GDALMetadata etc
	// usr/include/gdal/rawdataset.h
	// Non-standard http://www.gdal.org/frmt_gtiff.html

	//file.setGdalMetaData(prop["what:nodata"], prop.get("what:gain", 1.0), prop.get("what:offset", 0.0));
	std::string nodata;
	drain::StringTools::import(odim.nodata, nodata);
	file.setGdalScale(odim.scaling.scale, odim.scaling.offset);
	file.setGdalNoData(nodata);

	drain::image::GeoFrame frame;

	if (!odim.projdef.empty()){

		frame.setGeometry(src.getWidth(), src.getHeight());
		frame.setProjection(odim.projdef);

		//drain::Rectangle<double> bboxD(prop["where:LL_lon"], prop["where:LL_lat"], prop["where:UR_lon"], prop["where:UR_lat"] );
		if (frame.isLongLat()){
			//frame.setBoundingBoxD(bboxD);
			frame.setBoundingBoxD(odim.getBoundingBoxD());
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


			const drain::Variable & p = src.properties["where:BBOX_native"];
			std::vector<double> v;
			p.toSequence(v);
			if (v.size() == 4){
				drain::Rectangle<double> bboxM;
				bboxM.assignSequence(v);
				//frame.setBoundingBoxM(v[0], v[1], v[2], v[3]);
				frame.setBoundingBoxM(bboxM);
				mout.note() << "Setting exact (metric) BBOX=";
				mout.precision(20);
				mout << frame.getBoundingBoxM() << mout;
			}
			else {
				mout.warn("where:BBOX_native (", p, ") missing or invalid, using bbox in degrees (approximative)");
				// frame.setBoundingBoxD(bboxD);
				frame.setBoundingBoxD(odim.getBoundingBoxD());
			}
		}
	}
	else {
		mout.note("where:projdef missing, cannot write GeoTIFF projection definitions");
	}

	// mout.attention("file.setGeoMetaData(frame)");
	file.setGeoMetaData(frame);

	// file.writeMetadata(); // Metadata first, for cloud optimized GeoTIFF, COG.
	// file.setUpTIFFDirectory_rack(src); // <-- check if could be added finally
	file.writeMetadata(); // Metadata first, for cloud optimized GeoTIFF, COG.
	file.writeImageData(src);

}




} // namespace rack
