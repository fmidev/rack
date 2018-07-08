/**

    Copyright 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef DRAIN_IMAGE_TRAY
#define DRAIN_IMAGE_TRAY


#include <stdexcept>

#include "Image.h"
#include "ImageView.h"

namespace drain {

namespace image {

// Under construction. To be introduced.

/// Set of images, usable for general calls of type traverseFrame(src, dst) .
/**
 *  \tparam T - ImageFrame or Image, const or non-const.
 */
template <class T>
class Tray : public std::map<size_t, T &> {

public:

	typedef T image_t;
	typedef std::map<size_t, image_t &> map_t;

	// todo getImageFrames, getAlphaFrames
	Tray(){};

	Tray(const Tray<image_t> &t){}; // copy?

	//Tray(std::vector<image_t> & v){
	//	append(v);
	//};

	/*
	Tray(image_t & image){
		append(image);
	};
	*/

	virtual
	~Tray(){};



	/// Returns the i'th image
	inline
	const image_t & get(size_t i = 0) const {
		typename map_t::const_iterator it = this->find(i);
		if (it == this->end()){
			std::runtime_error("Tray::get(i): find failed");
		}
		return it->second;
	}

	/// Returns the i'th image
	inline
	image_t & get(size_t i = 0){
		typename map_t::iterator it = this->find(i);
		if (it == this->end()){
			std::runtime_error("Tray::get(i): find failed");
		}
		return it->second;
	}


	/// Replace image in position i.
	// TODO: consider swapping parameter order
	inline
	void set(image_t & img, size_t i=0){
		this->insert(typename map_t::value_type(i, img));
	}

	/// Add image to the end.
	inline virtual
	void appendImage(image_t & img){
		this->insert(typename map_t::value_type(this->size(), img));
	}

	/// Add image sequence. Replaces old values.
	template <class T2>
	inline
	void copy(const Tray<T2> &t){
		this->clear();
		for (typename Tray<T2>::const_iterator it=t.begin(); it!=t.end(); ++it){
			appendImage(it->second);
		}
	}

	/// Add image sequence.
	template <class T2>
	inline //virtual
	void copyVector(std::vector<T2> & v){
		this->clear();
		for (typename std::vector<T2>::iterator it = v.begin(); it != v.end(); ++it) {
			appendImage(*it);
		}
	}

	/// Returns the geometry of the first frame.
	virtual
	const Geometry & getGeometry() const {
		geometry.setGeometry(this->get().getGeometry());
		geometry.setChannelCount(this->size());
		return geometry;
	}

	/// Returns true, if all the frames have the same width and height.
	virtual
	bool checkGeometry(const Geometry & g) const {
		if (this->empty()){
			return false;
		}
		for (typename Tray<T>::const_iterator it=this->begin(); it!=this->end(); ++it){
			if (g != it->second.getGeometry())
				return false;
		}
		return true;
	}

	/// Returns true, if all the frames have the same width and height.
	bool checkGeometry() const {
		if (this->empty()){
			return false;
		}
		return checkGeometry(this->get().getGeometry());
	}




	/// Put intensity vector
	// Applied by?
	template <class T2>
	inline
	void putPixel(const Point2D<int> & p, const std::vector<T2> & v){
		typename map_t::iterator it=this->begin();
		typename std::vector<T2>::const_iterator vit = v.begin();
		while (it!=this->end()){
			it->second.put(p, *vit);
			++it;
			++vit;
		}
	}

	/// Get intensity vector
	// Applied by?
	template <class T2>
	inline
	void getPixel(const Point2D<int> & p, std::vector<T2> & v) const {
		typename std::vector<T2>::iterator vit = v.begin();
		typename map_t::const_iterator it=this->begin();
		while (it!=this->end()){
			*vit = it->second.template get<T2>(p);
			++it;
			++vit;
		}
	}

	virtual inline
	void adjustCoordinateHandler(CoordinateHandler2D & handler) const {

		Logger mout("Tray<T>", __FUNCTION__);

		if (this->empty()){
			mout.warn() << "empty (no frames), leaving coordHandler intact" << mout.endl;
			return;
		}

		if (!this->checkGeometry()){
			mout.warn() << "non-uniform geometry:" <<  mout.endl;
			mout.note() << *this       << mout.endl;
		}

		const Geometry & g = getGeometry();
		handler.setLimits(g.getWidth(), g.getHeight());
		handler.setPolicy(get().getCoordinatePolicy()); // ~policy of the first one

	};

	template <class T2>
	bool hasOverlap(const Tray<T2> & tray) const {
		for (typename map_t::const_iterator it=this->begin(); it!=this->end(); ++it){
			for (typename Tray<T2>::map_t::const_iterator it2=tray.begin(); it2!=tray.end(); ++it2){
				if (it->second.hasOverlap(it2->second)){
					//std::cerr << __FUNCTION__ << ':' << it->first << '=' << it2->first << '\n';
					return true;
				}
			}
		}
		return false;
	}

	inline // virtual? (would reveal alpha?)
	void toOStr(std::ostream & ostr) const {
		for (typename map_t::const_iterator it=this->begin(); it!=this->end(); ++it){
			ostr << it->first << ':' << it->second << '\n';
		}
	}

