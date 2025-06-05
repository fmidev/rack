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

#include <cstddef>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include <string>
//#include <set>

#include <drain/StringBuilder.h>
#include <drain/TupleBase.h>
#include <drain/Type.h>  // Utils
// #include <drain/Sprinter.h>  lower ==  cannot use Sprinter, as long as SprinterLayouit uses UniTuple...
// #include <drain/VariableT.h> lower

namespace drain {



//class VariableT<VariableInitializer<VariableBase> >;
//class VariableT<VariableInitializer<ReferenceT<VariableBase> > >;
//class VariableT<ReferenceT<Castable> >;


/// Tuple of N elements of type T
/**
 *  Uniformity means that all the elements are of same type unlike in std::tuple.
 *
 */
template <class T, size_t N=2>
class UniTuple : public TupleBase<T,N> {


public:

	typedef T  value_type;   // why value_type, not value_t (ok, STL also )
	typedef UniTuple<T,N> tuple_t;
	static const size_t tuple_size = N;

	typedef T* iterator;
	typedef const T* const_iterator;


	inline
	UniTuple() : start(this->arr), init(nullptr){ // start(this->arr),
		this->fill(TupleType<value_type>::neutral_value);
	};


	template<typename ... TT>
	inline
	UniTuple(const TT &... args) : start(this->arr), init(nullptr){ //
		this->fill(TupleType<value_type>::neutral_value);
		this->set(args...);
	}


	/// Copy constructor
	inline
	UniTuple(const UniTuple<T,N> & t) : start(this->arr), init(nullptr){ // start(this->arr),
		this->set(t);
	};



	template<typename S>
	inline
	UniTuple(std::initializer_list<S> l) : start(this->arr), init(nullptr){ // start(this->arr),
		this->set(l);
	};



	virtual inline
	~UniTuple(){};

	tuple_t & operator=(const tuple_t &t){
		if (&t != this){ // this check should be unneeded, as it is in assign().
			this->assignSequence(t);
		}
		return *this;
	}

	tuple_t & operator=(const value_type & value){
		this->assign(value);
		return *this;
	}


	template<typename S>
	tuple_t & operator=(std::initializer_list<S> l){
		this->assignSequence(l);
		return *this;
	}




	virtual inline
	const_iterator begin() const override final {
		return start; // this->arr
		// return this->arr; //start;
	}

	virtual inline
	const_iterator end() const override final {
		return start + N; // this->arr
		// return this->arr + N;
	}

	virtual inline
	iterator begin() override final {
		return start; // this->arr
		// return this->arr; //start;
		//return (iterator)this;
	}

	virtual inline
	iterator end() override final {
		return start + N; // this->arr
		// return (iterator)this + N;
	}


	// Self-reference for casting
	inline
	const tuple_t & tuple() const{
		return *this;
	}


	// Self-reference for casting
	inline
	tuple_t & tuple(){
		return *this;
	}

	template<typename ... TT>
	inline
	tuple_t & tuple(const TT &... args){
		// tuple_t & tuple(const T & arg, const TT &... rest){
		// set(arg, rest);
		this->set(args...);
		return *this;
	}




	void debug(std::ostream & ostr) const {
		ostr << "UniTuple<" << typeid(T).name() << sizeof(T) << ',' << N << ">: {" << *this << '}';
	}


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
	UniTuple(UniTuple<T,N2> &tuple, size_t i): start(tuple.begin()+i), init(nullptr){ // 2023/04/24
		if ((i+N)> N2){
			throw std::runtime_error(drain::StringBuilder<>(drain::TypeName<UniTuple<T,N2> >::str(), "(", tuple, "): constructor index[", i,"] overflow with referenced tuple") );
		}
	};



private:

	/// Main container
	T arr[N];

};

/*
template <class T, size_t N>
std::ostream & operator<<(std::ostream & ostr, const UniTuple<T,N> & tuple){
	return tuple.toStream(ostr);
}
*/


template <class T, size_t N>
struct TypeName<UniTuple<T,N> > {

    static const std::string & str(){
		static const std::string name = drain::StringBuilder<>("UniTuple<", drain::TypeName<T>::str(), ',', N, ">");
        return name;
    }

};


/*
 // NOTE: cannot use Sprinter, as long as SprinterLayout uses UniTuple...
template <class T, size_t N>
std::ostream & Sprinter::toStream(std::ostream & ostr, const  UniTuple<T,N> & tuple, const SprinterLayout & layout) {
	return Sprinter::sequenceToStream(ostr, tuple, layout);
}
*/

}  // drain


#endif
