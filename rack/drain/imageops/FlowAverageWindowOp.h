/**

    Copyright 2015 -   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef FLOW_AVG_W_H_
#define FLOW_AVG_W_H_

#include <algorithm>

// #include "CopyOp.h"
// #include "Window.h"
//
#include "util/TypeUtils.h"
#include "SlidingWindowOp.h"

namespace drain
{

namespace image
{

/// Window average that preserves the magnitude
/**
 *
 */
// TODO: redesign RadarWindowConfig<Polar> ? and
class FlowAverageWindow : public SlidingWindow<WindowConfig, MultiChannelWindowCore> {

public:

	typedef FlowAverageWindow unweighted;

	FlowAverageWindow(int width=5, int height=0) : SlidingWindow<WindowConfig, MultiChannelWindowCore>(width, height){ //qualitySensitive(false)
		clear();
	};

	FlowAverageWindow(const WindowConfig & conf) : SlidingWindow<WindowConfig, MultiChannelWindowCore>(conf) { //qualitySensitive(false) {
		clear();
	};

	// ODIM odimSrc; future option.

	//bool qualitySensitive;

protected:

	double sumMagnitude;
	std::vector<double> sum;
	/*
	double sumU;
	double sumV;
	double sumU2;
	double sumV2;
	 */
	double sumW;
	unsigned int count;
	typename drain::typeLimiter<double>::value_t limiter;
	//ImageScaling dstScaling

	void initialize(){

		drain::Logger mout("FlowAverageWindow", __FUNCTION__);

		mout.debug(1) << "srcTray:\n" << srcTray  << mout.endl;
		mout.debug(1) << "dstTray:\n" << dstTray << mout.endl;

		if (srcWeight.isEmpty())
			mout.debug() << "no src alpha"  << mout.endl;

		if (dstWeight.isEmpty())
			mout.debug() << "no dst alpha"  << mout.endl;

		if (!srcTray.checkGeometry()){
			mout.error() << "srcTray geometry inconsistent"  << mout.endl;
		};

		if (!dstTray.checkGeometry()){
			mout.error() << "dstTray geometry inconsistent"  << mout.endl;
		};

		setImageLimits();
		setLoopLimits();
		this->location.setLocation(0,0);

		limiter = this->dst.getLimiter<double>();

		sum.resize(dstTray.size(), 0.0);
		fillBoth();
	}

	/// Clears statistics
	void clear(){
		std::fill(sum.begin(), sum.end(), 0);
		sumMagnitude = 0.0;
		sumW  = 0.0;
		count = 0;
	};

	void setImageLimits() const {
		src.adjustCoordinateHandler(this->coordinateHandler);
	}

	virtual
	void removePixel(Point2D<int> & p);

	virtual
	void addPixel(Point2D<int> & p);

	virtual
	void write();

};

class FlowAverageWindowWeighted : public FlowAverageWindow {

public:

	FlowAverageWindowWeighted(int width=5, int height=0) : FlowAverageWindow(width, height) {
	};

	FlowAverageWindowWeighted(const WindowConfig & conf) : FlowAverageWindow(conf){
	};


	virtual
	void removePixel(Point2D<int> & p);

	virtual
	void addPixel(Point2D<int> & p);

	virtual
	void write();

};

/// Window average that preserves the magnitude
/**
 *
 */
class FlowAverageOp : public SlidingWindowOp<FlowAverageWindowWeighted> {

public:

	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		// drain::Logger mout(this->name+"[const ImageTray &, ImageTray &]", __FUNCTION__);
		this->traverseMultiChannel(src, dst);
	}

};





}

}

#endif // FLOW_AVG_W_H_
