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
#ifndef SEGMENT_PROBER_H
#define SEGMENT_PROBER_H

#include <drain/image/ProbingControl.h>  // Control
#include <sstream>
#include <ostream>

#include "drain/util/ReferenceMap.h"
#include "CoordinateHandler.h"
#include "FilePng.h"


namespace drain
{
namespace image
{


/// Container for parameters of SegmentProber
/**
 *
 *   \tparam S - storage type of the source image data (int by default, but should be floating-type, if src is).
 *   \tparam D - storage type of the destination image data
 *
 *   \author Markus.Peura@fmi.fi
 */
template <class S, class D>
class SegmentProberConf : public drain::ReferenceMap {

public:

	SegmentProberConf(S anchorMin=1, S anchorMax=255.0, D markerValue = 1) : anchor(anchorMin, anchorMax), markerValue(markerValue){
		link("anchor", this->anchor.tuple(), "intensity");
		//link("anchorMin", this->anchorMin = anchorMin, "intensity");
		//link("anchorMax", this->anchorMax = anchorMax, "intensity");
		link("marker", this->markerValue, "marker not universal?");
	}

	SegmentProberConf(const SegmentProberConf & conf) : anchor(conf.anchor), markerValue(conf.markerValue){
		// link("anchor", this->anchor.tuple(), "intensity");
		copyStruct(conf, conf, *this);
	}

	Range<S> anchor;

	/// "fill value", also dynamically changing visit marker?
	D markerValue;

	/// Criterion
	inline
	bool isValidIntensity(S x) const {
		return anchor.contains(x);
		//return (x >= anchorMin) && (x <= anchorMax);
	}


};

/// A recursive method for visiting pixels of a segment in an image.
/**
 *   A helper class applied by FloodFillOp and SegmentAreaOp.
 *
 *   \tparam S - storage type of the source image data (int by default, but should be floating-type, if src is).
 *   \tparam D - storage type of the destination image data
 *   \tparam C - configuration type,
 *
 *   \author Markus.Peura@fmi.fi
 */
template <class S, class D, class C>
class SegmentProber {

protected:

	SimpleProberControl basicControl;

public:

	ProberControl & control;

	typedef S src_t;
	typedef D dst_t;
	typedef C conf_t;

	conf_t conf;

	SegmentProber(const Channel &s) : control(basicControl), src(s), dst(NULL){
		init();
	};

	SegmentProber(const Channel &s, Channel &d) : control(basicControl), src(s), dst(&d) {
		control.markerImage.setGeometry(s.getGeometry().area);
		init();
	};

	virtual
	~SegmentProber(){};

	/// Set new target channel. Needed in multichannel operations.
	void setDst(Channel & d){
		dst = &d;
	}

	/*
	void setParams(const C & conf){
		this->conf = conf;
		// init()
	}
	*/

	/// Fills the segment having intensity between min and max.
	/*
	void setParams(src_t min, src_t max, dst_t fillValue){
		anchorMin = min;
		anchorMax = max;
		value = fillValue;
	}
	*/

	/// Called after src and dst have been set, but before processing. See clear().
	virtual
	void init(){

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		control.handler.set(src.getGeometry(), src.getCoordinatePolicy());
		// src.adjustCoordinateHandler(handler);
		mout.debug(control.handler );
		mout.debug2(src );
		if (dst){
			mout.debug2(*dst );
		}

	}


	/// A convenience function for traversing a whole image.
	/**
	 *  Applicable in cases where probing parameters are not spatically ("dynamically") changing.
	 *
	 */
	void scan(){

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		const CoordinatePolicy & cp = control.handler.getPolicy();
		bool HORZ_MODE = ((cp.xUnderFlowPolicy != EdgePolicy::POLAR) && (cp.xOverFlowPolicy != EdgePolicy::POLAR));

		if (HORZ_MODE){
			mout.debug("Horz Probe");
			for (size_t i=0; i<src.getWidth(); ++i){
				for (size_t j=0; j<src.getHeight(); ++j){
					probe(i,j, true);
					//probeH(i,j);
					/*
					clear();
					if (handler.validate(i, j)){
						scanHorzProbeVert(i,j);
					}*/
				}
			}
		}
		else {
			mout.debug("Vert Probe");
			for (size_t j=0; j<src.getHeight(); ++j){
				for (size_t i=0; i<src.getWidth(); ++i){
					probe(i,j, false);
					/*clear();
					if (handler.validate(i, j)){
						scanVertProbeHorz(i,j);
					}
					*/
				}
			}
		}
	}


