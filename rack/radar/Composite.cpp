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
#include "drain/util/Log.h"
#include "drain/util/Time.h"  // decayTime
#include "drain/util/Variable.h"

#include "drain/image/AccumulationMethods.h"
#include "drain/imageops/DistanceTransformFillOp.h"

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

	//odim.link("type", odim.type = drain::Type::getTypeChar(typeid(void)));
	//odim.link("type", odim.type = "C");
	odim.link("type", odim.type = "C");

	odim.link("gain", odim.scaling.scale);
	odim.link("offset", odim.scaling.offset);
	odim.link("undetect", odim.undetect);
	odim.link("nodata", odim.nodata);

	odim.scaling.scale = 0.0;

	dataSelector.count = 1;
	// dataSelector.order = "ELANGLE:MIN";  // Should be separately for cart and polar data?
	//static DataCoder converter;
	//setConverter(converter);

}

void Composite::checkQuantity(const std::string & quantity){

	drain::Logger mout("Composite", __FUNCTION__);
	/// TODO: regexp for accepting quantities

	if (!this->odim.quantity.empty()){

		if (this->odim.quantity != quantity){
			mout.note() << "composite of quantity=" << this->odim.quantity << ", input with " << quantity << mout;
			mout.experimental() << "Not replacing '" << this->odim.quantity << "' with '" << quantity << "'" << mout;
		}

		if (odim.ACCnum > 1){
			// quantity.find
			//if (quantity.substr(0, 4) == "VRAD"){
			if (quantity.find("VRAD", 0, 4) == 0){
				mout.experimental() << "Revised VRAD check (TRUE)" << mout;
				mout.warn() << "compositing VRAD directly, consider Doppler dealiasing (u,v) first" << quantity << mout;
			}
			else {
				mout.debug() << "Revised VRAD check: FALSE" << mout;
			}
		}

	}
	else // <- 2021/06/23
		this->odim.quantity = quantity;

	// mout.special() << "Setting quantity" << quantity << mout.endl;

	// omp critical?
	//this->odim.quantity = quantity;

}

