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
#ifndef SLIDING_W_H_
#define SLIDING_W_H_

//#include "CopyOp.h"
#include "Window.h"
//#include "SlidingWindowOp.h"

namespace drain
{

namespace image
{

template <class C = WindowConfig, class R = WindowCore>
class SlidingWindow : public Window<C,R>
{
public:

	SlidingWindow(const C & conf, bool horzMultiple=true, bool vertMultiple=true) : Window<C,R>(conf){
		setSlidingMode(horzMultiple, vertMultiple);
	}; // , resetAtEdges(false)


	SlidingWindow(int width=0, int height=0, bool horzMultiple=true, bool vertMultiple=true) : Window<C,R>(width,height){
		setSlidingMode(horzMultiple, vertMultiple);
	};

public:

	virtual ~SlidingWindow(){};

	void setSlidingMode(bool horzMultiple, bool vertMultiple){

		modeStr = "??";
		modeStr.at(0) = horzMultiple?'H':'h';
		modeStr.at(1) = vertMultiple?'V':'v';

		updateHorz = horzMultiple ? &SlidingWindow<C,R>::updateHorzMultiple : &SlidingWindow<C,R>::updateHorzSingle;
		updateVert = vertMultiple ? &SlidingWindow<C,R>::updateVertMultiple : &SlidingWindow<C,R>::updateVertSingle;

		if (horzMultiple && vertMultiple)
			fill = & SlidingWindow<C,R>::fillBoth;
		else if (horzMultiple)
			fill = & SlidingWindow<C,R>::fillVert; //Horz;
		else if (vertMultiple)
			fill = & SlidingWindow<C,R>::fillHorz; // Vert;
		else {
			drain::Logger mout(getImgLog(), "SlidingWindow", __FUNCTION__);
			mout << "illegal sliding mode single-horz, single-vert" << mout.endl;
			fill = & SlidingWindow<C,R>::fillBoth;
		}

	}

	/// Sets coord handler, calls initialise, sets pos(0,0), fills, writes and slides.
	/**
	 *  Notice that this top-level function is final.
	 */
	void run(){

		drain::Logger mout(getImgLog(), "SlidingWindow", __FUNCTION__);

		mout.debug3("calling initialize" );
		initialize();

		//this->myFunctor.updateBean();
		mout.debug2((*this) );
		//mout.warn("final functor:" , this->myFunctor );
		/*
		for (int i=0; i<50; ++i){
			std::cerr << __FUNCTION__ << i << '\t' << this->myFunctor(i) << '\n';
		}
		*/

		(this->*fill)();
		mout.debug3("SCALE=" , (int)this->SCALE );
		write();

		if (this->isHorizontal()){
			mout.debug2("start slideHorz" );
			slideHorz();
			mout.debug2("end slideHorz" );
		}
		else {
			mout.debug2("slideVert" );
			slideVert();
		}
	}

	/// Sets coord handler, calls initialise, sets pos(0,0), fills, writes and slides.
	/**
	 *  Notice that this top-level function is FINAL.
	 */
	void runHorz(){

		drain::Logger mout(getImgLog(), "SlidingWindow", __FUNCTION__);
		mout.debug3("start" );

		mout.debug3("initialize" );
		initialize();

		mout.debug3((*this) );

		fill();
		mout.debug3("SCALE=" , (int)this->SCALE );
		write();

		mout .debug3() << "slideHorz" << mout.endl;
		slideHorz();
	}

	/// Sets coord handler, calls initialise, sets pos(0,0), fills, writes and slides.
	/**
	 *  Notice that this top-level function is FINAL.
	 */
	void runVert(){

		drain::Logger mout(getImgLog(), "SlidingWindow", __FUNCTION__);
		mout.debug3("start" );

		mout.debug3("initialize" );
		initialize();

		mout.debug3((*this) );

		fill();
		mout.debug3("SCALE=" , (int)this->SCALE );
		write();

		mout .debug3() << "slideVert" << mout.endl;
		slideVert();
	}



