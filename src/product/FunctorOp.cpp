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
#include <drain/image/ImageFile.h>
#include "drain/util/Fuzzy.h"
#include "drain/util/FunctorBank.h"
#include "radar/Geometry.h"

#include "radar/Analysis.h"

#include "FunctorOp.h"
//#include "DataSelector.h"



namespace rack
{

using namespace drain::image;


void FunctorOp::processData(const Data<PolarSrc> & src, Data<PolarDst> &dst) const {

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL name,__FUNCTION__);

	const drain::FunctorBank & functorBank = drain::getFunctorBank();

	try {

		std::string ftorName;
		std::string ftorParams;
		drain::StringTools::split2(ftorSetup, ftorName, ftorParams, ":");
		/*
		size_t iSeparator = ftorSetup.find(':');
		const bool PARAMS = (iSeparator != std::string::npos);
		const std::string ftorName   = PARAMS ? ftorSetup.substr(0, iSeparator) : ftorSetup;
		const std::string ftorParams = PARAMS ? ftorSetup.substr(iSeparator+1) : "";
		*/

		mout.experimental("name: ", ftorName);
		mout.experimental("pars: ", ftorParams);

		// NEW
		drain::UniCloner<drain::UnaryFunctor> localBank(functorBank);
		drain::UnaryFunctor & ftor = localBank.getCloned(ftorName);
		// OLD
		// drain::UnaryFunctor & ftor = functorBank.clone(ftorName);

		ftor.setParameters(ftorParams, '=', ':');
		//	ftors.push_back(ftor);

		const double dstMax = dst.data.getConf().getTypeMax<double>();
		typedef drain::typeLimiter<double> Limiter;
		Limiter::value_t limit = drain::Type::call<Limiter>(dst.data.getType());

		Image::const_iterator s  = src.data.begin();
		Image::iterator d = dst.data.begin();
		double s2;
		if (true){ //this->LIMIT){
			while (d != dst.data.end()){
				s2 = static_cast<double>(*s);
				if (s2 == src.odim.nodata)
					*d = dst.odim.nodata;
				else if (s2 == src.odim.undetect)
					*d = dst.odim.nodata;
				else
					*d = limit(dstMax * ftor(src.odim.scaleForward(s2)));
				++s;
				++d;
			}
		}
		else {
			while (d != dst.data.end()){
				s2 = static_cast<double>(*s);
				if (s2 == src.odim.nodata)
					*d = dst.odim.nodata;
				else if (s2 == src.odim.undetect)
					*d = dst.odim.undetect;
				else
					*d = dstMax * ftor(src.odim.scaleForward(s2));
				++s;
				++d;
			}
		}

	}
	catch (const std::exception & e){
		mout.warn()  << functorBank << mout;
		mout.error() << e.what() << mout;
	}
	/*
	for (long int i = 0; i < dst.odim.geometry.width; ++i) {
		//std::cerr << i << '\t' << ground << " m\t h=" << h << " >" << h/odim.scaling.scale << " m\n";
		h = Geometry::heightFromEtaGround(eta, i*dst.odim.rscale)/gainMetres;
		if (h < max)
			dst.data.put(i, h);
		else
			dst.data.put(i, dst.odim.undetect); // quality?
	}
	*/


}


}

// Rack
