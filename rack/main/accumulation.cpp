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

	PolarAdd() : BasicCommand(__FUNCTION__, "Add polar data to accumulation array."), weight(1.0) { //, count(1) {
	};

	void exec() const {

		Logger mout(__FUNCTION__, __FILE__);

		RackResources & resources = getResources();

		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;

		if (!acc.isMethodSet()){
			acc.setMethod("AVERAGE"); // TODO: add pMethod command?
		}

		// acc.dataSelector.path     = "data[0-9]+$";
		acc.dataSelector.path = ""; // remove after deprecated

		if (acc.dataSelector.quantity.empty())
			acc.dataSelector.quantity = "^(DBZH|RATE)$";

		if (!resources.select.empty()){
			acc.dataSelector.setParameters(resources.select);
			resources.select.clear();
		}

		mout.debug() << acc << mout.endl;

		// NEW
		acc.dataSelector.convertRegExpToRanges();

		//selector.data.max = 0;
		//mout.note() << "selector: " << selector << mout.endl;
		ODIMPath path;
		acc.dataSelector.getPathNEW(*resources.currentHi5, path, ODIMPathElem::DATASET); //, true);

		const DataSet<PolarSrc> srcDataSet((*resources.currentPolarHi5)(path));
		const Data<PolarSrc>  & srcData = srcDataSet.getFirstData();
		// mout.note() << "input ACCnum " << srcData.odim.ACCnum << mout.endl;

		if (srcData.data.isEmpty()){
			mout.warn() << "No data found with selector:" << acc.dataSelector << ", skipping..." << mout.endl;
			return;
		}

		mout.info() << "using path=" << path << ", quantity=" << srcData.odim.quantity << mout.endl;

		const bool LOCAL_QUALITY = srcData.hasQuality();
		if (LOCAL_QUALITY)
			mout.info() << "has local quality" << mout.endl;

		const PlainData<PolarSrc> & srcQuality = LOCAL_QUALITY ? srcData.getQualityData() : srcDataSet.getQualityData();
		mout.debug() << "quality: " << srcQuality.odim << mout.endl;


		if ((acc.getWidth()==0) || (acc.getHeight()==0)){
			//acc.odim.update(srcData.odim);
			acc.setGeometry(srcData.odim.nbins, srcData.odim.nrays);
			acc.odim.type = "S";
			acc.odim.nbins  = srcData.odim.nbins;
			acc.odim.nrays  = srcData.odim.nrays;
			acc.odim.rscale = srcData.odim.rscale;
			acc.odim.gain = 0.0; // !!
			acc.odim.ACCnum = 0;
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

		mout.debug() << "Encoding:" << EncodingODIM(acc.odim) << mout.endl;
		ProductBase::applyODIM(acc.odim, srcData.odim, true);
		if (!resources.targetEncoding.empty()){
			// ProductBase::completeEncoding(acc.odim, resources.targetEncoding);
			mout.info() << "targetEncoding already at this stage may be deprecating(?) - use it only in extraction "  << mout.endl;
			acc.setTargetEncoding(resources.targetEncoding);
			resources.targetEncoding.clear();
		}



		double coeff = 1.0;
		const std::string & name = acc.getMethod().name;
		if ((name == "AVERAGE") || (name == "WAVG")){

		 	mout.debug() << "avg-type method=" << acc.getMethod() << ' ';

			if (acc.odim.ACCnum > 1){
				mout << ", dividing weight by current ACCnum=" << acc.odim.ACCnum << ' ';
				coeff = coeff / static_cast<double>(acc.odim.ACCnum);
			}

			if (srcData.odim.ACCnum > 1) {
				coeff = coeff*static_cast<double>(srcData.odim.ACCnum);
				mout << ", rescaling weight with src ACCnum=" << srcData.odim.ACCnum;
			}

			mout << mout.endl;
		}


		if (srcDataSet.hasQuality("COUNT")){
			mout.note() << "COUNT field detected, using restored weighted sums" << mout.endl;
			const PlainData<PolarSrc> & srcCount = srcDataSet.getQualityData("COUNT");
			acc.addData(srcData, srcQuality, srcCount);
		}
		else {
			if (coeff != 1.0)
				mout.note() << "No COUNT field detected, ACCnum "<< acc.odim.ACCnum << '+' << srcData.odim.ACCnum << ", using tuned weight: " << coeff << '*' << weight  << mout.endl;
			acc.addData(srcData, srcQuality, coeff*weight, 0, 0);
		}

		//acc.counter += std::max(1L, srcData.odim.ACCnum);
		//acc.odim.ACCnum += std::max(1L, srcData.odim.ACCnum);

	};

protected:

	PolarAdd(const std::string & name, const std::string & description) : BasicCommand(name, description), weight(1.0){};

	double weight;

	//long int count;


};


