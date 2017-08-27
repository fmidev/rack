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


#include "ImageOp.h"

#include "DistanceModel.h"

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
    
    
    virtual ~DistanceTransformOp(){};

	void traverse(const Image &src, Image &dst) const ;

	double horz;
	double vert;
	//double diag;


	/// Ensures dst the same geometry and type with src.
	/*
	 */
	virtual
	inline
	void makeCompatible(const Image &src, Image &dst) const  {

		drain::MonitorSource mout(iMonitor, name+"(ImageOp)", __FUNCTION__);
		mout.debug(2) << "src: " << src << mout.endl;

		//if (!dst.typeIsSet())
			//dst.setType(src.getType());
		if (dst.getType() != src.getType()){
			mout.note() << " changing dst image type: " << dst.getType().name() << '>' << src.getType().name() << mout.endl;
		}

		dst.setType(src.getType());
		dst.setGeometry(src.getGeometry());

		mout.debug(3) << "dst: " << dst << mout.endl;

	};


protected:

	mutable T distanceModel;

	DistanceTransformOp(const std::string &name, const std::string &description, double horz, double vert) :
			//int weight_supported, int in_place, bool multichannel) :
		ImageOp(name, description) { // , distanceModel(model)
		parameters.reference("horz", this->horz = horz);
		parameters.reference("vert", this->vert = vert);
	};


	/// Sets internal parameters
	inline
	virtual
	void initializeParameters(const Image &src, const Image &dst) const {
		distanceModel.setMax(src.getMax<double>());
		distanceModel.setRadius(horz, vert);
	}


	void traverseDownRight(const Image &src, Image &dst) const ;
	void traverseUpLeft(const Image &src, Image &dst) const ;


	virtual
	const DistanceModel & getDistanceModel() const { return distanceModel; }; // const = 0; //

	virtual
	DistanceModel & getDistanceModel() { return distanceModel; }; // = 0; //

protected:

	//DistanceModel & distanceModel;

	//mutable DistanceModelLinear distanceLinear;
 	//mutable DistanceModelExponential distanceExponential;
};
  


template <class T>
void DistanceTransformOp<T>::traverse(const Image &src, Image &dst) const
{


	MonitorSource mout(iMonitor, name, __FUNCTION__);

	//mout.warn() << getDistanceModel() << mout.endl;
	mout.debug() << "model max" << getDistanceModel().getMax() << mout.endl;


	//File::write(dst,"DistanceTransformOp-dst0.png");
	traverseDownRight(src,dst);
	//File::write(dst,"DistanceTransformOp-dst1.png");
	traverseUpLeft(dst,dst);
	//File::write(dst,"DistanceTransformOp-dst2.png");

}


