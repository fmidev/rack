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

#include <drain/prog/CommandInstaller.h>
#include <product/EchoTopOp.h>
#include <product/EchoTopOpOld.h>
#include "data/Quantity.h"


#include "product/BeamAltitudeOp.h"
#include "product/CappiOp.h"
#include "product/MaxEchoOp.h"

#include "product/PolarSlidingWindowOp.h"

#include "product/DopplerOp.h"
#include "product/DopplerAvgExpOp.h"
#include "product/DopplerWindowOp.h"
#include "product/DopplerDevOp.h"
#include "product/DopplerInversionOp.h"
#include "product/DopplerSamplerOp.h"


#include "product/ConvOp.h"
#include "product/RainRateOp.h"
#include "product/RainRateDPOp.h"
//#include "product/RainRateDPSimpleOp.h"
//#include "product/RainRateZDROp.h"

#include "product/DrawingOp.h"
#include "product/SunShineOp.h"


#include "product/PseudoRhiOp.h"
#include "product/VerticalProfileOp.h"


#include "radar/Precipitation.h"


// AnDRe
#include "andre/BirdOp.h"
#include "andre/BioMetOp.h"
#include "andre/ClutterOp.h"

#include "product/FunctorOp.h"


#include "products.h"
#include "resources.h"


namespace rack {


/// "Virtual" command extracting one sweep - actually only modifying selector
class CmdSweep : public drain::BasicCommand {

public:

	CmdSweep() : drain::BasicCommand(__FUNCTION__, "Return a single sweep") {
		getParameters().link("quantity", quantity = "DBZH");
		getParameters().link("index", elevIndex = 0);
	};

	CmdSweep(const CmdSweep &cmd) : drain::BasicCommand(cmd), elevIndex(0) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}



	// COnsider selector
	std::string quantity;
	size_t elevIndex;

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		// ODIMPathElem elem(ODIMPathElem::DATASET, 1+elevIndex);
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		//ctx.select = drain::sprinter(parameters.getMap(), "", ",", "=").str();
		std::stringstream sstr;
		sstr << "dataset" << (1+elevIndex) << ',' << "quantity=" << quantity;
		ctx.select = sstr.str();
		mout.note(ctx.select );
		ctx.currentHi5 = ctx.currentPolarHi5;
		ctx.unsetCurrentImages();

	}


};


// Special command for bookkeeping of products and outputQUantities.
class CmdOutputQuantity : public drain::SimpleCommand<> {

public:

	CmdOutputQuantity() : drain::SimpleCommand<>(__FUNCTION__, "Return default outout quantity","productCmd") {
	};

	typedef std::map<std::string,std::string> map_t;

	static
	map_t quantityMap;

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		mout.deprecating("future versions may use --encoding quantity=... " );


		map_t::const_iterator it = quantityMap.find(value.substr(0, value.find(',')));
		if (it != quantityMap.end()){
			mout.note("found '" , it->first , "' " );
			mout.note("setting what:quantity: " , it->second , " " );
			ctx.getStatusMap()["what:quantity"] = it->second;
		}
		else {
			mout.note(drain::sprinter(quantityMap) );
			mout.error("meteorological product '" , value , "' does not exist " );
		}

	}


};

CmdOutputQuantity::map_t CmdOutputQuantity::quantityMap;




/// Wrapper for meteorological products derived from VolumeOp.
/**
 *  \tparam T - product operator, like CappiOp.
 */
template <class OP>
class ProductCommand : public drain::BeanCommand<OP>{

public:

	ProductCommand(){
	};

	ProductCommand(const ProductCommand & cmd){
		this->bean.getParameters().copyStruct(cmd.bean.getParameters(), cmd, *this);
	};

	/// Returns a description for help functions.
	/*
	const std::string & getType() const {
		return this->bean.getOutputQuantity();
	}
	*/


