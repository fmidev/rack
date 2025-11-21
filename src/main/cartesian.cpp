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

#include <drain/Log.h>
#include <drain/prog/CommandInstaller.h>

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


class CartesianInit : public drain::BasicCommand {

public:

	CartesianInit() : drain::BasicCommand(__FUNCTION__, "Allocate memory to --cSize, applying --target and --select, if set."){};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		ctx.composite.dataSelector.consumeParameters(ctx.select);
		ctx.composite.allocate();
		ctx.composite.consumeTargetEncoding(ctx.targetEncoding);
	}

};


class CompositeMethod : public drain::SimpleCommand<std::string> {
public:

	CompositeMethod() : drain::SimpleCommand<std::string>(__FUNCTION__,
			"Method in accumulating values on a composite.",
			"method", "MAXIMUM", "LATEST|MAXIMUM|MAXW|AVERAGE|WAVG,p,r,bias") { // , method() {
		exec(); // well, ok...
	};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>(); //
		ctx.composite.setMethod(value);  // method? Or Method obj?
		#pragma omp critical
		{
			getResources().polarAccumulator.setMethod(value);  // Ensures correct (primary) name.
		}
	};


	// Note: this is unlike meant - not parameters in order but options for the single parameter.
	virtual
	void parametersToStream(std::ostream & ostr, const std::string & indent= "  ") const override {

		AccMethodBank & bank = getAccMethodBank();
		for (const auto & entry: bank.getMap()){
			const AccumulationMethod & method = entry.second->getSource();
			ostr << indent << entry.first << " – " << method.getDescription();
			if (method.hasParameters()){
				ostr << " – parameters: ";
				method.getParameters().getKeys(ostr);
			}
			ostr << '\n';
		}

	}

};

/// First, commands applicable in any Cartesian product generation, not only radar compositeing.

// This is defined here because Create needs this
class CartesianProj : public drain::SimpleCommand<>{ // public drain::BasicCommand {

public:

	CartesianProj() : drain::SimpleCommand<>(__FUNCTION__, "Set projection", "projection", "", "<EPSG-code>|<projstr> Proj.4 syntax"){
		getParameters().separator = 0;
	};


	inline
	void exec() const final {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		mout.debug("Setting projection to local composite");

		if (!value.empty()){
			ctx.composite.setProjection(value);
		}
		// ctx.composite.odim.projdef = value; // plain EPSG code OK??
		// NOTE: could be :
		//
		// ctx.composite.odim.projdef = ctx.composite.getProjection(); // drain::Projector::SIMPLE
		// mout.attention<LOG_DEBUG>("Set projection '", value, "' to local comp:", ctx.composite.getProjection(), " (EPSG:", ctx.composite.odim.epsg, ") projdef:", ctx.composite.odim.projdef);

	};

};



class CartesianSize : public drain::BasicCommand {

public:

	inline
	CartesianSize() : drain::BasicCommand(__FUNCTION__, "Set size of the compositing array. Does not allocate memory."){
		getParameters().link("width",  width = 400, "pixels");
		getParameters().link("height", height = 0, "pixels");
	};

	CartesianSize(const CartesianSize & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		if (height == 0){
			ctx.getComposite(RackContext::PRIVATE).setGeometry(width, width);
			ctx.getComposite(RackContext::SHARED ).setGeometry(width, width);
		}
		else {
			ctx.getComposite(RackContext::PRIVATE).setGeometry(width, height);
			ctx.getComposite(RackContext::SHARED ).setGeometry(width, height);
		}
		/*
			ctx.composite.setGeometry(width, width);
		else
			ctx.composite.setGeometry(width, height);
			*/
	};

protected:

	int width = 400;
	int height = 0;


};



class CartesianTime : public drain::SimpleCommand<std::string> {

public:

