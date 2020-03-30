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


#include <string>

#include <data/DataSelector.h>
#include <prog/Command.h>
#include <radar/Composite.h>
#include <radar/RadarAccumulator.h>
#include <util/Log.h>
#include <util/SmartMap.h>

#include <main/cartesian.h>  // for cmdFormat called by
#include <main/cartesian-add.h>
#include <main/cartesian-bbox.h>
#include <main/cartesian-create.h>
#include <main/cartesian-grid.h>
#include <main/cartesian-motion.h>
#include <main/cartesian-plot.h>




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

	inline
	void exec() const {
		getResources().initComposite();
		//CompositingModule::initComposite();
	}

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

public:

	int width;
	int height;

	inline
	CartesianSize() : BasicCommand(__FUNCTION__, "Set size of the compositing array. Does not allocate memory."){
		parameters.reference("width",  width = 400, "pixels");
		parameters.reference("height", height = 0, "pixels");
	};

	inline
	void exec() const {
		// int w = width  ? width  : 500;
		// int h = height ? height : 500;
		if (height == 0)
			getResources().composite.setGeometry(width, width);
		else
			getResources().composite.setGeometry(width, height);
	};

};



class CartesianTime : public SimpleCommand<std::string> {

public:

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
		const size_t n = resources.composite.odim.time.length();
		if (n < 6){
			//drain::Logger mout(__FUNCTION__, getName());
			//mout.info() << "appending " << (6-n) <<  " letters" << mout.endl;
			resources.composite.odim.time.append(6-n, '0');
		}
		//std::cout << resources.composite.odim.date << ',' << resources.composite.odim.time << std::endl;
	};

};


///
// TODO: change to half-time in minutes
class CompositeTimeDecay : public BasicCommand {

	public:
	CompositeTimeDecay() : BasicCommand(__FUNCTION__, "Delay weight (0.9...1.0) per minute. 1=no decay. See --cTime"){
		parameters.reference("decay", getResources().composite.decay = 1.0, "coeff");
	};

};


/// half-time in minutes
/**
 *    x/2 = x * (1/2)^{t/T}
 *
 *	log(0.5) = t/T*log(0.5) =>
 */
class CompositeDecayTime : public SimpleCommand<int> {

	public:
	CompositeDecayTime() : SimpleCommand<int>(__FUNCTION__, "Delay half-time in minutes. 0=no decay", "time", 0, "minutes"){
		//parameters.reference("halftime", getResources().composite.decay = 1.0, "coeff");
	};

	inline
	void exec() const {
		RackResources & resources = getResources();
		if (this->value > 0){
			resources.composite.decay = ::pow(0.5, 1.0 / static_cast<double>(value));
		}
		else {
			resources.composite.decay = 1.0;
		}
	}


};



/*
//void CompositeInit::exec() const {
void CompositingModule::initComposite() {

	drain::Logger mout(__FUNCTION__, getName());

	RackResources & resources = getResources();

	/// Set data selector
	if (!resources.select.empty()){
		resources.composite.dataSelector.setParameters(resources.select);
		resources.select.clear();
		// resources.composite.odim.quantity.clear();
	}

	if ((resources.composite.getFrameWidth() == 0) || (resources.composite.getFrameHeight() == 0)){
		resources.composite.setGeometry(500,500); // frame only
		mout.warn() << "size unset, applying " << resources.composite.getFrameWidth() << ',' << resources.composite.getFrameHeight() << mout.endl;
	}
	resources.composite.allocate();

	if (!resources.targetEncoding.empty()){
		resources.composite.setTargetEncoding(resources.targetEncoding);
		mout.debug() << "target encoding: " << resources.composite.getTargetEncoding() << mout.endl;
		resources.targetEncoding.clear();
	}


}
*/







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
	static RackLetAdapter<CompositeDecayTime> cCompositeDecayTime;


	static RackLetAdapter<CartesianCreate> cCreate("create", 'c', CartesianCreate(cAdd, cExtract));
	static RackLetAdapter<CartesianCreateTile> cCreateTile("createTile", 0, CartesianCreateTile(cAdd, cExtract));
	static RackLetAdapter<CartesianSun> cCreateSun("createSun", 0); //, CartesianSun(cExtract));


	static RackLetAdapter<CartesianOpticalFlow> opticalFlow("opticalFlow");

	// static RackLetAdapter<CmdMotionFill> motionFill("motionFill"); => drain/image/ImageOpBank => FlowAverage

}

}  // namespace rack::



// Rack
