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
*//*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

    This file is part of AnoRack, a module of Rack for C++.

    AnoRack is not free software.

   Created on: Sep 12, 2010
       Author: mpeura
*/

#ifndef POLARSLIDINGWINDOWOP_H_
#define POLARSLIDINGWINDOWOP_H_

#include <drain/image/SlidingWindowOp.h>
#include "PolarProductOp.h"

namespace drain {

namespace radar {

class PolarSlidingWindowOp : public PolarProductOp {
	//public image::SlidingWindowOp<T,T2> {
public:

	PolarSlidingWindowOp(SlidingWindow & w,const std::string &name, const std::string &description) : PolarProductOp(name,description) {
				//SlidingWindowOp(w,name,description,parameterNames,defaultValues) {
		//reference("altitude", this->altitude, altitude);
		reference("width", width);
		reference("height", height);
	};


	virtual void initialize() const {
		window.setSize(width,height);
	}

	int width;
	int height;

protected:
	SlidingWindow & window;
	mutable std::vector<double> lookupSin;
	mutable std::vector<double> lookupCos;
};

}

}

#endif /* POLARSLIDINGDOPPLERWINDOW_H_ */
