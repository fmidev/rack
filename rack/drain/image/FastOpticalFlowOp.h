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
#ifndef SLIDINGWINDOWOPTICALFLOWOP_H_
#define SLIDINGWINDOWOPTICALFLOWOP_H_

#include <sstream>
#include <math.h>

//#include "SlidingWindow.h"
#include "SlidingWindowOp.h"
#include "QuadraticSmootherOp.h"
#include "GradientOp.h"

namespace drain
{

namespace image
{

/// Consider using SlidingStripeOpticalFlow instead
/**
 *  T  = class of the input
 *  T2 = class of the derivative  images, weight, and output (motion field and weight)
 */

class SlidingOpticalFlowParams : public WindowConfig {
    public: //re 

};

class SlidingOpticalFlow : public SlidingWindow<SlidingOpticalFlowParams> {

public:

	typedef SlidingOpticalFlow unweighted;

	double wXX;
	double wXY;
	double wYY;
	double wXT;
	double wYT;
	//double area;

	/// Precomputed horizontal diffential 
	Image Dx;

	/// Precomputed vertical diffential 
	Image Dy;

	/// Precomputed time diffential 
	Image Dt;

	/// Precomputed weight field (optional) 
	Image w;

	/// Horizontal velocity
	Image u;

	/// Vertical velocity
	Image v;

	/// Quality of the velocity
	Image q;


	SlidingOpticalFlow(int width = 0, int height = 0) : SlidingWindow<SlidingOpticalFlowParams>(width, height) {
		//setSize(width,height);
	}

	SlidingOpticalFlow(const config & conf) : SlidingWindow<SlidingOpticalFlowParams>(conf) {
		//setSize(conf.width, conf.height);
	}


	// protected:
	double areaD;

	virtual inline
	void initialize() {

		/// Source
		Dx.setView(src, 0);
		Dy.setView(src, 1);
		Dt.setView(src, 2);
		w.setView(srcWeight);

		/// Target
		u.setView(dst,0);
		v.setView(dst,1);
		q.setView(dst,2);

		areaD = getArea();
		//coordinateHandler.setPolicy(src.getCoordinatePolicy());
		//------------------------------
		//clear();
		//fill(0, 0);

	}

	/**
	 *   TODO: weight = > weightSUm
	 *   TODO: multiple channels (banks)?
	 */
	virtual
	void clear(){
		wXX = 0.0;
		wXY = 0.0;
		wYY = 0.0;
		wXT = 0.0;
		wYT = 0.0;
	}




	virtual
	inline
	void addPixel(Point2D<int> & p){

		if (! coordinateHandler.validate(p))
			return;

		address = dst.address(p.x, p.y);

		DX = Dx.get<double>(address);
		DY = Dy.get<double>(address);
		DT = Dt.get<double>(address);
		W = 1;
		//W  = w.get<double>(address);

		// if ((location.x == 100) && (location.y == 100)) std::cout << locationHandled << '\n';

		wXX += W*DX*DX;
		wXY += W*DX*DY;
		wYY += W*DY*DY;
		wXT += W*DX*DT;
		wYT += W*DY*DT;

	}

	virtual inline
	void removePixel(Point2D<int> & p){

		if (! coordinateHandler.validate(p))
			return;

		address = dst.address(p.x, p.y);

		DX = Dx.get<double>(address);
		DY = Dy.get<double>(address);
		DT = Dt.get<double>(address);
		W = 1;
		//W  = w.get<double>(address);

		wXX -= W*DX*DX;
		wXY -= W*DX*DY;
		wYY -= W*DY*DY;
		wXT -= W*DX*DT;
		wYT -= W*DY*DT;

	}


	inline
	double nominator() const {
		return static_cast<double>(wXX*wYY - wXY*wXY);
	};

	/// Returns the horizontal component of motion. Must be scaled by nominator().
	inline
	double uDenominator() const {
		return static_cast<double>(wXY*wYT - wYY*wXT);
	};

	/// Returns the vertical component of motion. Must be scaled by nominator().
	inline
	double vDenominator() const {
		return static_cast<double>(wXY*wXT - wXX*wYT);
	};


