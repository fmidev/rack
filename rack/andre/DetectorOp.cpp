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

// DEBUGGING
#include <drain/image/File.h>
#include <drain/util/Debug.h>

#include <drain/image/MarginalStatisticOp.h>
#include <drain/image/BasicOps.h>
#include <drain/image/DistanceTransformOp.h>
#include <drain/image/GammaOp.h>

//#include "main/rack.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"

//#include "radar/DataCopyOp.h"
#include "product/DataConversionOp.h"

#include "QualityCombinerOp.h"
#include "DetectorOp.h"


using namespace drain::image;

namespace rack {

bool DetectorOp::STORE(false);

//DetectorOp::Cumulation DetectorOp::cumulateDetections = DetectorOp::MAX;
//std::string DetectorOp::dstGroupPrefix = "quality";

bool DetectorOp::SUPPORT_UNIVERSAL(true);

unsigned short int DetectorOp::_count(0);



void DetectorOp::processDataSets(const DataSetSrcMap & srcDataSets, DataSetDstMap<PolarDst> & dstDataSets) const {

	drain::MonitorSource mout(name+"(DetectorOp)", __FUNCTION__);

	mout.debug(1) << "start1" << mout.endl;

	DataSetSrcMap::const_iterator its = srcDataSets.begin();
	DataSetDstMap<>::iterator     itd = dstDataSets.begin();
	while (its != srcDataSets.end()){
		mout.info() << "processing elangle:" << its->first << mout.endl;
		if (its->first == itd->first){

			const DataSetSrc<> & srcDataSet = its->second;
			DataSetDst<> & dstDataSet = itd->second;

			const Data<PolarSrc> & srcData = srcDataSet.getFirstData();

			Data<PolarDst>  & dstData = dstDataSet.getFirstData(); // only for QIND and CLASS

			/// TODO: UNIVERSAL and several inputs?
			// OVERALL QUALITY (PROB.)
			PlainData<PolarDst> & dstQind = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData("QIND") : dstData.getQualityData("QIND"); // of first data (eg. TH)
			initDataDst(srcData, dstQind, "QIND");

			// OVERALL QUALITY FIELD
			PlainData<PolarDst> & dstClass = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData("CLASS") : dstData.getQualityData("CLASS"); // TODO: local (not only univ.)
			initDataDst(srcData, dstClass, "CLASS");

			// PROBABILITY OF THE CLASS APPROXIMATED BY THIS DETECTOR
			const std::string & CLASSNAME = getQuantityName();
			//const std::string & KEYNAME = DetectorOp::STORE ? CLASSNAME : std::string("~")+CLASSNAME;

			// add tmp here
			PlainData<PolarDst> & dstProb = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData(CLASSNAME) : dstData.getQualityData(CLASSNAME);
			dstProb.tree.data.noSave = !DetectorOp::STORE;
			initDataDst(srcData, dstProb);

			/// MAIN COMMAND
			processDataSet(srcDataSet, dstProb,  dstDataSet);
			dstProb.updateTree(); // create /what, /where etc.
			DataSelector::updateAttributes(dstProb.tree); // collect attributes from /what, /where to /data:data properties so that srcData.getQualityData() works below.
			// update other trees?

			/*
			const PlainData<PolarSrc> & srcProb = (SUPPORT_UNIVERSAL && UNIVERSAL) ? srcDataSet.getQualityData(CLASSNAME) : srcData.getQualityData(CLASSNAME); // slows, due to src/dst const
			 */
			const PlainData<PolarSrc> srcProb(dstProb);

			if (srcProb.data.isEmpty()){
				mout.error() << "empty srcProb," << CLASSNAME << " univ=" << (int)(SUPPORT_UNIVERSAL && UNIVERSAL) << mout.endl;
			}

			/*
			File::write(srcProb.data, "srcProb.png");
			if (!dstQind.data.isEmpty())
				File::write(dstQind.data, "dstQind1.png");
			if (!dstClass.data.isEmpty())
				File::write(dstClass.data, "dstClass1.png");
			 */
			QualityCombinerOp::updateOverallDetection(srcProb, dstQind, dstClass, CLASSNAME, classCode);
			//File::write(dstQind.data, "dstQind2.png");
			//File::write(dstClass.data, "dstClass2.png");

			// Ensure palette
			//HI5TREE & palette = hi5::Hi5Base::getPalette(dstDataSet.tree);
			//hi5::Hi5Base::linkPalette(palette, dstClass.tree);

		}
		else {
			mout.warn() << "something went wrong, dst has different angle, dst=" << itd->first << " != " << its->first << mout.endl;
			return;
		}
		++its;
		++itd;
	}

	mout.debug(1) << "end" << mout.endl;

}

void DetectorOp::processDataSet(const DataSetSrc<PolarSrc> & srcDataSet, PlainData<PolarDst> & dstProb, DataSetDst<PolarDst> & aux) const {

	drain::MonitorSource mout(name+"(DetectorOp)", __FUNCTION__);

	if (srcDataSet.size() == 0){
		mout.warn() << "dataset contains no data, skipping" << mout.endl;
		return;
	}
	else {
		const PlainData<PolarSrc> & srcData = srcDataSet.getFirstData();
		/*
		if (srcDataSet.size() > 1){
			mout.warn() << "single-input detector, but dataset contains several possible quantities, using " << srcData.odim.quantity << mout.endl;
			processData(srcData, dstProb);
		}
		else {
		 */
		if (REQUIRE_STANDARD_DATA){
			mout.debug() << "requires standardized srcData, ok" << mout.endl;
			//aux.getNormalizedData(srcData.odim.quantity);
			//const PlainData<PolarSrc> & srcDataNrm =
			const HI5TREE & tree = DataConversionOp::getNormalizedData(srcDataSet, aux,  srcData.odim.quantity); // srcDataSet.getData(srcData.odim.quantity+'~');
			//mout.warn() << tree << mout.endl;
			const PlainData<PolarSrc> srcDataNrm(tree);
			processData(srcDataNrm, dstProb);
		}
		else
			processData(srcData, dstProb);

		writeHow(dstProb);
		//}
	}

}


/// If raised, make template? Cf. Volume::initDst ?
void DetectorOp::initDataDst(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, const std::string & quantity) const {
	if (dstData.data.isEmpty()){
		//dstData.odim.set(srcData.odim); // nbins, nrays, rscale
		//const std::type_info & t = typeid(unsigned char);
		//const drain::Type t(typeid(unsigned char));
		//dstData.setType("C");
		//dstData.odim.type = t; // drain::Type::getTypeChar(t);
		//if (quantity == getQuantityName()){ // BIOMET, EMITTER, SHIP, etc.
		if (quantity.empty()){ // BIOMET, EMITTER, SHIP, etc.
			//dstData.odim.setQuantityDefaults("PROB");
			dstData.odim.quantity = getQuantityName();
			getQuantityMap().setQuantityDefaults(dstData, "PROB");
		}
		else
			getQuantityMap().setQuantityDefaults(dstData, quantity);

		dstData.data.setGeometry(srcData.data.getGeometry());
		// dstData.odim.setQuantityDefaults(quantity);
		dstData.odim.nbins  = srcData.odim.nbins; // nbins, nrays, rscale
		dstData.odim.nrays  = srcData.odim.nrays; // nbins, nrays, rscale
		dstData.odim.rscale = srcData.odim.rscale; // nbins, nrays, rscale
		if (quantity == "QIND")
			dstData.data.fill(dstData.odim.scaleInverse(1.0)); // max quality (250) by default.
		//dstData.data.fill(250);
		dstData.updateTree(); // conditional?
	}
}

//const PlainData<PolarSrc> & DetectorOp::getNormalizedData(const DataSetSrc<> & srcDataSet, DataSetDst<> & dstDataSet, const std::string & quantity) const {

/*
const PolarODIM & DetectorOp::getStandardODIM(const std::string & quantity) const {

	drain::MonitorSource mout(name+"(DetectorOp)", __FUNCTION__);

	static
	//mutable
	std::map<std::string,PolarODIM> ODIMregistry;

	std::map<std::string,PolarODIM>::const_iterator it = ODIMregistry.find(quantity);
	if (it != ODIMregistry.end()){
		mout.debug(4) << "using cached quantity:" << quantity << mout.endl;
		return it->second;
	}
	else {
		mout.debug(4) << "creating cached entry for quantity:" << quantity << mout.endl;
		PolarODIM & result = ODIMregistry[quantity]; // create
		result.setQuantityDefaults(quantity);  // TODO: who "owns" the standard spex? ODIM?

		return result;
	}
}
 */

const std::string & DetectorOp::getQuantityName() const {

	// If unset, copy in uppercase letters.
	if (upperCaseName.empty()) {
		upperCaseName = name;
		for (std::string::iterator it = upperCaseName.begin(); it!=upperCaseName.end(); ++it){
			if ((*it >= 'a') && (*it <= 'z'))
				*it = *it + ('A' - 'a');
		}
		/*
		upperCaseName.resize(name.size());
		std::string::iterator it = upperCaseName.begin();
		std::string::const_iterator nit = name.begin();
		while (it != upperCaseName.end()){
			if ((*nit >= 'a') && (*nit <= 'z'))
		 *it = *nit + ('A' - 'a');
			else
		 *it = *nit;
			++it;
			++nit;
		}
		 */
	}

	return upperCaseName;

}

void DetectorOp::storeDebugData(const Image & srcImage, PlainData<PolarDst> & dstData, const std::string & quantityLabel) const {

	DataSetDst<PolarDst> dstDataSet(dstData.tree);

	PlainData<PolarDst> & dstDebugData = dstDataSet.getQualityData(quantityLabel);

	// Copy ?
	UnaryFunctorOp<ScalingFunctor>().filter(srcImage, dstDebugData.data);
	//ScaleOp().filter(srcImage, dstDebugData.data);

	dstDebugData.odim.quantity = quantityLabel;
	dstDebugData.updateTree();

}

void DetectorOp::storeDebugData(int debugLevel, const Image & srcImage, const std::string & label) const {

	static int counter=0;

	drain::MonitorSource mout(name, __FUNCTION__);

	if (mout.isDebug(debugLevel)){
		std::stringstream sstr;
		sstr.width(2);
		sstr.fill('0');
		sstr << name << '-' << counter << '-' << label << ".png";
		const std::string filename = sstr.str();
		mout.debug() << "writing " << filename << mout.endl;
		File::write(srcImage, filename);
		counter++;
	}




}


void DetectorOp::writeHow(PlainData<PolarDst> & dstData) const {
	drain::VariableMap & a = dstData.tree["how"].data.attributes;
	a["task"] = std::string("fi.fmi.")+__RACK__+".AnDRe."+name+':'+getParameters().getKeys();
	a["task_args"] = getParameters().getValues();
}



void DetectorOp::_enhanceDirectionally(Image & dst, float medianPos, int width) const {

	drain::MonitorSource mout(drain::monitor,"AndreOp::_enhance");
	mout.debug() << " called by " << name << mout.endl;
	if (mout.isDebug(12))
		File::write(dst,"andre-enh0-src.png");

	Image tmp;

	MarginalStatisticOp statOp;
	statOp.mode = "horz";
	statOp.stat = "m"; // median
	statOp.medianPos = medianPos;
	mout.debug(10) << statOp << mout.endl;
	statOp.filter(dst, tmp);
	if (mout.isDebug(20))
		File::write(tmp,"andre-enh1-stat.png");

	GammaOp gammaOp;
	gammaOp.gamma = 4.0;
	mout.debug(10) << gammaOp << mout.endl;
	gammaOp.filter(tmp,tmp);
	if (mout.isDebug(20))
		File::write(tmp,"andre-enh2-gamma.png");


	DistanceTransformExponentialOp dtfOp(1,width);
	mout.debug(10) << dtfOp << mout.endl;
	dtfOp.filter(tmp, tmp);
	if (mout.isDebug(20))
		File::write(tmp,"andre-enh3-dtf.png");

	// MultiplicationOp
	BinaryFunctorOp<MultiplicationFunctor> mulOp;
	mout.debug(10) << mulOp << mout.endl;
	mulOp.filter(tmp,dst);
	if (mout.isDebug(22))
		File::write(dst,"andre-enh4-dst.png");


}

///
void DetectorOp::_infect(Image & dst, int windowWidth, int windowHeight, double enhancement) const {

	drain::MonitorSource mout(drain::monitor,"AndreOp::_infect");

	DistanceTransformExponentialOp distOp(windowWidth, windowHeight);

	Image tmp;
	mout.debug(5) << distOp << mout.endl;
	distOp.filter(dst, tmp);
	if (mout.isDebug(10))
		File::write(tmp,"andre-infect-1dist.png");

	BinaryFunctorOp<MixerFunctor> op;
	op.functor.coeff = enhancement;
	op.filter(tmp,dst);
	// MixerOp(enhancement).filter(tmp,dst);

	if (mout.isDebug(10))
		File::write(dst, "andre-infect-2mix.png");

}

}  // namespace rack

// Rack