class PolarAddWeighted : public PolarAdd {

public:

	PolarAddWeighted() : PolarAdd(__FUNCTION__, "Adds the current product to the composite applying weight.") {
		parameters.reference("weight", this->weight = weight, "0...1");//, count(1) {
	};

};

class PolarExtract : public SimpleCommand<std::string> {
    public: //re 
	//std::string value;

	PolarExtract() : SimpleCommand<std::string>(__FUNCTION__, "Extract polar-coordinate data that has been accumulated.",
			"channels", "dw", "Layers: data,count,weight,std.deviation") {
		// RackLet(__FUNCTION__,"Extract polar-coordinate data that has been accumulated.") {
		// parameters.reference("channels", channels, "dw", "Accumulation layers to be extracted");
	};

	void exec() const {

		Logger mout(__FUNCTION__, __FILE__);

		RackResources & resources = getResources();

		RadarAccumulator<Accumulator,PolarODIM> & acc = resources.polarAccumulator;

		/// Clumsy?
		if (!resources.targetEncoding.empty()){
			acc.setTargetEncoding(resources.targetEncoding);
			resources.targetEncoding.clear();
		}
		//
		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(acc.odim, acc.odim.quantity, acc.getTargetEncoding());

		mout.info() << "acc.odim Encoding " << EncodingODIM(acc.odim) << mout.endl;

		Hi5Tree & dst = resources.polarHi5;
		dst.clear();

		Hi5Tree & dstDataSetGroup = dst["dataset1"];
		DataSet<PolarDst> dstProduct(dstDataSetGroup);

		acc.odim.object = "SCAN";
		acc.odim.product = "ACC";

		//acc.odim.ACCnum += acc.count;
		//acc.count = 0; // NOTE CHECK - if data re-added?
		acc.extract(acc.odim, dstProduct, value);
		acc.odim.ACCnum += acc.counter;

		ODIM::copyToH5<ODIMPathElem::DATASET>(acc.odim, dstDataSetGroup); //@dstProduct odim.copyToDataSet(dstDataSetGroup);
		// dst.odim.copyToData(dstDataGroup); ??
		DataTools::updateCoordinatePolicy(dst, RackResources::polarLeft);
		DataTools::updateInternalAttributes(dst); // why not start form "dataset1" ?

		//mout.warn() << "ODIM lat" <<  acc.odim << mout.endl;
		ODIM::copyToH5<ODIMPathElem::ROOT>(acc.odim, dst);

		dst["how"].data.attributes["software"]   = __RACK__;
		dst["how"].data.attributes["sw_version"] = __RACK_VERSION__;

		resources.currentHi5 = & dst;
		resources.currentPolarHi5 = & dst;
		resources.currentImage = NULL;
		resources.currentGrayImage = NULL;

	};


};

AccumulationModule::AccumulationModule(const std::string & section, const std::string & prefix) : drain::CommandGroup(section, prefix) {

	static RackLetAdapter<PolarAdd>         polarAdd;
	static RackLetAdapter<PolarAddWeighted> polarAddW;
	static RackLetAdapter<PolarExtract> polarExtract;


}
//static CommandEntry<PolarAdd> polarAdd("pAdd");
//static CommandEntry<PolarExtract> polarExtract("pExtract");



} // namespace rack::
