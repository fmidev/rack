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


#ifndef DRAIN_UNITUPLE
#define DRAIN_UNITUPLE

#include <iostream>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include <string>


namespace drain {


/// Tuple of N elements of type T
/**
 *  Uniformity means that all the elements are of same type unlike in std::tuple.
 *
 */
template <class T, size_t N=2>
class UniTuple {


public:

	typedef T  value_t;
	typedef UniTuple<T,N> tuple_t;

	typedef T* iterator;
	typedef const T* const_iterator;

	/// Proposed for tuple only; derived classes should not shadow this.
	tuple_t & assign(const tuple_t & t){
		// initTuple()
		iterator it = begin();
		const_iterator tit = t.begin();
		while(it != end()){
			*it = *tit;
			++it;
			++tit;
		}
		updateTuple();
		return *this;
	}

	tuple_t & assign(const value_t & value){
		// initTuple();
		fill(value);
		updateTuple();
		return *this;
	}


	tuple_t & operator=(const tuple_t &t){
		assign(t);
		return *this;
	}

	tuple_t & operator=(const value_t & value){
		assign(value);
		return *this;
	}


	/// Return the number of elements.
	/**
	 *  Can be static, as any object has the same size.
	 */
	static inline
	size_t size(){
		return N;
	}

	///
	tuple_t & set(const tuple_t & t){
		assign(t);
		return *this;
	}

	/// Set element(s).
	// Variadic-argument member set function.
	// https://en.cppreference.com/w/cpp/language/parameter_pack
	template<typename ... TT>
	inline
	void set(T arg, const TT &... rest) {
		// initTuple()
		setIndexed(0, arg, rest...);
		updateTuple();
	}

	inline
	const_iterator begin() const {
		return start;
		//return (const_iterator)this;
	}

	inline
	const_iterator end() const {
		return start+N;
		//return (const_iterator)this + N;
	}

	inline
	iterator begin() {
		return start;
		//return (iterator)this;
	}

	inline
	iterator end() {
		return start+N;
		// return (iterator)this + N;
	}


	/// Return const reference to element i.
	/**
	 *  Checks index overflow, but does not check if the actual members have been declared.
	 *  Use in check() in derived constructors to ensure total size of the object.
	 */
	const T & at(size_t i) const {
		//(std::stringstream("UniTuple: ") << __FUNCTION__).r
		if (i<N)
			return *(begin() + i);
		else {
			std::stringstream sstr;
			sstr << "UniTuple<," << N << ">:" << __FUNCTION__ << " index overflow:" << i;
			throw std::runtime_error(sstr.str());
		}
	}

	const T & operator[](size_t i) const {
		return at(i);
	}

	/// Return reference to element i.
	/**
	 *  Checks index overflow, but does not check if the actual members have been declared.
	 *  Use in check() in derived constructors to ensure total size of the object.
	 */
	T & at(size_t i){
		if (i<N)
			return *(begin() + i);
		else{
			std::stringstream sstr;
			sstr << "UniTuple<," << N << ">:" << __FUNCTION__ << " index overflow:" << i;
			throw std::runtime_error(sstr.str());
		}
	}

	T & operator[](size_t i){
		return at(i);
	}


