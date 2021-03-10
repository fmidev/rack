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

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>

#include "drain/util/Log.h"
#include "drain/util/SmartMap.h"
#include "drain/util/Tree.h"
#include "drain/util/Variable.h"

#include "data/Data.h"
#include "data/PolarODIM.h"
#include "data/QuantityMap.h"
#include <hi5/Hi5.h>
#include "drain/image/Image.h"
#include "drain/image/ImageFrame.h"
#include <main/rack.h>

#include "radar/Analysis.h"
#include "data/EchoClass.h"

#include "QualityCombinerOp.h"

// using namespace std;

using namespace drain;
using namespace drain::image;

namespace rack {

//double QualityCombinerOp::DEFAULT_QUALITY(0.95);

void QualityCombinerOp::initDstQuality(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstQind, const std::string & quantity){

	drain::Logger mout(__FUNCTION__, __FILE__);

	const double DEFAULT_QUALITY = 0.9;
	// const double DEFAULT_QUALITY = 1.0;

	if (dstQind.data.isEmpty()){

		if (dstQind.odim.quantity.empty())
			dstQind.odim.quantity = quantity;

		mout.debug() << "Creating quality field [" << dstQind.odim.quantity <<  "] ~=[" << quantity <<  ']' << mout.endl;

		// Scaling, encoding
		getQuantityMap().setQuantityDefaults(dstQind, quantity);  // or PROB

		// Geometry
		dstQind.setGeometry(srcData.odim.area);
		mout.debug() << "set geometry: " << dstQind.data.getGeometry() << mout.endl;
		dstQind.odim.rscale = srcData.odim.rscale; // nbins, nrays, rscale

		//mout.special() << "quality " << dstQind << mout.endl;

		// Fill with init value
		if (quantity == "QIND"){
			const double minCode = dstQind.data.getScaling().inv(DEFAULT_QUALITY);
			mout.debug() << "Creating QIND data with 1-qMin: " << (DEFAULT_QUALITY) << " [" << minCode << "]" << mout.endl;
			dstQind.data.fill(minCode);
			//dstQind.data.fill(dstData.odim.scaleInverse(1.0)); // max quality (250) by default.
		}	// Fill with init value
		else if (quantity == "CLASS"){ // TODO and unclass
			/*
			mout.note() << "Creating CLASS data based on src[" << srcData.odim.quantity << "]"<< mout.endl;
			mout.note() << dstQind.data << mout.endl;
			RadarFunctorOp<DataMarker> marker;
			marker.odimSrc = srcData.odim;
			int codeUnclass = getClassCode("tech.class.unclass");
			marker.functor.set(codeUnclass);
			//mout.warn() << marker << mout.endl;
			marker.process(srcData.data, dstQind.data);
			*/
			mout.debug() << "created empty CLASS" << mout.endl;
			//dstQind.data.fill(minCode);
			mout.debug2() << " => DST: " << dstQind.data.getScaling() << mout.endl;
		}

		dstQind.data.setName(dstQind.odim.quantity);

	};

}

//void QualityCombinerOp::updateOverallDetection(const PlainData<PolarSrc> & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string & label, unsigned short index) { //const {
void QualityCombinerOp::updateOverallDetection(const drain::image::ImageFrame & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string & label, unsigned short index) { //const {

	drain::Logger mout(__FUNCTION__, label+"(DetectorOp)");
	//mout.debug()  <<  EncodingODIM(srcProb.odim) << mout.endl;
	mout.debug2() <<  EncodingODIM(dstQind.odim) << mout.endl;

	//QualityCombinerOp::initDstQuality(srcProb, dstQind, "QIND");

	if (dstQind.data.isEmpty()){
		mout.note() << "Creating QIND data" << mout.endl;
		getQuantityMap().setQuantityDefaults(dstQind, "QIND");
		dstQind.setGeometry(srcProb.getGeometry());
		//dstQind.data.setGeometry(srcProb.getGeometry());
		dstQind.data.fill(dstQind.odim.scaleInverse(0.6)); // ? max quality by default
	};


	drain::VariableMap & qindHow = dstQind.getHow();
	qindHow["task"] = std::string("fi.fmi.")+__RACK__+".AnDRe.Detector.OverallQuality";
	qindHow["task_args"] << label; // like SHIPOP


	if (dstClass.data.isEmpty()){
		mout.note() << "Creating CLASS data" << mout.endl;
		getQuantityMap().setQuantityDefaults(dstClass, "CLASS");
		dstClass.data.setGeometry(srcProb.getGeometry());
		// dstClass.fill(0);
	};

	//
	drain::VariableMap & classWhat = dstClass.getWhat();
	std::stringstream sstr;

	//const classdict_t & dict = getClassDict();
	const classdict_t & dict = getClassPalette().dictionary;
	mout.debug2() <<  index << ':' << dict.getValue(index) << '/' << label << mout.endl;

	sstr << index << ':' << dict.getValue(index);
	classWhat["legend"] << sstr.str();

	mout.debug() << "Updating QIND and CLASS data" << mout.endl;

	Image::const_iterator  it = srcProb.begin();
	Image::iterator pit = dstQind.data.begin();
	Image::iterator cit = dstClass.data.begin();

	mout.debug() << "srcProb: " << srcProb << mout;
	//mout.special() << '\t' << dstQind.odim.scaleForward(i)  << mout;
	//mout.special() << '\t' << dstClass.odim.scaleForward(i) << mout;

	/*
	for (int i : {0,1,64,128,192,255}){
		mout.special() << i << mout;
		//mout.special() << '\t' << srcProb.odim.scaleForward(i)  << mout;
		mout.special() << '\t' << srcProb.getConf().fwd(i) << mout;
		mout.special() << '\t' << dstQind.odim.scaleForward(i)  << mout;
		mout.special() << '\t' << dstClass.odim.scaleForward(i) << mout;
	}
	*/

	/// Probability of anomaly
	/// Quality = 1-Probability

	// Yes, quality is *decreased* here.

	const drain::ValueScaling & srcScale = srcProb.getConf();
	//const double DEFAULT_QUALITY = 0.7; // redesign
	const double classUpdateThreshold = 0.5; // because no use to increase quality with anomaly
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
			if (q < classUpdateThreshold)
				*cit = index;
		}
		++it; ++pit; ++cit;
	}

	//dstQind.tree

}

