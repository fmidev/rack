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
#ifndef GLOBAL_FLAGS_H_
#define GLOBAL_FLAGS_H_

#include "Flags.h"
#include "Static.h"

namespace drain
{

/*
template <class E>
class MultiFlagger2 : public SingleFlagger<E> {


public:


	static inline
	Flags & getShared(){
		return Static::get<Flags,E>();
	}

	static inline
	Flagger::dict_t & getSharedDict(){
		return getShared().getDict();
	}


	MultiFlagger2() : SingleFlagger<E>(ownValue, getSharedDict()){
	};

	/// Sets value, ie. set or unsets all the flags.
	template <class T2>
	inline
	MultiFlagger2<T> & operator=(const T2 &x){
		assign(x);
		return *this;
	}

};
*/


// A Flagger with a global dict. Also provides a global Flagger, ie. Flagger with a global value and global dict.
/// Flags using a global dictionary.
/**
 *
 */
//
template <class T>
class GlobalFlags : public MultiFlagger<int> {


public:


	static inline
	MultiFlagger<int> & getShared(){
		return Static::get<MultiFlagger<int>,T>();
	}

	static inline
	const MultiFlagger<int>::dict_t & getSharedDict(){
		return getShared().getDict();
	}


	/*
	GlobalFlags() : MultiFlagger<int>(ownValue, getSharedDict()){
	};
	*/

	/// Sets value, ie. set or unsets all the flags.
	template <class T2>
	inline
	GlobalFlags<T> & operator=(const T2 &x){
		assign(x);
		return *this;
	}

	/// Add a new entry in the dictionary
	/**
	 *  \param i - if zero, call getFreeBit() to reserve a new, unused bit.
	 *
	 *
	 */
	// Consider adding a warning if value reserved
	// Consider adding a notif   if key already defined
	/*
	static inline
	int addEntry(const std::string & key, int i=0){
		return FlaggerBase<T>::addEntry(GlobalFlags<T>::getSharedDict(), key, i);
	}
	*/

	// System error (Network error, , memory error)



};


} // ::drain

#endif
