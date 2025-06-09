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
#ifndef SLIDINGWINDOWOP13_H_
#define SLIDINGWINDOWOP13_H_

#include "drain/image/SlidingWindow.h"

#include "CopyOp.h"
#include "WindowOp.h"

namespace drain
{

namespace image
{



/// Template for operators applying pipeline-like sliding window.
/**
 *  \tparam W - sliding window class with W::conf_t structure, potentially suporting user modifiable parameters.
 *
 *  Class W must have:
 *  - W::conf_t - conf structure
 *  - W::unweighted - typedef for unweighted version of the operator.
 *  - W::slide() - method implementing the sliding.
 *
 *  \see SlidingWindow
 */
template <class W>
class SlidingWindowOp : public WindowOp<W> {

public:

	typedef W window_t;

	SlidingWindowOp(const typename W::conf_t & conf) : WindowOp<W>(conf, __FUNCTION__, ""){
	};

	//SlidingWindowOp(const SlidingWindowOp & op) : WindowOp<W>(op){};

	virtual inline
	~SlidingWindowOp(){};

	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		// drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name+"[const ImageTray &, ImageTray &]", __FUNCTION__);
		this->traverseChannelsSeparately(src, dst);
	}



	virtual
	void traverseChannel(const Channel &src, Channel &dst) const {

		//Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(SlidingWindowOp/2) (unweighted)", __FUNCTION__);
		Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		mout.debug("unweighted version");

		typename W::unweighted window(this->conf); // copies parameters (hopefully)
		//ImageTray<const Channel> & src
		window.setSrcFrame(src);
		window.setDstFrame(dst);

		mout.debug2(window );
		//mout.warn(window.myFunctor.getName() , '#' , window.myFunctor.getParameters() );

		/*
		for (int i=0; i<50; ++i){
			std::cerr << __FUNCTION__ << i << '\t' << window.myFunctor(i) << '\n';
		}
		*/

		//mout.debug3(window.getSrc() );
		//mout.debug3("slide:" );

		window.run();
		//mout.debug3("end" );

	};

	virtual
	void traverseChannel(const Channel &src, const Channel &srcWeight, Channel &dst, Channel &dstWeight) const {

		//Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(SlidingWindowOp/4)", __FUNCTION__);

		Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		mout.debug("weighted version" );
		if (srcWeight.isEmpty()){
			mout.error("weight image empty" );
		}

		W window(this->conf);  // W::conf_t & must be compatible & sufficient
		window.setSrcFrame(src);
		window.setSrcFrameWeight(srcWeight);
		window.setDstFrame(dst);
		window.setDstFrameWeight(dstWeight);

		mout.debug3(window );

		window.run();

	};

	void traverseMultiChannel(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

		// Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(SlidingWindowOp/4)", __FUNCTION__);
		Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		mout.debug("start" );

		if (!src.hasAlpha() || !dst.hasAlpha()){
			mout.info("unweighted window" );
			typename W::unweighted window(this->conf);
			window.setSrcFrames(src);
			window.setDstFrames(dst);
			mout.debug3(window );
			window.run();
		}
		else {
			mout.info("weighted window" );
			W window(this->conf);
			window.setSrcFrames(src);
			window.setDstFrames(dst);
			mout.debug3(window );
			window.run();
		}


	};


	// With templates, instantaneous classes can be created (instead of inheritance)
	SlidingWindowOp(const std::string &name = __FUNCTION__, const std::string &description = "") : WindowOp<W>(name, description){
	};

	virtual inline
	const std::string & getName() const override {
		return TypeName<image::SlidingWindowOp<W> >::str();
	}


};

} // image::

template <class W>
struct TypeName<image::SlidingWindowOp<W> > {

    static const std::string & str(){
    	static const std::string name = drain::StringBuilder<>("SlidingWindowOp<", drain::TypeName<W>::str(), ">");
        return name;
    }

};

}  // drain::

#endif // DRAIN_SLIDINGWINDOWOP_H_
