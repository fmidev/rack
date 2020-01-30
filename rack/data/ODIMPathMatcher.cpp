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

#include <drain/util/Log.h>
#include <drain/util/TextReader.h>

#include "ODIMPathMatcher.h"

namespace rack {

void ODIMPathElemMatcher::extractIndex(const std::string &s){

	drain::Logger mout(__FUNCTION__, __FILE__);

	std::stringstream sstr(s);
	// sstr >> this->index;
	this->index = 0;
	this->indexMax = 0xffff;
	if (drain::TextReader::scanSegmentToValue(sstr, ":", this->index)){
		//this->indexMax = this->index;
		sstr >> this->indexMax; // can this fail? For example with "1:"
		//std::cout << "KUKKUU: " << this->index << "..." << this->indexMax << std::endl;
	}
	else {
		this->indexMax = this->index;
	}


	if (this->indexMax < this->index){
		mout.warn() << "indexMax(" << this->indexMax << ") < index(" << this->index << ')';
		mout << ", adjusting indexMax=" << this->index << mout.endl;
		this->indexMax = this->index;
		//mout.warn() << " -> setting " << *this << mout.endl;
	}

	//else this->indexMax = this->index;

}


bool ODIMPathElemMatcher::test(const ODIMPathElem & elem) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (elem.group != this->group){
		return false;
	}
	else if (elem.isIndexed()){
		// same group, indexed (DATASET, DATA, QUALITY)
		mout.debug(1) <<  this->index  << '(' << elem.index << ')' << this->indexMax << '!';
		if (elem.index < this->index){
			mout << "below " <<  this->index  << '!' << mout.endl;
			return false;
		}
		else if (elem.index > this->indexMax){
			mout << "above " <<  this->indexMax << '!' << mout.endl;
			return false;
		}
		else
			return true;
	}
	else { // same group, not indexed (WHAT, WHERE, HOW, ARRAY)
		return true;
	}

}


std::ostream & ODIMPathElemMatcher::toOStr(std::ostream & sstr) const {

	/// Step 1: prefix (by group type)
	sstr << getPrefix();

	/// Step 2: index
	if (isIndexed(this->group))
		sstr << this->index << ':' << this->indexMax;
	//sstr << '{' << this->index << '}';

	return sstr;
}

// const rack::ODIMPathMatcher & matcher
bool ODIMPathMatcher::match(const rack::ODIMPath & path) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (this->empty())
		return true;
	else if (this->front().isRoot())
		return matchHead(path);
	else
		return matchTail(path);
}


//const rack::ODIMPathMatcher & matcher
bool ODIMPathMatcher::matchHead(const rack::ODIMPath & path)  const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	//mout.debug() << "matcher: " << *this << mout.endl;
	mout.debug() << "path:    " << path    << mout.endl;

	rack::ODIMPathMatcher::const_iterator mit = this->begin();
	if (mit->isRoot())
		++mit;

	rack::ODIMPath::const_iterator pit = path.begin();
	if (pit->isRoot())
		++pit;

	while ((mit!=this->end()) && (pit!=path.end())){
		mout.debug(1) << *mit << ":\t" << *pit;
		if (!mit->test(*pit)){
			mout << '*' << mout.endl;
			return false;
		}
		mout << mout.endl;
		++mit, ++pit;
	}

	return (mit==this->end());
}

bool ODIMPathMatcher::matchTail(const rack::ODIMPath & path) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	//mout.debug() << "matcher: " << *this << mout.endl;
	mout.debug() << "path:    " << path    << mout.endl;

	rack::ODIMPathMatcher::const_reverse_iterator mit = this->rbegin();
	rack::ODIMPath::const_reverse_iterator pit = path.rbegin();

	while ((mit!=this->rend()) && (pit!=path.rend())){
		//mout.debug(1) << *mit << ":\t" << *pit;
		if (!mit->test(*pit)){
			mout.debug(1) << *mit << ":\t*" << *pit << mout.endl;
			// mout << '*' << mout.endl;
			return false;
		}
		//mout << '%' << mout.endl;
		mout.debug(1) << *mit << ":\t%" << *pit << mout.endl;
		++mit, ++pit;
	}
	//mout << mout.endl;

	// return true;
	return (mit==this->rend());
}

}  // namespace rack



