/*

    Copyright 2006 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */
#include <drain/util/Log.h>
#include <drain/util/Time.h>  // decayTime
#include <drain/util/Variable.h>

#include <drain/image/AccumulationMethods.h>
#include <drain/image/File.h>
#include <drain/imageops/RecursiveRepairerOp.h>
#include <drain/imageops/DistanceTransformFillOp.h>

#include "main/rack.h"  // for version toOStr

#include "data/DataCoder.h"
#include "data/SourceODIM.h"
#include "product/ProductOp.h"

#include "Geometry.h"
#include "Composite.h"
#include "Coordinates.h"


using namespace drain::image;

namespace rack
{

using namespace drain;

//static DataCoder converter;

// Notice: =-32.0 is good only for DBZH
/**!
 *
 */
Composite::Composite() :  decay(1.0), cropping(false)
{

	dataSelector.quantity = ""; // "DBZH";
	dataSelector.path = ".*/(data|quality)[0-9]+/?$";  // groups  .. but quality??

	//odim.reference("type", odim.type = drain::Type::getTypeChar(typeid(void)));
	odim.reference("type", odim.type = "C");

	odim.reference("gain", odim.gain);
	odim.reference("offset", odim.offset);
	odim.reference("undetect", odim.undetect);
	odim.reference("nodata", odim.nodata);

	odim.gain = 0.0;

	//static DataCoder converter;
	//setConverter(converter);

}

void Composite::checkQuantity(const std::string & quantity){

	drain::Logger mout("Composite", __FUNCTION__);

	if (!this->odim.quantity.empty()){
		if (this->odim.quantity != quantity){
			mout.note() << "composite of quantity=" << this->odim.quantity << ", input with " << quantity << mout.endl;
		}
		/*
		if (!this->dataSelector.quantity.empty()){
			mout.note() << "applying quantity selector: " << this->dataSelector.quantity << mout.endl;
			const RegExp quantityRegExp(this->dataSelector.quantity);
			if (!quantityRegExp.test(quantity)){
				mout.warn() << "composite of '" << this->dataSelector.quantity << "', input with '" << quantity << "', resetting data? " << mout.endl;
				//this->odim.gain   = 0.0;
				//this->odim.offset = 0.0;
				//this->clear();
			}
		}*/
	}

	/*
	if (!this->odim.quantity.empty()){

		if ( this->odim.quantity != quantity){
			if (this->getMethod().name != "LATEST"){
				mout.warn() << "method: " << this->getMethod().name << ", ";
			}
			else {
				mout.warn() << "clearing scale " << EncodingODIM(this->odim) << mout.endl;
				this->odim.gain   = 0.0;
				this->odim.offset = 0.0;
				mout.note();
			}
			mout << "different quantities: ";
			mout << this->odim.quantity << " <> ";
			mout << quantity;
			mout << mout.endl;

			mout.warn() << "clearing array and scaling=" << EncodingODIM(this->odim) << mout.endl;
			this->odim.gain   = 0.0;
			this->odim.offset = 0.0;
			this->clear();
		}
	}
	*/

	this->odim.quantity = quantity;

}

void Composite::addPolar(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality, double priorWeight, bool autoProj) {

	drain::Logger mout("Composite", __FUNCTION__);

	if (!projR2M.isSet())
		autoProj = true;

	odim.object = "COMP";

	checkQuantity(srcData.odim.quantity);

	ProductBase::applyODIM(this->odim, srcData.odim);
	checkCompositingMethod(srcData.odim);


	/// TODO: regexp for accepting quantities
	const bool USE_QUALITY = !srcQuality.data.isEmpty() && (priorWeight > 0.0); // && (odim.quantity == "DBZH"); // quantity != QIND
	//const bool USE_QUALITY = src.hasQuality();



	mout.info() << "quality data: priorWeight=" << priorWeight << ',';
	if (USE_QUALITY) {
		mout << " input q properties: " << EncodingODIM(srcQuality.odim);
	}
	else {
		mout << " input q not found, empty=" << (int)srcQuality.data.isEmpty() <<  ", ok ";
	}
	mout << mout.endl;

	/// GEOGRAPHIC DEFINITIONS: USE THOSE OF THE MAIN COMPOSITE, OR USE AEQD FOR SINGLE RADAR
	RadarProj pRadarToComposite;
	pRadarToComposite.setSiteLocationDeg(srcData.odim.lon, srcData.odim.lat);

	mout.debug(1) << "Info: \"" << *this << '"' << mout.endl;
	//mout.debug(1) << "undetectValue=" << undetectValue << mout.endl;

	// Later used for data update.
	drain::Rectangle<double> bboxM;

	if (autoProj || !isDefined()){

		if (autoProj){
			const std::string & projstr = pRadarToComposite.getProjectionSrc();
			mout.info() << "Using default projection aeqd (azimuthal equidistant)." << mout.endl;
			// mout.debug() << aeqd.str() << mout.endl;
			// setProjection(aeqd.str());
			// setProjection(pRadarToComposite.getProjectionSrc());  // AEQD
			setProjection(projstr);
		}

		//mout.note(1) << "Using projection: " << getProjection() << mout.endl;

		pRadarToComposite.setProjectionDst(getProjection());

		if ((getFrameWidth() == 0)||(getFrameHeight() == 0)) // TODO
			setGeometry(srcData.data.getWidth(), srcData.data.getWidth());

		if (defaultRange > 0.0)
			pRadarToComposite.determineBoundingBoxM(defaultRange*1000.0, bboxM);
		else
			pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange(), bboxM);
		setBoundingBoxM(bboxM);

		//pRadarToComposite.determineBoundingBox
		//setBoundingBoxD(bboxD);!!
		mout.debug(1) << "BboxM: " << bboxM << mout.endl;

	}
	else {
		mout.info() << "Using user-defined projection: " << getProjection() << mout.endl;
		pRadarToComposite.setProjectionDst(getProjection());
		if (cropping){
			//drain::Rectangle<double> bboxM;
			pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange() , bboxM.xLowerLeft, bboxM.yLowerLeft, bboxM.xUpperRight, bboxM.yUpperRight);
			cropWithM(bboxM);
			//mout.toOStr() << "Cropped to: " << getBoundingBoxM() << mout.endl;
			if (getBoundingBoxM().getArea() == 0){
				mout.info() << "Cropping returned empty area." << mout.endl;
				mout.note() << "Data outside bounding box, returning" << mout.endl;
				allocate();
				updateGeoData();
				return;
			}
		}

	}
	/// Note: area not yet defined.

	// mout.warn() << "range: " << (srcData.odim.getMaxRange() / 1000.0) << " km "<< mout.endl;
	if (!pRadarToComposite.isSet()){
		mout.error() << "source or dst projection is unset " << mout.endl;
	}


	if (mout.isDebug(3)){
		/// Check mapping for the origin (= location of the radar)?
		double _x,_y;
		pRadarToComposite.projectFwd(0.0, 0.0, _x,_y);
		mout.debug(3) << "Test origin mapping: " << _x << ' ' << _y << mout.endl;
	}


	/// Limit to data extent
	//drain::Rectangle<double> bboxM; // be careful, if consider shared...
	pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange(), bboxM);
	//mout.warn() << "bbox composite:" <<  getBoundingBoxM() << ", data:" << bboxM << '\n';
	bboxM.crop(getBoundingBoxM());
	//mout.warn() << "cropped, data:" << bboxM << '\n';



	/// Area for main loop
	drain::Rectangle<int> bboxPix;
	m2pix(bboxM.xLowerLeft,  bboxM.yLowerLeft,  bboxPix.xLowerLeft,  bboxPix.yLowerLeft);
	m2pix(bboxM.xUpperRight, bboxM.yUpperRight, bboxPix.xUpperRight, bboxPix.yUpperRight);
	//mout.warn() << "Should use:" <<  bboxPix << ", in " << getFrameWidth() << 'x' << getFrameHeight() << '\n';

	//if (drain::Debug > 4){
	mout.debug();
	//mout << "Composite" << composite << '\n';
	mout << "Composite (cropped) " << *this;
	mout << " geom: " << width << 'x' << height << '\n';
	mout << "Proj:\n" << pRadarToComposite << '\n';
	mout << "Pix area:\n" << bboxPix << '\n';
	mout << mout.endl;

	mout.debug() << "allocating" << mout.endl;
	allocate();
	//mout.debug(1) << "allocated" << mout.endl;
	//std::cerr << count << std::endl;

	/// -------------------------------------------------------
	/// DATA PROEJCTION (MAIN LOOP)
	mout.debug() << "projecting" << mout.endl;
	const unsigned int bins     = srcData.data.getWidth();  // TODO odimize
	const unsigned int azimuths = srcData.data.getHeight(); // TODO odimize
	const float RAD2J = static_cast<double>(azimuths) / (2.0*M_PI);
	double range;

	//bool computeBinSpan, restartBinSpan;
	double azimuth; ///
	double x,y; ///

	// speedup
	/*
	const float j2rad = (2.0*M_PI) / static_cast<double>(azimuths);
	std::vector<double> sinLookUp(azimuths);
	std::vector<double> cosLookUp(azimuths);
	for (size_t a=0; a<azimuths; a++){
		azimuth = a * j2rad;
		sinLookUp[a] = sin(azimuth);
		cosLookUp[a] = cos(azimuth);
	}
	 */

	// converter.undetectValue = undetectValue;  // quantity.getZero() ?
	const Quantity &q = getQuantityMap().get(srcData.odim.quantity);
	const bool SKIP_UNDETECT = (DataCoder::undetectQualityCoeff == 0.0) || (! q.hasUndetectValue); //(undetectQualityCoeff == 0.0);
	//mout.warn() << "skip undetect:" << (int)SKIP_UNDETECT << mout.endl;

	DataCoder converter(srcData.odim, srcQuality.odim);



	double s, w;
	size_t address;
	for (int j=bboxPix.yLowerLeft; j>bboxPix.yUpperRight; j--){ // notice negative

		// Beam index (azimuthal image coordinate)
		unsigned int a;

		// Bin index (radial image coordinate)
		unsigned int b;

		//if ((j&31)==0)
		//	mout.debug(2) << j << mout.endl;
		//for (unsigned int i=0; i<width; i++){
		for (int i=bboxPix.xLowerLeft; i<bboxPix.xUpperRight; i++){


			pix2m(i,j,x,y);
			pRadarToComposite.projectInv(x,y);
			range = sqrt(x*x + y*y);
			b = srcData.odim.getBinIndex(range);

			//if (i==j)
			//	std::cerr << " Pix (" << i << ',' << j << "),\t=>(" << x << ',' << y << "),\t range=" << range << ",\t bin=" << b << "\n";


			/// TODO: asin, acos lookup => atan2
			/// TODO: check nodata
			if ((b >= 0) && (b < bins)){  // (if non-undetectValue rstart)
				azimuth = atan2(x,y);  // notice x <=> y  in radars
				a = (static_cast<int>(azimuth*RAD2J)+azimuths) % azimuths;
				//srcData.odim.getAzimuthalBins(azimuth);

				if (a < azimuths){

					address = data.address(i, j);

					s = srcData.data.get<double>(b,a);

					if ((SKIP_UNDETECT) && (s == srcData.odim.undetect)){
						add(address, 0.0, 0.0); // last arg (weight) 0.0 => only counter updated
					}
					else {

						if (USE_QUALITY){
							w = srcQuality.data.get<double>(b,a);
							if (converter.decode(s, w))
								add(address, s, w);
						}
						else {
							if (converter.decode(s))
								add(address, s, converter.defaultQuality);
						}
					}
				}
			}
			// DEBUG, with tile background:
			// else add(data.address(i, j), 20.0, converter.defaultQuality);

		}
	}
		// TODO: interpolation (for INJECTION)


	//updateGeoData();
	//const Rectangle<double> srcExtent(cartSrc.odim.LL_lon, cartSrc.odim.LL_lat, cartSrc.odim.UR_lon, cartSrc.odim.UR_lat);
	drain::Rectangle<double> bboxD;
	m2deg(bboxM.xLowerLeft,  bboxM.yLowerLeft,  bboxD.xLowerLeft,  bboxD.yLowerLeft);
	m2deg(bboxM.xUpperRight, bboxM.yUpperRight, bboxD.xUpperRight, bboxD.yUpperRight);
	updateDataExtent(bboxD);



	int i, j;
	m2pix((bboxM.xLowerLeft + bboxM.xUpperRight)/2.0,  (bboxM.yLowerLeft+bboxM.yUpperRight)/2.0,  i,  j);
	updateNodeMap(SourceODIM(srcData.odim.source).getSourceCode(), i, j);

	odim.update(srcData.odim); // Time, date, new
	++odim.ACCnum;

	mout.debug() << "completed" << mout.endl;

	// drain::image::File::write(data,"compD.png");
	// drain::image::File::write(weight,"compQ.png");


}