	void createVector(std::vector<Image> & v) const {
		for (typename map_t::const_iterator it=this->begin(); it!=this->end(); ++it){
			//v.insert(v.end(), Image(it->second.getType(), it->second.getGeometry()));
			v.insert(v.end(), Image(it->second.getType()));
		}
	}

protected:

	mutable Geometry geometry;


};


template <class T>
inline
std::ostream & operator<<(std::ostream &ostr, const Tray<T> & tray){
	ostr << "Tray " << tray.getGeometry() << ":\n";
	tray.toOStr(ostr);
	return ostr;
}



/// Container applicable for Channels and Images, with alpha support.
/**
 *  \tparam T - ImageFrame or Image, const or non-const.
 */
template <class T>  // TODO: rename to ImagePack2 etc. because valid also for Image's, not only for Channel's
class ImageTray : public Tray<T> {

public:

	// "Inherit" types.
	typedef typename Tray<T>::image_t image_t;
	typedef typename Tray<T>::map_t     map_t;

	Tray<T> alpha;

	// todo getChannels, getAlphaFrames
	ImageTray(){};

	template <class T2>
	inline
	ImageTray(const ImageTray<T2> &t){
		copy(t);
	}

	template <class T2>
	inline
	ImageTray(ImageTray<T2> &t){
		copy(t);
	}

	/// Appends the whole single image.
	/*
	ImageTray(image_t & img){
		this->set(img);
	}
	*/

	/// Split image and alpha image to channels. This is different from Tray(image_t &) which appends the whole single image.
	/**
	 *  \param image - image object, the image channels of which will referenced.
	 *  \param alpha - image object, the image channels of which will referenced as alpha channels.
	 *  Notice: if first argument contains alpha channels, they will be appended before those given as second argument (alpha).
	 */
	/*
	ImageTray(image_t & img, image_t & alpha){
		this->set(img);
		this->setAlpha(alpha);
	}
	*/

	//ImageTray(std::vector<image_t> & v) : Tray<T>(v){
	//}

	const Geometry & getGeometry() const {
		this->geometry.setGeometry(this->get().getGeometry());
		this->geometry.setChannelCount(this->size(), this->alpha.size());
		return this->geometry;
	}

	inline
	bool hasAlpha() const {
		return !alpha.empty();
	}


	/// Returns the i'th alpha image
	inline
	const image_t & getAlpha(size_t i = 0) const {
		typename map_t::const_iterator it = alpha.find(i);
		if (it == alpha.end()){
			std::runtime_error("ChannelTray::getAlpha(i): find failed");
		}
		return it->second;
	}


	void clear(){
		Tray<T>::clear();
		this->alpha.Tray<T>::clear();
	}

	/// Add image sequence.
	template <class T2>
	inline
	void copy(const Tray<T2> &t){
		Tray<T>::copy(t);
	}

	/// Add image sequence.
	template <class T2>
	inline
	void copy(Tray<T2> &t){
		Tray<T>::copy(t);
	}

	template <class T2>
	inline
	void copy(ImageTray<T2> &t){
		this->Tray<T>::copy(t);
		this->alpha.Tray<T>::copy(t.alpha);
	}

	template <class T2>
	inline
	void copy(const ImageTray<T2> &t){
		Tray<T>::copy(t);
		alpha.Tray<T>::copy(t.alpha);
	}


	/// Replace image in position i.
	inline
	void setAlpha(image_t & img, size_t i = 0){
		this->alpha.insert(typename map_t::value_type(i, img));
	}

	/// Returns the i'th alpha image
	inline
	image_t & getAlpha(size_t i = 0){
		typename map_t::iterator it = alpha.find(i);
		if (it == alpha.end()){
			std::runtime_error("ChannelTray::getAlpha(i): find failed");
		}
		return it->second;
	}


	/// Add image to the end of the container.
	inline
	void appendAlpha(image_t & img){
		setAlpha(img, this->alpha.size());
	}


	/// Splits
	/**
	 *   \tparam -
	 */
	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	inline
	void setChannels(T2 & img){
		takeImageChannels(*this, img);
		takeAlphaChannels(alpha, img);
	}

	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	inline
	void setChannels(const T2 & img){
		takeImageChannels(*this, img);
		takeAlphaChannels(alpha, img);
	}

	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	inline
	void setChannels(T2 & img, T2 & alphaImg){
		takeImageChannels(*this, img);
		takeImageChannels(alpha, alphaImg);
	}

	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	inline
	void setChannels(const T2 & img, const T2 & alphaImg){
		takeImageChannels(*this, img);
		takeImageChannels(alpha, alphaImg);
	}

	/// Splits
	/**
	 *   \tparam -
	 */
	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	inline
	void setAlphaChannels(T2 & img){
		takeImageChannels(alpha, img);
	}

	/// Splits
	/**
	 *   \tparam -
	 */
	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	inline
	void setAlphaChannels(const T2 & img){
		takeImageChannels(this->alpha, img);
	}



