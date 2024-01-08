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
#include <stdexcept>

// DEBUGGING
#include "drain/util/Log.h"
#include "drain/util/FunctorPack.h"

#include "drain/imageops/DistanceTransformOp.h"
#include "drain/imageops/FunctorOp.h"
#include "drain/imageops/GammaOp.h"
#include "drain/imageops/MarginalStatisticOp.h"

//#include "main/rack.h"
#include "hi5/Hi5.h"
//#include "hi5/Hi5Write.h"

//#include "radar/DataCopyOp.h"
#include "product/DataConversionOp.h"

#include "QualityCombinerOp.h"
#include "DetectorOp.h"


//using namespace drain::image;

namespace rack {

//bool DetectorOp::STORE(false);

//DetectorOp::Cumulation DetectorOp::cumulateDetections = DetectorOp::MAX;
//std::string DetectorOp::dstGroupPrefix = "quality";

bool DetectorOp::SUPPORT_UNIVERSAL(true);

unsigned short int DetectorOp::_count(0);


/*
void DetectorOp::processProduct(DataSetList<PolarDst> & srcVolume, DataSetList<PolarDst> & dstVolume) const {
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented("TODO...");
	mout.error("TODO...");
}
*/

/*
void DetectorOp::processDataSets(const DataSetMap<PolarSrc> & srcDataSets, DataSetMap<PolarDst> & dstDataSets) const {
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented("TODO...");
	mout.error("TODO...");
}
*/

void DetectorOp::runDetection(const DataSetMap<PolarSrc> & srcVolume, DataSetMap<PolarDst> & dstVolume) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	const std::string & CLASSNAME = getOutputQuantity();

	mout.attention(CLASSNAME);

	/*
	for (const auto & entry: srcVolume){

		mout.attention("entry: ", entry.first, ':', entry.second);

		const DataSet<PolarSrc> & srcDataSet = entry.second;

		// mout.attention(srcDataSet.odim);
		const drain::Castable & c = srcDataSet.getWhere()["elangle"];
		double d = c;
		mout.debug("metadata: elangle: ", d);


		DataSet<PolarDst> & dstDataSet =  dstVolume[entry.first];  //itd->second;

		const Data<PolarSrc> & srcData = srcDataSet.getFirstData();
		if (srcData.data.isEmpty()){
			mout.warn("empty srcData [" , srcDataSet.begin()->first , "]: " , srcData );
			return;
		}
		else if (!srcData.data.typeIsSet()){
			mout.warn("unset type in srcData [" , srcDataSet.begin()->first , "]: " , srcData );
			return;
		}

		if (srcData.odim.area.empty()){ // width==0) || (srcData.odim.area.height==0)){
			mout.warn("empty geom in odim of srcData [" , srcDataSet.begin()->first , "]: " , srcData.odim );
			return;
		}

		// Note: if a detector is run like a product, this should CREATE data.

		Data<PolarDst> & dstData = dstDataSet.getFirstData(); // only for appending QIND and CLASS


		mout.debug("CLASSNAME=", CLASSNAME, " universal=", SUPPORT_UNIVERSAL, '&', UNIVERSAL );

		//const std::string QIND = "QIND"; // (SUPPORT_UNIVERSAL && UNIVERSAL)? "QIND" : "qind";

		/// TODO: UNIVERSAL and several inputs?
		// OVERALL QUALITY (PROB.)
		PlainData<PolarDst> & dstQind = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData("QIND") : dstData.getQualityData("QIND"); // of first data (eg. TH)
		initDataDst(srcData, dstQind, "QIND");

		// OVERALL QUALITY FIELD
		PlainData<PolarDst> & dstClass = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData("CLASS") : dstData.getQualityData("CLASS"); // TODO: local (not only univ.)
		initDataDst(srcData, dstClass, "CLASS");

		// PROBABILITY OF THE CLASS APPROXIMATED BY THIS DETECTOR
		PlainData<PolarDst> & dstProb = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData(CLASSNAME) : dstData.getQualityData(CLASSNAME);
		//dstProb.tree.data.exclude = !DetectorOp::STORE;
		initDataDst(srcData, dstProb);
		mout.debug("outputDataVerbosity ", outputDataVerbosity);
		dstProb.setExcluded(outputDataVerbosity==0);

		//mout.warn("dstProb: " , dstProb );
		/// MAIN COMMAND
		processDataSet(srcDataSet, dstProb,  dstDataSet);


		//QualityCombinerOp::updateOverallDetection(srcProb, dstQind, dstClass, CLASSNAME, classCode);
		QualityCombinerOp::updateOverallDetection(dstProb.data, dstQind, dstClass, CLASSNAME, classCode);
		//File::write(dstQind.data, "dstQind2.png");
		//File::write(dstClass.data, "dstClass2.png");
		//mout.note(dstDataSet );

	}
	*/

