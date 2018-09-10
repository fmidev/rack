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
#ifndef ODIM_STRUCT
#define ODIM_STRUCT

#include <ostream>
#include <cmath>
#include <string>
#include <set>
#include <algorithm>
//#include <drain/util/Options.h>
#include <drain/util/ReferenceMap.h>
#include <drain/util/Rectangle.h>
#include <drain/util/Time.h>

#include "hi5/Hi5.h"
#include "radar/Constants.h"
#include "BaseODIM.h"

namespace rack {


/**
 *  Within each ODIM class, each variable should have
 *
 *  -# name
 *  -# type
 *  -# group/name
 *  -# dataset/group/name
 *
 *  The scope for variables is "down to the data array". Ie.
 *
 *  Common interface:
 *
 *  -# operator[](toStr)
 *  -#
 *
 *  Class specific members:
 *
 *  -# native variables (double, long int, std::string)
 *
 *  Typically used in creating and writing a product.
 *  See also: LinearScaling (could be used as base class?)
 */
class EncodingODIM : public BaseODIM, public drain::ReferenceMap {

public:

	typedef unsigned char group_t;
	/*
	static const group_t NONE = 0;
	static const group_t ROOT = 1;
	static const group_t DATASET = 2;
	static const group_t DATA = 4; // or quality
	static const group_t ALL = (ROOT | DATASET | DATA);
	*/
	// data
	/*
	static const group_t ROOT = 0;
	static const group_t IS_INDEXED = 1;
	static const group_t DATASET = 2 | IS_INDEXED;
	static const group_t DATA    = 4 | IS_INDEXED; // or quality
	static const group_t QUALITY = 8 | DATA | IS_INDEXED; // or quality
	static const group_t ARRAY = 16;
	static const group_t OTHER = 32; // tmp, user defined, etc.
	static const group_t OTHER_INDEXED = (OTHER | IS_INDEXED); // tmp, user defined, etc.
	static const group_t ALL = (ROOT | DATASET | DATA | QUALITY);
	static const group_t NONE = 128;
	*/

	inline
	EncodingODIM(group_t initialize = ALL){
		init(initialize);
	};

	inline  // todo raise initFromMap (was: FromODIM)
	EncodingODIM(const EncodingODIM & odim){
		init(ALL);
		updateFromMap(odim); // importMap can NOT be used because non-EncodingODIM arg will have a larger map
	};


	inline
	EncodingODIM & operator=(const EncodingODIM & odim) {
		// std::cerr << "EncodingODIM & operator=" << std::endl;
		updateFromMap(odim);
		return *this;
	}



	/// This is non-standard (not in ODIM), but a practical means of handling storage type of datasets.
	//  See drain::Type.
	std::string type;

	/// data[n]/what (obligatory)
	double gain;
	double offset;
	double nodata;
	double undetect;

	EncodingODIM & setScaling(double gain, double offset = NAN);

	EncodingODIM & setScaling(double gain, double offset, double undetect, double nodata);

	inline
	bool isSet() const {
		return ((gain != 0.0) && (!type.empty()) && (type.at(0) != '*'));
	}

	/// Checks if data encoding is similar (storage type, gain, offset, undetect and nodata are the same).
	// todo: bool operator==(const EncodingODIM & odim);
	static
	inline
	bool haveSimilarEncoding(const EncodingODIM & odim1, const EncodingODIM & odim2){
		return  (odim1.type == odim2.type) &&
				(odim1.gain == odim2.gain) &&
				(odim1.offset == odim2.offset) &&
				(odim1.undetect == odim2.undetect) &&
				(odim1.nodata == odim2.nodata)
				;
	};

	void setRange(double min, double max);  // todo rename setRange

	/// Sets gain=1, offset=0, undetect=type_min, nodata=type_max. Note: sets type only if unset.
	template <class T>
	inline
	void setTypeDefaults(const T & type, const std::string & values = ""){

		drain::Logger mout("EncodingODIM", __FUNCTION__);

		drain::Type t(type);
		const char typechar = drain::Type::getTypeChar(t);
		if (this->type.empty())
			this->type = typechar;
		else {
			if (this->type.at(0) != typechar)
				mout.warn() << "different types: " << this->type << '/' << typechar << mout.endl;
		}

		gain = 1.0;
	    offset = 0.0;

	    //if (!type.empty()){ // ?
	    if (typechar != '*'){
	    	undetect = drain::Type::call<drain::typeMin, double>(t); //drain::Type::getMin<double>(t);
	    	nodata   = drain::Type::call<drain::typeMax, double>(t); // drain::Type::call<drain::typeMax,double>(t);
	    }
	    else {
	    	undetect = drain::Type::call<drain::typeMin, double>(this->type); //drain::Type::getMin<double>(this->type);
	    	nodata   = drain::Type::call<drain::typeMax, double>(this->type); //drain::Type::call<drain::typeMax,double>(this->type);
	    }

		setValues(values);

	}

