/**


    Copyright 2014 - 2015   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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


*/

#include <drain/prog/CommandAdapter.h>
#include "data/Quantity.h"


#include "product/BeamAltitudeOp.h"
#include "product/CappiOp.h"
#include "product/EchoTopOp.h"
#include "product/MaxEchoOp.h"

#include "product/DopplerOp.h"
#include "product/DopplerWindowOp.h"
#include "product/DopplerDeAliasOp.h"


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
	static ProductAdapter<DopplerOp> dopplerMapper;  // circles
	static ProductAdapter<DopplerAvgOp> dopplerAvg;
	static ProductAdapter<DopplerDevOp> dopplerDev;
	static ProductAdapter<DopplerDeAliasOp> dopplerDeAlias;

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


