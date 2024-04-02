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
#include <drain/Log.h>
#include <drain/VariableLike.h>  // decayTime
#include "drain/util/Time.h"  // decayTime
//#include "drain/util/Variable.h"

#include "drain/image/ImageFile.h"
#include "drain/image/AccumulationMethods.h"
#include "drain/imageops/DistanceTransformFillOp.h"

#include "main/rack.h"  // for version toOStr

#include "data/DataCoder.h"
#include "data/SourceODIM.h"
#include "data/ODIMPathTools.h"

#include "product/ProductOp.h"

//#include "Geometry.h"
#include "Composite.h"
//#include "Coordinates.h"
//#include "RadarProj.h"

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


	//dataSelector.setPathMatcher(ODIMPathElem::DATA); // 2024/01
	//dataSelector.setQuantities(""); // "DBZH";
	dataSelector.setQuantityRegExp(""); // "DBZH";
	//dataSelector.setParameter("path", "data:");
	dataSelector.setMaxCount(1);
	// dataSelector.order = "ELANGLE:MIN";  // Should be separately for cart and polar data?

	//odim.link("type", odim.type = drain::Type::getTypeChar(typeid(void)));
	//odim.link("type", odim.type = "C");
	odim.link("type", odim.type = "C");

	odim.link("gain", odim.scaling.scale);
	odim.link("offset", odim.scaling.offset);
	odim.link("undetect", odim.undetect);
	odim.link("nodata", odim.nodata);

	odim.scaling.scale = 0.0;

	//static DataCoder converter;
	//setConverter(converter);

}

// With current settings, create simple "Polar volume" containing coordinates.
void Composite::createBinIndex(Hi5Tree & dst){ //const AreaGeometry & binGeometry){

	/// Automatically creates some metadata.
	drain::Logger mout(__FILE__, __FUNCTION__);

	DataSet<PolarDst> sweep(dst["dataset1"]);
	createBinIndex(dst, sweep.getFirstData().odim); // empty?

}

// With current settings, create simple "Polar volume" containing coordinates.
void Composite::createBinIndex(Hi5Tree & dst, const PolarODIM & odim){ //const AreaGeometry & binGeometry){

	/// Automatically creates some metadata.
	drain::Logger mout(__FILE__, __FUNCTION__);

	//dst.clear(); // ok?

	const std::type_info & t = typeid(unsigned int);
	// RootData<PolarDst> root(dst);

	{
		PolarODIM rootOdim;
		rootOdim.updateLenient(odim);
		rootOdim.object = "PVOL";
		rootOdim.type.clear();
		rootOdim.setTypeDefaults(t);
		//rootOdim.lat = 60.0;
		//rootOdim.lon = 25.0;
		rootOdim.setGeometry(500, 360);
		rootOdim.rscale = 500.0;
		rootOdim.scaling.set(1.0, 0.0);
		ODIM::updateH5AttributeGroups<ODIMPathElem::ROOT>(rootOdim, dst);

		//mout.attention("Root");
		//hi5::Hi5Base::writeText(dst, std::cerr);

		ODIMPathElem elem(ODIMPathElem::DATASET); // "dataset" + <n>
		mout.attention("Creating groups");
		ODIMPathTools::getNewChild(dst, elem);
		mout.attention("Creating ");
		DataSet<PolarDst> polar(dst[elem]);  //ODIMPathElem::DATASET

		mout.attention("Creating groups");
		Data<PolarDst> & beam = polar.getData("BIN_INDEX");
		beam.initialize(t, odim.getGeometry());

		for (size_t i=0; i<odim.getGeometry().getArea(); ++i){
			beam.data.put(i, i);
		}
		beam.odim.updateLenient(rootOdim);

	}
	// std::cerr << "\nUPON dst\n";
	// hi5::Hi5Base::writeText(dst["dataset1"], std::cerr);
	DataTools::updateInternalAttributes(dst);

}


