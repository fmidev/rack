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
#include <drain/util/Log.h>
#include <drain/util/Time.h>  // decayTime
#include <drain/util/Variable.h>

#include <drain/image/AccumulationMethods.h>
#include <drain/image/File.h>
//#include <drain/imageops/RecursiveRepairerOp.h>
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
	//dataSelector.path = ".*/(data|quality)[0-9]+/?$";  // groups  .. but quality??

	//odim.reference("type", odim.type = drain::Type::getTypeChar(typeid(void)));
	//odim.reference("type", odim.type = "C");
	odim.reference("type", odim.type = "C");

	odim.reference("gain", odim.scale);
	odim.reference("offset", odim.offset);
	odim.reference("undetect", odim.undetect);
	odim.reference("nodata", odim.nodata);

	odim.scale = 0.0;

	//static DataCoder converter;
	//setConverter(converter);

}

void Composite::checkQuantity(const std::string & quantity){

	drain::Logger mout("Composite", __FUNCTION__);
	/// TODO: regexp for accepting quantities

	if (!this->odim.quantity.empty()){
		if (this->odim.quantity != quantity){
			mout.note() << "composite of quantity=" << this->odim.quantity << ", input with " << quantity << mout.endl;
		}
		if (odim.ACCnum > 1){
			if (quantity.substr(0, 4) == "VRAD"){
				mout.warn() << "compositing VRAD directly, consider Doppler dealiasing (u,v) first" << quantity << mout.endl;
			}
		}
	}

	this->odim.quantity = quantity;

}

