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

//#include "image/ImageTray.h"
#include "image/File.h"

//#include "SlidingWindow.h"
#include "SlidingWindowOp.h"
//#include "QuadraticSmootherOp.h"
#include "DifferentialOp.h"
#include "BlenderOp.h"


namespace drain
{

namespace image
{

/// Consider using SlidingStripeOpticalFlow instead
/**
 *  T  = class of the input
 *  T2 = class of the derivative  images, weight, and output (motion field and weight)
 */

class OpticalFlowConfig : public WindowConfig {

public:


	OpticalFlowConfig() : invertU(false), invertV(true) {

	}

	/// If true, negate X so that it will increase towards left (West)
	bool invertU;

	/// If true, negate Y so that it will increase towards top (North)
	bool invertV;

};


class OpticalFlowCore : public WindowCoreBase {

public:

	typedef double data_t;
	typedef double cumul_t;

	/// Precomputed weight field (optional)
	ImageView srcWeight;

	inline
	void setSrcFrameWeight(const ImageFrame & srcW){
		Logger mout("WeightedOpticalFlowCore", __FUNCTION__);
		this->srcWeight.setView(srcW);
		//mout.debug() << "srcW: " << srcW << mout.endl;
		mout.debug() << "srcWeight (view): " << this->srcWeight << mout.endl;
	};

	// Destination images

	/// Horizontal velocity
	ImageView uField;

	/// Vertical velocity
	ImageView vField;

	/// Quality of the velocity
	ImageView dstWeight; // q


	/// Set outputs as channels (uField, vField, w)
	/**
	    Channels
	    - 0: uField
	    - 1: vField
	    - alpha: weight
	 */
	void setDstFrames(ImageTray<Channel> & dstTray);

	inline
	void setDstFrameWeight(const ImageFrame & dstW){
		//Logger mout("OpticalFlowCore2", __FUNCTION__);
		this->dstWeight.setView(dstW);
		//mout.debug() << "srcW: " << srcW << mout.endl;
		//mout.debug() << "view: " << this->srcWeight << mout.endl;
	};

	inline
	data_t nominator() const {
		return static_cast<data_t>(Gxx*Gyy - Gxy*Gxy);
	};

	/// Returns the horizontal component of motion. Must be scaled by nominator().
	inline
	data_t uDenominator() const {
		return static_cast<data_t>(Gxy*Gyt - Gyy*Gxt);
	};

	/// Returns the vertical component of motion. Must be scaled by nominator().
	inline
	data_t vDenominator() const {
		return static_cast<data_t>(Gxy*Gxt - Gxx*Gyt);
	};


protected:

	// sum w fx*fx
	cumul_t Gxx;
	// sum w fx*fy
	cumul_t Gxy;
	// sum w fx*fx
	cumul_t Gyy;
	// sum w fx*ft
	cumul_t Gxt;
	// sum w fy*ft
	cumul_t Gyt;

	// sum w ft*ft (needed only for prediction error, and quality index thereof)
	cumul_t Gtt;
	data_t W;

	/**
	 *
	 */
	// Consider: weight = > weightSUm
	//virtual
	void clearStats(){
		Gxx = 0.0;
		Gxy = 0.0;
		Gyy = 0.0;
		Gxt = 0.0;
		Gyt = 0.0;
		// Quality only:
		Gtt = 0.0;
		W = 0.0;

		//std::cerr << __FUNCTION__ << ": " << this->nominator() << ", " << this->uDenominator() << ", " << this->vDenominator() << std::endl;

	}

};

class OpticalFlowCore1 : public OpticalFlowCore {

public:

	static inline
	size_t getDiffChannelCount() {return 3;};

	/// Precomputed horizontal diffential
	ImageView Dx;
	/// Precomputed vertical diffential
	ImageView Dy;
	/// Precomputed time diffential
	ImageView Dt;

	/// Set inputs as channels 0:Dx, 1:Dy, 2:Dt, 3/alpha: weight
	/**
	    Channels
	    -# Dx
	    -# Dy
	    -# Dt
	    -# 3/alpha: weight
	*/
	void setSrcFrames(const ImageTray<const Channel> & srcTray);


protected:

	~OpticalFlowCore1(){};

};



template <class R = OpticalFlowCore1 >
class SlidingOpticalFlow : public SlidingWindow<OpticalFlowConfig, R> {

public:

	SlidingOpticalFlow(int width = 0, int height = 0) : SlidingWindow<OpticalFlowConfig, R>(width, height) {
		//setSize(width,height);
	}

	SlidingOpticalFlow(const OpticalFlowConfig & conf) : SlidingWindow<OpticalFlowConfig, R>(conf) {
		//setSize(conf.width, conf.height);
	}

	virtual
	~SlidingOpticalFlow(){};

	// "Inherit" data types.
	typedef OpticalFlowCore1::data_t   data_t;
	typedef OpticalFlowCore1::cumul_t cumul_t;

	// protected:
	//double areaD;

	 virtual
	 inline
	 void setImageLimits() const {
		 //this->adjustCoordHandler(this->coordinateHandler);
		 this->coordinateHandler.setPolicy(this->Dx.getCoordinatePolicy());
		 this->coordinateHandler.setLimits(this->Dx.getHeight(), this->Dy.getHeight());
	 }


	 virtual
	 void initialize();





	virtual	inline
	void addPixel(Point2D<int> & p){

		if (! this->coordinateHandler.validate(p))
			return;

		address = this->Dx.address(p.x, p.y);

		dx = this->Dx.template get<data_t>(this->address);
		dy = this->Dy.template get<data_t>(this->address);
		dt = this->Dt.template get<data_t>(this->address);
		// now "w = 1";

		// if ((location.x == 100) && (location.y == 100)) std::cout << locationHandled << '\n';
		/*
		if (this->debugDiag(4)){
			std::cout << this->location
					<< dx*dx << '\t'
					<< dx*dy << '\t'
					<< dy*dy << '\t'
					<< dx*dt << '\t'
					<< dy*dt << '\n';
		}
		 */


		this->Gxx += dx*dx;
		this->Gxy += dx*dy;
		this->Gyy += dy*dy;
		this->Gxt += dx*dt;
		this->Gyt += dy*dt;

		// Quality only
		this->Gtt -= dt*dt;
		this->W   += 1.0;
	}

	virtual inline
	void removePixel(Point2D<int> & p){

		if (! this->coordinateHandler.validate(p))
			return;

		address = this->Dx.address(p.x, p.y);

		dx = this->Dx.template get<data_t>(address);
		dy = this->Dy.template get<data_t>(address);
		dt = this->Dt.template get<data_t>(address);
		//now "w = 1";

		this->Gxx -= dx*dx;
		this->Gxy -= dx*dy;
		this->Gyy -= dy*dy;
		this->Gxt -= dx*dt;
		this->Gyt -= dy*dt;

		// Quality only
		this->Gtt -= dt*dt;
		this->W   -= 1.0;
	}


	/// Returns the mean squared error of predicted
	inline
	data_t predictionError(double u, double v) const {
		//return u*Dx.get<double>(address) + v*Dy.get<double>(address) - Dt.get<double>(address);
		if (this->W > 0.0)
			//return sqrt((Gtt - 2.0*(Gxt*u + Gyt*v)  +  Gxx*u*u + 2.0*Gxy*u*v + Gyy*v*v)/W);
			return sqrt((this->Gtt + 2.0*(this->Gxy*u*v - this->Gxt*u - this->Gyt*v)  +  this->Gxx*u*u +  this->Gyy*v*v)/this->W);
		else
			return 0;
	};

