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

#include "drain/util/Input.h"

#include "commands.h"
#include "accumulation.h"

#include "product/ProductOp.h"
//#include "radar/Geometry.h"
#include "radar/Coordinates.h"

namespace rack {

class PolarSite : public BasicCommand {

public:

	double lon;
	double lat;

	PolarSite() : BasicCommand(__FUNCTION__,
			"Set radar size location of the accumulated data. Also size etc., if --encoding set."){
		parameters.reference("lon", lon = 0.0, "degrees");
		parameters.reference("lat", lat = 0.0, "degrees");
	}

	void exec() const {

		Logger mout(__FUNCTION__, __FILE__);

		RackResources & resources = getResources();
		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;
		acc.odim.lat = lat;
		acc.odim.lon = lon;

		if (!resources.targetEncoding.empty()){
			allocateAccumulator();
		}

	}

	static
	void allocateAccumulator() {

		Logger mout(__FUNCTION__, __FILE__);

		RackResources & resources = getResources();
		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;
		if ((acc.getWidth()==0) || (acc.getHeight()==0)){

			if (resources.targetEncoding.empty()){
				mout.error() << "missing --encoding to set polar geometry: " << acc.odim.getKeys() << mout.endl;
				return;
			}

			acc.odim.addShortKeys();
			acc.odim.setValues(resources.targetEncoding);

			acc.setGeometry(acc.odim.geometry.width, acc.odim.geometry.height);
			acc.count.fill(1); // no weight, ie "undetect" values.
			// mout.warn() << acc.odim  << mout.endl;

		}
	}

};

// TODO. combine Cartesian & Polar
class PolarPlot : public BasicCommand {

public:

	double lon;
	double lat;
	double value;
	double weight;

	PolarPlot() : BasicCommand(__FUNCTION__, "Add a single data point."){
		parameters.reference("lon", lon = 0.0, "longitude");
		parameters.reference("lat", lat = 0.0, "latitude");
		parameters.reference("value", value = 0.0, "value");
		parameters.reference("weight", weight = 1.0, "weight");
	};


	void exec() const {

		Logger mout(__FUNCTION__, __FILE__);

		RackResources & resources = getResources();

		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;

		// if ((acc.odim.lat ???) || (acc.getHeight()==0)){
		PolarSite::allocateAccumulator();

		///  Converts the polar coordinates for a radar to geographical coordinates.
		RadarProj proj;
		proj.setSiteLocationDeg(acc.odim.lon, acc.odim.lat);
		proj.setLatLonProjection();
		mout.warn() << proj << mout.endl;
		//drain::Point2D<double> point(lon*drain::DEG2RAD, lat*drain::DEG2RAD);
		//proj.projectInv(point.x, point.y); // lon, lat,
		//mout.warn() << " " << this->getParameters() << " => " << point << mout.endl;

		drain::Point2D<double> point;
		proj.projectInv(lon*drain::DEG2RAD, lat*drain::DEG2RAD, point.x, point.y); // lon, lat,
		//point.x *= drain::RAD2DEG;
		//point.y *= drain::RAD2DEG;
		//mout.warn() << " " << this->getParameters() << " => " << point << mout.endl;
		// double r = point.x*point.x + point.y*point.y;
		// mout.warn() << "AEQD coords: " << point << mout.endl;

	};


};

// TODO. combine Cartesian & Polar
class PolarPlotFile : public SimpleCommand<std::string> {

public:

	PolarPlotFile() : SimpleCommand<>(__FUNCTION__, "Plot file containing rows '<lat> <lon> <value> [weight] (skipped...)'.",
			"file", "", "filename"){
	};

