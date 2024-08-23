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

#include "drain/util/Output.h" // DEBUGGING

#include "data/SourceODIM.h"
#include "main/palette-manager.h"

#include "radar/Coordinates.h"
#include "radar/Geometry.h"
#include "radar/PolarSmoother.h"

#include "EchoTop2Op.h"
#include "PolarSlidingWindowOp.h"

/* OLD STYLE: CLASS / 255.0 = QIND
template <>
const drain::FlagResolver::dict_t drain::EnumDict<rack::EchoTop2Op::Reliability>::dict = {
		{"UNDEFINED",     rack::EchoTop2Op::Reliability::UNDEFINED},
		{"UNDERSHOOTING", rack::EchoTop2Op::Reliability::UNDERSHOOTING},
		{"OVERSHOOTING",  rack::EchoTop2Op::Reliability::OVERSHOOTING},
		{"INTERPOLATION", rack::EchoTop2Op::Reliability::INTERPOLATION},
};
*/

namespace rack
{

EchoTop2Op::EchoTop2Op(double threshold) :
		PolarProductOp(__FUNCTION__,"Estimates maximum altitude of given reflectivity"),
		threshold(threshold), weights(1.0, 0.8, 0.6, 0.4, 0.2) {

	parameters.link("threshold", this->threshold, "reflectivity limit (dB)");
	parameters.link("reference", this->reference.tuple(), "'dry point' of low reflectivity and high altitude [dBZ:m]");
	parameters.link("undetectValue",    this->undetectReflectivity, "reflectivity replacing 'undetect' [dBZ]"); // if set, reference will be applied also here
#ifndef NDEBUG
	parameters.link("weights", this->weights.tuple(), "weights for INTERPOLATION, INTERP_UNDET, EXTRAP_UP, EXTRAP_DOWN, CLEAR");
	parameters.link("referenceWindow", refWindow.tuple() = {0.0,0.0}, "optional reference window [metres,degrees]");
	parameters.link("EXTENDED", this->EXTENDED_OUTPUT, "store also DBZ_SLOPE and CLASS");

	// parameters.link("_EXTENDED", this->EXTENDED, "append classified data");
#endif

	dataSelector.setQuantities("^DBZH$");

	this->undetectReflectivity = getQuantityMap().get("DBZH").undetectValue;

	odim.product = "ETOP";
	odim.quantity = "HGHT"; // METRES
	odim.type = "S";
	odim.scaling.scale = 0.0;

	/*
	 The original idea was to "encode" weights to class indices 0...255. But that affects their order in legends.
	const EncodingODIM & odimQ = getQuantityMap().get("QIND")['C'];
	const drain::image::Palette & cp = PaletteManager::getPalette("CLASS-ETOP");
	weights.interpolation = odimQ.scaleForward(cp.getEntryByCode("interpolated").first);
	weights.interpolation_dry  = odimQ.scaleForward(cp.getEntryByCode("interpolated.undetect").first);
	weights.truncated = odimQ.scaleForward(cp.getEntryByCode("extrapolated.dry").first);
	weights.weak          = odimQ.scaleForward(cp.getEntryByCode("weak").first);
	weights.clear         = weights.interpolation_dry; // slightly lower than weights.interpolation
	*/

	// drain::Logger(__FILE__, __LINE__, __FUNCTION__).attention("weights: ", weights);
	// drain::Logger(__FILE__, __LINE__, __FUNCTION__).attention("this: ", this);

};

/*
class Etop2WindowCore : public drain::image::WindowCoreBase {

public:
	PolarODIM odimSrc;
};
*/

/*
class Etop2WindowConfig : public RadarWindowConfig {
public:
};
*/

class EtopWindowCore : public drain::image::WeightedWindowCore {

public:

	inline
	EtopWindowCore(){
	}

public:

	int N = 0;
	double sumZd  = 0.0;
	double sumZd2 = 0.0;
	double sumV   = 0.0;
	double sumZdV = 0.0;

};


//class Etop2Window : public SlidingRadarWindowBase<RadarWindowConfig,Etop2WindowCore> {
//class Etop2Window : public SlidingRadarWindowBase<RadarWindowConfig, EtopWindowCore> {
class Etop2Window : public SlidingWindow<RadarWindowConfig, EtopWindowCore> {


public:

	static
	const EncodingODIM sharedODIM;  // float (double)


	Etop2Window(const RadarWindowConfig & conf) : // , const PolarODIM & odimOut
		// SlidingRadarWindowBase<RadarWindowConfig,Etop2WindowCore>(conf){};
		// SlidingRadarWindowBase<RadarWindowConfig, EtopWindowCore>(conf){
		SlidingWindow<RadarWindowConfig, EtopWindowCore>(conf){
		this->resetAtEdges = false;
	};

	// DOES NOT WORK  PolarSrc vs PolarDST!
	// void setSrc(const PlainData<PolarSrc> & srcGradient, const PlainData<PolarSrc> & srcCoeff){