	//DataSetMap<PolarSrc>::const_iterator its = srcDataSets.begin();
	//DataSetMap<PolarDst>::iterator       itd = dstDataSets.begin();
	DataSetMap<PolarSrc>::const_iterator its = srcVolume.begin();
	DataSetMap<PolarDst>::iterator       itd = dstVolume.begin();
	while (its != srcVolume.end()){

		//mout.info("processing elangle: ", its->first, "⁰");  // TODO: waiting for DataSetMap<elangle> renewal..
		mout.info("processing: ", its->first);  // TODO: waiting for DataSetMap<elangle> renewal..

		if (its->first == itd->first){

			//mout.warn(its->first );
			//return;

			const DataSet<PolarSrc> & srcDataSet = its->second;

			// mout.attention(srcDataSet.odim);
			const drain::Castable & c = srcDataSet.getWhere()["elangle"];
			double d = c;
			mout.debug("metadata: elangle: ", d);


			DataSet<PolarDst> & dstDataSet = itd->second;

			const Data<PolarSrc> & srcData = srcDataSet.getFirstData();
			if (srcData.data.isEmpty()){
				mout.warn("empty srcData [" , srcDataSet.begin()->first , "]: " , srcData );
				return;
			}
			else if (!srcData.data.typeIsSet()){
				mout.warn("unset type in srcData [" , srcDataSet.begin()->first , "]: " , srcData );
				return;
			}
			if ((srcData.odim.area.width==0) || (srcData.odim.area.height==0)){
				mout.warn("empty geom in odim of srcData [" , srcDataSet.begin()->first , "]: " , srcData.odim );
				return;
			}

			// Note: if a detector is run like a product, this should CREATE data.

			Data<PolarDst> & dstData = dstDataSet.getFirstData(); // only for appending QIND and CLASS


			mout.debug("CLASSNAME=", CLASSNAME, " universal=", SUPPORT_UNIVERSAL, '&', UNIVERSAL );

			/// TODO: UNIVERSAL and several inputs?
			// OVERALL QUALITY (PROB.)
			PlainData<PolarDst> & dstQind = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData("QIND") : dstData.getQualityData("QIND"); // of first data (eg. TH)
			initDataDst(srcData, dstQind, "QIND");

			// OVERALL QUALITY FIELD
			PlainData<PolarDst> & dstClass = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData("CLASS") : dstData.getQualityData("CLASS"); // TODO: local (not only univ.)
			initDataDst(srcData, dstClass, "CLASS");

			// PROBABILITY OF THE CLASS APPROXIMATED BY THIS DETECTOR
			PlainData<PolarDst> & dstProb = (SUPPORT_UNIVERSAL && UNIVERSAL) ? dstDataSet.getQualityData(CLASSNAME) : dstData.getQualityData(CLASSNAME);
			//dstProb.tree.data.exclude = !DetectorOp::STORE;
			initDataDst(srcData, dstProb);
			mout.debug("outputDataVerbosity ", outputDataVerbosity);
			dstProb.setExcluded(outputDataVerbosity==0);
			// mout.debug2("dstProb: ", dstProb);

			/// MAIN COMMAND
			runDetection(srcDataSet, dstProb,  dstDataSet);


			//QualityCombinerOp::updateOverallDetection(srcProb, dstQind, dstClass, CLASSNAME, classCode);
			//QualityCombinerOp::updateOverallDetection(dstProb.data, dstQind, dstClass, CLASSNAME, classCode);
			QualityCombinerOp::updateOverallDetection(dstProb.data, dstQind, dstClass, CLASSNAME, classEntry.first);
			//File::write(dstQind.data, "dstQind2.png");
			//File::write(dstClass.data, "dstClass2.png");
			//mout.note(dstDataSet );

		}
		else {
			mout.warn("Something went wrong, dst has different angle, dst=", itd->first, " != ", its->first);
			return;
		}
		++its;
		++itd;
	}

