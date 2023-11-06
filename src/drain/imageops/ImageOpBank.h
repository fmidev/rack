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
#ifndef DRAIN_IMAGE_OP_BANK_H_
#define DRAIN_IMAGE_OP_BANK_H_

//
#include <iostream>
#include <map>

//#include "drain/util/Log.h"

//#include "drain/util/Registry.h"
//#include "drain/util/Cloner.h"
#include "drain/util/Bank.h"
#include "drain/util/FunctorPack.h"
#include "drain/util/RegExp.h"

#include "ImageOp.h"
#include "FunctorOp.h"

#include "drain/prog/Command.h"
#include "drain/prog/CommandBank.h"

#include "drain/util/Cloner.h"
#include "drain/util/Fuzzy.h"




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
#include "MultiThresholdOp.h"
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
// using namespace std;

namespace drain
{

namespace image
{

template <class T>
class ImageOpCloner : public Cloner<ImageOp,T> {
public:

	static
	const std::string & getName(){
		static T op;
		return op.getName();
	}

};

template <class F>
class UnaryFunctorOpCloner : public ImageOpCloner<UnaryFunctorOp<F> > {
    public:
};


template <class F>
class BinaryFunctorOpCloner : public ImageOpCloner<BinaryFunctorOp<F> > {
    public:
};


class NegateOp : public ImageOp {

public:

	NegateOp() : ImageOp(__FILE__, __FUNCTION__){
	}

	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		traverseChannelsEqually(src, dst);
	}


	/// Apply to single channel.
	virtual inline
	void traverseChannel(const Channel & src, Channel & dst) const {

		double srcMax = src.getConf().getTypeMax<double>();
		double dstMax = dst.getConf().getTypeMax<double>();
		double rescale = dstMax/srcMax;

		Channel::const_iterator s  = src.begin();
		Channel::iterator d = dst.begin();

		while (d != dst.end()){
			*d = dstMax - rescale*static_cast<double>(*s);
			++s;
			++d;
		}

	};



};


class ImageOpBank : public Bank<ImageOp> {

public:

	//CommandBank & cmdBank;

	//ImageOpBank(CommandBank & cmdBank) : cmdBank(cmdBank) {
	ImageOpBank() {
	};

	virtual
	~ImageOpBank(){}

	/// Add ImageOp command to registry (CommandBank).
	/**
	 * \tparam OP - Class derived from ImageOp or Command
	 */
	template <class OP>
	OP & install(const std::string & name = OP().getName()){

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		std::string key(name);
		drain::CommandBank::deriveCmdName(key, 0);
		mout.warn() << name << ">" <<  OP().getName() << '>' << key << '\n';
		return Bank<ImageOp>::add<OP>(key);
	}

	/// Supports querying operator with parameters set, eg. gaussianAverage,width=10,height=5
	ImageOp & getComplete(const std::string & query, char separator=',', char assign='=', const drain::SmartMap<std::string> & aliasMap = drain::SmartMap<std::string>());

