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
#include <stdexcept>

//#include <drain/VariableAssign.h>
//#include <drain/util/Fuzzy.h>
//#include <drain/image/AccumulationMethods.h>
//#include "RackOp.h"

//#include "data/Conversion.h"

//#include "radar/Extractor.h"
#include "PolarProductOp.h"



namespace rack {

using namespace drain::image;

const CoordinatePolicy PolarProductOp::polarCoordPolicy(EdgePolicy::POLAR, EdgePolicy::WRAP, EdgePolicy::LIMIT,EdgePolicy::WRAP);

PolarProductOp::PolarProductOp(const std::string & name, const std::string & description) : VolumeOp<PolarODIM>(name, description) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	allowedEncoding.link("type", odim.type = "C");
	allowedEncoding.link("gain", odim.scaling.scale);
	allowedEncoding.link("offset", odim.scaling.offset);
	// 2018
	allowedEncoding.link("undetect", odim.undetect);
	allowedEncoding.link("nodata", odim.nodata);

	allowedEncoding.link("rscale", odim.rscale);
	allowedEncoding.link("nrays", odim.area.height);
	allowedEncoding.link("nbins", odim.area.width);

	aboveSeaLevel = true;
	this->odim.product = "PPROD"; // NEW
	//this->odim.object  = "PVOL"; // consider!

	//dataSelector.orderFlags.set(DataSelector::ELANGLE, DataSelector::MIN);
	dataSelector.setOrder(DataOrder::DATA, DataOrder::MIN);
	dataSelector.setPrf(DataSelector::Prf::SINGLE);

	//dataSelector.updateBean(); // NOT NEEDED, when objects updated directly

	mout.debug2(dataSelector);

};



PolarProductOp::PolarProductOp(const PolarProductOp & op) : VolumeOp<PolarODIM>(op){
	//odim.importMap(op.odim);
	//odim.copyStruct(op.odim, op, odim); // // may contain more /less links?
	aboveSeaLevel = op.aboveSeaLevel;
	//dataSelector.setParameters(op.getParameters()); // should not be needed
	dataSelector.setParameters(op.dataSelector.getParameters()); // should not be needed
	allowedEncoding.copyStruct(op.allowedEncoding, op.odim, odim);
}


///  This method searches a geometry that "maximizes" resolution and range with respect to input data.
/**
 *  In other words, it searches for
 *  - largest beam count ( \c nrays ), unless set
 *  - smallest beam directional resolution (\c rscale ), unless set
 *  - largest range \c rscale × \c nbins , unless set
 *
 *
 */
void PolarProductOp::deriveDstGeometry(const DataSetMap<PolarSrc> & srcSweeps, PolarODIM & dstOdim, bool MINIMISE_RSCALE) const {

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL name+"(CumulativeProductOp)", __FUNCTION__);

	const bool DERIVE_NBINS  = (dstOdim.area.width == 0); // ||(dstOdim.rscale == 0.0);
	const bool DERIVE_NRAYS  = (dstOdim.area.height == 0);
	const bool DERIVE_RSCALE = (dstOdim.rscale == 0.0); // (dstOdim.geometry.width == 0); //
	//const bool MAXIMIZE_AZM_RESOLUTION = (dstOdim.area.height == 0); //
	const bool MAXIMIZE_AZM_RESOLUTION = DERIVE_NRAYS; // currently simply bound
	const bool MAXIMIZE_RANGE = (DERIVE_NBINS && DERIVE_RSCALE); // Use those producing maximal scale

	double range;
	double rangeMax = 0;
	size_t nbinsMax = 0;
	double rscaleMin = 2000;

	mout.debug2(dstOdim);

	//if (MAXIMIZE_AZM_RESOLUTION || DERIVE_NBINS || DERIVE_RSCALE){
	if (DERIVE_NBINS || DERIVE_NRAYS || DERIVE_RSCALE || MINIMISE_RSCALE){

		dstOdim.angles.clear();
		//dstOdim.angles.resize(0); // DO NOT USE clear(), it changes address of 1st elem
		//dstOdim.angles.resize(srcSweeps.size()); // DO NOT USE clear(), it changes address of 1st elem

		for (const auto & entry: srcSweeps){

			const DataSet<PolarSrc> & srcDataSet = entry.second;
			const Data<PolarSrc>       & srcData = srcDataSet.getFirstData();
			const PolarODIM            & srcOdim = srcData.odim;

			if (srcData.data.isEmpty()){
				mout.warn("srcData empty");
				continue; // warning issued later, in the main loop.
				//mout.warn("selected quantity=" , quantity , " not present in elangle=" , it->first );
			}

			dstOdim.angles.push_back(srcOdim.elangle);

			mout.debug2("testing: " , srcOdim);

			if (MAXIMIZE_AZM_RESOLUTION){
				if (srcOdim.area.height > dstOdim.area.height){
					dstOdim.area.height = srcOdim.area.height;
					mout.info("Updating dst nrays to: " , dstOdim.area.height );
				}
			}
			/*
			else if (dstOdim.area.height == 0){
				mout.info("Adapting 1st array height: ", srcOdim.area.height );
				dstOdim.area.height = srcOdim.area.height;
			}
			*/

			nbinsMax  = std::max(nbinsMax,  srcOdim.area.width);
			rscaleMin = std::min(rscaleMin, srcOdim.rscale);
			range = static_cast<double>(srcOdim.area.width) * srcOdim.rscale;
			if (range > rangeMax){
				rangeMax = range;
				if (MAXIMIZE_RANGE){
					dstOdim.area.width  = srcOdim.area.width;
					dstOdim.rscale = srcOdim.rscale;
				}
			}

		}

		if (MAXIMIZE_RANGE){
			mout.info("Applied input geometry with maximum range" ); // special<LOG_NOTICE>
		}
		else if (dstOdim.area.width ==0){
			dstOdim.area.width = rangeMax/static_cast<int>(dstOdim.rscale);
			mout.special<LOG_NOTICE>("Derived nbins=" , dstOdim.area.width );
		}
		else if (dstOdim.rscale ==0){
			dstOdim.rscale = rangeMax/static_cast<double>(dstOdim.area.width);
			mout.special<LOG_NOTICE>("Derived rscale=" , dstOdim.rscale );
		}
		else {
			// mout.debug("Adapting user-defined nbins and rscale" );
		}

		mout.info("Setting dst geometry:" , dstOdim.area.width , "bin x " , dstOdim.rscale , "m/bin (" , (dstOdim.getMaxRange()/1000.0) , "km) " );

	}
	else {
		//mout.warn("output prop" , dstOdim );
		mout.info("output properties: " , EncodingODIM(dstOdim) );
	}

	if (MINIMISE_RSCALE){
		range = dstOdim.getMaxRange();
		dstOdim.rscale = rscaleMin;
		dstOdim.area.width = ::lround(range/dstOdim.rscale);
	}

}

void PolarProductOp::determineMetadata(const DataSetMap<PolarSrc> & srcSweeps, PolarODIM & dstOdim) const {

	for (const auto & entry: srcSweeps){
		const DataSet<PolarSrc> & srcDataSet = entry.second;
		//const Data<PolarSrc>       & srcData = srcDataSet.getFirstData();
		// const PolarODIM            & srcOdim = srcData.odim;
		dstOdim.updateLenient(srcDataSet.getFirstData().odim);
	}

}



}  // namespace rack

// Rack
