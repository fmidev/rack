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
#ifndef WINDOW_H_
#define WINDOW_H_

#include "util/Functor.h"
#include "ImageView.h"
#include "ImageTray.h"
//#include "ImageOp.h"


namespace drain
{

namespace image
{

/// Base class for configurations applied in image processing windows, e.g. for operators of type WindowOp<WindowConfig> .
/**
 *   This base base class has only two members: width and height.
 *   Derived classes may have further parameters like smoothing coefficients.
 *
 *   A class derived from WindowConfig should \e not contain information of source or destination data, but of the "window itself".
 */
class WindowConfig {  // TODO: public BeanLike

public:

	inline
	WindowConfig(int width=1, int height=1) : width(width), height(height), ftor(idFtor) {
	};

	inline
	WindowConfig(UnaryFunctor & functor, int width=1, int height=1) : width(width), height(height), ftor(functor) {
	};

	/// Width of the window, in pixels.
    int width;

	/// Height of the window, in pixels.
    int height;

    inline
    int getWidth() const { return width; };

    inline
    int getHeight() const { return height>0 ? height : width; };

    // Could/should be const?
    UnaryFunctor & ftor;

protected:

    // Could/should be const?
    static IdentityFunctor idFtor;


};

/// Provides splitting frames
/**
 *
 */
class WindowCoreBase {

public:

	virtual inline
	~WindowCoreBase(){};

	virtual inline
	void setSrcFrame(const ImageFrame & src){
		ImageTray<const Channel> srcTray; //(src);
		srcTray.setChannels(src);
		setSrcFrames(srcTray);
	}

	virtual
	void setSrcFrames(const ImageTray<const Channel> & srcTray) = 0;



	virtual inline
	void setDstFrame(ImageFrame & dst){
		ImageTray<Channel> dstTray; //(dst);
		dstTray.setChannels(dst);
		setDstFrames(dstTray);
	}

	virtual
	void setDstFrames(ImageTray<Channel> & dstTray) = 0;


};


/// Container for source data (input) and destination data (output). May also contain metadata of them.
/**
 *
 */
class WindowCore : public WindowCoreBase { // rename BasicWindow

public:

	ImageView src;
	ImageView dst;


	///
	virtual inline
	void setSrcFrames(const ImageTray<const Channel> & srcTray){

		Logger mout("WindowCore", __FUNCTION__);

		if (srcTray.empty()){
			mout.error() << "src: no channels" << mout.endl;
			return;
		}

		if (srcTray.size() > 1){
			mout.warn()  << "src: multiple channels" << mout.endl;
		}

		//mout.warn()  << "scale:" << srcTray.get(0).getScaling() << mout.endl;
		//mout.note()  << "scale:" << srcTray.get(0).getChannel(0).getScaling() << mout.endl;
		src.setView(srcTray.get(0));
		//mout.warn()  << "scale:" << src.getScaling() << mout.endl;

		if (!srcTray.alpha.empty()){
			setSrcFrameWeight(srcTray.getAlpha(0));
		}

	};


	///
	virtual inline
	void setDstFrames(ImageTray<Channel> & dstTray){

		Logger mout("GaussianStripe", __FUNCTION__);

		if (dstTray.empty()){
			mout.error() << "dst: no channels" << mout.endl;
			return;
		}

		if (dstTray.size() > 1){
			mout.warn()  << "dst: multiple channels" << mout.endl;
		}

		dst.setView(dstTray.get(0));

		if (!dstTray.alpha.empty()){
			setDstFrameWeight(dstTray.getAlpha(0));
		}

	};

	// Optional
	virtual inline
	void setSrcFrameWeight(const ImageFrame & srcW){
		Logger mout("WindowCore", __FUNCTION__);
		mout.warn() << "Not implemented" << mout.endl;
	};

	virtual inline
	void setDstFrameWeight(ImageFrame & dstW){
		Logger mout("WindowCore", __FUNCTION__);
		mout.warn() << "Not implemented" << mout.endl;
	};

	/*
	virtual
	void adjustCoordHandler(CoordinateHandler2D & handler) const {
		handler.setPolicy(this->src.getCoordinatePolicy());
		handler.setLimits(this->src.getWidth(), this->src.getHeight());
	}
	*/
};

class WeightedWindowCore : public WindowCore {

public:

	WeightedWindowCore(){};

	virtual
	~WeightedWindowCore(){};


	virtual inline
	void setSrcFrameWeight(const ImageFrame & srcW){
		srcWeight.setView(srcW);
	};

