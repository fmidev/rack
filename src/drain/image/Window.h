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
#ifndef DRAIN_WINDOW_H_
#define DRAIN_WINDOW_H_

#include "drain/util/Frame.h"
#include "drain/util/Functor.h"
#include "drain/util/FunctorBank.h"
#include "drain/util/Static.h"
#include "ImageView.h"
#include "ImageTray.h"
#include "CoordinateHandler.h"


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
class WindowConfig { //: public BeanLike { // {  // TODO:

public:

	//IdentityFunctor f;

	/*
	inline
	WindowConfig(int width=1, int height=0) : frame(width, height ? height : width){
		ftorMapEntry = getFunctorBank().get<IdentityFunctor>(); //  getMap().begin();
		//ftor(idFtor) {
		// parameters.link("width", frame.width, "pix");
		// parameters.link("height", frame.height, "pix");
	};
	*/

	template <class FT=IdentityFunctor>
	inline
	WindowConfig(int width=1, int height=0, const FT & functor = FT()) : frame(width,height ? height : width){
		// Cloner<UnaryFunctor,FT> & cloner = Static::get<Cloner<UnaryFunctor,FT>,FunctorBank::bank_id>();
		// ftorMapEntry.first = functor.getName(); // cloner.getSource().getName();  // === functor.getName();
		//clonerBase = & Static::get<Cloner<UnaryFunctor,FT>,FunctorBank::bank_id>();
		key = functor.getName();
		functorParameters.importCastableMap(functor.getParameters());
	};

	inline
	WindowConfig(const WindowConfig & conf) :
		// BeanLike(conf),
		frame(conf.frame),
		key(conf.key)
		//clonerBase(conf.clonerBase)
	{
		//ftorMapEntry = getFunctorBank().find2<IdentityFunctor>(); //  .getMap().begin();
		functorParameters.importCastableMap(conf.functorParameters); // maybe different from cloned ones!
		//parameters.copyStruct(conf.parameters, conf, *this);
	};


	// Width and height of the window, in pixels.
	Frame2D<int> frame;

    inline
    int getWidth() const {
    	return frame.width;
    };

    inline
    int getHeight() const {
    	return frame.height;
    };

    typedef typename FunctorBank::map_t fmap_t;
    typedef typename fmap_t::const_iterator ftor_entry_t;

    VariableMap functorParameters;

    /// Get the cloner of the current functor type.

    /// Changes the current functor specification.
    /**
     *  The functor name (key) is checked in the functorBank.
     *   No actual instance is created by this function.
     */
    inline
	void setFunctor(const std::string & ftorKey){

    	FunctorBank & functorBank = getFunctorBank();
    	const std::string k = functorBank.resolve(ftorKey); // empty ok, idFtor!

    	const fmap_t & m = functorBank.getMap();
    	ftor_entry_t it = m.find(k);
    	if (it != m.end()){
    		key = it->first;
    	}
    	else {
    		throw std::runtime_error(ftorKey + '[' + k + "]: no such entry, using IdentityFunctor");
    	}
    };


    // Needed?
    inline
	const fmap_t::key_type & getFunctorName() const {
    	return key;
    };



    /// Return the parameters to be set for proceeding getFunctor() calls.
    inline
	const VariableMap & getFunctorParams() const{
        return functorParameters;
    };


protected:

    std::string key;


};