	virtual inline
	void write(){

		/*
		if (this->debugDiag(4)){

			dx = this->Dx.template get<double>(address);
			dy = this->Dy.template get<double>(address);
			dt = this->Dt.template get<double>(address);
			//w  = srcWeight.get<data_t>(address);

			std::cout << this->location << " \t"
					<< dx*dx << '\t'
					<< dx*dy << '\t'
					<< dy*dy << '\t'
					<< dx*dt << '\t'
					<< dy*dt << '\n';
		}
		*/

		nom = this->nominator();
		/*
		Logger mout("SlidingOpticalFlow", __FUNCTION__);
		mout.warn() << "start: " << location << mout.endl;
		mout.note() << "uField: " << uField << mout.endl;
		mout.note() << "vField: " << vField << mout.endl;
		mout.note() << "w: " << dstWeight << mout.endl;
		File::write(uField, "uField.png");
		File::write(vField, "vField.png");
		File::write(dstWeight, "w.png");
		mout.error() << "halt" << mout.endl;
		 */

		if (nom > 0.01){  // todo minQuality
		//if (w > 0.05){  // todo minQuality

			u = this->uDenominator()/nom;
			v = this->vDenominator()/nom;
			quality = sqrt(nom/this->W);
			//quality = this->srcWeight.template get<data_t>(this->location); // gradient stability
			//quality = predictionError(u, v); // TODO multiply with gradQuality
			//quality = this->srcWeight.getScaled(this->location.x, this->location.y);
			//quality = sqrt(nom/W) * this->srcWeight.getScaled(this->location.x, this->location.y); // predictionError(u, v); // TODO multiply with gradQuality
			//quality  = sqrt(nom/(u*u + v*v));
			//quality = sqrt(nom/(Gxx*Gxx + Gyy+Gyy));



			/*
			if (this->location.x == this->location.y)
				if ((this->location.x & 7) == 0){
					std::cout << this->location << ' ';
					std::cout << "\t v=(" << u << ',' << v << ")\t";
					//std::cout << "Diff:\t" << dx << '\t' << dy << '\t' << dt << "\tuvq:\t";
					//std::cout << uDenominator()/nom << '\t' << vDenominator()/nom << '\t' << quality << '\n';
					//std::cout << "\t w=(" << this->srcWeight.template get<data_t>(this->location) << ',' << (W/areaD)  << ")\t";
					//std::cout << "\t w=(" << w << ',' << W << ',' << this->srcWeight.template get<data_t>(this->location) << ")\t";
					std::cout << "\t w=(" << (W/areaD)  << ")\t";
					std::cout << quality << '\n';
				}
			 */

			this->uField.put(this->location, this->conf.invertU ? -u : u);
			this->vField.put(this->location, this->conf.invertV ? -v : v);
			//this->dstWeight.put(this->location, ( 1.0 - 1.0/(1.0+quality)) * this->dstWeight.getScaling().getScale());
			//this->dstWeight.putScaled(this->location.x, this->location.y, quality);
			this->dstWeight.put(this->location, quality);
			//this->dstWeight.put(this->location, this->dstWeight.getScaling().inv( 1.0 - 1.0/(1.0+quality)));
		}
		else {
			this->uField.put(this->location, 0);
			this->vField.put(this->location, 0);
			this->dstWeight.put(this->location, 0);
		}

	}

protected:

	virtual
	void clear(){
		std::cerr << "SlidingOpticalFlow::" << __FUNCTION__ << std::endl;
		this->clearStats();
	}

	// Used by addPixel, removePixel
	mutable size_t address;
	mutable	data_t dx, dy, dt, w; // inner-loop (instantaneous)

	// Used by write
	mutable data_t u, v;
	mutable data_t nom;
	mutable data_t quality;

};

template <class R>
void SlidingOpticalFlow<R>::initialize() {

	Logger mout(getImgLog(), "SlidingOpticalFlow", __FUNCTION__);

	//this->areaD = this->getArea();
	this->setImageLimits();
	this->setLoopLimits();

	mout.debug() << "window: "  << *this << mout.endl;
	mout.debug(2) << "Dx: " << this->Dx << mout.endl;
	mout.debug(2) << "Dy: " << this->Dy << mout.endl;
	mout.debug(2) << "Dt: " << this->Dt << mout.endl;

	//clear();
	//fill(0, 0);

}


/*
class SlidingOpticalFlow : public SlidingOpticalFlowBase<OpticalFlowCore1> {

public:

	SlidingOpticalFlow(const config & conf) : SlidingOpticalFlowBase<OpticalFlowCore1>(conf) {
		//setSize(conf.width, conf.height);
	}

};
*/

class SlidingOpticalFlowWeighted : public SlidingOpticalFlow<OpticalFlowCore1> {

public:

	SlidingOpticalFlowWeighted(const conf_t & conf) : SlidingOpticalFlow<OpticalFlowCore1>(conf) {
	}

