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
#ifndef ImpulseAvgOp_H
#define ImpulseAvgOp_H

#include <sstream>
#include <ostream>
//#include "drain/utility>

#include "drain/image/Coordinates.h"
#include "drain/image/FilePng.h"
//#include "drain/image/SegmentProber.h"

#include "ImageOp.h"
#include "ImpulseResponseOp.h"

namespace drain
{
namespace image
{





// IMPLEMENTATIONS


struct ImpulseAvgConf : public BeanLike {

	inline
	ImpulseAvgConf() : BeanLike(__FUNCTION__, "Infinite-impulse response type spreading"), decays(4, 0.75){
		// this->parameters.reference("decayHorz", decayHorz = 0.9);
		// this->parameters.reference("decayVert", decayVert = 0.9);
		this->parameters.reference("decay", decays).fillArray = true;
		//this->parameters.reference("decay", decay = 0.9);
		// this->parameters.reference("decayVert", decayVert = 0.9);
	};

	inline
	ImpulseAvgConf(const ImpulseAvgConf & conf) :
		BeanLike(__FUNCTION__, "Infinite-impulse response type spreading"), decays(4, 0.75){
		this->parameters.reference("decay", decays).fillArray = true;
		// this->parameters.reference("decayHorz", decayHorz = conf.decayHorz);
		// this->parameters.reference("decayVert", decayVert = conf.decayHorz);
	};

	//double decay;
	std::vector<double> decays;
	//double decayHorz;
	//double decayVert;

};


/// Averaging operator. A simple example implementation of ImpulseBucket
/**
 \code
   drainage image.png --impulseAvg  0.5            -o impulseAvg.png
   drainage image.png --impulseAvg  0.2,20,20      -o impulseAvgMarg.png
   drainage image-rgba.png --target S --impulseAvg  0.5 -o impulseAvg-16b.png
 \endcode
 */
class ImpulseAvg : public ImpulseBucket<ImpulseAvgConf> {

public:


	inline
	ImpulseAvg(){

	};

	inline
	ImpulseAvg(const ImpulseAvg & avg){
		decays = avg.decays;
	}

	inline
	ImpulseAvg(const ImpulseAvgConf & conf){
		decays = conf.decays;
	}

	virtual
	void init(const Channel & src, bool horizontal);

	virtual
	void reset();

	/// Accumulate encoded value
	virtual
	void addLeft(int i, double value, double weight);

	/// Accumulate encoded value
	virtual
	void addRight(int i, double value, double weight);

	/// Accumulate encoded value
	virtual
	void addDown(int i, double value, double weight);

	/// Accumulate encoded value
	virtual
	void addUp(int i, double value, double weight);


	virtual
	double get(int i);

	virtual
	double getWeight(int i);


protected:

private:

	 drain::ValueScaling scaling;

	/// Accumulating unit using natural values
	struct entry {

		double x;
		double w;

		inline void set(double value, double weight){
			x = value;
			w = weight;
		}

	};

	/*
	 *  \param xNew - value to be added
	 *  \param wNew - weight of xNew
	 */
	inline
	void mix(entry & prev, const entry & e, double decay){

		double w1 = decay*e.w;
		double w2 = (1.0-decay);

		if (decay < 1.0)
			prev.x =(w1*e.x + w2*prev.x) / (w1 + w2);
		else // decay==1.0
			prev.x = e.x;

		prev.w = w1 + w2*prev.w;

	}

	typedef std::pair<entry,entry> entryPair;
	typedef std::vector<entryPair> container;

	container data;

	entry e;
	entryPair latest; // utility


};



} // image::

} // drain::


#endif /* ImpulseResponse_H_ */
