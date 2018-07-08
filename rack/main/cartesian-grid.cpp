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
#include <drain/imageops/DistanceTransformFillOp.h>
#include <drain/imageops/RecursiveRepairerOp.h>

#include "data/DataCoder.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"

//#include "compositing.h"
#include "images.h"
#include "cartesian-grid.h"


namespace rack {



void CartesianGrid::exec() const {

	drain::Logger mout(name, __FUNCTION__);

	RackResources & resources = getResources();

	Composite & composite = resources.composite;

	if (!composite.isDefined()){

		mout.info() << "Defining (but not allocating) composite with input data specifications" << mout.endl;

		/// Defining composite is needed for calling  composite.pix2deg(i,j,lon,lat) further below.

		const HI5TREE & cartesian = resources.cartesianHi5;

		//const drain::VariableMap & where = cartesian["where"].data.attributes;
		//const drain::VariableMap & where = cartesian["dataset1"]["where"].data.attributes;
		// mout.warn() << where << mout.endl;
		// drain::Rectangle<double> bboxD(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
		/*
			drain::VariableMap a;
			DataSelector::getAttributes(cartesian, "dataset1", a);

		 */
		CartesianODIM odim;
		DataTools::getAttributes(cartesian, "dataset1", odim, true);
		//odim.addShortKeys();
		//odim.updateFromMap(a);
		//drain::Rectangle<double> bboxD(a["where:LL_lon"], a["where:LL_lat"], a["where:UR_lon"], a["where:UR_lat"]);
		drain::Rectangle<double> bboxD(odim.LL_lon, odim.LL_lat, odim.UR_lon, odim.UR_lat);
		composite.setBoundingBoxD(bboxD);
		composite.setGeometry(odim.xsize, odim.ysize);
		//composite.setGeometry(a["where:xsize"], a["where:ysize"]);
		//const std::string projdef = a["where:projdef"];
		//mout.warn() << "trying: " << projdef<< mout.endl;
		composite.setProjection(odim.projdef);
		//composite.setProjection(where["projdef"]);
		//mout.warn() << "passed" << mout.endl;
	}

	if ((resources.currentImage != & resources.grayImage) && (resources.currentImage != &resources.colorImage)){  // resources.grayImage.isEmpty()
		//mout.error() << "Gray or color image not created yet, use --image " << mout.endl;
		mout.info() << "Gray or color image not created yet, calling --image " << mout.endl;
		cmdImage.exec();
	}

	double width = this->width;
	if (width == 0.0)
		width = 1.0;
	//width = pow(latSpacing * lonSpacing, 0.25) / 100.0;

	const bool RGB = (resources.currentImage == &resources.colorImage);

	Image & img = RGB ? getResources().colorImage : getResources().grayImage;

	const bool ALPHA = resources.currentImage->getAlphaChannelCount();

	ImageFrame & alpha = ALPHA ? img.getAlphaChannel() : img.getChannel(0);

	const drain::Rectangle<double> & bboxD = composite.getBoundingBoxD();
	const double lonResolution = static_cast<double>(composite.getFrameWidth())  / (bboxD.xUpperRight - bboxD.xLowerLeft) / width ;
	const double latResolution = static_cast<double>(composite.getFrameHeight()) / (bboxD.yUpperRight - bboxD.yLowerLeft) / width ;

	drain::FuzzyBell2<double> peak(0.0, width);
	double lat, lon;

	double latWeight, lonWeight;
	double weight;
	/// Intensity
	double f;
	const double fMax = intensity * img.getMax<double>();
	const double fMaxAlpha = img.getMax<double>();
	for (size_t j = 0; j<img.getHeight(); ++j){
		for (size_t i = 0; i<img.getWidth(); ++i){
			composite.pix2deg(i,j,lon,lat);
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