	virtual inline
	void write(){

		nom = nominator();
		quality = sqrt(nom/(areaD*areaD*2048.0));  // hihasta kerroin

		if (quality > 0.01){  // todo minQuality

			/*  // DEBUGGING
			if (this->locationHandled.x == this->locationHandled.y)
				if ((this->locationHandled.x & 7) == 0){
					//std::cout << nom << '\t' << wXX << '\t' << wYY << '\t' << wXY << '\t' << areaD << '\n';
					//std::cout << nom << '\t' << DX << '\t' << DY << '\t' << DT << '\n';
					std::cout << uDenominator()/nom << '\t' << vDenominator()/nom << '\t' << quality << '\n';
				}
			*/

			u.put(location, uDenominator()/nom);
			v.put(location, vDenominator()/nom);
			q.put(location, sqrt(static_cast<double>( 255.0 - 255.0 / (1.0+quality) )));
		}
		else {
			u.put(location, 0);
			v.put(location, 0);
			q.put(location, 0);
		}

	}

private:

	// Point2D<int> p;

	mutable
	size_t address;

	mutable
	double DX, DY, DT, W;

	mutable double nom;
	mutable double quality;

};



//------------------------------------------------------------------------------------------


/// Detects motion between two subsequent images. Does not extrapolate images.
/// Applies recursive correction for smoothing motion field.
/**  Notice:
 *   T  input data (images)
 *   T2 all the other arrays (differentials, motion)
 *
 *   - gradPow - highlighting the role of gradient magnitude
 *   - gradWidth -
 */
class FastOpticalFlowOp : public SlidingWindowOpT<SlidingOpticalFlow> {

public:

	double smoothing;
	double gradPow;
	//double gradWidth;

	FastOpticalFlowOp(int width=5, int height=5, double smoothing=0.01) : //, double gradPow=2.0) : //, double gradWidth = 16) :
		SlidingWindowOpT<SlidingOpticalFlow>(__FUNCTION__, "A pipeline implementation of optical flow. "){
		this->parameters.reference("smoothing", this->smoothing = smoothing);
	}

	/// Creates a 2+1 channel target image for storing motion (u,v) and quality (q).
	virtual
	inline
	void makeCompatible(const Image & src, Image & dst) const  {
		size_t width = src.getWidth();
		size_t height = src.getHeight();
		dst.setType<double>();
		dst.setGeometry(width, height, 2, 1);
	};

	/// Computes an image with channels dx, dy, dt and w (quality of gradients). User may redefine this.
	virtual
	void computeDerivativeImage(const Image & src1, const Image & src2, Image & dst) const;

	inline
	void smoothen(const Image & src, Image & dst) const {
		//QuadraticSmootherOp(width/2,height/2, smoothing).filter(src, dst);
		QuadraticSmootherOp(conf.width, conf.height, smoothing).filter(src, dst);
	}

protected:

	inline
	bool traverseChannelsSeparately(const Image & src, Image & dst) const {
		return false;
	}

	inline
	bool traverseChannelsSeparately(const Image & src, const Image & srcWeight, Image & dst, Image & dstWeight) const {
		return false;
	}

	void traverse(const Image & src, Image & dst) const {
		//Image tmp;
		MonitorSource mout(iMonitor, name, __FUNCTION__);
		mout.error() << "Not implemented (1/1)" << mout.endl;
	};

	/*
	void traverse(const Image & src, const Image & srcWeight, Image & dst, Image & dstWeight) const {
		//Image tmp;
		MonitorSource mout(iMonitor, name, __FUNCTION__);
		mout.error() << "Not implemented (2/2)" << mout.endl;
	};
	*/

	//mutable SlidingOpticalFlow opticalFlowWindow;

private:


	/// Computes 2d intensity derivative (horz and vert)
	inline
	void computeGradient(const Image & src, Image & dstHorz, Image & dstVert) const {
		GradientHorizontalOp(1.0, 0.0, +2).filter(src, dstHorz);
		GradientVerticalOp(1.0, 0.0, -2).filter(src, dstVert);
	}

public:

	void writeDebugImage(const Image & src, const std::string & filename, double scale=1.0, double bias=0.0) const;

};

}
}

#endif

// Drain