	virtual
	void debug(){
		drain::Logger mout(getImgLog(), "SlidingWindow", __FUNCTION__);
		mout.warn("Using apply() recommended for debugging only." );
		Window<C,R>::run();
	}

	/// Write the result in the target image.
	/** Sliding windows enjoys a confidence of the application,
	 *  as it has the
	 */
	virtual
	void write() = 0; //{dst.at(location) = value;};

	inline
	const std::string & getModeStr(){
		return modeStr;
	}

protected:


	/// Sets class-specific initial values. Does not change general window state (e.g. location). Should not accumulate any statistics.
	/**
	 *   \see fill()
	 *   \see clear()
	 */
	virtual
	void initialize() = 0;

	///  Returns false, if traversal should be ended.
	virtual
	inline
	bool reset(){
		(this->*fill)();
		return true;
	};

	/// High-level functionality of a sliding window. FINAL
	inline
	void slideHorz(){

		while (true){

			while (moveRight())
				write();

			if (!moveDown())
				return;

			write();

			while (moveLeft())
				write();

			if (!moveDown())
				return;

			write();

			// For 1) cleaning numerical residues and 2) updating window parameters
			if (this->resetAtEdges){
				if (!this->reset())
					return;
			}

		}

	}

	/// High-level functionality of a sliding window. FINAL
	inline
	void slideVert(){

		while (true){

			while (moveDown())
				write();

			if (!moveRight())
				return;

			write();

			while (moveUp())
				write();

			if (!moveRight())
				return;

			write();

			// For 1) cleaning numerical residues and 2) updating window parameters
			if (this->resetAtEdges){
				if (!this->reset())
					return;
			}


		}

	}


private:

	std::string modeStr;
	//bool horzMultiple;
	//bool vertMultiple;


protected:

	Point2D<int> locationLead;
	Point2D<int> locationTrail;


	/// Moves one pixel down. Stops at the edge, and returns false.
	/**
	 *  High-level functionality of a sliding window.
	 *
	 *  @return true, if the new location is within im		mout.debug3(window );
	 *  age, otherways false.
	 */
	inline // FINAL
	bool moveDown(){

		++this->location.y;
		if (this->location.y <= this->coordinateHandler.getYRange().max){ //  < this->srcHeight){
			locationLead.y  = this->location.y + this->jRange.max;
			locationTrail.y = this->location.y + this->jRange.min - 1;
			(this->*updateVert)();
			return true;
		}
		else { // step back
			--this->location.y;
			return false;
		}
	}

	/// Moves one pixel up. Stops at the edge, and returns false.
	/**
	 *   High-level functionality of a sliding window.
	 *
	 *  @return true, if the new location is within image, otherways false.
	 */
	inline // FINAL
	bool moveUp(){

		--this->location.y;
		if (this->location.y >= 0){
			locationLead.y  = this->location.y + this->jRange.min;
			locationTrail.y = this->location.y + this->jRange.max + 1;
			(this->*updateVert)();
			return true;
		}
		else { // step back
			++this->location.y;
			return false;
		}
	}

	/// Moves one pixel right. Stops at the edge, and returns false.
	/**
	 *  High-level functionality of a sliding window.
	 *
	 *  @return true, if the new location is within image, otherways false.
	 */
	inline // FINAL
	bool moveRight(){

		++this->location.x;
		if (this->location.x <= this->coordinateHandler.getXRange().max){ // this->srcWidth){
			locationLead.x  = this->location.x + this->iRange.max;
			locationTrail.x = this->location.x + this->iRange.min-1;
			(this->*updateHorz)();
			return true;
		}
		else { // step back
			--this->location.x;
			return false;
		}

	}