	inline
	void setSrc(const drain::image::Image & srcGradient, const drain::image::Image & srcCoeff){
		srcGradientView.setView(srcGradient);
		srcGradientView.setCoordinatePolicy(polarLeftCoords);
		// odimGradient.updateFromCastableMap(srcGradient.odim); // unused (yet)
		srcCoeffView.setView(srcCoeff);
		srcCoeffView.setCoordinatePolicy(polarLeftCoords);
		// odimCoeff.updateFromCastableMap(srcCoeff.odim);// unused (yet)
	}

	inline
	void setDst(PlainData<PolarDst> & dstHeight, PlainData<PolarDst> & dstReflectivity){
		// dstHeight.data.setCoordinatePolicy(polarLeftCoords); // not needed
		// dstHeight.setGeometry(srcGradientView.getGeometry());
		dstHeightView.setView(dstHeight.data);
		dstHeight.copyEncoding(sharedODIM_HGHT);

		// dstReflectivity.data.setCoordinatePolicy(polarLeftCoords); // not needed
		// dstReflectivity.setGeometry(srcGradientView.getGeometry());
		dstReflectivityView.setView(dstReflectivity.data);
		dstHeight.copyEncoding(sharedODIM_DBZH);
	}

	 void initialize() final {
		 drain::Logger mout(__FILE__, __FUNCTION__);

		 setImageLimits();
		 setLoopLimits();
		 this->location.setLocation(0,0);
		 this->coordinateHandler.setPolicy(polarLeftCoords);


		 //void updatePixelSize(const PolarODIM & inputODIM);
		 // TÄHÄN updatePixelSize();
		 this->setSize(5,5);

		 mout.special(*this);
		 mout.special(this->coordinateHandler);
		 mout.special("srcGrad: ", srcGradientView);
		 mout.special("srcCoeff: ", srcCoeffView);
		 mout.special("dstHeight: ", dstHeightView);
		 mout.special("dstReflectivity: ", dstReflectivityView);

	 };

	 void setImageLimits() const final{
		 //this->coordinateHandler.setPolicy(polarLeftCoords);

		 this->coordinateHandler.set(srcGradientView.getGeometry());
	 }

protected:
	// Echo top gradient matrix, second term.
	drain::image::ImageView srcGradientView;
	EncodingODIM odimGradient;
	drain::image::ImageView srcCoeffView;
	EncodingODIM odimCoeff;
	// drain::image::ImageView src2;

public:

	// Echo top Reference: height
	// drain::image::ImageView dst2;
	static
	const EncodingODIM sharedODIM_HGHT;
	drain::image::ImageView dstHeightView;

	static
	const EncodingODIM sharedODIM_DBZH;
	drain::image::ImageView dstReflectivityView;



/*
	inline
	void setSrcFrame2(const drain::image::ImageFrame & srcFrame){
		//drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		// mout.debug2("src props for odim: " , src.getProperties() );
		// drain::image::SlidingWindow<C, R>::setSrcFrame(src);
		// mout.debug("src Scaling: " , srcFrame.getScaling() );
		//setSrcFrame(srcFrame);
		src2.setView(srcFrame);
	}

	inline
	void setDstFrame2(drain::image::ImageFrame & dstFrame){
		//drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		// mout.debug2("src props for odim: " , src.getProperties() );
		// drain::image::SlidingWindow<C, R>::setSrcFrame(src);
		// mout.debug("src Scaling: " , srcFrame.getScaling() );
		dst2.setView(dstFrame);
	}
*/

public:

	/** konsider
	inline virtual
	void initialize(){
	}
	*/

	// int sumX = 0;
	// int sumY = 0;
	virtual inline
	void addPixel(drain::Point2D<int> & p) override {
		if (this->coordinateHandler.validate(p)){
			double Zd = srcGradientView.get<double>(p); // this->location
			double V  = srcCoeffView.get<double>(p);
			if (sharedODIM.isValue(Zd)){
				sumZd2 += Zd*Zd;
				sumZd  += Zd;  // NOTE minus
				N++; //      += 1.0;
				sumZdV += Zd*V; // NOTE minus
				sumV   += V;
				/*
				N      += 1.0;
				sumHd  += Hd;  // NOTE minus
				sumHd2 += Hd*Hd;
				sumK   += k;
				sumHdK += Hd*k; // NOTE minus
				*/
			}
		}
	};

	virtual inline
	void removePixel(drain::Point2D<int> & p) override {
		if (this->coordinateHandler.validate(p)){
			double Zd = srcGradientView.get<double>(p); // this->location
			double V  = srcCoeffView.get<double>(p);
			if (sharedODIM.isValue(Zd)){
				sumZd2 -= Zd*Zd;
				sumZd  -= Zd;  // NOTE minus
				N--; //      += 1.0;
				sumZdV -= Zd*V; // NOTE minus
				sumV   -= V;
				/*
				N      -= 1.0;
				sumHd  -= Hd;  // NOTE minus
				sumHd2 -= Hd*Hd;
				sumK   -= k;
				sumHdK -= Hd*k; // NOTE minus
				*/
			}
		}
	};


