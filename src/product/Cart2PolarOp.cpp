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

#include <stdexcept>

#include "Cart2PolarOp.h"

#include "radar/RadarProj.h"


namespace rack {

using namespace drain::image;

const CoordinatePolicy Cart2PolarOp::polarCoordPolicy(EdgePolicy::POLAR, EdgePolicy::WRAP, EdgePolicy::LIMIT,EdgePolicy::WRAP);

Cart2PolarOp::Cart2PolarOp(const std::string & name, const std::string & description) :  RadarProductOp<const CartesianODIM, PolarODIM> (name, description) {

	drain::Logger mout(__FILE__, __FUNCTION__);
	getParameters().link("nrays",  odim.area.height = 360, "Number of azimuthal rays");
	getParameters().link("nbins",  odim.area.width = 500, "Number of bins on a ray");
	getParameters().link("rscale", odim.rscale = 500, "Range, in metres");
	// getParameters().link("lon", radarSite.x = std::numeric_limits<double>::quiet_NaN()); // ::min());
	// getParameters().link("lat", radarSite.y = std::numeric_limits<double>::quiet_NaN());
	getParameters().link("lon", odim.lon = std::numeric_limits<double>::quiet_NaN()); // ::min());
	getParameters().link("lat", odim.lat = std::numeric_limits<double>::quiet_NaN());
	// getParameters().link("range", rangeM); // alternative?
	/*
	allowedEncoding.link("type", odim.type = "C");
	allowedEncoding.link("gain", odim.scaling.scale);
	allowedEncoding.link("offset", odim.scaling.offset);
	// 2018
	allowedEncoding.link("undetect", odim.undetect);
	allowedEncoding.link("nodata", odim.nodata);

	allowedEncoding.link("nrays", odim.area.height);
	allowedEncoding.link("nbins", odim.area.width);

	aboveSeaLevel = true;
	*/
	// this->odim.product = "PPROD"; // NEW
	this->odim.object  = "PVOL"; // consider!

	this->odim.quantity = "DBZV";
	//dataSelector.orderFlags.set(DataSelector::ELANGLE, DataSelector::MIN);
	// dataSelector.setOrder(DataOrder::DATA, DataOrder::MIN);
	// dataSelector.setPrf(DataSelector::Prf::SINGLE);

	//dataSelector.updateBean(); // NOT NEEDED, when objects updated directly

	// mout.debug2(dataSelector);

};



Cart2PolarOp::Cart2PolarOp(const Cart2PolarOp & op) :  RadarProductOp<const CartesianODIM, PolarODIM>(op){
	// dataSelector.setParameters(op.dataSelector.getParameters()); // should not be needed
	//allowedEncoding.copyStruct(op.allowedEncoding, op.odim, odim);
	getParameters().copyStruct(op.getParameters(), op, *this);
	// setParameters(op.getParameters());
}

void Cart2PolarOp::processData(const Data<src_t > & srcData, Data<dst_t > & dstData) const {
	drain::Logger mout(__FILE__, __FUNCTION__);


	if (std::isnan(odim.lon) || std::isnan(odim.lat)) {
		mout.warn(DRAIN_LOG(odim));
		mout.error("Radar site lon/lat not set; provide lon=...,lat=... parameters");
		return;
	}

	if (srcData.odim.projdef.empty()) {
		mout.error("Source Cartesian data has no projdef");
		return;
	}

	// dstData.odim.copyEncoding(srcData.odim);
	dstData.initialize(srcData.odim.type, odim.getGeometry());
	dstData.copyEncoding(srcData.odim);
	dstData.odim.rscale = odim.rscale;
	dstData.odim.rstart = odim.rstart;

	if (srcData.odim.type != odim.type) {
		mout.warn(DRAIN_LOG(srcData.odim.type));
		mout.warn(DRAIN_LOG(odim.type));
		mout.warn("Using storage type '", srcData.odim.type, "' of source (discarding odim type '", odim.type, "'");
	}


	// Propagate site location to output ODIM
	/*
	dstData.odim.lon = radarSite.x;
	dstData.odim.lat = radarSite.y;
	*/

	// Projection: radar-centred AEQD (src) → native Cartesian CRS of composite (dst)
	RadarProj proj;
	proj.setSiteLocationDeg(odim.lon, odim.lat);

	drain::image::GeoFrame cartesianFrame;
	cartesianFrame.setGeometry(srcData.odim.area);
	cartesianFrame.setBoundingBoxD(srcData.odim.getBoundingBoxDeg());

	// proj.setDirectMapping(false);
	if (srcData.odim.epsg > 0){
		proj.setProjectionDst(srcData.odim.epsg);
		cartesianFrame.setProjectionEPSG(srcData.odim.epsg);
	}
	else {
		proj.setProjectionDst(srcData.odim.projdef);
		cartesianFrame.setProjection(srcData.odim.projdef);
	}


	// Determine native metric lower-left corner of the Cartesian source image
	// by forwarding the geographic LL corner through the Cartesian projection.
	/*
	drain::Proj6 lonlatToCart;
	lonlatToCart.setProjectionSrc("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
	//lonlatToCart.setProjectionSrc(4326);
	lonlatToCart.setProjectionDst(srcData.odim.projdef);
	double xLL = srcData.odim.bboxD.lowerLeft.x;  // LL_lon [deg]
	double yLL = srcData.odim.bboxD.lowerLeft.y;  // LL_lat [deg]
	lonlatToCart.projectFwd(xLL, yLL);             // → native metric [m]
	*/

	const size_t srcWidth  = srcData.data.getWidth();
	const size_t srcHeight = srcData.data.getHeight();
	// const double xscale    = srcData.odim.xscale;
	// const double yscale    = srcData.odim.yscale;
	// Native y of the image's top edge; image rows go top-to-bottom, metric y bottom-to-top.
	// const double yUR = yLL + static_cast<double>(srcHeight) * yscale;

	const size_t nbins = dstData.data.getWidth();   // radial (i)
	const size_t nrays = dstData.data.getHeight();  // azimuthal (j)

	mout.note("Resampling ", srcWidth, "x", srcHeight, " Cartesian → ", nbins, "x", nrays, " polar");
	// mout.note("Native Cartesian LL: (", xLL, ", ", yLL, ")  xscale=", xscale, "  yscale=", yscale);

	mout.note(cartesianFrame);

	drain::Point2D<double> pGeo;
	drain::Point2D<int>    pImg;

	for (size_t j = 0; j < nrays; ++j) {
		const double azimuth = dstData.odim.getAzimuth(j);  // [rad], 0=North, CW
		const double s = std::sin(azimuth);
		const double c = std::cos(azimuth);

		for (size_t i = 0; i < nbins; ++i) {

			const double range = dstData.odim.getBinDistance(i);  // [m]

			// Radar-centred AEQD: East=x, North=y (radar convention: sin/cos swapped from math)
			pGeo.x = range * s;
			pGeo.y = range * c;

			// Cartesian radar-centered AEQD -> Cartesian (composite) coordinates.
			proj.projectFwd(pGeo);
			cartesianFrame.m2pix(pGeo, pImg);
			/*
			if (((i&31)==0) && ((j&31)==0)){
				mout.note(i, '\t', j, '\t', range, '\t', pGeo, "\t -> ", pImg);
			}
			*/

			if (pImg.x < 0 || pImg.x >= static_cast<int>(srcWidth) ||
					pImg.y < 0 || pImg.y >= static_cast<int>(srcHeight)) {
				dstData.data.put(i, j, dstData.odim.nodata);
			}
			else {
				// Same encoding in source and destination: raw values can be copied directly.
				dstData.data.put(i, j, srcData.data.get<double>(pImg));
			}
		}
	}

};

void Cart2PolarOp::setGeometry(const CartesianODIM & srcODIM, PlainData<dst_t > & dstData) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.experimental("Setting bin count (nbins) to half-width of Cartesian input");