	/// Equality operator
	bool operator==(const tuple_t &t) const {
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
	bool operator!=(const tuple_t &t) const {
		return !(*this == t);
	}

	/// Copy elements to a Sequence, like stl::list, stl::set or stl::vector.
	template <class S>
	S & toSequence(S & sequence) const {
		sequence.clear();
		for (const_iterator it = begin(); it != end(); ++it){
			sequence.insert(sequence.end(), *it);
		}
		return sequence;
	}

	/// Copy elements from a Sequence, like stl::list, stl::set or stl::vector.
	template <class S>
	UniTuple<T,N> & fromSequence(S & sequence){
		typename S::const_iterator cit = sequence.begin();
		iterator it = begin();
		while (it != end()){
			*it = *cit;
			++it;
			++cit;
			if (cit ==  sequence.end())
				break;
		}
		return *this;
	}

	/// Set all the elements to i
	void fill(T i=0){
		for (iterator it = begin(); it != end(); ++it){
			*it = i;
		}
	}

	// Self-reference for casting
	tuple_t & tuple(){
		return *this;
	}

	// Self-reference for casting
	const tuple_t & tuple() const{
		return *this;
	}

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

	// AreaGeom etc.
	virtual inline
	void updateTuple(){};


protected:

	const iterator start;


private:

	// Utility for initializing references is derived classes  Point(x,y): x(init), y(++init),
	iterator init;

protected:

	T & next(){
		if (init==nullptr){
			//std::cerr << __FILE__ << ':' << __FUNCTION__ << "warning: null ptr" << *this << std::endl;
			return *(init=begin());
		}
		else if (init==end()){
			std::cerr << __FILE__ << ':' << __FUNCTION__ << "warning: exceeded inits: " << *this << std::endl;
			return *(init=begin());
		}
		return *(++init);
	}

	UniTuple() : start(this->arr), init(nullptr){
		fill(0);
	};

	inline
	UniTuple(const UniTuple<T,N> & t) : start(this->arr), init(nullptr){
		set(t);
	};

	// Parasite
	template <size_t N2>
	UniTuple(UniTuple<T,N2> &tuple, size_t i=0): start(tuple.begin()+i), init(nullptr){
		if ((i+N)> N2){
			std::stringstream sstr;
			sstr << __FUNCTION__ << "<," << N << ">(" << __FUNCTION__ << "<," << N2 << ">),  index(" << i << ") overflow";
			std::cerr <<  sstr.str() << '\n';
			throw std::runtime_error(sstr.str());
		}
		/*
		*/
	};

	/*
	template <size_t N2>
	UniTuple(UniTuple<T,N2>::iterator, size_t count): start(tuple.begin()+i), init(nullptr){
		if ((i+N)> N2){
			std::stringstream sstr;
			sstr << __FUNCTION__ << "<," << N << ">(" << __FUNCTION__ << "<," << N2 << ">),  index(" << i << ") overflow";
			std::cerr <<  sstr.str() << '\n';
			throw std::runtime_error(sstr.str());
		}

	};
	*/

	/// Argument stack endpoint function; final step of variadic argument set(arg, ...) .
	void setIndexed(size_t i){
		//std::cout << __FUNCTION__ << " complete " << i << '\n';
		updateTuple();
	}

	/// Worker called by set(T2 arg, T2 arg2, ...)
	template<typename T2, typename ... TT>
	void setIndexed(size_t i, T2 arg, const TT &... rest){
		if (i>=N){
			std::stringstream sstr;
			sstr << "UniTuple<,"<< N << ">:" << __FUNCTION__ << " too many (" << i << ") arguments";
			throw std::runtime_error(sstr.str());
			return;
		}
		// std::cout << __FUNCTION__ << "N=" << N << " arg=" << arg << " remaining:" << i << std::endl;
		at(i) = static_cast<T>(arg);
		setIndexed(i+1, rest...);
	}


	// consider

private:

	T arr[N];

};

template <class T, size_t N>
std::ostream & operator<<(std::ostream & ostr, const UniTuple<T,N> & tuple){
	return tuple.toStream(ostr);
}

/// An inexact heuristic for checking validity of derived classes.
/**
 *  Compares the size of the derived class to the size required by the N elements of type T.
 *  If class size is smaller, not enough elements have defined, for sure.
 *  Note that this does not imply the opposite: equal (or larger) class size
 *  does not quarantee that N elements of type T only have been defined.
 *
 *  \tparam T2 - a class derived from UniTuple<T,N>
 */
/*
template <class T>
static
bool checkTupleSize(bool throwException = true){

	const size_t tupleSize = T::size()*sizeof(typename T::value_t);

	std::cerr << __FUNCTION__ << ' ' <<  sizeof(T) << " vs " << tupleSize << '\n';

	return false;

	/// Possibly ok.
	if (sizeof(T) == tupleSize)
		return true;

	/// Possibly ok, but suspicious. More members defined.
	if (sizeof(T) > tupleSize){
		// warn?
		std::cerr << __FUNCTION__ << ' ' <<  sizeof(T) << " vs " << tupleSize << '\n';
		return true;
	}

	/// Certainly error.
	if (sizeof(T) < tupleSize){
		std::stringstream sstr;
		//const std::type_info &t = typeid(typename T::value_t);
		sstr << "UniTuple<,"  << T::size() << ">::" << __FUNCTION__ << ": " ;
		// typeid(T2).name
		sstr << "derived class " << typeid(T).name()  << " too small for " << T::size() << " members of type " << typeid(typename T::value_t).name();
		throw std::runtime_error(sstr.str());
		return false;
	}
}
*/



}  // drain


#endif