	/// General resources
	// Palette palette;

};



std::ostream & operator<<(std::ostream & ostr, const ImageOpBank & bank);



// consider separate
extern
ImageOpBank & getImageOpBank();

template <class T>
void installImageOps(T & installer) {


	try {
		//installer.install<CmdHistogram>();
		installer.template install<NegateOp>();
		installer.template install<UnaryFunctorOp<ScalingFunctor> >("Rescale");
		//installer.template install<UnaryFunctorOp<NegateFunctor> >();
		installer.template install<UnaryFunctorOp<RemappingFunctor> >("Remap");
		installer.template install<UnaryFunctorOp<ThresholdFunctor> >();
		installer.template install<UnaryFunctorOp<BinaryThresholdFunctor> >("ThresholdBinary");


		///	Binary functors - operations on two inputs
		installer.template install<BinaryFunctorOp<AdditionFunctor> >("Add");
		installer.template install<BinaryFunctorOp<DivisionFunctor> >("Div");
		installer.template install<BinaryFunctorOp<MaximumFunctor> >("Max");
		installer.template install<BinaryFunctorOp<MinimumFunctor> >("Min");
		installer.template install<BinaryFunctorOp<MixerFunctor> >("Mix");
		installer.template install<BinaryFunctorOp<MultiplicationFunctor> >("Mul");
		installer.template install<BinaryFunctorOp<SubtractionFunctor> >("Sub");


		/// Fuzzy remapping ops. See examples in drain/util/Fuzzy.h
		installer.template install<UnaryFunctorOp<FuzzyBell<double>,true> >();
		installer.template install<UnaryFunctorOp<FuzzyBell2<double>,true> >();
		installer.template install<UnaryFunctorOp<FuzzyStep<double>,true>  >();
		installer.template install<UnaryFunctorOp<FuzzyStepsoid<double>,true> >();
		installer.template install<UnaryFunctorOp<FuzzyTriangle<double>,true> >();
		installer.template install<UnaryFunctorOp<FuzzyTwinPeaks<double>,true> >();


		installer.template install<ChannelCatenatorOp>("Catenate");

		installer.template install<CopyOp>();
		installer.template install<CropOp>();

		installer.template install<DistanceTransformLinearOp>("DistanceTransform");  // consider rename op
		installer.template install<DistanceTransformExponentialOp>("DistanceTransformExp");  // consider rename op
		installer.template install<DistanceTransformFillLinearOp>("DistanceTransformFill"); // consider rename op
		installer.template install<DistanceTransformFillExponentialOp>("DistanceTransformFillExp");  // consider rename op

		installer.template install<FastAverageOp>("Average");

		installer.template install<FlowAverageOp>();

		installer.template install<BlenderOp>();

		installer.template install<ImpulseResponseOp<ImpulseAvg> >("impulseAvg"); // consider getName -> avg.getName()


		//static ImageOpCloner<FastOpticalFlowOp> fastopticalflow;
		//bank.template install< >(fastopticalflow);

		installer.template install<FloodFillOp>();

		// static ImageOpCloner<FunctorOp> functor;
			installer.template install<UnaryFunctorOp<GammaFunctor,true> >();

		//installer.template install<QuantizatorOp>();
		installer.template install<UnaryFunctorOp<QuantizatorFunctor> >();

		installer.template install<GaussianAverageOp>();

		/// Differential ops
		installer.template install<GradientOp>();
		installer.template install<GradientHorzOp>();
		installer.template install<GradientVertOp>();
		installer.template install<LaplaceOp>();
		installer.template install<LaplaceHorzOp>();
		installer.template install<LaplaceVertOp>();


		installer.template install<HighBoostOp>();

		installer.template install<HighPassOp>();

		installer.template install<MarginalStatisticOp>("marginStat"); // consider rename op

		//static ImageOpCloner<MotionExtrapolatorOp> motionextrapolator;
		//static ImageOpCloner<MotionIllustratorOp> motionillustrator;
		//static ImageOpCloner<PaletteOp> palette;

		//static ImageOpCloner<PixelVectorOp> pixelvector;
		//bank.template install< >(pixelvector, "pixelvector");
		installer.template install<GrayOp>();
		installer.template install<DistanceOp>();
		installer.template install<ProductOp>();
		installer.template install<MagnitudeOp>();

		installer.template install<MultiThresholdOp>();

		//static ImageOpCloner<Distance2Op> distance2;
		//bank.template install< >(distance2);

		// installer.template install<QuadraticSmootherOp>();
		// static ImageOpCloner<QuadraticSmootherOp> quadraticSmoother;
		// bank.template install< >(quadraticSmoother);
		installer.template install<QualityThresholdOp>();

		installer.template install<QualityMixerOp>();

		installer.template install<QualityOverrideOp>();


		installer.template install<ResizeOp>();

		//static ImageOpCloner<RecursiveRepairerOp> recursiveRepairer;
		//bank.template install< >(recursiveRepairer);

		installer.template install<RunLengthHorzOp>();
		installer.template install<RunLengthVertOp>();

		installer.template install<SegmentAreaOp<float,unsigned short> >();
		installer.template install<SegmentStatisticsOp>("segmentStats");


		installer.template install<SlidingWindowHistogramOp>("windowHistogram");
		installer.template install<SlidingWindowMedianOp>("median");
		installer.template install<TransposeOp>();

		//static ImageOpCloner<ImageOp> image;
		//static ImageOpCloner<SequentialImageOp> sequentialimage;
		//bank.template install< >(sequentialimage, "sequentialimage");
		//static ImageOpCloner<SlidingWindowOp> slidingWindow;
		//bank.template install< >(slidingWindow);
		//static ImageOpCloner<WindowOp> window;
		//bank.install< >(window, "window");

	} catch (std::exception & e) {
		std::cerr << "Bank " << e.what() << std::endl;
	}


}


}

}

#endif

// Drain