	inline
	void setTypeDefaults(){
		setTypeDefaults(this->type);
	}


	/// Converts a quantity from storage scale: y = offset + gain*y .
	inline
	double scaleForward(double x) const {
		return offset + gain*x;
	}

	/// Converts a quantity to storage scale: x = (y-offset)/gain .
	inline
	double scaleInverse(double y) const {
		return (y-offset)/gain;
	}

	/// Returns the minimum value that can be returned using current storage type, gain and offset.
	inline
	double getMin() const {

		const std::type_info & t = drain::Type::getTypeInfo(type);

		if (t == typeid(void))
			throw std::runtime_error(std::string("ODIM")+__FUNCTION__+" type unset");

		if (drain::Type::call<drain::typeIsInteger>(t)){
			long int i = drain::Type::call<drain::typeMin, long int>(t);
			if (static_cast<double>(i) != undetect)
				return scaleForward(static_cast<double>(i));
			else
				return scaleForward(static_cast<double>(i+1));
		}
		else
			return scaleForward( drain::Type::call<drain::typeMin, double>(t) );

	}

	/// Returns the minimum value that can be returned using current storage type, gain and offset.
	inline
	double getMax() const {

		const std::type_info & t = drain::Type::getTypeInfo(type);

		if (t == typeid(void))
			throw std::runtime_error(std::string("ODIM")+__FUNCTION__+" type unset");

		if (drain::Type::call<drain::typeIsInteger>(t)){
			long int i = drain::Type::call<drain::typeMax, long int>(t);
			if (static_cast<double>(i) != nodata)  // or undetect ?
				return scaleForward(static_cast<double>(i));
			else
				return scaleForward(static_cast<double>(i-1));
		}
		else
			return scaleForward( drain::Type::call<drain::typeMax, double>(t) );

	}



	/// Functor (why inverse?)
	inline
	double operator()(double y) const {
		return (y-offset)/gain;
	}


	/// Resets the values.
	// ?virtual?
	void clear();


	/// Todo: keep the function, but move implementation to (future single-exec) register ?
	/*
	inline
	void getShortKeys(std::map<std::string,std::string> & m){
		 for (ReferenceMap::iterator it = begin(); it != end(); ++it){
			 const std::string & longKey = it->first;
			 const std::string  shortKey = longKey.substr(0,longKey.find(':'));
			 m[shortKey] = longKey;
			 std::cerr << shortKey << '#' << longKey << '\n';
		 }
	}
	*/


	/// Sets type, if unset (empty).
	virtual
	void update(const EncodingODIM & odim);


	/// Creates a short alias \c (attrib) for each \c (group):(attrib). Example: "gain" => "what:gain".
	/*  The object itself can be given as an argument.
	 *	Does not change the keys of the object.
	 */
	void addShortKeys(drain::ReferenceMap & ref);

	/// Copies image attributes and \b type . Experimental.
	/**
	 *  Image storage type, "type", is a non-standard property that must be copied separately.
	 */
	void copyFrom(const drain::image::Image & data);

	/// Traverses recursively subtrees and checks the types of PolarODIM variables.
	/**
	 *  \param dst - start group
	 */
	static
	inline
	void checkType(HI5TREE & dst){ EncodingODIM odim; checkType(dst, odim); } // Temp for thread-safety.


	/// Copies contents of this to a h5 group.
	/**
	 *  \param dstGroup - target group under which the respective \c /what , \c /where , and \c /how groups will be populated directly.
	 *
	 *  Currently, uses direct sub groups only, which means that subgroups referring to /dataset??/data??/data are used only.
	 *  \todo Change "what:quantity" -style namings to "dataset:data:what:quantity" .
	 *
	 *  dataset1/data1/quality1
	 */


	/// A set containing "what", "where" and "how".
	static
	const std::set<std::string> & attributeGroups;



protected:



	static
	void checkType(HI5TREE & dst, EncodingODIM & odim); // SEE ABOVE?


	static
	const std::set<std::string> & createAttributeGroups();


private:

	virtual // must
	void init(group_t initialize=ALL);



};


/// ODIM metadata (quantity, gain, offset, undetect, nodata, date, time)
class ODIM : public EncodingODIM {

public:

	inline
	ODIM(group_t initialize=ALL) : EncodingODIM(initialize){
		init(initialize);
	};

	inline
	ODIM(const ODIM & odim){
		initFromMap(odim);
	};

	inline
	ODIM(const drain::image::Image & image, const std::string & quantity=""){
		initFromImage(image, quantity);
	};

	inline
	~ODIM(){};

	/// /what (obligatory)
	std::string object;
	std::string version;

	std::string date;
	std::string time;
	std::string source;

	/// dataX/what (obligatory)
	std::string quantity;  // raise?
	std::string product;   // raise?
	std::string prodpar;   // raise?
	std::string startdate;
	std::string starttime;
	std::string enddate;
	std::string endtime;


	// Maximum Nyquist velocity
	double NI;

	// Number of images used in precipitation accumulation (lenient, not linked)
	long ACCnum;

	/// Sets number of bins (nbins) and number of rays (nrays)
	virtual
	void setGeometry(size_t cols, size_t rows){
		drain::Logger mout(__FUNCTION__,__FUNCTION__);
		mout.warn() << "trying to set geometry for plain ODIM; geom=(" << cols << ',' << rows << ")" << mout.endl;
	};


	/// Updates object, quantity, product and time information.
	/*!
	 *  Fills empty values. Updates time variables.
	 */
	virtual
	void update(const ODIM & odim);

	/// Retrieves the stored time. Returns true if successful, throws error if fail.
	bool getTime(drain::Time & t) const;

	/// Retrieves the start time. Returns true if successful, throws error if fail.
	bool getStartTime(drain::Time & t) const;

	/// Retrieves the end time. Returns true if successful, throws error if fail.
	bool getEndTime(drain::Time & t) const;

	/// Returns recommended coordinate policy. Redefined in PolarODIM.
	virtual inline
	const drain::image::CoordinatePolicy & getCoordinatePolicy() const {
		using namespace drain::image;
		static const CoordinatePolicy p(CoordinatePolicy::LIMIT);
		return p;
	}

	/// If nodata==undetect, set nodata=maxValue (hoping its not nodata...)
	/**
	 *  This oddity is needed because some manufactures do not distinguish between undetect and nodata (esp. with VRAD).
	 *
	 *  \param quantity - set only if this quantity applies.
	 */
	inline
	bool distinguishNodata(const std::string & quantity = ""){

		if (quantity.empty() || (quantity == this->quantity)){  // Fix Vaisala IRIS bug
			//std::cerr << "setNodata" << quantity << '\n';
			if (nodata == undetect){
				nodata = drain::Type::call<drain::typeMax,double>(type);
				return true;
			}
			//std::cerr << "nodata: " << nodata << '\n';
		}
		return false;
	}


	static
	const std::string dateformat; //, "%Y%m%d");

	static
	const std::string timeformat; // "%H%M%S");

	/// Write ODIM data relevant for data level, eg. \c /dataset2, \c data4, and root.
	/**
	 *  \tparam G - group selector
	 *  \tparam T - ODIM class
	 *
	 *  Examples of usage:
	 *
	 * 	- ODIM::copyToH5<ODIM::ROOT>(odim, resources.inputHi5);
	 *	- ODIM::copyToH5<ODIM::DATASET>(odim, resources.inputHi5(dataSetPath));
	 *  - ODIM::copyToH5<ODIM::DATA>(odim, dst);
	 *
	 */
	template <group_t G, class T>
	static inline
	void copyToH5(const T &odim, HI5TREE & dst) {
		static T odimLimited(G);
		odim.copyTo(odimLimited.getKeyList(), dst);
	}

	template <group_t G, class T>
	static inline
	void copyToH5(const T &odim, const HI5TREE & dst) {
		//static T odimLimited(G);
		//odim.copyTo(odimLimited.getKeyList(), dst);
	}


protected:

	///
	void copyTo(const std::list<std::string> & keys, HI5TREE & dst) const;


	template <class T>
	inline
	void initFromMap(const std::map<std::string,T> & m){
		init(ALL);
		updateFromMap(m);
	}

	virtual inline
	void initFromImage(const drain::image::Image & img){  // =""
		init(ALL);
		this->quantity = img.getProperties().get("what:quantity", "");
		copyFrom(img);
	}

	// deprec
	virtual inline
	void initFromImage(const drain::image::Image & img, const std::string & quantity){  // =""
		init(ALL);
		this->quantity = quantity;
		copyFrom(img);
	}

private:

	virtual // must
	void init(group_t initialize=ALL);



};



}  // namespace rack


#endif

// Rack
