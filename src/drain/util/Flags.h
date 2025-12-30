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

#ifndef DRAIN_FLAGS
#define DRAIN_FLAGS

#include <iostream>
#include <list>
#include <iterator>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
//#include <typeinfo>
#include <stdlib.h>

#include <drain/Log.h>
#include <drain/String.h>
#include <drain/Type.h>

#include "Dictionary.h"

#include "FlagBase.h"

namespace drain {


/*
template <typename E>
const SprinterLayout drain::FlaggerBase<E>::dictLayout = {",", "?", "=", ""};
*/





///
/**
 *   Abstract class, because getDict() returning Dictionary<std::string,E> still undefined.
 */
template <typename E>
class SingleFlagger : public FlaggerBase<E,E> {

public:

	//typedef typename FlaggerBase<E>::dict_value_t dict_value_t;
	typedef E value_t;
	typedef E storage_t;

	typedef FlaggerBase<E,E> flagger_t;
	// typedef typename EnumDict<E>::dict_t dict_t;
	typedef typename flagger_t::dict_t dict_t;
	typedef typename dict_t::key_t key_t; // ~string

protected:

	const dict_t & dict;
	// dict_t ownDict;

public:

	virtual
	const dict_t & getDict() const override {
		return dict;
	};


	inline
	SingleFlagger(const dict_t & dict) : dict(dict) {
	};

	/***
	 *
	 *  storage_t == value_t
	 */
	inline
	SingleFlagger(const dict_t & dict, const storage_t &v) : dict(dict) {
		this->value = v;
	};


	/***
	 *
	 *  storage_t == value_t
	 */
	inline
	SingleFlagger(const dict_t & dict, storage_t & v, char sep=',') : flagger_t(v, sep), dict(dict){
	};


	virtual inline
	~SingleFlagger(){};


	/// Return true, if value is exactly \c x .
	virtual
	bool isSet(const storage_t & x) const override {
		return (this->value == x);
	};

	// Start copy Base
	virtual inline
	void set(const key_t & key){
		if (key.empty())
			return; // needed?
		assign(key);
	}

	/// Set a single flag.
	virtual inline
	void set(const value_t & value){
		this->value = value;
	};

	virtual inline
	void set(const FlaggerBase<E> & flagger){
		this->value = flagger.value;
	}

	// END Base

	template <class T>
	inline
	SingleFlagger<E> & operator=(const T & v){
		this->set(v);
		return *this;
	}

	/// String corresponding the current value. Returns empty, if not found.
	virtual inline
	const key_t & str() const  override {
		return this->getDict().getKey(this->value);
	}

protected:

	/// Assign key. Expects a single key, not a combination of keys.
	/**
	 *  By definition, SingleFlagger contains a value that corresponds to a single key, at a time.
	 *
	 *  MultiFlagger::assign() supports setting several keys at once.
	 */
	virtual
	void assign(const std::string & key) override {

		// NO this->set(s);

		if ((key.find(',')!= std::string::npos)||(key.find('|')!= std::string::npos)){
			Logger mout(__FILE__, __FUNCTION__);
			mout.unimplemented<LOG_ERR>(__FILE__, ':', TypeName<E>::str(), ": multi-key arg: '"+ key, "', for single-flagger: ", sprinter(this->getDict().getKeys()));
		}

		if (this->getDict().hasKey(key)){
			this->value = static_cast<storage_t>(this->getDict().getValue(key)); // Cast needed (only) if MultiFlagger converts enum value_t -> storage_t
		}
		else {
			Logger mout(__FILE__, __FUNCTION__);
			value_t v = static_cast<value_t>(atoi(key.c_str()));
			if (!this->getDict().hasValue(v)){
				mout.suspicious("value: ", v, " corresponds to no value in dict:",  this->getDict());
				//mout.error(__FILE__, ':', TypeName<E>::str(), ": no such key: '"+ key, "', keys=", sprinter(this->getDict().getKeys()));
				mout.error("Flagger@", TypeName<E>::str(), ": no such key: '"+ key, "', dict=", sprinter(this->getDict()));
			}
			// this->value = FlagResolver::getIntValue(this->getDict(), key);  // SingleFlagger does not like this.
			// throw std::runtime_error(std::string("Dictionary[") + typeid(dict_value_t).name()+ "]: no such key: "+ key);
			this->value = v;
		}

	};



};

template <typename E>
inline
std::ostream & operator<<(std::ostream & ostr, const drain::SingleFlagger<E> & flagger) {
	ostr << flagger.str();
	return ostr;
}


///
/**
 *   Note that if enum definition contains overlapping (combined) values, SingleFlagger may be sufficient.
 *   If arbitrary bit combinations should be supported, MultiFlagger must be used.
 *   Example {USER=1, GROUP=2, OTHER=4, ALL=USER|GROUP|OTHER}.
 *
 *   Abstract class, because getDict() returning Dictionary<std::string,E> still undefined.
 */
template <typename E, typename T=size_t>
class MultiFlagger : public FlaggerBase<E,T> {  // <E,FlagResolver::ivalue_t> { //

public:

	typedef E value_t;
	typedef T storage_t;
	typedef FlaggerBase<E,T> flagger_t;
	//typedef FlagResolver::ivalue_t storage_t;
	//typedef FlaggerBase<E,FlagResolver::ivalue_t> flagger_t;
	typedef typename flagger_t::dict_t dict_t;
	typedef typename dict_t::key_t key_t; // ~string


	inline
	MultiFlagger(const dict_t & dict) : dict(dict) {
	};

	template <typename ... V>
	inline
	MultiFlagger(const dict_t & dict, const V &... args) : dict(dict) {
		set(args...);
	};

	/**
	 *  \param v - linked variable
	 */
	inline
	MultiFlagger(const dict_t & dict, storage_t & target, char sep=',') : flagger_t(target, sep), dict(dict){
	};

	virtual inline
	~MultiFlagger(){};


	virtual inline
	const dict_t & getDict() const override {
		return dict;
	};

	/// Return true, all the bits of argument \c x are set.
	/**
	 *  \param x - bits to be studied
	 *  \return - true, if bits overlap fully - or argument x is zero.
	 */
	virtual inline
	bool isSet(const storage_t & x) const override {
		return ((this->value & x) == x);
	};

	/// Return true, all the bits of argument \c x are set.
	inline
	bool isAnySet(const storage_t & x) const {
		return ((this->value & x) != 0);
	};

	/// Unset some bit(s).
	inline
	void unset(const storage_t & x){
		this->value = (this->value & ~x);
		//return ((this->value & x) == x);
	}

	/// Set bits, accepting numeric values or keys.
	/**
	 *  Starts with resetting the flags.
	 *
	 *  \tparam T – numeric values or strings
	 *
	 */
	template <typename ... V>
	inline
	void set(const V &... args) {
		this->reset();
		add(args...);
	}


	/// Add bit values
	template <typename V, typename ...VV>
	inline
	void add(const V & arg, const VV &... args) {
		//std::cerr << __FUNCTION__ << ':' << arg << std::endl;
		addOne(arg);
		add(args...);
	}

	/// For exporting values.
	virtual
	const key_t & str() const  override  {
		// const dict_t & dict = this->getDict();
		currentStr = FlagResolver::getKeys(this->getDict(), this->value, this->separator);
		return currentStr;
	}


	/// For importing values. After assignment, update() should be called. Experimental
	virtual
	std::string & str(){
		// NEW: also non-const is updated
		// const dict_t & dict = this->getDict();
		currentStr = FlagResolver::getKeys(this->getDict(), this->value, this->separator);
		return currentStr;  // CHECK USAGE
	}



protected:

	const dict_t & dict;

	/// Split a string to separate keys and/or values and set them.
	virtual
	void assign(const std::string & s) override {
		const dict_t & dict = this->getDict();
		this->value = FlagResolver::getIntValue(dict, s, this->separator); // uses dict.separator if needed
	}


	inline
	void add(){};

	inline
	void addOne(const key_t & key){
		// Resolves also "" and numeric keys.
		this->value |= FlagResolver::getIntValue(this->getDict(), key, this->separator);
	}

	// New
	inline
	void addOne(const storage_t & value){
		this->value |= value;
	}

	template <typename T2>
	inline
	void addOne(const T2 & value){
		// why OR op in dvalue
		// this->value = static_cast<dict_value_t>((dvalue_t)this->value | (dvalue_t)value);
		this->value |= static_cast<value_t>(value);
	}

	mutable
	std::string currentStr;


};



template <typename E,typename T>
inline
std::ostream & operator<<(std::ostream & ostr, const drain::MultiFlagger<E,T> & flagger) {  // ,T
	return FlagResolver::keysToStream(flagger.getDict(), flagger.getValue(), ostr);
}






/// Flagger with own dictionary, and accepting values of (integer) type T
/**
 *  Designed for local, instantaneous use.
 *  Should several flaggers use the same dictionary, use Flagger2<F> instead.
 *
 *  \tparam F – SingleFlagger<T> or MultiFlagger<T>
 *
 */
template <class F>
class Flags2 : public F {

public:

	typedef F fbase_t;
	typedef typename F::value_t value_t;
	typedef typename F::dict_t   dict_t;


	virtual
	const dict_t & getDict() const {
		return dict;
	};

	virtual
	dict_t & getDict(){
		return dict;
	};


	///
	/** If the dictionary is small, consider short init:
	 *  Flags2({"A",1}, {"B",2}}
	 *
	 */
	inline
	Flags2(){
	};

	/// Constructor for relatively short dictionaries
	/**
	 *  \code
	 *  Flags2 flags({"A",1}, {"B",2}});
	 *  \endcode
	 */
	inline
	Flags2(const std::initializer_list<typename dict_t::entry_t> & list) : dict(list){
	};

	inline
	Flags2(const Flags2<F> & flags) : dict(flags.dict){
	}

	template <class T>
	inline
	Flags2<F> & operator=(const T & v){
		this->set(v);
		return *this;
	}

protected:

	// Consider inherited dictionary, through a wrapper (first inherit, to guarantee init ref validity?).
	dict_t dict;


};



} // drain::


#endif