	virtual inline
	void setDstFrameWeight(ImageFrame & dstW){
		dstWeight.setView(dstW);
	};

	// toOStr() ?

// protected:

	ImageView srcWeight;

	ImageView dstWeight;


};


class MultiChannelWindowCore : public drain::image::WeightedWindowCore {

public:

	MultiChannelWindowCore(){};

	virtual
	~MultiChannelWindowCore(){};

	ImageTray<const Channel> srcTray;
	ImageTray<Channel> dstTray;

	virtual inline
	void setSrcFrame(const Channel & srcChannel){
		ImageTray<const Channel> srcChannels;
		srcChannels.set(srcChannel);
		setSrcFrames(srcChannels);
	}

	virtual inline
	void setDstFrame(Channel & dstChannel){
		ImageTray<Channel> dstChannels;
		dstChannels.set(dstChannel);
		setDstFrames(dstChannels);
	}

	///	virtual inline
	void setSrcFrames(const ImageTray<const Channel> & srcTray){

		drain::Logger mout("MultiChannelWindowCore", __FUNCTION__);

		mout.debug()  << "setting srcTray" << mout.endl;

		//this->srcTray.clear();
		this->srcTray.copy(srcTray);
		if (!srcTray.empty()){
			this->src.setView(srcTray.get());
		}
		else {
			mout.warn()  << "setting empty srcTray" << mout.endl;
		}


		if (!srcTray.alpha.empty()){
			this->srcWeight.setView(srcTray.alpha.get());
		}
		else {
			mout.debug()  << "no srcTray.alpha" << mout.endl;
		}

		mout.debug(1) << this->srcTray << mout.endl;

	};

	///	virtual inline
	void setDstFrames(ImageTray<Channel> & dstTray){

		drain::Logger mout("MultiChannelWindowCore", __FUNCTION__);

		mout.debug()  << "setting dstTray" << mout.endl;

		//this->dstTray.clear();
		this->dstTray.copy(dstTray);
		if (!dstTray.empty()){
			this->dst.setView(this->dstTray.get());
		}
		else {
			mout.warn()  << "setting empty dstTray" << mout.endl;
		}


		if (!dstTray.alpha.empty()){
			this->dstWeight.setView(this->dstTray.alpha.get());
		}
		else {
			mout.debug()  << "no srcTray.alpha" << mout.endl;
			// mout.warn()  << "setting empty srcTray" << mout.endl;
		}

		mout.debug(1) << this->dstTray << mout.endl;

		/*
		if (dstTray.size() != 2){
			mout.note() << dstTray << mout.endl;
			mout.error()  << "dst should have exactly 2 image channels" << mout.endl;
		}
		*/

	};



};



/// Base class for windows applied by WIndowOp's
/**
 *  \tparam C - configuration (width, height and str metadata)
 *  \tparam R - source and target images, and their setters.
 */
template <class C = WindowConfig, class R = WindowCore>
class Window : public R // consider : public BeanLike ?
{
public:

	typedef C conf_t;

	conf_t conf;

	/// Constructor with geometry setting option.
	Window(size_t width=1, size_t height=0) : resetAtEdges(false), iMin(0), iMax(0), jMin(0), jMax(0){
  		setSize(width, height==0 ? width : height);
		location.setLocation(0, 0);
	};

	/// Constructor adapting given configuration.
	Window(const C & conf) : conf(conf), resetAtEdges(false), iMin(0), iMax(0), jMin(0), jMax(0){
		setSize(conf.width, conf.height);
		location.setLocation(0, 0);
	}

	// Future option...
	/*
	Window(const R & core, size_t width=1, size_t height=0) : R(core), resetAtEdges(false){
	  		setSize(width, height==0 ? width : height);
			location.setLocation(0, 0);
	};
	*/

	/// Destructor
	virtual
	~Window(){};

	/// Sets the window size
	//  Not final, because some derived classes like SlidingStripe's need to redefine this
	virtual inline
	void setSize(size_t width, size_t height){
		conf.width = width;
		conf.height = height;
	}


	/// Returns the nominal area in pixels
	inline
    size_t getArea(){ return conf.width * conf.height;};

	/// Returns the area which has eventually been scaled (in a non-linear coordinate system)
	inline
	size_t getSamplingArea(){ return samplingArea;};



	virtual
	void toStream(std::ostream & ostr) const;

    /// Main loop: traverses the source image and writes result to dst image.
    virtual
    void run();



protected:

	/// To avoid accumulated numerical errors esp. with floats, reset the statistics at row/cols ends. See reset() .
	bool resetAtEdges;

	/// If set, scaling is applied, potentially slowering the computation.
    bool SCALE;

