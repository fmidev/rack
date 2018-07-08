/**

    Copyright 2001 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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

#include "util/FunctorPack.h"
//#include "SlidingStripeAverageOp.h"

namespace drain
{
namespace image
{


/// Smoothes image and mixes the result with the original by coeff*100%.
/*!

   This operator produces
   \f[
   F2 = (1-c)F + cM\{ (1-c)F + cM\{F\} \}
      = (1-c)F + (1-c)cM{F} + c^2M^2{F}
   \f]
   where
   \f$F\f$ is an image,
   \f$M\{\}\f$ is a FastAverage operator of size \f$W \times H\f$, and
   \f$c\f$ is the mixing coefficient between 0.0 and 1.0.

   \code
   	drainage shapes.png --blender 25,mix=b/coeff:0.25 -o hazy25a.png
   	drainage shapes.png --blender 25,mix=b/coeff:0.50 -o hazy50a.png
   	drainage shapes.png --blender 25,mix=b/coeff:0.75 -o hazy75a.png
   	drainage shapes.png --blender 25,smooth=g,mix=b/coeff:0.75 -o hazy75g.png
   	drainage shapes.png --blender 25,mix=m            -o hazyMax.png
   \endcode

	This operator can be also used for restoring images containing specks of low quality.
   \code
     drainage flowers-rgba.png --blender 50,smooth=a,mix=m -o restored-a1.png
     drainage flowers-rgba.png --blender 50,smooth=a,mix=m,loops=3 -o restored-a3.png
     drainage flowers-rgba.png --blender 50,smooth=g,mix=m -o restored-g1.png
     drainage flowers-rgba.png --blender 50,smooth=g,mix=m,loops=3 -o restored-g3.png
     drainage flowers-rgba.png --blender 50,smooth=d -o restored--d.png
     drainage flowers-rgba.png --blender 50,smooth=D -o restored--D.png
   \endcode


 */
class BlenderOp: public WindowOp<> {

public:

	/// Default constructor
	/**
	 *
	 */
	// TODO: re-consider the order of params?
	BlenderOp(int width=5, int height=0, char smoother='a', char mixer='m', unsigned short loops=1) :  // , double coeff=0.5
		WindowOp<>(__FUNCTION__, "Smoothes image repeatedly, mixing original image with the result at each round.", width, height){
		initRefs();
		this->smootherKey = smoother;
		this->mixerKey = mixer;
		this->loops = loops;
	};

	// Every Op should have a copy const
	BlenderOp(const BlenderOp & op) :
		WindowOp<>(__FUNCTION__, "Smoothes image repeatedly, mixing original image with the result at each round.", op.conf.width, op.conf.height){
		initRefs();
	};


	inline
	const std::string & getSmootherKey(){
		return smootherKey;
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

protected:

	//mutable	drain::SmartMap<std::string> aliasMap;

	void getSmootherAliasMap(drain::SmartMap<std::string> & aliasMap, bool weighted=false) const;


	// int width=5, int height=0, char smoother='a', char mixer='m', unsigned short loops=1
	void initRefs();

	//double coeff;
	unsigned short loops;

	// SMOOTHING
	std::string smootherKey;
	/**
	 *  \param loops - sets loops=1 if looping not supported smoother(key).
	 */
	ImageOp & getSmoother(const std::string & key, bool weighted, unsigned short & loops) const;

	// MIXING
	std::string mixerKey;
	ImageOp & getMixer(const std::string & key, bool weighted) const;
	// double coeff;

};




} // namespace drain

} //namespace image


#endif
