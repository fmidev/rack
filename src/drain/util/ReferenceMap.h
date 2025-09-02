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
//#include <typeinfo>
#include <drain/Log.h>
#include <stdexcept>
//#include <iostream>
//#include <vector>
#include <string>

#include <map>
#include <list>


#include <drain/Reference.h>
#include <drain/String.h>

#include "Range.h"
// #include "FlexibleVariable.h"
//#include "ReferenceVariable.h"

#include "SmartMap.h"


#ifndef REFERENCE_MAP
#define REFERENCE_MAP


namespace drain {

/// A map of references to base type scalars, arrays or std::string; changing values in either are equivalent operations.
/**
 *  New, templated implementation, extending support to FlexVariables, for example.
 *
 *  Future extension: units. Should be contained in Variable type, not "externally" in the map.
 *
 *  \tparam T SmartVariable: Reference or FlexibleVariable; in future, also Parameter<Reference> or Parameter<FlexibleVariable>. Must implement link()
 *
 *
 */
template <class T=Reference>
class ReferenceMap2 : public SmartMap<T> {

public:

	typedef T ref_t;
	typedef SmartMap<T> map_t;


	/// Associates a map entry with a variable
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */
	template <class F>
	inline
	ref_t & link(const std::string & key, F &x){

		ref_t & r = (*this)[key];
		r.link(x); // .setSeparator(this->arraySeparator);
		return r;

	}

	/// Create a reference to a basic type or std::string. (Also for basetype arrays.)
	inline
	ref_t  & link(const std::string & key, void *ptr, const std::type_info &type, size_t count=1){

		ref_t & r = (*this)[key];
		r.link(ptr, type, count);
		return r;

		/*
		if (this->find(key) == this->end()) // not  already referenced
			this->keyList.push_back(key);

		Reference & r = map_t::operator[](key);
		r.setSeparator(this->arraySeparator); // applicable, if array type element
		r.link(ptr, type, count);
		// unitMap[key] = unit;
		return r;
		*/
	}

	inline
	void unlink(const std::string & key){
		map_t::erase(key);
		for (std::list<std::string>::iterator it = this->keyList.begin(); it != this->keyList.end(); ++it){
			if (*it == key){
				this->keyList.erase(it);
				break;
			}
		}
		//unitMap.erase(key);
	}


	/** Alternatives in handling a link which is outside the source object.
	 *
	 */
	typedef enum {
		LINK,    /**<  Link also external targets */
		SKIP,    /**<  No action */
		RESERVE, /**< No not link, but add a void entry as a placeholder */
		ERROR    /**< Throw exception */
	} extLinkPolicy;

	/// Experimental. Copies references and values of a structure to another.
	/**
	 * 	\param m - links to the members of the source object
	 * 	\param src - the source object
	 * 	\param dst - the destination object
	 *
	 *  Also updates key list and unit map?
	 */
	template <class T2>
	//static  // Start with T. Todo: consider other
	void copyStruct(const ReferenceMap2<T> & m, const T2 & src, T2 & dst, extLinkPolicy policy=RESERVE) {
		//, bool copyValues = true, bool linkExternal = false){

		Logger mout(__FILE__, __FUNCTION__);
		long s = sizeof(T2); // yes signed
		//mout.warn("experimental, obj.size=" , s );

		// Clearing is bad, it resets work of base class constructors
		// if (policy==LINK)
		//	clear();

		typedef unsigned long addr_t;
		typedef          long addr_diff_t;

		const addr_t srcAddr = (addr_t)(&src);
		const addr_t dstAddr = (addr_t)(&dst);
		//for (std::list<std::string>::const_iterator it = m.getKeyList().begin(); it != m.getKeyList().end(); ++it){
		for (const std::string & key: m.getKeyList()){
			// const std::string & key = *it;
			const ref_t & srcRef = m[key];
			addr_t srcVarAddr = (addr_t)srcRef.getPtr();
			addr_diff_t relativeAddr = srcVarAddr - srcAddr;
			if ((relativeAddr >= 0) && (relativeAddr < s)){ // INTERNAL
				//Reference & dstMemberRef = (*this)[key];
				ref_t & dstMemberRef = link(key, (void *)(dstAddr + relativeAddr), srcRef.getType(), srcRef.getElementCount());
				//mout.warn("local: " , key , ':' , srcRef.getElementCount() );
				dstMemberRef.copyFormat(srcRef);
				dstMemberRef.assignCastable(srcRef); // value
			}
			else {
				//mout.warn("external: " , key     );
				switch (policy) {
				case LINK:
					link(key, (void *)srcVarAddr, srcRef.getType(), srcRef.getElementCount()).copyFormat(srcRef);
					break;
				case RESERVE:
					// reserve(key);
					(*this)[key];
					//mout.warn("reserved: " , key     );
					//mout.warn("keyList:  " , getKeys() );
					break;
				case SKIP:
					mout.debug("skipping external variable: '" , key , '=' , srcRef , "' relative addr=" , relativeAddr );
					break;
				case ERROR:
					mout.error("external variable: '" , key , '=' , srcRef , "' relative addr=" , relativeAddr );
					break;
				default:
					mout.warn("unknown enum option in handling external variable '" , key , '=' , srcRef , "' relative addr=" , relativeAddr );
				}
			}
		}
	}
};


/// A map of references to base type scalars, arrays or std::string; changing values in either are equivalent operations.
/** A class designed for objects returned by CastableIterator. Contains a pointer to external object.
 *
 *  \example ReferenceMap-example.cpp
 */
// TODO: consider clear() (shallow op)
class ReferenceMap : public SmartMap<Reference> {//public std::map<std::string,Reference> {

public:

