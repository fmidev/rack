/*

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
