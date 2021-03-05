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
 * Path.h
 *
 *  Created on: Jul 21, 2010
 *      Author: mpeura
 */

#ifndef DRAIN_PATH_H_
#define DRAIN_PATH_H_

#include <stdexcept>
#include <iostream>
#include <string>
#include <list>
#include <iterator>


#include "String.h" // ?
#include "UniTuple.h"
#include "Sprinter.h"



namespace drain {

/// Determines if separators will be stored in the path.
/*
 *
 *  In unix style, intermediate separators are accepted but not stored.
 *
 *  Note that technically, accepting leading, intermediate or trailing separators means also
 *  accepting \e empty path elements, respectively.
 *
 *  When using strings as path elements, the root is identfied with an empty string -- not actually with the separator char like '/'.
 *
 */
struct PathSeparatorPolicy : UniTuple<bool,3> {

	// Separator character
	char character;

	/// If true, allow leading empties (ie leading separators) // Accepts one. to start with.
	bool & acceptLeading;

	/// If true, allow empty elements in the middle (appearing as repeated separators)
	bool & acceptRepeated;

	/// If true, allow trailing empty elements (appearing as repeated separators)
	bool & acceptTrailing;

	PathSeparatorPolicy(char separator='/', bool lead=true, bool repeated=false, bool trail=true) :
			character(separator), acceptLeading(next()), acceptRepeated(next()), acceptTrailing(next()) {
		set(lead, repeated, trail);
		if (!separator)
			throw std::runtime_error("PathSeparatorPolict (char separator): separator=0, did you mean empty init (\"\")");
	}
	PathSeparatorPolicy(const PathSeparatorPolicy &policy) :  UniTuple<bool,3>(policy),
			character(policy.character), acceptLeading(next()), acceptRepeated(next()), acceptTrailing(next()) {
	};


};

inline
std::ostream & operator<<(std::ostream & ostr, const PathSeparatorPolicy & policy) {
	ostr << "Separator='" << policy.character << "', policy=" << policy.tuple();
	return ostr;
}


///
/**
 *
 *   \tparam T    - path element, eg. PathElement
 *   \tparam SEP  -
 *   \tparam ALEAD
 *   \tparam AREPEAT
 *   \tparam ATRAIL
 *
 */
template <class T,char SEP='/', bool ALEAD=true, bool AREPEAT=false, bool ATRAIL=true>
class Path : public std::list<T> {

public:

	typedef T elem_t;

	typedef std::list<Path<T> > list_t;

	const PathSeparatorPolicy separator;

	// Consider flags
	//inline	Path() : separator(SEP, ALEAD, AREPEAT, ATRAIL) {};
	/// Initialize with given path
	inline
	Path(const std::string & s="") : separator(SEP, ALEAD, AREPEAT, ATRAIL){ //: separator(separator) {
		appendPath(s, 0);
		//assign(s); // , sep) <- consider split here!
	};

	// Either this or previous is unneeded?
	template<typename ... TT>
	Path(const elem_t &elem, const TT &... rest){
		appendElems(elem, rest...);
	}


	inline
	Path(const char *s) : separator(SEP, ALEAD, AREPEAT, ATRAIL){ //: s, char separator='/') : separator(separator) {
		appendPath(s, 0); //assign(s);
	};

	/// Copy constructor. Note: copies also the separator.
	inline
	Path(const Path<T> & p) : std::list<T>(p), separator(p.separator){
	};


	virtual inline
	~Path(){};


	/// Note that an empty path is not a root.
	//  Todo: also accept empty path as a root? Perhaps no, because appending may cause relative.
	inline
	bool isRoot() const {
		return ((this->size()==1) && this->front().empty());
		//return separator.acceptLeading && ((this->size()==1) && this->front().empty());
	}

	/// Removes trailing empty elements, except for the root itself.
	void removeTrailing(){
		if ((this->size() > 1)){
			if (this->back().empty()){
				this->pop_back();
				removeTrailing();
			}
		}
	}

	/// Main method for adding elements.
	void appendElem(const elem_t & elem){

		if (!elem.empty()){
			// Always allow non-empty element
			// std::cerr << __FUNCTION__ << ":" << elem << '\n';
			if (!separator.acceptRepeated)
				removeTrailing();
			this->push_back(elem);
		}
		// elem EMPTY:
		else if (this->empty()){
			if (separator.acceptLeading)
				this->push_back(elem); // empty
			// std::cerr << __FUNCTION__ << " leading=" << separator.acceptLeading << " "<< separator.character  << '\n';
		}
		else if (this->back().empty()){
			//std::cerr << __FUNCTION__<< " repeating=" << separator.acceptRepeated << " " << separator.character << '\n';
			if (separator.acceptRepeated){
				this->push_back(elem);
			}
		}
		else if (separator.acceptTrailing){
			// std::cerr << __FUNCTION__ << " trailing="  << separator.acceptTrailing << " " << separator.character << '\n';
			this->push_back(elem); // empty
		}
		else {
			std::cerr << __FUNCTION__ << "? failed in appending: " << sprinter(*this) << '\n';
		}

	};

	template<typename ... TT>
	void appendElems(const elem_t & elem, const TT &... rest) {
		appendElem(elem);
		appendElems(rest...);
	}

	/// Clear the path and append elements.
	template<typename ... TT>
	void setElems(const elem_t & elem, const TT &... rest){
		this->clear();
		appendElems(elem, rest...);
	}