	/// Start probings
	void probe(int i, int j, bool HORIZONTAL){


		clear();
		if (control.handler.validate(i, j)){
			/*
			if (isValidPixel(i,j)){
				Logger mout(getImgLog(), "SegmentProber", __FUNCTION__);
				mout.warn("accept: " , i , ',' , j , "\t=" , src.get<double>(i,j) , '\t' , dst->get<double>(i,j) );
			}
			*/
			if (HORIZONTAL)
				scanHorzProbeVert(i,j);
			else
				scanVertProbeHorz(i,j);
		}

	};


// consider protected:

	const Channel & src;
	Channel *dst;

	// CoordinateHandler2D handler;



	/// Returns isValidSegment(i,j) and !isVisited(i,j).
	inline
	bool isValidPixel(int i, int j) const {
		return (isValidSegment(i,j) && !isVisited(i,j));
	}

	/// Application dependent, to be redefined. Assumes checked coordinates.
	/**
	 *  Determines if the current position is within a segment. The criterion of "segment"
	 *  depends on the application.
	 *
	 *  Note: does not check coordinates, assumes them to be checked by the calling scope.
	 *
	 */
	virtual
	bool isValidSegment(int i, int j) const = 0;
	//return (src.get<src_t>(i,j) > 0);
	//return (src.get<src_t>(i,j) >= anchorMin) && (src.get<src_t>(i,j) <= anchorMax);

	/// Application dependent, to be redefined. Note: assumes checked coordinates.
	/*
	virtual inline
	bool isValidSegment(int i, int j) const {
		return conf.isValidIntensity(src.get<src_t>(i,j));
	}
	*/

protected:

	/// Application dependent. Assumes checked coordinates.
	virtual inline
	bool isValidMove(int i0, int j0, int i, int j) const {
		return true;
	}


	///  Application dependent operation performed in each segment location (i,j).
	virtual	inline
	void update(int i, int j){
	};


	/// Visiting a single pixel when not coming from any direction.
	/**
	 *  Visiting means
	 *  - updating the status of this prober, for example by updating statistics like segment size
	 *  - marking the pixel visited in dst image
	 *
	 *  \param i0 - current i coordinate (always valid)
	 *  \param j0 - current j coordinate (always valid)
	 *
	 */
	virtual inline
	void visit(int i, int j) {

		update(i, j);

		/// Mark visited
		dst->put(i,j, 1);

	}

protected:

	/// Called before processing each segment. Compare with init(), which is called once for each image.
	/**
	 *  Application dependent initialisation for statistics updated with update(int i, int j) function.
	 */
	virtual
	void clear(){};


	/// Experimental
	/**
	 */
	virtual inline
	bool isVisited(int i, int j) const {
		return (dst->get<D>(i,j) != 0);
	}



	/// Try to move; change coords in success, else keep them intact.
	template <int DI, int DJ>
	inline
	bool move(int & i, int & j){ //, int DI, int DJ) {

		// Save
		const int i0 = i;
		const int j0 = j;

		i += DI;
		j += DJ;

		// Drop isValidSegment
		if (control.handler.validate(i,j)){ // must be first, changes coords
			if (isValidPixel(i,j))
				if (isValidMove(i0,j0, i,j))
					return true;
		}

		// restore orig
		i = i0;
		j = j0;
		return false;

	}




    /// A semi-recursive approach that turns the 2D recursion to 1D traversal + 1D recursion.
	/**
	 *
s	 *   the horizontal direction is handled sequentially whereas the vertical direction handled recursively.
	 *  Compared to fully recursive approach, this technique preserves the speed but offers smaller consumption of memory.
	 */
	void scanHorzProbeVert(int i, int j) {

		if (!isValidPixel(i,j))
			return;

		visit(i,j); // mark & update

		const int i0 = i;
		const int j0 = j;

		/// Scan right. Note than i may wrap beyond image width.
		int iEnd;
		while (move<1,0>(i,j)){
			visit(i,j);
		}
		iEnd = i;
		// Now i2 is the maximum valid i coordinate.

		// Rewrite code: iMax should support wrapping, so 200...256...10.

		/// Scan left
		i=i0;
		j=j0;
		while (move<-1,0>(i,j)){
			visit(i, j);
		}
		// Now i is the minimum valid i coordinate.


		/// Scan again, continuing one step above and below.
		int i2, j2;
		bool done = false;
		//while (i != iEnd){
		while (!done){

			done = (i == iEnd);

			i2 = i;
			j2 = j-1;
			if (control.handler.validate(i2, j2)){
				if (isValidMove(i,j, i2,j2))
					scanHorzProbeVert(i2,j2);
			}

			i2 = i;
			j2 = j+1;
			if (control.handler.validate(i2, j2)){
				if (isValidMove(i,j, i2,j2))
					scanHorzProbeVert(i2,j2);
			}

			++i;
			control.handler.validate(i, j); // check?

		}; // while (i != iEnd);

	}


