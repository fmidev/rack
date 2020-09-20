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


#include "drain/util/Log.h"

#include "drain/util/Registry.h"
#include "drain/util/Cloner.h"
#include "drain/util/Fuzzy.h"

#include "ImageOpBank.h"


#include "GammaOp.h"
#include "RunLengthOp.h"

#include "BlenderOp.h"

#include "CatenatorOp.h"
#include "CopyOp.h"
#include "CropOp.h"
#include "DistanceTransformFillOp.h"
#include "DistanceTransformOp.h"
#include "FastAverageOp.h"
//#include "FastOpticalFlowOp.h"
#include "FlowAverageWindowOp.h" // for SlidingWindowOp
#include "FloodFillOp.h"
#include "FunctorOp.h"
#include "GammaOp.h"
#include "GaussianAverageOp.h"
#include "DifferentialOp.h"
#include "HighBoostOp.h"
#include "HighPassOp.h"
#include "ImpulseAvgOp.h"
#include "MarginalStatisticOp.h"
//#include "MotionExtrapolatorOp.h"
//#include "MotionIllustratorOp.h"
#include "PaletteOp.h"
#include "PixelVectorOp.h"
#include "QualityMixerOp.h"
#include "QualityOverrideOp.h"
#include "QuantizatorOp.h"
//#include "RecursiveRepairerOp.h"
#include "ResizeOp.h"
#include "RunLengthOp.h"
//#include "drain/image/SegmentProber.h"
#include "SegmentAreaOp.h"
#include "SegmentStatisticsOp.h"
#include "SlidingWindowHistogramOp.h"
#include "SlidingWindowMedianOp.h"
#include "SlidingWindowOp.h"
#include "TransposeOp.h"
//#include "WindowOp.h"

// using namespace std;

namespace drain
{

namespace image
{

ImageOp & ImageOpBank::getComplete(const std::string & query, char assignmentChar, char separatorChar, const drain::SmartMap<std::string> & aliasMap){

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), "ImageOpBank", __FUNCTION__);

	size_t index = query.find(separatorChar);
	const bool PARAMS = (index != std::string::npos);
	const std::string n = PARAMS ? query.substr(0,index) : query;
	const std::string name = aliasMap.get(n, n);  // :-D default value is the key itself.
	const std::string params = PARAMS ? query.substr(index+1) : "";
	mout.debug() << "op: " << name << '\t' << params << mout.endl;
	if (!has(name)){
		//mout.note() << bank << mout.endl;
		mout.error() << "invalid op: '" << name << "' extracted from query: '" << query << "'" << mout.endl;
	}

	ImageOp & op = get(name);
	if (PARAMS)
		op.setParameters(params, assignmentChar, separatorChar);

	return op;
}

std::ostream & operator<<(std::ostream & ostr, const ImageOpBank & bank){
	for (ImageOpBank::map_t::const_iterator it = bank.getMap().begin(); it != bank.getMap().end(); ++it){
		const ImageOp & op = it->second.get();
		//ostr << it->first << ':' << '\t' << op.getName() << ',' << op.getClassName() << ',' << op.getParameters() << '\n';
		ostr << it->first << ' ' << '(' << op.getName() << ')' << ':' << ' ' << op.getDescription() << '\n';
		ostr << '\t' << op.getParameters() << '\n';   // consider op.help() ?
	}
	return ostr;
}


/*
class CmdPaletteLoad : public SimpleCommand<std::string> {

public:

	CmdPaletteLoad() : SimpleCommand<std::string>(__FUNCTION__, "Load palette.", "filename", "", "<filename>.[txt|json]") {
	};

	virtual
	void exec() const {
		load(value);
	};

	void load(const std::string &s) const {
		getImageOpBank.palette.load(s);
	}

};
*/

