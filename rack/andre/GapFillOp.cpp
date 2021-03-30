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




#include "drain/image/File.h"
#include "drain/image/Intensity.h"
#include "drain/imageops/CopyOp.h"
#include "drain/imageops/DistanceTransformFillOp.h"
//#include "drain/imageops/RecursiveRepairerOp.h"
#include "drain/imageops/BlenderOp.h"



#include "GapFillOp.h"



namespace rack {

using namespace drain::image;
using namespace hi5;

/// TODO: move to data tools etc?
/*
void applyMask(const PlainData<PolarSrc> & srcData, PlainData<PolarSrc> & dstData) {
}
*/

//void GapFillOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {
void GapFillOp::processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
		PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQuality) const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	mout.debug() << *this << mout;

	//File::write(srcData.data,"GapFillOp-in.png");
	//File::write(srcQuality.data,"GapFillOp-inq.png");

	// DistanceTransformFillLinearOp op;
	//DistanceTransformFillExponentialOp op;
	//FastAverageOp op;

	// Pixels, but float, because radii.
	double horz = widthM  / srcData.odim.rscale;
	double vert = heightD * srcData.data.getHeight() / 360.0;
	//op.setRadius(horz, vert, horz, vert);
	//op.setSize(horz,  vert);
	BlenderOp blenderOp(horz, vert, "avg", "max", loops, expansionCoeff);
	//DistanceTransformFillExponentialOp distOp(horz, vert, DistanceModel::PIX_CHESS_CONNECTED);
	DistanceTransformFillLinearOp distOp(horz, vert, DistanceModel::PIX_CHESS_CONNECTED);

	ImageOp & op = (loops == 0) ? (ImageOp &)distOp : (ImageOp &)blenderOp;

	mout.special() << "loops=" << loops << " => using " << op << mout;



	// Esp. radius
	mout.debug() << op << mout;

	drain::image::Image tmpData(srcData.data.getConf());
	tmpData.setName("tmpData");
	// mout.warn() << tmpData << mout;
	mout.debug() << tmpData.getChannel(0) << mout;

	drain::image::Image tmpQuality; //(srcQuality.data.getConf());
	tmpQuality.setName("tmpQuality");
	//tmpQuality.copyDeep(srcQuality.data); //??
	tmpQuality.setConf(srcQuality.data.getConf()); //??

	srcData.createSimpleQualityData(tmpQuality, NAN, 0, 0); // = skip quality of values, and clear quality of special codes

	// tmpQuality.setPhysicalRange(0, 255, true);
	// tmpQuality.getChannel(0).setPhysicalRange(0, 250, true);
	// mout.warn() << tmpQuality << mout;
	// mout.warn() << tmpQuality.getChannel(0) << mout;

	//File::write(tmpQuality,"GapFillOp-intq.png");

	//dstData.setEncoding(srcData.data.getType()); // itself?
	//dstData.setGeometry(srcData.data.getGeometry()); // itself?
	//double qNAN = std::numeric_limits<double>::quiet_NaN();
	//dstData.createSimpleQualityData(dstQuality, NAN, 0, 0); // = skip special codes

	ImageTray<const Channel> srcTray;
	srcTray.setChannels(srcData.data, tmpQuality);
	mout.debug() << "srcTray:\n" << srcTray << mout;
	// File::write(srcTray.get(),"GapFillOp-ind.png");
	// File::write(srcTray.getAlpha(),"GapFillOp-inq.png");

	ImageTray<Channel> dstTray;
	//dstQuality.data.clear();
	//dstQuality.data.setPhysicalRange(0, 250, true); // temporary scaling
	//dstQuality.data.getChannel(0).setPhysicalRange(0, 250, true); // temporary scaling
	//drain::image::Image foo;
	//foo.copyDeep(srcQuality.data);
	//foo.setConf(srcQuality.data.getConf());
	//dstTray.setChannels(dstData.data, foo);
	//dstTray.setChannels(tmpData, tmpQuality);
	dstTray.setChannels(tmpData, dstQuality.data);
	mout.debug() << "dstTray:\n" << dstTray << mout;

	//File::write(dstQuality.data,"GapFillOp-inzq.png");

	op.traverseChannels(srcTray, dstTray);

	//


	// Finally, restore UNDETECT where it was originally. (Leaves some nodata overwritten)
	const double defaultQuality = dstQuality.odim.scaleInverse(0.75);
	Image::iterator  it  = srcData.data.begin();
	Image::iterator tit  = tmpData.begin();
	Image::iterator dit  = dstData.data.begin();
	Image::iterator dqit = dstQuality.data.begin();
	//double undetect = dstData.odim.undetect;
	while (it != srcData.data.end()){
		//if ((*it != odim.nodata) && (*it != odim.undetect))
		if (*it == srcData.odim.undetect){
			*dit = dstData.odim.undetect;
			*qdit = defaultQuality;
		}
		else {
			*dit = *tit; // same encoding, direct assignment ok
		}
		++it;
		++tit;
		++dit;
		++dqit;
	}

	// op.traverseChannels(srcData.data, srcData.getQualityData().data, dstData.data, dstData.getQualityData().data);
	// File::write(dstTray.get(),"GapFillOp-out.png");
	// File::write(dstTray.getAlpha(),"GapFillOp-outq.png");
	// File::write(tmpData,"GapFillOp-out.png");
	// File::write(tmpQuality,"GapFillOp-outq.png");
	// File::write(foo, "GapFillOp-outQ.png");
	//File::write(dstQuality.data.getChannel(0), "GapFillOp-outQ.png");
}

//void GapFillRecOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {
void GapFillRecOp::processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
		PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQuality) const {

	//srcData.odim.getBinIndex()
	double h = widthM / srcData.odim.rscale;  //srcData.odim.getBinDistance(width); //
	double v = heightD * srcData.data.getHeight() / 360.0;

	BlenderOp op(h, v, "avg", "mix", loops);

	ImageTray<const Channel> srcTray;
	srcTray.setChannels(srcData.data, srcQuality.data);
	ImageTray<Channel> dstTray;
	dstTray.setChannels(dstData.data, dstQuality.data);
	//op.filter(srcData.data, srcData.getQualityData().data, dstData.data, dstData.getQualityData().data);
	op.traverseChannels(srcTray, dstTray);

}



}  // rack::

// Rack