void Composite::checkQuantity(const std::string & quantity){

	drain::Logger mout(__FILE__, __FUNCTION__);
	/// TODO: regexp for accepting quantities

	if (!this->odim.quantity.empty()){

		if (this->odim.quantity != quantity){
			mout.note("composite of quantity=" , this->odim.quantity , ", input with " , quantity );
			mout.experimental("Not replacing '" , this->odim.quantity , "' with '" , quantity , "'" );
		}

		if (odim.ACCnum > 1){
			// quantity.find
			//if (quantity.substr(0, 4) == "VRAD"){
			if (quantity.find("VRAD", 0, 4) == 0){
				mout.experimental("Revised VRAD check (TRUE)" );
				mout.warn("compositing VRAD directly, consider Doppler dealiasing (u,v) first" , quantity );
			}
			else {
				mout.debug("Revised VRAD check: FALSE" );
			}
		}

	}
	else // <- 2021/06/23
		this->odim.quantity = quantity;

	// mout.special("Setting quantity" , quantity );

	// omp critical?
	//this->odim.quantity = quantity;

}

void Composite::addPolarInnerLoop(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
		double priorWeight, const RadarProj & pRadarToComposite, const drain::Rectangle<int> & bboxPix) {

	/// -------------------------------------------------------
	/// DATA PROJECTION (MAIN LOOP)

	drain::Logger mout(__FILE__, __FUNCTION__);

	const bool USE_QUALITY_FIELD = (priorWeight > 0.0) && !srcQuality.data.isEmpty();

	mout.debug("projecting");
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
	//mout.warn("skip undetect:" , (int)SKIP_UNDETECT );

	DataCoder converter(srcData.odim, srcQuality.odim);



	double s;
	double w = priorWeight * converter.defaultQuality;

	drain::Point2D<int> pComp;
	drain::Point2D<double> pMetric;

	/*
	const int WIDTH = bboxPix.getWidth();
	std::vector<double> xLookUp(WIDTH);
	pComp.y = bboxPix.lowerLeft.y;
	for (pComp.x = bboxPix.lowerLeft.x; pComp.x<bboxPix.upperRight.x; ++pComp.x){
		pix2m(pComp, pMetric);
		xLookUp[pComp.x - bboxPix.lowerLeft.x] = pMetric.x;
	}
	*/

	size_t address;
	for (pComp.y = bboxPix.lowerLeft.y; pComp.y>bboxPix.upperRight.y; --pComp.y){ // notice negative



		// Beam index (azimuthal coordinate of polar input data)
		int a;

		// Bin index (radial coordinate of polar input data)
		int b;

		for (pComp.x = bboxPix.lowerLeft.x; pComp.x<bboxPix.upperRight.x; ++pComp.x){

			pix2m(pComp, pMetric);
			//pMetric.x = xLookUp[pComp.x - bboxPix.lowerLeft.x];
			/**
			if (((pComp.y%15)==0) && ((pComp.x % 15) ==0)){
				std::cerr << pMetric.x << ' ';
			};
			*/

			pRadarToComposite.projectInv(pMetric.x, pMetric.y);
			range = ::sqrt(pMetric.x*pMetric.x + pMetric.y*pMetric.y);
			b = srcData.odim.getBinIndex(range);

			//	std::cerr << " Pix (" << i << ',' << j << "),\t=>(" << x << ',' << y << "),\t range=" << range << ",\t bin=" << b << "\n";

			if ((b >= 0) && (b < bins)){  // (if non-undetectValue rstart)

				azimuth = atan2(pMetric.x, pMetric.y);  // notice x <=> y  in radars

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
		// if (((pComp.y % 15) ==0)) std::cerr << "#\n";
	}


}


void Composite::addPolar(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality, double priorWeight, bool projAEQD) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (drain::Logger::TIMING){
		SourceODIM source(srcData.odim.source);
		mout.startTiming(source.NOD);
	}

	//const DataSet<PolarSrc> konsta(srcData.getTree()["dataset1"]);  // TODO REMOVE XX

	if (!projGeo2Native.isSet()){
		mout.debug("projGeo2Native src: ", projGeo2Native.getProjectionSrc());
		mout.info("projGeo2Native dst: projection unset, using AEQD");
		projAEQD = true;
	}

	odim.object = "COMP";

	// Various checks

	checkQuantity(srcData.odim.quantity);

	checkCompositingMethod(srcData.odim);

	if (srcData.odim.rscale <= 0.0){
		mout.advice("Consider quick fix, like --/dataset1/where:rscale=500");
		mout.fail("Illegal or missing bin length (where:rscale): ", srcData.odim.rscale);
		return;
	}

	if (srcData.odim.area.width <= 0.0){
		mout.advice("Consider quick fix, like --/dataset1/where:nbins=", srcData.data.getWidth());
		mout.fail("Illegal or missing bin count (where:nbins): ", srcData.odim.area.width);
		srcData.odim.area.width = srcData.data.getWidth();
		mout.warn("Setting where:nbins=", srcData.odim.area.width);
	}

	if (srcData.odim.area.height <= 0.0){
		mout.advice("Consider quick fix, like --/dataset1/where:nrays=", srcData.data.getHeight());
		mout.fail("Illegal or missing beam count (where:nrays): ", srcData.odim.area.height);
		srcData.odim.area.height = srcData.data.getHeight();
		mout.warn("Setting where:nrays=", srcData.odim.area.height);
	}

	if (srcData.odim.lat == 0.0){
		mout.warn("Suspicious latitude (where:lat): 0.0");
	}

	if (srcData.odim.lon == 0.0){
		mout.warn("Suspicious longitude (where:long): 0.0");
	}

	ProductBase::applyODIM(this->odim, srcData.odim);


	//const bool USE_PRIOR_WEIGHT = (priorWeight > 0.0);

	const bool USE_QUALITY_FIELD = (priorWeight > 0.0) && !srcQuality.data.isEmpty(); // && (odim.quantity == "DBZH"); // quantity != QIND


	if (USE_QUALITY_FIELD) {
		mout.info(" using input q: " , EncodingODIM(srcQuality.odim) );
	}
	else if (priorWeight > 0.0) { // exclusive?
		mout.info(" using input weight=" , priorWeight );
		// TODO
		// mout.info() << "input quality exists=" << srcQuality.data.isEmpty() << ',';
	}
	else {
		mout.info(" quality weighting not applied" );
		if (srcQuality.data.isEmpty())
			mout.note(" (input quality would be available) " );
	}

	/// GEOGRAPHIC DEFINITIONS: USE THOSE OF THE MAIN COMPOSITE, OR USE AEQD FOR SINGLE RADAR
	RadarProj pRadarToComposite(srcData.odim.lon, srcData.odim.lat);
	// pRadarToComposite.setSiteLocationDeg(srcData.odim.lon, srcData.odim.lat);

	if (odim.source.empty())
		odim.source = srcData.odim.source; // for tile (single-radar "composite")

	if (! geometryIsSet()){
		setGeometry(500, 500);
		mout.info("Size not given, using default: ", this->getFrameWidth(), ',', this->getFrameHeight() );
	}

	mout.info("Info: \"", *this, '"');
	//mout.debug2("undetectValue=" , undetectValue );

	// Defined here, because later used for data update.
	//drain::Rectangle<double> bboxM;

	drain::Rectangle<double> bboxInput;


	if (projAEQD || !isDefined()){

		if (projAEQD){
			mout.info("Using default projection AEQD (azimuthal equidistant).");
			const std::string & aeqdStr = pRadarToComposite.getProjectionSrc();
			// mout.debug(aeqdStr );
			setProjection(aeqdStr);
		}

		pRadarToComposite.setProjectionDst(getProjection());

		double range = PolarODIM::defaultRange;
		if (range > 0.0){
			mout.info("Using predefined range: " , range );
			// pRadarToComposite.determineBoundingBoxM(PolarODIM::defaultRange, bboxM);
		}
		else {
			range = srcData.odim.getMaxRange(false);
			mout.info("Using maximum range: ", range);
			//pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange(true), bboxM);
		}

		mout.debug("Range:", range, " (max: ", srcData.odim.getMaxRange(true), ')');

		//drain::Rectangle<double> bboxNat;
		pRadarToComposite.determineBoundingBoxM(range, bboxInput);

		mout.accept("Detected 'native' input BBOX: ", bboxInput);

		setBoundingBoxM(bboxInput);

		// mout.note("Now this: " , *this );

	}
	else {
		mout.info("Using user-defined projection: ", getProjection());
		pRadarToComposite.setProjectionDst(getProjection());
		pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange(), bboxInput);

		if (cropping){
			//pRadarToComposite.determineBoundingBoxM(srcData.odim.getMaxRange() , bboxInput); // ALREADY?
			mout.debug("Orig: ", getBoundingBoxM());
			mout.debug("Cropping with ", srcData.odim.getMaxRange(), " range with bbox=", bboxInput );
			cropWithM(bboxInput);
			mout.info("Cropped to: ", getBoundingBoxM());
			if (getBoundingBoxM().getArea() == 0){
				mout.info("Cropping returned empty area." );
				mout.note("Data outside bounding box, returning" );
				allocate(); // ?
				updateGeoData();
				return;
			}
		}

	}
	/// Note: area not yet defined.

	// mout.warn("range: " , (srcData.odim.getMaxRange() / 1000.0) , " km ");
	if (!pRadarToComposite.isSet()){
		mout.error("source or dst projection is unset " );
		return;
	}

	if (mout.isDebug(1)){
		/// Check mapping for the origin (= location of the radar)?
		drain::Point2D<double> aeqd;
		drain::Point2D<double> dest;

		aeqd   = {0,0};
		pRadarToComposite.projectFwd(aeqd.x, aeqd.y, dest.x, dest.y);
		mout.special("Test origin", aeqd, "mapping: ", dest);

		aeqd   = {0, 100000};
		pRadarToComposite.projectFwd(aeqd.x, aeqd.y, dest.x, dest.y);
		mout.special("Test ", aeqd, " (100 km North) : ", dest);

		aeqd   = {-50000,0};
		pRadarToComposite.projectFwd(aeqd.x, aeqd.y, dest.x, dest.y);
		mout.special("Test ", aeqd, " (50 km West) : ", dest);
	}

	bboxInput.crop(getBoundingBoxM());

	drain::Rectangle<int> bboxPix;
	m2pix(bboxInput.lowerLeft,  bboxPix.lowerLeft);
	m2pix(bboxInput.upperRight, bboxPix.upperRight);
	mout.debug("cropped, data:", bboxInput, ", pix area: ", bboxPix);

	//mout.warn() << "Should use:" <<  bboxPix << ", in " << getFrameWidth() << 'x' << getFrameHeight() << '\n';
	mout.debug("Composite (cropped) ", *this, " geom: ", accArray.getGeometry(), "\nProj:\n", pRadarToComposite, "\n Pix area:\n", bboxPix, '\n');

	mout.debug("allocating");
	allocate();
	//mout.debug2("allocated" );
	//std::cerr << count << std::endl;

	/// -------------------------------------------------------
	/// DATA PROJECTION (MAIN LOOP)
	addPolarInnerLoop(srcData, srcQuality, priorWeight, pRadarToComposite, bboxPix);
	/// -------------------------------------------------------
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

	mout.debug("completed");

	// drain::image::File::write(data,"compD.png");
	// drain::image::File::write(weight,"compQ.png");


}