	/// Default constructor
	/**
	 *  \strictness - if true, allow attempts of assigning to non-existing entries; otherways throw exception on attempt.
	 */
	// ReferenceMap(bool ordered, char separator) : SmartMap<Reference>(ordered, separator){}; //, STRICTLY_CLOSED, keys(orderedKeyList), units(unitMap)
	ReferenceMap(char separator=',') : SmartMap<Reference>(separator){}; //, STRICTLY_CLOSED, keys(orderedKeyList), units(unitMap)

	/// Copy constructor copies only the separators; does not copy the items. \see copy()
	inline
	ReferenceMap(const ReferenceMap & rmap): SmartMap<Reference>(rmap.separator, rmap.arraySeparator){
	};

	inline virtual
	~ReferenceMap(){};

	// Temporary catch for Range
	template <class F>
	Reference & link(const std::string & key, Range<F> &x, const std::string & unit = std::string()){
		Logger mout(__FILE__, __FUNCTION__);
		mout.deprecating(" type drain::Range<>  use .tuple() instead: ", key, '[', unit, ']');
		return link(key, &x, typeid(F), 2, unit);
	}

	/// Associates a map entry with a variable
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */
	template <class F>
	Reference & link(const std::string & key, F &x, const std::string & unit = std::string()){

		if (find(key) == end()){ // not  already referenced

			if (keyList.empty()){
				if (key.empty()){
					std::cerr << "empty key referencing to " << sprinter(x) << " unit=" << unit << std::endl;
				}
				else if (key.at(0) == '_'){
					throw std::runtime_error(key + ": hidden parameters can be added only after visible");
				}
			}
			else if ((keyList.back().at(0) == '_') && (key.at(0) != '_')){
				throw std::runtime_error(key + ": cannot add visible parameters after hidden");
			}


			keyList.push_back(key);

		}

		// Create
		Reference & r = std::map<std::string,Reference>::operator[](key);
		r.setSeparator(arraySeparator); // applicable, if array type element
		// Link
		r.link(x);
		unitMap[key] = unit;
		/*
		if (!unit.empty()){
			unitMap[key] = unit;
		}
		else {
			unitMap[key] = drain::Type::call<drain::simpleName>(typeid(F));
		}
		*/
		return r;
	}

	inline
	Reference & link(const std::string & key, Reference &x, const std::string & unit = std::string()){
		return link(key, x.getPtr(), x.getType(), x.getElementCount(), unit); //.fillArray = item.fillArray;
	}


	/// For arrays.
	inline
	Reference & link(const std::string & key, void *ptr, const std::type_info &type, size_t count, const std::string & unit = std::string()){

		if (find(key) == end()) // not  already referenced
			keyList.push_back(key);

		Reference & r = std::map<std::string,Reference>::operator[](key);
		r.setSeparator(arraySeparator); // applicable, if array type element
		r.link(ptr, type, count);
		unitMap[key] = unit;
		return r;

	}

