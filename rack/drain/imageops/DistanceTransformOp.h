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
#ifndef DISTANCETRANSFORMOP_H_
#define DISTANCETRANSFORMOP_H_



#include <math.h>

#include "drain/image/CoordinateHandler.h"
#include "drain/image/DistanceModelLinear.h"
#include "drain/image/DistanceModelExponential.h"

#include "ImageOp.h"


namespace drain
{

namespace image
{



/** Fast distance transform using 8-directional distance function.
 *  Class D is used for computing distances?
 */
template <class T>
class DistanceTransformOp : public ImageOp
{

public:


	typedef float dist_t;
    
    virtual ~DistanceTransformOp(){};

    inline
    void setRadius(dist_t width, dist_t height=NAN, dist_t width2=NAN, dist_t height2=NAN){
    	distanceModel.setRadius(width, height, width2, height2);
    };



    virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
    	this->traverseChannelsSeparately(src, dst);
    };

    virtual
	void traverseChannel(const Channel & src, Channel &dst) const ;

    /// Apply to single channel with alpha.
    virtual
    inline
    void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {
    	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);
    	mout.note() << "discarding alpha channels, redirecting to traverseChannel(src, dst)" << mout.endl;
    	traverseChannel(src, dst);
    };

    /// Ensures dst the same geometry and type with src.
    /*
     */
    virtual inline
    void getDstConf(const ImageConf &src, ImageConf & dst) const {

    	dst.setGeometry(src.getGeometry());

    	if (src.isPhysical()){
    		dst.setPhysicalRange(src.getPhysicalRange(), true);
    	}
    	// dst.setPhysicalRange(0.0, 1.0);

    	dst.coordinatePolicy.set(src.coordinatePolicy);


    }


    /*
	virtual
	inline
	void make Compatible(const ImageFrame &src, Image &dst) const  {

		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.debug3() << "src: " << src << mout.endl;

		//if (!dst.typeIsSet())
			//dst.setType(src.getType());
		if (dst.getType() != src.getType()){
			mout.note() << " changing dst image type: " << dst.getType().name() << '>' << src.getType().name() << mout.endl;
		}

		dst.copyShallow(src);
		// dst.initialize(src.getType(), src.getGeometry());
		// dst.adoptScaling(src);

		mout.debug(3) << "dst: " << dst << mout.endl;

	};
	*/


protected:

	mutable T distanceModel;

	//int topology;

	DistanceTransformOp(const std::string &name, const std::string &description, float width, float height, DistanceModel::topol_t topology=2) :
		ImageOp(name, description) {
		parameters.append(this->distanceModel.getParameters());
		distanceModel.setTopology(topology);
		distanceModel.setRadius(width, height, width, height);
	};

	DistanceTransformOp(const DistanceTransformOp & op) : ImageOp(op) {
		parameters.append(this->distanceModel.getParameters());
		setParameters(op.getParameters());
	};



	/// Sets internal parameters
	/**
	 *  Max value in the model should be set according to the channel which is used as control field.
	 */
	virtual
	void initializeParameters(const ImageFrame &src, const ImageFrame &dst) const {

		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);

		// WAS: src (all)  // .getEncoding()
		const double physMax = dst.getConf().requestPhysicalMax(100.0);
		const double codeMax = dst.getScaling().inv(physMax);

		mout.debug() << "dst of type=" << Type::getTypeChar(dst.getType()) << ", scaling: " << dst.getScaling();

		if (dst.getScaling().isPhysical()){
			mout.info() << "ok, physical scaling [" << dst.getConf().getPhysicalRange() << "] : ";
		}
		else if (Type::call<typeIsSmallInt>(dst.getType())){
			mout.note() << "no physical scaling, but small int, guessing: ";
		}
		else {
			mout.warn() << "no physical scaling, no small int: default ";
		}
		mout << "physMax=" << physMax << " => " << codeMax << mout.endl;