	virtual
	void update() override {

		///this->bean.setParameters(params);
		RackContext & ctx  = this->template getContext<RackContext>(); // OMP

		drain::Logger mout(ctx.log, __FUNCTION__, this->bean.getName());

		mout.debug("Applying data selector and targetEncoding ");
		mout.debug("Initial data selector: ", this->bean.dataSelector);

		if (this->bean.dataSelector.consumeParameters(ctx.select)){
			mout.special("User defined select: ", this->bean.getDataSelector() );
		}

		if (!ctx.targetEncoding.empty()){
			mout.debug("Setting target parameters: ", ctx.targetEncoding );
			this->bean.setEncodingRequest(ctx.targetEncoding);
			//mout.debug2("New values: " , this->bean.odim  );
			ctx.targetEncoding.clear();
		}

		this->bean.appendResults = ctx.appendResults;
		// this->bean.outputDataVerbosity = ctx.outputDataVerbosity; // deprecated
		this->bean.outputDataVerbosity = ctx.outputDataVerbosity;
		/// Determines if also intermediate results (1) are saved. See --aStore

		mout.special<LOG_DEBUG>("My data selector: ", this->bean.getDataSelector() );

		mout.attention<LOG_DEBUG>(this->getName(), " last pars: ", this->getLastParameters());

	}

	void exec() const override {

		RackContext & ctx  = this->template getContext<RackContext>(); // OMP

		drain::Logger mout(ctx.log, __FUNCTION__, this->bean.getName());
		// mout.timestamp("BEGIN_PRODUCT");

		mout.info("Computing: ", this->bean.getName(), ' ', drain::sprinter(this->getParameters().getMap(), drain::Sprinter::cmdLineLayout)); // XXX

		// op.filter(getResources().inputHi5, getResources().polarHi5);
		// const Hi5Tree & src = ctx.getCurrentInputHi5();
		// RackContext::CURRENT not ok, it can be another polar product
		// const Hi5Tree & src = ctx.getHi5(RackContext::POLAR | RackContext::INPUT); // what about ANDRE processing?
		const Hi5Tree & src = ctx.getHi5(
				ctx.inputFlags,
				RackContext::PRIVATE|RackContext::POLAR|RackContext::INPUT,
				RackContext::SHARED |RackContext::POLAR|RackContext::INPUT
		);
		ctx.inputFlags.reset();

		// mout.warn("Private ", ctx.id);
		// mout.warn("BaseCtx ", getResources().baseCtx().id);
		/*
		RackContext::h5_role::value_t filter = RackContext::CURRENT;
		Hi5Tree & dst1  = ctx.getMyHi5(filter);
		Hi5Tree & dst1b = getResources().baseCtx().getMyHi5(filter);
		Hi5Tree & dst2  = drain::Static::get<Hdf5Context>().getMyHi5(filter);
		  mout.warn("local   ", (size_t)&dst1,  '\n');
		  mout.warn("local.b ", (size_t)&dst1b, '\n');
		  mout.warn("hdf5.b  ", (size_t)&dst2,  '\n');
		*/

		if (src.empty()){
			mout.warn("Empty, but proceeding...");
		}


		if (&src == &ctx.polarProductHi5){
			mout.warn("src = ctx.polarProductHi5");
		}

		Hi5Tree & dst = this->bean.processVolume(src, ctx.polarProductHi5);
		// mout.warn(dst);
		// hi5::Writer::writeFile("test1.h5", dst);

		// More ODIM compliant. Also attributes["prodpars"] written, with full list.
		dst[ODIMPathElem::WHAT].data.attributes["prodpar"] = this->getLastParameters();
		if (ODIM::versionFlagger.isSet(ODIM::RACK_EXTENSIONS)){
			dst[ODIMPathElem::WHAT].data.attributes["rack_product"] = this->getName();
		}

		// mout.warn("updateInternalAttributes:...  ", dst);
		DataTools::updateInternalAttributes(dst);
		ctx.currentPolarHi5 = & ctx.polarProductHi5; // if cartesian, be careful with this...
		ctx.currentHi5      = & ctx.polarProductHi5;

		mout.special("modified code: currentImage");
		ctx.findImage();
		// mout.timestamp("END_PRODUCT");

		// hi5::Writer::writeFile("test2.h5", dst);
	};