	/// Convenience: create a reference to a scalar. For arrays, use the
	inline
	Reference & link(const std::string & key, void *ptr, const std::type_info &type, const std::string & unit = std::string()){
		return link(key, ptr, type, 1, unit);
	}

	/// Associates a map entry with a variable, adding key in the beginning of key list.
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */
	template <class F>
	Reference & referenceTop(const std::string & key, F &x, const std::string & unit = std::string()){

		if (find(key) != end()) // already referenced
			keyList.push_front(key);

		//Reference & r = std::map<std::string,Reference>::operator[](key).link(x);
		Reference & r = std::map<std::string,Reference>::operator[](key);
		r.setSeparator(arraySeparator); // applicable, if array type element
		// Link
		r.link(x);

		unitMap[key] = unit;
		return r;
	}


	/// Adopts the references of r. If replace==false, only new entries are appended.
	void append(ReferenceMap & rMap, bool replace=true){
		//std::cerr << __FILE__ << " -> " << __FUNCTION__ <<  std::endl;
		const std::list<std::string> & keys = rMap.getKeyList();
		//for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
		for (const std::string & key: keys){
			//std::cerr << " -> " << *it <<  std::endl;
			if (replace || !hasKey(key)){
				Reference & srcItem = rMap[key];
				Reference & item = link(key, srcItem, rMap.unitMap[key]); //.fillArray = item.fillArray;
				// Reference & item = link(key, (const Castable &)srcItem, rMap.unitMap[key]); //.fillArray = item.fillArray;
				// Reference & item = link(key, srcItem.getPtr(), srcItem.getType(), srcItem.getElementCount(), rMap.unitMap[key]); //.fillArray = item.fillArray;
				item.setFill(srcItem.fillArray);
				item.setInputSeparator(srcItem.getInputSeparator());
				item.setOutputSeparator(srcItem.getOutputSeparator());
			}
		}
	}

	// todo: consider NULL? (but leads to problems)

	/// Removes an entry from the map.
	inline
	void delink(const std::string & key){
		std::map<std::string,Reference>::erase(key);
		for (std::list<std::string>::iterator it = keyList.begin(); it != keyList.end(); ++it)
			if (*it == key){
				keyList.erase(it);
				break;
			}
		unitMap.erase(key);
	}

	/// Adds a null entry, expecting the link later.
	/**
	 *  The key will be added in the key list.
	 *
	 */
	void reserve(const std::string & key){

		if (find(key) == end()) // not  already referenced
			keyList.push_back(key);

		// Create
		// Reference & r =
		std::map<std::string,Reference>::operator[](key);
		// Now r type is unset. (ptr undefined)

	}

	/// Removes all the elements of the map.
	/**
	 *  Clears the map and its ordered keylist (and units).
	 */
	virtual inline
	void clear(){
		SmartMap<Reference>::clear();
		keyList.clear();
		unitMap.clear();
	}

	/** Alternatives in handling a link which is outside the source object.
	 *
	 */
	typedef enum {
		LINK,    /**< Link also external targets */
		RESERVE, /**< No not link, but add entry (void) */
		SKIP,    /**< No action */
		ERROR    /**< Throw exception */
	} extLinkPolicy;