	virtual inline
	void write() final {

		/**
		 *    |a  bc| = |N       -sumHd |
		 *    |bc  d|   |-sumHd   sumHd2|
		 */

		//const double & a = N;
		double a  = sumZd2;
		double bc = -sumZd;
		double d  = static_cast<double>(N);
		//const double div = 1.0 / (a*d  - bc*bc); // N*sumHd - sumHd*sumHd);
		double div = (a*d  - bc*bc); // N*sumHd - sumHd*sumHd);

		if (::abs(div) < 0.0001){
			// if (::isnan(div)){
			return;
		}

		div = 1.0/div;

		// TODO: quality from div
		/**
		 *    |A  BC| = div * | d  -bc|
		 *    |BC  D|         |-bc   a|
		 */
		double A  =  div*d;
		double BC =  div*(-bc);
		double D  =  div*a;


		// dstHeightView.put(this->location, 12000 + this->location.x*10);
		// dstReflectivityView.put(this->location, 32768 + (this->location.y-180)*100);

		// NOTE: sumV "requires" minus sign
		if (this->location.x == this->location.y){
			// std::cout << "sumZd=" << sumZd << "\t sumZd2=" << sumZd2 << "\t sumZdV=" << sumZdV  << "\t N=" << N << "\t div=" << div << '\n';
			// std::cout << "  -> dBZ=" << (BC*sumZdV -  D*sumV) << " H=" << (A*sumZdV - BC*sumV) << " = " << (A*sumZdV) << " - " << (BC*sumV)<< '\n';
			std::cout << "  dBZ=" << (BC*sumZdV -  D*sumV) << " H=" << (A*sumZdV - BC*sumV) << "\t N=" << N << "\t div=" << div << '\n';
		}

		dstHeightView.put(this->location,       Etop2Window::sharedODIM_HGHT.scaleInverse(A*sumZdV  - BC*sumV) );
		dstReflectivityView.put(this->location, Etop2Window::sharedODIM_DBZH.scaleInverse(BC*sumZdV -  D*sumV) );
		// dst.put(this->location,  Etop2Window::sharedODIM_HGHT.scaleInverse(A*sumZdV + BC*sumV)  );
		// dst2.put(this->location, Etop2Window::sharedODIM_DBZH.scaleInverse(BC*sumZdV +  D*sumV) );

	}

};

// const EncodingODIM Etop2Window::sharedODIM('d'); // consider float
const EncodingODIM Etop2Window::sharedODIM('f'); // consider float


//const EncodingODIM Etop2Window::sharedODIM_DBZH('S', 1.0, 0, 0xfffe, 0xffff); // 65535
// const EncodingODIM Etop2Window::sharedODIM_HGHT('S', 0.1, -1, 0xfffe, 0xffff); // max: 65533 metres
const EncodingODIM Etop2Window::sharedODIM_HGHT('S', 0.9, -32768, 0xfffe, 0xffff); // max: 65533 metres
const EncodingODIM Etop2Window::sharedODIM_DBZH('S', 0.01, -327.68); // 65535
// const EncodingODIM Etop2Window::sharedODIM_DBZH(getQuantityMap().get("DBZH").get('S')); // 65535

//void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;
void EchoTop2Op::computeSingleProduct(const DataSetMap<src_t> & srcSweeps, DataSet<dst_t> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	// mout.warn("Starting...");
	// mout.special<LOG_NOTICE>(*this);

	const QuantityMap & quantityMap = getQuantityMap();

	const bool USE_KILOMETRES = ODIM::versionFlagger.isSet(ODIM::KILOMETRES);

	if (USE_KILOMETRES){
		mout.attention("ODIM version ", ODIM::versionFlagger, ", using kilometres (not metres).");
		mout.advice("Use --odim <version> to change. See --help odim");
	}
	else {
		mout.info("ODIM version ", ODIM::versionFlagger, ", using metres (not kilometres).");
	}

	const double odimVersionMetricCoeff = USE_KILOMETRES ? 0.001 : 1.0;

	/// Shared, because input data MAY contain different encoding(s)
	const EncodingODIM & dbzOdim = quantityMap.get("DBZH").get('S');
	//dstOdim.updateFromCastableMap(dbzOdim);

	// The actual result
	Data<dst_t> & dstEchoTop = dstProduct.getData(odim.quantity);
	dstEchoTop.odim.updateFromCastableMap(odim);
	deriveDstGeometry(srcSweeps, dstEchoTop.odim);
	determineMetadata(srcSweeps, dstEchoTop.odim);

	mout.debug("ODIM scaling 1: ", dstEchoTop.odim.scaling);


	// applyODIM(dstEchoTop.odim, odim);
	dstEchoTop.setEncoding(odim.type, targetEncoding);

	if ((dstEchoTop.odim.scaling.scale == 0) || (dstEchoTop.odim.scaling.scale == 1.0)){ // ~ unset
		//odim.scaling.setPhysicalRange(0.0, 25.6);
		dstEchoTop.odim.setRange(0.0, 20000.0 * odimVersionMetricCoeff);
		// dstEchoTop.odim.scaling.scale *= odimVersionMetricCoeff;
		// mout.attention("ODIM scaling 2: ", dstEchoTop.odim.scaling);
	}

	dstEchoTop.setGeometry(dstEchoTop.odim);
	// mout.attention("ODIM scaling 3: ", dstEchoTop.odim.scaling);
	// mout.special(dstEchoTop.odim);

	// clumsy
	const drain::image::AreaGeometry & area = dstEchoTop.odim.getGeometry();

	bool COMPUTE_REFERENCE = true;

	PlainData<dst_t> & dstETOP_Zd = dstProduct.getData("ETOP_AUX_GRAD");
	dstETOP_Zd.copyEncoding(Etop2Window::sharedODIM);
	dstETOP_Zd.setExcluded(!COMPUTE_REFERENCE);

	PlainData<dst_t> & dstETOP_V  = dstProduct.getData("ETOP_AUX_COEFF");
	dstETOP_V.copyEncoding(Etop2Window::sharedODIM);
	dstETOP_V.setExcluded(!COMPUTE_REFERENCE);
	// dstETOP_hd.setEncoding(typeid(float));

	if (COMPUTE_REFERENCE){
		dstETOP_Zd.setGeometry(area);
		dstETOP_Zd.data.fill(Etop2Window::sharedODIM.nodata);
		dstETOP_V.setGeometry(area);
		dstETOP_V.data.fill(Etop2Window::sharedODIM.nodata);
	}



	// setGeometry(dstOdim, dstEchoTop);
	// setEncoding(dstOdim, dstEchoTop);
	// dstEchoTop.setGeometry(area);

	const std::type_info & type = dstEchoTop.data.getType();
	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(type);
	//dstEchoTop.data.getScaling().g

	/*
	double d1 = drain::Type::call<drain::typeMin, double>(type);
	double d2 = drain::Type::call<drain::typeMax, double>(type);
	for (int i = 0; i < 100; ++i) {
		double d = i * (d2-d1)/100.0 + d1;
		double m = dstEchoTop.odim.scaleForward(d);
		mout.note("limiter:", i, '\t', d, '\t', m, '\t', dstEchoTop.odim.scaleInverse(m), '\t', limit(dstEchoTop.odim.scaleInverse(m)));
	}
	mout.special("limiter:", limit);
	*/


	PlainData<dst_t> & dstQuality = dstProduct.getQualityData();
	quantityMap.setQuantityDefaults(dstQuality, "QIND", "C");
	dstQuality.setGeometry(area);

	/// Quality (confidence) (using storage type encoding)
	MethodWeights<short int> WEIGHTS; // "assumes"
	WEIGHTS.interpolation = dstQuality.odim.scaleInverse(weights.interpolation);
	WEIGHTS.interpolation_dry  = dstQuality.odim.scaleInverse(weights.interpolation_dry);
	WEIGHTS.extrapolation_up   = dstQuality.odim.scaleInverse(weights.extrapolation_up);
	WEIGHTS.extrapolation_down = dstQuality.odim.scaleInverse(weights.extrapolation_down);
	WEIGHTS.clear         = dstQuality.odim.scaleInverse(weights.clear);
	// WEIGHTS::value_t
	short int WEIGHT = 0;

	mout.debug("WEIGHTS: ", WEIGHTS);

	/// Class codes (using storage type encoding)
	MethodWeights<unsigned char> CLASSES;
	// CLASSES::value_t
	unsigned char CLASS = 0;

// #ifndef NDEBUG

	PlainData<dst_t> & dstClass = dstEchoTop.getQualityData("CLASS-ETOP");
	quantityMap.setQuantityDefaults(dstClass, "CLASS", "C");

	PlainData<dst_t> & dstSlope = dstProduct.getData("DBZ-SLOPE");
	// Z(dB) decay per metre (or kilometre until ODIM 2.3?)
	dstSlope.setEncoding(typeid(unsigned short));
	//dstSlope.setPhysicalRange(-0.01, +0.01); // m/dBZ
	dstSlope.setPhysicalRange(-10, +10); // dBZ/km
	Limiter::value_t limitSlope = drain::Type::call<Limiter>(dstSlope.data.getType());

	if (EXTENDED_OUTPUT){
		//dstEchoTop.setGeometry(dstEchoTop.odim);
		dstClass.setGeometry(dstEchoTop.odim);
		//drain::Variable & classLegend = dstClass.getWhat()["legendTEST"];
		//classLegend = "0:Clear,128:OverShooting"; // (TODO: redesign "direct" legend struct, as properties?)
		const drain::image::Palette & cp = PaletteManager::getPalette("CLASS-ETOP");
		CLASSES.clear = dstClass.odim.undetect;
		CLASSES.interpolation = cp.getEntryByCode("interpolated").first;
		CLASSES.interpolation_dry  = cp.getEntryByCode("interpolated.undetect").first; //  cp.getEntryByCode("overshooting").first;
		CLASSES.extrapolation_up = cp.getEntryByCode("strong.extrapolated").first; // cp.getEntryByCode("undershooting").first;
		CLASSES.extrapolation_down  = cp.getEntryByCode("weak").first;
		CLASSES.error = cp.getEntryByCode("error").first; // could be nodata as well


		dstSlope.setGeometry(dstEchoTop.odim);
		dstSlope.getHow()["unit"] = "m/dBZ";
		//dstSlope.getHow()["unit"] = "m/dBZ";
	}
	else {
		dstClass.setExcluded();
		dstSlope.setExcluded();
	}

// #endif

	// MAIN -----------------------------------------------------------------

	double groundDistance;


	struct MeasurementHolder : public Measurement {

		/// Pointer to data array (DBZH)
		const Data<src_t> * dataPtr = nullptr;

		/// Index of the bin at the ground distance D of all the elevation beams considered
		size_t binIndex = 0;

		/*
		/// Height from the ground surface (from radar site, ASL)
		double height = 0;

		/// Reflectivity observed at the current bin.
		double reflectivity = NAN;
		*/

		/// Reliability/confidence of the measurement value, maximal (1.0) when reflectivity is the threshold value.
		// double quality = 0.0;
	};


	// TODO: test overlapping (repeated) elangles, warn and suggest selector (PRF for example)


	/// Pointer to the measurement
	/**
	 *  - lower than the given threshold – but minimally lower
	 *  - above a measurement exceeding the threshold
	 *  - the highest possible
	 */
	MeasurementHolder *weakMsrm;

	/// Pointer to the measurement
	/**
	 *  - higher than the given threshold – but minimally higher
	 *  - below the last measurement reaching the threshold
	 *  - the highest possible
	 */
	MeasurementHolder *strongMsrm;

	std::map<double,MeasurementHolder> dbzData; // D bit slow, but for now

	double azm;
	double height; // always metres
	double slope; // EXTENDED
	int j2;

	size_t i2;

	// const bool REPLACE_UNDETECT = !::isnan(undetectReflectivity);
	// mout.attention("Replace undetect:", REPLACE_UNDETECT, " dryTop value=", undetectReflectivity, drain::TextStyle::Colour::GREEN, "test", drain::TextStyle::Colour::NO_COLOR);

	const bool USE_INTERPOLATION     = (weights.interpolation     > 0.0);
	const bool USE_INTERPOLATION_DRY = (weights.interpolation_dry > 0.0) && !::isnan(undetectReflectivity);


	// Direct array index of a pixel (i,j)
	size_t address;

	// DEBUG
	std::string outputFilename;

	for (const auto & sweep: srcSweeps){
		const Data<src_t> & src =  sweep.second.getFirstData();
		SourceODIM srcOdim(src.odim.source);
		outputFilename = drain::StringBuilder<'-'>(src.odim.date, src.odim.time, srcOdim.NOD, "vect.dat");
		break;
	}
	drain::Output output(outputFilename);


	// MAIN
	for (size_t i = 0; i < area.width; ++i){

		groundDistance = dstEchoTop.odim.getBinDistance(i); // These bins are on the ground...
		mout.debug("bin #", i, ", distance=", groundDistance);

		// Initialize: collect sweeps which are inside range.
		dbzData.clear();
		for (const auto & sweep: srcSweeps){
			const Data<src_t> & src =  sweep.second.getData("DBZH");

			int index = src.odim.getBinIndex(groundDistance);
			if (index >= 0){
				i2 = index;
				//if ((i & 15) == 0){
				if (i2 < src.odim.area.width){
					MeasurementHolder & info = dbzData[src.odim.elangle];
					info.dataPtr  = &src;
					info.binIndex = index;
					info.height = Geometry::heightFromEtaGround(src.odim.getElangleR(), groundDistance);
					/*
					if ((i & 15) == 0){
						mout.accept<LOG_DEBUG>(entry.first, " elangle=",src.odim.elangle, " altitude=", info.altitude);
					}
					*/
				}
				else {
					// mout.reject<LOG_NOTICE>(entry.first, " elangle=",src.odim.elangle);
				}
			}
		}

		/// Azimuthal loop: [0,360[ degrees
		for (size_t j = 0; j < area.height; ++j){

			azm = dstEchoTop.odim.getAzimuth(j);

			strongMsrm = nullptr;
			weakMsrm = nullptr;

			// Column loop:
			for (auto & entry: dbzData){

				MeasurementHolder & msr = entry.second;
				const Data<src_t> & srcData = *msr.dataPtr;

				j2 = srcData.odim.getRayIndex(azm);

				/*
				if (i==50){
					mout.reject<LOG_NOTICE>(j, " -> ", j2 , ", AZM=", azm);
				}
				*/

				// Raw byte value
				msr.reflectivity = srcData.data.get<double>(msr.binIndex, j2);

				if (msr.reflectivity == srcData.odim.nodata){
					// Treat as missing data indeed (like missing whole beam) - skip it
					// (This state is possible, if data has been quality controlled or radar range is smaller than image width, and image margin is filled nodata values.)
					continue;
				}

				if (msr.reflectivity == srcData.odim.undetect){ // Measured, but no echo. Treat as dry air.

					if ((strongMsrm != nullptr) && (weakMsrm == nullptr)){
						// Strong echo has been detected below, so store this measurement.
						weakMsrm = &msr;
						/* If applied, this drops "dry-top" class
						if (REPLACE_UNDETECT){
							weakMsrm->reflectivity = undetectReflectivity;
						}
						*/
						// weakMsrm->reflectivity = dbzOdim.undetect; // No effect
					}

					// dstEchoTop.data.put(i,j, ::rand() & 0xffff); // DEBUG
					continue;
				}

				// Ok, info.dBZ is a valid echo (usually precip).
				// Convert it from storage-type scaling to physical value.
				msr.reflectivity = srcData.odim.scaleForward(msr.reflectivity);

				if (msr.reflectivity >= threshold) {

					// A higher sample exceeding the threshold was found. = The highest this far.
					strongMsrm = & msr;
					weakMsrm = nullptr; // reset, "drop" the weaker (and look for a new one, from above bins).

				}
				else {

					/*
					if ((!USE_INTERPOLATION) && (strongMsrm!=nullptr)){
						// KEEP strong, it is prioritized
					}
					*/
					if (true) { //
						// Ensure start of a (new) precipitation column
						if (weakMsrm == nullptr){
							weakMsrm = & msr;
						}
						else if (msr.reflectivity > weakMsrm->reflectivity) {
							// The new measurement is closer to threshold than the current weak one.
							// -> Update.
							// Consider: is this needed? lower measurement was anyway CLOSER (in height).
							weakMsrm = & msr;
						}
					}

				}

			} // End column loop


			/// Main: compute the echo top altitude.

			address = dstEchoTop.data.address(i, j);

			if (weakMsrm != nullptr){

				// dstETOP_hd.data.put(address, ::rand());

				if (strongMsrm != nullptr){

					if (dbzOdim.isValue(weakMsrm->reflectivity)){

						// INTERPOLATION, as both values are available
						if (weakMsrm->reflectivity < strongMsrm->reflectivity){

							if (USE_INTERPOLATION){
								// EtopWin: store SLOPE_HGHT: weakMsrm->height
								slope  = getSlope(*weakMsrm, *strongMsrm);  // UNIT: m/dBZ
								WEIGHT = WEIGHTS.interpolation;
								CLASS  = CLASSES.interpolation;
								if (((i & 15) == 0) && ((j & 15) == 0)){
									//output << weakMsrm->height << ' ' << weakMsrm->reflectivity << '\t' << strongMsrm->height << ' ' << strongMsrm->reflectivity << '\n';
									output << weakMsrm->reflectivity << '\t' << weakMsrm->height << '\t' << strongMsrm->reflectivity << '\t'  << strongMsrm->height << '\n';
								}
							}
							else {
								slope = getSlope(reference, *strongMsrm);
								WEIGHT = WEIGHTS.extrapolation_up;
								CLASS  = CLASSES.extrapolation_up;
							}

							height = strongMsrm->height + slope*(threshold - strongMsrm->reflectivity); // metres

							dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height)));
							//dstQuality.data.put<int>(address, USE_INTERPOLATION ? WEIGHTS.interpolation : WEIGHTS.extrapolation_up);
							dstQuality.data.put<int>(address, WEIGHT);
//#ifndef NDEBUG
							if (EXTENDED_OUTPUT){
								// dstClass.data.put<int>(address, USE_INTERPOLATION ? CLASSES.interpolation : CLASSES.extrapolation_up);
								dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0/slope))); // dBZ/km
								dstClass.data.put<int>(address, CLASS);
							}
// #endif
							if (COMPUTE_REFERENCE){
								dstETOP_Zd.data.put(address, 1.0/slope);
								dstETOP_V.data.put(address, strongMsrm->reflectivity - strongMsrm->height/slope);
								// dstETOP_hd.data.put(address, ::rand());
								// dstETOP_V.data.put(address, strongMsrm->reflectivity*slope - strongMsrm->height);

							}

						}
						else {
							// This should not happen!
							// Ie. the highest bin has higher Z than the lower bin?
							// But the lower bin should has been reseted.
							mout.error("Illegal state in measurement conf: Z+ > Z-  for h+ > h-");
							/*
							dstEchoTop.data.put(address, dstEchoTop.odim.nodata);
							dstQuality.data.put<int>(address, WEIGHTS.error);
// #ifndef NDEBUG
							if (EXTENDED_OUTPUT){
								dstClass.data.put<int>(address, CLASS.error); //
							}
// #endif
							*/
						}
					}
					else {
						// INTERPOLATION_DRY - interpolate between strong and "dry" point.
						// overshooting, "DRY TOP"
						if (USE_INTERPOLATION_DRY){ // && !REFERENCE_ONLY
							weakMsrm->reflectivity = undetectReflectivity;
							slope = getSlope(*weakMsrm, *strongMsrm);
							// slope = getSlope(weakMsrm->height, strongMsrm->height, undetectReflectivity, strongMsrm->reflectivity);
							WEIGHT = WEIGHTS.interpolation_dry;
							CLASS  = CLASSES.interpolation_dry;
						}
						else {
							slope = getSlope(reference, *strongMsrm);
							WEIGHT = WEIGHTS.extrapolation_up;
							CLASS  = CLASSES.extrapolation_up;
							// slope = getSlope(reference.height, strongMsrm->height, reference.reflectivity, strongMsrm->reflectivity);
						}

						height = strongMsrm->height + slope*(threshold - strongMsrm->reflectivity); // metres

						dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height)));
						dstQuality.data.put(address, WEIGHT);
						// dstQuality.data.put(address, USE_INTERPOLATION_DRY ? WEIGHTS.interpolation_dry : WEIGHTS.extrapolation_up);
