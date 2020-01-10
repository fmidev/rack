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



#include <drain/util/Fuzzy.h>
#include <drain/image/GeoFrame.h>

#include <drain/image/File.h>
//#include <drain/imageops/DistanceTransformFillOp.h>
//#include <drain/imageops/RecursiveRepairerOp.h>

#include "data/DataCoder.h"
#include "hi5/Hi5.h"
//#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"

//#include "compositing.h"
#include "images.h"
#include "cartesian-grid.h"


namespace rack {



void CartesianGrid::exec() const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	RackResources & resources = getResources();

	// Composite & geoFrame = resources.geoFrame;
	// drain::image::GeoFrame geoFrame(resources.composite);
	drain::image::GeoFrame geoFrame;

	//if (!geoFrame.isDefined()){

	mout.info() << "Defining (but not allocating) composite with input data specifications" << mout.endl;
	/// Defining geoFrame is needed for calling  geoFrame.pix2deg(i,j,lon,lat) further below.

	const Hi5Tree & cartesian = resources.cartesianHi5;

	CartesianODIM odim;
	DataTools::getAttributes(cartesian, "dataset1", odim, true);

	drain::Rectangle<double> bboxD(odim.LL_lon, odim.LL_lat, odim.UR_lon, odim.UR_lat);
	if (bboxD.getArea() == 0.0){
		mout.warn() << "empty bbox, returning" << mout.endl;
		return;
	}

	geoFrame.setBoundingBoxD(bboxD);
	geoFrame.setBoundingBoxD(odim.LL_lon, odim.LL_lat, odim.UR_lon, odim.UR_lat);
	geoFrame.setGeometry(odim.xsize, odim.ysize);

	if (odim.projdef.empty()){
		mout.warn() << "projdef missing, returning" << mout.endl;
		return;
	}

	geoFrame.setProjection(odim.projdef);
	// mout.warn() << "passed" << mout.endl;
	// }

	if ((resources.currentImage != & resources.grayImage) && (resources.currentImage != &resources.colorImage)){  // resources.grayImage.isEmpty()
		//mout.error() << "Gray or color image not created yet, use --image " << mout.endl;
		mout.info() << "Gray or color image not created yet, calling --image " << mout.endl;
		cmdImage.exec();
	}

	double width = this->width;
	if (width == 0.0)
		width = 1.0;

	const bool RGB = (resources.currentImage == &resources.colorImage);
	Image & img = RGB ? getResources().colorImage : getResources().grayImage;

	const bool ALPHA = resources.currentImage->getAlphaChannelCount();
	ImageFrame & alpha = ALPHA ? img.getAlphaChannel() : img.getChannel(0);

	//const drain::Rectangle<double> & bboxD = geoFrame.getBoundingBoxD();
	/*
	const double lonResolution = static_cast<double>(geoFrame.getFrameWidth())  / (bboxD.upperRight.x - bboxD.lowerLeft.x) / width ;
	const double latResolution = static_cast<double>(geoFrame.getFrameHeight()) / (bboxD.upperRight.y - bboxD.lowerLeft.y) / width ;
	*/
	const double lonResolution = static_cast<double>(img.getWidth())  / (bboxD.upperRight.x - bboxD.lowerLeft.x) / width ;
	const double latResolution = static_cast<double>(img.getHeight()) / (bboxD.upperRight.y - bboxD.lowerLeft.y) / width ;

	drain::FuzzyBell2<double> peak(0.0, width);
	double lat, lon;

	double latWeight, lonWeight;
	double weight;
	/// Intensity
	double f;
	const double fMax = intensity * img.getEncoding().getTypeMax<double>();
	const double fMaxAlpha = img.getEncoding().getTypeMax<double>();
	for (size_t j = 0; j<img.getHeight(); ++j){
		for (size_t i = 0; i<img.getWidth(); ++i){
			geoFrame.pix2deg(i,j,lon,lat);
			lonWeight = peak(lonResolution * (lon - lonSpacing*round(lon/lonSpacing)));
			latWeight = peak(latResolution * (lat - latSpacing*round(lat/latSpacing)));
			weight = std::max(lonWeight, latWeight);
			//if (j == 400) std::cerr << i << ' ' << lon << '\t' <<weight << '\n';
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
