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
//#include <set>


namespace drain {


/// Tuple of N elements of type T
/**
 *  Uniformity means that all the elements are of same type unlike in std::tuple.
 *
 */
template <class T, size_t N=2>
class UniTuple {


public:

	typedef T  value_type;
	typedef UniTuple<T,N> tuple_t;

	typedef T* iterator;
	typedef const T* const_iterator;

	inline
	UniTuple() : start(this->arr), init(nullptr){
		fill(0);
	};


	template<typename S>
	inline
	UniTuple(std::initializer_list<S> l) : start(this->arr), init(nullptr){
		set(l);
	};




	inline
	UniTuple(const UniTuple<T,N> & t) : start(this->arr), init(nullptr){
		set(t);
	};

	inline
	~UniTuple(){};

	/// Proposed for tuples only; derived classes should not shadow this.
	tuple_t & assign(const tuple_t & t){
		assignSequence(t);
		return *this;
	}

	tuple_t & assign(const value_type & value){
		fill(value);
		updateTuple();
		return *this;
	}

	tuple_t & operator=(const tuple_t &t){
		assign(t);
		return *this;
	}

	tuple_t & operator=(const value_type & value){
		assign(value);
		return *this;
	}


	template<typename S>
	tuple_t & operator=(std::initializer_list<S> l){
		assignSequence(l);
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

	template<typename S>
	inline
	void set(std::initializer_list<S> l){
		assignSequence(l);
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
	UniTuple<T,N> & assignSequence(S & sequence){
		typename S::const_iterator cit = sequence.begin();
		iterator it = begin();
		while (cit != sequence.end()){
			if (it == end()){
				std::stringstream sstr;
				sstr << __FILE__ << ':' << __FUNCTION__ << ": run out of indices in assigning: " << *cit;
				throw std::runtime_error(sstr.str());
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
	void fill(T i){
		for (iterator it = begin(); it != end(); ++it){
			*it = i;
		}
	}

	inline
	void clear(){
		fill(T());
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

	void debug(std::ostream & ostr) const {
		ostr << "UniTuple<" << typeid(T).name() << sizeof(T) << ',' << N << ">: {" << *this << '}';
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


	// Parasite
	template <size_t N2>
	UniTuple(UniTuple<T,N2> &tuple, size_t i=0): start(tuple.begin()+i), init(nullptr){
		if ((i+N)> N2){
			std::stringstream sstr;
			debug(sstr);
			sstr << ": constructor index[" << i << "] overflow with referenced tuple: ";
			tuple.debug(sstr);
			std::cerr <<  sstr.str() << '\n';
			throw std::runtime_error(sstr.str());
		}
		/*
		*/
	};

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
			debug(sstr);
			sstr << ':' << __FUNCTION__ << "(" << i << ',' << arg << ", ...), index overflow";
			// TODO: dump rest
			throw std::runtime_error(sstr.str());
			return;
		}
		// std::cout << __FUNCTION__ << "N=" << N << " arg=" << arg << " remaining:" << i << std::endl;
		at(i) = static_cast<T>(arg);
		setIndexed(i+1, rest...);
	}

private:

	/// Main container
	T arr[N];

};

template <class T, size_t N>
std::ostream & operator<<(std::ostream & ostr, const UniTuple<T,N> & tuple){
	return tuple.toStream(ostr);
}




}  // drain


#endif