ImageOpBank & getImageOpBank() {

	/// Shared ImageOp bank
	static ImageOpBank bank;

	if (bank.getMap().empty()){


		try {


			static UnaryFunctorOpCloner<RemappingFunctor> remap;
			bank.add(remap, "remap");

			static UnaryFunctorOpCloner<NegateFunctor>  negate;
			bank.add(negate);
			//negate.
			//negate.get().functor.setScale(-1.0,1.0);

			static UnaryFunctorOpCloner<ScalingFunctor>  scale;
			bank.add(scale, "rescale");
			static UnaryFunctorOpCloner<ThresholdFunctor> threshold;
			bank.add(threshold);
			static UnaryFunctorOpCloner<BinaryThresholdFunctor> binaryThreshold; // Here "binary" refers to black and white, not to two arguments!
			bank.add(binaryThreshold, "thresholdBinary");


			///	Binary functors - operations on two inputs
			static BinaryFunctorOpCloner<AdditionFunctor> add; // addOp.imageOp.reference("LIMIT2", addOp.imageOp.LIMIT=true);
			bank.add(add, "add");
			static BinaryFunctorOpCloner<DivisionFunctor> div;
			bank.add(div, "div");
			static BinaryFunctorOpCloner<MaximumFunctor> max;
			bank.add(max, "max");
			static BinaryFunctorOpCloner<MinimumFunctor> min;
			bank.add(min, "min");
			static BinaryFunctorOpCloner<MixerFunctor> mix;
			bank.add(mix, "mix");
			static BinaryFunctorOpCloner<MultiplicationFunctor> mul;
			bank.add(mul, "mul");
			static BinaryFunctorOpCloner<SubtractionFunctor> sub;
			bank.add(sub, "sub");


			/// Fuzzy remapping ops
			/**
				\code
				  drainage image.png --fuzzyBell 0.5,0.2 -o fuzzyBell.png
				  drainage image.png --fuzzyStep 0.4,0.6 -o fuzzyThreshold.png
				\endcode
			 */
			static UnaryFunctorOpCloner<FuzzyBell<double> > fuzzyBell;
			bank.add(fuzzyBell);
			static UnaryFunctorOpCloner<FuzzyBell2<double> > fuzzyBell2;
			bank.add(fuzzyBell2);
			static UnaryFunctorOpCloner<FuzzyStep<double> > fuzzyStep;
			bank.add(fuzzyStep);
			static UnaryFunctorOpCloner<FuzzyStepsoid<double> > fuzzyStepSoid;
			bank.add(fuzzyStepSoid);
			static UnaryFunctorOpCloner<FuzzyTriangle<double> > fuzzyTriange;
			bank.add(fuzzyTriange);
			static UnaryFunctorOpCloner<FuzzyTwinPeaks<double> > fuzzyTwinPeaks;
			bank.add(fuzzyTwinPeaks);


			//static ImageOpCloner<VerticalCatenatorOp> catenator; // causes memory error?
			static ImageOpCloner<ChannelCatenatorOp> catenator; // causes memory error?
			bank.add(catenator, "catenate");


			static ImageOpCloner<CopyOp> copy;
			bank.add(copy);


			static ImageOpCloner<CropOp> crop;
			bank.add(crop);

			static ImageOpCloner<DistanceTransformLinearOp> distanceTransform;
			bank.add(distanceTransform, "distanceTransform");

			static ImageOpCloner<DistanceTransformExponentialOp> distanceTransformExp;
			bank.add(distanceTransformExp, "distanceTransformExp");

			static ImageOpCloner<DistanceTransformFillLinearOp> distanceTransformFill;
			bank.add(distanceTransformFill, "distanceTransformFill");

			static ImageOpCloner<DistanceTransformFillExponentialOp> distanceTransformFillExp;
			bank.add(distanceTransformFillExp, "distanceTransformFillExp");

			static ImageOpCloner<FastAverageOp> fastaverage;
			bank.add(fastaverage, "average");

			//static ImageOpCloner<SlidingWindowOp<FlowAverageWindow> > flowAverage;
			static ImageOpCloner<FlowAverageOp> flowAverage;
			bank.add(flowAverage, "flowAverage");

			static ImageOpCloner<BlenderOp> blender;
			bank.add(blender);

			static ImageOpCloner<ImpulseResponseOp<ImpulseAvg> > impulseResponse;
			//std::cerr << "response: " << impulseResponse.getName() << std::endl;
			bank.add(impulseResponse, "impulseAvg");


			//static ImageOpCloner<FastOpticalFlowOp> fastopticalflow;
			//bank.add(fastopticalflow);

			static ImageOpCloner<FloodFillOp> floodfill;
			bank.add(floodfill);

			// static ImageOpCloner<FunctorOp> functor;
			// bank.add(functor, "functor");

			//static ImageOpCloner<GammaOp> gamma;
			static UnaryFunctorOpCloner<GammaFunctor> gamma;
			bank.add(gamma);

			static UnaryFunctorOpCloner<QuantizatorFunctor> quantizator;
			bank.add(quantizator);


			static ImageOpCloner<GaussianAverageOp> gaussianAverage;
			bank.add(gaussianAverage);

			/// Differential ops
			static ImageOpCloner<GradientOp> gradient;
			bank.add(gradient);
			static ImageOpCloner<GradientHorzOp> gradientHorz;
			bank.add(gradientHorz);
			static ImageOpCloner<GradientVertOp> gradientVert;
			bank.add(gradientVert);
			static ImageOpCloner<LaplaceOp> laplace;
			bank.add(laplace);
			static ImageOpCloner<LaplaceHorzOp> laplaceHorz;
			bank.add(laplaceHorz);
			static ImageOpCloner<LaplaceVertOp> laplaceVert;
			bank.add(laplaceVert);


			static ImageOpCloner<HighBoostOp> highBoost;
			bank.add(highBoost);

			static ImageOpCloner<HighPassOp> highPass;
			bank.add(highPass);

			//static ImageOpCloner<ImageOp> image;
			//bank.add(image, "image");

			static ImageOpCloner<MarginalStatisticOp> marginalStatistic;
			bank.add(marginalStatistic, "marginStat");

			//static ImageOpCloner<MotionExtrapolatorOp> motionextrapolator;
			//bank.add(motionextrapolator, "motionextrapolator");

			//static ImageOpCloner<MotionIllustratorOp> motionillustrator;
			//bank.add(motionillustrator, "motionillustrator");

			//static ImageOpCloner<PaletteOp> palette;
			//bank.add(palette, "palette");

			//static ImageOpCloner<PixelVectorOp> pixelvector;
			//bank.add(pixelvector, "pixelvector");
			static ImageOpCloner<DistanceOp> distance;
			bank.add(distance);
			static ImageOpCloner<ProductOp> product;
			bank.add(product);
			static ImageOpCloner<MagnitudeOp> magnitude;
			bank.add(magnitude);

			//static ImageOpCloner<Distance2Op> distance2;
			//bank.add(distance2);


			// static ImageOpCloner<QuadraticSmootherOp> quadraticSmoother;
			// bank.add(quadraticSmoother);
			static ImageOpCloner<QualityThresholdOp> qualityThreshold;
			bank.add(qualityThreshold);


			static ImageOpCloner<QualityMixerOp> qualityMixer;
			bank.add(qualityMixer);

			static ImageOpCloner<QualityOverrideOp> qualityoverride;
			bank.add(qualityoverride);

			//static ImageOpCloner<QuantizatorOp> quantizator;
			//bank.add(quantizator);

			static ImageOpCloner<ResizeOp> resize;
			bank.add(resize);

			//static ImageOpCloner<RecursiveRepairerOp> recursiveRepairer;
			//bank.add(recursiveRepairer);

			static ImageOpCloner<RunLengthHorzOp> runLengthHorz;
			bank.add(runLengthHorz);
			static ImageOpCloner<RunLengthVertOp> runLengthVert;
			bank.add(runLengthVert);

			static ImageOpCloner<SegmentAreaOp<float,unsigned short> > segmentArea;
			bank.add(segmentArea, "segmentArea");

			static ImageOpCloner<SegmentStatisticsOp> segmentStats;
			bank.add(segmentStats, "segmentStats");

			//static ImageOpCloner<SequentialImageOp> sequentialimage;
			//bank.add(sequentialimage, "sequentialimage");

			static ImageOpCloner<SlidingWindowHistogramOp> slidingwindowHistogram;
			bank.add(slidingwindowHistogram, "histogram");

			static ImageOpCloner<SlidingWindowMedianOp> slidingWindowMedian;
			bank.add(slidingWindowMedian, "median");

			//static ImageOpCloner<SlidingWindowOp> slidingWindow;
			//bank.add(slidingWindow);

			static ImageOpCloner<TransposeOp> transpose;
			bank.add(transpose);

			//static ImageOpCloner<WindowOp> window;
			//bank.add(window, "window");



		} catch (std::exception & e) {
			std::cerr << "Bank " << e.what() << std::endl;
		}

	}

	return bank;

}



}

}

// Drain
