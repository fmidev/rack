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

#include "image/DistanceModel.h"

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

	typedef typename T::dist_t dist_t;
    
    virtual ~DistanceTransformOp(){};

    inline
    void setRadius(dist_t width, dist_t height){
    	distanceModel.setRadius(width, height);
    };

    /*
    virtual inline
    void process(const ImageTray<const Channel> & srcChannels, ImageTray<Image> & dstImages) const {
    	drain::Logger mout(getImgLog(), __FUNCTION__, getName());
    	mout.note() << "redirecting to ImageOp::process(srcChannels, dstImages, false)" << mout.endl;
    	ImageOp::processConditional(srcChannels, dstImages, false);
    }
    */


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
	virtual
	inline
	void makeCompatible(const ImageFrame &src, Image &dst) const  {

		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.debug(2) << "src: " << src << mout.endl;

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


protected:

	mutable T distanceModel;

	//int topology;

	DistanceTransformOp(const std::string &name, const std::string &description, double width, double height) :
		ImageOp(name, description) {
		parameters.append(this->distanceModel.getParameters());
		distanceModel.setTopology(2);
		distanceModel.setRadius(width, height);
	};


	/// Sets internal parameters
	/**
	 *  Max value in the model should be set according to the channel which is used as control field.
	 */
	virtual
	inline
	void initializeParameters(const ImageFrame &src, const ImageFrame &dst) const {

		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);

		const double physMax = src.requestPhysicalMax(100.0);
		const double codeMax = src.getScaling().inv(physMax);

		mout.debug() << "src of type=" << Type::getTypeChar(src.getType()) << ", scaling: " << src.getScaling();


		if (src.getScaling().isPhysical()){
			mout.info() << "ok, physical scaling: ";
		}
		else if (Type::call<typeIsSmallInt>(src.getType())){
			mout.note() << "no physical scaling, but small int, guessing: ";
		}
		else {
			mout.warn() << "no physical scaling, no small int: default ";
		}
		mout << "physMax=" << physMax << " => " << codeMax << mout.endl;


		distanceModel.setMax(codeMax);
		// distanceModel.setMax(src.getMax<double>()); // why not dst
		distanceModel.init();
	}


	void traverseDownRight(const Channel & src, Channel & dst) const ;

	void traverseUpLeft(const Channel & src, Channel & dst) const ;


	virtual
	const DistanceModel & getDistanceModel() const { return distanceModel; }; // const = 0; //

	virtual
	DistanceModel & getDistanceModel() { return distanceModel; }; // = 0; //


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

	const DistanceModel & distanceModel = getDistanceModel();
	mout.debug(1) << "distanceModel:" << distanceModel << mout.endl;

	CoordinateHandler2D coordinateHandler;
	src.adjustCoordinateHandler(coordinateHandler);
	mout.debug(1) << "coordHandler:" << coordinateHandler << mout.endl;

	// proximity (inverted distance)

	dist_t d;
	dist_t dTest;

	// Point in the source image
	Point2D<int> pTest;

	// Point in the target image
	Point2D<int> p;
	int &px = p.x;
	int &py = p.y;


	// Experimental
	// Possibly wrong. Theres are not interchangible due to to scanning element geometry?
	typedef std::pair<int,int> range;
	range horz(0, coordinateHandler.getXMax()); // note: including 0 and max.
	range vert(0, coordinateHandler.getYMax()); // note: including 0 and max.

	// Experimental (element topology not yet implemented)
	bool HORZ = true;
	range & inner = HORZ ? horz : vert;
	range & outer = HORZ ? vert : horz;
	int & innerValue  = HORZ ? p.x : p.y;
	int & outerValue  = HORZ ? p.y : p.x;

	std::pair<int,int> g;

	mout.debug(1) << "outer range:" << outer.first << ':' << outer.second << mout.endl;
	mout.debug(1) << "inner range:" << inner.first << ':' << inner.second << mout.endl;

	// Todo: extended area
	for (outerValue=outer.first; outerValue<=outer.second; outerValue++)
	{
		for (innerValue=inner.first; innerValue<=inner.second; innerValue++)
		//for (px=0; px<width; px++)
		{

			// Take source value as default
			d = src.get<dist_t>(p);  // todo  convert to dst scale?

			// Compare to previous value
			dTest = dst.get<dist_t>(p);
			if (dTest > d)
			{
				d = dTest;
			}

			// Compare to upper left neighbour
			if (distanceModel.DIAG){
				pTest.setLocation(px-1,py-1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseDiag(dst.get<dist_t>(pTest));
				if (dTest > d)
				{
					d = dTest;
				}
			}

			// Compare to upper neighbour
			pTest.setLocation(px,py-1);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseVert(dst.get<dist_t>(pTest));
			if (dTest > d)
			{
				d = dTest;
			}

			// Compare to upper right neighbour
			if (distanceModel.DIAG){
				pTest.setLocation(px+1,py-1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseDiag(dst.get<dist_t>(pTest));
				if (dTest > d)
				{
					d = dTest;
				}
			}

			// Compare to left neighbour
			pTest.setLocation(px-1,py);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseHorz(dst.get<dist_t>(pTest));
			if (dTest > d)
			{
				d = dTest;
			}

			if (distanceModel.KNIGHT){

				// Compare to further upper, left neighbour
				pTest.setLocation(px-1, py-2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further upper, right neighbour
				pTest.setLocation(px+1, py-2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further left, upper  neighbour
				pTest.setLocation(px-2, py-1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further right, upper neighbour
				pTest.setLocation(px+2, py-1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}

			}


			if (d > 0)
				dst.put(p,d);


		};
	};

}


//public WritableRaster traverseUpLeft(Raster srcDist, WritableRaster dst){
//template <class T=unsigned char,class T2=unsigned char>
template <class T>
void DistanceTransformOp<T>::traverseUpLeft(const Channel &src, Channel &dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);
	mout.debug() << "start" << mout.endl;

	const DistanceModel & distanceModel = getDistanceModel();
	mout.debug(1) << "distanceModel:" << distanceModel << mout.endl;

	CoordinateHandler2D coordinateHandler;
	src.adjustCoordinateHandler(coordinateHandler);
	mout.debug(1) << "coordHandler:" << coordinateHandler << mout.endl;

	// proximity (inverted distance)
	//typedef float dist_t;
	dist_t d;
	dist_t dTest;

	/// Point in the source image
	Point2D<int> pTest(0,0);

	// Point in the target image
	Point2D<int> p;
	int & px = p.x;
	int & py = p.y;


	for (py=coordinateHandler.getYMax(); py>=0; --py)
	{
		for (px=coordinateHandler.getXMax(); px>=0; --px)
		{
			// Source
			d = src.get<dist_t>(p);

			// Compare to previous value
			dTest = dst.get<dist_t>(p);
			if (dTest > d){
				d = dTest;
			}

			// Compare to lower left neighbour
			if (distanceModel.DIAG){
				pTest.setLocation(px-1,py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseDiag(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}
			}

			// Compare to lower neighbour
			pTest.setLocation(px,py+1);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseVert(dst.get<dist_t>(pTest));
			if (dTest > d){
				d = dTest;
			}

			// Compare to lower right neighbour
			if (distanceModel.DIAG){
				pTest.setLocation(px+1,py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseDiag(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}
			}

			// Compare to right neighbour
			pTest.setLocation(px+1,py);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseHorz(dst.get<dist_t>(pTest));
			if (dTest > d){
				d = dTest;
			}

			if (distanceModel.KNIGHT){

				// Compare to further lower, right neighbour
				pTest.setLocation(px+1, py+2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further lower, left neighbour
				pTest.setLocation(px-1, py+2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further right, lower  neighbour
				pTest.setLocation(px+2, py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further left, lower neighbour
				pTest.setLocation(px-2, py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dst.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
				}

			} // knight

			if (d>0)
				dst.put(p,d);


		}
	}
	//return dst;
}


  



/// Computes inverse linear distance to bright points.
/**
\~exec
make dots.png #make
make dots-16bit.png #make
\~

Examples:
\code
drainage dots.png       --distanceTransform 70      -o dist.png
drainage dots.png       --distanceTransform 70,70,0 -o dist-diamond.png
drainage dots.png       --distanceTransform 70,70,1 -o dist-simple.png
drainage dots.png       --distanceTransform 25,10  -o dist-horz.png
drainage dots-16bit.png --distanceTransform 25    -o dist-16b.png

\endcode

*/
class DistanceTransformLinearOp : public DistanceTransformOp<DistanceModelLinear>
{
public:

	inline
	DistanceTransformLinearOp(double horz = 10.0, double vert = NAN) :
	DistanceTransformOp<DistanceModelLinear>(__FUNCTION__, "Linearly decreasing intensities. Set decrements.", horz, vert) {
	};


};

/** Computes inverse exponential distance to bright points.

Examples:
 \code
 drainage dots.png       --distanceTransformExp 25    -o distExp.png
 drainage dots.png       --distanceTransformExp 25,10 -o distExp-horz.png
 drainage dots-16bit.png --distanceTransformExp 25    -o distExp-16b.png
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
	DistanceTransformExponentialOp(double horz = 10.0, double vert = NAN) :
		DistanceTransformOp<DistanceModelExponential>(__FUNCTION__, "Exponentially decreasing intensities. Set half-decay radii.",	horz, vert) {
	};

};	    

}
}
	
#endif /*DISTANCETRANSFORMOP_H_*/

// Drain