void Composite::addPolar(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality, double priorWeight, bool autoProj) {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const DataSet<PolarSrc> konsta(srcData.getTree()["dataset1"]);  // TODO REMOVE XX

	//DataSet<PolarDst> varjo(srcData.tree["dataset1"]);  // TODO REMOVE XX


	if (!projR2M.isSet())
		autoProj = true;

	odim.object = "COMP";

	checkQuantity(srcData.odim.quantity);

	ProductBase::applyODIM(this->odim, srcData.odim);

	checkCompositingMethod(srcData.odim);

	const bool USE_PRIOR_WEIGHT = (priorWeight > 0.0);

	const bool USE_QUALITY_FIELD = USE_PRIOR_WEIGHT && !srcQuality.data.isEmpty(); // && (odim.quantity == "DBZH"); // quantity != QIND



	if (USE_QUALITY_FIELD) {
		mout.info() << " using input q: " << EncodingODIM(srcQuality.odim) << mout.endl;
	}
	else if (USE_PRIOR_WEIGHT) {
		mout.info() << " using input weight=" << priorWeight << mout.endl;
		// TODO
		// mout.info() << "input quality exists=" << srcQuality.data.isEmpty() << ',';
	}
	else {
		mout.info() << " quality weighting not applied" << mout.endl;
		if (srcQuality.data.isEmpty())
			mout.note() << " (input quality would be available) " << mout.endl;
	}

	/// GEOGRAPHIC DEFINITIONS: USE THOSE OF THE MAIN COMPOSITE, OR USE AEQD FOR SINGLE RADAR
	RadarProj pRadarToComposite;
	pRadarToComposite.setSiteLocationDeg(srcData.odim.lon, srcData.odim.lat);

	if (odim.source.empty())
		odim.source = srcData.odim.source; // for tile (single-radar "composite")

	mout.debug(1) << "Info: \"" << *this << '"' << mout.endl;
	//mout.debug(1) << "undetectValue=" << undetectValue << mout.endl;

	// Defined here, because later used for data update.
	drain::Rectangle<double> bboxM;

	if (autoProj || !isDefined()){

		if (autoProj){
			mout.info() << "Using default projection aeqd (azimuthal equidistant)." << mout.endl;
			const std::string & aeqdStr = pRadarToComposite.getProjectionSrc();
			// mout.debug() << aeqdStr << mout.endl;
			setProjection(aeqdStr);
		}

		//mout.note(1) << "Using projection: " << getProjection() << mout.endl;

		pRadarToComposite.setProjectionDst(getProjection());

		if (! geometryIsSet()){
			setGeometry(500, 500);
			mout.info() << "Size not given, using default: " << this->getFrameWidth() << ',' << this->getFrameHeight() << mout.endl;
		}

		if (PolarODIM::defaultRange > 0.0){
			mout.info() << "Using default range: " << (PolarODIM::defaultRange) << mout.endl;
			pRadarToComposite.determineBoundingBoxM(PolarODIM::defaultRange, bboxM);
		}
		else {
			mout.info() << "Using maximum range: " << srcData.odim.getMaxRange(false) << mout.endl;
			pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange(true), bboxM);
		}
		setBoundingBoxM(bboxM);

		//setBoundingBoxD(bboxD); !?
		mout.debug() << "BBoxM: " << bboxM << ", range:" << srcData.odim.getMaxRange(true)<< mout.endl;

	}
	else {
		mout.info() << "Using user-defined projection: " << getProjection() << mout.endl;
		pRadarToComposite.setProjectionDst(getProjection());
		if (cropping){
			//drain::Rectangle<double> bboxM;
			pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange() , bboxM);
			mout.debug() << "Orig: " << getBoundingBoxM() << mout.endl;
			mout.debug() << "Cropping with " << srcData.odim.getMaxRange() << " range with bbox=" << bboxM << mout.endl;
			cropWithM(bboxM);
			mout.info() << "Cropped to: " << getBoundingBoxM() << mout.endl;
			if (getBoundingBoxM().getArea() == 0){
				mout.info() << "Cropping returned empty area." << mout.endl;
				mout.note() << "Data outside bounding box, returning" << mout.endl;
				allocate(); // ?
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


	if (mout.isDebug(2)){
		/// Check mapping for the origin (= location of the radar)?
		double _x,_y;
		pRadarToComposite.projectFwd(0.0, 0.0, _x,_y);
		mout.debug(2) << "Test origin mapping: " << _x << ' ' << _y << mout.endl;
	}


	/// Limit to data extent
	pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange(), bboxM);
	//mout.warn() << "bbox composite:" <<  getBoundingBoxM() << ", data:" << bboxM << '\n';
	bboxM.crop(getBoundingBoxM());
	//mout.warn() << "cropped, data:" << bboxM << '\n';



	/// Area for main loop
	drain::Rectangle<int> bboxPix;
	m2pix(bboxM.lowerLeft,  bboxPix.lowerLeft);
	m2pix(bboxM.upperRight, bboxPix.upperRight);
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
	const int bins  = srcData.data.getWidth();  // TODO odimize
	const int beams = srcData.data.getHeight(); // TODO odimize
	const float RAD2J = 1.0/srcData.odim.getBeamWidth();  //static_cast<double>(beams) / (2.0*M_PI);
	double range;

	//bool computeBinSpan, restartBinSpan;
	double azimuth; ///
	//double x,y; ///

	// speedup
	/*
	const float j2rad = (2.0*M_PI) / static_cast<double>(beams);
	std::vector<double> sinLookUp(beams);
	std::vector<double> cosLookUp(beams);
	for (size_t a=0; a<beams; a++){
		azimuth = a * j2rad;
		sinLookUp[a] = sin(azimuth);
		cosLookUp[a] = cos(azimuth);
	}
	 */

	// converter.undetectValue = undetectValue;  // quantity.getZero() ?
	//const Quantity &q = getQuantityMap().get(srcData.odim.quantity);
	//const bool SKIP_UNDETECT = (DataCoder::undetectQualityCoeff == 0.0) || (! q.hasUndetectValue); //(undetectQualityCoeff == 0.0);
	//mout.warn() << "skip undetect:" << (int)SKIP_UNDETECT << mout.endl;

	DataCoder converter(srcData.odim, srcQuality.odim);



	double s;
	double w = priorWeight * converter.defaultQuality;

	drain::Point2D<int> pComp;
	drain::Point2D<double> pMetric;

	size_t address;
	for (pComp.y = bboxPix.lowerLeft.y; pComp.y>bboxPix.upperRight.y; --pComp.y){ // notice negative

		// Beam index (azimuthal coordinate of polar input data)
		int a;

		// Bin index (radial coordinate of polar input data)
		int b;

		//if ((j&31)==0)
		//	mout.debug(2) << j << mout.endl;
		//for (unsigned int i=0; i<width; i++){
		for (pComp.x = bboxPix.lowerLeft.x; pComp.x<bboxPix.upperRight.x; ++pComp.x){


			//pix2m(i,j,x,y);
			pix2m(pComp, pMetric);
			pRadarToComposite.projectInv(pMetric.x, pMetric.y);
			//pRadarToComposite.projectInv(x,y);
			range = ::sqrt(pMetric.x*pMetric.x + pMetric.y*pMetric.y);
			b = srcData.odim.getBinIndex(range);

			//if (i==j)
			//	std::cerr << " Pix (" << i << ',' << j << "),\t=>(" << x << ',' << y << "),\t range=" << range << ",\t bin=" << b << "\n";

			/// TODO: check nodata
			if ((b >= 0) && (b < bins)){  // (if non-undetectValue rstart)
				azimuth = atan2(pMetric.x, pMetric.y);  // notice x <=> y  in radars
				//a = (static_cast<int>(azimuth*RAD2J)+beams) % beams;
				a = static_cast<int>(azimuth * RAD2J);
				if (a < 0)
					a += beams;

				/*
				if (b == 50){ //<< '\t' << (a%beams)
					std::cerr << a << '\t' << (atan2(pMetric.x, pMetric.y)*M_PI/180.0) <<  '\n';
				}
				*/

				if (a < beams){

					s = srcData.data.get<double>(b,a);

					address = data.address(pComp.x, pComp.y);

					if (converter.SKIP_UNDETECT && (s == srcData.odim.undetect)){
						add(address, 0.0, 0.0); // weight=0.0 => only counter updated, important!
					}
					else {

						if (USE_QUALITY_FIELD){
							w = priorWeight * srcQuality.data.get<double>(b,a);
							if (converter.decode(s, w))
								add(address, s, w);
						}
						else {
							if (converter.decode(s))
								add(address, s, w);
								//add(address, s, converter.defaultQuality);
						}
					}
				}
			}
			// DEBUG, with tile background:
			// else add(data.address(i, j), 20.0, converter.defaultQuality);

		}
	}
		// TODO: interpolation (for INJECTION)


	drain::Rectangle<double> bboxD;
	//m2deg(bboxM.lowerLeft.x,  bboxM.lowerLeft.y,  bboxD.lowerLeft.x,  bboxD.lowerLeft.y);
	m2deg(bboxM.lowerLeft, bboxD.lowerLeft);
	//m2deg(bboxM.upperRight.x, bboxM.upperRight.y, bboxD.upperRight.x, bboxD.upperRight.y);
	m2deg(bboxM.upperRight, bboxD.upperRight);
	updateDataExtent(bboxD);

	//int i, j;
	drain::Point2D<double> cMetric;
	drain::Point2D<int> cImg;
	bboxM.getCenter(cMetric);
	//m2pix((bboxM.lowerLeft.x + bboxM.upperRight.x)/2.0,  (bboxM.lowerLeft.y+bboxM.upperRight.y)/2.0,  i,  j);
	m2pix(cMetric, cImg);
	updateNodeMap(SourceODIM(srcData.odim.source).getSourceCode(), cImg.x, cImg.y);

	odim.updateLenient(srcData.odim); // Time, date, new
	if (odim.NI == 0)
		odim.NI = srcData.odim.getNyquist();
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
	updateNodeMap(SourceODIM(cartSrc.odim.source).getSourceCode(), i0 + cartSrc.odim.geometry.width/2, j0 + cartSrc.odim.geometry.height/2);
	//updateGeoData();
	//mout.warn() << "nodemap keys: " << nodeMap << mout.endl;

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

	drain::Logger mout(__FUNCTION__, __FILE__);

	odim.updateGeoInfo(*this);

	// Produces ...,12568,12579,bymin,dkbor,dkrom,dksin,...
	odim.nodes = nodeMap.getKeys();

	// if (odim.source.empty()) { // nodeMap.size() > 1){ // consider AccNUM

	// ? this used to be commented
	if (nodeMap.size() > 1){ // consider AccNUM
		const drain::RegExp nodSyntax("^([a-z]{2})([a-z]{3}?)");
		if (nodSyntax.execute(odim.nodes) == 0){
			odim.source = "NOD:"+nodSyntax.result[1]+",ORG:"+nodSyntax.result[1];
		}
		else {
			mout.info() << "could not derive composite source NOD from nodes: " << odim.nodes << mout.endl;
		}
	}
	else {
		//odim.source = "xx";
	}

	odim.camethod = getMethod().name;


}




double Composite::getTimeDifferenceMinute(const CartesianODIM & odimIn) const {

	drain::Logger mout("Composite", __FUNCTION__);
	//mout.note(2) << "Tile time: " << tileTime.toStr("%Y/%m/%d %H:%M.%S") << mout.endl;

	drain::Time compositeTime;
	if (!odim.getTime(compositeTime)){
		mout.warn() << "time:" << odim.time << mout.endl;
		mout.warn() << "composite date/time=(" <<  odim.date << '/' << odim.time << ") not set, but requested" << mout.endl;
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


// Rack