template <class T>
void DistanceTransformOp<T>::traverseDownRight(const Image &src, Image &dst) const
{

	const DistanceModel & distanceModel = getDistanceModel();

	const int width  = src.getWidth();
	const int height = src.getHeight();
	const CoordinateHandler2D coordinateHandler(width, height, src.getCoordinatePolicy());

	// proximity (inverted distance)
	typedef float dtype;
	dtype d;
	dtype dTest;

	// Point in the source image
	Point2D<int> pTest;

	// Point in the target image
	Point2D<int> p;
	int &px = p.x;
	int &py = p.y;

	/// NEW
	struct iter {
		int start;
		int end;
	//	int value;
	};


	typedef std::pair<int,int> range;
	range horz;
	horz.first = 0;
	horz.second = width;

	range vert;
	vert.first = 0;
	vert.second = height;

	bool HORZ = true;
	range & inner = HORZ ? horz : vert;
	range & outer = HORZ ? vert : horz;
	int & innerValue  = HORZ ? p.x : p.y;
	int & outerValue  = HORZ ? p.y : p.x;

	std::pair<int,int> g;


	// Todo: extended area
	//for (py=0; py<height; py++)
	for (outerValue=outer.first; outerValue<outer.second; outerValue++)
	{
		for (innerValue=inner.first; innerValue<inner.second; innerValue++)
		//for (px=0; px<width; px++)
		{

			// Take source value as default
			d = src.get<dtype>(p);  // todo  convert to dst scale?

			// Compare to previous value
			dTest = dst.get<dtype>(p);
			if (dTest > d)
			{
				d = dTest;
			}

			// Compare to upper left neighbour
			pTest.setLocation(px-1,py-1);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseDiag(dst.get<dtype>(pTest));
			if (dTest > d)
			{
				d = dTest;
			}

			// Compare to upper neighbour
			pTest.setLocation(px,py-1);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseVert(dst.get<dtype>(pTest));
			if (dTest > d)
			{
				d = dTest;
			}

			// Compare to upper right neighbour
			pTest.setLocation(px+1,py-1);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseDiag(dst.get<dtype>(pTest));
			if (dTest > d)
			{
				d = dTest;
			}

			// Compare to left neighbour
			pTest.setLocation(px-1,py);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseHorz(dst.get<dtype>(pTest));
			if (dTest > d)
			{
				d = dTest;
			}

			if (distanceModel.KNIGHT){

				// Compare to further upper, left neighbour
				pTest.setLocation(px-1, py-2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dst.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further upper, right neighbour
				pTest.setLocation(px+1, py-2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dst.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further left, upper  neighbour
				pTest.setLocation(px-2, py-1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dst.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further right, upper neighbour
				pTest.setLocation(px+2, py-1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dst.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
				}

			}


			if (d > 0)
				dst.put(p,d);


		};
	};
	//return dst;

}


//public WritableRaster traverseUpLeft(Raster srcDist, WritableRaster dst){
//template <class T=unsigned char,class T2=unsigned char>
template <class T>
void DistanceTransformOp<T>::traverseUpLeft(const Image &src, Image &dst) const {

	const int width  = src.getWidth();
	const int height = src.getHeight();

	const CoordinateHandler2D coordinateHandler(width, height, src.getCoordinatePolicy());

	// proximity (inverted distance)
	typedef float dtype;
	dtype d;
	dtype dTest;

	/// Point in the source image
	Point2D<int> pTest(0,0);

	// Point in the target image
	Point2D<int> p;
	int & px = p.x;
	int & py = p.y;

	const DistanceModel & distanceModel = getDistanceModel();

	for (py=height-1; py>=0; --py)
	{
		for (px=width-1; px>=0; --px)
		{
			// Source
			d = src.get<dtype>(p);

			// Compare to previous value
			dTest = dst.get<dtype>(p);
			if (dTest > d){
				d = dTest;
			}

			// Compare to lower left neighbour
			pTest.setLocation(px-1,py+1);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseDiag(dst.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
			}

			// Compare to lower neighbour
			pTest.setLocation(px,py+1);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseVert(dst.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
			}

			// Compare to lower right neighbour
			pTest.setLocation(px+1,py+1);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseDiag(dst.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
			}

			// Compare to right neighbour
			pTest.setLocation(px+1,py);
			coordinateHandler.handle(pTest);
			dTest = distanceModel.decreaseHorz(dst.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
			}

			if (distanceModel.KNIGHT){

				// Compare to further lower, right neighbour
				pTest.setLocation(px+1, py+2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dst.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further lower, left neighbour
				pTest.setLocation(px-1, py+2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dst.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further right, lower  neighbour
				pTest.setLocation(px+2, py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dst.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
				}

				// Compare to further left, lower neighbour
				pTest.setLocation(px-2, py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dst.get<dtype>(pTest));
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
make dots.png
make dots-16bit.png
\~

Examples:
 \code
 drainage dots.png       --distanceTransform 15    -o dist.png
 drainage dots.png       --distanceTransform 15,5  -o dist-horz.png
 drainage dots-16bit.png --distanceTransform 15    -o dist-16b.png
 \endcode

*/
class DistanceTransformLinearOp : public DistanceTransformOp<DistanceModelLinear>
{
public:

	inline
	DistanceTransformLinearOp(double horz = 10.0, double vert = -1.0) :
	DistanceTransformOp<DistanceModelLinear>(__FUNCTION__, "Linearly decreasing intensities. Set decrements.", horz, vert) {
	};


};

/** Computes inverse exponential distance to bright points.

Examples:
 \code
 drainage dots.png       --distanceTransformExp 15    -o distExp.png
 drainage dots.png       --distanceTransformExp 15,5  -o distExp-horz.png
 drainage dots-16bit.png --distanceTransformExp 15    -o distExp-16b.png
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
	DistanceTransformExponentialOp(double horz = 10.0, double vert = -1.0) :
		DistanceTransformOp<DistanceModelExponential>(__FUNCTION__, "Exponentially decreasing intensities. Set half-decay radii.",	horz, vert) {
	};

};	    

}
}
	
#endif /*DISTANCETRANSFORMOP_H_*/

// Drain
