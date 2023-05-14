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
#ifndef WINDOWOP_H_
#define WINDOWOP_H_

#include "ImageOp.h"
#include "drain/image/Window.h"

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
		this->parameters.link("width",  conf.frame.tuple()).fillArray = true;
		// this->parameters.link("width",  conf.frame.width);
		// this->parameters.link("height", conf.frame.height);
	};

	WindowOp(const std::string & name, const std::string & description, unsigned int width, unsigned int height) :
		ImageOp(name, description) {
		this->parameters.link("width",  conf.frame.tuple()).fillArray = true;
		// this->parameters.link("width",  conf.frame.width);
		// this->parameters.link("height", conf.frame.height);
		setSize(width, height);
	};

	WindowOp(typename W::conf_t & c, const std::string & name = __FUNCTION__, const std::string & description="") :
		ImageOp(name, description), conf(c) {
		this->parameters.link("width",  conf.frame.tuple()).fillArray = true;
		// this->parameters.link("width",  conf.frame.width);
		// this->parameters.link("height", conf.frame.height);
	};

	WindowOp(const WindowOp<W> & op) : ImageOp(op), conf(op.conf){
		this->parameters.link("width",  conf.frame.tuple()).fillArray = true;
		// this->parameters.link("width",  conf.frame.width);
		// this->parameters.link("height", conf.frame.height);
	}

	virtual ~WindowOp(){};

	// TODO virtual?
	//void setSize(unsigned int width, unsigned int height = 0){
	inline
	void setSize(unsigned int width){
		setSize(width, width);
	}

	void setSize(unsigned int width, unsigned int height){
		conf.frame.set(width, height);
	}


};



}

}

#endif /*WINDOWOP_H_*/

// Drain
