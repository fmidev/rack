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
#ifndef SLIDINGWINDOWOPTICALFLOWOP2_H_
#define SLIDINGWINDOWOPTICALFLOWOP2_H_

#include <sstream>
#include <math.h>

#include "drain/util/Fuzzy.h"

#include "drain/image/File.h"

#include "FastOpticalFlowOp.h" // OpticalFlowConfig, OpticalFlowCore1


namespace drain
{

namespace image
{




class OpticalFlowCore2 : public OpticalFlowCore {

public:

	// Sources
	static inline
	size_t getDiffChannelCount(){return 5;};

	ImageView GXX;
	ImageView GXY;
	ImageView GYY;

	ImageView GXT;
	ImageView GYT;


	/// Set inputs as channels
	/**
	    Channels
	    -# Gxx - sum of squared horz gradients
	    -# Gxy
	    -# Gyy - sum of squared vert gradients
	    -# Gxt
	    -# Gyt

	 */
	void setSrcFrames(const ImageTray<const Channel> & srcTray);


protected:

	~OpticalFlowCore2(){};

};


/// Currently not in use. See SlidingOpticalFlowWeighted2 below
/**
 *
 */
template <class R = OpticalFlowCore2 >
class SlidingOpticalFlow2 : public SlidingWindow<OpticalFlowConfig, R> {

public:

	SlidingOpticalFlow2(int width = 0, int height = 0) : SlidingWindow<OpticalFlowConfig, R>(width, height) {
		address = 0;
		w = u = v = nom = quality = 0.0;
	}

	SlidingOpticalFlow2(const OpticalFlowConfig & conf) : SlidingWindow<OpticalFlowConfig, R>(conf) {
		address = 0;
		w = u = v = nom = quality = 0.0;
	}

	virtual inline
	~SlidingOpticalFlow2(){};

	// "Inherit" data types.
	typedef OpticalFlowCore1::data_t   data_t;
	typedef OpticalFlowCore1::cumul_t cumul_t;

	virtual
	inline
	void setImageLimits() const {
		this->coordinateHandler.setPolicy(this->GXX.getCoordinatePolicy());
		this->coordinateHandler.setLimits(this->GXX.getWidth(), this->GXX.getHeight());
	}


	virtual
	void initialize();


	virtual inline
	void addPixel(Point2D<int> & p){

		Logger mout(getImgLog(), "SlidingOpticalFlow", __FUNCTION__);
		mout.error() << "unimplemented" << mout.endl;
		/*
		if (! this->coordinateHandler.validate(p))
			return;

		address = this->GXX.address(p.x, p.y);

		Gxx += this->GXX.template get<data_t>(this->address);
		Gxy += this->GXY.template get<data_t>(this->address);
		Gyy += this->GYY.template get<data_t>(this->address);
		Gxt += this->GXT.template get<data_t>(this->address);
		Gyt += this->GYT.template get<data_t>(this->address);

		// Quality only
		W   += this->srcW.template get<data_t>(this->address);
		 */
	}

	virtual inline
	void removePixel(Point2D<int> & p){

		Logger mout(getImgLog(), "SlidingOpticalFlow", __FUNCTION__);
		mout.error() << "unimplemented" << mout.endl;

		/*
		if (! this->coordinateHandler.validate(p))
			return;

		address = this->GXX.address(p.x, p.y);

		Gxx -= this->GXX.template get<data_t>(this->address);
		Gxy -= this->GXY.template get<data_t>(this->address);
		Gyy -= this->GYY.template get<data_t>(this->address);
		Gxt -= this->GXT.template get<data_t>(this->address);
		Gyt -= this->GYT.template get<data_t>(this->address);

		// Quality only
		W   -= this->srcW.template get<data_t>(this->address);
		 */
	}



	/// Returns the mean squared error of predicted
	/*
	inline
	data_t predictionError(double u, double v) const {
		//return u*Dx.get<double>(address) + v*Dy.get<double>(address) - Dt.get<double>(address);
		if (W > 0.0)
			//return sqrt((Gtt - 2.0*(Gxt*u + Gyt*v)  +  Gxx*u*u + 2.0*Gxy*u*v + Gyy*v*v)/W);
			return sqrt((Gtt + 2.0*(Gxy*u*v - Gxt*u - Gyt*v)  +  Gxx*u*u +  Gyy*v*v)/W);
		else
			return 0;
	};
	 */