void Composite::addCartesian(const PlainData<CartesianSrc> & cartSrc, const PlainData<CartesianSrc> & srcQuality, double weight, int i0, int j0){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//if (drain::Logger::TIMING){
		// SourceODIM source(cartSrc.odim);
	mout.startTiming(cartSrc.odim.source);
	//}

	checkQuantity(cartSrc.odim.quantity);

	// EnsureEncoding
	// checkInputODIM(cartSrc.odim); // RadarAccumulator
	addData(cartSrc, srcQuality, weight, i0, j0);
	++odim.ACCnum;

	/// Cartesian
	updateNodeMap(SourceODIM(cartSrc.odim.source).getSourceCode(), i0 + cartSrc.odim.area.width/2, j0 + cartSrc.odim.area.height/2);
	//updateGeoData();
	//mout.warn("nodemap keys: " , nodeMap );

	// Update geographical extent (optional information)
	// const Rectangle<double> srcExtent(cartSrc.odim.LL_lon, cartSrc.odim.LL_lat, cartSrc.odim.UR_lon, cartSrc.odim.UR_lat);
	// updateDataExtent(srcExtent);
	updateDataExtent(cartSrc.odim.bboxD);

	// odim.update(cartSrc.odim); // moved to add Data

	mout.debug("completed");
}