		distanceModel.setMax(codeMax);
		// distModel.setMax(dst.getMax<double>()); // why not dst
		distanceModel.update(); // radii
	}


	void traverseDownRight(const Channel & src, Channel & dst) const ;

	void traverseUpLeft(const Channel & src, Channel & dst) const ;


	virtual
	const DistanceModel & getDistanceModel() const {
		return distanceModel;
	};

	virtual
	DistanceModel & getDistanceModel() {
		return distanceModel;
	};

};
  


template <class T>
void DistanceTransformOp<T>::traverseChannel(const Channel &src, Channel & dst) const
{

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	//mout.warn() << "start" << mout.endl;
	mout.debug() << "model max" << getDistanceModel().getMax() << mout.endl;

	//File::write(dst,"DistanceTransformOp-dst0.png");
	traverseDownRight(src,dst);
	//File::write(dst,"DistanceTransformOp-dst1.png");
	traverseUpLeft(dst,dst);
	//File::write(dst,"DistanceTransformOp-dst2.png");

}


template <class T>
void DistanceTransformOp<T>::traverseDownRight(const Channel &src, Channel &dst) const
{

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);
	mout.debug() << "start" << mout.endl;

	//const DistanceModel & distModel = getDistanceModel();
	mout.debug2() << "distModel:" << this->distanceModel << mout.endl;

	DistanceNeighbourhood chain;
	this->distanceModel.createChain(chain, true);

	CoordinateHandler2D coordinateHandler(src); // TODO: change param to ImageConf
	mout.debug2() << "coordHandler:" << coordinateHandler << mout.endl;


	const ValueScaling src2dst(dst.getConf().getTypeMax<double>()/src.getConf().getTypeMax<double>(), 0);
	//const ValueScaling src2dst(src.getScaling(), dst.getScaling());

	for (double d: {0.0, 0.5, 1.0, 255.0, 65535.0}){
		mout.special() << d << " => " << src2dst.fwd(d) << " inv:" << src2dst.inv(d) << mout;
	}

	// proximity (inverted distance)
	dist_t d;
	dist_t dTest;

	// Point in the source image
	Point2D<int> pTest;

	// Point in the target image
	Point2D<int> p;

	const Range<int> & xRange = coordinateHandler.getXRange();
	const Range<int> & yRange = coordinateHandler.getYRange();

	// Experimental (element horz/vert topology not yet implemented)
	// Possibly wrong...  not interchangible due to to scanning element geometry?
	/*
	bool HORZ = true;
	const Range<int> & inner     = HORZ ? xRange : yRange;
	const Range<int> & outer     = HORZ ? yRange : xRange;
	int & innerValue  = HORZ ? p.x : p.y;
	int & outerValue  = HORZ ? p.y : p.x;
	mout.debug2() << "outer range:" << outer << mout.endl;
	mout.debug2() << "inner range:" << inner << mout.endl;
	*/


	// Todo: extended area, needs coordinateHandler.handle(p);?
	//for (outerValue=outer.min; outerValue<=outer.max; outerValue++){
	//	for (innerValue=inner.min; innerValue<=inner.max; innerValue++){
	for (p.y=0; p.y<=yRange.max; ++p.y){
		for (p.x=0; p.x<=xRange.max; ++p.x){

			// Take (converted) source value as default
			d = src2dst.fwd(src.get<dist_t>(p));

			// Compare to previous value
			dTest = dst.get<dist_t>(p);
			if (dTest > d){
				d = dTest;
			}

			for (const DistanceElement & elem: chain){
				pTest.setLocation(p.x + elem.diff.x, p.y + elem.diff.y);
				coordinateHandler.handle(pTest);
				dTest = this->distanceModel.decrease(dst.get<dist_t>(pTest), elem.coeff);
				if (dTest > d){
					d = dTest;
				}
			}

			if (d > 0)
				dst.put(p,d);

		};
	};

}



