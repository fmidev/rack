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
#ifndef FLOW_AVG_W_H_
#define FLOW_AVG_W_H_

#include <drain/TypeUtils.h>
#include <algorithm>

// #include "CopyOp.h"
// #include "Window.h"
//
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
	drain::typeLimiter<double>::value_t limiter;
	// drain::ValueScaling dstScaling

	void initialize(){

		drain::Logger mout(getImgLog(), "FlowAverageWindow", __FUNCTION__);

		mout.debug2("srcTray:\n" , srcTray  );
		mout.debug2("dstTray:\n" , dstTray );

		if (srcWeight.isEmpty())
			mout.debug("no src alpha"  );

		if (dstWeight.isEmpty())
			mout.debug("no dst alpha"  );

		if (!srcTray.checkGeometry()){
			mout.special("scrTray geom: " , srcTray.getGeometry() );
			mout.special("content geom: " , srcTray );
			mout.error("srcTray geometry inconsistent"  );
		};

		if (!dstTray.checkGeometry()){
			mout.special("dstTray geom: " , dstTray.getGeometry() );
			mout.special("content geom: " , dstTray );
			mout.error("dstTray geometry inconsistent"  );
		};

		setImageLimits();
		setLoopLimits();
		this->location.setLocation(0,0);

		limiter = this->dst.getConf().getLimiter<double>();

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
		this->coordinateHandler.set(this->src.getGeometry(), this->src.getCoordinatePolicy());
		//src.adjustCoordinateHandler(this->coordinateHandler);
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

	FlowAverageOp() : SlidingWindowOp<FlowAverageWindowWeighted>(__FUNCTION__, "Window average that preserves the magnitude"){
	};

	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		// drain::Logger mout(getImgLog(), __FUNCTION__,__FILE__);  //REP (this->name+"[const ImageTray &, ImageTray &]", __FUNCTION__);
		this->traverseMultiChannel(src, dst);
	}

	virtual inline
	const std::string & getName() const override {
		return name;
	};


};





}

}

#endif // FLOW_AVG_W_H_

// Drain
