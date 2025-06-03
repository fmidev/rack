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
#include <main/palette-manager.h>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>

#include <drain/Sprinter.h>
#include "drain/image/Image.h"
#include "drain/image/ImageFrame.h"
#include "drain/util/SmartMap.h"
#include "drain/util/TreeOrdered.h"

#include "data/Data.h"
#include "data/PolarODIM.h"
#include "data/QuantityMap.h"
#include "hi5/Hi5.h"
#include "radar/Analysis.h"

#include "QualityCombinerOp.h"

// using namespace std;

//using namespace drain;
//using namespace drain::image;

namespace rack {

//double QualityCombinerOp::DEFAULT_QUALITY(0.95);

//void QualityCombinerOp::initDstQuality(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstQind, const std::string & quantity){
void QualityCombinerOp::initDstQuality(const PolarODIM & srcODIM, PlainData<PolarDst> & dstQind, const std::string & quantity){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("Ensuring dst quality [", dstQind.odim.quantity, "] data array...");

	if (!dstQind.data.isEmpty()){
		mout.note("dst quality [", quantity ,"?] exists already");
	}
	else {

		if (dstQind.odim.quantity.empty())
			dstQind.odim.quantity = quantity;

		mout.debug("Creating quality field [", dstQind.odim.quantity, "] ~=[", quantity, ']');

		// Scaling, encoding
		getQuantityMap().setQuantityDefaults(dstQind, quantity);  // or PROB

		// Geometry
		// dstQind.setGeometry(srcData.odim.area);
		dstQind.setGeometry(srcODIM.area);
		mout.debug("set geometry: ", dstQind.data.getGeometry());
		//dstQind.odim.rscale = srcData.odim.rscale; // nbins, nrays, rscale
		dstQind.odim.rscale = srcODIM.rscale; // nbins, nrays, rscale

		//mout.special("quality " , dstQind );

		// Fill with init value
		if (quantity == "QIND"){
			const double DEFAULT_QUALITY = 0.9;
			// const double DEFAULT_QUALITY = 1.0;
			const double minCode = dstQind.data.getScaling().inv(DEFAULT_QUALITY);
			//mout.debug("Creating quality data [", dstQind.odim.quantity, "] with value=", DEFAULT_QUALITY);
			mout.debug("Creating QIND data with 1-(qMin=",  DEFAULT_QUALITY,  ") -> code [", minCode, "]");
			dstQind.data.fill(minCode);
			//dstQind.data.fill(dstData.odim.scaleInverse(1.0)); // max quality (250) by default.
		}	// Fill with init value
		else if (quantity == "CLASS"){ // TODO and unclass
			/*
			mout.note("Creating CLASS data based on src[" , srcData.odim.quantity , "]");
			mout.note(dstQind.data );
			RadarFunctorOp<DataMarker> marker;
			marker.odimSrc = srcData.odim;
			int codeUnclass = getClassCode("tech.class.unclass");
			marker.functor.set(codeUnclass);
			//mout.warn(marker );
			marker.process(srcData.data, dstQind.data);
			*/
			mout.debug("created empty CLASS" );
			//dstQind.data.fill(minCode);
			mout.debug2(" => DST: " , dstQind.data.getScaling() );
		}

		dstQind.data.setName(dstQind.odim.quantity);

	};

}

//void QualityCombinerOp::updateOverallDetection(const PlainData<PolarSrc> & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string & label, unsigned short index) { //const {
void QualityCombinerOp::updateOverallDetection(const drain::image::ImageFrame & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string & label, unsigned short index) { //const {

	drain::Logger mout(__FUNCTION__, label+"(DetectorOp)");
	//mout.debug(EncodingODIM(srcProb.odim) );
	mout.debug2(EncodingODIM(dstQind.odim) );

	/*
	mout.note("srcQ", srcQind.data,  " ", srcQind.odim);
	mout.note("srcC", srcClass.data, " ", srcClass.odim);
	mout.note("dstQ", dstQind.data,  " ", dstQind.odim);
	mout.note("dstC", dstClass.data, " ", dstClass.odim);
	*/

	mout.success(srcProb);
	//mout.success("srcProb ", srcProb.getScaling());

	if (srcProb.getScaling().scale == 0){
		mout.fail("No scaling factor in srcProb: ", srcProb);
	}

	//QualityCombinerOp::initDstQuality(srcProb, dstQind, "QIND");

	if (dstQind.data.isEmpty()){
		// ??? replace this with initDstQuality() or like?  Or is this for external input handling?
		mout.note("Creating QIND data");
		getQuantityMap().setQuantityDefaults(dstQind, "QIND");
		dstQind.setGeometry(srcProb.getGeometry());
		//dstQind.data.setGeometry(srcProb.getGeometry());
		dstQind.data.fill(dstQind.odim.scaleInverse(0.5)); // ? max / defaultQuality COMPARE with  initDstQuality
	};


	drain::VariableMap & qindHow = dstQind.getHow();
	//qindHow["task"] = std::string("fi.fmi.")+__RACK__+".AnDRe.Detector.OverallQuality";
	qindHow["task"] = "fi.fmi.rack.AnDRe.Detector.OverallQuality";
	qindHow["task_args"] << label; // like SHIPOP
	ProductBase::setRackVersion(qindHow);


	if (dstClass.data.isEmpty()){
		mout.note("Creating CLASS data");
		getQuantityMap().setQuantityDefaults(dstClass, "CLASS");
		dstClass.data.setGeometry(srcProb.getGeometry());
		// dstClass.fill(0);
	};

	//
	drain::VariableMap & classWhat = dstClass.getWhat();
	std::stringstream sstr;

	mout.debug("string legend revised");
	sstr << index << ':' << label;
	/*
	const classdict_t & dict = getClassPalette().dictionary;
	mout.debug2(index , ':' , dict.getValue(index) , '/' , label );
//#include "drain/util/SmartMap.h"

	sstr << index << ':' << dict.getValue(index);
	*/
	classWhat["legend"] << sstr.str();


	mout.debug("srcProb: ", srcProb);
	//mout.special('\t' , dstQind.odim.scaleForward(i)  );
	//mout.special('\t' , dstClass.odim.scaleForward(i) );

	/*
	for (int i : {0,1,64,128,192,255}){
		mout.special(i );
		//mout.special('\t' , srcProb.odim.scaleForward(i)  );
		mout.special('\t' , srcProb.getConf().fwd(i) );
		mout.special('\t' , dstQind#include "drain/util/SmartMap.h"
		.odim.scaleForward(i)  );
		mout.special('\t' , dstClass.odim.scaleForward(i) );
	}
	*/


	mout.debug("Updating QIND and CLASS data");

	Image::const_iterator  it = srcProb.begin();
	Image::iterator pit = dstQind.data.begin();
	Image::iterator cit = dstClass.data.begin();

	// Probability of anomaly
	// Quality = 1-Probability
	// Yes, quality is *decreased* here.

	const drain::ValueScaling & srcScale = srcProb.getConf();
	//const double DEFAULT_QUALITY = 0.7; // redesign
	//const double classUpdateThreshold = 0.5; // because no use to increase quality with anomaly
	double q;
	/// max quality this far
	double qCurrent;
	while (it != srcProb.end()){
		//p = *it;
		//q = 1.0 - srcProb.odim.scaleForward(*it);
		//q = 1.0 - srcProb.odim.scaleForward(*it);
		q = 1.0 - srcScale.fwd(*it);
		//qMax = *pit;
		qCurrent = dstQind.odim.scaleForward(*pit);
		if (q < qCurrent){
			//if (c < static_cast<int>(*pit) ){
			*pit = dstQind.odim.scaleInverse(q);
			//if (q < classUpdateThreshold)
			*cit = index;
		}
		++it; ++pit; ++cit;
	}

	//dstQind.tree

}

void QualityCombinerOp::updateOverallQuality(const PlainData<PolarSrc> & srcQind, const PlainData<PolarSrc> & srcClass, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass) { //const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("Ensuring quality [", dstQind.odim.quantity, "] ~ [QIND] data array...");

	//QualityCombinerOp::initDstQuality(srcQind, dstQind, "QIND");
	QualityCombinerOp::initDstQuality(srcQind.odim, dstQind, "QIND");

	// const double QUALITY_THRESHOLD = 0.99;

	std::set<std::string> srcClasses;
	srcQind.getHow()["task_args"].toSequence(srcClasses, ',');
	mout.special("srcClasses: ", drain::sprinter(srcClasses));

	drain::Variable & task_args = dstQind.getHow()["task_args"];
	std::set<std::string> dstClasses;
	task_args.toSequence(dstClasses, ',');
	mout.special("dstClasses: ", drain::sprinter(dstClasses));

	bool update = false;

	for (const std::string & quantity: srcClasses){
		if (dstClasses.find(quantity) == dstClasses.end()){
			update = true;
			mout.info("adding quantity: ", quantity);
			task_args << quantity;
		}
		else {
			mout.info("already updated quantity: ", quantity);
		}
	}


	if (!update){
		mout.info("no new quantities, returning" );
		return;
	}

	if (dstQind.data.getScaling().scale == 0){
		mout.attention("Dst QIND data had no scaling...");
		dstQind.data.setScaling(srcQind.data.getScaling());
		dstQind.odim.scaling.set(srcQind.data.getScaling());
	}

	if (srcClass.data.isEmpty()){

		if (!dstClass.data.isEmpty())
			mout.note("No CLASS data, ok, simpler update");
		else
			mout.warn("CLASS not provided in input, only QIND will be updated");

		Image::const_iterator  it  = srcQind.data.begin();
		Image::iterator pit = dstQind.data.begin();

		// Input quality
		double q;
		// Maximum quality this far
		double qMax;
		while (it != srcQind.data.end()){
			q = srcQind.odim.scaleForward(*it);
			qMax = *pit;
			qMax = dstQind.odim.scaleForward(qMax);
			if (q < qMax){
				*pit = dstQind.odim.scaleInverse(q);
			}
			++it; ++pit;
		}

	}
	else {

		mout.note("Updating QIND and CLASS data.");

		//QualityCombinerOp::initDstQuality(srcQind, dstClass, "CLASS");
		QualityCombinerOp::initDstQuality(srcQind.odim, dstClass, "CLASS");
		/*
		if (dstClass.data.isEmpty()){
			mout.note("Creating CLASS data" );
			getQuantityMap().setQuantityDefaults(dstClass, "CLASS");
			dstClass.data.setGeometry(srcQind.data.getGeometry());
			// dstClass.fill(0);
		};
		*/
		if (dstClass.data.getScaling().scale == 0){
			mout.attention("Dst CLASS data had no scaling...");
			dstClass.data.setScaling(srcClass.data.getScaling());
			dstClass.odim.scaling.set(srcClass.data.getScaling());
		}

		/*
		mout.note("srcQ ", srcQind.data,  " ", srcQind.odim);
		mout.note("srcC ", srcClass.data, " ", srcClass.odim);
		mout.note("dstQ ", dstQind.data,  " ", dstQind.odim);
		mout.note("dstC ", dstClass.data, " ", dstClass.odim);

		drain::image::FilePng::write(srcQind.data,  "srcQind.png");
		drain::image::FilePng::write(srcClass.data, "srcClass.png");
		drain::image::FilePng::write(dstQind.data,  "dstQind.png");
		drain::image::FilePng::write(dstClass.data, "dstClass.png");
		*/

		Image::const_iterator  sitQ  = srcQind.data.begin();
		Image::const_iterator  sitC = srcClass.data.begin();
		Image::iterator ditQ = dstQind.data.begin();
		Image::iterator ditC = dstClass.data.begin();

		// Input quality
		double q;
		// Maximum quality this far
		double qMax;
		while (sitQ != srcQind.data.end()){
			//p = *it;
			//p = 1.0 - srcQind.odim.scaleForward(p);
			q = srcQind.odim.scaleForward(*sitQ);
			//qMax = *ditQ;
			qMax = dstQind.odim.scaleForward(*ditQ);
			if (q < qMax){
				*ditQ = dstQind.odim.scaleInverse(q);
				*ditC = *sitC;
			}
			++sitQ; ++sitC; ++ditQ; ++ditC;
		}

		// drain::image::FilePng::write(dstQind.data,  "dstQind.png");
		// drain::image::FilePng::write(dstClass.data, "dstClass.png");


		// drain::Variable & classLegend = dstClass.getTree()["how"].data.attributes["legend"];
		drain::Variable & classLegend = dstClass.getWhat()["legend"];

		std::set<std::string> classCodes;
		classLegend.toSequence(classCodes, ',');

		// Add (combine) new classes
		std::set<std::string> classCodesNew;
		srcClass.getWhat()["legend"].toSequence(classCodesNew, ',');
		//for (std::set<std::string>::const_iterator it = classCodesNew.begin(); it != classCodesNew.end(); ++it){
		for (const std::string & code: classCodesNew){
			classCodes.insert(code);
		}

		mout.debug(" Updating CLASS, old: ", classLegend);
		classLegend = classCodes;
		mout.debug(" Updating CLASS, new: ", classLegend);
		//@ dstClass.updateTree();
		//@ DataTools::updateInternalAttributes(dstClass.tree);
	}

	//@ dstQind.updateTree();
	//@ DataTools::updateInternalAttributes(dstQind.tree);



	/*
	case MAX:
	case NONE:
	case LATEST: ?
	case UNDEFINED:
	*/

}


void QualityCombinerOp::updateLocalQuality(const DataSet<PolarSrc> & srcDataSet, Data<PolarDst> & dstData){

	//drain::Logger mout("QualityCombinerOp", __FUNCTION__);
	drain::Logger mout(__FILE__, __FUNCTION__);

	// Create in order:
	// quality1 => QIND
	// quality2 => CLASS

	const bool UPDATE_EXISTING = dstData.hasQuality();

	if (!UPDATE_EXISTING){
		mout.debug2("Skipping update, because srcData won't get it "); //??
		return;
	}

	const PlainData<PolarSrc> & srcQIND  = srcDataSet.getQualityData();
	const PlainData<PolarSrc> & srcCLASS = srcDataSet.getQualityData("CLASS");

	PlainData<PolarDst> & dstQIND  = dstData.getQualityData();
	PlainData<PolarDst> & dstCLASS = dstData.getQualityData("CLASS");

	if (UPDATE_EXISTING){
		mout.info("updating quality fields [QIND] & [CLASS] of [", dstData.odim.quantity, "]");
		updateOverallQuality(srcQIND, srcCLASS, dstQIND, dstCLASS);
	}
	else {
		// FIX: DEPRECATED code
		mout.attention("Not...? Copying local QIND and CLASS for ", dstData.odim.quantity);
		dstQIND.data.copyDeep(srcQIND.data);
		dstQIND.odim.importMap(srcQIND.odim);
		mout.note(EncodingODIM(dstQIND.odim));
		dstCLASS.data.copyDeep(srcCLASS.data);
		dstCLASS.odim.importMap(srcCLASS.odim);
	}

	//mout.advice("src qualities");
	for (const auto & entry: srcDataSet.getQuality()){
		if ((entry.first != "QIND") && (entry.first != "CLASS")){
			mout.experimental("quality information [", entry.first, "] added here / elangle=", entry.second.odim.elangle);
			//double marker = drain::image::PaletteOp::ge
			try {
				drain::image::Palette & palette = PaletteManager::getPalette("CLASS");
				const drain::image::Palette::value_type & legendEntry = palette.getEntryByCode(entry.first, true);
				// double marker = palette.getValueByCode(entry.first, true);
				mout.attention("found palette entry: ", sprinter(legendEntry.second, drain::Sprinter::jsonLayout));
				//updateOverallDetection(entry.second.data, dstQIND, dstCLASS, entry.first, (short unsigned int)123);
				mout.attention("srcData: ", entry.second.data);
				updateOverallDetection(entry.second.data, dstQIND, dstCLASS, entry.first, legendEntry.first);
			} catch (const std::exception & e) {
				mout.fail("Could not retrieve code (palette/legend entry) for [", entry.first, "]");
			}
		}
	}

}

/*  Essentially,
 *
 *
 */
//void QualityCombinerOp::processSweep(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const {
void QualityCombinerOp::processDataSet(const DataSet<PolarSrc> & srcDataSet, DataSet<PolarDst> & dstDataSet) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.info("Src: ", srcDataSet);