template <class T>
void DistanceTransformOp<T>::traverseUpLeft(const Channel &src, Channel &dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);
	mout.debug() << "start" << mout.endl;

	//const DistanceModel & distModel = getDistanceModel();
	mout.debug2() << "distModel:" << this->distanceModel << mout.endl;

	DistanceNeighbourhood chain;
	this->distanceModel.createChain(chain, false);

	CoordinateHandler2D coordinateHandler(src);
	mout.debug2() << "coordHandler:" << coordinateHandler << mout.endl;

	//const ValueScaling src2dst(src.scaling.scale, src.scaling.offset, dst.scaling.scale, dst.scaling.offset);
	// const ValueScaling src2dst(src.getScaling(), dst.getScaling());
	const ValueScaling src2dst(dst.getConf().getTypeMax<double>()/src.getConf().getTypeMax<double>(), 0);

	// proximity (inverted distance)
	dist_t d;
	dist_t dTest;

	/// Point in the source image
	Point2D<int> pTest(0,0);

	// Point in the target image
	Point2D<int> p;


	const Range<int> & xRange = coordinateHandler.getXRange();
	const Range<int> & yRange = coordinateHandler.getYRange();

	for (p.y=yRange.max; p.y>=0; --p.y){
		for (p.x=xRange.max; p.x>=0; --p.x){

			// Take (converted) source value as default
			d = src2dst.fwd(src.get<dist_t>(p));

			// Compare to previous value
			dTest = dst.get<dist_t>(p);
			if (dTest > d){
				d = dTest;
			}

			for (const DistanceElement & elem: chain){
				pTest.setLocation(p.x + elem.diff.x, p.y + elem.diff.y);
				coordinateHandler.handle(pTest);
				dTest = this->distanceModel.decrease(dst.get<dist_t>(pTest), elem.coeff);
				if (dTest > d){
					d = dTest;
				}
			}

			if (d>0)
				dst.put(p,d);

		}
	}
	//return dst;
}


  



/// Computes inverse linear distance to bright points.
/**

\~exec
make dots.png
make dots-16b.png
\~

Examples:
\code
drainage dots.png     --iDistanceTransform 70      -o dist.png
drainage dots-16b.png --iDistanceTransform 25      -o dist-16b.png
drainage dots.png     --iDistanceTransform 70,70,0 -o dist-diamond.png
drainage dots.png     --iDistanceTransform 70,70,1 -o dist-simple.png
drainage dots.png     --iDistanceTransform 40,20   -o dist-horz.png

\endcode

*/
class DistanceTransformLinearOp : public DistanceTransformOp<DistanceModelLinear>
{
public:

	inline
	DistanceTransformLinearOp(float horz = 10.0, float vert = NAN, DistanceModel::topol_t topology=2) :
	DistanceTransformOp<DistanceModelLinear>(__FUNCTION__, "Linearly decreasing intensities. Set decrements.", horz, vert, topology) {
	};


};

/** Computes inverse exponential distance to bright points.

Examples:
 \code
 drainage dots.png       --iDistanceTransformExp 25    -o distExp.png
 drainage dots-16bit.png --iDistanceTransformExp 25    -o distExp-16b.png
 drainage dots.png       --iDistanceTransformExp 40,20 -o distExp-horz.png
 drainage dots-16bit.png --iDistanceTransformExp 10:40,20:80 -o distExp-asym.png
 \endcode

 TODO: gnuplot curves
 */
class DistanceTransformExponentialOp : public DistanceTransformOp<DistanceModelExponential>
{
public:

	/// Constructor with default dimensions.
	/**
	 *
	 */
	inline
	DistanceTransformExponentialOp(dist_t horz = 10.0, dist_t vert = NAN, DistanceModel::topol_t topology=2) :
		DistanceTransformOp<DistanceModelExponential>(__FUNCTION__, "Exponentially decreasing intensities. Set half-decay radii.",	horz, vert, topology) {
	};

};	    

}
}
	
#endif /*DISTANCETRANSFORMOP_H_*/

// Drain

