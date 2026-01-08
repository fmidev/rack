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

#include <drain/UniTuple.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <list>
#include <iterator>


#include <drain/String.h> // ?
#include <drain/Sprinter.h>



namespace drain {

/// Determines if separators will be stored in the path.
/*
 *
 *  In unix style, intermediate separators are accepted but not stored.
 *
 *  Note that technically, accepting leading, intermediate or trailing separators means also
 *  accepting \e empty path elements, respectively.
 *
 *  When using strings as path elements, the root is identified with an empty string -- not actually with the separator char like '/'.
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



/**
 *
 *   \tparam T    - path element, eg. PathElement
 *   \tparam SEP  - separator char
 *   \tparam ALEAD - accept leading separator
 *   \tparam AREPEAT - accept repeated separators
 *   \tparam ATRAIL - accept trailing separator
 *
 */
template <class T,char SEP='/', bool ALEAD=true, bool AREPEAT=false, bool ATRAIL=true>
class Path : public std::list<T> {

public:

	typedef T elem_t;

	typedef Path<T,SEP,ALEAD,AREPEAT,ATRAIL> path_t;

	typedef std::list< path_t > list_t;

	// typedef Path<T,SEP,ALEAD,AREPEAT,ATRAIL> path_t;

	// const PathSeparatorPolicy separator;
	static
	const PathSeparatorPolicy separator;

	inline
	Path(){ //  : separator(SEP, ALEAD, AREPEAT, ATRAIL){
	};

	/// Copy constructor.
	inline
	Path(const path_t & p) : std::list<T>(p){ // , separator(p.separator){
	};

	/// Secondary copy constructor. Handy for creating a parent path, for example.
	inline
	Path(typename path_t::const_iterator it, typename path_t::const_iterator it2){ //  : separator(SEP, ALEAD, AREPEAT, ATRAIL) {
		while (it != it2){
			append(*it);
			++it;
		}
	};

	/// Initialize with a path.
	// All the elements are treated as paths.
	template <typename ... TT>
	inline
	Path(const path_t & arg, const TT &... args){ //  : separator(SEP, ALEAD, AREPEAT, ATRAIL){
		append(arg, args...);
	};

	/// Initialize with a path.
	// All the elements are treated as paths.
	template <typename ... TT>
	inline
	Path(const std::string & arg, const TT &... args){ // : separator(SEP, ALEAD, AREPEAT, ATRAIL){
		append(arg, args...);
	};

	/// Initialize with a path.
	// All the elements are treated as paths.
	template <typename ... TT>
	inline
	Path(const char * arg, const TT &... args){ // : separator(SEP, ALEAD, AREPEAT, ATRAIL){
		append(arg, args...);
	};


	/// Why the three above instead of this?
	//  Answer: to prevent elem and string
	/*
	template <typename ... TT>
	inline
	Path(const TT &... args) : separator(SEP, ALEAD, AREPEAT, ATRAIL){ //: separator(separator) {
		append(args...);
	};
	*/




	virtual inline
	~Path(){};

	// Design principle: handle all strings as paths!
	// This means that if path elements are strings, assigning elements goes through an "extra" path check.
	// If string arguments were accepted directly as elements, separator characters could be passed in the elements.

	template <typename ... TT>
	void set(const TT &... args) {
		this->clear();
		append(args...);
	}

	/// Append path with element(s), path(s) or string(s).
	//  needed?
	template <typename T2, typename ... TT>
	void append(const T2 & arg, const TT &... args) {
		//append_(arg); // replace with appendElem(elem), remove _append?
		appendElem(arg); // replace with appendElem(elem), remove _append?
		append(args...);
	}

	template <typename ... TT>
	void append(const path_t &p, const TT &... args) {
		this->insert(this->end(), p.begin(), p.end());
		append(args...);
	}

	// NEW
	template <typename ... TT>
	void append(char c, const TT &... args) {
		appendElem(c);
		append(args...);
	}

	template <typename ... TT>
	void append(const char * arg, const TT &... args) {
		_appendPath(arg, 0);
		append(args...);
	}

