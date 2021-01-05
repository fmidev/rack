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


//#include "drain/util/Dictionary.h"

#include "ODIMPath.h"


namespace rack {

/// Element in a chain in testing a path for a match.
/*
 *  Index range [index,indexMax] is applied in testing path elements of a same group.
 *  \see ODIMPathMatcher
 */
class ODIMPathElemMatcher : public ODIMPathElem {

public:

	inline
	ODIMPathElemMatcher(group_t group = ROOT, index_t index = 0, index_t indexMax = 0xffff) :
		ODIMPathElem(group, index), indexMax(indexMax),  flags(this->group, ODIMPathElem::getDictionary(), '|'){
		//flags = group;
	};


	inline
	ODIMPathElemMatcher(const ODIMPathElemMatcher &e) : ODIMPathElem(e), indexMax(e.indexMax),
	flags(this->group, ODIMPathElem::getDictionary(), '|') {
		//flags = e.flags.value;
		group  = e.group;
	}

	inline
	ODIMPathElemMatcher(const std::string &s) : indexMax(0xffff), flags(this->group, ODIMPathElem::getDictionary(), '|') {
		set(s);
	}

	//  ambiguous! index 5 could mean range 5:5 or 0:0xffff ?
	/*
	inline
	ODIMPathElemMatcher & operator=(const ODIMPathElemMatcher & elem){
		index = 0;
		indexMax = 0xffff;
		group = elem.getType();
		return *this;
	}
	*/

	inline
	bool isSingle() const {
		return (!isIndexed()) || (index == indexMax);
	}

	inline
	ODIMPathElemMatcher & operator=(ODIMPathElem::group_t g){
		index = 0;
		indexMax = 0xffff;
		group = g;
		return *this;
	}

	/// Test if the elem has the same group, and elem.index is within [index,indexMax].
	bool test(const ODIMPathElem & elem) const;

	// Applied only as a upper limit in path matching.
	index_t indexMax;

	drain::Flags flags;

	virtual
	std::ostream & toOStr(std::ostream & sstr) const;


protected:

	/// Extracts index range of type <index>[:<indexMax>]
	virtual
	void extractIndex(const std::string &s);


	virtual
	bool extractPrefix(const std::string &s, bool indexed);


};

/// Structure for testing if a path matches a given sequence of path elements.
/**
 *  Path will be matched from its start or end - not in the middle.
 *  Uses index range [index,indexMax] in testing path elements of same group.
 *
 */
class ODIMPathMatcher : public drain::Path<ODIMPathElemMatcher> {

public:

	/// Basic constructor
	ODIMPathMatcher() : drain::Path<ODIMPathElemMatcher>() {
	}

	/// Copy constructor
	ODIMPathMatcher(const ODIMPathMatcher & matcher) : drain::Path<ODIMPathElemMatcher>(matcher) {
	}

	/// Almost copy constructor
	ODIMPathMatcher(const std::string & path) : drain::Path<ODIMPathElemMatcher>(path) {
	}

	/// Almost copy constructor
	ODIMPathMatcher(const char * path) : drain::Path<ODIMPathElemMatcher>(path) {
	}


	/// Checks if corresponds to a single path, implying that all the index ranges are singletons.
	bool isLiteral() const;

	/// Convert to a single path, assuming uniqueness. Future option: extract all the enumerated paths.
	void extract(ODIMPath & path) const;

	/// Match the leading part of \c path , if \c matcher starts with root. Else, match the trailing part.
	bool match(const rack::ODIMPath & path) const;

	/// Match leading part of \c path.
	bool matchHead(const rack::ODIMPath & path) const;

	/// Match trailing part of \c path.
	bool matchTail(const rack::ODIMPath & path) const;

	/// Match single element. If matcher path does not contain it, return defaultValue.
	bool matchElem(const rack::ODIMPathElem & elem, bool defaultValue = true) const;

};


}  // namespace rack


#endif