// #ifndef NDEBUG
						if (EXTENDED_OUTPUT){
							dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0/ slope))); // dBZ/km ALERT div by 0 RISK
							dstClass.data.put<int>(address, CLASS);
							// dstClass.data.put<int>(address, USE_INTERPOLATION_DRY ? CLASSES.interpolation_dry : CLASSES.extrapolation_up);
						}
// #endif
						if (COMPUTE_REFERENCE){
							dstETOP_Zd.data.put(address, 1.0/slope);
							dstETOP_V.data.put(address, strongMsrm->reflectivity - strongMsrm->height/slope);
							// dstETOP_Zd.data.put(address, slope);
							// dstETOP_V.data.put(address, strongMsrm->reflectivity*slope - strongMsrm->height);
						}
					}
				}
				// No strong measurement.
				else if (dbzOdim.isValue(weakMsrm->reflectivity)){
					// Weak measurement only.
					slope  = getSlope(reference, *weakMsrm);
					height = weakMsrm->height + slope*(threshold - weakMsrm->reflectivity);
					dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height))); // ::rand()
					dstQuality.data.put(address, WEIGHTS.extrapolation_down);
// #ifndef NDEBUG
					if (EXTENDED_OUTPUT){
						dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0/ slope)));
						dstClass.data.put<int>(address, CLASSES.extrapolation_down);
					}
