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
#ifndef IMAGENEWOP_H_
#define IMAGENEWOP_H_

//#include <drain/util/DataScaling.h>
#include "util/BeanLike.h"
#include "util/DataScaling.h"
//#include "util/Data.h"
//#include "util/Options.h"
//#include "util/Tree.h"
#include "util/ReferenceMap.h"
#include "Image.h"



namespace drain {

namespace image {

/// Base class for image processing functions.
/**
 *
 *  TODO:
 */
class ImageOp : public BeanLike {
public:

	/**
	 * \param  weight_supported - not supported (0), flexibly supported (1) or required (2)
	 *  \param in_place - 0 = separate dst required, 1 = separate dst encouraged, 2 = dst is allowed to be src
	 */
	inline
	ImageOp(const std::string &name = __FUNCTION__, const std::string &description="")  : BeanLike(name, description)  // , int weight_supported=1, int in_place=2, bool multichannel=false) :
		//name(name), description(description)
		{}; //, WEIGHT_SUPPORTED(weight_supported), IN_PLACE(in_place), MULTICHANNEL(multichannel) {};

	inline
	virtual ~ImageOp(){};


	/// Run this operator. Calls first makeCompatible() and initializeParameters(), then calls traverse().
	virtual
	void filter(const Image & src, Image & dst) const;

	/// Run this operator. Calls first makeCompatible() and initializeParameters(), then calls traverse().
	virtual
	void filter(const Image & src, const Image & src2, Image &dst) const;  //{
		// throw std::runtime_error(name+"::"+__FUNCTION__+"(src,src2,dst) unimplemented.");  // consider INPUT_COUNT
	// }


	/// Run this operator with input weighting. Calls first makeCompatible() and initializeParameters(), then calls traverse().
	virtual
	void filter(const Image &src, const Image &srcAlpha, Image &dst, Image &dstAlpha) const ;



	// inline	void setParameters(const std::string &p){ parameters.setValues(p); }; // BeanLike

	// inline const ReferenceMap & getParameters() const { return parameters; }   // BeanLike


	template <class F>
	inline
	void setParameter(const std::string &p, const F &value){ parameters[p] = value; }

	inline
	bool hasParameter(const std::string &p) const { return parameters.hasKey(p); }


	// TODO: would template on top of Castable be better?
	inline
	const drain::Castable & getParameter(const std::string &p) const {
		if (hasParameter(p))
			return parameters[p];
		else // Unneeded?
			throw std::runtime_error(p + ": no such parameter in ImageOp::getParameter");
	}

	//inline
	template <class F>
	F getParameter(const std::string &p, const F & defaultValue) const {
		if (hasParameter(p))
			return parameters[p];
		else {
			std::cerr << name << ": warning: undefined parameter '"<< p << "' (ImageOp::getParameter) \n";
			return defaultValue;
		}
	}


	inline
	int getParameterCount(){ return parameters.size(); };

	inline
	const std::map<std::string,drain::Castable> & getParameterMap() const {
		return parameters.getMap();
	}

	inline
	const std::list<std::string> & getParameterNames() const {
		return parameters.getKeyList();
	}



	virtual
	void help(std::ostream &ostr = std::cout) const;

	// BeanLike
	/*
	const std::string name;
	inline 	const std::string & getName() const { return this->name; };
	inline	const std::string & getDescription() const { return this->description; };
	*/

    static
    inline
    void adaptCoordinateHandler(const Image & src, CoordinateHandler2D & handler){
    	handler.setLimits(src.getWidth(), src.getHeight());
    	handler.setPolicy(src.getCoordinatePolicy());
    }

    /*
    virtual
    inline
    bool isInPlace(){

    }
    */

	/// Depending on the operator, modifies the geometry and type of dst.
	/*  This default implementation
	 *  -# returns immediately, if dst==src.
	 *  -# sets dst type, if unset, to that of src
	 *  -# sets dst geometry to that of src
	 */
	virtual
	void makeCompatible(const Image & src, Image & dst) const;

	/// Modifies the geometry and type of dst to fit the computation result.
	/*
	 * This default implementation
	 *  - returns immediately, if dst has overlap with src1 or src2.
	 *  - else, calls makeCompatible(src1, dst); src2 has no effect.
	 */
	virtual
	void makeCompatible(const Image & src1, const Image & src2, Image & dst) const;


	/* TODO?
	virtual
	inline
	void makeCompatibleAlpha(const Image & src, Image & dst) const;
	*/


	/// Depending on the operator, modifies the geometry and type of dst.
	/*  This default implementation
	 *  -# sets dst type, if unset, to that of src
	 *  -# sets dst geometry equal to that of src
	 */
	virtual
	inline
	void makeCompatible(const Image &src, const Image &srcAlpha, Image &dst, Image &dstAlpha) const  {

		drain::MonitorSource mout(iMonitor, name+"(ImageOp)", __FUNCTION__);

		makeCompatible(src, dst);
		makeCompatible(srcAlpha, dstAlpha);

	};


	//static  // TODO: move to Image?
	//void setView(Image & src, const std::string & view, Image & dstView);