	virtual
	inline
	std::ostream & toStream(std::ostream & ostr) const {
		ostr << "adapterName" << ": " << this->bean;
		return ostr;
	}

protected:

	//	mutable std::string descriptionFull;

};




/*! Connects meteorological products to program core.
 *
 */

/// Wraps OP of class ProductOp to a Command of class ProductCommand<OP>
template <class OP>
drain::Command & ProductModule::install(char alias){  // = 0 TODO: EMBED "install2"
	static const OP op;
	std::string name = op.getName(); // OP()
	drain::CommandBank::deriveCmdName(name, getPrefix());
	drain::Command  & cmd = cmdBank.add<ProductCommand<OP> >(name);
	cmd.section = getSection().index;
	// drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.special(name , "\n -> " , op.getOutputQuantity() , "\t test:" , op.getOutputQuantity("TEST") );
	CmdOutputQuantity::quantityMap[name] =  op.getOutputQuantity();
	return cmd;
}





//CmdSweep sweepCmd;

ProductModule::ProductModule(drain::CommandBank & cmdBank) : module_t(cmdBank){

	drain::CommandBank::trimWords().insert("Op");

	//ProductInstaller installer(drain::getCommandBank());
	//ProductModule & installer = *this;


	// Visualization of geometry etc
	install<BeamAltitudeOp>();
	install<DrawingOp>();

	// Polar coord met.product based on dBZ
	install<CappiOp>(); //    cappi;
	install<EchoTopOp>(); //  echoTop;
	install<EchoTopOpOld>(); //  echoTop;

	install<MaxEchoOp>(); //  maxEcho;

	//install<PolarSlidingWindowOp<RadarWindowAvg> > test;
	//PolarSlidingWindowOp<RadarWindowAvg<drain::FuzzyStep<double,double> > test;
	//RadarWindowAvg<drain::FuzzyStep<double,double> > test;rack::RadarWindowConfig
	//DopplerDevWindow test;

	//install<PolarSlidingWindowOp<RadarWindowAvg<RadarWindowConfig> > > test;
	install<PolarSlidingAvgOp>(); //  test;

	// Polar coord met.product based on VRAD
	install<DopplerSamplerOp>(); //  dopplerSampler;  // circles
	install<DopplerAvgOp> (); // dopplerAvg;
	//install<DopplerAvg2Op> dopplerAvg2;
	install<DopplerDevOp> (); // dopplerDev;
	install<DopplerInversionOp> (); // dopplerInversion;
	install<DopplerReprojectOp>(); //  dopplerRealias;
	install<DopplerCrawlerOp>(); //  dopplerCrawler;
	install<DopplerDiffPlotterOp>(); //  dopplerDiffPlotter;
	install<DopplerDiffOp>(); //  dopplerDiff;
	install<DopplerAvgExpOp>(); //  dopplerAvgExp;
	install<DopplerEccentricityOp>(); //  dopplerEccentricity;

	// Vertical met.products
	install<VerticalProfileOp>(); //  verticalProfile;
	install<PseudoRhiOp>(); //  pseudoRhi;



	// AnDRe detectors installed as products
	install<BirdOp>(); //
	install<BiometOp>(); //  TODO HIDDEN
	install<ClutterOp>(); //

	install<ConvOp>(); //  conv;

	install<RainRateOp>(); //     rainRate;
	install<RainRateDPOp>(); //  rainRateDP;
	// install<RainRateDPSimpleOp> rainRateDPSimple;

	install<rack::FunctorOp>(); //

	// Geographical products
	install<SunShineOp>(); //  sun;


	const drain::FlagResolver::ivalue_t SECTION = getSection().index;
	const char PREFIX = getPrefix();

	static CmdSweep sweepCmd;
	cmdBank.addExternal(PREFIX, sweepCmd).section = SECTION;

	/*
	static CmdInputObject inputSelectCmd;
	cmdBank.addExternal(PREFIX, inputSelectCmd).section = SECTION;
	*/

	// Special command
	static CmdOutputQuantity outputQuantityCmd;
	cmdBank.addExternal(PREFIX, outputQuantityCmd).section = SECTION;

}



} // namespace rack::



// Rack
