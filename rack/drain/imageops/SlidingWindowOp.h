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

#include "image/SlidingWindow.h"

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

	SlidingWindowOp(typename W::conf_t & conf) : WindowOp<W>(conf, __FUNCTION__, ""){
	};

	virtual inline
	~SlidingWindowOp(){};

	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		// drain::Logger mout(__FUNCTION__, __FILE__); //REPL this->name+"[const ImageTray &, ImageTray &]", __FUNCTION__);
		this->traverseChannelsSeparately(src, dst);
	}



	virtual
	void traverseChannel(const Channel &src, Channel &dst) const {

		//Logger mout(__FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(SlidingWindowOp/2) (unweighted)", __FUNCTION__);
		Logger mout(getImgLog(), __FUNCTION__, __FILE__);

		mout.debug() << "unweighted version" << mout.endl;

		typename W::unweighted window(this->conf); // copies parameters (hopefully)
		//ImageTray<const Channel> & src
		window.setSrcFrame(src);
		window.setDstFrame(dst);

		mout.debug(2) << window << mout.endl;
		//mout.debug(2) << window.getSrc() << mout.endl;
		//mout.debug(2) << "slide:" << mout.endl;

		window.run();
		//mout.debug(2) << "end" << mout.endl;

	};

	virtual
	void traverseChannel(const Channel &src, const Channel &srcWeight, Channel &dst, Channel &dstWeight) const {

		//Logger mout(__FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(SlidingWindowOp/4)", __FUNCTION__);

		Logger mout(getImgLog(), __FUNCTION__, __FILE__);

		mout.debug() << "weighted version" << mout.endl;
		if (srcWeight.isEmpty()){
			mout.error() << "weight image empty" << mout.endl;
		}

		W window(this->conf);  // W::conf_t & must be compatible & sufficient
		window.setSrcFrame(src);
		window.setSrcFrameWeight(srcWeight);
		window.setDstFrame(dst);
		window.setDstFrameWeight(dstWeight);

		mout.debug(2) << window << mout.endl;

		window.run();

	};

	void traverseMultiChannel(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

		// Logger mout(__FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(SlidingWindowOp/4)", __FUNCTION__);
		Logger mout(getImgLog(), __FUNCTION__, __FILE__);

		mout.debug() << "start" << mout.endl;

		if (!src.hasAlpha() || !dst.hasAlpha()){
			mout.info() << "unweighted window" << mout.endl;
			typename W::unweighted window(this->conf);
			window.setSrcFrames(src);
			window.setDstFrames(dst);
			mout.debug(2) << window << mout.endl;
			window.run();
		}
		else {
			mout.info() << "weighted window" << mout.endl;
			W window(this->conf);
			window.setSrcFrames(src);
			window.setDstFrames(dst);
			mout.debug(2) << window << mout.endl;
			window.run();
		}


	};


	// Since templating was introduced, instantaneous classes can be created (instad of inheritance)
	SlidingWindowOp(const std::string &name = __FUNCTION__, const std::string &description = "") : WindowOp<W>(name, description){
	};

protected:

	// virtual	void processOLD(const ImageFrame & src, Image & dst) const;


};



} // image::

}  // drain::

#endif /*SLIDINGWINDOWOP_H_*/

// Drain
