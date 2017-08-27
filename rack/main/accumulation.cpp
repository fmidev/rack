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



#include "commands.h"
#include "accumulation.h"

#include "product/ProductOp.h"

namespace rack {


class PolarAdd : public BasicCommand {

public:

	PolarAdd() : BasicCommand(__FUNCTION__, "Add polar data to accumulation array.") {
	};

	void exec() const {

		MonitorSource mout(name, __FUNCTION__);

		RackResources & resources = getResources();

		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;

		if (!acc.isMethodSet()){
			acc.setMethod("AVG");
		}

		acc.dataSelector.path     = "data[0-9]+$";

		if (acc.dataSelector.quantity.empty())
			acc.dataSelector.quantity = "^(DBZH|RATE)$";

		if (!resources.select.empty()){
			acc.dataSelector.setParameters(resources.select);
			resources.select.clear();
		}

		mout.debug() << acc << mout.endl;


		std::string path = "dataset1";
		DataSelector::getPath(*resources.currentPolarHi5, acc.dataSelector, path);

		Data<PolarSrc> srcData((*resources.currentPolarHi5)(path));

		if (srcData.data.isEmpty()){
			mout.warn() << "No data found with selector:" << acc.dataSelector << ", skipping..." << mout.endl;
			return;
		}

		mout.info() << "using path=" << path << ", quantity=" << srcData.odim.quantity << mout.endl;

		const PlainData<PolarSrc> & srcQuality = srcData.getQualityData();

		if ((acc.getWidth()==0) || (acc.getHeight()==0)){
			acc.setGeometry(srcData.odim.nbins, srcData.odim.nrays);
			acc.odim.nbins  = srcData.odim.nbins;
			acc.odim.nrays  = srcData.odim.nrays;
			acc.odim.rscale = srcData.odim.rscale;
			acc.odim.gain = 0.0; // !!
		}
		else if ((srcData.odim.nbins != acc.getWidth()) || (srcData.odim.nrays != acc.getHeight())){
			mout.warn() << "Input geometry (" << srcData.odim.nbins << 'x' << srcData.odim.nrays << ')';
			mout        << " different from: " << acc.getWidth() << 'x' << acc.getHeight() << ", skipping..." << mout.endl;
			return;
		}
		else if (srcData.odim.rscale != acc.odim.rscale){
			mout.warn() << "Input rscale ("<< srcData.odim.rscale << ") " << acc.odim.rscale << ", skipping..." << mout.endl;
			return;
		}

		mout.debug() << EncodingODIM(acc.odim) << mout.endl;
		ProductOp::applyODIM(acc.odim, srcData.odim);
		ProductOp::handleEncodingRequest(acc.odim, resources.targetEncoding);

		resources.targetEncoding.clear();

		/*
		if (!resources.targetEncoding.empty()){
			acc.setTargetEncoding(resources.targetEncoding);
			resources.targetEncoding.clear();
			mout.info() << "Target encoding: " << rack::EncodingODIM(acc.odim) << mout.endl;
		}
		*/

		acc.addData(srcData, srcQuality, 1.0, 0, 0); // 1 => a::defaultQuality ?


	};

};


class PolarExtract : public SimpleCommand<std::string> {
    public: //re 
	//std::string value;

	PolarExtract() : SimpleCommand<std::string>(__FUNCTION__, "Extract polar-coordinate data that has been accumulated.",
			"channels", "dw", "Accumulation layers to be extracted") {
		// RackLet(__FUNCTION__,"Extract polar-coordinate data that has been accumulated.") {
		// parameters.reference("channels", channels, "dw", "Accumulation layers to be extracted");
	};

	void exec() const {

		MonitorSource mout(name, __FUNCTION__);

		RackResources & resources = getResources();

		HI5TREE & dst = resources.polarHi5;

		dst.clear();

		HI5TREE & dstDataSetGroup = dst["dataset1"];

		DataSetDst<PolarDst> dstProduct(dstDataSetGroup);

		RadarAccumulator<Accumulator,PolarODIM> & acc = resources.polarAccumulator;

		acc.setTargetEncoding(resources.targetEncoding);
		resources.targetEncoding.clear();

		//acc.checkInputODIM();
		PolarODIM odim(acc.odim);

		acc.extract(odim, dstProduct, value);

		odim.copyToDataSet(dstDataSetGroup);
		// odim.copyToDataSet(dstDataSetGroup);
		// dst.odim.copyToData(dstDataGroup);

		RackResources::updateCoordinatePolicy(dst, RackResources::polarLeft);
		DataSelector::updateAttributes(dst); // why not start form "dataset1" ?

		resources.currentHi5 = & dst;
		resources.currentPolarHi5 = & dst;
		resources.currentImage = NULL;
		resources.currentGrayImage = NULL;

	};


};

AccumulationModule::AccumulationModule(const std::string & section, const std::string & prefix) : drain::CommandGroup(section, prefix) {

	static RackLetAdapter<PolarAdd>     polarAdd;
	static RackLetAdapter<PolarExtract> polarExtract;


}
//static CommandEntry<PolarAdd> polarAdd("pAdd");
//static CommandEntry<PolarExtract> polarExtract("pExtract");



} // namespace rack::



// Rack
