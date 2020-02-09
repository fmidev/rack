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
#include <drain/util/Log.h>
#include <drain/util/FunctorPack.h>

#include <drain/imageops/DistanceTransformOp.h>
#include <drain/imageops/FunctorOp.h>
#include <drain/imageops/GammaOp.h>
#include <drain/imageops/MarginalStatisticOp.h>

//#include "main/rack.h"
#include "hi5/Hi5.h"
//#include "hi5/Hi5Write.h"

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



void DetectorOp::processDataSets(const DataSetMap<PolarSrc> & srcDataSets, DataSetMap<PolarDst> & dstDataSets) const {

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL name+"(DetectorOp)", __FUNCTION__);

	const std::string & CLASSNAME = getOutputQuantity();

	mout.debug(1) << "start1" << mout.endl;

	DataSetMap<PolarSrc>::const_iterator its = srcDataSets.begin();
	DataSetMap<PolarDst>::iterator       itd = dstDataSets.begin();
	while (its != srcDataSets.end()){

		mout.info() << "processing elangle:" << its->first << mout.endl;

		if (its->first == itd->first){

			const DataSet<PolarSrc> & srcDataSet = its->second;
			DataSet<PolarDst> & dstDataSet = itd->second;

			const Data<PolarSrc> & srcData = srcDataSet.getFirstData();
			if (srcData.data.isEmpty()){
				mout.warn() << "empty srcData [" << srcDataSet.begin()->first << "]: " << srcData << mout.endl;
				return;
			}
			else if (!srcData.data.typeIsSet()){
				mout.warn() << "unset type in srcData [" << srcDataSet.begin()->first << "]: " << srcData << mout.endl;
				return;
			}
			if ((srcData.odim.nbins==0) || (srcData.odim.nrays==0)){
				mout.warn() << "empty geom in odim of srcData [" << srcDataSet.begin()->first << "]: " << srcData.odim << mout.endl;
				return;
			}

			Data<PolarDst>  & dstData = dstDataSet.getFirstData(); // only for QIND and CLASS


			mout.debug() << "CLASSNAME=" << CLASSNAME << " using universal=" << (SUPPORT_UNIVERSAL && UNIVERSAL) << mout.endl;

			const std::string QIND = "QIND"; // (SUPPORT_UNIVERSAL && UNIVERSAL)? "QIND" : "qind";

			/// TODO: UNIVERSAL and several inputs?
			// OVERALL QUALITY (PROB.)
			PlainData<PolarDst> & dstQind = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData(QIND) : dstData.getQualityData(QIND); // of first data (eg. TH)
			initDataDst(srcData, dstQind, "QIND");

			// OVERALL QUALITY FIELD
			PlainData<PolarDst> & dstClass = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData("CLASS") : dstData.getQualityData("CLASS"); // TODO: local (not only univ.)
			initDataDst(srcData, dstClass, "CLASS");

			// PROBABILITY OF THE CLASS APPROXIMATED BY THIS DETECTOR
			PlainData<PolarDst> & dstProb = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData(CLASSNAME) : dstData.getQualityData(CLASSNAME);
			//dstProb.tree.data.noSave = !DetectorOp::STORE;
			initDataDst(srcData, dstProb);
			dstProb.setNoSave(DetectorOp::STORE == 0);

			mout.debug() << "dstProb: " << dstProb << mout.endl;

			/// MAIN COMMAND
			if (DetectorOp::STORE >= 0)
				processDataSet(srcDataSet, dstProb,  dstDataSet);
			// else skip! To collect legends.

			//@ dstProb.updateTree(); // create /what, /where etc.
			//@ DataTools::updateInternalAttributes(dstProb.tree); // collect attributes from /what, /where to /data:data properties so that srcData.getQualityData() works below.
			// update str trees?

			/*
			const PlainData<PolarSrc> & srcProb = (SUPPORT_UNIVERSAL && UNIVERSAL) ? srcDataSet.getQualityData(CLASSNAME) : srcData.getQualityData(CLASSNAME); // slows, due to src/dst const
			 */
			const PlainData<PolarSrc> srcProb(dstProb);

			if (srcProb.data.isEmpty()){
				//mout.warn() << "srcProb" << srcProb << mout.endl;
				//mout.note() << "dstProb" << dstProb << mout.endl;
				mout.warn() << "empty srcProb," << CLASSNAME << " univ=" << (int)(SUPPORT_UNIVERSAL && UNIVERSAL) << mout.endl;
				return;
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

void DetectorOp::processDataSet(const DataSet<PolarSrc> & srcDataSet, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & cache) const {

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL name+"(DetectorOp)", __FUNCTION__);

	mout.info() << "start" << mout.endl;
	//mout.warn() << "start" << mout.endl;

	if (srcDataSet.size() == 0){
		mout.warn() << "dataset contains no data, skipping" << mout.endl;
		return;
	}
	else {

		const PlainData<PolarSrc> & srcData = srcDataSet.getFirstData();

		if (REQUIRE_STANDARD_DATA){
			//mout.note() << "dstProb (target): " << dstProb << mout.endl;
			mout.debug() << "requires normalized srcData" << mout.endl;
			//const Hi5Tree & tree = DataConversionOp<PolarODIM>::getNormalizedData(srcDataSet, cache,  srcData.odim.quantity); // srcDataSet.getData(srcData.odim.quantity+'~');
			//const PlainData<PolarSrc> srcDataNrm(tree);
			//const PlainData<PolarSrc> & srcDataNorm = dstProb; // kokkeilu

		    const QuantityMap & qmap = getQuantityMap();
		    if (qmap.isNormalized(srcData.odim)){
				mout.debug() << "srcData is normalized already, ok" << mout.endl;
		    	processData(srcData, dstProb);
		    }
		    else {
		    	mout.debug() << "requesting normalized data..." << mout.endl;
		    	PlainData<PolarDst> & srcDataNorm = DataConversionOp<PolarODIM>::getNormalizedData(srcDataSet, cache,  srcData.odim.quantity);
				//mout.debug(2) << "got normalized data, now processing" << mout.endl;
				mout.debug(1) << "srcDataNorm: " << srcDataNorm << mout.endl;
				processData(srcDataNorm, dstProb);
		    }
			mout.debug(1) << "dstProb (result): " << dstProb << " OK?" << mout.endl;
		}
		else {
			mout.debug() << "no data normalization needed, ok" << mout.endl;
			processData(srcData, dstProb);
		}

		writeHow(dstProb);
		//dstProb.getTree()["what"].data.attributes["mika"] = 123.456;
		//}
	}

	//mout.warn() << "end" << dstProb.odim.quantity << mout.endl;

}


/// If raised, make template? Cf. Volume::initDst ?
void DetectorOp::initDataDst(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, const std::string & quantity) const {

	drain::Logger mout(__FUNCTION__, getName() + "(DetectorOp)");

	if (dstData.data.isEmpty()){

		if (quantity.empty()){
			//mout.warn() << "requested: " << quantity << mout.endl;
			dstData.odim.quantity = getOutputQuantity();
			QualityCombinerOp::initDstQuality(srcData, dstData, "PROB");
		}
		else {
			dstData.odim.quantity = quantity;
			QualityCombinerOp::initDstQuality(srcData, dstData, quantity);
		}

	}
	else {
		mout.debug() << "already initialized: " << EncodingODIM(dstData.odim) << mout.endl;
		mout.debug(1) << dstData << mout.endl;
	}
}



const std::string & DetectorOp::getOutputQuantity() const {

	// If unset, copy in uppercase letters.
	if (upperCaseName.empty()) {
		//size_t i = name.find_last_of("Op");
		upperCaseName = name; //.substr(0, i);
		drain::StringTools::upperCase(upperCaseName);
	}

	return upperCaseName;

}

void DetectorOp::storeDebugData(const ImageFrame & srcImage, PlainData<PolarDst> & dstData, const std::string & quantityLabel) const {

	DataSet<PolarDst> dstDataSet(dstData.getTree());

	PlainData<PolarDst> & dstDebugData = dstDataSet.getQualityData(quantityLabel);

	// Copy ?
	UnaryFunctorOp<ScalingFunctor>().process(srcImage, dstDebugData.data);
	//ScaleOp().filter(srcImage, dstDebugData.data);

	dstDebugData.odim.quantity = quantityLabel;
	// dstDebugData.updateTree();

}

void DetectorOp::storeDebugData(int debugLevel, const ImageFrame & srcImage, const std::string & label) const {

	static int counter=0;

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (mout.isDebug(debugLevel)){
		std::stringstream sstr;
		sstr << name << '-';
		sstr.width(2);
		sstr.fill('0');
		sstr << counter << '-' << label << ".png";
		const std::string filename = sstr.str();
		mout.debug() << "writing " << filename << mout.endl;
		File::write(srcImage, filename);
		counter++;
	}

}


void DetectorOp::writeHow(PlainData<PolarDst> & dstData) const {

	drain::VariableMap & a = dstData.getHow();
	a["task"] = std::string("fi.fmi.Rack.AnDRe.")+name;
	a["task_args"] = getParameters().toStr(':');

}



void DetectorOp::_enhanceDirectionally(Image & dst, float medianPos, int width) const {

	drain::Logger mout(drain::getLog(), __FUNCTION__, getName());
	mout.debug() << " called by " << name << mout.endl;
	if (mout.isDebug(12))
		File::write(dst,"andre-enh0-src.png");

	Image tmp;

	MarginalStatisticOp statOp;
	statOp.mode = "horz";
	statOp.stat = "m"; // median
	statOp.medianPos = medianPos;
	mout.debug(10) << statOp << mout.endl;
	statOp.process(dst, tmp);
	if (mout.isDebug(20))
		File::write(tmp,"andre-enh1-stat.png");

	//GammaOp gammaOp;
	drain::image::UnaryFunctorOp<drain::image::GammaFunctor> gammaOp;
	gammaOp.functor.gamma = 4.0;
	mout.debug(10) << gammaOp << mout.endl;
	gammaOp.traverseChannel(tmp.getChannel(0), tmp.getChannel(0));
	if (mout.isDebug(20))
		File::write(tmp,"andre-enh2-gamma.png");


	DistanceTransformExponentialOp dtfOp(1,width);
	mout.debug(10) << dtfOp << mout.endl;
	dtfOp.traverseChannel(tmp.getChannel(0), tmp.getChannel(0));
	if (mout.isDebug(20))
		File::write(tmp,"andre-enh3-dtf.png");

	// MultiplicationOp
	BinaryFunctorOp<MultiplicationFunctor> mulOp;
	mout.debug(10) << mulOp << mout.endl;
	mulOp.traverseChannel(tmp.getChannel(0), dst.getChannel(0));
	if (mout.isDebug(22))
		File::write(dst,"andre-enh4-dst.png");


}

///
void DetectorOp::_infect(Image & dst, int windowWidth, int windowHeight, double enhancement) const {

	drain::Logger mout(drain::getLog(), __FUNCTION__, getName());

	DistanceTransformExponentialOp distOp(windowWidth, windowHeight);

	drain::image::Image tmp;
	mout.debug(5) << distOp << mout.endl;
	distOp.process(dst, tmp);
	if (mout.isDebug(10))
		File::write(tmp,"andre-infect-1dist.png");

	BinaryFunctorOp<MixerFunctor> op;
	op.functor.coeff = enhancement;
	op.traverseChannel(tmp.getChannel(0), dst.getChannel(0));
	// MixerOp(enhancement).filter(tmp,dst);

	if (mout.isDebug(10))
		File::write(dst, "andre-infect-2mix.png");

}

}  // namespace rack

// Rack
