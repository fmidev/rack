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
    	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
    	mout.note("discarding alpha channels, redirecting to traverseChannel(src, dst)");
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
    virtual // TODO: non-virtual, ie, final!
	void makeCompatible(const ImageConf & src, Image & dst) const {
    	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
    	mout.warn() << "derived " << mout;
    	ImageOp::makeCompatible(src, dst);
    }
    */

    /*
	virtual
	inline
	void make Compatible(const ImageFrame &src, Image &dst) const  {

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.debug3() << "src: " << src << mout.endl;

		//if (!dst.typeIsSet())
			//dst.setType(src.getType());
		if (dst.getType() != src.getType()){
			mout.note() << " changing dst image type: " << dst.getType().name() << '>' << src.getType().name() << mout.endl;
		}

		dst.copyShallow(src);
		// dst.initialize(src.getType(), src.getGeometry());
		// dst.adoptScaling(src);

		mout .debug3() << "dst: " << dst << mout.endl;

	};
	*/

    // Debugging

	virtual
	const DistanceModel & getDistanceModel() const {
		return distanceModel;
	};

	virtual
	DistanceModel & getDistanceModel() {
		return distanceModel;
	};


protected:

	mutable T distanceModel;

	//int topology;

	DistanceTransformOp(const std::string &name, const std::string &description, float width, float height,
			DistanceModel::topol_t topology=DistanceModel::KNIGHT) : // PIX_ADJACENCY_
		ImageOp(name, description) {
		parameters.append(this->distanceModel.getParameters());
		distanceModel.setTopology(topology);
		distanceModel.setRadius(width, height, width, height);
	};

	DistanceTransformOp(const DistanceTransformOp & op) : ImageOp(op) {
		parameters.append(this->distanceModel.getParameters());
		//setParameters(op.getParameters());
		this->distanceModel.setParameters(op.distanceModel.getParameters());
		this->distanceModel.update(); // Important: handles NAN's
	};

	// Extend default range (of src) when coord policy is WRAP
	Range<int> getHorzRange(const CoordinateHandler2D & coordinateHandler) const ;

	// Extend default range (of src) when coord policy is WRAP
	Range<int> getVertRange(const CoordinateHandler2D & coordinateHandler) const;


	/// Sets internal parameters
	/**
	 *  Max value in the model should be set according to the channel which is used as control field.
	 */
	virtual
	void initializeParameters(const ImageFrame &src, const ImageFrame &dst) const {

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		// WAS: src (all)  // .getEncoding()
		const double physMax = dst.getConf().requestPhysicalMax(100.0);
		const double codeMax = dst.getScaling().inv(physMax);

		mout.debug("dst of type=", Type::getTypeChar(dst.getType()), ", scaling: ", dst.getScaling());

		if (dst.getScaling().isPhysical()){
			mout.info("ok, physical scaling [", dst.getConf().getPhysicalRange(), "] : ");
		}
		else if (Type::call<typeIsSmallInt>(dst.getType())){
			mout.note("no physical scaling, but small int, guessing: ");
		}
		else {
			mout.warn("no physical scaling, no small int: default ");
		}
		mout.note("physMax=", physMax, " => ", codeMax);

		distanceModel.setMax(codeMax);
		// distModel.setMax(dst.getMax<double>()); // why not dst
		distanceModel.update(); // radii
	}



	void traverseDownRight(const Channel & src, Channel & dst) const ;

	void traverseUpLeft(const Channel & src, Channel & dst) const ;



};

template <class T>
Range<int> DistanceTransformOp<T>::getHorzRange(const CoordinateHandler2D & coordinateHandler) const {

	Range<int> xRange = coordinateHandler.getXRange();
	const Bidirectional<float> & radiusHorz = getDistanceModel().getRadiusHorz();

	if (coordinateHandler.policy.xUnderFlowPolicy == CoordinatePolicy::WRAP)
		xRange.min -= radiusHorz.backward;

	if (coordinateHandler.policy.xOverFlowPolicy == CoordinatePolicy::WRAP)
		xRange.max += radiusHorz.forward;

	return xRange;
}