	/// Extract elements from the string, starting at position i.
	void appendPath(const std::string & p, size_t start=0){

		if (start == p.length()){
			// Last char in the string has been passed by one, meaning that: previous elem was empty
			// That is: the previous char was a separator.
			appendElems(elem_t()); // Try to append empty.
			return;
		}

		// Append next segment, ie. up to next separator char.
		size_t nextSep = p.find(separator.character, start);

		// Remaining string...
		if (nextSep==std::string::npos){
			// ... is a single element
			appendElem(p.substr(start));
			return;
		}
		else {
			// ... contains separator, hence contains several elements
			appendElem(p.substr(start, nextSep - start)); // maybe empty, nextSep==start
			appendPath(p, nextSep + 1);
		}

	}

	/// Replaces the full path
	void assign(const std::string & p){

		// Assignment could be done directly, if all accepted:
		// StringTools::split(p, *this, separator.character);
		// ... but then illegal paths could become accepted.

		this->clear();

		// Note: given empty string, nothing will be appended.
		// Especially, no empty element ("root"), either.
		if (p.empty())
			return;

		appendPath(p, 0);


	}

	/// Assigns a path directly with std::list assignment.
	/**
	 *  Should be safe, because separator policy is the same.
	 */
	Path<T> & operator=(const Path<T> & p){
		std::list<T>::operator=(p);
		//return assignPa(*this);
		return *this;
	}

	/// Conversion from str path type
	/**
	 *   Does separator checking, trims leading or trailing paths if needed.
	 */
	template <class T2>
	Path<T> & operator=(const Path<T2> & p){
		this->clear();
		for (typename Path<T2>::const_iterator it = p.begin(); it != p.end(); ++it) {
			appendElems(*it);
		}
		return *this;
	}


	inline
	Path<T> & operator=(const std::string & p){
		assign(p);
		return *this;
	}

	inline
	Path<T> & operator=(const char *p){
		assign(p);
		return *this;
	}

	// Note: this would be ambiguous!
	// If (elem_t == std::string), elem cannot be assigned directly, because string suggest full path assignment, at least
	// Path<T> & operator=(const elem_t & e)

	/// Append an element, unless empty string.
	/*
	inline
	Path<T> & operator<<(const char *elem){
		*this <<
	}
	*/

	/// Append an element. If path is rooted, allows empty (root) element only as the first.
	Path<T> & operator<<(const elem_t & elem){
		appendElem(elem);
		return *this;
	}

	inline
	Path<T> & operator<<(const Path<T> & path){
		this->insert(this->end(), path.begin(), path.end());
		return *this;
	}


	// Experimental
	template <class T2>
	Path<T> & operator<<(const T2 & strlike){
		appendPath((const std::string &) strlike);
		return *this;
	}


	/// Extract last element.
	Path<T> & operator>>(elem_t & e){
		e = this->back();
		this->pop_back();
		return *this;
	}

	/// Path is written like a list, adding the separator between items. Exception: root is displayed as separator.
	/**
	 *
	 *
	 *
	 */
	virtual
	std::ostream & toStream(std::ostream & ostr) const { //, char dirSeparator = 0) const {

		if (isRoot())
			ostr << separator.character;
		else {
			static const SprinterLayout layout("/", "", "", "");
			layout.arrayChars.separator = separator.character;
			// SprinterLayout layout;
			// layout.arrayChars.set(0, separator.character,0);
			// layout.stringChars.fill(0);
			SprinterBase::sequenceToStream(ostr, *this, layout);
		}

		//SprinterBase::toStream(ostr, *this, layout);
		return ostr;
	}

	virtual
	std::string  str() const {
		std::stringstream sstr;
		toStream(sstr);
		return sstr.str();
	}

	virtual
	operator std::string() const {
		return str();
	}

	std::ostream & debug(std::ostream & ostr = std::cout) const {
		//ostr << "Path<" << typeid(T).name() << "> " << separator << "\n";
		ostr << "Path elems:" << this->size() << " sep:" << separator << " typeid: " << typeid(T).name() << "\n";
		ostr << *this << '\n';
		int i=0;
		for (typename Path<T>::const_iterator it = this->begin(); it != this->end(); ++it) {
			//if (it->empty())
			//	ostr << "  " << i << '\t' << "{empty}" << '\n';		//else
			ostr << "    " << i << '\t' << *it << '\n';
			++i;
		}
		return ostr;
	}

protected:

	/// Terminal function for variadic templates
	void appendElems(){
	}

	/// Terminal function for variadic templates
	void setElems(){
	};

};



template <class T, char SEP='/', bool ALEAD=true, bool AREPEAT=false, bool ATRAIL=true>
inline
std::ostream & operator<<(std::ostream & ostr, const Path<T,SEP,ALEAD,AREPEAT,ATRAIL> & p) {
	return p.toStream(ostr);
}

template <class T>
inline
std::istream & operator>>(std::istream &istr, Path<T> & p) {
	std::string s;
	istr >> s;
	p = s;
	return istr;
}

/*
template <class T>
inline
Path<T> & operator<<(Path<T> & path, const Path<T> & path2){
	path.insert(path.end(), path2.begin(), path2.end());
	return path;
}
*/


}

#endif /* Path_H_ */

// Drain
