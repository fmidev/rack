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
#include "data/Quantity.h"


#include "product/BeamAltitudeOp.h"
#include "product/CappiOp.h"
#include "product/EchoTopOp.h"
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

#include "andre/BirdOp.h"

#include "product/FunctorOp.h"


#include "products.h"
#include "resources.h"


namespace rack {


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
	const std::string & getType() const {
		return this->bean.getOutputQuantity();
	}


	virtual
	void update() {

		///this->bean.setParameters(params);
		RackContext & ctx  = this->template getContext<RackContext>(); // OMP

		drain::Logger mout(ctx.log, __FUNCTION__, this->bean.getName());

		mout.debug() << "Applying data selector and targetEncoding " << mout.endl;

		if (this->bean.dataSelector.consumeParameters(ctx.select)){
			mout.special() << "User defined select: " << this->bean.getDataSelector() << mout.endl;
		}

		if (!ctx.targetEncoding.empty()){
			mout.debug() << "Setting target parameters: " << ctx.targetEncoding << mout.endl;
			this->bean.setEncodingRequest(ctx.targetEncoding);
			//mout.debug2() << "New values: " << this->bean.odim  << mout.endl;
			ctx.targetEncoding.clear();
		}


	}

	void exec() const {

		RackContext & ctx  = this->template getContext<RackContext>(); // OMP

		drain::Logger mout(ctx.log, __FUNCTION__, this->bean.getName());
		mout.timestamp("BEGIN_PRODUCT");

		mout.debug() << "Running: " << this->bean.getName() << mout.endl;

		//op.filter(getResources().inputHi5, getResources().polarHi5);
		//const Hi5Tree & src = ctx.getCurrentInputHi5();
		// RackContext::CURRENT not ok, it can be another polar product
		const Hi5Tree & src = ctx.getHi5(RackContext::INPUT); // what about ANDRE processing?

		// if (only if) ctx.append, then ctx? shared?
		Hi5Tree & dst = ctx.polarHi5; //getTarget();  //For AnDRe ops, src serves also as dst.

		if (&src == &dst){
			mout.warn() << "src=dst" << mout.endl;
		}

		//mout.warn() << dst << mout.endl;
		this->bean.processVolume(src, dst);
		// hi5::Writer::writeFile("test1.h5", dst);

		DataTools::updateCoordinatePolicy(dst, RackResources::polarLeft);
		DataTools::updateInternalAttributes(dst);
		ctx.currentPolarHi5 = & dst; // if cartesian, be careful with this...
		ctx.currentHi5      = & dst;

		mout.timestamp("END_PRODUCT");

		// hi5::Writer::writeFile("test2.h5", dst);
	};


	virtual
	inline
	std::ostream & toOstream(std::ostream & ostr) const {
		ostr << "adapterName" << ": " << this->bean;
		return ostr;
	}

protected:

	//	mutable std::string descriptionFull;

};


/// Wraps OP of class ProductOp to a Command of class ProductCommand<OP>

/*
class ProductInstaller: public drain::CommandInstaller<'p',ProductSection>{

public:

	ProductInstaller(drain::CommandBank & bank): drain::CommandInstaller<'p',ProductSection>(bank){
	}

	// todo: ProductInstaller(bank)

	template <class OP>
	//ProductCommand<OP>
	drain::Command & install(char alias = 0){  // TODO: EMBED "install2"
		std::string name = OP().getName();
		drain::CommandBank::deriveCmdName(name, getPrefix());
		drain::Command  & cmd = cmdBank.add<ProductCommand<OP> >(name);
		cmd.section = getSection().index;
		return cmd;
	}

};
*/



ProductModule::ProductModule(drain::CommandBank & cmdBank) : module_t(cmdBank){

	drain::CommandBank::trimWords().insert("Op");

	//std::cerr << __FUNCTION__ << std::endl;
	//return;

	//ProductInstaller installer(drain::getCommandBank());
	//ProductModule & installer = *this;

	// Visualization of geometry etc
	install<BeamAltitudeOp>(); //  beamAltitude;
	install<DrawingOp>();// draw;

	// Polar coord met.product based on dBZ
	install<CappiOp>(); //    cappi;
	install<EchoTopOp>(); //  echoTop;
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




	install<BirdOp>(); //  bird;

	install<ConvOp>(); //  conv;

	install<RainRateOp>(); //     rainRate;
	install<RainRateDPOp>(); //  rainRateDP;
	// install<RainRateDPSimpleOp> rainRateDPSimple;

	install<rack::FunctorOp>(); //  ftor;


	// Geographical products
	install<SunShineOp>(); //  sun;



}



} // namespace rack::



// Rack