	/// Experimental. Copies references and values of a structure to another.
	/**
	 * 	\param m - links to the members of the source object
	 * 	\param src - the source object
	 * 	\param dst - the destination object
	 *
	 *  Also updates key list and unit map?
	 */
	template <class T>
	void copyStruct(const ReferenceMap & m, const T & src, T & dst, extLinkPolicy policy=RESERVE) {
		//, bool copyValues = true, bool linkExternal = false){

		Logger mout(__FILE__, __FUNCTION__);
		long s = sizeof(T); // yes signed
		//mout.warn("experimental, obj.size=" , s );

		// Clearing is bad, it resets work of base class constructors
		// if (policy==LINK)
		//	clear();

		typedef unsigned long addr_t;
		typedef          long addr_diff_t;

		const addr_t srcAddr = (addr_t)(&src);
		const addr_t dstAddr = (addr_t)(&dst);
		//for (std::list<std::string>::const_iterator it = m.getKeyList().begin(); it != m.getKeyList().end(); ++it){
		for (const std::string & key: m.getKeyList()){
			// const std::string & key = *it;
			const Reference & srcRef = m[key];
			addr_t srcVarAddr = (addr_t)srcRef.getPtr();
			addr_diff_t relativeAddr = srcVarAddr - srcAddr;
			if ((relativeAddr >= 0) && (relativeAddr < s)){ // INTERNAL
				Reference & dstMemberRef = link(key, (void *)(dstAddr + relativeAddr), srcRef.getType(), srcRef.getElementCount());
				//mout.warn("local: " , key , ':' , srcRef.getElementCount() );
				dstMemberRef.copyFormat(srcRef);
				dstMemberRef = srcRef; // value
				//dstMemberRef.setInputSeparator(srcRef.getInputSeparator());
			}
			else {
				//mout.warn("external: " , key     );
				switch (policy) {
				case LINK:
					link(key, (void *)srcVarAddr, srcRef.getType(), srcRef.getElementCount()).copyFormat(srcRef);
					break;
				case RESERVE:
					reserve(key);
					//mout.warn("reserved: " , key     );
					//mout.warn("keyList:  " , getKeys() );
					break;
				case SKIP:
					mout.debug("skipping external variable: '" , key , '=' , srcRef , "' relative addr=" , relativeAddr );
					break;
				case ERROR:
					mout.error("external variable: '" , key , '=' , srcRef , "' relative addr=" , relativeAddr );
					break;
				default:
					mout.warn("unknown enum option in handling external variable '" , key , '=' , srcRef , "' relative addr=" , relativeAddr );
				}
			}
		}

		// Shared properties.
		//keyList = m.keyList;
		separator = m.separator;
		arraySeparator = m.arraySeparator;
		unitMap = m.unitMap;

	}


	/// Import map, adopting the element types.
	template <class T>
	inline
	ReferenceMap & operator=(const SmartMap<T> & v){
		//Logger log(__FILE__, __FUNCTION__);
		//log.error() << "in:" << v << log.endl;
		importMap(v);
		return *this;
	}

	/// Return element associated with \c key.
	virtual
	mapped_type & operator[](const std::string &key){

		Logger mout(__FILE__, __FUNCTION__);

		iterator it = this->find(key);
		if (it != this->end()) {
			return it->second;
		}
		else {
			mout.warn("current contents: " , *this );

			mout.error("key '" , key ,"' not declared (referenced)" );
			//throw std::runtime_error(key + ": ReferenceMap & operator[] : key not found");
			static mapped_type empty;
			return empty;
		}

	}

	///  Calling unreferenced key throws an exception.
	virtual
	const mapped_type & operator[](const std::string &key) const {

		Logger mout(__FILE__, __FUNCTION__); //Logger mout(__FILE__, __FUNCTION__);

		const_iterator it = this->find(key);
		if (it != this->end()) {
			return it->second;
		}
		else {
			mout.error("key '" , key ,"' not declared (referenced)" );
			//throw std::runtime_error(key + ": ReferenceMap & operator[] : key not found");
			static const mapped_type empty;
			return empty;
		}
	}

	typedef std::map<std::string,std::string> unitmap_t;

	/// Returns measurement unit information of the actual map entries.
	inline
	const unitmap_t & getUnitMap() const { return unitMap; };

	/// Creating a common segment for
	/*
	template <class T>
	static
	size_t serialize(const SmartMap<T> & rmap, std::vector<char> & memory);
	*/

	/// Associates a map entry with a variable
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */


protected:

	/// Defines the units of each element.
	// Questionable: consider SmartReference etc. with caster and unit info?
	unitmap_t unitMap;


};

/*
template <class T>
size_t ReferenceMap::serialize(const SmartMap<T> & smap, std::vector<char> & memory){

	typedef SmartMap<T> smap_t;

	size_t i;
	for (smap_t::const_iterator it = smap.begin(); it != smap.end(); ++it){

		const std::string & key = it->first;
		const Castable & value  = it->second;

		if (v.isString()){

			///  Variable-length string datatype.

		}
		else {
		}

	}

}
*/

}  // namespace drain


#endif

// Drain