	/*
	const PlainData<PolarSrc> & srcQuality = src.getQualityData("QIND");
	if (srcQuality.data.isEmpty()){
		mout.info("no global (dataset-level) quality index data (QIND), skipping?");
	}

	const PlainData<PolarSrc> & srcClass   = src.getQualityData("CLASS");
	if (srcClass.data.isEmpty()){
		mout.info("no global (dataset-level) quality class data (CLASS), skipping.");
	}
	*/

	/*
	for (DataSet<PolarSrc>::const_iterator it = src.begin(); it != src.end(); ++it){
		mout.warn("src data quantity:" , it->first );
	}

	for (DataSet<PolarDst>::iterator it = dst.begin(); it != dst.end(); ++it){
		mout.warn("dst data quantity:" , it->first );
	}


	mout.special(src);
	for (const auto & entry: src.getQuality()){
		if ((entry.first != "QIND") && (entry.first != "CLASS")){
			mout.unimplemented("quality information [", entry.first, "] discarded here, elangle=", entry.second.odim.elangle);
		}
	}
	*/



	/// Note: iteration in src dataset (keys), because data selector applies to it.
	for (DataSet<PolarSrc>::const_iterator it = srcDataSet.begin(); it != srcDataSet.end(); ++it){

		mout.special("Handling: ", it->first);

		Data<PolarDst> & dstData = dstDataSet.getData(it->first);

		if (dstData.data.isEmpty()){
			// This is ok, if quality data has been pruned.
			mout.note("empty data [", it->first, "] in dst");
			// mout.fail("Something went wrong, dst has no quantity [", it->first, "]");
			// mout.advice("Weird, because dstData should be found (referenced) in srcDataSet");
			// continue;
		}

		/// TODO: move these to updateLocalQuality

		for (const char *quality: {"QIND", "CLASS"}){
			if (!dstData.hasQuality(quality)){
				mout.experimental("quality data [", quality," missing for [", it->first, "], adding...");
				// mout.experimental("");
				dstData.createSimpleQualityData(dstData.getQualityData("QIND"), 1.0, 0.95, 0.0);
			}
		}

		/*
		if (!dstData.hasQuality("QIND")){
			mout.note("no quality index data (QIND) for quantity=", it->first, ", skipping.");
			mout.experimental("");
			dstData.createSimpleQualityData(dstData.getQualityData("QIND"), 1.0, 0.95, 0.9);
			// mout.unimplemented("...Skipping");
			// continue;
		}
		// PlainData<PolarDst> & dstQuality = dstData.getQualityData("QIND");

		if (!dstData.hasQuality("CLASS")){
			mout.info("no quality class data (CLASS) for quantity=", it->first, ", skipping.");
			mout.unimplemented("Shouldn't it be added, instead?");
			continue;
		}
		*/

		updateLocalQuality(srcDataSet, dstData);

	}


}


}  // rack::

