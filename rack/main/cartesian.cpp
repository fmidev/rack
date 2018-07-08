/**


    Copyright 2006 - 2010   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

   Created on: Sep 30, 2010
 */



#include <drain/util/Fuzzy.h>

#include <drain/image/File.h>
#include <drain/imageops/DistanceTransformFillOp.h>
#include <drain/imageops/RecursiveRepairerOp.h>

#include <drain/prog/CommandOpticalFlow.h>

#include "data/DataCoder.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"

#include "commands.h"  // for cmdFormat called by
#include "images.h"     // for cmdImage("image") called by cGrid("cGrid")

#include "cartesian.h"  // for cmdFormat called by
#include "cartesian-add.h"
#include "cartesian-bbox.h"
#include "cartesian-create.h"
#include "cartesian-grid.h"
#include "cartesian-extract.h"
#include "cartesian-motion.h"
#include "cartesian-plot.h"



namespace rack {




class CompositeDefaultQuality : public BasicCommand { //SimpleCommand<double> {

public:

	CompositeDefaultQuality() : BasicCommand(__FUNCTION__, "Set default quality (for data without quality field)"){
		//parameters.reference("weight", getResources().cDefaultQuality = 0.75, "0...1");
		parameters.reference("weight", getResources().composite.defaultQuality = 0.75, "0...1");
	};

};




// This is defined here because Create needs this
class CartesianProj : public BasicCommand {

public:

	CartesianProj() : BasicCommand(__FUNCTION__, "Set projection"){
		parameters.separator = 0;
		parameters.reference("projstr", getResources().projStr, "Proj4 syntax");
	};


	inline
	void exec() const {
		RackResources & resources = getResources();
		resources.composite.setProjection(resources.projStr);
	};

};




class CompositeInit : public BasicCommand {

public:

	CompositeInit() : BasicCommand(__FUNCTION__, "Allocate memory to --cSize, applying --target and --select, if set."){};

	void exec() const;

};



/*
CartesianBBoxTest() : SimpleCommand<int>(__FUNCTION__, "Tests whether the radar range is inside the composite.",
			"mode", 0, "If no overlap and n==0, only set inputOk=false. Else exit with return value n."	) {
 */

class CompositeMethod : public SimpleCommand<std::string> {
public:

	CompositeMethod() : SimpleCommand<std::string>(__FUNCTION__, "Method to be used in accumulating the (weighted) values.",
			"method", "MAXIMUM", "LATEST|MAXIMUM|MAXW|AVERAGE|WAVG,p,r,bias") { // , method() {
		getResources().composite.setMethod(value);
	};

	inline
	void exec() const {
		getResources().composite.setMethod(value);  // method? Or Method obj?
		getResources().polarAccumulator.setMethod(value);  // method? Or Method obj?
	};

};
/// static RackLetAdapter<CompositeMethod> cMethod("cMethod");



class CartesianSize : public BasicCommand {
    public: //re 
	int width;
	int height;

	inline
	CartesianSize() : BasicCommand(__FUNCTION__, "Set size of the compositing array. Does not allocate memory."){
		parameters.reference("width",  width = 100, "pixels");
		parameters.reference("height", height = 0, "pixels");
	};

	inline
	void exec() const {
		if (height == 0)
			getResources().composite.setGeometry(width, width);
		else
			getResources().composite.setGeometry(width, height);
	};

};



class CartesianTime : public SimpleCommand<std::string> {
    public: //re 

	inline
	CartesianTime() : SimpleCommand<>(__FUNCTION__, "Modify the time of the current composite. See --cTimeDecay ",
			"time", "201412091845", "YYYYmmddHHMMSS"){
		//parameters.separators.clear();
		//parameters.reference("time",  time,  "201412091845", "YYYYmmddHHMMSS");
	};

