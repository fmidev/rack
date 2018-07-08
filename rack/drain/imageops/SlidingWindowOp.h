/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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
 *  \tparam W - window class with W::conf_t structure, potentially suporting user modifiable parameters.
 *
 *  Class W must have:
 *  - W::conf_t - conf structure
 *  - W::unweighted - typedef for unweighted version of the operator.
 *  - W::slide() - method implementing the sliding.
 */
template <class W>
class SlidingWindowOp : public WindowOp<W> {

public:

	typedef W window_t;

	SlidingWindowOp(typename W::conf_t & conf) : WindowOp<W>(conf, __FUNCTION__, ""){
	};



	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		// drain::Logger mout(this->name+"[const ImageTray &, ImageTray &]", __FUNCTION__);
		this->traverseChannelsSeparately(src, dst);
	}



	virtual
	void traverseChannel(const Channel &src, Channel &dst) const {

		Logger mout(getImgLog(), this->name+"(SlidingWindowOp/2) (unweighted)", __FUNCTION__);

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

		Logger mout(getImgLog(), this->name+"(SlidingWindowOp/4)", __FUNCTION__);

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

		Logger mout(getImgLog(), this->name+"(SlidingWindowOp/4)", __FUNCTION__);

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

	virtual
	void processOLD(const ImageFrame & src, Image & dst) const;


};

template <class T>
void SlidingWindowOp<T>::processOLD(const ImageFrame & src, Image & dst) const {

	Logger mout(getImgLog(), this->name, __FUNCTION__);

	this->makeCompatible(src, dst);
	mout.debug() << "src: " << src << mout.endl;
	mout.debug() << "dst: " << dst << mout.endl;


	// TODO: check dst weighting!!!
	T window(this->conf);
	ImageTray<const Channel> srcChannels;
	srcChannels.setChannels(src);
	window.setSrcFrames(srcChannels);
	//window.setSrcFrameWeight(src.getAlphaChannel());

	ImageTray<Channel> dstChannels;
	dstChannels.setChannels(dst);
	window.setDstFrames(dstChannels);
	// window.setDstFrameWeight(dst.getAlphaChannel());
	mout.warn() << "window mode=" << window.getModeStr() << mout.endl;
	mout.debug(2) << window << mout.endl;

	window.run();

}
} // image::

}  // drain::

#endif /*SLIDINGWINDOWOP_H_*/