template <class T>
Range<int> DistanceTransformOp<T>::getVertRange(const CoordinateHandler2D & coordinateHandler) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	Range<int> yRange = coordinateHandler.getYRange();

	// mout.warn() << "yRange: " << yRange << mout;

	const Bidirectional<float> & radiusVert = getDistanceModel().getRadiusVert();

	// mout.warn() << "radiusVert: " << radiusVert << mout;


	if (coordinateHandler.policy.yUnderFlowPolicy == CoordinatePolicy::WRAP)
		yRange.min -= radiusVert.backward;

	if (coordinateHandler.policy.yOverFlowPolicy == CoordinatePolicy::WRAP)
		yRange.max += radiusVert.forward;

	return yRange;

}

template <class T>
void DistanceTransformOp<T>::traverseChannel(const Channel &src, Channel & dst) const
{

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	//mout.warn() << "start" << mout.endl;
	mout.debug("model max: ",  getDistanceModel().getMax());

	//File::write(dst,"DistanceTransformOp-dst0.png");
	traverseDownRight(src,dst);
	//File::write(dst,"DistanceTransformOp-dst1.png");
	traverseUpLeft(dst,dst);
	//File::write(dst,"DistanceTransformOp-dst2.png");

}


template <class T>
void DistanceTransformOp<T>::traverseDownRight(const Channel &src, Channel &dst) const
{

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	//mout.debug() << "start" << mout.endl;

	//const DistanceModel & distModel = getDistanceModel();
	mout.debug2("distModel:", this->distanceModel);

	DistanceNeighbourhood chain;
	this->distanceModel.createChain(chain, true);

	CoordinateHandler2D coordinateHandler(src); // TODO: change param to ImageConf
	mout.debug2("coordHandler:", coordinateHandler);


	const ValueScaling src2dst(dst.getConf().getTypeMax<double>()/src.getConf().getTypeMax<double>(), 0);
	//const ValueScaling src2dst(src.getScaling(), dst.getScaling());

	for (double d: {0.0, 0.5, 1.0, 255.0, 65535.0}){
		mout.special(d, " => ", src2dst.fwd(d), " inv:", src2dst.inv(d));
	}

	// proximity (inverted distance)
	dist_t d;
	dist_t dTest;

	// Point in the source image
	Point2D<int> pTest;

	// Point in the target image
	Point2D<int> p;
	Point2D<int> pSafe;

	Range<int> xRange = getHorzRange(coordinateHandler); //coordinateHandler.getXRange();
	Range<int> yRange = getVertRange(coordinateHandler); // coordinateHandler.getYRange();
	mout.special() << xRange << ',' << yRange << mout;

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
	//Range<int>

	// Todo: extended area, needs coordinateHandler.handle(p);?
	for (p.y=yRange.min; p.y<=yRange.max; ++p.y){
		for (p.x=xRange.min; p.x<=xRange.max; ++p.x){

			pSafe.setLocation(p);
			if (coordinateHandler.handle(pSafe) == CoordinateHandler2D::IRREVERSIBLE)
				continue;

			// Take (converted) source value as default
			d = src2dst.fwd(src.get<dist_t>(pSafe));

			// Compare to previous value
			dTest = dst.get<dist_t>(pSafe);
			if (dTest > d){
				d = dTest;
			}

			for (const DistanceElement & elem: chain){
				pTest.setLocation(pSafe.x + elem.diff.x, pSafe.y + elem.diff.y);
				coordinateHandler.handle(pTest);
				dTest = this->distanceModel.decrease(dst.get<dist_t>(pTest), elem.coeff);
				if (dTest > d){
					d = dTest;
				}
			}

			if (d > 0){
				dst.put(pSafe,d);
			}

		};
	};

}