	virtual inline
	void write(){


		nom = this->nominator();

		/*
		if (this->debugDiag(4)){
			// std::cerr << this->location << '\t' << this->nominator() << '\t' << this->uDenominator() << ',' << this->vDenominator() << '\n';
			// this->uField.put(this->location, rand());

			std::cerr << this->location << "  \t"
					<< this->GXX.template get<double>(this->location) << '\t'
					<< '\n';
			if (nom != 0.0){
				u = this->uDenominator()/nom;
				v = this->vDenominator()/nom;
				std::cerr << " => " << u << ',' << v << '\n';
			}
		}
		 */

		if (nom > 0.00000001){  // todo minQuality

			u = this->uDenominator()/nom;
			v = this->vDenominator()/nom;
			this->uField.put(this->location, this->conf.invertU ? -u : u);
			this->vField.put(this->location, this->conf.invertV ? -v : v);

			static const drain::FuzzySigmoid<double> sigmoid(0.0, 127.0, 254.0);
			quality = sqrt(nom/this->W);
			this->dstWeight.put(this->location, sigmoid(quality));
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
		this->clearStats();
		//std::cerr << "SlidingOpticalFlow2::" << __FUNCTION__ << ": " << this->nominator() << ", " << this->uDenominator() << ", " << this->vDenominator() << std::endl;
	}

	// Used by addPixel, removePixel
	mutable size_t address;
	//mutable	data_t dx, dy, dt, w; // inner-loop (instantaneous)
	mutable	data_t w; // inner-loop (instantaneous)

	// Used by write
	mutable data_t u, v;
	mutable data_t nom;
	mutable data_t quality;

};

template <class R>
void SlidingOpticalFlow2<R>::initialize() {

	Logger mout(getImgLog(), "SlidingOpticalFlow2", __FUNCTION__);

	this->setImageLimits();
	this->setLoopLimits();
	this->location.setLocation(0,0);

	mout.debug() << "window: "  << *this << mout.endl;
	mout.debug2();
	mout << "GXX: " << this->GXX << '\n';
	mout << "GXY: " << this->GXY << '\n';
	mout << "GXX: " << this->GYY << '\n';
	mout << "GXX: " << this->GXT << '\n';
	mout << "GXX: " << this->GYT << '\n';
	mout << "W:   " << this->srcWeight   << '\n';
	mout << mout.endl;
	//this->resetAtEdges = true;

	/*
	std::cout << "Annapa input\n";
	drain::Point2D<int> p;
	std::string s;
	while (getline(std::cin, s)){
	//while ((std::cin>>p.x) && (std::cin>>p.y)){
		std::stringstream sstr(s);
		sstr >> p.x >> p.y;
		std::cout << p << "\t =>";
		int result = this->coordinateHandler.validate(p);
		std::cout << p << "\t (" << result << ")\n" ;
	}
	*/

}



class SlidingOpticalFlowWeighted2 : public SlidingOpticalFlow2<OpticalFlowCore2> {

public:

	SlidingOpticalFlowWeighted2(const conf_t & conf) : SlidingOpticalFlow2<OpticalFlowCore2>(conf) {
	}

	typedef SlidingOpticalFlow2<OpticalFlowCore2> unweighted;


	virtual	inline
	void addPixel(Point2D<int> & p){

		if (! this->coordinateHandler.validate(p))
			return;

		address = this->GXX.address(p.x, p.y);
		/*
		if (p.y > GXX.getHeight()){
			std::cerr << " coord overflow at " << p << std::endl;
		}
		*/
		w = this->srcWeight.get<data_t>(address);

		W   += w;
		// w already included in cumulants
		Gxx += this->GXX.get<data_t>(address);
		Gxy += this->GXY.get<data_t>(address);
		Gyy += this->GYY.get<data_t>(address);
		Gxt += this->GXT.get<data_t>(address);
		Gyt += this->GYT.get<data_t>(address);

		// Quality only
	}