	void exec() const;
};

void PolarPlotFile::exec() const {

	drain::Logger mout(__FUNCTION__, __FILE__); // = getResources().mout; = getResources().mout;

	RackResources & resources = getResources();

	//Composite & composite = resources.composite;
	RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;

	PolarSite::allocateAccumulator();

	RadarProj proj;
	proj.setSiteLocationDeg(acc.odim.lon, acc.odim.lat);
	proj.setLatLonProjection();
	mout.warn() << proj << mout.endl;

	drain::Point2D<double> metricCoord;
	drain::Point2D<int> radarCoord;

	drain::Input input(value);

	std::string line;
	drain::Point2D<double> geoCoord;
	double d;
	double w;

	std::stringstream sstr;

	double r2, r, azm;
	r = acc.odim.getMaxRange();
	const double r2max = r*r;
	size_t addr;

	while ( getline((std::istream &)input, line) ){

		line = drain::StringTools::trim(line.substr(0, line.find_first_of("%#")));

		if (!line.empty()){

			sstr.clear();
			sstr.str(line);
			sstr >> geoCoord.x >> geoCoord.y;
			sstr >> d;
			if (!sstr.eof())
				sstr >> w;
			else
				w = 1.0;
			// std::cout << '#' << line << '\n';
			// std::cout << lon << ',' << lat << '\t' << d << ',' << w << '\n';
			proj.projectInv(geoCoord.x*drain::DEG2RAD, geoCoord.y*drain::DEG2RAD, metricCoord.x, metricCoord.y); // lon, lat,
			//std::cout << geoCoord << '\t' << d << ',' << w << '\t' << metricCoord << '\n';

			r2 = metricCoord.x*metricCoord.x + metricCoord.y*metricCoord.y;
			if (r2 <= r2max){
				radarCoord.x = acc.odim.getBinIndex(sqrt(r2));
				azm = atan2(metricCoord.x, metricCoord.y);
				if (azm < 0.0)
					azm += (2.0*M_PI);
				radarCoord.y = acc.odim.getRayIndex(azm);
				mout.debug(2) << "adding " << geoCoord << " => "<< radarCoord << mout.endl;
				addr = acc.data.address(radarCoord.x, radarCoord.y);
				acc.add(addr, d, w);
			}
			else {
				mout.debug(2) << "outside radar range: " << geoCoord << " => " << metricCoord << mout.endl;
			}
		}

	}

	//ifstr.close();

}





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

		// acc.dataSelector.path = "data[0-9]+$";
		// acc.dataSelector.path = ""; // remove after deprecated

		if (acc.dataSelector.quantity.empty())
			acc.dataSelector.quantity = "^(DBZH|RATE)$";

		if (!resources.select.empty()){
			acc.dataSelector.setParameters(resources.select);
			resources.select.clear();
		}

		mout.debug() << acc << mout.endl;

		// NEW
		//acc.dataSelector.convertRegExpToRanges();

		//selector.data.max = 0;
		//mout.note() << "selector: " << selector << mout.endl;
		ODIMPath path;
		acc.dataSelector.pathMatcher.setElems(ODIMPathElem::DATASET);
		acc.dataSelector.getPath3(*resources.currentHi5, path);  //, ODIMPathElem::DATASET); //, true);

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
			acc.setGeometry(srcData.odim.geometry.width, srcData.odim.geometry.height);
			acc.odim.type = "S";
			acc.odim.geometry.width  = srcData.odim.geometry.width;
			acc.odim.geometry.height  = srcData.odim.geometry.height;
			acc.odim.rscale = srcData.odim.rscale;
			acc.odim.scale = 0.0; // !!
			acc.odim.ACCnum = 0;
		}
		else if ((srcData.odim.geometry.width != acc.getWidth()) || (srcData.odim.geometry.height != acc.getHeight())){
			mout.warn() << "Input geometry (" << srcData.odim.geometry.width << 'x' << srcData.odim.geometry.height << ')';
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

public:

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

	static RackLetAdapter<PolarSite>        polarSite;
	static RackLetAdapter<PolarPlot>        polarPlot;
	static RackLetAdapter<PolarPlotFile>    polarPlotFile;

	static RackLetAdapter<PolarAdd>         polarAdd;
	static RackLetAdapter<PolarAddWeighted> polarAddW;
	static RackLetAdapter<PolarExtract>     polarExtract;


}
//static CommandEntry<PolarAdd> polarAdd("pAdd");
//static CommandEntry<PolarExtract> polarExtract("pExtract");



} // namespace rack::
