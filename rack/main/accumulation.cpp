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



#include "commands.h"
#include "accumulation.h"

#include "product/ProductOp.h"

namespace rack {


class PolarAdd : public BasicCommand {

public:

	PolarAdd() : BasicCommand(__FUNCTION__, "Add polar data to accumulation array."), weight(1.0) { //, count(1) {
	};


	void exec() const {

		Logger mout(name, __FUNCTION__);

		RackResources & resources = getResources();

		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;

		if (!acc.isMethodSet()){
			acc.setMethod("AVERAGE"); // TODO: add pMethod command?
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
		//mout.note() << "input ACCnum " << srcData.odim.ACCnum << mout.endl;

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
		ProductBase::applyODIM(acc.odim, srcData.odim);
		ProductBase::handleEncodingRequest(acc.odim, resources.targetEncoding);
		resources.targetEncoding.clear();

		//mout.warn() << srcData.odim.ACCnum << mout.endl;
		//const long int count = std::min(1l, srcData.odim.ACCnum);
		//const double weight = (acc.getMethod().name == "AVERAGE") ? static_cast<double>(count) : 1.0;
		const std::string & name = acc.getMethod().name;
		if ((name == "AVERAGE") || (name == "WAVG")){
			//const long int count = acc.odim.ACCnum + 1;
			//mout.note() << "input ACCnum " << srcData.odim.ACCnum << mout.endl;
			acc.odim.ACCnum +=  std::max(1l, srcData.odim.ACCnum);
			mout.info() << "avg-type method, dividing weight by " << acc.odim.ACCnum << mout.endl;
			acc.addData(srcData, srcQuality, weight/static_cast<double>(acc.odim.ACCnum), 0, 0);
			//acc.odim.ACCnum = count;
		}
		else
			acc.addData(srcData, srcQuality, weight, 0, 0); // 1 => a::defaultQuality ?
		// acc.count.fill(count);
		// acc.odim.ACCnum = count;

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

		Logger mout(name, __FUNCTION__);

		RackResources & resources = getResources();

		RadarAccumulator<Accumulator,PolarODIM> & acc = resources.polarAccumulator;
		acc.setTargetEncoding(resources.targetEncoding);
		resources.targetEncoding.clear();


		HI5TREE & dst = resources.polarHi5;
		dst.clear();

		HI5TREE & dstDataSetGroup = dst["dataset1"];
		DataSet<PolarDst> dstProduct(dstDataSetGroup);

		PolarODIM odim(acc.odim);
		odim.object = "SCAN";
		odim.product = "ACC";

		acc.extract(odim, dstProduct, value);

		ODIM::copyToH5<ODIM::DATASET>(odim, dstDataSetGroup); //@dstProduct odim.copyToDataSet(dstDataSetGroup);
		// dst.odim.copyToData(dstDataGroup); ??
		DataTools::updateCoordinatePolicy(dst, RackResources::polarLeft);
		DataTools::updateAttributes(dst); // why not start form "dataset1" ?

		ODIM::copyToH5<ODIM::ROOT>(odim, dst);

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