	/// Moves one pixel left. Stops at the edge, and returns false.
	/**
	 *  High-level functionality of a sliding window.
	 *
	 *  @return true, if the new location is within image, otherways false.
	 */
	inline // FINAL
	bool moveLeft(){

		--this->location.x;
		if (this->location.x >= 0){
			locationLead.x  = this->location.x + this->iRange.min;
			locationTrail.x = this->location.x + this->iRange.max+1;
			(this->*updateHorz)();
			return true;
		}
		else { // step back
			++this->location.x;
			return false;
		}
	}



	/// Clears the applied statistics. Redefined in derived classes.
	virtual
	void clear(){
		std::cerr << __FUNCTION__ << " (plain) " << std::endl;
	};


	/// Clears and computes the statistics for the current location.
	/**
	 *  Initialises the statistics in the current \c locationv.W
	 *
	 */
	virtual // ?
	inline
	void fillBoth(){
		//drain::Logger mout(getImgLog(), "SlidingWindow", __FUNCTION__);
		this->clear();
		//mout.warn("init window" );
		for (int i = this->iRange.min; i <= this->iRange.max; i++) {
			for (int j = this->jRange.min; j <= this->jRange.max; j++) {
				this->locationTmp.setLocation(this->location.x+i, this->location.y+j);
				this->addPixel(this->locationTmp);
			}
		}
	}

	/// Clears and computes the statistics for the current location. FINAL
	/**
	 *  Note: the direction (horz) describes the orientation of the window stripe, not the direction of motion.
	 *
	 */
	inline
	void fillHorz(){
		this->clear();
		for (int i = this->iRange.min; i <= this->iRange.max; i++) {
			this->locationTmp.setLocation(this->location.x+i, this->location.y);
			this->addPixel(this->locationTmp);
		}
	}

	/// Clears and computes the statistics for the current location. FINAL
	/**
	 *  Note: the direction (vert) describes the orientation of the window stripe, not the direction of motion.
	 *
	 */
	inline
	void fillVert(){
		this->clear();
		for (int j = this->jRange.min; j <= this->jRange.max; j++) {
			this->locationTmp.setLocation(this->location.x, this->location.y+j);
			this->addPixel(this->locationTmp);
		}
	}

	/// Pointer to fill operation preformed at initial location (0,0)
	/**
	 *  Initially set to fillMultiple()
	 */
	void (SlidingWindow<C,R>::* fill)(); //

	/// Pointer to update function invoked at each horizontal move.
	/**
	 *  Note: the direction (horz) describes window motion, not the orientation of the window stripe.
	 *  Only a single element may be handled, like in the case of FastAvergaeOp ( SlidingWindowStripe ).
	 *
	 *  Initially set to updateHorzMultiple()
	 */
	void (SlidingWindow<C,R>::* updateHorz)(); // initially

	/// Pointer to update function invoked at each vertical move.
	/**
	 *  Note: the direction (vert) describes window motion, not the orientation of the window stripe.
	 *  Only a single element may be handled, like in the case of FastAvergaeOp ( SlidingWindowStripe ).
	 *
	 *  Initially set to updateVertMultiple()
	 */
	void (SlidingWindow<C,R>::* updateVert)();


	/// In moving horizontally, updates the window centered at current location. Calls removePixel() and addPixel().
	void updateHorzMultiple() {

		for (int j = this->jRange.min; j <= this->jRange.max; j++) {

			locationTmp.setLocation(locationTrail.x, this->location.y + j);
			this->removePixel(locationTmp);

			locationTmp.setLocation(locationLead.x, this->location.y + j);
			this->addPixel(locationTmp);

		}
	}

	void updateHorzSingle(){

		this->locationTmp.setLocation(this->locationTrail.x, this->location.y);
		this->removePixel(this->locationTmp);

		this->locationTmp.setLocation(this->locationLead.x, this->location.y);
		this->addPixel(this->locationTmp);

	}

	/// In moving vertically, updates the window centered at current location. Calls removePixel() and addPixel().
	void updateVertMultiple(){

		for (int i=this->iRange.min; i<=this->iRange.max; i++) {

			locationTmp.setLocation(this->location.x + i, locationTrail.y);
			this->removePixel(locationTmp);

			locationTmp.setLocation(this->location.x + i, locationLead.y);
			this->addPixel(locationTmp);

		}

	}

