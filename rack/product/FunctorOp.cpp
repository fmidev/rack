/*

    Copyright 2010-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)



    This file is part of Rack.

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
#include <drain/util/Fuzzy.h>
#include <drain/util/FunctorBank.h>
#include <drain/image/File.h>

#include "radar/Geometry.h"

#include "radar/Analysis.h"

#include "FunctorOp.h"
//#include "DataSelector.h"



namespace rack
{

using namespace drain::image;


void FunctorOp::processData(const Data<PolarSrc> & src, Data<PolarDst> &dst) const {

	drain::Logger mout(name,__FUNCTION__);

	drain::FunctorBank & functorBank = drain::getFunctorBank();

	try {
		//const std::string ftorStr = ftorSetup.substr(iStart, iEnd-iStart);
		size_t iSeparator = ftorSetup.find(':');
		const bool PARAMS = (iSeparator != std::string::npos);

		const std::string ftorName   = PARAMS ? ftorSetup.substr(0, iSeparator) : ftorSetup;
		const std::string ftorParams = PARAMS ? ftorSetup.substr(iSeparator+1) : "";
		drain::UnaryFunctor & ftor = functorBank.get(ftorName).clone();
		ftor.setParameters(ftorParams, '=', ':');
		//	ftors.push_back(ftor);

		const double dstMax = dst.data.getMax<double>();
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
	catch (std::exception & e){
		mout.error() << e.what() << mout.endl;
	}
	/*
	for (long int i = 0; i < dst.odim.nbins; ++i) {
		//std::cerr << i << '\t' << ground << " m\t h=" << h << " >" << h/odim.gain << " m\n";
		h = Geometry::heightFromEtaGround(eta, i*dst.odim.rscale)/gainMetres;
		if (h < max)
			dst.data.put(i, h);
		else
			dst.data.put(i, dst.odim.undetect); // quality?
	}
	*/


}


}
