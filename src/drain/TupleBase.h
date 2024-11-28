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


#ifndef DRAIN_TUPLE_BASE
#define DRAIN_TUPLE_BASE

#include <cstddef>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include <string>
//#include <set>

#include "StringBuilder.h"
#include "Type.h"  // TypeName

namespace drain {


// Consider inheritance TupleType->TupleBase, so all S-typed stuff here?
template <typename S>
struct TupleType {

	/// Zero, by default. For enum types, must be explicitly given.
	/**
	  \code
	   template <>
	    const my_enum TupleType<my_enum>::neutral_value(my_enum::DEFAULT_VALUE);
	  \endcode
	 *
	 */
	static
	const S neutral_value;
};

template <typename S>
const S TupleType<S>::neutral_value(0);


/**
 *  \tparam S - storage class of members (int, double, char)
 *  \tparam N - number of the members included
 */
template <typename S, size_t N=2>
class TupleBase {

public:

	typedef TupleBase<S,N> tuplebase_t;
	typedef S value_type;

	typedef S* iterator;
	typedef S const* const_iterator;

	static const size_t storageTypeSize;

	TupleBase(){};

	virtual
	~TupleBase(){};

	/// Return the number of elements.
	/**
	 *  Can be static, as any object has the same size.
	 */
	static inline
	size_t size(){
		return N;
	}

	virtual
	const_iterator begin() const = 0;

	virtual
	const_iterator end() const = 0;

	virtual
	iterator begin() = 0;

	virtual
	iterator end() = 0;

	/// Return const reference to element i.
	/**
	 *  Checks index overflow, but does not check if the actual members have been declared.
	 *  Use in check() in derived constructors to ensure total size of the object.
	 */
	const S & at(size_t i) const {
		//(std::stringstream("UniTuple: ") << __FUNCTION__).r
		if (i<N)
			return *(begin() + i);
		else {
			/*
			std::stringstream sstr;
			sstr << // TypeName<tuple_t>::str() << ':' <<
					__FUNCTION__ << " index overflow:" << i;
			throw std::runtime_error(sstr.str());
			*/
			throw std::runtime_error(StringBuilder<' '>(__FUNCTION__, "index overflow:", i));
		}
	}

	const S & operator[](size_t i) const {
		return at(i);
	}

	/// Return reference to element i.
	/**
	 *  Checks index overflow, but does not check if the actual members have been declared.
	 *  Use in check() in derived constructors to ensure total size of the object.
	 */
	S & at(size_t i){
		if (i<N)
			return *(begin() + i);
		else {
			/*
			std::stringstream sstr;
			sstr << // << TypeName<tuple_t>::str() << ':'
			 __FUNCTION__ << " index overflow:" << i;
			throw std::runtime_error(sstr.str());
			*/
			throw std::runtime_error(StringBuilder<' '>(__FUNCTION__, "index overflow:", i));
		}
	}

	S & operator[](size_t i){
		return at(i);
	}


	/// Equality operator
	bool operator==(const tuplebase_t &t) const {
		const_iterator it  = begin();
		const_iterator tit = t.begin();
		while (it != end()){
			if (*it != *tit)
				return false;
			++it;
			++tit;
		}
		return true;
	}

	/// Inequality operator
	bool operator!=(const tuplebase_t &t) const {
		return !(*this == t);
	}

	/// Copy elements to a Sequence, like stl::list, stl::set or stl::vector.
	template <class T>
	T & toSequence(T & sequence) const {
		sequence.clear();
		for (const_iterator it = begin(); it != end(); ++it){
			sequence.insert(sequence.end(), *it);
		}
		return sequence;
	}



	///
	//tuple_t & set(const tuple_t & t){
	void set(const tuplebase_t & t){
		if (&t != this){
			this->assignSequence(t);
		}
		// this->assign(t);
		/*
		if (&t != &this->tuple()){
			this->assign(t);
		}
		*/
		//return *this;
	}

	/// Assign tuple of different type and/or size.
	template <class T2, size_t N2=2>
	//tuple_t & set(const UniTuple<T2,N2> & t){
	//void set(const UniTuple<T2,N2> & t){
	void set(const TupleBase<T2,N2> & t){
		/*
		if (&t == this){
			throw std::runtime_error(StringBuilder<>(__FILE__, ':', __FUNCTION__, ": self-assignment violation: ", t, " [", drain::TypeName<TupleBase<T2,N2> >::str(), "]"));
		}
		*/
		this->assignSequence(t, true); // by default LENIENT, or how should it be?
		// return *this;
	}


