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

#include "CopyOp.h"
#include "Window.h"
//#include "SlidingWindowOp.h"

namespace drain
{

namespace image
{

template <class C = WindowConfig>
class SlidingWindow : public Window<C>
{
public:


	SlidingWindow(int width=0, int height=0) : Window<C>(width,height) {}; // , resetAtEdges(false)

	SlidingWindow(const C & conf) : Window<C>(conf) {}; // , resetAtEdges(false)

	virtual ~SlidingWindow(){};


	/// Sets coord handler, calls initialise, sets pos(0,0), fills, writes and slides.
	/**
	 *  Notice that this top-level function is final.
	 */
	void slide(){

		drain::MonitorSource mout(iMonitor, "SlidingWindow", __FUNCTION__);

		this->coordinateHandler.setPolicy(this->src.getCoordinatePolicy());
		this->coordinateHandler.setLimits(this->src.getWidth(), this->src.getHeight());

		mout.debug(2) << "initialize." << mout.endl;
		this->location.setLocation(0,0);
		initialize();

		mout.debug(2) << (*this) << mout.endl;

		//this->location.setLocation(0,0);
		fill();
		write();

		if (this->isHorizontal())
			slideHorz();
		else
			slideVert();
	}


	virtual
	void traverse(){
		drain::MonitorSource mout(iMonitor, "SlidingWindow", __FUNCTION__);
		mout.warn() << "Using traverse() recommended for debugging only." << mout.endl;
		Window<C>::traverse();
	}

	/// Write the result in the target image.
	/** Sliding windows enjoys a confidence of the application,
	 *  as it has the
	 */
	virtual
	void write() = 0; //{dst.at(location) = value;};



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
		fill();
		return true;
	};

	/// High-level functionality of a sliding window.
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

	/// High-level functionality of a sliding window.
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


	Point2D<int> locationLead;
	Point2D<int> locationTrail;

protected:


