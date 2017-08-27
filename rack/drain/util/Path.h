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

#include <iostream>
#include <string>
#include <list>
#include <iterator>

//#include "RegExp.h"
//#include "Path.h"


// // using namespace std;

namespace drain {


/// Atomary element in a path; not containing a separator.
/**  
 *  not to be separated
 */
struct PathElement : public std::string {

	PathElement(const std::string & s = ""){
		std::string::assign(s);
	}

	PathElement(const char *s){
		std::string::assign(s);
	}
	/*
	inline
	PathElement & assign(const std::string & p){
		std::string::assign(p);
		return *this;
	}
	*/



};

typedef std::list<PathElement> PathList;

struct PathSegment {

	PathSegment(char separator='/') : separator(separator){
	};

	PathSegment(const PathSegment & segment) : separator(segment.separator), first(segment.first), last(segment.last){
	};

	PathSegment(PathList::const_iterator b, PathList::const_iterator e, char separator='/') :
		separator(separator), first(b), last(e) {
	};


	char separator;

	PathSegment parent() const {

	  PathSegment p(first, last, separator);
	  if (first != last)
	    --p.last;
	  /*
	    else
	    throw std::runtime_exeption("PathSegment parent root");
	  }
	  */
  
	  return p;
	}

	PathSegment child() const {

	  PathSegment segment(last, last, separator);
	  if (first != last){
	    --segment.first;
	    if (segment.first == first){ // no change
	      ++segment.first;
	    }
	  }
	  /*
	    else
	    throw std::runtime_exeption("PathSegment parent root");
	  }
	  */

	  /*
	  static PathSegment c;
			c.separator = separator;
			c.first = last;
			--c.first;
			c.last = last;
	  */
	  return segment;
			
	}


	inline std::ostream & toOStream(std::ostream & ostr) const {
		char sep = 0;
		for (PathList::const_iterator it = first; it!=last; ++it){
			if (sep)
				ostr << sep;
			else
				sep = separator;
			//ostr << '[' << *it << ']';
			ostr << *it;
		}
		return ostr;
	}


protected:

	PathList::const_iterator first;
        PathList::const_iterator last;  // actually end (invalid)

	void set(PathList::const_iterator b, PathList::const_iterator e, char separator='/'){
		first = b;
		last = e;
		this->separator = separator;
	}

  //static
  //const PathList dummy;

};

inline
std::ostream & operator<<(std::ostream & ostr, const PathSegment & p) {
	return p.toOStream(ostr);
}


class Path : public PathList  {

public:

	/// Replaces instances of 'from' to 'to' in src, storing the result in dst.
	/** Safe. Uses temporary Path.
	 *
	 */
	Path (char separator = '/') : separator(separator){
	}

	Path (const Path & path) : separator(path.separator) {
		for (const_iterator it = path.begin(); it != path.end(); ++it) {
			push_back(*it);
		}
	}

	Path (const std::string & p, char separator = '/') : separator(separator){
		assign(p);
	}

	inline
	Path & operator=(const std::string & p){
		assign(p);
		return *this;
	};

	inline
	PathSegment segment() const {
	  return PathSegment(begin(), end(), separator);
	}

	inline
	PathSegment child() const {
	  return segment().child();
	}

	inline
	PathSegment parent() const {
	  return segment().parent();
	}


	char separator;

	inline
	Path & assign(const std::string & s){
  	   clear();
	   return appendSubString(s);
	}

	inline
	Path & append(const std::string & s){
	   return appendSubString(s);
	}

	/*
	inline
	Path & append(const PathElement & e){
	   push_back(e);
	   return *this;
	}
	*/


	/*
	inline
	std::ostream & toOStream(std::ostream & ostr) const {
		char sep = 0;
		for (const_iterator it = begin(); it!=end(); ++it){
			if (sep)
				ostr << sep;
			else
				sep = separator;
			//ostr << '[' << *it << ']';
			ostr << *it;
		}
		return ostr;
	}
	*/

protected:

	inline
	Path & appendSubString(const std::string & p, size_t i=0){
		const size_t j = p.find(separator, i);
		//cout << p << " => ";
		if (j == std::string::npos){
			//cout << p.substr(i) << endl;
		  if (i != p.length())
		    push_back(p.substr(i));
		}
		else {
			//cout << p.substr(i,j-i) << endl;
		  if (j != i){ // skip empty
			push_back(p.substr(i,j-i));
                  }
			appendSubString(p, j+1);
		}
		return *this;
	}

};



inline
std::ostream & operator<<(std::ostream & ostr, const Path & path) {
	//return PathSegment()::toOStream(path.begin(), path.end(), path.separator);
	return path.segment().toOStream(ostr);
}

}

#endif /* Path_H_ */

// Drain