	inline
	void toOStr(std::ostream & ostr = std::cout) const {
		//ostr << "ImageTray " << this->getGeometry() << ":\n";
		Tray<T>::toOStr(ostr);
		if (this->hasAlpha()){
			ostr << "alphas: " << std::endl;
			alpha.Tray<T>::toOStr(ostr);
		}
	}

protected:

	/// Splits
	/**
	 *   \tparam tray-
	 */
	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	static inline
	void takeImageChannels(Tray<T> & tray, T2 & img){
		tray.clear();
		for (size_t i = 0; i < img.getImageChannelCount(); ++i) {
			tray.appendImage(img.getChannel(i));
		}
	}

	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	static inline
	void takeAlphaChannels(Tray<T> & tray, T2 & img){
		tray.clear();
		for (size_t i = 0; i < img.getAlphaChannelCount(); ++i) {
			tray.appendImage(img.getAlphaChannel(i));
		}
	}

	/// Splits
	/**
	 *   \tparam -
	 */
	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	static inline
	void takeImageChannels(Tray<const T> & tray, const T2 & img){
		tray.clear();
		for (size_t i = 0; i < img.getImageChannelCount(); ++i) {
			tray.appendImage(img.getChannel(i));
		}
	}

	template <class T2> // consider copyVector(v, channelGeometry) with ChannelGeometry
	static inline
	void takeAlphaChannels(Tray<const T> & tray, const T2 & img){
		tray.clear();
		for (size_t i = 0; i < img.getAlphaChannelCount(); ++i) {
			tray.appendImage(img.getAlphaChannel(i));
		}
	}


};


template <class T>
inline
std::ostream & operator<<(std::ostream &ostr, const ImageTray<T> & tray){
	ostr << "ChannelTray " << tray.getGeometry() << ":\n";
	tray.toOStr(ostr);
	ostr << "*\n";
	return ostr;
}





/// Container for a single image and distinct alpha channel(s).
/**
 *  \tparam T - ImageFrame or Image, const or non-const.
 */
/*
template <class T>
class Clip : public Tray<T> {

public:

	// "Inherit" types.
	typedef typename Tray<T>::image_t image_t;
	typedef typename Tray<T>::map_t     map_t;

	Tray<T> alpha; // what if always Frame?

	Clip(){};

	Clip(const Clip<image_t> &t){};

	/// Set image
	/ **
	 *  \param image - image object to be referenced
	 *  Notice: if image contains alpha channels, they will \e not be separated.
	 * /
	Clip(image_t & image){
		this->set(0, image);
	}

	/// Set image and alpha image.
	/ **
	   \param image - image object, the image channels of which will referenced.
	   \param alpha - image object, the image channels of which will referenced as alpha channels.
	   Notice: if first argument contains alpha channels, they will \e not be separated.

	 TODO: consider if empty alpha?
	 * /
	Clip(image_t & image, image_t & alpha){
		this->set(0, image);
		setAlpha(0, alpha);
	}

	virtual
	~Clip(){};

	///
	const Geometry & getGeometry() const {
		geometry.setGeometry(this->get().getGeometry());
		geometry.setChannelCount(this->size(), this->alpha.size());
		return geometry;
	}
	// TODO consider setGeometry


	/// Add image to the end of the container.
	/ *
	inline
	void appendAlpha(image_t & image){
		alpha.insert(typename map_t::value_type(alpha.size(), image));
	}
	* /

	inline
	bool hasAlpha() const {
		return !alpha.empty();
	}


	/// Returns the i'th alpha image
	// TODO: if no explicit alpha, return images alpha?
	// what about frame?
	inline
	const image_t & getAlpha(size_t i = 0) const {
		typename map_t::const_iterator it = alpha.find(i);
		if (it == alpha.end()){
			std::runtime_error("ImageTray::getAlpha(i): find failed");
		}
		return it->second;
	}


	/// Returns the i'th alpha image
	inline
	image_t & getAlpha(size_t i = 0){
		typename map_t::iterator it = alpha.find(i);
		if (it == alpha.end()){
			std::runtime_error("ImageTray::getAlpha(i): find failed");
		}
		return it->second;
	}


	inline
	void toOStr(std::ostream & ostr) const {
		Tray<T>::toOStr(ostr);
		ostr << "alphas:\n";
		for (typename Tray<T>::map_t::const_iterator it=alpha.begin(); it!=alpha.end(); ++it){
			ostr << '\t' << it->first << ':' << it->second << '\n';
		}
	}

protected:

	/// Replace image in position i.
	inline
	void setAlpha(size_t i, image_t & image){
		alpha.insert(typename map_t::value_type(i, image));
	}

	/// Add image to the end.
	inline
	virtual
	void append(image_t & image){
		throw std::runtime_error("Clip::append defunct");
		//this->insert(typename map_t::value_type(this->size(), image));
	}

	mutable Geometry geometry;

};
*/

// todo: lower append() ?

/*
class ImageClip : public Clip<Image> {

public:
	void setGeometry(){}; // TODO
};
*/

/// Set of images, usable for general calls of type traverseFrame(src, dst) .
/**
 *  \tparam T - ImageFrame or Image, const or non-const.
 */



}  // image
}  // drain





#endif
