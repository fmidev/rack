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



#include <drain/image/ImageFile.h>
#include "drain/util/Fuzzy.h"

#include "drain/util/Input.h"
#include "drain/imageops/BlenderOp.h"

#include "data/DataCoder.h"
#include "hi5/Hi5.h"

//#include "radar/Coordinates.h"
#include "radar/Composite.h"

#include "cartesian-plot.h"


namespace rack {


void CartesianPlotFile::exec() const {

	drain::Logger mout(__FILE__, __FUNCTION__); // = getResources().mout; = getResources().mout;

	RackContext & ctx = getContext<RackContext>();

	ctx.composite.dataSelector.consumeParameters(ctx.select);
	ctx.composite.allocate(); // check size?
	ctx.composite.consumeTargetEncoding(ctx.targetEncoding);

	if (! ctx.composite.isMethodSet()){
		ctx.composite.setMethod("LATEST");
		mout.note(" no method set, using " , ctx.composite.getMethod() , " (see --cMethod) " );
	}

	drain::Input input(value);

	std::string line;
	double lat;
	double lon;
	double d;
	double w;

	std::stringstream sstr;

	while ( getline((std::istream &)input, line) ){

		// line = line.substr(0, line.find_first_of("%#"));
		line = drain::StringTools::trim(line.substr(0, line.find_first_of("%#")));

		if (!line.empty()){

			sstr.clear();
			sstr.str(line);
			sstr >> lon >> lat;
			sstr >> d;
			if (!sstr.eof())
				sstr >> w;
			else
				w = 1.0;
			// std::cout << '#' << line << '\n';
			// std::cout << lon << ',' << lat << '\t' << d << ',' << w << '\n';
			ctx.composite.addUnprojected(lon, lat, d, w);

		}
	}

	//ifstr.close();

}

// OBSOLETE? iDistranceTransform/Fill Or develop...
void CartesianSpread::exec() const {  // TODO iDistanceFill

	drain::Logger mout(__FILE__, __FUNCTION__); // = getResources().mout;

	//RackResources & resources = getResources();
	RackContext & ctx = getContext<RackContext>();

	DataSet<CartesianDst> dstDataSet((*ctx.currentHi5)["dataset1"]);

	Data<CartesianDst> & dst = dstDataSet.getFirstData(); // first data encountered; consider "DBZH"?

	if (dst.data.isEmpty()){
		mout.warn("Empty data, skipping..." );
		return;
	}

	//mout.warn(dst.odim );

	/*
	DistanceTransformFillLinearOp distLinear;
	DistanceTransformFillExponentialOp distExp; // would need float-valued tmp image

	//DistanceTransformFillOpBase  & dist = (key == "cSpread") ? distLinear : distExp;
	DistanceTransformFillLinearOp  & dist = distLinear;
	*/

	//RecursiveRepairerOp recOp;  // slow, smooth

	// 5x5 window, block averaging, maximum-mixer
	//BlenderOp recOp(5,5, 'a', 'm', 1);
	BlenderOp recOp(5,5, "avg", "mix", 1);

	//recOp.conf.width  = 5;
	//recOp.conf.height = 5;
	//recOp.width = 5;
	//recOp.height = 5;
	//recOp.loops = loops;
	//recOp.decay = decay;

	/*
	double h = this->horz*1000.0/dst.odim.xscale;
	double v = (this->vert > 0.0) ? this->vert*1000.0/dst.odim.yscale : h;
	dist.setRadius(h, v);
	*/

	mout.debug3(getName() , ": pixel resolution: " , dst.odim.xscale , ',' , dst.odim.yscale );
	//mout.debug3(dist );



	PlainData<CartesianDst> & dstQuality = dstDataSet.getQualityData();
	// OR local?? ::
	// PlainData<CartesianDst> & dstQuality = dst.getQuality();

	mout.debug3("data: "    , dst );
	mout.debug3("quality: " , dstQuality );

	if (dstQuality.data.isEmpty()){
		mout.warn("Empty quality data, skipping..." );
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
		mout.debug3(recOp );
		recOp.filter(dst.data, dstQuality.data, dst.data, dstQuality.data);
	}
	else {
		mout.debug3("copying..." );
		CopyOp().filter(tmpWeight, dstQuality.data); // ??
	}
	*/

	//drain::image::File::write(dst.data,        "dstSpread-d2.png");
	//drain::image::File::write(dstQuality.data, "dstSpread-q2.png");

	ctx.currentImage     = & dst.data;
	ctx.currentGrayImage = & dst.data;

}



}  // namespace rack::



// Rack