void QualityCombinerOp::updateOverallQuality(const PlainData<PolarSrc> & srcQind, const PlainData<PolarSrc> & srcClass, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass) { //const {

	drain::Logger mout(__FUNCTION__, __FILE__);


	QualityCombinerOp::initDstQuality(srcQind, dstQind, "QIND");

	const double DEFAULT_QUALITY = 0.7;


	std::set<std::string> classesNew;
	srcQind.getHow()["task_args"].toSequence(classesNew, ',');
	mout.special() << drain::sprinter(classesNew) << mout;

	drain::Variable & task_args = dstQind.getHow()["task_args"];
	std::set<std::string> classes;
	task_args.toSequence(classes, ',');
	mout.special() << drain::sprinter(classes) << mout;

	bool update = false;

	for (const std::string & quantity: classesNew){
		if (classes.find(quantity) == classes.end()){
			update = true;
			mout.info() << "adding quantity: " << quantity << mout.endl;
			task_args << quantity;
		}
		else {
			mout.info() << "already updated quantity: " << quantity << mout.endl;
		}
	}


	if (!update){
		mout.info() << "no new quantities, returning" << mout.endl;
		return;
	}

	if (srcClass.data.isEmpty()){

		if (!dstClass.data.isEmpty())
			mout.note() << "no CLASS data, ok, simpler update" << mout.endl;
		else
			mout.warn() << "CLASS not provided in input, only QIND will be updated" << mout.endl;

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

		mout.debug() << "Updating QIND and CLASS data." << mout.endl;

		QualityCombinerOp::initDstQuality(srcQind, dstClass, "CLASS");
		/*
		if (dstClass.data.isEmpty()){
			mout.note() << "Creating CLASS data" << mout.endl;
			getQuantityMap().setQuantityDefaults(dstClass, "CLASS");
			dstClass.data.setGeometry(srcQind.data.getGeometry());
			// dstClass.fill(0);
		};
		*/

		Image::const_iterator  it  = srcQind.data.begin();
		Image::const_iterator  itc = srcClass.data.begin();
		Image::iterator pit = dstQind.data.begin();
		Image::iterator cit = dstClass.data.begin();

		// Input quality
		double q;
		// Maximum quality this far
		double qMax;
		while (it != srcQind.data.end()){
			//p = *it;
			//p = 1.0 - srcQind.odim.scaleForward(p);
			q = srcQind.odim.scaleForward(*it);
			qMax = *pit;
			qMax = dstQind.odim.scaleForward(qMax);
			if (q < qMax){
				//if (c < static_cast<int>(*pit) ){
				*pit = dstQind.odim.scaleInverse(q);
				if (q < DEFAULT_QUALITY)
					*cit = *itc;
			}
			++it; ++itc; ++pit; ++cit;
		}

		// drain::Variable & classLegend = dstClass.getTree()["how"].data.attributes["legend"];
		drain::Variable & classLegend = dstClass.getWhat()["legend"];

		std::set<std::string> classCodes;
		classLegend.toSequence(classCodes, ',');

		// Add (combine) new classes
		std::set<std::string> classCodesNew;
		srcClass.getWhat()["legend"].toSequence(classCodesNew, ',');
		for (std::set<std::string>::const_iterator it = classCodesNew.begin(); it != classCodesNew.end(); ++it){
			classCodes.insert(*it);
		}

		mout.debug() << " Updating CLASS, old: " << classLegend << mout.endl;
		classLegend = classCodes;
		mout.debug() << " Updating CLASS, new: " << classLegend << mout.endl;
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
	drain::Logger mout(__FUNCTION__, __FILE__);

	// Create in order:
	// quality1 => QIND
	// quality2 => CLASS

	const bool UPDATE_EXISTING = dstData.hasQuality();

	if (!UPDATE_EXISTING){
		mout.debug2() << "Skipping update, because srcData won't get it " << mout.endl;
		return;
	}

	const PlainData<PolarSrc> & srcQIND  = srcDataSet.getQualityData();
	const PlainData<PolarSrc> & srcCLASS = srcDataSet.getQualityData("CLASS");

	PlainData<PolarDst> & dstQIND  = dstData.getQualityData();
	PlainData<PolarDst> & dstCLASS = dstData.getQualityData("CLASS");

	if (UPDATE_EXISTING){
		mout.success() << "updating.." << mout;
		updateOverallQuality(srcQIND, srcCLASS, dstQIND, dstCLASS);
	}
	else {
		// FIX: DEPRECATED code
		mout.note() << " NOT? Copying local QIND and CLASS for " <<  dstData.odim.quantity << mout.endl;
		dstQIND.data.copyDeep(srcQIND.data);
		dstQIND.odim.importMap(srcQIND.odim);
		mout.note() << EncodingODIM(dstQIND.odim) << mout.endl;
		dstCLASS.data.copyDeep(srcCLASS.data);
		dstCLASS.odim.importMap(srcCLASS.odim);
	}

}

/*  Essentially, difference of images filtered with median in two directions.
 *
 *
 */
//void QualityCombinerOp::processSweep(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const {
void QualityCombinerOp::processDataSet(const DataSet<PolarSrc> & src, DataSet<PolarDst> & dst) const {

	Logger mout(__FUNCTION__, __FILE__);

	mout.warn() << "Src: " << src << mout;

	//DataDst & dstData = dst.getFirstData("QIND"); //targetQuantity);
	//PlainData<PolarDst> & dstData = dst.getQualityData("QIND"); //targetQuantity);
	const PlainData<PolarSrc> & srcQuality = src.getQualityData("QIND");
	if (srcQuality.data.isEmpty()){
		mout.info() << "no global (dataset-level) quality index data (QIND), skipping." << mout.endl;
	}

	const PlainData<PolarSrc> & srcClass   = src.getQualityData("CLASS");
	if (srcClass.data.isEmpty()){
		mout.info() << "no global (dataset-level) quality class data (CLASS), skipping. " << mout.endl;
	}

	/*
	for (DataSet<PolarSrc>::const_iterator it = src.begin(); it != src.end(); ++it){
		mout.warn() << "src data quantity:" << it->first << mout.endl;
	}

	for (DataSet<PolarDst>::iterator it = dst.begin(); it != dst.end(); ++it){
		mout.warn() << "dst data quantity:" << it->first << mout.endl;
	}
	*/



	/// Note: iteration in src dataset (keys), because data selector applies to it.
	for (DataSet<PolarSrc>::const_iterator it = src.begin(); it != src.end(); ++it){

		mout.special() << it->first << mout;

		Data<PolarDst> & dstData = dst.getData(it->first);

		/// TODO: move these to updateLocalQuality

		if (!dstData.hasQuality("QIND")){
			mout.info() << "no quality index data (QIND) for quantity=" << it->first << ", skipping." << mout.endl;
			continue;
		}
		// PlainData<PolarDst> & dstQuality = dstData.getQualityData("QIND");

		if (!dstData.hasQuality("CLASS")){
			mout.info() << "no quality class data (CLASS) for quantity=" << it->first << ", skipping." << mout.endl;
			continue;
		}
		// PlainData<PolarDst> & dstClass = dstData.getQualityData("CLASS");

		updateLocalQuality(src, dstData);

	}


}


}  // rack::


// Rack
