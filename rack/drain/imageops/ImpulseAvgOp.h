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


#include "drain/util/UniTuple.h"

#include "drain/image/Coordinates.h"
#include "drain/image/FilePng.h"
//#include "drain/image/SegmentProber.h"

#include "ImageOp.h"
#include "ImpulseResponseOp.h"

namespace drain
{
namespace image
{

template <class T>
class Decay2 : public UniTuple<T,2> {

public:

	double & forward;
	double & backward;

	Decay2(double decay = 1.0) : forward(this->next()), backward(this->next()) {
		this->fill(decay);
	}

	// Reference
	template <size_t N>
	Decay2(drain::UniTuple<T,N> & tuple, size_t i) : drain::UniTuple<T,2>(tuple, i),  forward(this->next()), backward(this->next()){
	};
};

template <class T>
struct Decay4 : public drain::UniTuple<T,4> {

	Decay2<T> horz;
	Decay2<T> vert;

	Decay4(T decay=0.5) : horz(this->tuple(), 0), vert(this->tuple(), 2) {
		this->fill(decay);
	}


	Decay4(const Decay4 & r) : horz(this->tuple(), 0), vert(this->tuple(), 2){
		this->assign(r);
	};

	Decay4 & operator=(const Decay4<T> & decay){
		this->set(decay.tuple());
		return *this;
	}

};


// IMPLEMENTATIONS


struct ImpulseAvgConf : public BeanLike {


	inline
	ImpulseAvgConf() : BeanLike(__FUNCTION__, "Infinite-impulse response type spreading"), decays(0.75){
		// this->parameters.link("decayHorz", decayHorz = 0.9);
		// this->parameters.link("decayVert", decayVert = 0.9);
		this->parameters.link("decay", decays.tuple()); //.fillArray = true;
		//this->parameters.link("decay", decay = 0.9);
		// this->parameters.link("decayVert", decayVert = 0.9);
	};

	inline
	ImpulseAvgConf(const ImpulseAvgConf & conf) :
		BeanLike(__FUNCTION__, "Infinite-impulse response type spreading"), decays(0.75){
		this->parameters.link("decay", decays.tuple()); //.fillArray = true;
		// this->parameters.link("decayHorz", decayHorz = conf.decayHorz);
		// this->parameters.link("decayVert", decayVert = conf.decayHorz);
	};

	Decay4<double> decays;
	//double decay;
	//std::vector<double> decays;
	//double decayHorz;
	//double decayVert;

};


/// Averaging operator. A simple example implementation of ImpulseBucket
/**
 \code
   drainage image.png --iImpulseAvg  0.5            -o impulseAvg.png
   drainage image.png --iImpulseAvg  0.2,20,20      -o impulseAvgMarg.png
   drainage image-rgba.png --target S --iImpulseAvg  0.5 -o impulseAvg-16b.png
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
		double weight;

		inline void set(double value, double weight){
			this->x = value;
			this->weight = weight;
		}

	};

	/*
	 *  \param xNew - value to be added
	 *  \param wNew - weight of xNew
	 */
	inline
	void mix(entry & prev, const entry & e, double decay){

		double w1 = decay*e.weight;
		double w2 = (1.0-decay);

		if (decay < 1.0)
			prev.x =(w1*e.x + w2*prev.x) / (w1 + w2);
		else // decay==1.0
			prev.x = e.x;

		prev.weight = w1 + w2*prev.weight;

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