// #endif
				}
				else { // No strong or weak measurement
					dstEchoTop.data.put(address, dstEchoTop.odim.undetect);
					dstQuality.data.put(address, WEIGHTS.clear); // CONFIRMED CLEAR?
// #ifndef NDEBUG
					if (EXTENDED_OUTPUT){
						dstClass.data.put<int>(address, CLASSES.clear);
					}
// #endif
				}

			}
			// no weakMeasurement found
			else {

				if (strongMsrm != nullptr){
					// TRUNCATED / UNDERSHOOTING = the highest bin has Z exceeding the threshold
					slope  = getSlope(reference, *strongMsrm);
					height = strongMsrm->height + slope*(threshold - strongMsrm->reflectivity);

					if (j == 90){
						// mout.note(i,',', j, '\t', height, '\t', dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height), '\t', limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height)));
					}
					// dstEchoTop.data.put(address, ::rand());
					dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height))); // strong
					dstQuality.data.put(address, WEIGHTS.extrapolation_up);
// #ifndef NDEBUG
					if (EXTENDED_OUTPUT){
						dstClass.data.put<int>(address, CLASSES.extrapolation_up);
						dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0 / slope))); // ALERT div by 0 RISK
					}
// #endif
				}
				else {
					// No echo at all was found in this column above ground bin (i,j).
					dstEchoTop.data.put(address, dstEchoTop.odim.undetect);
					dstQuality.data.put(address, WEIGHTS.clear);
// #ifndef NDEBUG
					if (EXTENDED_OUTPUT){
						dstClass.data.put<int>(address, CLASSES.clear);
						//dstSlope.data.put(address, limitSlope(dstSlope.odim.undetect));
						dstSlope.data.put(address, dstSlope.odim.undetect);
					}
// #endif
				}


			}

		}

	}


	// SPESSU TEST
	{
		mout.attention("Entering test");
		// 11×500m × 11⁰
		RadarWindowConfig windowConf(5500,11.0);

		if ((refWindow.widthM != 0.0) && (refWindow.heightD != 0.0)){
			windowConf.set(refWindow);
		}


		windowConf.updatePixelSize(dstEchoTop.odim);


		Etop2Window window(windowConf);

		mout.special("Window: ", window);

		// Input
		window.setSrc(dstETOP_Zd.data, dstETOP_V.data);
		// window.setSrcFrame(dstETOP_Zd.data);
		// window.odimSrc.updateFromCastableMap(dstETOP_k.odim); // MUST! Clumsy, but dstETOP_k has not (yet) the odim's...
		// window.odimSrc.updateFromCastableMap(Etop2Window::sharedODIM); // MUST! Clumsy, but dstETOP_k has not (yet) the odim's...
		// window.odimSrc.setGeometry(area); // MUST!
		// window.setSrcFrame2(dstETOP_V.data);

		// Output
		PlainData<dst_t> & dstEchoTopReferenceHeight = dstProduct.getData("ETOP_REF_HGHT");
		dstEchoTopReferenceHeight.copyEncoding(Etop2Window::sharedODIM_HGHT);
		// dstEchoTopReferenceHeight.setEncoding(typeid(unsigned short));
		// quantityMap.setQuantityDefaults(dstEchoTopReferenceHeight, "HGHT");
		dstEchoTopReferenceHeight.setGeometry(area);
		dstEchoTopReferenceHeight.setExcluded(false); // check later

		PlainData<dst_t> & dstEchoTopReferenceDbz    = dstProduct.getData("ETOP_REF_DBZH");
		dstEchoTopReferenceDbz.copyEncoding(Etop2Window::sharedODIM_DBZH);
		//dstEchoTopReferenceDbz.setEncoding(typeid(unsigned short)); // float?
		//quantityMap.setQuantityDefaults(dstEchoTopReferenceDbz, "DBZH");
		dstEchoTopReferenceDbz.setGeometry(area);
		dstEchoTopReferenceDbz.setExcluded(false); // check later

		window.setDst(dstEchoTopReferenceHeight, dstEchoTopReferenceDbz);
		//window.setDstFrame(dstEchoTopReferenceHeight.data);
		//window.setDstFrame2(dstEchoTopReferenceDbz.data);
		// dstEchoTopReferenceDbz.data.fill(32767);
		// drain::image::FilePng::write(dstEchoTopReferenceDbz.data, "out.png");

		mout.attention("Entering test / run");
		window.run();
		// drain::image::FilePng::write(dstEchoTopReferenceHeight.data, "out2.png");
		// drain::image::FilePng::write(dstEchoTopReferenceDbz.data, "out3.png");
		mout.attention("Ended test");

	}

	// Spessu test 2
	{
		Data<dst_t> & dstSlopeSmooth = dstProduct.getData("SLOPE_SMOOTH");

		RadarWindowAvg<RadarWindowConfig> window;

		RadarWindowConfig conf2(3000, 5.0);

		conf2.setPixelConf(window.conf, dstSlope.odim);
		window.setSrcFrame(dstSlope.data);
		window.setDstFrame(dstSlopeSmooth.data);
		mout.attention(window);
		window.run();
	}


