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
#include "drain/util/Log.h"
#include "drain/util/Point.h"
#include "drain/prog/CommandBankUtils.h"
#include "drain/prog/CommandSections.h"
#include "drain/prog/CommandInstaller.h"


#include "product/ProductOp.h"
#include "radar/RadarProj.h"

#include "resources.h"
#include "accumulation.h"

namespace rack {

/**
 *   This command should be called in main thread.
 */
//class PolarSite : public drain::BasicCommand {
class PolarSite : public drain::SimpleCommand<drain::Point2D<double>::tuple_t> {

public:

	//double lon;
	//double lat;

	PolarSite() : drain::SimpleCommand<drain::Point2D<double>::tuple_t> (__FUNCTION__,
			"Set radar size location of the accumulated data. Also size etc., if --encoding set.", "location", {25.2,60.1}){
		// parameters.link("lon", lon = 0.0, "degrees");
		// parameters.link("lat", lat = 0.0, "degrees");
	}

	/*
	PolarSite() : drain::BasicCommand(__FUNCTION__,
			"Set radar size location of the accumulated data. Also size etc., if --encoding set."){
		parameters.link("lon", lon = 0.0, "degrees");
		parameters.link("lat", lat = 0.0, "degrees");
	}
	*/

	//parameters.copyStruct(op.getParameters(), op, *this);

	void exec() const {

		RackResources & resources = getResources();
		RackContext ctx = resources.baseCtx();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;
		acc.odim.lon = value[0];
		acc.odim.lat = value[1];

		if (!ctx.targetEncoding.empty()){
			allocateAccumulator();
			// ctx.targetEncoding.clear?
		}

	}

	static
	void allocateAccumulator() {

		RackResources & resources = getResources();
		RackContext ctx = resources.baseCtx();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;
		if ((acc.accArray.getWidth()==0) || (acc.accArray.getHeight()==0)){

			if (resources.baseCtx().targetEncoding.empty()){
				mout.error("missing --encoding to set polar geometry: " , acc.odim.getKeys() );
				return;
			}

			acc.odim.addShortKeys();
			acc.odim.setValues(resources.baseCtx().targetEncoding);

			acc.accArray.setGeometry(acc.odim.area.width, acc.odim.area.height);
			acc.accArray.count.fill(1); // no weight, ie "undetect" values.
			// mout.warn(acc.odim  );

		}
	}

};

// TODO. combine Cartesian & Polar
class PolarPlot : public drain::BasicCommand {

public:

	double lon;
	double lat;
	double value;
	double weight;

	PolarPlot() : drain::BasicCommand(__FUNCTION__, "Add a single data point."){
		parameters.link("lon", lon = 0.0, "longitude");
		parameters.link("lat", lat = 0.0, "latitude");
		parameters.link("value", value = 0.0, "value");
		parameters.link("weight", weight = 1.0, "weight");
	};


	void exec() const {

		RackResources & resources = getResources();
		RackContext ctx = resources.baseCtx();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;

		// if ((acc.odim.lat ???) || (acc.getHeight()==0)){
		PolarSite::allocateAccumulator();

		///  Converts the polar coordinates for a radar to geographical coordinates.
		RadarProj proj;
		proj.setSiteLocationDeg(acc.odim.lon, acc.odim.lat);
		proj.setLatLonProjection();
		mout.note(proj );
		//drain::Point2D<double> point(lon*drain::DEG2RAD, lat*drain::DEG2RAD);
		//proj.projectInv(point.x, point.y); // lon, lat,
		//mout.warn(" " , this->getParameters() , " => " , point );

		drain::Point2D<double> point;
		proj.projectInv(lon*drain::DEG2RAD, lat*drain::DEG2RAD, point.x, point.y); // lon, lat,
		//point.x *= drain::RAD2DEG;
		//point.y *= drain::RAD2DEG;
		//mout.warn(" " , this->getParameters() , " => " , point );
		// double r = point.x*point.x + point.y*point.y;
		// mout.warn("AEQD coords: " , point );

	};


};

// TODO. combine Cartesian & Polar
/**
 *   This command can be run in thread.
 */
class PolarPlotFile : public drain::SimpleCommand<std::string> {

public:

	PolarPlotFile() : drain::SimpleCommand<>(__FUNCTION__, "Plot file containing rows '<lat> <lon> <value> [weight] (skipped...)'.",
			"file", "", "filename"){
	};

