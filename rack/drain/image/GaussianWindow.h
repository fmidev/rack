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
#ifndef GaussianWindow_H_
#define GaussianWindow_H_

#include "Window.h"
#include "WindowOp.h"
//#include "SlidingWindow.h"

namespace drain
{

namespace image
{


class GaussianWindowParams : public WindowConfig {
    public: //re 
	double radius;

};

class GaussianStripe : public Window<GaussianWindowParams> { // consider scaleWeight
    public: //re 



protected:

	std::vector<double> lookUp;

	virtual
	void initialize();


	virtual
	void write(){
		dst.put(location, value);
	};

protected:

	GaussianStripe(int width, int height, double radius = 1.0) : weightSum(0), value(0) {
		setSize(width, height); // todo: drop origs
		conf.radius = radius;
	};

	double weightSum;

	mutable
	double value;

	double scaleResult;
	// double scaleResultWeight;  // consider!

	virtual
	void update() = 0;

	mutable
	Point2D<int> locationTmp;

};

class GaussianStripeHorz : public GaussianStripe {
    public: //re 

	GaussianStripeHorz(int width = 1, double radius=1.0) : GaussianStripe(width, 1, radius) {};

	void update();

};

class GaussianStripeVert : public GaussianStripe {
    public: //re 

	GaussianStripeVert(int height = 1, double radius=1.0) : GaussianStripe(1, height, radius) {};

	void update();

};


class GaussianStripeWeighted : public GaussianStripe {
    public: //re 

protected:

	GaussianStripeWeighted(int width, int height, double radius=1.0) : GaussianStripe(width, height, radius) {
	};

	virtual
	void write(){
		dst.put(location, value);
		dstWeight.put(location, weightSum);
	};

	double w;
	double sumW;

};

class GaussianStripeHorzWeighted : public GaussianStripeWeighted {
    public: //re 

	GaussianStripeHorzWeighted(int width = 1, double radius=1.0) : GaussianStripeWeighted(width, 1, radius) {};

	void update();

};


class GaussianStripeVertWeighted : public GaussianStripeWeighted {
    public: //re 

	GaussianStripeVertWeighted(int height = 1, double radius=1.0) : GaussianStripeWeighted(1, height, radius) {};  // check

	void update();

};




}  // image::

}  // drain::

#endif

// Drain
