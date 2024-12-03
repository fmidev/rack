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
/*
 * ClassXML.h
 *
 *      Author: mpeura
 */



#ifndef DRAIN_CLASS_XML
#define DRAIN_CLASS_XML

#include <ostream>

//#include <drain/Sprinter.h>
//#include <drain/FlexibleVariable.h>

#include "EnumFlags.h"
#include "ReferenceMap.h"
#include "TreeUnordered.h"

namespace drain {



/// Container for style classes. Essentially a set of strings, with space-separated output support.
/**
 *
 */
class ClassListXML : public std::set<std::string> {

public:

	template <typename ... TT>
	inline
	ClassListXML(const TT &... args){
		add(args...);
	};

	/// Add one or several classes.
	template <typename ... TT>
	inline
	void add(const std::string & arg, const TT &... args) {
		if (!arg.empty()){
			insert(arg);
		}
		add(args...);
	};

	template <typename ... TT>
	inline
	void add(const char *arg, const TT &... args) {
		add(std::string(arg), args...);
	}

	/// Add one or several classes.
	/**
	 *   \tparam E - enum type expected (but unchecked)
	 *
	 *   Issues compile time error if type not applicable, or dictionary undefined.
	 *   Enum types should be used, although template allows any types, but strict
	 */
	template <typename E, typename ...TT>
	inline
	void add(const E & arg, const TT &... args) {
		insert(drain::EnumDict<E>::dict.getKey(arg));
		add(args...);
	};


	/* Compiler cannot derive (inner?) template
	template <typename T, typename ... TT>
	inline  // drain::EnumFlagger<
	void add(const typename drain::EnumFlagger<SingleFlagger<T> >::ivalue_t & enumArg, const TT &... args) {
		// std::cerr << "ENUMS:" << enumArg.str() << '\n';
		insert(enumArg.str());
		add(args...);
	};

	template <typename EE>
	inline  // drain::EnumFlagger<
	void add2(const typename SingleFlagger<EE>::ivalue_t & enumArg) {
		// std::cerr << "ENUMS:" << enumArg.str() << '\n';
		insert(enumArg.str());
		// add(args...);
	};
	*/

	inline
	bool has(const std::string & arg) const {
		return (find(arg) != end());
	};

	inline
	bool has(const char *arg) const {
		return (find(arg) != end());
	};

	template <typename E>
	inline
	bool has(const E & arg) const {
		return (find(drain::EnumDict<E>::dict.getKey(arg)) != end());
	};


	inline
	void remove(const std::string & arg) {
		iterator it = find(arg);
		if (it != end()){
			erase(it);
		}
	};

	/// Uses spaces as separators.
	static
	const SprinterLayout layout; //  = {" "}; // , "\n", "=", ""};

protected:

	inline
	void add(){};

};


inline
std::ostream & operator<<(std::ostream &ostr, const ClassListXML & cls){
	// static const SprinterLayout layout = {" "}; // , "\n", "=", ""};
	Sprinter::sequenceToStream(ostr, (const std::set<std::string> &)cls, ClassListXML::layout);
	return ostr;
}





}  // drain::

#endif /* TREEXML_H_ */

