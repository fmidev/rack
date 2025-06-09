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

#ifndef DOPPLER_AvgExpOP_H_
#define DOPPLER_AvgExpOP_H_

#include <drain/imageops/ImpulseResponseOp.h>
#include <drain/imageops/ImpulseAvgOp.h>

#include "data/DataCoder.h"
#include "DopplerOp.h"



namespace rack {

/// Averaging operator. A simple example implementation of ImpulseCumulator
/**
 \code
   drainage image.png --impulseAvg  0.8       -o impulseAvg.png
   drainage image.png --impulseAvg  0.8,40,20 -o impulseAvgMarg.png
 \endcode
 */
class DopplerAvg : public drain::image::ImpulseBucket<ImpulseAvgConf> {

public:


	inline
	DopplerAvg(){

	};

	inline
	DopplerAvg(const ImpulseAvg & avg){
		decays = avg.decays;
	}

	inline
	DopplerAvg(const ImpulseAvgConf & conf){
		decays = conf.decays;
	}

	virtual
	void init(const Channel & src, bool horizontal);

	virtual
	void reset();

	virtual
	void addLeft(int i, double value, double weight);

	virtual
	void addRight(int i, double value, double weight);

	virtual inline
	void addDown(int i, double value, double weight){
		addLeft(i, value, weight);
	}

	virtual inline
	void addUp(int i, double value, double weight){
		addRight(i, value, weight);
	}

	/// Return natural (not encoded) value at position i.
	virtual
	double get(int i);

	/// Returns the smoothed average of the input weights modulated with quality (non-centrity) of "radial" speed
	virtual
	double getWeight(int i);

	PolarODIM odim;

private:

	/// Accumulation on a unit circle
	struct entry {

		double x;
		double y;
		double w;

		inline void set(double x, double y, double w){
			this->x = x;
			this->y = y;
			this->w = w;
		}

	};

	/*
	 *  \param xNew - value to be added
	 *  \param wNew - weight of xNew
	 */
	inline
	void mix(entry & prev, const entry & current, double decay){

		double w1 = decay*current.w;
		double w2 = (1.0-decay);

		if (decay < 1.0){
			prev.x = (w1*current.x + w2*prev.x) / (w1 + w2);
			prev.y = (w1*current.y + w2*prev.y) / (w1 + w2);
		}
		else {// decay==1 => w2=0 => denominator would be zero if w1==0 <=> current.w==0
			prev.x = current.x;
			prev.y = current.y;
		}

		prev.w = w1 + w2*prev.w;

	}

	typedef std::pair<entry,entry> entryPair;
	typedef std::vector<entryPair> container;

	container data;

	entry e;
	entryPair latest; // utility

};



class DopplerAvgExpOp : public PolarProductOp {
public:

	DopplerAvgExpOp() : PolarProductOp(__FUNCTION__, "Doppler field smoother with exponential decay weighting") {
		parameters.append(conf.getParameters());
		parameters.link("horzExtension", horzExt = 0, "pix");
		parameters.link("vertExtension", vertExt = 0, "pix");
		//parameters.link("decay", decay = 0.8, "[0.0,1.0]");
		//parameters.link("smoothNess", smoothNess = 0.5, "[0.0,1.0]"); // neighbor weight
		dataSelector.setMaxCount(1);

		//dataSelector.setQuantity("VRAD[H]?");
		//dataSelector.setQuantity("C");
	}

	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;

	virtual
	void processData1D(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;

	drain::image::ImpulseAvgConf conf;
	int horzExt;
	int vertExt;

};

}  // rack::


#endif