void Composite::addCartesian(const PlainData<CartesianSrc> & cartSrc, const PlainData<CartesianSrc> & srcQuality, double weight, int i0, int j0){

	drain::Logger mout("Composite", __FUNCTION__);

	checkQuantity(cartSrc.odim.quantity);

	// EnsureEncoding
	// checkInputODIM(cartSrc.odim); // RadarAccumulator
	addData(cartSrc, srcQuality, weight, i0, j0);
	++odim.ACCnum;

	/// Cartesian
	updateNodeMap(SourceODIM(cartSrc.odim.source).getSourceCode(), i0 + cartSrc.odim.xsize/2, j0 + cartSrc.odim.ysize/2);
	//updateGeoData();

	// Update geographical extent (optional information)
	const Rectangle<double> srcExtent(cartSrc.odim.LL_lon, cartSrc.odim.LL_lat, cartSrc.odim.UR_lon, cartSrc.odim.UR_lat);
	updateDataExtent(srcExtent);

	// odim.update(cartSrc.odim); // moved to add Data

	mout.debug() << "completed" << mout.endl;
}

void Composite::updateNodeMap(const std::string & node, int i, int j){
	drain::Variable & v = nodeMap[node];
	v.setType(typeid(int));
	v.setSeparator(':');
	v << i << j;
}

