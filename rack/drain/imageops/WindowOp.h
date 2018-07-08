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
#ifndef WINDOWOP_H_
#define WINDOWOP_H_

#include "ImageOp.h"
#include "image/Window.h"

namespace drain
{

namespace image
{



/**
 *  \tparam W - window class (e.g. drain::Window); W must have member of type W::conf_t.
 *
 */
template <class W = Window<WindowConfig> >
class WindowOp : public ImageOp {
public:

	typedef  W window_t;

	typename W::conf_t conf;

	WindowOp(const std::string & name = __FUNCTION__, const std::string & description="") :
		ImageOp(name, description) {
		this->parameters.reference("width",  conf.width);  // NEW 2017
		this->parameters.reference("height", conf.height);  // NEW 2017

	};

	WindowOp(const std::string & name, const std::string & description, unsigned int width, unsigned int height) :
		ImageOp(name, description) {
		this->parameters.reference("width",  conf.width = width);   // NEW 2017
		this->parameters.reference("height", conf.height = height);  // NEW 2017
		setSize(width, height);
	};

	WindowOp(typename W::conf_t & c, const std::string & name = __FUNCTION__, const std::string & description="") :
		ImageOp(name, description), conf(c) {
		this->parameters.reference("width",  conf.width);
		this->parameters.reference("height", conf.height);

	};


	virtual ~WindowOp(){};

	// TODO virtual?
	//void setSize(unsigned int width, unsigned int height = 0){
	inline
	void setSize(unsigned int width){
		conf.width  = width;
		conf.height = width;
	}

	void setSize(unsigned int width, unsigned int height){
		conf.width = width;
		/*if (height == 0)
			conf.height = width;
		else
		*/
		conf.height = height;
	}

	/*
	virtual
	void process(const ImageFrame & src, Image & dst) const{

		drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

		//mout.debug() << "delegating back to ImageOp::processOverlappingWithTemp" << mout.endl;
		if (processOverlappingWithTemp(src, dst)){
			mout.debug() << "finished processing overlapping images" << mout.endl;
			return;
		}

		makeCompatible(src, dst);

		ImageTray<const Channel> srcTray;
		srcTray.setChannels(src);

		ImageTray<Channel> dstTray;
		dstTray.setChannels(dst);

		traverseChannels(srcTray, dstTray);

	}
	*/

};



}

}

#endif /*WINDOWOP_H_*/
