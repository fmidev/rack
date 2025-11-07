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

// New design (2020)

#ifndef DRAIN_CMD_SEQ_H_
#define DRAIN_CMD_SEQ_H_

#include <iostream>
#include <list>

#include <drain/Sprinter.h>



namespace drain {

/// A base class for Script and Program
template <class T>
class CommandSequence : public std::list<T> {

public:

	typedef T entry_t;

	typedef std::list<T> list_t;

	// virtual consider
	// typename list_t::value_type & add(key, cmd) = 0;

	/// Check if this routine is defined.
	inline
	operator bool(){ return !this->empty(); };

};

template <class T>
inline
std::ostream & operator<<(std::ostream & ostr, const CommandSequence<T> & commands){

	/**
	 *  Array layout: elements (of the Sequence) will be separated by newline.
	 *  Map layout {,}: unused - or something is going wrong.
	 *  Pair layout : key=value
	 *  String layout: plain, because keys are strings but not desired to be hyphenated.
	 *  (Future extensions of Sprinter will separate keys and values?)
	 */
	static const SprinterLayout layout("\n", "{,}", "=", "");

	Sprinter::sequenceToStream(ostr, commands, layout);
	return ostr;
}




class Script : public CommandSequence<std::pair<std::string,std::string> > {

public:

	inline
	typename list_t::value_type & add(const std::string & key, const std::string & params=""){
	//typename Script::entry_t & Script::add(const std::string & cmd, const std::string & params){
		this->push_back(typename list_t::value_type(key, params));
		return back();
	}

};





} /* namespace drain */

#endif // CMD_SEQ