void Composite::updateNodeMap(const std::string & node, int i, int j){
	drain::Variable & v = nodeMap[node];
	v.setType(typeid(int));
	v.setSeparator(':');
	v << i << j;
}

void Composite::updateGeoData(){

	drain::Logger mout(__FILE__, __FUNCTION__);

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
			mout.info("could not derive composite source NOD from nodes: " , odim.nodes );
		}
	}
	else {
		//odim.source = "xx";
	}

	odim.camethod = getMethod().getName(); //getNaMethod().name;


}

void Composite::updateInputSelector(const std::string & select){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!select.empty()){
		// mout.warn("Setting selector=" , resources.select );
		// const std::string quantityOrig(dataSelector.quantity);
		mout.experimental<LOG_DEBUG>("Changed code: quantityOrig => odim.quantity=", odim.quantity);
		const std::string quantityOrig(odim.quantity);

		//composite.dataSelector.setParameters(resources.baseCtx().select);
		dataSelector.setParameters(select);  // consume (clear)?

		mout.debug("Composite selector: ", select, " -> ", dataSelector);
		//resources.select = "quantity=" + composite.dataSelector.quantity;
		//resources.select.clear(); // PROBLEMS HERE?

		// TODO: selecor.quantity is allowed to be regExp?
		// TODO: what if one wants to add TH or DBZHC in a DBZH composite?
		//if (!quantityOrig.empty() && (quantityOrig != dataSelector.quantity)){
		if (!quantityOrig.empty() && !dataSelector.getQuantitySelector().testQuantity(quantityOrig)){
			mout.warn("quantityOrig=", quantityOrig, " !~ ", dataSelector.getQuantity());
			mout.warn("quantity selector changed, resetting accumulation array" );
			accArray.clear();
			odim.quantity.clear();
			odim.scaling.set(0.0, 0.0);
		}
	}
	else {
		if (!dataSelector.quantityIsSet()){
			mout.info("Setting selector quantity=" , odim.quantity );
			dataSelector.setQuantities(odim.quantity);
			//dataSelector.quantity = odim.quantity; // consider "^"+...+"$"
			//
		}
	}
}



double Composite::getTimeDifferenceMinute(const CartesianODIM & odimIn) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	//mout.note(2) << "Tile time: " << tileTime.toStr("%Y/%m/%d %H:%M.%S") << mout.endl;

	drain::Time compositeTime;
	if (!odim.getTime(compositeTime)){
		mout.warn("time:", odim.time);
		mout.warn("composite date/time=(", odim.date, '/', odim.time, ") not set, but requested");
		return 0.0;
	}
	//compositeTime.setTime(odim.date, "%Y%m%d");
	//compositeTime.setTime(odim.time, "%H%M%S");

	drain::Time tileTime;
	if (!odimIn.getTime(tileTime)){
		mout.warn("tile date/time not set, but requested");
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