    /// The main functionality called by filter() after image compatibility check and tmp allocations
    /**
     *  Other classes - like other operators - may call these also directly, but then compatibility must be ensured.
     */
    virtual
    void traverse(const Image &src, Image &dst) const {
    	throw std::runtime_error(name+"::"+__FUNCTION__+"(src,dst) unimplemented.");
    };

    virtual
    void traverse(const Image &src, const Image & src2, Image &dst) const {
    	throw std::runtime_error(name+"::"+__FUNCTION__+"(src, src2 ,dst) unimplemented.");
    };

    /// The main functionality called by filter() after image compatibility check and tmp allocations
    /**
     */
    virtual void traverse(const Image &src, const Image &srcAlpha, Image &dst, Image &dstAlpha) const {
    	throw std::runtime_error(name+"::"+__FUNCTION__+"(src,srcAlpha,dst,dstAlpha) unimplemented.");
    };


protected:
    // const std::string description; // BeanLike


    /*
public:

    const int WEIGHT_SUPPORTED; // Not supported (0), flexibly supported (1) or required (2)

    const int IN_PLACE; // If dst == src, needs tmp image; for example window ops cannot be computed in-place.

    const bool MULTICHANNEL; // Several channels are applied in parallel; do not traverse them separately
*/



protected:

    // drain::ReferenceMap parameters;  // BeanLike

    // Reconsider these (use direct parameters.reference() calls instead.)
    template <class T>
    inline
    void reference(const std::string key, T & target){
    	parameters.reference(key, target);
    }


	/// Set applicable internal parameters before calling traverse().
	inline
	virtual
	void initializeParameters(const Image &src, const Image &dst) const {
		drain::MonitorSource mout(iMonitor, name+"(ImageOp) src,dst", __FUNCTION__);
		mout.debug() << "nothing defined (ok)" << mout.endl;
	}

	/// Set applicable internal parameters before calling traverse().
	inline
	virtual
	void initializeParameters(const Image &src, const Image &src2, const Image &dst) const {
		drain::MonitorSource mout(iMonitor, name+"(ImageOp) src,src2,dst", __FUNCTION__);
		mout.debug() << "nothing defined (ok)" << mout.endl;
	}

	void filterCompatible(const Image & src, const Image & srcAlpha, Image & dst, Image & dstAlpha) const;


	/// If src or dst have more than one channel, traverse through channels.
	/**
	 *  \par src - source image
	 *  \par dst - destination image
	 *
	 *  Some image operators should process each channel, independently from each others.
		Place this in the beginning of your filter(src,dst) as follows:

	 *  \code
	 *  if (traverseChannelsSeparately(src,dst))
	 *  	return;
	 *  \endcode
	 *
	 *
	 *   In multi-channel operations, this function should simply return \c false.
	 */
	virtual
	bool traverseChannelsSeparately(const Image & src, Image & dst) const;

	/// If src1 or src2 have more than one channel, traverse through channels.
	/*
	 *   In multi-channel operations, this function should simply return \c false.
	 */
	virtual
	bool traverseChannelsSeparately(const Image & src1, const Image & src2, Image & dst) const;

	/// If src or dst have more than one channel, traverse through channels.
	/*
	 *   In multi-channel operations, this function should simply return \c false.
	 */
	virtual
	bool traverseChannelsSeparately(const Image & src, const Image & srcAlpha, Image & dst, Image & dstAlpha) const;


	/// Utility for re-calling filter() with a temporary dst image, when dst==src would corrupt the computation.
	/**
	 *  \par src - source image
	 *  \par dst - destination image
	 *
	 *  Place this in the beginning of your filter(src,dst) as follows:
	 *  \code
	 *  if (filterWithTmp(src,dst))
	 *  	return;
	 *
	 *   If temporary images are never needed, this function should simply return \c false.
	 */
	virtual
	bool filterWithTmp(const Image & src, Image & dst) const;

	/*
	 *   If temporary images are never needed, this function should simply return \c false.
	 */
	virtual
	bool filterWithTmp(const Image & src, const Image & src2, Image & dst) const;

	/// Utility for re-calling filter() with a temporary dst image, when dst==src would corrupt the computation.
	/**
	 *  \par src - source image
	 *  \par srcAlpha - source image handled as alpha channel.
	 *  \par dst - destination image
	 *  \par dstAlpha - destination image handled as alpha channel.
	 *
	 *  Place this in the beginning of your filter(src,dst) as follows:
	 *  \code
	 *  if (filterWithTmp(src, srcAlpha, dst, dstAlpha))
	 *  	return;
	 *  \endcode
	 */
	virtual
	bool filterWithTmp(const Image & src, const Image & srcAlpha, Image & dst, Image & dstAlpha) const;


	/// If src has alpha channels, separates them from image channels and calls filter(src, srcAlpha, dst, dstAlpha).
	/*
	 *   If weighting is not supported, this function should simply return \c false.
	 */
	virtual
	bool filterUsingAlphas(const Image & src, Image & dst) const;






};



inline
std::ostream & operator<<(std::ostream & ostr, const ImageOp &op){
	op.help(ostr); // todo: name & params only
	return ostr;
}

}  // namespace image

}  // namespace drain


#endif /* IMAGE_OP_H_ */

// Drain