	mout.debug2("end");

}

void DetectorOp::runDetection(const DataSet<PolarSrc> & srcDataSet, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & cache) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.info("start");
	//mout.warn("start" );

	if (srcDataSet.size() == 0){
		mout.warn("dataset contains no data, skipping");
		return;
	}
	else {

		const PlainData<PolarSrc> & srcData = srcDataSet.getFirstData();

		if (REQUIRE_STANDARD_DATA){
			//mout.note("dstProb (target): " , dstProb );
			mout.debug("requires normalized srcData" );

		    const QuantityMap & qmap = getQuantityMap();
		    if (qmap.isNormalized(srcData.odim)){
				mout.debug("srcData is normalized already, ok");
		    	runDetector(srcData, dstProb);
		    }
		    else {
		    	mout.info("using 'normalized' data...");
		    	PlainData<PolarDst> & srcDataNorm = DataConversionOp<PolarODIM>::getNormalizedData(srcDataSet, cache,  srcData.odim.quantity);
				//mout.debug3("got normalized data, now processing" );
				mout.debug("srcDataNorm: ", srcDataNorm.odim);
				runDetector(srcDataNorm, dstProb);
		    }
			mout.debug2("dstProb (result): ", dstProb, " OK?");
		}
		else {
			mout.debug("no data normalization needed, ok");
			runDetector(srcData, dstProb);
		}

		writeHow(dstProb);
		//dstProb.getTree()["what"].data.attributes["mika"] = 123.456;
		//}
	}

	//mout.warn("end" , dstProb.odim.quantity );

}


/// If raised, make template? Cf. Volume::initDst ?
void DetectorOp::initDataDst(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, const std::string & quantity) const {

	drain::Logger mout(__FUNCTION__, getName() + "(DetectorOp)");

	if (dstData.data.isEmpty()){

		if (quantity.empty()){
			//mout.warn("requested: " , quantity );
			dstData.odim.quantity = getOutputQuantity();
			//QualityCombinerOp::initDstQuality(srcData, dstData, "PROB");
			QualityCombinerOp::initDstQuality(srcData.odim, dstData, "PROB");
		}
		else {
			dstData.odim.quantity = quantity;
			//QualityCombinerOp::initDstQuality(srcData, dstData, quantity);
			QualityCombinerOp::initDstQuality(srcData.odim, dstData, quantity);
		}

	}
	else {
		mout.debug("already initialized: " , EncodingODIM(dstData.odim) );
		mout.debug2(dstData );
	}
}



const std::string & DetectorOp::getOutputQuantity(const std::string & inputQuantity) const {

	// If unset, copy in uppercase letters.
	if (upperCaseName.empty()) {
		// upperCaseName = name.substr(0, name.length()-2); // Rely on "Op" in the end.
		upperCaseName = classEntry.second.code;
		drain::StringTools::upperCase(upperCaseName);
	}

	return upperCaseName;

}