	void exec() const;
};

void PolarPlotFile::exec() const {

	RackResources & resources = getResources();
	RackContext ctx = resources.baseCtx();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = getResources().mout; = getResources().mout;

	//Composite & composite = resources.composite;
	RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;

	PolarSite::allocateAccumulator();

	RadarProj proj;
	proj.setSiteLocationDeg(acc.odim.lon, acc.odim.lat);
	proj.setLatLonProjection();
	mout.warn(proj );

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
				mout.debug3() << "adding " << geoCoord << " => "<< radarCoord << mout.endl;
				addr = acc.accArray.data.address(radarCoord.x, radarCoord.y);
				acc.add(addr, d, w);
			}
			else {
				mout.debug3() << "outside radar range: " << geoCoord << " => " << metricCoord << mout.endl;
			}
		}

	}

	//ifstr.close();

}


///
/**
 *   This command can be run in thread.
 */
class PolarAdd : public drain::BasicCommand {

public:

	PolarAdd() : drain::BasicCommand(__FUNCTION__, "Add polar data to accumulation array."), weight(1.0) { //, count(1) {
	};

	void exec() const {


		RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		RadarAccumulator<Accumulator,PolarODIM>	& acc = resources.polarAccumulator;

		//if (!acc.isMethodSet()){
		acc.setMethod("AVERAGE"); // TODO: add pMethod command?
		//}
		// OR check at least

		// acc.dataSelector.path = "data[0-9]+$";
		// acc.dataSelector.path = ""; // remove after deprecated

		if (acc.dataSelector.quantity.empty())
			acc.dataSelector.quantity = "^(DBZH|RATE)$";

		/// OR: resources.baseCtx().select
		acc.dataSelector.consumeParameters(ctx.select);

		mout.debug(acc );
		mout.info(acc.dataSelector );
		mout.debug("DataCoder::undetectQualityCoeff: " , DataCoder::undetectQualityCoeff );


		const Hi5Tree & src = ctx.getHi5(RackContext::CURRENT|RackContext::POLAR);

		//selector.data.second = 0;
		//mout.note("selector: " , selector );

		ODIMPath path;
		acc.dataSelector.pathMatcher.set(ODIMPathElem::DATASET); // .setElems(
		acc.dataSelector.getPath(src, path);  //, ODIMPathElem::DATASET); //, true);

		const DataSet<PolarSrc> srcDataSet(src(path));
		const Data<PolarSrc>  & srcData = srcDataSet.getFirstData();
		// mout.note("input ACCnum " , srcData.odim.ACCnum );

		if (srcData.data.isEmpty()){
			mout.warn("No data found with selector:" , acc.dataSelector , ", skipping..." );
			return;
		}

		mout.info("using path=" , path , ", quantity=" , srcData.odim.quantity );

		const bool LOCAL_QUALITY = srcData.hasQuality();
		if (LOCAL_QUALITY)
			mout.info("has local quality" );

		const PlainData<PolarSrc> & srcQuality = LOCAL_QUALITY ? srcData.getQualityData() : srcDataSet.getQualityData();
		mout.debug("quality: " , srcQuality.odim );


		if ((acc.accArray.getWidth()==0) || (acc.accArray.getHeight()==0)){
			//acc.odim.update(srcData.odim);
			acc.accArray.setGeometry(srcData.odim.area.width, srcData.odim.area.height);
			acc.odim.type = "S";
			acc.odim.area.width  = srcData.odim.area.width;
			acc.odim.area.height  = srcData.odim.area.height;
			acc.odim.rscale = srcData.odim.rscale;
			acc.odim.scaling.scale = 0.0; // !!
			acc.odim.ACCnum = 0;
		}
		else if ((srcData.odim.area.width != acc.accArray.getWidth()) || (srcData.odim.area.height != acc.accArray.getHeight())){
			mout.warn() << "Input geometry (" << srcData.odim.area.width << 'x' << srcData.odim.area.height << ')';
			mout        << " different from: " << acc.accArray.getWidth() << 'x' << acc.accArray.getHeight() << ", skipping..." << mout.endl;
			return;
		}
		else if (srcData.odim.rscale != acc.odim.rscale){
			mout.warn("Input rscale (", srcData.odim.rscale , ") " , acc.odim.rscale , ", skipping..." );
			return;
		}

		mout.debug("Encoding:" , EncodingODIM(acc.odim) );
		ProductBase::applyODIM(acc.odim, srcData.odim, true);
		if (!resources.baseCtx().targetEncoding.empty()){
			// ProductBase::completeEncoding(acc.odim, resources.baseCtx().targetEncoding);
			mout.info("targetEncoding already at this stage may be deprecating(?) - use it only in extraction "  );
			acc.setTargetEncoding(resources.baseCtx().targetEncoding);
			resources.baseCtx().targetEncoding.clear();
		}



		double coeff = 1.0;
		const std::string & name = acc.getMethod().getName();
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

		// MAINS
		if (srcDataSet.hasQuality("COUNT")){
			mout.note("COUNT field detected, using restored weighted sums" );
			const PlainData<PolarSrc> & srcCount = srcDataSet.getQualityData("COUNT");
			acc.addData(srcData, srcQuality, srcCount);
		}
		else {
			if (coeff != 1.0)
				mout.note("No COUNT field detected, ACCnum ", acc.odim.ACCnum , '+' , srcData.odim.ACCnum , ", using tuned weight: " , coeff , '*' , weight  );
			acc.addData(srcData, srcQuality, coeff*weight, 0, 0);
		}

		//acc.counter += std::max(1L, srcData.odim.ACCnum);
		//acc.odim.ACCnum += std::max(1L, srcData.odim.ACCnum);

	};

protected:

	PolarAdd(const std::string & name, const std::string & description) : drain::BasicCommand(name, description), weight(1.0){};

	double weight;

	//long int count;


};


class PolarAddWeighted : public PolarAdd {

public:

	PolarAddWeighted() : PolarAdd(__FUNCTION__, "Adds the current product to the composite applying weight.") {
		parameters.link("weight", this->weight = weight, "0...1");//, count(1) {
	};

};

// TODO. combine Cartesian & Polar
/**
 *   Typically, this command is run in main thread, as the accumulation array is shared.
 *   It is also possible to run this in thread, storing the product in local polar product.
 */
class PolarExtract : public drain::SimpleCommand<std::string> {

public:

	PolarExtract() : drain::SimpleCommand<std::string>(__FUNCTION__, "Extract polar-coordinate data that has been accumulated.",
			"channels", "dw", "Layers: data,count,weight,std.deviation") {
		// RackLet(__FUNCTION__,"Extract polar-coordinate data that has been accumulated.") {
		// parameters.link("channels", channels, "dw", "Accumulation layers to be extracted");
	};

	void exec() const {

		RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);


		RadarAccumulator<Accumulator,PolarODIM> & acc = resources.polarAccumulator;

		/// Clumsy?
		if (!resources.baseCtx().targetEncoding.empty()){
			acc.setTargetEncoding(resources.baseCtx().targetEncoding);
			resources.baseCtx().targetEncoding.clear();
		}
		//
		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(acc.odim, acc.odim.quantity, acc.getTargetEncoding());

		mout.info("acc.odim Encoding " , EncodingODIM(acc.odim) );

		Hi5Tree & dst = ctx.polarProductHi5;
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

		//mout.warn("ODIM lat" ,  acc.odim );
		ODIM::copyToH5<ODIMPathElem::ROOT>(acc.odim, dst);

		ProductBase::setODIMsoftwareVersion(dst[ODIMPathElem::HOW].data.attributes);
		//dst[ODIMPathElem::HOW].data.attributes["software"]   = __RACK__;
		//dst[ODIMPathElem::HOW].data.attributes["sw_version"] = __RACK_VERSION__;

		ctx.currentHi5 = & dst;
		ctx.currentPolarHi5 = & dst;
		ctx.currentImage = NULL;
		ctx.currentGrayImage = NULL;

	};


};

/*
struct AccumulationSection : public drain::CommandSection {

	inline
	AccumulationSection(): CommandSection("accumulation"){
		// hello(__FUNCTION__);
		drain::CommandBank::trimWords().insert("Polar");
		// std::cout << __FUNCTION__ << ' ' << std::endl;
	};

};
*/


/*
class AccumulationInstaller : public drain::CommandInstaller<'p', AccumulationModule> {
public:
	//AccumulationInstaller(char alias = 0): drain::CommandWrapper<C,'p',128>(alias) {};
};
*/


AccumulationModule::AccumulationModule(drain::CommandBank & cmdBank) : module_t(cmdBank) { // : CommandSection("accumulation"){

	drain::CommandBank::trimWords().insert("Polar");

	// drain::CommandInstaller<'p', AccumulationSection>installer;
	//AccumulationInstaller installer;

	install<PolarSite>(); //        polarSite;
	install<PolarPlot>(); //        polarPlot;
	install<PolarPlotFile>(); //     polarPlotFile;

	install<PolarAdd>(); //         polarAdd;
	install<PolarAddWeighted>(); //  polarAddW;
	install<PolarExtract>(); //      polarExtract;

}


/*
void AccumulationModule::initialize(){ // : CommandSection("accumulation"){
	drain::CommandBank::trimWords().insert("Polar");

	//drain::CommandInstaller<'p', AccumulationSection>installer;

	install<PolarSite>(); //        polarSite;
	install<PolarPlot>(); //        polarPlot;
	install<PolarPlotFile>(); //     polarPlotFile;

	install<PolarAdd>(); //         polarAdd;
	install<PolarAddWeighted>(); //  polarAddW;
	install<PolarExtract>(); //      polarExtract;
}
*/


} // namespace rack::