    /// In current location, this is called to calculate and store something in members.
    virtual
    void update(){};

    /// Write the results of update() to dstImage(s).
    virtual
    void write() = 0; // {};

    /// Current location of this window.
	Point2D<int> location;

	int samplingArea;

	/// Tells if the window should be moved (traversed of slided) row-by-row (horizontally) or column-by-column (vertically).
	/**
	 *   One may think of a boat. Boats typically move more easily forward or backward (ie. "horizontally") than sideways ("vertically").
	 */
	virtual inline
	bool isHorizontal() const { return (this->conf.width > this->conf.height); };


	// For 1) cleaning numerical residues and 2) updating window parameters
	/**
	 *  \return - true if scanning should be ended (if window dimensions have been changed?)
	 */
	virtual inline
	bool reset(){
		//fill();
		return true;
	};

	/**
	 * This function must
	 * -# set image limits (limits of the total scanning area)
	 * -# set (initial) loop limits (limits of the window scanning area. typically window origin in the center)
	 * -# set starting location (typically 0,0)
	 *
	 */
    virtual inline
    void initialize(){
    	//setScaling();
    	setImageLimits();
    	setLoopLimits();
    	this->location.setLocation(0,0);
    };

    /// Studies source and destination images and decides whether scaling (SCALE=true) should be set
    /*
    virtual
    void se t S caling() const {
    	// SCALE = src.scaling.isScaled() || dst.scaling.isScaled();
    };
    */

    /// Sets internal limits corresponding to image geometries. Typically using coordHandler.
    /**
     *   Studies source image(s) and sets srcWidth, srcHeight and coordHandler.
     *   Consider already implemented methods like WindowCore::adjustCoordHandler()
     */
    virtual
    void setImageLimits() const = 0;


    mutable int iMin;
    mutable int iMax;
    mutable int jMin;
    mutable int jMax;

    /// Sets the actual traversal range inside the window. Sometimes applied dynamically by reset().
	virtual inline
	void setLoopLimits(int width, int height){
		samplingArea = width * height;
		iMin = -(static_cast<int>(width)-1)/2;   // note unsigned risk!
		iMax = width/2;
		jMin = -(static_cast<int>(height)-1)/2;
		jMax = height/2;
	}

	/// Sets the actual traversal range inside the window. Sometimes applied dynamically by reset().
	inline
	void setLoopLimits(){
		setLoopLimits(conf.width, conf.height);
	}


	// This is like a global member, useful for update(int) functions.
	mutable CoordinateHandler2D coordinateHandler;

	// Debugging utility. Returns true on every (2^bit)'th diagonal location (x==y).
	inline
	bool debugDiag(int bit = 4){
		return (this->location.x == this->location.y) && ((this->location.x&((1<<bit)-1)) == 0);
	};
};



template <class C, class R>
void Window<C,R>::toStream(std::ostream &ostr) const {
	ostr << "Window: " << conf.width << 'x' <<  conf.height << ' ';
	ostr << '[' << iMin << ',' << jMin << ',' << iMax << ',' << jMax << ']';
	//ostr << " in (" << srcWidth << 'x' <<  srcHeight << ") ";
	ostr << '@' << location << '\n';
	ostr << coordinateHandler << '\n';
	//ostr << "src: " << src << " scaling:" << scaleResult << '\n';
	/*
	ostr << "src: " << src << " scaling:" << (int)SCALE << '\n';
	ostr << "dst: " << dst << '\n';
	ostr << "srcW:" << srcWeight << '\n';
	ostr << "dstW:" << dstWeight << '\n';
	*/
}


template <class P, class R>
void Window<P,R>::run(){

	initialize();

	int &i = location.x;
	int &j = location.y;

	const int iMax = coordinateHandler.getXMax();
	const int jMax = coordinateHandler.getYMax();

	if (isHorizontal()){
		for (j = 0; j <= jMax; ++j) {
			for (i = 0; i <= iMax; ++i) {
				update();
				write();
			}
			if (resetAtEdges){
				if (!reset())
					return; // ?
			}
		}
	}
	else {
		for (i = 0; i <= iMax; ++i) {
			for (j = 0; j <= jMax; ++j) {
				update();
				write();
			}
			if (resetAtEdges){
				if (!reset())
					return;// ?
			}
		}
	}

}





template <class P, class R>
inline
std::ostream & operator<<(std::ostream &ostr, const Window<P,R> &w){
	w.toStream(ostr);
	return ostr;
}



}

}

#endif /*WINDOWOP_H_*/

// Drain