	/// Moves one pixel down. Stops at the edge, and returns false.
	/**
	 *  High-level functionality of a sliding window.
	 *
	 *  @return true, if the new location is within image, otherways false.
	 */
	inline // FINAL
	bool moveDown(){

		++this->location.y;
		if (this->location.y < this->srcHeight){
			locationLead.y  = this->location.y + this->jMax;
			locationTrail.y = this->location.y + this->jMin-1;
			updateVert();
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
			locationLead.y  = this->location.y + this->jMin;
			locationTrail.y = this->location.y + this->jMax+1;
			updateVert();
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
		if (this->location.x < this->srcWidth){
			locationLead.x  = this->location.x + this->iMax;
			locationTrail.x = this->location.x + this->iMin-1;
			updateHorz();
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
			locationLead.x  = this->location.x + this->iMin;
			locationTrail.x = this->location.x + this->iMax+1;
			updateHorz();
			return true;
		}
		else { // step back
			++this->location.x;
			return false;
		}
	}



	/// Clears the applied statistics.
	virtual
	void clear(){};

	/// Resets the window: initiates the statistics in the current location.
	/**
	 *  Initialises the statistics in the current \c location
	 */
	virtual
	inline
	void fill(){  // rename updateFull ?
		clear();
		for (int i = this->iMin; i <= this->iMax; i++) {
			for (int j = this->jMin; j <= this->jMax; j++) {
				locationTmp.setLocation(this->location.x+i, this->location.y+j);
				addPixel(locationTmp);
			}
		}
	}



	/// In moving horizontally, updates the window centered at current location. Calls removeTrailingPixel() and addLeadingPixel().
	virtual inline
	void updateHorz() {

		for (int j = this->jMin; j <= this->jMax; j++) {

			locationTmp.setLocation(locationTrail.x, this->location.y + j);
			removePixel(locationTmp);

			locationTmp.setLocation(locationLead.x, this->location.y + j);
			addPixel(locationTmp);

		}
	}

	/// In moving vertically, updates the window centered at current location. Calls removeTrailingPixel() and addLeadingPixel().
	virtual inline // final?
	void updateVert(){

		for (int i=this->iMin; i<=this->iMax; i++) {

			locationTmp.setLocation(this->location.x + i, locationTrail.y);
			removePixel(locationTmp);

			locationTmp.setLocation(this->location.x + i, locationLead.y);
			addPixel(locationTmp);

		}

	}

	/// For n x 1 sized windows
	virtual inline
	void updateHorzStripe(){

		locationTmp.setLocation(locationTrail.x, this->location.y);
		removePixel(locationTmp);

		locationTmp.setLocation(locationLead.x, this->location.y);
		addPixel(locationTmp);

	}

	/// For 1 x n sized windows
	virtual inline
	void updateVertStripe(){

		locationTmp.setLocation(this->location.x, locationTrail.y);
		removePixel(locationTmp);

		locationTmp.setLocation(this->location.x, locationLead.y);
		addPixel(locationTmp);
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
	void addPixel(Point2D<int> &p) = 0;

	/// Removes a pixel from window statistics. Unvalidated location.
	/**
	 *   Updates class-specific window statistics by removing a pixel value  - typically at the trailing edge of the window.
	 *
	 *   \param p - location at which contribution should be removed
	 *
	 *   The argument p is \em not validated in advance but
	 *   it should be checked within the implementation using coordinateHandler.validate(p), for example.
	 */
	virtual
	void removePixel(Point2D<int> &p) = 0;


//private:
protected:

	mutable Point2D<int> locationTmp;

};








/// A horizontal, one-dimensional SlidingWindow (Nx1).
/**
 *
 */
template <class C = WindowConfig>
class SlidingStripeHorz : public SlidingWindow<C>
{
public:

	SlidingStripeHorz(size_t width=1) {
		setSize(width,1);
	};

	void setSize(size_t n = 0){
		SlidingWindow<C>::setSize(n, 1);
	}


	/// Resets the window
	virtual
	inline
	void fill(){
		this->clear();
		for (int i = this->iMin; i <= this->iMax; i++) {
			this->locationTmp.setLocation(this->location.x+i, this->location.y);
			this->addPixel(this->locationTmp);
		}
	}


protected:


	virtual
	void setSize(size_t width, size_t height){
		if (height > 1)
			throw std::runtime_error("SlidingStripeHorz: height > 1");
		SlidingWindow<C>::setSize(width, 1);
	}

protected:

	virtual inline
	void updateHorz(){
		this->updateHorzStripe();
	}

	virtual inline
	void updateVert(){
		this->fill();
	}



};


/// A vertical one dimensional SlidingWindow (1xN).
/**
 *
 */
template <class C = WindowConfig>
class SlidingStripeVert : public SlidingWindow<C>
{
public:

	SlidingStripeVert(size_t height=1) {
		setSize(1, height);
	};

	void setSize(size_t n = 0){
		SlidingWindow<C>::setSize(1, n);
	}

	/// Resets the window
	virtual
	inline
	void fill(){
		this->clear();
		for (int j = this->jMin; j <= this->jMax; j++) {
			this->locationTmp.setLocation(this->location.x, this->location.y+j);
			this->addPixel(this->locationTmp);
		}
	}

protected:

	virtual
	void setSize(size_t width, size_t height){
		if (width > 1)
			throw std::runtime_error("SlidingStripeVert: width > 1");
		SlidingWindow<C>::setSize(1, height);
	}

protected:

	virtual inline
	void updateHorz(){
		this->fill();
	}

	virtual inline
	void updateVert(){

		this->updateVertStripe();

		/*
		// REMOVE
		locationHandled.setLocation(location.x, locationTrail.y);
		coordinateHandler.handle(locationHandled);
		removeTrailingPixel();

		// ADD
		locationHandled.setLocation(location.x, locationLead.y);
		coordinateHandler.handle(locationHandled);
		addLeadingPixel();
		*/
	}


};


}

}

#endif /*SLIDINGWINDOWOP_H_*/

// Drain
