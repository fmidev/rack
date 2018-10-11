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

#include <drain/prog/CommandAdapter.h>
#include "data/Quantity.h"


#include "product/BeamAltitudeOp.h"
#include "product/CappiOp.h"
#include "product/EchoTopOp.h"
#include "product/MaxEchoOp.h"

#include "product/DopplerOp.h"
#include "product/DopplerAvgExpOp.h"
#include "product/DopplerWindowOp.h"
#include "product/DopplerInversionOp.h"
#include "product/DopplerSamplerOp.h"


#include "product/ConvOp.h"
#include "product/RainRateOp.h"
#include "product/RainRateDPOp.h"
#include "product/RainRateDPSimpleOp.h"
//#include "product/RainRateZDROp.h"

#include "product/DrawingOp.h"
#include "product/SunShineOp.h"


#include "product/PseudoRhiOp.h"
#include "product/VerticalProfileOp.h"


#include "radar/Precipitation.h"

#include "andre/BirdOp.h"

#include "product/FunctorOp.h"


#include "products.h"

#include "productAdapter.h"


namespace rack {




ProductModule::ProductModule(const std::string & section, const std::string & prefix) : drain::CommandGroup(section, prefix){



	// Visualization of geometry etc
	static ProductAdapter<BeamAltitudeOp> beamAltitude;
	static ProductAdapter<DrawingOp> draw;

	// Polar coord met.product based on dBZ
	static ProductAdapter<CappiOp>   cappi;
	static ProductAdapter<EchoTopOp> echoTop;
	static ProductAdapter<MaxEchoOp> maxEcho;

	// Polar coord met.product based on VRAD
	static ProductAdapter<DopplerSamplerOp> dopplerSampler;  // circles
	static ProductAdapter<DopplerAvgOp> dopplerAvg;
	static ProductAdapter<DopplerAvg2Op> dopplerAvg2;
	static ProductAdapter<DopplerDevOp> dopplerDev;
	static ProductAdapter<DopplerInversionOp> dopplerInversion;
	static ProductAdapter<DopplerReprojectOp> dopplerRealias;
	static ProductAdapter<DopplerCrawlerOp> dopplerCrawler;
	static ProductAdapter<DopplerDiffPlotterOp> dopplerDiffPlotter;
	static ProductAdapter<DopplerDiffOp> dopplerDiff;
	static ProductAdapter<DopplerAvgExpOp> dopplerAvgExp;

	// Vertical met.products
	static ProductAdapter<VerticalProfileOp> verticalProfile;
	static ProductAdapter<PseudoRhiOp> pseudoRhi;




	static ProductAdapter<BirdOp> bird;

	static ProductAdapter<ConvOp> conv;

	static ProductAdapter<RainRateOp>    rainRate;
	static ProductAdapter<RainRateDPOp> rainRateDP;
	static ProductAdapter<RainRateDPSimpleOp> rainRateDPSimple;

	static ProductAdapter<rack::FunctorOp> ftor;


	// Geographical products
	static ProductAdapter<SunShineOp> sun;



}



} // namespace rack::



// Rack