void Composite::addPolar(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality, double priorWeight, bool projAEQD) {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const DataSet<PolarSrc> konsta(srcData.getTree()["dataset1"]);  // TODO REMOVE XX

	if (!projR2M.isSet())
		projAEQD = true;

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

	drain::Rectangle<double> bboxInput;

	if (odim.source.empty())
		odim.source = srcData.odim.source; // for tile (single-radar "composite")

	if (! geometryIsSet()){
		setGeometry(500, 500);
		mout.info() << "Size not given, using default: " << this->getFrameWidth() << ',' << this->getFrameHeight() << mout.endl;
	}

	mout.debug2() << "Info: \"" << *this << '"' << mout.endl;
	//mout.debug2() << "undetectValue=" << undetectValue << mout.endl;

	// Defined here, because later used for data update.
	//drain::Rectangle<double> bboxM;


	if (projAEQD || !isDefined()){

		if (projAEQD){
			mout.info() << "Using default projection aeqd (azimuthal equidistant)." << mout.endl;
			const std::string & aeqdStr = pRadarToComposite.getProjectionSrc();
			// mout.debug() << aeqdStr << mout.endl;
			setProjection(aeqdStr);
		}

		pRadarToComposite.setProjectionDst(getProjection());

		double range = PolarODIM::defaultRange;
		if (range > 0.0){
			mout.info() << "Using predefined range: " << range << mout.endl;
			// pRadarToComposite.determineBoundingBoxM(PolarODIM::defaultRange, bboxM);
		}
		else {
			range = srcData.odim.getMaxRange(false);
			mout.info() << "Using maximum range: " << range << mout.endl;
			//pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange(true), bboxM);
		}

		mout.debug() << "Range:" << range << " (max: "<< srcData.odim.getMaxRange(true) << ')' << mout.endl;

		//drain::Rectangle<double> bboxNat;
		pRadarToComposite.determineBoundingBoxM(range, bboxInput);

		mout.debug() << "Detected 'native' bbox " << bboxInput << mout.endl;

		setBoundingBoxM(bboxInput);

		// mout.note() << "Now this: " << *this << mout.endl;

	}
	else {
		mout.info() << "Using user-defined projection: " << getProjection() << mout.endl;
		pRadarToComposite.setProjectionDst(getProjection());
		pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange() , bboxInput);

		if (cropping){
			//pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange() , bboxInput); // ALREADY?
			mout.debug() << "Orig: " << getBoundingBoxM() << mout.endl;
			mout.debug() << "Cropping with " << srcData.odim.getMaxRange() << " range with bbox=" << bboxInput << mout.endl;
			cropWithM(bboxInput);
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
		return;
	}

	if (mout.isDebug(2)){
		/// Check mapping for the origin (= location of the radar)?
		double _x,_y;
		pRadarToComposite.projectFwd(0.0, 0.0, _x,_y);
		mout.debug3() << "Test origin mapping: " << _x << ' ' << _y << mout.endl;
	}


	/// Limit to data extent
	//drain::Rectangle<double> bboxNatCommon(bboxInput);
	//drain::Rectangle<double> & bboxNatCommon = bboxInput;
	//mout.warn() << "input bbox:" <<  bboxNatCommon << mout.endl;

	/// Area for main loop
	/*
	if (pRadarToComposite.isLongLat()){
		mout.warn() << "cropping with bboxR (radians):" <<  getBoundingBoxR()  << mout.endl;
		bboxNatCommon.crop(getBoundingBoxR());
		//deg2pix(bboxM.lowerLeft,  bboxPix.lowerLeft);
		//deg2pix(bboxM.upperRight, bboxPix.upperRight);
	}
	else {
		mout.warn() << "cropping with bboxM (metric): " <<  getBoundingBoxM()  << mout.endl;
		bboxNatCommon.crop(getBoundingBoxM());
	}*/
	bboxInput.crop(getBoundingBoxM());

	drain::Rectangle<int> bboxPix;
	m2pix(bboxInput.lowerLeft,  bboxPix.lowerLeft);
	m2pix(bboxInput.upperRight, bboxPix.upperRight);
	mout.debug() << "cropped, data:" << bboxInput << ", pix area: " << bboxPix << mout.endl;


	//mout.warn() << "Should use:" <<  bboxPix << ", in " << getFrameWidth() << 'x' << getFrameHeight() << '\n';

	//if (drain::Debug > 4){
	mout.debug();
	//mout << "Composite" << composite << '\n';
	mout << "Composite (cropped) " << *this;
	mout << " geom: " << accArray.getWidth() << 'x' << accArray.getHeight() << '\n';
	mout << "Proj:\n" << pRadarToComposite << '\n';
	mout << "Pix area:\n" << bboxPix << '\n';
	mout << mout.endl;

	mout.debug() << "allocating" << mout.endl;
	allocate();
	//mout.debug2() << "allocated" << mout.endl;
	//std::cerr << count << std::endl;

	/// -------------------------------------------------------
	/// DATA PROEJCTION (MAIN LOOP)
	mout.debug() << "projecting" << mout.endl;
	const int bins  = srcData.data.getWidth();  // TODO odimize
	const int beams = srcData.data.getHeight(); // TODO odimize
	const float RAD2J = 1.0/srcData.odim.getBeamWidth();
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

		for (pComp.x = bboxPix.lowerLeft.x; pComp.x<bboxPix.upperRight.x; ++pComp.x){

			pix2m(pComp, pMetric);
			pRadarToComposite.projectInv(pMetric.x, pMetric.y);
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

					address = accArray.data.address(pComp.x, pComp.y);

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
	m2deg(bboxInput.lowerLeft,  bboxD.lowerLeft);
	m2deg(bboxInput.upperRight, bboxD.upperRight);
	updateDataExtent(bboxD);

	// Non-standard: add position of radar in image coords
	drain::Point2D<double> cMetric;
	drain::Point2D<int> cImg;
	bboxInput.getCenter(cMetric);
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
	updateNodeMap(SourceODIM(cartSrc.odim.source).getSourceCode(), i0 + cartSrc.odim.area.width/2, j0 + cartSrc.odim.area.height/2);
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

	odim.camethod = getMethod().getName(); //getNaMethod().name;


}

void Composite::updateInputSelector(const std::string & select){

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (!select.empty()){
		// mout.warn() << "Setting selector=" << resources.select << mout.endl;
		const std::string quantityOrig(dataSelector.quantity);

		//composite.dataSelector.setParameters(resources.baseCtx().select);
		dataSelector.setParameters(select);  // consume (clear)?

		mout.debug("Applied: ", select, " -> ", dataSelector);
		//resources.select = "quantity=" + composite.dataSelector.quantity;
		//resources.select.clear(); // PROBLEMS HERE?

		// TODO: selecor.quantity is allowed to be regExp?
		// TODO: what if one wants to add TH or DBZHC in a DBZH composite?
		if (!quantityOrig.empty() && (quantityOrig != dataSelector.quantity)){
			mout.warn() << "quantity selector changed, resetting accumulation array" << mout;
			accArray.clear();
			odim.quantity.clear();
			odim.scaling.set(0.0, 0.0);
		}
	}
	else {
		if (dataSelector.quantity.empty()){
			mout.info() << "Setting selector quantity=" << odim.quantity << mout;
			dataSelector.quantity = odim.quantity; // consider "^"+...+"$"
			//
		}
	}
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