	inline
	void exec() const {
		RackResources & resources = getResources();
		resources.composite.odim.date = value.substr(0,8);
		resources.composite.odim.time = value.substr(8);
		//std::cout << resources.composite.odim.date << ',' << resources.composite.odim.time << std::endl;
	};

};


class CompositeTimeDecay : public BasicCommand {
    public: //re 
	CompositeTimeDecay() : BasicCommand(__FUNCTION__, "Delay weight (0.0...1.0) per minute. 1=no decay. See --cTime"){
		parameters.reference("decay", getResources().composite.decay = 1.0, "coeff");
	};

};








void CompositeInit::exec() const {

	drain::Logger mout(name, __FUNCTION__);

	RackResources & resources = getResources();

	/// Set data selector
	if (!resources.select.empty()){
		resources.composite.dataSelector.setParameters(resources.select);
		resources.select.clear();
		// resources.composite.odim.quantity.clear();
	}

	if ((resources.composite.getFrameWidth() == 0) || (resources.composite.getFrameHeight() == 0)){
		resources.composite.setGeometry(500,500); // frame only
		mout.warn() << "size unset, applying" << resources.composite.getFrameWidth() << ',' << resources.composite.getFrameHeight() << mout.endl;
	}
	resources.composite.allocate();

	if (!resources.targetEncoding.empty()){
		resources.composite.setTargetEncoding(resources.targetEncoding);
		mout.debug() << "target encoding: " << resources.composite.getTargetEncoding() << mout.endl;
		resources.targetEncoding.clear();
	}

	/*
	if (!resources.targetEncoding.empty()){
		resources.composite.odim.type = "*";
		resources.composite.odim.setValues(resources.targetEncoding);
		if (resources.composite.odim.gain == 0.0)
			mout.note() << "scaling not set, waiting for 1st input" << mout.endl;
		resources.targetEncoding.clear();
	}
	 */



}








CompositingModule::CompositingModule(const std::string & section, const std::string & prefix) :
		drain::CommandGroup(section, prefix) { //, cCreate("create",'c') {

	static RackLetAdapter<CompositeAdd> cAdd;
	static RackLetAdapter<CartesianAddWeighted> cAddWeighted;

	//static RackLetAdapter<CompositeAddOLD> cAddOLD;

	static RackLetAdapter<CompositeDefaultQuality> cDefaultQuality;
	static RackLetAdapter<CartesianBBox> cBBox;
	static RackLetAdapter<CartesianBBoxReset> cBBoxReset;
	static RackLetAdapter<CartesianBBoxTest> cBBoxTest;
	static RackLetAdapter<CartesianBBoxTile> cBBoxTile;

	static RackLetAdapter<CartesianExtract> cExtract;
	static RackLetAdapter<CartesianGrid> cGrid;
	static RackLetAdapter<CompositeInit> cInit;
	static RackLetAdapter<CompositeMethod> cMethod;
	static RackLetAdapter<CartesianPlot> cPlot;
	static RackLetAdapter<CartesianPlotFile> cPlotFile;
	static RackLetAdapter<CartesianProj> cProj;  // shared
	static RackLetAdapter<CartesianQuantity> cQuantity;
	static RackLetAdapter<CartesianRange> cRange;
	static RackLetAdapter<CartesianSize> cSize;
	static RackLetAdapter<CartesianSpread> cSpread;
	static RackLetAdapter<CartesianTime> cTime;
	static RackLetAdapter<CompositeTimeDecay> cTimeDecay;

	static RackLetAdapter<CartesianCreate> cCreate("create", 'c', CartesianCreate(cAdd, cExtract));
	static RackLetAdapter<CartesianCreateTile> cCreateTile("createTile", 0, CartesianCreateTile(cAdd, cExtract));


	static RackLetAdapter<CartesianOpticalFlow> opticalFlow("opticalFlow");

	// static RackLetAdapter<CmdMotionFill> motionFill("motionFill"); => drain/image/ImageOpBank => FlowAverage

}

}  // namespace rack::


