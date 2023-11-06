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
#include "drain/image/GeoFrame.h"

#include "data/DataCoder.h"
#include "hi5/Hi5.h"

// #include "radar/Coordinates.h"
#include "radar/Composite.h"

#include "cartesian-grid.h"


namespace rack {



void CartesianGrid::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	/// GeoFrame is needed for calling  geoFrame.pix2deg(i,j, lon,lat) further below.
	drain::image::GeoFrame geoFrame;

	mout.info("Defining (but not allocating) composite with input data specifications" );

	const Hi5Tree & cartesian = ctx.cartesianHi5;

	CartesianODIM odim;
	DataTools::getAttributes(cartesian, "dataset1", odim, true);
	mout.debug2() << odim << mout.endl;

	if (odim.projdef.empty()){
		mout.warn("projdef missing, returning" );
		return;
	}
	geoFrame.setProjection(odim.projdef);

	//drain::Rectangle<double> bboxD(odim.LL_lon, odim.LL_lat, odim.UR_lon, odim.UR_lat);
	// const drain::Rectangle<double> bboxD(odim.bboxD);
	if (odim.bboxD.getArea() == 0.0){
		mout.warn("empty bbox, returning" );
		return;
	}
	geoFrame.setBoundingBoxD(odim.bboxD);

	geoFrame.setGeometry(odim.area.width, odim.area.height);

	//geoFrame.setGeometry(odim.geometry.width, odim.geometry.height);
	mout.debug(geoFrame );

	/*
	if ((ctx.currentImage != & ctx.grayImage) && (ctx.currentImage != &ctx.colorImage)){  // ctx.grayImage.isEmpty()
		//mout.error("Gray or color image not created yet, use --image " );
		mout.info("Gray or color image not created yet, calling --image " );
		//cmdImage.exec();
		if (CmdImage::convertGrayImage(ctx)){
			mout.fail("could not find image" );
		}
	}
	*/
	// TODO: replace with CmdImage::getModifiableImage(ctx);

	double width = this->width;
	if (width == 0.0)
		width = 1.0;

	/*
	const bool RGB = (ctx.currentImage == &ctx.colorImage);
	Image & img = RGB ? ctx.colorImage : ctx.grayImage;
	const bool ALPHA = ctx.currentImage->hasAlphaChannel();
	*/

	Image & img = ctx.getModifiableImage(); // ImageKit::getModifiableImage(ctx);
	const bool RGB = (img.getChannelCount()>=3);
	const bool ALPHA = img.hasAlphaChannel();

	ImageFrame & alpha = ALPHA ? img.getAlphaChannel() : img.getChannel(0); // Latter dummy

	//const drain::Rectangle<double> & bboxD = geoFrame.getBoundingBoxD();
	// const double lonResolution = static_cast<double>(img.getWidth())  / (bboxD.upperRight.x - bboxD.lowerLeft.x) / width ;
	// const double latResolution = static_cast<double>(img.getHeight()) / (bboxD.upperRight.y - bboxD.lowerLeft.y) / width ;

	const double lonResolution = static_cast<double>(img.getWidth())  / odim.bboxD.getWidth()  / width ;
	const double latResolution = static_cast<double>(img.getHeight()) / odim.bboxD.getHeight() / width ; // linewidth

	drain::FuzzyBell2<double> peak(0.0, width);
	double lat, lon;

	double latWeight, lonWeight;
	double weight;
	/// Intensity
	double f;
	const double fMax = intensity * img.getConf().getTypeMax<double>();
	const double fMaxAlpha = img.getConf().getTypeMax<double>();
	for (size_t j = 0; j<img.getHeight(); ++j){
		for (size_t i = 0; i<img.getWidth(); ++i){
			geoFrame.pix2deg(i,j,lon,lat);
			lonWeight = peak(lonResolution * (lon - lonSpacing*round(lon/lonSpacing)));
			latWeight = peak(latResolution * (lat - latSpacing*round(lat/latSpacing)));
			weight = std::max(lonWeight, latWeight);
			//weight = 0.5;<<
			// if (i == j) std::cerr << i << ' ' << lon << '\t' <<  lat << '\t' <<weight << '\n';
			if (RGB){
				for (int k = 0; k < 3; ++k) {
					f = img.get<double>(i,j,k);
					img.put(i,j,k, (1.0-weight)*f + weight*fMax);
				}
			}
			else {
				f = img.get<double>(i,j);
				img.put(i,j, (1.0-weight)*f + weight*fMax);
			}
			if (ALPHA){
				f = alpha.get<double>(i,j);
				alpha.put(i,j, std::max(f, weight*fMaxAlpha));
			}
			//img.put(i,j,k, (1.0-weight)*f + weight*(fMax-f));  XOR
			//img.put(i,j, (1.0-w)*f + w*255.0);
		}
	}
}



}  // namespace rack::



// Rack
