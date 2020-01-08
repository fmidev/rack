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

#include <drain/util/Log.h>
#include <drain/util/SmartMap.h>
#include <drain/util/Tree.h>
#include <drain/util/Variable.h>

#include <data/Data.h>
#include <data/PolarODIM.h>
#include <data/QuantityMap.h>
#include <hi5/Hi5.h>
#include <image/Image.h>
#include <image/ImageFrame.h>
#include <main/rack.h>

#include <radar/Analysis.h>
#include <data/EchoClass.h>

#include "QualityCombinerOp.h"

// using namespace std;

using namespace drain;
using namespace drain::image;

namespace rack {

double QualityCombinerOp::DEFAULT_QUALITY(0.95);

void QualityCombinerOp::initDstQuality(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstQind, const std::string & quantity){

	drain::Logger mout("QualityCombinerOp", __FUNCTION__);

	if (dstQind.data.isEmpty()){

		if (dstQind.odim.quantity.empty())
			dstQind.odim.quantity = quantity;

		mout.debug() << "Creating quality field [" << dstQind.odim.quantity <<  "] ~=[" << quantity <<  ']' << mout.endl;

		// Scaling, encoding
		getQuantityMap().setQuantityDefaults(dstQind, quantity);  // or PROB

		// Geometry
		dstQind.setGeometry(srcData.odim.nbins, srcData.odim.nrays);
		mout.debug() << "set geometry: " << dstQind.data.getGeometry() << mout.endl;
		dstQind.odim.rscale = srcData.odim.rscale; // nbins, nrays, rscale

		// Fill with init value
		if (quantity == "QIND"){
			const double minCode = dstQind.data.getScaling().inv(QualityCombinerOp::DEFAULT_QUALITY);
			mout.debug() << "Creating QIND data with 1-qMin: " << (QualityCombinerOp::DEFAULT_QUALITY) << " [" << minCode << "]" << mout.endl;
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
			mout.debug(1) << " => DST: " << dstQind.data.getScaling() << mout.endl;
		}

		dstQind.data.setName(dstQind.odim.quantity);

	};


	/*
	if (dstData.data.isEmpty()){

		if (quantity.empty()){ // BIOMET, EMITTER, SHIP, etc.
			//dstData.odim.setQuantityDefaults("PROB");
			dstData.odim.quantity = getOutputQuantity();
			mout.debug() << "quantity ["<< dstData.odim.quantity << "], setting defaults of PROB" << mout.endl;
			getQuantityMap().setQuantityDefaults(dstData, "PROB");
		}
		else
			getQuantityMap().setQuantityDefaults(dstData, quantity);

		//dstData.data.setGeometry(srcData.data.getGeometry());
		dstData.setGeometry(srcData.odim.nbins, srcData.odim.nrays);
		mout.debug() << "set geometry: " << dstData.data.getGeometry() << mout.endl;
		//dstData.odim.nbins  = srcData.odim.nbins; // nbins, nrays, rscale
		//dstData.odim.nrays  = srcData.odim.nrays; // nbins, nrays, rscale
		dstData.odim.rscale = srcData.odim.rscale; // nbins, nrays, rscale
		if (quantity == "QIND"){
			dstData.data.fill(dstData.odim.scaleInverse(1.0)); // max quality (250) by default.
		}
		dstData.data.setName(dstData.odim.quantity);
	}
	else {
		mout.debug() << "already initialized: " << EncodingODIM(dstData.odim) << mout.endl;
		mout.debug(1) << dstData << mout.endl;
	}
	*/
}

//void QualityCombinerOp::updateOverallDetection(const PlainData<PolarDst> & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string & label, unsigned short index) { //const {
void QualityCombinerOp::updateOverallDetection(const PlainData<PolarSrc> & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string & label, unsigned short index) { //const {

	drain::Logger mout(label+"(DetectorOp)", __FUNCTION__);
	mout.debug()  <<  EncodingODIM(srcProb.odim) << mout.endl;
	mout.debug(1) <<  EncodingODIM(dstQind.odim) << mout.endl;

	QualityCombinerOp::initDstQuality(srcProb, dstQind, "QIND");

	/*
	if (dstQind.data.isEmpty()){
		mout.note() << "Creating QIND data" << mout.endl;
		getQuantityMap().setQuantityDefaults(dstQind, "QIND");
		dstQind.data.setGeometry(srcProb.data.getGeometry());
		//const
		dstQind.data.fill(dstQind.odim.scaleInverse(1.0)); // max quality by default
	};
	*/

	// drain::VariableMap & a = dstQind.getTree()["how"].data.attributes;
	drain::VariableMap & a = dstQind.getHow();
	a["task"] = std::string("fi.fmi.")+__RACK__+".AnDRe.Detector.OverallQuality";
	a["task_args"] << label; //getQuantityName();


	if (dstClass.data.isEmpty()){
		mout.note() << "Creating CLASS data" << mout.endl;
		getQuantityMap().setQuantityDefaults(dstClass, "CLASS");
		dstClass.data.setGeometry(srcProb.data.getGeometry());
		// dstClass.fill(0);
	};

	drain::VariableMap & howClass = dstClass.getHow();
	std::stringstream sstr;
	sstr << label << ':' << index;
	howClass["task_args"] << sstr.str();

	mout.debug() << "Updating QIND and CLASS data" << mout.endl;

	Image::const_iterator  it = srcProb.data.begin();
	Image::iterator pit = dstQind.data.begin();
	Image::iterator cit = dstClass.data.begin();

	/// Probability of anomaly
	//double p;
	/// Quality = 1-p
	const double qThreshold = DEFAULT_QUALITY; // because no use to increase quality with anomaly
	double q;
	/// max quality this far
	double qCurrent;
	while (it != srcProb.data.end()){
		//p = *it;
		q = 1.0 - srcProb.odim.scaleForward(*it);
		//qMax = *pit;
		qCurrent = dstQind.odim.scaleForward(*pit);
		if (q < qCurrent){
			//if (c < static_cast<int>(*pit) ){
			*pit = dstQind.odim.scaleInverse(q);
			if (q < qThreshold)
				*cit = index;
		}
		++it; ++pit; ++cit;
	}

	//dstQind.tree

}

void QualityCombinerOp::updateOverallQuality(const PlainData<PolarSrc> & srcQind, const PlainData<PolarSrc> & srcClass, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass) { //const {

	drain::Logger mout("QualityCombinerOp", __FUNCTION__);


	QualityCombinerOp::initDstQuality(srcQind, dstQind, "QIND");

	/*
	if (dstQind.data.isEmpty()){
		getQuantityMap().setQuantityDefaults(dstQind, "QIND");
		dstQind.data.setGeometry(srcQind.data.getGeometry());
		//const
		const double minCode = dstQind.data.getScaling().inv(QualityCombinerOp::DEFAULT_QUALITY);
		mout.note() << "Creating QIND data with q min: " << QualityCombinerOp::DEFAULT_QUALITY << " [" << minCode << "]" << mout.endl;
		dstQind.data.fill(minCode);
	};
	*/

	std::set<std::string> classesNew;
	srcQind.getHow()["task_args"].toContainer(classesNew, ',');

	drain::Variable & task_args = dstQind.getHow()["task_args"];
	std::set<std::string> classes;
	task_args.toContainer(classes, ',');

	bool update = false;

	for (std::set<std::string>::const_iterator it = classesNew.begin(); it != classesNew.end(); ++it){
		if (classes.find(*it) == classes.end()){
			update = true;
			mout.info() << "adding quantity: " << *it << mout.endl;
			task_args << *it;
		}
		else {
			mout.info() << "already updated quantity: " << *it << mout.endl;
		}
	}


	if (!update){
		mout.note() << "no new quantities, returning" << mout.endl;
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

		// drain::Variable & task_args_class = dstClass.getTree()["how"].data.attributes["task_args"];
		drain::Variable & task_args_class = dstClass.getHow()["task_args"];

		std::set<std::string> classCodes;
		task_args_class.toContainer(classCodes, ',');

		std::set<std::string> classCodesNew;
		srcClass.getHow()["task_args"].toContainer(classCodesNew, ',');
		// srcClass.getHow()["task_args"].toJSON(std::cerr, ' ', 3);
		// mout.debug() << " task args: " << classCodesNew.size() << mout.endl;

		// std::set<std::string> classCodesFinal;
		// set_union(classCodes.begin(), classCodes.end(), classCodesNew.begin(), classCodesNew.end(), classCodesFinal.begin());
		for (std::set<std::string>::const_iterator it = classCodesNew.begin(); it != classCodesNew.end(); ++it){
			classCodes.insert(*it);
		}
		//std::copy(classCodesNew.begin(), classCodesNew.end(), classCodes.begin());

		mout.debug() << " Updating CLASS, old: " << task_args_class << mout.endl;
		task_args_class = classCodes;
		mout.debug() << " Updating CLASS, new: " << task_args_class << mout.endl;
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

	drain::Logger mout("QualityCombinerOp", __FUNCTION__);

	// Create in order:
	// quality1 => QIND
	// quality2 => CLASS

	const bool UPDATE_EXISTING = dstData.hasQuality();

	if (!UPDATE_EXISTING){
		mout.debug(1) << "Skipping update, because srcData won't get it " << mout.endl;
		return;
	}

	const PlainData<PolarSrc> & srcQIND  = srcDataSet.getQualityData();
	const PlainData<PolarSrc> & srcCLASS = srcDataSet.getQualityData("CLASS");

	PlainData<PolarDst> & dstQIND  = dstData.getQualityData();
	PlainData<PolarDst> & dstCLASS = dstData.getQualityData("CLASS");

	if (UPDATE_EXISTING){
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

	Logger mout(name, __FUNCTION__);

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
	for (DataSet<>::const_iterator it = src.begin(); it != src.end(); ++it){
		mout.warn() << "src data quantity:" << it->first << mout.endl;
	}

	for (DataSet<>::iterator it = dst.begin(); it != dst.end(); ++it){
		mout.warn() << "dst data quantity:" << it->first << mout.endl;
	}
	*/


	/// Note: iteration in src dataset (keys), because data selector applies to it.
	for (DataSet<PolarSrc>::const_iterator it = src.begin(); it != src.end(); ++it){

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