	template <typename ... TT>
	void append(const std::string &arg, const TT &... args) {
		_appendPath(arg, 0);
		append(args...);
	}


	/// Specialized handler for strings (note, possibly: elem_t==std:::string)
	inline
	void appendPath(const char *p){
		_appendPath(p, 0);
	}

	// Reverse convenience...
	inline
	void appendPath(const path_t &p){
		appendPath(p);
	}


	/// Main method for adding elements.
	void appendElem(const elem_t & elem){

		if (!elem.empty()){
			// Always allow non-empty element
			// std::cerr << __FUNCTION__ << ":" << elem << '\n';
			if (!separator.acceptRepeated){
				//std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << ":" << " validate " << *this << " + " << elem << '\n';
				//removeTrailing();
				trimTail();
			}
				//stripTail();
			this->push_back(elem);
		}
		// elem is EMPTY: -----------------------------------------------------------
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

	/// To be specialized in subclasses.
	template <typename T2>
	inline
	void appendElem(const T2 & elem){
		appendElem(elem_t(elem));
	}

	/// Returns true, if the path as only one element which is empty. An empty path is not a root.
	//
	/**
	 *
	 *
	 *  \see hasRoot()s
	 *  \see ODIMPath::isRoot()
	 *
	 *  Todo: also accept empty path as a root? Perhaps no, because appending creates a relative path.
	 */
	inline
	bool isRoot() const {
		return ((this->size()==1) && this->front().empty());
		//return separator.acceptLeading && ((this->size()==1) && this->front().empty());
	}

	/// Returns true, if the path is not empty and the  first element is empty.
	/**
	 *
	 *  \see isRoot()
	 *
	 */
	inline
	bool hasRoot() const {
		if (!this->empty()){
			return this->front().empty();
		}
		else {
			return false;
		}
	}

	/// Returns true, if the path is not empty and the  first element is empty.
	/**
	 *
	 *  \see isRoot()
	 *
	 */
	inline
	path_t & ensureRoot(){
		if (!this->hasRoot()){
			this->push_front(elem_t());
		}
		return *this;
	}


	/// Removes leading empty elements. The resulting string presentation will not start with the separator.
	/**
	 *  After this operation there is no root, ie. there is no leading separator.
	 *
	 */
	void trimHead(bool COMPLETE=false){
		const size_t minLength = COMPLETE ? 0 : 1;
		while (this->size() > minLength){
		//while (!this->empty()){
			if (!this->front().empty()){
				return;
			}
			this->pop_front();
			//this->trimHead();
		}
	}


	/// Removes trailing empty elements. The resulting string presentation will not end with the separator.
	void trimTail(bool COMPLETE=false){ //
		const size_t minLength = COMPLETE ? 0 : 1;
		while (this->size() > minLength){
		//while (!this->empty()){
			if (!this->back().empty()){
				return;
			}
			this->pop_back();
			//this->trimTail();
		}
	}

	/// ORIGINAL. Removes trailing empty elements, except for the root itself.
	/** keep this for a while
	void removeTrailing(){
		if ((this->size() > 1)){
			if (this->back().empty()){
				this->pop_back();
				removeTrailing();
			}
		}
	}
	*/

	/// Remove duplicates of empty elements from the start and the end.
	/**
	 */
	inline
	void simplify(){
		simplifyHead();
		simplifyTail();
	}

	/// Remove duplicates of empty elements from the start.
	/**
	 */
	void simplifyHead(){

		if (this->empty())
			return;

		if (this->front().empty()){
			typename path_t::const_iterator it0 = ++this->begin();
			typename path_t::const_iterator it = it0;
			while (it != this->end()) {
				if (!it->empty()){
					break;
				}
				++it;
			}
			if (it != it0)
				this->erase(it0,it);
		}


	}

	/// Remove duplicates of empty elements from the end.
	/**
	 */
	void simplifyTail(){

		if (this->empty())
			return;

		if (this->back().empty()){
			typename path_t::const_iterator it0 = --this->end();
			typename path_t::const_iterator it = it0;

			while (it != this->begin()) {
				--it;
				if (!it->empty()){
					++it;
					break;
				}
			}

			if (it != it0)
				this->erase(it,it0);
		}


	}



	/// Assigns a path directly with std::list assignment.
	/**
	 *  Should be safe, because separator policy is the same.
	 */
	path_t & operator=(const path_t & p){

		if (&p != this){
			// Yes, important self-check.
			// Otherwise clear() also makes the"source" empty.
			set(p);
		}
		// std::list<T>::operator=(p);
		// return assignPa(*this);
		return *this;
	}

	/// Conversion from str path type
	/**
	 *   Does separator checking, trims leading or trailing paths if needed.
	 */
	template <class T2>
	 path_t & operator=(const Path<T2> & p){
		set(p);
		return *this;
	}


	inline
	 path_t & operator=(const std::string & p){
		set(p);
		return *this;
	}

	inline
	 path_t & operator=(const char *p){
		set(p);
		return *this;
	}

	// Note: this would be ambiguous!
	// If (elem_t == std::string), elem cannot be assigned directly, because string suggest full path assignment, at least
	//  path_t & operator=(const elem_t & e)

	/// Append an element, unless empty string.
	/*
	inline
	 path_t & operator<<(const char *elem){
		*this <<
	}
	*/


	// TODO: replace these with single
	/*
	template <class T2>
	path_t & operator<<(const T2 & arg){
		append(args);
		return *this;
	}
	*/


	/// Append an element. If path is rooted, allows empty (root) element only as the first.
	 path_t & operator<<(const elem_t & elem){
		appendElem(elem);
		return *this;
	}

	inline
	 path_t & operator<<(const  path_t & path){
		this->insert(this->end(), path.begin(), path.end());
		return *this;
	}


	// Experimental
	template <class T2>
	 path_t & operator<<(const T2 & strlike){
		appendPath((const std::string &) strlike);
		return *this;
	}


	/// Extract last element.
	 path_t & operator>>(elem_t & e){
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
			Sprinter::sequenceToStream(ostr, *this, layout);
		}

		//Sprinter::toStream(ostr, *this, layout);
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
		for (typename  path_t::const_iterator it = this->begin(); it != this->end(); ++it) {
			//if (it->empty())
			//	ostr << "  " << i << '\t' << "{empty}" << '\n';		//else
			ostr << "    " << i << '\t' << *it << '\n';
			++i;
		}
		return ostr;
	}

protected:

	/// "Default" append function.
	/*
	void _append(const elem_t &elem){
		appendElem(elem);
	}


	//  Handler for all the rest (non elem_t) arguments.
	/// Handler for all the rest args, assumed convertable to elem_t.
	template <typename T2>
	void _append(const T2 & p){
		appendElem(p);
	}
	*/

	/// Extract elements from the string, starting at position i.
	void _appendPath(const std::string & p, size_t start=0){

		if (start == p.length()){
			// Last char in the string has been passed by one, meaning that: previous elem was empty
			// That is: the previous char was a separator.
			//appendElems(elem_t()); // Try to append empty.
			appendElem(elem_t()); // Try to append empty.
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
			_appendPath(p, nextSep + 1);
		}

	}


protected:

	/// Terminal function for variadic templates
	void append(){
	}

	/// Terminal function for variadic templates
	//void appendElems(){
	//}

	/// Terminal function for variadic templates
	//void setElems(){
	// };

};



template <class T, char SEP='/', bool ALEAD=true, bool AREPEAT=false, bool ATRAIL=true>
inline
std::ostream & operator<<(std::ostream & ostr, const Path<T,SEP,ALEAD,AREPEAT,ATRAIL> & p) {
	return p.toStream(ostr);
}


template <class T, char SEP='/', bool ALEAD=true, bool AREPEAT=false, bool ATRAIL=true>
inline
std::istream & operator>>(std::istream &istr, Path<T,SEP,ALEAD,AREPEAT,ATRAIL> & p) {
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


template <class T, char SEP, bool ALEAD, bool AREPEAT, bool ATRAIL>
const PathSeparatorPolicy Path<T,SEP,ALEAD,AREPEAT,ATRAIL>::separator(SEP, ALEAD, AREPEAT, ATRAIL);

}

#endif /* Path_H_ */

// Drain