void Composite::updateGeoData(){

	drain::Logger mout("Composite", __FUNCTION__);

	odim.projdef = getProjection();
	odim.xsize   = getFrameWidth();
	odim.ysize   = getFrameHeight();

	const drain::Rectangle<double> &bboxD = getBoundingBoxD();
	odim.LL_lon = bboxD.xLowerLeft;
	odim.LL_lat = bboxD.yLowerLeft;
	odim.UR_lon = bboxD.xUpperRight;
	odim.UR_lat = bboxD.yUpperRight;
	double x2,y2;
	pix2LLdeg(0,-1, x2,y2); // (vertically outside)
	odim.UL_lon = x2;
	odim.UL_lat = y2;
	pix2LLdeg(getFrameWidth(), getFrameHeight()-1, x2,y2);  // (horizontally outside)
	odim.LR_lon = x2;
	odim.LR_lat = y2;

	// Determining horizontal and vertical scale in meters.
	// More reliably computed from degrees than from the metric BoundingBoxM, because the latter can actually be in radians!
	// Uses a 2 x 2 pix bboxDeg in the centre of the grid.
	/*
	if (projR2M.isLongLat()){
		mout.info() << "approx xscale and yscale (LON-LAT grid)" << mout.endl;
		const int im = getFrameWidth()/2;
		const int jm = getFrameHeight()/2;
		drain::Rectangle<double> bboxDeg;
		pix2deg(im-1, jm-1, bboxDeg.xLowerLeft,  bboxDeg.yLowerLeft);
		pix2deg(im+1, jm+1, bboxDeg.xUpperRight, bboxDeg.yUpperRight);

		odim.xscale = (bboxDeg.xUpperRight-bboxDeg.xLowerLeft )/2.0 * drain::DEG2RAD * drain::EARTH_RADIUS * cos(DEG2RAD*(bboxDeg.yLowerLeft+bboxDeg.yUpperRight)/2.0);
		odim.yscale = (bboxDeg.yLowerLeft -bboxDeg.yUpperRight)/2.0 * drain::DEG2RAD * drain::EARTH_RADIUS; //
	}
	else {
	*/
		//drain::Rectangle<double> & bboxM = getBoundingBoxM();
	odim.xscale = getXScale();
	odim.yscale = getYScale();

	// Produces ...,12568,12579,bymin,dkbor,dkrom,dksin,...
	odim.nodes = nodeMap.getKeys();

	drain::RegExp nodSyntax("^([a-z]{2})([a-z]{3}?)");
	if (nodSyntax.execute(odim.nodes) == 0){
		odim.source = "NOD:"+nodSyntax.result[1]+",ORG:"+nodSyntax.result[1];
	}
	else {
		mout.info() << "could not derive composite source NOD from nodes: " << odim.nodes << mout.endl;
	}
	//odim.source = odim.nodes.substr(0, std::min<size_t>(2, odim.nodes.length()));
	// mout.warn() << odim.nodes << mout.endl;

	odim.camethod = getMethod().name;


}