	inline
	void set(const S & arg) {
		setIndexed(0, arg);
		this->updateTuple();
	}

	/// Set element(s).
	// Variadic-argument member set function.
	// https://en.cppreference.com/w/cpp/language/parameter_pack
	template<typename ... SS>
	inline
	void set(const S & arg, const SS &... rest) {
		setIndexed(0, arg, rest...);
		this->updateTuple();
	}

	template<typename T>
	inline
	void set(std::initializer_list<T> l){
		this->assignSequence(l);
	}


	/// Proposed for tuples only; derived classes should not shadow this.
	/*
	tuplebase_t & assign(const tuplebase_t & t){
		if (&t != this){
			this->assignSequence(t);
		}
		return *this;
	}

	tuplebase_t & assign(const value_type & value){
		this->fill(value);
		updateTuple();
		return *this;
	}
	 */

	/// Copy elements from a Sequence, like stl::list, stl::set or stl::vector.
	/**
	 *  If lenient, accepts sequences of different lengths, stopping at the shorter.
	 */
	template <class T>
	tuplebase_t & assignSequence(T & sequence, bool LENIENT = false){
		typename T::const_iterator cit = sequence.begin();
		iterator it = begin();
		while (cit != sequence.end()){
			if (it == end()){
				if (LENIENT){
					return *this;
				}
				else {
					throw std::runtime_error(StringBuilder<':'>(__FILE__, ':', __FUNCTION__, ": index overflow in assigning: ", *cit));
					/*
					std::stringstream sstr;
					sstr << __FILE__ << ':' << __FUNCTION__ << ": run out of indices in assigning: " << *cit;
					throw std::runtime_error(sstr.str());
					*/
				}
				break;
			}
			//	break;
			*it = *cit;
			++it;
			++cit;
		}
		return *this;
	}


	/// Set all the elements to i
	void fill(S i){
		for (iterator it = begin(); it != end(); ++it){
			*it = i;
		}
	}

	inline
	void clear(){
		fill(S());
	}


	/**
	 *   Derived classes may change the layout.
	 */
	virtual
	std::ostream & toStream(std::ostream & ostr, char separator=',') const {
		char sep = 0;
		for (const_iterator it = begin(); it != end(); ++it){
			if (sep)
				ostr << sep;
			else
				sep = separator;
			ostr << *it; //(*this)[i];
		}
		return ostr;
	}

	std::string toStr(char separator=',') const {
		std::stringstream sstr;
		toStream(sstr, separator);
		return sstr.str();
	}


// protected?

	// AreaGeom etc.
	virtual inline
	void updateTuple(){};


protected:

	/// Argument stack endpoint function; final step of variadic argument set(arg, ...) .
	void setIndexed(size_t i){
		//std::cout << __FUNCTION__ << " complete " << i << '\n';
		this->updateTuple();
	}

	/// Worker called by set(T2 arg, T2 arg2, ...)
	template<typename T2, typename ... TT>
	void setIndexed(size_t i, T2 arg, const TT &... rest){
		if (i>=N){
			throw std::runtime_error(StringBuilder<>(__FILE__, ':', __FUNCTION__, ": index (", i, ")  overflow in assigning: ", arg, ",..."));
			return;
		}
		// std::cout << __FUNCTION__ << "N=" << N << " arg=" << arg << " remaining:" << i << std::endl;
		this->at(i) = static_cast<S>(arg);
		setIndexed(i+1, rest...);
	}

};


template <typename S, size_t N>
const size_t TupleBase<S,N>::storageTypeSize = sizeof(S);


template <class S, size_t N>
std::ostream & operator<<(std::ostream & ostr, const TupleBase<S,N> & tuple){
	return tuple.toStream(ostr);
}


template <typename S, size_t N>
struct TypeName<TupleBase<S,N> > {

    static const std::string & str(){
    	if (N == 0){
        	static const std::string name = drain::StringBuilder<>("PseudoTuple<", drain::TypeName<S>::str(),',' , drain::TypeName<S>::str(),">");
            return name;
    	}
    	else {
        	static const std::string name = drain::StringBuilder<>("PseudoTuple<", drain::TypeName<S>::str(),',' , drain::TypeName<S>::str(),',' , N, ">");
            return name;
    	}
    }

};



}  // drain


#endif