void DetectorOp::storeDebugData(const ImageFrame & srcImage, PlainData<PolarDst> & dstData, const std::string & quantityLabel) const {

	DataSet<PolarDst> dstDataSet(dstData.getTree());

	PlainData<PolarDst> & dstDebugData = dstDataSet.getQualityData(quantityLabel);

	// Copy ?
	drain::image::UnaryFunctorOp<drain::ScalingFunctor>().process(srcImage, dstDebugData.data);
	//ScaleOp().filter(srcImage, dstDebugData.data);

	dstDebugData.odim.quantity = quantityLabel;
	// dstDebugData.updateTree();

}

void DetectorOp::storeDebugData(int debugLevel, const ImageFrame & srcImage, const std::string & label) const {

	static int counter=0;

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (mout.isDebug(debugLevel)){
		std::stringstream sstr;
		sstr << name << '-';
		sstr.width(2);
		sstr.fill('0');
		sstr << counter << '-' << label << ".png";
		const std::string filename = sstr.str();
		mout.debug("writing " , filename );
		ImageFile::write(srcImage, filename);
		counter++;
	}

}


void DetectorOp::writeHow(PlainData<PolarDst> & dstData) const {

	drain::VariableMap & a = dstData.getHow();
	a["task"] = std::string("fi.fmi.Rack.AnDRe.")+name;
	//a["task_args"] = getParameters().toStr(':');
	static const drain::SprinterLayout layout(",", ":");
	a["task_args"] = drain::sprinter(getParameters(), layout).str();


}



void DetectorOp::_enhanceDirectionally(Image & dst, float medianPos, int width) const {

	drain::Logger mout(drain::getLog(), __FUNCTION__, getName());
	mout.debug(" called by " , name );
	if (mout.isDebug(12))
		ImageFile::write(dst,"andre-enh0-src.png");

	Image tmp;

	MarginalStatisticOp statOp;
	statOp.mode = "horz";
	statOp.stat = "m"; // median
	statOp.medianPos = medianPos;
	mout .debug3() << statOp << mout.endl;
	statOp.process(dst, tmp);
	if (mout.isDebug(20))
		ImageFile::write(tmp,"andre-enh1-stat.png");

	//GammaOp gammaOp;
	drain::image::UnaryFunctorOp<drain::image::GammaFunctor> gammaOp;
	gammaOp.functor.gamma = 4.0;
	mout .debug3() << gammaOp << mout.endl;
	gammaOp.traverseChannel(tmp.getChannel(0), tmp.getChannel(0));
	if (mout.isDebug(20))
		ImageFile::write(tmp,"andre-enh2-gamma.png");


	DistanceTransformExponentialOp dtfOp(1,width);
	mout .debug3() << dtfOp << mout.endl;
	dtfOp.traverseChannel(tmp.getChannel(0), tmp.getChannel(0));
	if (mout.isDebug(20))
		ImageFile::write(tmp,"andre-enh3-dtf.png");

	// MultiplicationOp
	drain::image::BinaryFunctorOp<drain::MultiplicationFunctor> mulOp;
	mout .debug3() << mulOp << mout.endl;
	mulOp.traverseChannel(tmp.getChannel(0), dst.getChannel(0));
	if (mout.isDebug(22))
		ImageFile::write(dst,"andre-enh4-dst.png");


}

///
void DetectorOp::_infect(Image & dst, int windowWidth, int windowHeight, double enhancement) const {

	drain::Logger mout(drain::getLog(), __FUNCTION__, getName());

	DistanceTransformExponentialOp distOp(windowWidth, windowHeight);

	drain::image::Image tmp;
	mout.debug(5) << distOp << mout.endl;
	distOp.process(dst, tmp);
	if (mout.isDebug(10))
		ImageFile::write(tmp,"andre-infect-1dist.png");

	drain::image::BinaryFunctorOp<drain::MixerFunctor> op;
	op.functor.coeff = enhancement;
	op.traverseChannel(tmp.getChannel(0), dst.getChannel(0));
	// MixerOp(enhancement).filter(tmp,dst);

	if (mout.isDebug(10))
		ImageFile::write(dst, "andre-infect-2mix.png");

}

}  // namespace rack

// Rack
