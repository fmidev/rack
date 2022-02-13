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

#include "drain/util/Log.h"

///#include "drain/prog/Command.h"
///
#include "drain/prog/CommandInstaller.h"


#include "resources.h"  // for RackContext?

#include "composite.h"  // for cmdFormat called by
#include "cartesian.h"  // for cmdFormat called by
#include "cartesian-add.h"
#include "cartesian-extract.h"
#include "cartesian-bbox.h"
#include "cartesian-create.h"
#include "cartesian-grid.h"
#include "cartesian-motion.h"
#include "cartesian-plot.h"




namespace rack {




/// First, commands applicable in any Cartesian product generation, not only radar compositeing.

// This is defined here because Create needs this
class CartesianProj : public drain::SimpleCommand<>{ // public drain::BasicCommand {

public:

	CartesianProj() : drain::SimpleCommand<>(__FUNCTION__, "Set projection", "projstr", "", "Proj.4 syntax"){
		parameters.separator = 0;
	};


	inline
	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
		mout.debug("Setting projection to local composite");

		ctx.composite.setProjection(value);
		ctx.composite.odim.projdef = value; // ?? + "     ";
	};

};



class CartesianSize : public drain::BasicCommand {

public:

	inline
	CartesianSize() : drain::BasicCommand(__FUNCTION__, "Set size of the compositing array. Does not allocate memory."){
		parameters.link("width",  width = 400, "pixels");
		parameters.link("height", height = 0, "pixels");
	};

	CartesianSize(const CartesianSize & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.parameters, cmd, *this);
	}

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		if (height == 0)
			ctx.composite.setGeometry(width, width);
		else
			ctx.composite.setGeometry(width, height);
	};

protected:

	int width;
	int height;


};



class CartesianTime : public drain::SimpleCommand<std::string> {

public:

	inline
	CartesianTime() : drain::SimpleCommand<>(__FUNCTION__, "Modify the time of the current composite. See --cTimeDecay ",
			"time", "201412091845", "YYYYmmddHHMMSS"){
		//parameters.separators.clear();
		//parameters.link("time",  time,  "201412091845", "YYYYmmddHHMMSS");
	};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		ctx.composite.odim.setTime(value);
		//std::cout << composite.odim.date << ',' << composite.odim.time << std::endl;
	};

};



class CartesianInit : public drain::BasicCommand {

public:

	CartesianInit() : drain::BasicCommand(__FUNCTION__, "Allocate memory to --cSize, applying --target and --select, if set."){};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>(); // %%
		//Composite & composite = getComposite();
		ctx.composite.dataSelector.consumeParameters(ctx.select);
		ctx.composite.allocate();
		ctx.composite.consumeTargetEncoding(ctx.targetEncoding);
	}

};




/// Commands (more) related to radar composites.


/*
CartesianBBoxTest() : drain::SimpleCommand<int>(__FUNCTION__, "Tests whether the radar range is inside the composite.",
			"mode", 0, "If no overlap and n==0, only set inputOk=false. Else exit with return value n."	) {
 */

class CompositeMethod : public drain::SimpleCommand<std::string> {
public:

	CompositeMethod() : drain::SimpleCommand<std::string>(__FUNCTION__, "Method to be used in accumulating the (weighted) values.",
			"method", "MAXIMUM", "LATEST|MAXIMUM|MAXW|AVERAGE|WAVG,p,r,bias") { // , method() {
		exec();
	};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>(); // %%
		ctx.composite.setMethod(value);  // method? Or Method obj?
		getResources().polarAccumulator.setMethod(value);  // method? Or Method obj?
	};

};



class CompositeDefaultQuality : public drain::SimpleCommand<double> {

public:

	CompositeDefaultQuality() : drain::SimpleCommand<double>(__FUNCTION__, "Set default quality (for data without quality field)",
			"weight", 0.75, "0...1"){
	};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		ctx.composite.defaultQuality = value;
	};

};


///
// TODO: change to half-time in minutes
class CompositeTimeDecay : public drain::SimpleCommand<double> {  //public drain::BasicCommand {

	public:
	CompositeTimeDecay() : drain::SimpleCommand<double>(__FUNCTION__,
			"Weight (0.9...1.0) of delay, per minute. 1.0=no decay. See --cTime and --cDecayTime", "weight", 1.0){
		//parameters.link("decay", getResources().composite.decay = 1.0, "coeff");
	};

