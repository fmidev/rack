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
#ifndef BLENDER_OP
#define BLENDER_OP "BlenderOp Markus.Peura@iki.fi"

#include "WindowOp.h"

#include "DistanceTransformOp.h"
#include "DistanceTransformFillOp.h"
#include "FastAverageOp.h"
#include "FlowAverageWindowOp.h"
#include "GaussianAverageOp.h"
#include "QualityMixerOp.h"
#include "QualityOverrideOp.h"

#include "drain/util/FunctorPack.h"
//#include "SlidingStripeAverageOp.h"

namespace drain
{
namespace image
{


/// Smoothes image and mixes the result with the original.
/*!

Example. With \c loops=2 and smooth mixing with \c coeff=c (instead of maximum), this operator produces
\f[
F2 = (1-c)F + cM\{ (1-c)F + cM\{F\} \}
  = (1-c)F + (1-c)cM{F} + c^2M^2{F}
\f]
where
\f$F\f$ is an image,
\f$M\{\}\f$ is a FastAverage operator of size \f$W \times H\f$, and
\f$c\f$ is the mixing coefficient between 0.0 and 1.0.

\code
drainage shapes.png --iBlender 25,mix=0.25 -o hazy25a.png
drainage shapes.png --iBlender 25,mix=0.50 -o hazy50a.png
drainage shapes.png --iBlender 25,mix=0.75 -o hazy75a.png
drainage shapes.png --iBlender 25,avgGauss,mix=0.25 -o hazy25g.png
drainage shapes.png --iBlender 25,mix=max -o hazyMax.png
\endcode

This operator can be also used for restoring images containing specks of low quality.
\~exec
   make flowers-rgba.png  #exec
\~

\code
drainage flowers-rgba.png --iBlender 51,avg,max -o restored-a1.png
drainage flowers-rgba.png --iBlender 51,avg,mix=0.75 -o restored-a1blend.png
drainage flowers-rgba.png --iBlender 51,avg,max,loops=3 -o restored-a3.png
drainage flowers-rgba.png --iBlender 5,avg,max,loops=5,expansionCoeff=1.5  -o restored-a5exp.png
drainage flowers-rgba.png --iBlender 51,avgGauss,max -o restored-ag1.png
drainage flowers-rgba.png --iBlender 51,avgGauss,max,loops=3 -o restored-ag3.png
drainage flowers-rgba.png --iBlender 51,spreader=dist    -o restored--d.png
drainage flowers-rgba.png --iBlender 51,spreader=distExp -o restored--dexp.png
\endcode

\see FastAVerageOp
\see DistanceTransformOp

 */
class BlenderOp: public WindowOp<> {

public:

	/// Default constructor
	// TODO Rename, and use blender only for the op that mixes filtered t unfiltered
	/**
	 *
	 */
	// TODO: re-consider the order of params?
	// BlenderOp(int width=5, int height=0, char spreader='a', char mixer='m', unsigned short loops=1) :  // , double coeff=0.5
	BlenderOp(int width=5, int height=0, const std::string & spreader="avg", const std::string & blender="max",
			unsigned short loops=1, double expansionCoeff=1.0) :  // , double coeff=0.5
		WindowOp<>(__FUNCTION__, "Smoothes image repeatedly, mixing original image with the result at each round.", width, height){
		parameters.link("spreader", this->spreader = spreader, getSmootherAliasMap<false>().toStr()); //"a|g|d|D; avg, gaussianAvg, dist, distExp");
		parameters.link("mix", this->blender = blender, "max|<coeff>: (quality) max, (quality) blend");
		parameters.link("loops", this->loops = loops, "number of repetitions");
		parameters.link("expansionCoeff", this->expansionCoeff = expansionCoeff, "window enlargement");
	};

	// Every Op should have a copy const
	BlenderOp(const BlenderOp & op) : WindowOp<>(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	};



	inline
	const std::string & getSmootherKey(){
		return spreader;
	}

	virtual inline
	void initializeParameters(const ImageFrame &src, const ImageFrame &dst) const {
		if (conf.frame.height == 0){
			conf.frame.height = conf.frame.width;
		}
	}

	/// Main operation, requires weighted image data.
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;


	/// Force multi-channel processing
	inline
	virtual void traverseChannel(const Channel &src, Channel & dst) const {
		traverseAsChannelTrays(src, dst);
	}

	/// Force multi-channel processing
	inline
	void traverseChannel(const Channel &src, const Channel &srcWeight, Channel & dst, Channel & dstWeight) const {
		traverseAsChannelTrays(src, srcWeight, dst, dstWeight);
	}

	virtual inline
	const std::string & getName() const override {
		return name;
	};

protected:

	//mutable	drain::SmartMap<std::string> aliasMap;

	template <bool WEIGHTED=false>
	static
	const drain::SmartMap<std::string> & getSmootherAliasMap() {
		static drain::SmartMap<std::string> aliasMap;
		if (aliasMap.empty()){
			/// Use plain names (not prefixed)
			/*
			aliasMap["a"] = "average";
			aliasMap["f"] = "flowAverage"; // magnitude/energy saving
			aliasMap["g"] = "gaussianAverage";
			aliasMap["d"] = WEIGHTED ? "distanceTransformFill"    : "distanceTransform";
			aliasMap["D"] = WEIGHTED ? "distanceTransformFillExp" : "distanceTransformExp";
			*/
			// NEW
			aliasMap["avg"] = "average";
			aliasMap["avgFlow"] = "flowAverage"; // magnitude/energy saving
			aliasMap["avgGauss"] = "gaussianAverage";
			aliasMap["dist"] = WEIGHTED ? "distanceTransformFill"    : "distanceTransform";
			aliasMap["distExp"] = WEIGHTED ? "distanceTransformFillExp" : "distanceTransformExp";
		}
		return aliasMap;
	}

	template <bool WEIGHTED=false>
	static
	const drain::SmartMap<std::string> & getMixerAliasMap(){
		static drain::SmartMap<std::string> aliasMap;
		//aliasMap["b"] = WEIGHTED ? "qualityMixer"    : "mix";
		//aliasMap["m"] = WEIGHTED ? "qualityOverride" : "max";
		// NEW
		aliasMap["blend"] = WEIGHTED ? "qualityMixer"    : "mix";
		aliasMap["max"]   = WEIGHTED ? "qualityOverride" : "max";
		return aliasMap;
	}


	// SMOOTHING
	std::string spreader;

	// MIXING
	std::string blender;

	unsigned short loops = 1;

	double expansionCoeff = 1.0;


};




} // namespace drain

} //namespace image


#endif

// Drain