// #ifndef NDEBUG
	if (false){
		Data<dst_t> & dstEchoTopSmooth = dstProduct.getData(odim.quantity+"_SMOOTH");
		// qm.setQuantityDefaults(fuzzyCell2, "PROB");

		dstEchoTopSmooth.odim.importCastableMap(dstEchoTop.odim);
		// mout.special(dstEchoTopSmooth);
		dstEchoTopSmooth.copyEncoding(dstEchoTop);
		dstEchoTopSmooth.copyGeometry(dstEchoTop);

		mout.attention(dstEchoTopSmooth);

		PolarSlidingAvgOp smoother;
		/*
		smoother.conf.widthM  = 5*1000.0; // smoothAzm;
		smoother.conf.heightD = 100.0; // smoothRad;
		*/
		/*
		RadarWindowConfig test;
		smoother.conf.setPixelConf(test, dstEchoTop.odim);
		mout.warn("smoother.conf" , smoother.conf);
		mout.warn("    test conf" ,          test);
		*/

		//avgOp.setPixelConf()
		smoother.odim.importCastableMap(dstEchoTop.odim);
		smoother.odim.area.set(dstEchoTop.data.getGeometry());
		smoother.conf.updatePixelSize(dstEchoTop.odim);

		const PlainData<src_t> & pd = dstEchoTop;

		mout.attention("Next: processPlainData");
		mout.attention(pd);
		mout.attention(pd.odim);
		mout.attention(pd.odim.getGeometry());
		//mout.attention(dstEchoTop.odim.getGeometry());
		//mout.attention(dstEchoTop.odim);

		mout.attention(drain::TypeName<PlainData<dst_t> >::str());

		//smoother.processPlainData(dstEchoTop, dstEchoTopSmooth);
		smoother.processPlainData(pd, dstEchoTopSmooth);

		// Data<dst_t> & dstEchoTopSmooth = dstProduct.getData(odim.quantity+"_SMOOTH");
		// PolarSmoother::filter(dstEchoTop.odim, dstEchoTop.data, dstEchoTopSmooth.data, 100.0);
	}
// #endif

	/*
	for (const auto & entry: srcSweeps){
		const Data<src_t> & srcData = entry.second.getData("DBZH");
		smoother.processData(srcData, dstGradient);  // not in-place..
		return;
	}
	*/

	mout.accept<LOG_NOTICE>(" threshold=", threshold);

	dstProduct.getWhere()["rscale"] = dstEchoTop.odim.rscale;
	dstProduct.getWhere()["rscale10"] = dstEchoTop.odim.rscale+10;
	dstProduct.getWhat()["comment"] = "EchoTop2-test";
	//dstProduct.updateTree3(dstEchoTop.odim);

	mout.special<LOG_NOTICE>("First Data: ", dstProduct.getFirstData());
	Data<dst_t> & dst =  dstProduct.getFirstData();
	dstProduct.getHow()["from-qty"] = dst.odim.quantity;
	for (const auto & entry: srcSweeps){
		const Data<src_t> & src =  entry.second.getData("DBZH");
		if (!src.data.isEmpty()){
			dst.odim.angles.push_back(src.odim.elangle);
			// dstProduct.getHow()["angles"] << src.odim.elangle;
		}
	}

}


}


// Rack
