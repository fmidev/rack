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
//#include "Window.h"

namespace drain
{

namespace image
{



/**
 *
 *  TODO: const std::string name;
 */
class WindowOp : public ImageOp
{
  public:
	
	WindowOp(const std::string & name = __FUNCTION__, const std::string & description=""): //, int weight_supported=1, int in_place=2, bool multichannel=false) :
		ImageOp(name, description) {
		this->parameters.reference("width", width = 0);
		this->parameters.reference("height", height = 0);
	};

	virtual ~WindowOp(){};

	inline
	void setSize(unsigned int width, unsigned int height = 0) {
		setParameter("width", width);
		if (height == 0)
			height = width;
		setParameter("height", height);
	}


	int width;
	int height;

};


/**
 *  \tparam W - window class (e.g. drain::Window); W must have member of type W::config.
 *
 */
template <class W>
class WindowOpP : public ImageOp {
public:

	typename W::config conf;

	WindowOpP(const std::string & name = __FUNCTION__, const std::string & description="") :
		ImageOp(name, description) {
		this->parameters.reference("width",  conf.width);  // NEW 2017
		this->parameters.reference("height", conf.height);  // NEW 2017

	};

	WindowOpP(const std::string & name, const std::string & description, unsigned int width, unsigned int height) :
		ImageOp(name, description) {
		this->parameters.reference("width",  conf.width);   // NEW 2017
		this->parameters.reference("height", conf.height);  // NEW 2017
		setSize(width, height);
	};

	WindowOpP(typename W::config & c, const std::string & name = __FUNCTION__, const std::string & description="") :
		ImageOp(name, description), conf(c) {
		this->parameters.reference("width",  conf.width);
		this->parameters.reference("height", conf.height);

	};


	virtual ~WindowOpP(){};

	// TODO virtual?
	void setSize(unsigned int width, unsigned int height = 0){
		conf.width = width;
		if (height == 0)
			conf.height = width;
		else
			conf.height = height;
	}


};



}

}

#endif /*WINDOWOP_H_*/

// Drain