template <class T>
void DistanceTransformOp<T>::traverseUpLeft(const Channel &src, Channel &dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	mout.debug("start");

	//const DistanceModel & distModel = getDistanceModel();
	mout.debug2("distModel:", this->distanceModel);

	DistanceNeighbourhood chain;
	this->distanceModel.createChain(chain, false);

	CoordinateHandler2D coordinateHandler(src);
	mout.debug2("coordHandler:", coordinateHandler);

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
	Point2D<int> pSafe;

	//const Range<int> & xRange = coordinateHandler.getXRange();
	//const Range<int> & yRange = coordinateHandler.getYRange();
	Range<int> xRange = getHorzRange(coordinateHandler); //coordinateHandler.getXRange();
	Range<int> yRange = getVertRange(coordinateHandler); // coordinateHandler.getYRange();
	mout.special(xRange, ',', yRange);

	for (p.y=yRange.max; p.y>=yRange.min; --p.y){
		for (p.x=xRange.max; p.x>=xRange.min; --p.x){

			pSafe.setLocation(p);
			if (coordinateHandler.handle(pSafe) == CoordinateHandler2D::IRREVERSIBLE)
				continue;

			// Take (converted) source value as default
			d = src2dst.fwd(src.get<dist_t>(pSafe));

			// Compare to previous value
			dTest = dst.get<dist_t>(pSafe);
			if (dTest > d){
				d = dTest;
			}

			for (const DistanceElement & elem: chain){
				pTest.setLocation(pSafe.x + elem.diff.x, pSafe.y + elem.diff.y);
				coordinateHandler.handle(pTest);
				dTest = this->distanceModel.decrease(dst.get<dist_t>(pTest), elem.coeff);
				if (dTest > d){
					d = dTest;
				}
			}

			if (d>0)
				dst.put(pSafe,d);

		}
	}


}


  



/// Computes inverse linear distance to bright points.
/**

\~exec
make dots.png
make dots-16b.png
\~

Examples:
\code
# Default: 16-connected topology (chess king and knight combined)
drainage dots.png     --iDistanceTransform 70      -o dist.png
drainage dots-16b.png --iDistanceTransform 25      -o dist-16b.png
# Simpler 4-connected topology ("diamond" or "city-block" distance)
drainage dots.png     --iDistanceTransform 70,70,4-CONNECTED -o dist-diamond.png
# 8-connected topology (chess king)
drainage dots.png     --iDistanceTransform 70,70,8-CONNECTED -o dist-simple.png
# Horizontally distorted
drainage dots.png     --iDistanceTransform 40,20   -o dist-horz.png
# Asymmetric topology
drainage dots.png     --iDistanceTransform 10:40,20:80  -o dist-horz.png

\endcode

*/
class DistanceTransformLinearOp : public DistanceTransformOp<DistanceModelLinear>
{
public:

	inline
	DistanceTransformLinearOp(float horz = 10.0, float vert = NAN, DistanceModel::topol_t topology = DistanceModel::KNIGHT): // PIX_ADJACENCY_
	DistanceTransformOp<DistanceModelLinear>(__FUNCTION__, "Linearly decreasing intensities - applies decrements.", horz, vert, topology) {
	};


};

/** Computes inverse exponential distance to bright points.

Examples:
 \code
 # Basic example
 drainage dots.png     --iDistanceTransformExp 25    -o distExp.png

 # 16-bit image
 drainage dots-16b.png --iDistanceTransformExp 25    -o distExp-16b.png

 # Horizontal topology
 drainage dots.png     --iDistanceTransformExp 40,20 -o distExp-horz.png

 # Asymmetric distances
 drainage dots-16b.png --iDistanceTransformExp 10:40,20:80 -o distExp-asym.png
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
	DistanceTransformExponentialOp(dist_t horz = 10.0, dist_t vert = NAN, DistanceModel::topol_t topology = DistanceModel::KNIGHT): // PIX_ADJACENCY_
		DistanceTransformOp<DistanceModelExponential>(__FUNCTION__, "Exponentially decreasing intensities. Set half-decay radii.",	horz, vert, topology) {
	};

};	    

}  // image::
}  // drain::
	
#endif /*DISTANCETRANSFORMOP_H_*/

