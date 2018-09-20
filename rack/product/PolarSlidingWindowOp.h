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

#ifndef POLAR_SLIDINGWINDOWOP_H_
#define POLAR_SLIDINGWINDOWOP_H_

#include <drain/image/SlidingWindowOp.h>
#include "PolarProductOp.h"

namespace drain {

namespace radar {


template <class W>
class PolarSlidingWindowOp : public PolarProductOp {

public:

	PolarSlidingWindowOp(const std::string &name, const std::string &description) : PolarProductOp(name,description) {
		parameters.append(conf.getParameters());
		//reference("width", width);
		//reference("height", height);
	};

	typename W::conf_t conf;

	virtual
	void processData(const Data<src_t > & srcData, Data<dst_t > & dstData) const {
		drain::Logger mout(this->name, __FUNCTION__);
		if (srcData.hasQuality())
			processDataWeighted(srcData, dstData);
		else
			processPlainData(srcData, dstData);
	};

	virtual
	void processPlainData(const PlainData<src_t > & srcData, PlainData<dst_t > & dstData) const {
		drain::Logger mout(this->name, __FUNCTION__);
		mout.warn() << "not implemented" << mout.endl;
	};

	/// Quality-weighted prosessing of data
	virtual
	void processDataWeighted(const Data<src_t > & srcData, Data<dst_t > & dstData) const {
		drain::Logger mout(this->name, __FUNCTION__);
		mout.warn() << "not implemented" << mout.endl;
	};


protected:



};

}

/*
class PolarSlidingWindowOp : public PolarProductOp {

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
*/

}

#endif /* POLARSLIDINGDOPPLERWINDOW_H_ */