	typedef SlidingOpticalFlow<OpticalFlowCore1> unweighted;

	virtual
	inline
	void addPixel(Point2D<int> & p){

		if (! coordinateHandler.validate(p))
			return;

		address = Dx.address(p.x, p.y);

		dx = Dx.get<data_t>(address);
		dy = Dy.get<data_t>(address);
		dt = Dt.get<data_t>(address);
		w  = srcWeight.get<data_t>(address);

		// if ((location.x == 100) && (location.y == 100)) std::cout << locationHandled << '\n';
		/*
		if ((location.x == 70) && (location.y == 120)){
			Logger mout("SlidingOpticalFlow", __FUNCTION__);
			mout.note() << this->location << '\t' << p << mout.endl;
		}
		*/

		Gxx += w*dx*dx;
		Gxy += w*dx*dy;
		Gyy += w*dy*dy;
		Gxt += w*dx*dt;
		Gyt += w*dy*dt;
		// Quality only:
		Gtt += w*dt*dt;
		W   += w;
	}

	virtual inline
	void removePixel(Point2D<int> & p){

		if (! coordinateHandler.validate(p))
			return;

		address = Dx.address(p.x, p.y);

		dx = Dx.get<data_t>(address);
		dy = Dy.get<data_t>(address);
		dt = Dt.get<data_t>(address);
		w  = srcWeight.get<data_t>(address);
		// w = 1;

		Gxx -= w*dx*dx;
		Gxy -= w*dx*dy;
		Gyy -= w*dy*dy;
		Gxt -= w*dx*dt;
		Gyt -= w*dy*dt;
		// Quality only:
		Gtt -= w*dt*dt;
		W   -= w;
	}

};

//--------------------------------------------------------------std::cout << locationHandled << '\n';----------------------------


/// Detects motion between two subsequent images. Does not extrapolate images.
/// Applies recursive correction for smoothing motion field.
/**
   Optical flow needs two input images, computes derivatives on them
   and produces an approximation of motion, ie. the flow.


\~exec
   # Dots to diamonds
   # rainage --geometry 256,256,1 --plot 128,128,100 --distanceTransform 256,256,0 -o diamond1.png
   # rainage --geometry 256,256,1 --plot 148,118,100 --distanceTransform 256,256,0 -o diamond2.png
   make diamond1.png diamond2.png
\~

\code
 # rainage diamond1.png diamond2.png --opticalFlow 25,25 --format '{i} {j2} {-0} {1} {A0}' --sample 10,10,file=oflow.dat
\endcode

\~exec
   # see examples on Op2 !
\~

*/
class FastOpticalFlowOp : public SlidingWindowOp<SlidingOpticalFlowWeighted> {

public:

	FastOpticalFlowOp(int width=5, int height=5): //, double smoothing=0.01) : //, double gradPow=2.0) : //, double gradWidth = 16) :
		SlidingWindowOp<SlidingOpticalFlowWeighted>(__FUNCTION__, "A pipeline implementation of optical flow."), blender(width, height, 'a', 'b', 1) {
		parameters.append(blender.getParameters(), false);
	}

	/// Creates a double precision image of 2+1 channels for storing motion (uField,vField) and quality (q).
	virtual
	inline
	void makeCompatible(const ImageFrame & src, Image & dst) const  {
		dst.initialize(typeid(OpticalFlowCore1::data_t), src.getWidth(), src.getHeight(), 2, 1);
	};

	/// Computes an image with channels dx, dy, dt and w (quality of gradients). User may redefine this.
	/**
	 *
	 */
	virtual
	void computeDifferentials(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;


	//virtual	void traverseChannels(const ImageTray<const Channel> & srcTray, ImageTray<Channel> & dstTray) const;

	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		this->traverseMultiChannel(src, dst);
	}

	virtual inline
	void traverseChannel(const Channel & src, Channel & dst) const {
		Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.error() << "Not implemented (1/1)" << mout.endl;
	}

	virtual inline
	size_t getDiffChannelCount() const {
		return window_t::getDiffChannelCount();
	}


	inline
	bool preSmooth() const {
		return !blender.getSmootherKey().empty();
	}

	mutable
	BlenderOp blender;

};

}
}

#endif

// Drain
