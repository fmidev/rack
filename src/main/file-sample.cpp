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

//#include <exception>
#include <drain/Log.h>
#include <fstream>
#include <iostream>


#include <drain/util/Output.h>
#include <drain/image/Sampler.h>

#include "radar/RadarDataPicker.h"
#include "fileio.h"
#include "resources.h"

namespace rack {

template <class P>
void sampleData(const typename P::dataset_t & dataset, const Sampler & sampler, const std::string & format, std::ostream &ostr){ // const {

	//RackContext & ctx  = this->template getContext<RackContext>();

	//drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
	drain::Logger mout(__FILE__, __FUNCTION__);

	P picker(sampler.variableMap, dataset.getFirstData().odim);

	typename P::map_t dataMap;

	for (typename DataSet<typename P::src_t>::const_iterator it = dataset.begin(); it != dataset.end(); ++it){
		dataMap.insert(typename P::map_t::value_type(it->first, it->second));
	}

	const typename P::data_t & q = dataset.getQualityData();
	if (!q.data.isEmpty()){
		mout.note("using quality data, quantity=" , q.odim.quantity );
		if (q.odim.quantity.empty()){
			mout.warn(" empty data, properties: \n " ,  q.data.properties  );
		}
		dataMap.insert(typename P::map_t::value_type(q.odim.quantity, q));
	}
	else {
		mout.info("no quality data" );
	}

	sampler.sample(dataMap, picker, format, ostr);

}


void CmdOutputFile::writeSamples(const Hi5Tree & src, const std::string & filename) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	mout.info("Sample file (.dat)" );

	DataSelector selector;
	selector.setPathMatcher(ODIMPathElem::DATASET);
	selector.setParameters(ctx.select);
	//selector.convertRegExpToRanges();
	selector.setMaxCount(1);
	//selector.data.second = 0;
	mout.special("selector: " , selector );

	ODIMPath path;

	if (selector.getPath(src, path)){

		drain::Output ofstr(filename);

		mout.info("Sampling path: " , path );

		/*
		double d = NAN;
		double f = NAN;
		std::cerr << "NONNI " << NAN << ',' << d << ',' << f <<  std::endl;
		*/
		//std::cerr << "NONNI " << NAN <<  std::endl;

		const Hi5Tree & srcDataSet = src(path);

		/// Sampling parameters have been set by --sample (CmdSample)
		const Sampler & sampler = ctx.imageSampler.getSampler();

		std::ostream & ostr = ofstr;
		//ostr.width(20);
		//ostr.fill('x');
		ostr.precision(20);

		if (ctx.currentHi5 == ctx.currentPolarHi5){

			mout.debug("sampling polar data" );
			//const DataSet<PolarSrc> dataset(srcDataSet, drain::RegExp(selector.getQuantity()));
			const DataSet<PolarSrc> dataset(srcDataSet, selector.getQuantitySelector());
			mout.info("data: " , dataset );

			sampleData<PolarDataPicker>(dataset, sampler, ctx.formatStr, ofstr);

		}
		else {

			mout.debug("sampling Cartesian data: " );
			//const DataSet<CartesianSrc> dataset(srcDataSet, drain::RegExp(selector.getQuantity()));
			const DataSet<CartesianSrc> dataset(srcDataSet, selector.getQuantitySelector());
			mout.info("data: " , dataset );
			/*
			for (DataSet<CartesianSrc>::const_iterator it = dataset.begin(); it != dataset.end(); ++it){
				mout.warn("data:" , it->first );
			}
			 */

			sampleData<CartesianDataPicker>(dataset, sampler, ctx.formatStr, ofstr);

		}

	}


}






} // namespace rack
