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
#ifndef ODIM_PATH_MATCHER
#define ODIM_PATH_MATCHER

#include "ODIMPath.h"


namespace rack {

class ODIMPathElemMatcher : public ODIMPathElem {

public:

	inline
	ODIMPathElemMatcher(group_t group = ROOT, index_t index = 0, index_t indexMax = 0xffff) :
		ODIMPathElem(group, index), indexMax(indexMax) {
	};


	inline
	ODIMPathElemMatcher(const ODIMPathElemMatcher &e) : ODIMPathElem(e), indexMax(e.indexMax) {
	}

	inline
	ODIMPathElemMatcher(const std::string &s) : indexMax(0xffff) {
		set(s);
	}


	/// Test if the elem has the same group, and elem.index is within [index,indexMax].
	bool test(const ODIMPathElem & elem) const;

	// Applied only as a upper limit in path matching.
	index_t indexMax;

	virtual
	std::ostream & toOStr(std::ostream & sstr) const;


protected:

	virtual
	void extractIndex(const std::string &s);


};

/// Test if a path partially fits group and index range.
/**
 *
 *  Uses index range [index,indexMax] in testing path elements of same group.
 *
 */
class ODIMPathMatcher : public drain::Path<ODIMPathElemMatcher> {

public:

	ODIMPathMatcher(const std::string & path = "") : drain::Path<ODIMPathElemMatcher>(path) {
		//set(path);
	}

	ODIMPathMatcher(const char * path) : drain::Path<ODIMPathElemMatcher>(path) {
		//set(path);
	}

	ODIMPathMatcher(const ODIMPathMatcher & matcher) : drain::Path<ODIMPathElemMatcher>(matcher) {
		//set(path);
	}


	/// Match the leading part of \c path , if \c matcher starts with root. Else, match the trailing part.
	static
	bool match(const rack::ODIMPathMatcher & matcher, const rack::ODIMPath & path);

	/// Match leading part of \c path.
	static
	bool matchHead(const rack::ODIMPathMatcher & matcher, const rack::ODIMPath & path);

	/// Match trailing part of \c path.
	static
	bool matchTail(const rack::ODIMPathMatcher & matcher, const rack::ODIMPath & path);

protected:

	/// Given a string starting with a numeral, try to extract an index range <min>:<max> .
	//void extractIndex(const std::string &s){

};





}  // namespace rack


#endif

// Rack