	inline
	CartesianTime() : drain::SimpleCommand<>(__FUNCTION__, "Modify the time of the current composite. See --cTimeDecay ",
			"time", "201412091845", "YYYYmmddHHMMSS"){
		//getParameters().separators.clear();
		//getParameters().link("time",  time,  "201412091845", "YYYYmmddHHMMSS");
	};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		ctx.composite.odim.setTime(value);
		//std::cout << composite.odim.date << ',' << composite.odim.time << std::endl;
	};

};






/// Commands (more) related to radar composites.


/*
CartesianBBoxTest() : drain::SimpleCommand<int>(__FUNCTION__, "Tests whether the radar range is inside the composite.",
			"mode", 0, "If no overlap and n==0, only set inputOk=false. Else exit with return value n."	) {
 */



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
		//getParameters().link("decay", getResources().composite.decay = 1.0, "coeff");
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
		//getParameters().link("halftime", getResources().composite.decay = 1.0, "coeff");
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


CartesianModule::CartesianModule(drain::CommandBank & bank) : module_t(bank) {

	drain::CommandBank::trimWords().insert("Cartesian");
	drain::CommandBank::trimWords().insert("Composite");

	drain::Logger mout(__FILE__, __FUNCTION__);

//#define DRAIN_CMD_INSTALL(prefix, cmd) drain::Command & cmd = install<prefix##cmd>

	DRAIN_CMD_INSTALL(Cartesian,Create)('c');
	DRAIN_CMD_INSTALL(Composite,CreateTile)();
	DRAIN_CMD_INSTALL(Composite,Add)();
	DRAIN_CMD_INSTALL(Composite,AddWeighted)();
	linkRelatedCommands(Create, Add, AddWeighted);
	linkRelatedCommands(Create, CreateTile);

	DRAIN_CMD_INSTALL(Composite,DefaultQuality)();
	linkRelatedCommands(DefaultQuality, AddWeighted);

	DRAIN_CMD_INSTALL(Cartesian,BBox)();
	DRAIN_CMD_INSTALL(Cartesian,BBoxReset)();
	DRAIN_CMD_INSTALL(Cartesian,BBoxTest)();
	DRAIN_CMD_INSTALL(Cartesian,BBoxTile)();
	linkRelatedCommands(BBox, BBoxTile, CreateTile);
	linkRelatedCommands(BBox, BBoxTest);

	DRAIN_CMD_INSTALL(Cartesian,Extract)();
	linkRelatedCommands(Add, AddWeighted, Extract);

	DRAIN_CMD_INSTALL(Cartesian,Init)();
	DRAIN_CMD_INSTALL(Composite,Method)();
	linkRelatedCommands(Method, Add);

	DRAIN_CMD_INSTALL(Cartesian,Plot)();
	DRAIN_CMD_INSTALL(Cartesian,PlotFile)();
	linkRelatedCommands(Plot, PlotFile);

	DRAIN_CMD_INSTALL(Cartesian,Proj)();   // shared
	DRAIN_CMD_INSTALL(Cartesian,Reset)();
	DRAIN_CMD_INSTALL(Cartesian,Size)();
	linkRelatedCommands(BBox, Proj, Reset, Size);

	linkRelatedCommands(Init,Reset);
	linkRelatedCommands(BBoxReset,Reset);
	linkRelatedCommands(Method,Reset);

	DRAIN_CMD_INSTALL(Cartesian,Range)();
	linkRelatedCommands(Range, Size);

	DRAIN_CMD_INSTALL(Cartesian,Time)();
	DRAIN_CMD_INSTALL(Composite,TimeDecay)(); // Yes, both
	DRAIN_CMD_INSTALL(Composite,DecayTime)(); // Yes, both
	linkRelatedCommands(Time, TimeDecay, DecayTime);

	install<CartesianCreateLookup>();
	install<CartesianGrid>();
	install<CartesianSpread>();
	install<CartesianSun>("cCreateSun");
	install<CartesianOpticalFlow>("cOpticalFlow"); // class name: FastOpticalFlow2Op


}

}  // namespace rack::



// Rack
