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
#ifndef ENC_ODIM_STRUCT
#define ENC_ODIM_STRUCT

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
#include "ODIMPath.h"

namespace rack {

/// Structure for data storage type, scaling and marker codes. Does not contain quantity.
/**
 *  Within each ODIM class, each variable should have
 *
 *  -# name
 *  -# type
 *  -# group/name
 *  -# dataset/group/name
 *
 *  The scope for variables extends "down to the data array". Ie.
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
 *
 *  See also: LinearScaling (could be used as base class?)
 */
class EncodingODIM : public drain::ReferenceMap {  // public ODIMPathElem,

public:

	///
	typedef unsigned char group_t;

	/// Default constructor.
	inline
	EncodingODIM(group_t initialize = ODIMPathElem::ALL_LEVELS){
		init(initialize);
	};

	/// Copy constructor.
	inline  // todo raise initFromMap (was: FromODIM)
	EncodingODIM(const EncodingODIM & odim){
		init(ODIMPathElem::ALL_LEVELS);
		updateFromMap(odim); // importMap can NOT be used because non-EncodingODIM arg will have a larger map
	};

	inline
	EncodingODIM(const drain::image::Image & image){
		initFromImage(image);
	};


	inline
	EncodingODIM & operator=(const EncodingODIM & odim) {
		// std::cerr << "EncodingODIM & operator=" << std::endl;
		updateFromMap(odim);
		return *this;
	}



	/// This is non-standard (not in ODIM), but a practical means of handling storage type of datasets.
	/**
	 *  See drain::Type.
	 */
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

	void setRange(double min, double max);

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

	/// Returns true for a valid measurement value, false for \c undetect and \c nodata marker values.
	inline
	bool isValue(double x) const {
		return (x != undetect) && (x != nodata);
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

	/// Returns the minimum physical value that can be returned using current storage type, gain and offset.
	double getMin() const;

	/// Returns the minimum physical value that can be returned using current storage type, gain and offset.
	double getMax() const;

	/// Functor (why inverse?)
	inline
	double operator()(double y) const {
		return (y-offset)/gain;
	}


	/// Resets the values.
	// ?virtual?
	/// FIX rename !!
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
	/*
	 *	Does not change the values of the map.
	 *
	 *	The object itself can be given as an argument, \see addShortKeys().
	 */
	void addShortKeys(drain::ReferenceMap & ref);

	/// Creates a short alias \c (attrib) for each \c (group):(attrib). Example: "gain" => "what:gain".
	/*
	 *	Does not change the values of the map.
	 */
	inline
	void addShortKeys(){
		addShortKeys(*this);
	}

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
	void checkType(HI5TREE & dst){
		EncodingODIM odim;
		checkType(dst, odim);
	} // Temp for thread-safety.


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
	//const std::set<std::string> & attributeGroups;
	const std::set<ODIMPathElem> & attributeGroups;



protected:



	static
	void checkType(HI5TREE & dst, EncodingODIM & odim); // SEE ABOVE?


	static
	const std::set<ODIMPathElem> & createAttributeGroups();


private:

	virtual // must
	void init(group_t initialize = ODIMPathElem::ALL_LEVELS);

	void initFromImage(const drain::image::Image & img);

};



}  // namespace rack


#endif

// Rack