	void scanVertProbeHorz(int i, int j) {

		if (!isValidPixel(i,j))
			return;

		visit(i,j); // mark & update

		const int i0 = i;
		const int j0 = j;

		/// Scan DOWN. Note than j may wrap beyond image height.
		int jEnd;
		while (move<0,1>(i,j)){
			visit(i,j);
		}
		jEnd = j;
		// Now jEnd is the last (~maximum) valid i coordinate.

		// Rewrite code?: iMax should support wrapping, so 200...256...10.

		/// Scan UP
		i=i0;
		j=j0;
		while (move<0,-1>(i,j)){
			visit(i, j);
		}
		// Now j is the first (~minimum) valid j coordinate.


		/// Scan again, continuing one step above and below.
		int i2, j2;
		bool done = false;
		while (!done){
			done = (j == jEnd);
			j2 = j;
			// Test left side
			i2 = i-1;
			if (control.handler.validate(i2, j2)){
				if (isValidMove(i,j, i2,j2))
					scanVertProbeHorz(i2,j2);
			}
			j2 = j;
			// Test right side
			i2 = i+1;
			if (control.handler.validate(i2, j2)){
				if (isValidMove(i,j, i2,j2))
					scanVertProbeHorz(i2,j2);
			}
			++j;
			control.handler.validate(i, j); // check?
		};

	}

};



template <class S, class D, class C>
std::ostream & operator<<(std::ostream & ostr, const SegmentProber<S,D,C> & prober){
	/*
	ostr << "value="     << (float)prober.value << ',';
	ostr << "anchorMin=" << (float)prober.anchorMin << ',';
	ostr << "anchorMax=" << (float)prober.anchorMax << ',';
	ostr << "size="      << (float)prober.size << ',';
	*/
	ostr << "conf: " << prober.conf << ", ";
	//ostr << "width="     << (float)prober.width << ',';
	//ostr << "height="    << (float)prober.height << ',';
	ostr << "handler: "   << prober.control.handler << ',';
	//ostr << "p="         << prober.p;
	return ostr;
}




class FillProber : public SegmentProber<int,int,SegmentProberConf<int, int> > {

public:

	FillProber(const Channel &s, Channel &d) : SegmentProber<int,int,SegmentProberConf<int, int> >(s, d), count(0) {};

	FillProber(const Channel &s) : SegmentProber<int,int,SegmentProberConf<int, int> >(s), count(0) {};

	virtual inline
	void visit(int i, int j) {
		count++;
		update(i, j);
		dst->put(i,j, conf.markerValue);
	}

	/// Experimental
	/**
	 */
	virtual inline
	bool isVisited(int i, int j) const {
		return (dst->get<dst_t>(i,j) == conf.markerValue);
	}

	virtual inline
	bool isValidSegment(int i, int j) const {
		return conf.isValidIntensity(src.get<src_t>(i,j));
	}

	size_t count;

};


class SizeProber : public SegmentProber<int,int,SegmentProberConf<int, int> > {

public:

	SizeProber(const Channel &s, Channel &d) : SegmentProber<int,int,SegmentProberConf<int, int> >(s, d), size(0) {};

	virtual inline
	void clear(){
		size = 0;
	};


	virtual inline
	void visit(int i, int j) {
		update(i, j);
		dst->put(i,j, conf.markerValue);
	}

	/// Experimental
	/**
	 */
	virtual inline
	bool isVisited(int i, int j) const {
		return (dst->get<dst_t>(i,j) > 0);
	}


	///  Application dependent operation performed in each segment location (i,j).
	virtual	inline
	void update(int i, int j){
		++size;
	}

	virtual inline
	bool isValidSegment(int i, int j) const {
		return conf.isValidIntensity(src.get<src_t>(i,j));
	}

	mutable size_t size; // Consider deriving SegmentProber => SegmentAreaProber


};




} // image::
} // drain::

#endif /* SEGMENT_PROBER_H_ */



