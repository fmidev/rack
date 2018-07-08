/**

    Copyright 2015-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack library for C++.

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

#include <algorithm>


#include <drain/util/Log.h>

//#include <drain/image/MathOpPack.h>


// debugging
#include <drain/image/File.h>
#//include "odim/ODIM.h"

#include "AndreOp.h"

#include "QualityCombinerOp.h"

// using namespace std;

using namespace drain;
using namespace drain::image;

namespace rack {

double QualityCombinerOp::CLASS_UPDATE_THRESHOLD(0.5);

//void QualityCombinerOp::updateOverallDetection(const PlainData<PolarDst> & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string & label, unsigned short index) { //const {
void QualityCombinerOp::updateOverallDetection(const PlainData<PolarSrc> & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string & label, unsigned short index) { //const {

	drain::Logger mout(label+"(DetectorOp)", __FUNCTION__);
	mout.debug()  <<  EncodingODIM(srcProb.odim) << mout.endl;
	mout.debug(1) <<  EncodingODIM(dstQind.odim) << mout.endl;

	if (dstQind.data.isEmpty()){
		mout.note() << "Creating QIND data" << mout.endl;
		getQuantityMap().setQuantityDefaults(dstQind, "QIND");
		dstQind.data.setGeometry(srcProb.data.getGeometry());
		//const
		dstQind.data.fill(dstQind.odim.scaleInverse(1.0)); // max quality by default
	};

	drain::VariableMap & a = dstQind.tree["how"].data.attributes;
	a["task"] = std::string("fi.fmi.")+__RACK__+".AnDRe.Detector.OverallQuality";
	a["task_args"] << label; //getQuantityName();


	if (dstClass.data.isEmpty()){
		mout.note() << "Creating CLASS data" << mout.endl;
		getQuantityMap().setQuantityDefaults(dstClass, "CLASS");
		dstClass.data.setGeometry(srcProb.data.getGeometry());
		// dstClass.fill(0);
	};

	drain::VariableMap & howClass = dstClass.tree["how"].data.attributes;
	std::stringstream sstr;
	sstr << label << ':' << index;
	howClass["task_args"] << sstr.str();

	Image::const_iterator  it = srcProb.data.begin();
	Image::iterator pit = dstQind.data.begin();
	Image::iterator cit = dstClass.data.begin();

	/*
	switch (cumulateDetections){
	case MAX:
		//AndreMinCumulatorOp().filter(detection, probCumulation, probCumulation);
	 *
	 */
	/// Probability of anomaly
	double p;
	/// Quality = 1-p
	double q;
	/// max quality this far
	double qMax;
	while (it != srcProb.data.end()){
		p = *it;
		q = 1.0 - srcProb.odim.scaleForward(p);
		qMax = *pit;
		qMax = dstQind.odim.scaleForward(qMax);
		if (q < qMax){
			//if (c < static_cast<int>(*pit) ){
			*pit = dstQind.odim.scaleInverse(q);
			if (q < CLASS_UPDATE_THRESHOLD)
				*cit = index;
		}
		++it; ++pit; ++cit;
	}

	//dstQind.tree

}

void QualityCombinerOp::updateOverallQuality(const PlainData<PolarSrc> & srcQind, const PlainData<PolarSrc> & srcClass, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass) { //const {

	drain::Logger mout("QualityCombinerOp", __FUNCTION__);

	if (dstQind.data.isEmpty()){
		mout.debug() << "Creating QIND data" << mout.endl;
		getQuantityMap().setQuantityDefaults(dstQind, "QIND");
		dstQind.data.setGeometry(srcQind.data.getGeometry());
		//const
		dstQind.data.fill(250);
	};


	std::set<std::string> classesNew;
	srcQind.tree["how"].data.attributes["task_args"].toContainer(classesNew);

	drain::Variable & task_args = dstQind.tree["how"].data.attributes["task_args"];
	std::set<std::string> classes;
	task_args.toContainer(classes);

	bool update = false;

	for (std::set<std::string>::const_iterator it = classesNew.begin(); it != classesNew.end(); ++it){
		if (classes.find(*it) == classes.end()){
			update = true;
			mout.info() << "adding quantity:" << *it << mout.endl;
			task_args << *it;
		}
		else {
			mout.note() << "already updated quantity:" << *it << mout.endl;
		}
	}


	if (!update)
		return;



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

		if (dstClass.data.isEmpty()){
			mout.note() << "Creating CLASS data" << mout.endl;
			getQuantityMap().setQuantityDefaults(dstClass, "CLASS");
			dstClass.data.setGeometry(srcQind.data.getGeometry());
			// dstClass.fill(0);
		};

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
				if (q < CLASS_UPDATE_THRESHOLD)
					*cit = *itc;
			}
			++it; ++itc; ++pit; ++cit;
		}

		drain::Variable & task_args_class = dstClass.tree["how"].data.attributes["task_args"];

		std::set<std::string> classCodes;
		task_args_class.toContainer(classCodes);

		std::set<std::string> classCodesNew;
		srcClass.tree["how"].data.attributes["task_args"].toContainer(classCodesNew);

		//std::set<std::string> classCodesFinal;
		//set_union(classCodes.begin(), classCodes.end(), classCodesNew.begin(), classCodesNew.end(), classCodesFinal.begin());
		for (std::set<std::string>::const_iterator it = classCodesNew.begin(); it != classCodesNew.end(); ++it){
			classCodes.insert(*it);
		}
		mout.debug() << " Updating CLASS: " << task_args_class <<  " => ";
		task_args_class = classCodes;
		// task_args_class.clear();
		// task_args_class = fromContainer(classCodes);
		mout << task_args_class << mout.endl;


		//@ dstClass.updateTree();
		//@ DataTools::updateAttributes(dstClass.tree);
	}

	//@ dstQind.updateTree();
	//@ DataTools::updateAttributes(dstQind.tree);



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
	PlainData<PolarDst> & dstQIND  = dstData.getQualityData();
	PlainData<PolarDst> & dstCLASS = dstData.getQualityData("CLASS");

	updateOverallQuality(srcDataSet.getQualityData(), srcDataSet.getQualityData("CLASS"), dstQIND, dstCLASS);

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