	/// For 1 x n sized windows
	// virtual inline
	void updateVertSingle(){

		this->locationTmp.setLocation(this->location.x, this->locationTrail.y);
		this->removePixel(this->locationTmp);

		this->locationTmp.setLocation(this->location.x, this->locationLead.y);
		this->addPixel(this->locationTmp);

	}

	/// Adds a pixel to window statistics. Unvalidated location.
	/**
	 *   Updates class-specific window statistics by adding a pixel value - typically at the leading edge of the window.
	 *
	 *   \param p - location at which contribution should be removed
	 *
	 *   The argument p is \em not validated in advance but
	 *   it should be checked within the implementation using coordinateHandler.validate(p), for example.
	 */
	virtual
	void addPixel(Point2D<int> &p) = 0;  // consider addPixel(Point2D<int> &p, int index/double weight)

	/// Removes a pixel from window statistics. Unvalidated location.
	/**
	 *   Updates class-specific window statistics by removing a pixel value  - typically at the trailing edge of the window.
	 *
	 *   \param p - location at which contribution should be removed
	 *	const int h = (conf.height>0.0) ? conf.height : conf.width;
	//GaussianStripeVert window2(h, 0.5*conf.radius*static_cast{double}(h));
	GaussianStripe2<false> window2(h, 0.5*conf.radius*static_cast{double}(h));
	 *
	 *   The argument p is \em not validated in advance but
	 *   it should be checked within the implementation using coordinateHandler.validate(p), for example.
	 */
	virtual
	void removePixel(Point2D<int> &p) = 0; // consider addPixel(Point2D<int> &p, int index/double weight)


	//private:
protected:

	mutable Point2D<int> locationTmp;

};






/// A horizontal, one-dimensional SlidingWindow (Nx1).
/**
 *  \tparam DIR - direction: horizontal(true) or vertical(false)
 *  \tparam C - WindowConfig
 *  \tparam R - WindowCore
 */
template <class C = WindowConfig, class R = WindowCore, bool DIR=true>
class SlidingStripe : public SlidingWindow<C,R> {
public:

	SlidingStripe(size_t n=1) : SlidingWindow<C,R>(DIR?n:1, DIR?1:n, !DIR, DIR) {
	};

	virtual
	~SlidingStripe(){};

	void setSize(size_t width = 1){
		this->SlidingWindow<C,R>::setSize(width, 1);
	}

protected:

	virtual
	void setSize(size_t width, size_t height){

		drain::Logger mout(getImgLog(), "SlidingStripe", __FUNCTION__);
		//if (height > 1)
		//	mout.warn("horz stripe, height(" , height , ") discarded" );
		//SlidingWindow<C,R>::setSize(width, 1);
		if (DIR){
			if (height > 1)
				mout.warn("horz stripe, height(" , height , ") discarded" );
			SlidingWindow<C,R>::setSize(width, 1);
		}
		else {
			if (width > 1)
				mout.warn("vert stripe, width("  , width , ") discarded" );
			SlidingWindow<C,R>::setSize(1, height);
		}

	}

};




/// A horizontal, one-dimensional SlidingWindow (Nx1).
/**
 *  \tparam C - WindowConfig
 *  \tparam R - WindowCore
template <class C = WindowConfig, class R = WindowCore>
typedef SlidingStripe<true,C,R> SlidingStripeHorz;
 */

/// A horizontal, one-dimensional SlidingWindow (Nx1).
/**
 *  \tparam C - WindowConfig
 *  \tparam R - WindowCore
template <class C = WindowConfig, class R = WindowCore>
typedef SlidingStripe<false,C,R> SlidingStripeVert;
 */



}

}

#endif /*SLIDINGWINDOWOP_H_*/

// Drain