	virtual inline
	void removePixel(Point2D<int> & p){

		if (! this->coordinateHandler.validate(p))
			return;

		address = this->GXX.address(p.x, p.y);

		// Quality
		w = this->srcWeight.get<data_t>(address);

		W   -= w;
		// w already included in cumulants
		Gxx -= this->GXX.get<data_t>(address);
		Gxy -= this->GXY.get<data_t>(address);
		Gyy -= this->GYY.get<data_t>(address);
		Gxt -= this->GXT.get<data_t>(address);
		Gyt -= this->GYT.get<data_t>(address);


	}


};

//--------------------------------------------------------------std::cout << locationHandled << '\n';----------------------------


/// Detects motion between two subsequent images. Does not extrapolate images.
/// Applies recursive correction for smoothing motion field.
/**
   Optical flow needs two input images, computes derivatives on them
   and produces an approximation of motion, ie. the flow.


\~exec
   make diamond1.png diamond2.png
\~

\code
drainage diamond1.png diamond2.png --iOpticalFlow 25 --format '${i} ${j2} ${-0} ${1} ${A0}' --sample 10,10,file=oflow.dat
\endcode

\~exec
make oflow-clean
make oflow-result75.png oflow-result55.png oflow-result25.png oflow-result15.png  oflow-result05.png
make oflow-panel55.png
convert -frame 2 +append oflow-result??.png oflow-panel2.png
\~

 */
class FastOpticalFlow2Op : public SlidingWindowOp<SlidingOpticalFlowWeighted2> { // = window_t

public:

	typedef window_t::data_t data_t;

	FastOpticalFlow2Op(int width=5, int height=5): //, double smoothing=0.01) : //, double gradPow=2.0) : //, double gradWidth = 16) :
		SlidingWindowOp<SlidingOpticalFlowWeighted2>(__FUNCTION__, "Optical flow computed based on differential accumulation layers.")
		 {
		//parameters.append(blender.getParameters(), false);
		//blender.setParameter("mix", "b");
		// New. Blender will be obsolete
		parameters.link("resize", resize = 0, "0.0..1.0|pix");
		parameters.link("threshold", threshold = NAN, "value");
		parameters.link("spread", spread   = 0, "0|1");
		parameters.link("smooth", smoother = 0, "0|1");
		setSize(width, height);
	}

	inline
	FastOpticalFlow2Op(const FastOpticalFlow2Op & op): SlidingWindowOp<SlidingOpticalFlowWeighted2>(op) {
		//std::cerr << __FUNCTION__ << op.getParameters() << '\n';
		parameters.copyStruct(op.getParameters(), op, *this);
	}


	/// Creates a double precision image of 2+1 channels for storing motion (uField,vField) and quality (q).
	virtual inline
	//void makeCompatible(const ImageFrame & src, Image & dst) const  {
	void getDstConf(const ImageConf & src, ImageConf & dst) const  {
		//dst.initialize(typeid(OpticalFlowCore2::data_t), src.getWidth(), src.getHeight(), 2, 1);
		dst.setType(typeid(OpticalFlowCore2::data_t));
		dst.setArea(src);
		dst.setChannelCount(2, 1);
	};

	virtual inline
	size_t getDiffChannelCount() const {
		return window_t::getDiffChannelCount();
	}

	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		this->traverseMultiChannel(src, dst);
	}

	/// Computes an image with channels Gxx, Gxy, Gyy, Gxt, Gyt and w (quality of gradients). User may redefine this.
	/**
	 *
	 */
	virtual
	void computeDifferentials(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;



	virtual
	void preprocess(const Channel & srcImage, const Channel & srcWeight, Image & dstImage, Image & dstWeight) const;

	/// Returns true, if resizing, thresholding or smoothing is requested.
	inline
	bool optPreprocess() const {
		return optResize() || optThreshold() || optSmoother() || optSpread();
	}

	/// Returns true, if resizing is requested
	inline
	bool optResize() const { return resize > 0.0; } ;

protected:

	// NEW
	//std::vector<double> resize;
	double resize;
	double threshold;
	bool spread;
	bool smoother;



	/// Returns true, if threshold is requested as postprocessing
	inline
	bool optThreshold() const { return !std::isnan(threshold); };

	inline
	bool optSpread() const { return spread; };

	inline
	bool optSmoother() const { return smoother; };



	inline
	bool checkQuality(const Channel & alpha, std::size_t address, int DX, int DY) const {

		if (alpha.get<data_t>(address) == 0.0)
			return false;

		if (alpha.get<data_t>(address + DX) == 0.0)
			return false;

		if (alpha.get<data_t>(address - DX) == 0.0)
			return false;

		if (alpha.get<data_t>(address + DY) == 0.0)
			return false;

		if (alpha.get<data_t>(address - DY) == 0.0)
			return false;

		return true;
	}

};

}
}

#endif

// Drain