std::ostream & operator<<(std::ostream & ostr, const WindowConfig & conf);





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

		Logger mout(getImgLog(), "WindowCore", __FUNCTION__);

		if (srcTray.empty()){
			mout.error("src: no channels" );
			return;
		}

		if (srcTray.size() > 1){
			mout.warn("src: multiple channels" );
		}

		//mout.warn("scale:" , srcTray.get(0).getScaling() );
		//mout.note("scale:" , srcTray.get(0).getChannel(0).getScaling() );
		src.setView(srcTray.get(0));
		//mout.warn("scale:" , src.getScaling() );

		if (!srcTray.alpha.empty()){
			setSrcFrameWeight(srcTray.getAlpha(0));
		}

	};


	///
	virtual inline
	void setDstFrames(ImageTray<Channel> & dstTray){

		Logger mout(getImgLog(), "GaussianStripe", __FUNCTION__);

		if (dstTray.empty()){
			mout.error("dst: no channels" );
			return;
		}

		if (dstTray.size() > 1){
			mout.warn("dst: multiple channels" );
		}

		dst.setView(dstTray.get(0));

		if (!dstTray.alpha.empty()){
			setDstFrameWeight(dstTray.getAlpha(0));
		}

	};

	// Optional
	virtual inline
	void setSrcFrameWeight(const ImageFrame & srcW){
		Logger mout(getImgLog(), "WindowCore", __FUNCTION__);
		mout.warn("Not implemented" );
	};

	virtual inline
	void setDstFrameWeight(ImageFrame & dstW){
		Logger mout(getImgLog(), "WindowCore", __FUNCTION__);
		mout.warn("Not implemented" );
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
	void setDstFrame(Channel & dstChannel){  // ALERT! Unintentional Shadow (arg: ImageFrame -> Channel)
		ImageTray<Channel> dstChannels;
		dstChannels.set(dstChannel);
		setDstFrames(dstChannels);
	}

	///	virtual inline
	void setSrcFrames(const ImageTray<const Channel> & srcTray){

		drain::Logger mout(getImgLog(), "MultiChannelWindowCore", __FUNCTION__);

		mout.debug("setting srcTray" );

		//this->srcTray.clear();
		this->srcTray.copy(srcTray);
		if (!srcTray.empty()){
			this->src.setView(srcTray.get());
		}
		else {
			mout.warn("setting empty srcTray" );
		}


		if (!srcTray.alpha.empty()){
			this->srcWeight.setView(srcTray.alpha.get());
		}
		else {
			mout.debug("no srcTray.alpha" );
		}

		mout.debug2(this->srcTray );

	};

	///	virtual inline
	void setDstFrames(ImageTray<Channel> & dstTray){

		drain::Logger mout(getImgLog(), "MultiChannelWindowCore", __FUNCTION__);

		mout.debug("setting dstTray" );

		//this->dstTray.clear();
		this->dstTray.copy(dstTray);
		if (!dstTray.empty()){
			this->dst.setView(this->dstTray.get());
		}
		else {
			mout.warn("setting empty dstTray" );
		}


		if (!dstTray.alpha.empty()){
			this->dstWeight.setView(this->dstTray.alpha.get());
		}
		else {
			mout.debug("no srcTray.alpha" );
			// mout.warn("setting empty srcTray" );
		}

		mout.debug2(this->dstTray );

		/*
		if (dstTray.size() != 2){
			mout.note(dstTray );
			mout.error("dst should have exactly 2 image channels" );
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
class Window : public R { // consider : public BeanLike ?

public:

	typedef C conf_t;

	conf_t conf;

	UniCloner<UnaryFunctor> unicloner;

	// Final function, set upon instantiation
	drain::UnaryFunctor & myFunctor;

	/// Constructor with geometry setting option.
	Window(size_t width=1, size_t height=0) :
		//conf(width, height),
		unicloner(getFunctorBank()),
		myFunctor(unicloner.getCloned(conf.getFunctorName())), //
		//myFunctor(this->conf.getFunctor()),
		resetAtEdges(false),
		SCALE(true),
		iRange(0,0),
		jRange(0,0)
	{
  		setSize(width, height==0 ? width : height);
  		myFunctor.setParameters(conf.functorParameters);
		location.setLocation(0, 0);
	};

	/// Constructor adapting given configuration.
	Window(const C & conf) :
		conf(conf),
		unicloner(getFunctorBank()),
		myFunctor(unicloner.getCloned(conf.getFunctorName())), //
		// myFunctor(this->conf.getFunctor(conf.getFunctorName())), //
		resetAtEdges(false),
		SCALE(true),
		iRange(0,0),
		jRange(0,0)
	{
		setSize(conf.frame.width, conf.frame.height);
  		myFunctor.setParameters(conf.functorParameters);
		location.setLocation(0, 0);
	}

	Window(const Window & window) :
		conf(window.conf),
		myFunctor(this->conf.getFunctor(window.conf.getFunctorName())),
		resetAtEdges(window.resetAtEdges),
		SCALE(window.SCALE),
		iRange(window.iRange),
		jRange(window.jRange)
	{
		setSize(conf.frame.width, conf.frame.height);
		myFunctor.setParameters(window.conf.functorParameters);
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
	~Window(){
		//  << this->conf.getFunctorCloner().size() <<
		//std::cerr << "Erasing: myFunctor " << std::endl;
	};

	/// Sets the window size
	//  Not final, because some derived classes like SlidingStripe's need to redefine this
	virtual inline
	void setSize(size_t width, size_t height){
		conf.frame.set(width, height);
	}


	/// Returns the nominal area in pixels
	inline
    size_t getArea(){
		return conf.frame.getArea();
	};

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
	bool resetAtEdges = false;  // NEW

	/// If set, scaling is applied, potentially slowering the computation.
    bool SCALE = true; // NEW

    /// In current location, this is called to calculate and store something in members.
    virtual
    void update(){};

    /// Write the results of update() to dstImage(s).
    virtual
    void write() = 0; // {};

    /// Current location of this window.
	Point2D<int> location;

	/// Number of pixels in the window (frame width*height?).
	int samplingArea = 0;

	/// Tells if the window should be moved (traversed of slided) row-by-row (horizontally) or column-by-column (vertically).
	/**
	 *   One may think of a boat. Boats typically move more easily forward or backward (ie. "horizontally") than sideways ("vertically").
	 */
	virtual inline
	bool isHorizontal() const { return (this->conf.frame.width > this->conf.frame.height); };


	/// Function determing whether array should be cleared at the edge(s). Needed for 1) cleaning numerical residues and 2) updating window parameters
	/**
	 *  \return - true if scanning should be ended (if window dimensions have been changed?)
	 */
	virtual inline
	bool reset(){
		return true;
	};

	/**
	 * This function \i must
	 * -# set image limits (limits of the total scanning area)
	 * -# set (initial) loop limits (limits of the window scanning area. Typically window origin is in the center.
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

    // Window limits, not image limits
    mutable Range<int> iRange;
    mutable Range<int> jRange;


    /// Sets the actual traversal range inside the window. Sometimes applied dynamically by reset().
	virtual inline
	void setLoopLimits(int width, int height){
		samplingArea = width * height;
		iRange.min = -(static_cast<int>(width)-1)/2;   // note unsigned risk!
		iRange.max = width/2;
		jRange.min = -(static_cast<int>(height)-1)/2;
		jRange.max = height/2;
	}

	/// Sets the actual traversal range inside the window. Sometimes applied dynamically by reset().
	inline
	void setLoopLimits(){
		setLoopLimits(conf.frame.width, conf.frame.height);
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
	ostr << "Window: " << conf.frame << ' ';
	ostr << '[' << iRange << '|' << jRange << ']';
	//ostr << " in (" << srcWidth << 'x' <<  srcHeight << ") ";
	ostr << '@' << location << ',' << coordinateHandler << ' ';
	ostr << "functor: " << this->myFunctor << '\n';
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

	const Range<int> & horzRange = coordinateHandler.getXRange();
	const Range<int> & vertRange = coordinateHandler.getYRange();


	if (isHorizontal()){
		for (j = 0; j <= vertRange.max; ++j) {
			for (i = 0; i <= horzRange.max; ++i) {
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
		for (i = 0; i <= horzRange.max; ++i) {
			for (j = 0; j <= vertRange.max; ++j) {
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
std::ostream & operator<<(std::ostream &ostr, const drain::image::Window<P,R> &w){
	w.toStream(ostr);
	return ostr;
}


} // image

template <class P, class R>
struct TypeName<image::Window<P,R> > {

    static const std::string & str(){
		static const std::string name = std::string("Window<>"); // todo: develop
        return name;
    }

};

} // drain



#endif /* Drain_WINDOWOP_H_*/