	// TODO: detect if lon,lat unset, then this (centering) is "more ok".

	// Note: width can be large.
	dstData.setGeometry(srcODIM.getGeometry().getWidth(), 360);

	// Note: this could be odd...
	// dstData.setGeometry(srcODIM.getGeometry());

}



///  This method searches a geometry that "maximizes" resolution and range with respect to input data.
/**
 *  In other words, it searches for
 *  - largest beam count ( \c nrays ), unless set
 *  - smallest beam directional resolution (\c rscale ), unless set
 *  - largest range \c rscale × \c nbins , unless set
 *
 *
 */
/*
void Cart2PolarOp::deriveDstGeometry(const DataSetMap<PolarSrc> & srcSweeps, PolarODIM & dstOdim, bool MINIMISE_RSCALE) const {

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL name+"(CumulativeProductOp)", __FUNCTION__);

	const bool DERIVE_NBINS  = (dstOdim.area.width == 0); // ||(dstOdim.rscale == 0.0);
	const bool DERIVE_NRAYS  = (dstOdim.area.height == 0);
	const bool DERIVE_RSCALE = (dstOdim.rscale == 0.0); // (dstOdim.geometry.width == 0); //
	//const bool MAXIMIZE_AZM_RESOLUTION = (dstOdim.area.height == 0); //
	const bool MAXIMIZE_AZM_RESOLUTION = DERIVE_NRAYS; // currently simply bound
	const bool MAXIMIZE_RANGE = (DERIVE_NBINS && DERIVE_RSCALE); // Use those producing maximal scale

	double range;
	double rangeMax = 0;
	size_t nbinsMax = 0;
	double rscaleMin = 2000;

	mout.debug2(dstOdim);

	//if (MAXIMIZE_AZM_RESOLUTION || DERIVE_NBINS || DERIVE_RSCALE){
	if (DERIVE_NBINS || DERIVE_NRAYS || DERIVE_RSCALE || MINIMISE_RSCALE){

		dstOdim.angles.clear();
		//dstOdim.angles.resize(0); // DO NOT USE clear(), it changes address of 1st elem
		//dstOdim.angles.resize(srcSweeps.size()); // DO NOT USE clear(), it changes address of 1st elem

		for (const auto & entry: srcSweeps){

			const DataSet<PolarSrc> & srcDataSet = entry.second;
			const Data<PolarSrc>       & srcData = srcDataSet.getFirstData();
			const PolarODIM            & srcOdim = srcData.odim;

			if (srcData.data.isEmpty()){
				mout.warn("srcData empty");
				continue; // warning issued later, in the main loop.
				//mout.warn("selected quantity=" , quantity , " not present in elangle=" , it->first );
			}

			dstOdim.angles.push_back(srcOdim.elangle);

			mout.debug2("testing: " , srcOdim);

			if (MAXIMIZE_AZM_RESOLUTION){
				if (srcOdim.area.height > dstOdim.area.height){
					dstOdim.area.height = srcOdim.area.height;
					mout.info("Updating dst nrays to: " , dstOdim.area.height );
				}
			}

			nbinsMax  = std::max(nbinsMax,  srcOdim.area.width);
			rscaleMin = std::min(rscaleMin, srcOdim.rscale);
			range = static_cast<double>(srcOdim.area.width) * srcOdim.rscale;
			if (range > rangeMax){
				rangeMax = range;
				if (MAXIMIZE_RANGE){
					dstOdim.area.width  = srcOdim.area.width;
					dstOdim.rscale = srcOdim.rscale;
				}
			}

		}

		if (MAXIMIZE_RANGE){
			mout.info("Applied input geometry with maximum range" ); // special<LOG_NOTICE>
		}
		else if (dstOdim.area.width ==0){
			dstOdim.area.width = rangeMax/static_cast<int>(dstOdim.rscale);
			mout.special<LOG_NOTICE>("Derived nbins=" , dstOdim.area.width );
		}
		else if (dstOdim.rscale ==0){
			dstOdim.rscale = rangeMax/static_cast<double>(dstOdim.area.width);
			mout.special<LOG_NOTICE>("Derived rscale=" , dstOdim.rscale );
		}
		else {
			// mout.debug("Adapting user-defined nbins and rscale" );
		}

		mout.info("Setting dst geometry:" , dstOdim.area.width , "bin x " , dstOdim.rscale , "m/bin (" , (dstOdim.getMaxRange()/1000.0) , "km) " );

	}
	else {
		//mout.warn("output prop" , dstOdim );
		mout.info("output properties: " , EncodingODIM(dstOdim) );
	}

	if (MINIMISE_RSCALE){
		range = dstOdim.getMaxRange();
		dstOdim.rscale = rscaleMin;
		dstOdim.area.width = ::lround(range/dstOdim.rscale);
	}

}
*/

/*
void Cart2PolarOp::determineMetadata(const DataSetMap<PolarSrc> & srcSweeps, PolarODIM & dstOdim) const {

	for (const auto & entry: srcSweeps){
		const DataSet<PolarSrc> & srcDataSet = entry.second;
		//const Data<PolarSrc>       & srcData = srcDataSet.getFirstData();
		// const PolarODIM            & srcOdim = srcData.odim;
		dstOdim.updateLenient(srcDataSet.getFirstData().odim);
	}

}
*/



}  // namespace rack

