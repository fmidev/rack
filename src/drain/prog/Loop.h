/*

MIT License

Copyright (c) 2026 FMI Open Development / Markus Peura, first.last@fmi.fi

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
 * Loop.h
 *
 *  Created on: Jun 6, 2026
 *      Author: mpeura
 */

#ifndef DRAIN_PROG_LOOP
#define DRAIN_PROG_LOOP

#include <iostream>
#include <string>
#include <list>

#include <drain/StringTools.h>

namespace drain {


class Loop {
public:

	Loop(const std::string & key="", const std::string & values=""){
		set(key, values);
	}

	//typedef std::list<drain::Variable> valueList;
	typedef std::list<std::string> valueList;

	std::string key;

	Loop & set(const std::string & key, const std::string & values){
		this->key = key;
		drain::StringTools::split(values, this->values, ",");
		return *this;
	}

	// template <class C>
	// void set(const std::string & key, const std::string & values); //, const C & separator){

	inline
	const valueList & getValueList() const {
		return values;
	}

	typedef std::list<Loop> loopStack;

	/*
	static inline
	void traverse(const loopStack & stack){
		traverse(stack.begin(), stack.end());
	}
	*/

protected:


	//static inline
	//void traverse(loopStack::const_iterator it, loopStack::const_iterator itEnd);

	valueList values;

};

/*
void Loop::traverse(loopStack::const_iterator it, loopStack::const_iterator itEnd){

	std::cout << it->key << '\n';

	loopStack::const_iterator it2 = it;
	++it2;

	for (const auto & item: it->values){
		//std::cout << '\t' << it->key << ' ' << item << '\n';
		std::cout << '\t' << item << '\n';
		if (it2 != itEnd){
			Loop::traverse(it2, itEnd);
		}
	}

}
*/



}  // drain::

#endif // DRAIN_PROG_LOOP