	inline
	void exec() const {
		// Composite & composite = getComposite();
		RackContext & ctx = getContext<RackContext>();
		ctx.composite.decay = value;
	}

};


/// half-time in minutes
/**
 *    x/2 = x * (1/2)^{t/T}
 *
 *	log(0.5) = t/T*log(0.5) =>
 */
class CompositeDecayTime : public drain::SimpleCommand<int> {

	public:
	CompositeDecayTime() : drain::SimpleCommand<int>(__FUNCTION__, "Delay half-time in minutes. 0=no decay", "time", 0, "minutes"){
		//parameters.link("halftime", getResources().composite.decay = 1.0, "coeff");
	};

	inline
	void exec() const {
		// Composite & composite = getComposite();
		RackContext & ctx = getContext<RackContext>();
		if (this->value > 0){
			ctx.composite.decay = ::pow(0.5, 1.0 / static_cast<double>(value));
		}
		else {
			ctx.composite.decay = 1.0;
		}
	}


};







/*
template <class C=drain::Command>
class CartesianCmdWrapper : public drain::CommandWrapper<C,'c', CartesianSection> {
public:
	CartesianCmdWrapper(char alias = 0): drain::CommandWrapper<C,'c', CartesianSection>(alias) {};
};
*/


CartesianModule::CartesianModule(drain::CommandBank & bank) : module_t(bank) { // : CommandSection("cart"){

	drain::CommandBank::trimWords().insert("Cartesian");
	drain::CommandBank::trimWords().insert("Composite");

	//CartesianCmdWrapper<>::get
	//drain::CommandInstaller<'c', CartesianSection> installer;

	install<CartesianCreate>('c'); // cCreate2('c');
	install<CompositeAdd>(); // cAdd2;
	install<CompositeAddWeighted>(); // cAddWeighted2;
	install<CompositeDefaultQuality>(); // cDefaultQuality2;
	install<CartesianBBox>(); // cBBox2;
	install<CartesianBBoxReset>(); // cBBoxReset2;
	install<CartesianBBoxTest>(); // cBBoxTest2;
	install<CartesianBBoxTile>(); // cBBoxTile2;
	install<CartesianExtract>(); // cExtract2;
	install<CartesianGrid>(); // cGrid2;
	install<CartesianInit>(); // cInit2;
	install<CompositeMethod>(); // cMethod2;

	install<CartesianPlot>(); // cPlot2;
	install<CartesianPlotFile>(); // cPlotFile2;
	install<CartesianProj>(); // cProj2;  // shared
	install<CartesianRange>(); // cRange2;
	install<CartesianReset>(); // cReset2;
	install<CartesianSize>(); // cSize2;
	install<CartesianSpread>(); // cSpread2;
	install<CartesianTime>(); // cTime2;
	install<CompositeTimeDecay>(); // Yes, both
	install<CompositeDecayTime>(); // Yes, both
	install<CompositeCreateTile>(); // cCreateTile2;
	install<CartesianSun>("cCreateSun"); // cCreateSun2;
	install<CartesianOpticalFlow>("cOpticalFlow"); // class name: FastOpticalFlow2Op

	/// In future, unused.
	/*
	static RackLetAdapter<CompositeAdd> cAdd;
	static RackLetAdapter<CartesianAddWeighted> cAddWeighted;
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
	static RackLetAdapter<CartesianRange> cRange;
	static RackLetAdapter<CartesianReset> cReset;
	static RackLetAdapter<CartesianSize> cSize;
	static RackLetAdapter<CartesianSpread> cSpread;
	static RackLetAdapter<CartesianTime> cTime;
	static RackLetAdapter<CompositeTimeDecay> cTimeDecay;
	static RackLetAdapter<CompositeDecayTime> cCompositeDecayTime;
	static RackLetAdapter<CartesianCreate> cCreate("create", 'c');
	static RackLetAdapter<CartesianCreateTile> cCreateTile;
	static RackLetAdapter<CartesianSun> cCreateSun;
	static RackLetAdapter<CartesianOpticalFlow> opticalFlow("opticalFlow");
	//static RackLetAdapter<CartesianQuantity> cQuantity;

	// static RackLetAdapter<CmdMotionFill> motionFill("motionFill"); => drain/image/ImageOpBank => FlowAverage
	*/

}

}  // namespace rack::



// Rack
