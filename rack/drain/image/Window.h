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
#include "ImageOp.h"


namespace drain
{

namespace image
{


struct WindowConfig {



	inline
	WindowConfig(int width=1, int height=1) : width(width), height(height), ftor(idFtor) {
	};

	inline
	WindowConfig(UnaryFunctor & functor, int width=1, int height=1) : width(width), height(height), ftor(functor) {
	};


    int width;
    int height;

    /// Critical proportion of window are applied by some derived window operators, like median.
    //float percentage;

    UnaryFunctor & ftor;

protected:
    static IdentityFunctor idFtor;


};


template <class P = WindowConfig>
class Window // consider : public BeanLike ?
{
public:

	typedef P config;

	config conf;

	/// Constructor with geometry setting option.
	Window(size_t width=1, size_t height=0) : resetAtEdges(false){
  		setSize(width, height==0 ? width : height);
		location.setLocation(0, 0);
	};

	Window(const P & conf) : conf(conf), resetAtEdges(false){
		setSize(conf.width, conf.height);
		location.setLocation(0, 0);
	}


	virtual
	~Window(){};

	/// Sets the size of the window; setSize(width) equals setSize(width,width);
	virtual
	inline
	void setSize(size_t width,size_t height = 0){
		if (height == 0)
			height = width;
		conf.width = width;
		conf.height = height;
		setLoopLimits(width, height);
		/*
		iMin = -(static_cast<int>(conf.width)-1)/2;   // note unsigned risk!
		iMax = conf.width/2;
		jMin = -(static_cast<int>(conf.height)-1)/2;
		jMax = conf.height/2;
		*/
	}

	/// Returns the nominal area
	inline
    size_t getArea(){ return conf.width * conf.height;};

	/// Returns the area which has eventually been scaled (in a non-linear coordinate system)
	inline
	size_t getSamplingArea(){ return samplingArea;};

	void setSrc(const Image & img);

    void setDst(Image & img);

    // Optional
    void setSrcWeight(const Image & img);
    void setDstWeight(Image & img);

    virtual
    void initialize(){};
    //void initialize(const Point2D<int> & loc = Point2D<int>(0,0)){};

    /// protect?
    virtual
    void traverse();

    /// Calculate something and store it (in mutable members).
    virtual
    void update(){};

    /// Write the results of update() to dstImage(s).
    virtual
    void write(){};

	Point2D<int> location;

	virtual
	void toStream(std::ostream & ostr) const;


	/// To avoid accumulated numerical errors esp. with floats, reset the statistics at row/cols ends. See reset() .
	bool resetAtEdges;


protected:

	int samplingArea;

	/// Tells if the window should be moved (traversed of slided) row-by-row (horizontally) or column-by-column (vertically).
	/**
	 *   One may think of a boat. Boats typically move more easily forward or backward (ie. "horizontally") than sideways ("vertically").
	 */
	virtual inline
	bool isHorizontal(){ return (this->conf.width > this->conf.height); };


	// For 1) cleaning numerical residues and 2) updating window parameters
	virtual
	inline
	bool reset(){
		//fill();
		return true;
	};

    mutable int iMin;
    mutable int iMax;
    mutable int jMin;
    mutable int jMax;

    /// Sets the actual traversal range inside the window. Sometimes applied dynamically by reset().
	virtual
	inline
	void setLoopLimits(size_t w, size_t h){
		samplingArea = w*h;
		iMin = -(static_cast<int>(w)-1)/2;   // note unsigned risk!
		iMax = w/2;
		jMin = -(static_cast<int>(h)-1)/2;
		jMax = h/2;
	}


    int srcWidth;
    int srcHeight;
    float scaleResult;  // TODO: functor

	Image src;
	Image dst;
	Image srcWeight;
	Image dstWeight;

	// This is like a global member, useful for update(int) functions.
	mutable CoordinateHandler2D coordinateHandler;
};



template <class P>
void Window<P>::setSrc(const Image & src){
	this->src.setView(src);
	srcWidth = src.getWidth();
	srcHeight = src.getHeight();
	coordinateHandler.setLimits(srcWidth, srcHeight);
	if (coordinateHandler.getPolicy().xOverFlowPolicy != CoordinatePolicy::UNDEFINED)
		coordinateHandler.setPolicy(src.getCoordinatePolicy());
	scaleResult = src.getMax<float>()/dst.getMax<float>(); // TODO change order?

}

template <class P>
void Window<P>::setDst(Image &image){
	dst.setView(image);
	scaleResult = src.getMax<float>()/dst.getMax<float>();
}

// FIXME  VIRHE TÄSSÄ?
template <class P>
void Window<P>::setSrcWeight(const Image &image){
	srcWeight.setView(image);
}

template <class P>
void Window<P>::setDstWeight(Image & image){
	dstWeight.setView(image);
}

template <class P>
void Window<P>::toStream(std::ostream &ostr) const {
	ostr << "Window: " << conf.width << 'x' <<  conf.height << ' ';
	ostr << '[' << iMin << ',' << jMin << ',' << iMax << ',' << jMax << ']';
	//ostr << " within " << srcWidth << 'x' <<  srcHeight << '\n';
	ostr << '@' << location << '\n';
	ostr << coordinateHandler << '\n';
	ostr << "src: " << src << '\n';
	ostr << "dst: " << dst << '\n';
	ostr << "srcW:" << srcWeight << '\n';
	ostr << "dstW:" << dstWeight << '\n';
}


template <class P>
void Window<P>::traverse(){

	initialize();

	int &i = location.x;
	int &j = location.y;

	if (isHorizontal()){
		for (j = 0; j < srcHeight; ++j) {
			for (i = 0; i < srcWidth; ++i) {
				update();
				write();
			}
			if (resetAtEdges){
				if (!reset())
					return;
			}
		}
	}
	else {
		for (i = 0; i < srcWidth; ++i) {
			for (j = 0; j < srcHeight; ++j) {
				update();
				write();
			}
			if (resetAtEdges){
				if (!reset())
					return;
			}
		}
	}

}





template <class P>
inline
std::ostream & operator<<(std::ostream &ostr, const Window<P> &w){
	w.toStream(ostr);
	return ostr;
}



}

}

#endif /*WINDOWOP_H_*/

// Drain
