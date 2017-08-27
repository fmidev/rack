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
#ifndef SLIDINGWINDOWOP13_H_
#define SLIDINGWINDOWOP13_H_

#include "CopyOp.h"
#include "WindowOp.h"
#include "SlidingWindow.h"

namespace drain
{

namespace image
{


/*
 *   TODO: separate window from the operator, leave it to filter(...){ window.slide() };
 */

class SlidingWindowOp : public WindowOp
{

public:

	// For inherited classes
	SlidingWindowOp(const std::string &name = __FUNCTION__, const std::string &description = "") : //, int weight_supported=1, bool multichannel = false) :
		WindowOp(name, description){ // , weight_supported, 0, multichannel) {
	};


protected:

	virtual
	void traverse(const Image &src, Image &dst) const = 0;

	virtual
	void traverse(const Image &src, const Image &srcWeight, Image &dst,Image &dstWeight) const = 0;


};



/**
 *  \tparam W - a window class, which conf is adapted.
 *
 *  Class W must have:
 *  - W::config - conf structure
 *  - W::unweighted - typedef for unweighted version of the operator.
 */
template <class W>
class SlidingWindowOpT : public WindowOpP<W> {

public:

	SlidingWindowOpT(const std::string &name = __FUNCTION__, const std::string &description = "") : WindowOpP<W>(name, description){
	};

	SlidingWindowOpT(typename W::config & conf) : WindowOpP<W>(conf, __FUNCTION__, ""){
	};


protected:

	virtual
	void traverse(const Image &src, Image &dst) const {

		MonitorSource mout(iMonitor, this->name+"(SlidingWindowOpT/2)", __FUNCTION__);

		typename W::unweighted window(this->conf); // copies parameters (hopefully)
		window.setSrc(src);
		window.setDst(dst);

		mout.debug(2) << window << mout.endl;

		window.slide();

	};

	virtual
	void traverse(const Image &src, const Image &srcWeight, Image &dst, Image &dstWeight) const {

		MonitorSource mout(iMonitor, this->name+"(SlidingWindowOpT/4)", __FUNCTION__);
		//mout.error() << "weighted version" << mout.endl;

		W window(this->conf);  // W::config & must be compatible & sufficient
		window.setSrc(src);
		window.setSrcWeight(srcWeight);
		window.setDst(dst);
		window.setDstWeight(dstWeight);

		mout.debug(2) << window << mout.endl;

		window.slide();

	};


};


} // image::

}  // drain::

#endif /*SLIDINGWINDOWOP_H_*/

// Drain
