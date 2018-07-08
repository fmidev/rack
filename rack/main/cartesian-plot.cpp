/**


    Copyright 2006 - 2010   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

   Created on: Sep 30, 2010
 */



#include <drain/util/Fuzzy.h>


#include <drain/image/File.h>
//#include <drain/imageops/DistanceTransformFillOp.h>
#include <drain/imageops/RecursiveRepairerOp.h>

#include "data/DataCoder.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"


#include "cartesian-plot.h"


namespace rack {


void CartesianPlotFile::exec() const {

	drain::Logger mout(name, __FUNCTION__); // = getResources().mout; = getResources().mout;

	RackResources & resources = getResources();

	Composite & composite = resources.composite;

	composite.allocate();
	//composite.updateGeoData();

	if (! composite.isMethodSet()){
		composite.setMethod("LATEST");
		mout.note() << " no method set, using " << composite.getMethod() << " (see --cMethod) " << mout.endl;
	}

	const bool STDIN = (value.length()==1) && (value.at(0)=='-');

	std::ifstream ifstr;
	if (!STDIN){
		ifstr.open(value.c_str());
		if (! ifstr){
			mout.warn() << " plotfile: opening file failed: " << value << mout.endl; // or eof?
			//inpu tOk = false;
			ifstr.close();
			return;
		}

	}

	std::istream &istr = STDIN ? std::cin : ifstr; // std::cin;

	std::string line;
	double lat;
	double lon;
	double d;
	double w;

	std::stringstream sstr;

	while ( getline(istr, line) ){

		line = line.substr(0, line.find_first_of("%#"));
		if (!line.empty()){
			// if ((line.at(0) != '%')&&(line.at(0) != '#')){
			sstr.clear();
			sstr.str(line);
			sstr >> lon >> lat;
			sstr >> d;
			if (!sstr.eof())
				sstr >> w;
			else
				w = 1.0;
			//std::cout << '#' << line << '\n';
			// std::cout << lon << ',' << lat << '\t' << d << ',' << w << '\n';
			composite.addUnprojected(lon, lat, d, w);
			//std::cout << i << ' ' << x << '\t' << y << '\n';
			//}
		}
	}

	ifstr.close();



}


void CartesianSpread::exec() const {  // TODO iDistanceFill

	drain::Logger mout(name, __FUNCTION__); // = getResources().mout;

	RackResources & resources = getResources();

	DataSet<CartesianDst> dstDataSet((*resources.currentHi5)["dataset1"]);

	Data<CartesianDst> & dst = dstDataSet.getFirstData(); // first data encountered; consider "DBZH"?

	if (dst.data.isEmpty()){
		mout.warn() << "Empty data, skipping..." << mout.endl;
		return;
	}

	//mout.warn() << dst.odim << mout.endl;

	/*
	DistanceTransformFillLinearOp distLinear;
	DistanceTransformFillExponentialOp distExp; // would need float-valued tmp image

	//DistanceTransformFillOpBase  & dist = (key == "cSpread") ? distLinear : distExp;
	DistanceTransformFillLinearOp  & dist = distLinear;
	*/

	RecursiveRepairerOp recOp;  // slow, smooth
	recOp.width = 5;
	recOp.height = 5;
	recOp.loops = loops;
	//recOp.decay = decay;

	/*
	double h = this->horz*1000.0/dst.odim.xscale;
	double v = (this->vert > 0.0) ? this->vert*1000.0/dst.odim.yscale : h;
	dist.setRadius(h, v);
	*/

	mout.debug(2) << name << ": pixel resolution: " << dst.odim.xscale << ',' << dst.odim.yscale << mout.endl;
	//mout.debug(2) << dist << mout.endl;



	PlainData<CartesianDst> & dstQuality = dstDataSet.getQualityData();
	// OR local?? ::
	// PlainData<CartesianDst> & dstQuality = dst.getQuality();

	mout.debug(2) << "data: "    << dst << mout.endl;
	mout.debug(2) << "quality: " << dstQuality << mout.endl;

	if (dstQuality.data.isEmpty()){
		mout.warn() << "Empty quality data, skipping..." << mout.endl;
		return;
	}

	/*
	drain::image::Image tmpWeight;
	tmpWeight.setGeometry(dstQuality.data.getGeometry());
	tmpWeight.fill(0);

	//drain::image::File::write(dst.data,        "dstSpread-d0.png");
	//drain::image::File::write(dstQuality.data, "dstSpread-q0.png");
	//dist.filter(dst.data, dstQuality.data, dst.data, tmpWeight);
	ImageTray<const Channel> srcImage(dst.data);
	srcImage.appendAlpha(dstQuality.data);
	ImageTray<Channel> dstImage(dst.data);
	dstImage.appendAlpha(tmpWeight);
	dist.traverseChannels(srcImage, dstImage);
	*/
	//dist.process(dst.data, dstQuality.data, dst.data, tmpWeight);
	// drain::image::File::write(tmpWeight, "dstSpread-q1.png");
	// drain::image::File::write(dst.data,  "dstSpread.png");

	/*
	ImageTray<Channel> dstChannels(dst.data, dstQuality.data);
	const ImageTray<const Channel> srcChannels(dst.data, dstQuality.data);
	recOp.traverseChannels(srcChannels, dstChannels);
	*/
	recOp.traverseChannel(dst.data.getChannel(0), dstQuality.data.getChannel(0), dst.data.getChannel(0), dstQuality.data.getChannel(0));
	/*
	if (recOp.loops > 0){
		mout.debug(2) << recOp << mout.endl;
		recOp.filter(dst.data, dstQuality.data, dst.data, dstQuality.data);
	}
	else {
		mout.debug(2) << "copying..." << mout.endl;
		CopyOp().filter(tmpWeight, dstQuality.data); // ??
	}
	*/

	//drain::image::File::write(dst.data,        "dstSpread-d2.png");
	//drain::image::File::write(dstQuality.data, "dstSpread-q2.png");

	resources.currentImage     = & dst.data;
	resources.currentGrayImage = & dst.data;

}



}  // namespace rack::