double Composite::getTimeDifferenceMinute(const CartesianODIM & odimIn) const {

	drain::Logger mout("Composite", __FUNCTION__);
	//mout.note(2) << "Tile time: " << tileTime.toStr("%Y/%m/%d %H:%M.%S") << mout.endl;

	drain::Time compositeTime;
	if (!odim.getTime(compositeTime)){
		mout.warn() << "composite date/time not set, but requested" << mout.endl;
		return 0.0;
	}
	//compositeTime.setTime(odim.date, "%Y%m%d");
	//compositeTime.setTime(odim.time, "%H%M%S");

	drain::Time tileTime;
	if (!odimIn.getTime(tileTime)){
		mout.warn() << "tile date/time not set, but requested" << mout.endl;
		return 0.0;
	}
	// tileTime.setTime(odimIn.date, "%Y%m%d");
	// tileTime.setTime(odimIn.time, "%H%M%S");

	// mout.note(2) << "Composite time: " << compositeTime.toStr("%Y/%m/%d %H:%M.%S") << mout.endl;
	// double diffMins = static_cast<double>(abs(compositeTime.getTime() - tileTime.getTime()))/60.0;
	// mout.note(2) << "Time difference (minutes): " << diffMins << mout.endl;
	//weight = weight*pow(composite.decay, diffMins);
	return static_cast<double>(abs(compositeTime.getTime() - tileTime.getTime()))/60.0;
}

}

