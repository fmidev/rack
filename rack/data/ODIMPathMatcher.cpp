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

#include "drain/util/Log.h"
#include "drain/util/TextReader.h"

#include "ODIMPathMatcher.h"

namespace rack {

/*
void parse(const std::string & path){
}
*/

bool ODIMPathMatcher::ensureLimitingSlash(){

	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.unimplemented("implementation postponed/cancelled");

	/*
	if (empty()){
		push_front(ODIMPathElem::ROOT);
		mout.warn() << "empty matcher, added root -> '" << *this << "'");
		return true;
	}

	if (front().empty() && back().empty()){
		mout.info() << "no leading or trailing separator char '" << separator.character << "'");
		push_front(ODIMPathElem::ROOT);
		mout.note() << "added leading (root) -> '" << *this << "'");
		return true;
	}
	*/

	return false;

}

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
		mout.warn("indexMax(", this->indexMax, ") < index(", this->index, "), adjusting indexMax=", this->index);
		this->indexMax = this->index;
		//mout.warn( << " -> setting " << *this);;
	}

	//else this->indexMax = this->index;

}

bool ODIMPathElemMatcher::extractPrefix(const std::string & prefix, bool indexed){

	drain::Logger mout(__FUNCTION__, __FILE__);

	//mout.warn( << "current: " << flags << ", setting prefix=" << prefix << ", indexed=" << indexed);;

	// First, check if it is a simple, single prefix (conforming to base class ODIMPathElem)
	if (ODIMPathElem::extractPrefix(prefix, indexed)){
		//flags = this->group;
		//mout.warn( << " -> setting single-type " << prefix << "=" << group << " => " << flags);
		//flags.keysToStream(std::cout);
		//flags.valuesToStream(std::cout);
		return true;
	}

	try {
		//mout.warn( << "current: " << flags;
		//flags = prefix;
		//flags.set(prefix);
		//flags.set(flags.getValue(prefix));
		flags.assign(flags.getValue(prefix));
		// mout.warn( << " => flags=" << flags << '=' << flags.value );
		if (indexed){
			if (flags.isSet(ODIMPathElem::ARRAY)){ //  value & ODIMPathElem::ARRAY){
				mout.debug(" -> adjusting 'data' to indexed 'data[]'");
				flags.unset(ODIMPathElem::ARRAY);
				flags.set(ODIMPathElem::DATA);
			}
		}
		else {
			if (flags.isSet(ODIMPathElem::DATA)){ // if (flags.value & ODIMPathElem::DATA){
				mout.debug(" -> adjusting indexed 'data[]' to 'data'");
				flags.unset(ODIMPathElem::DATA);
				flags.set(ODIMPathElem::ARRAY);
			}
		}

		mout.debug(" -> set ", prefix, " ==> ", flags, '=', flags.value);
		return true;
	}
	catch (const std::runtime_error & e) {
		mout.warn("could not set '", prefix, "' ==> ?", flags);
		mout.note(" -> dict: ", ODIMPathElem::getDictionary());
		mout.note(" -> flag-sep: ", flags.separator);
		mout.note(" -> dict-sep: ",  ODIMPathElem::getDictionary().separator);
		mout.note(" -> value: ",  flags.getValue(prefix, '|'));
	return false;
	}

}

bool ODIMPathElemMatcher::test(const ODIMPathElem & elem) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (!elem.belongsTo(this->flags.value)){
		//if (elem.group != this->group){
		//mout.warn(  " elem type of ",  elem,  '~',  elem.getType(),  " not in ",  flags,  '~',  this->flags.value);
		return false;
	}
	else if (elem.isIndexed()){
		// same group, indexed (DATASET, DATA, QUALITY)
		//mout.debug2();
		//mout.warn();
		mout.debug2() <<  this->index  << '(' << elem.index <<  ')' <<  this->indexMax << '!';
		if (elem.index < this->index){
			mout << "below " << this->index << '!' << mout.endl;
			return false;
		}
		else if (elem.index > this->indexMax){
			mout << "above " << this->indexMax << '!' << mout.endl;
			return false;
		}
		else
			return true;
	}
	else { // same group, not indexed (WHAT, WHERE, HOW, ARRAY)
		return true;
	}

}


std::ostream & ODIMPathElemMatcher::toStream(std::ostream & ostr) const {

	//for (dict_t::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it){}
	ostr << flags; // << flags.value;
	//flags.keysToStream(ostr); // '|' !

	// returns true if any of thflagsem elems is indexed (DATASET | DATA | QUALITY);
	if (ODIMPathElem::isIndexed(flags.value))
		ostr << this->index << ':' << this->indexMax;

	return ostr;
}




/// Checks if corresponds to a single path, implying that all the index ranges are singletons.
bool ODIMPathMatcher::isLiteral() const {

	for (const_iterator it=this->begin(); it!=this->end(); ++it){
		if (!it->isSingle())
			return false;
	}

	return true;
}

/// Extracts a single, "deterministic" path only. TODO: enumerate, extract maximally N branches.
void ODIMPathMatcher::extract(ODIMPath & path) const {
	drain::Logger mout(__FUNCTION__, __FILE__);

	for (const_iterator it=this->begin(); it!=this->end(); ++it){
		if (!it->isSingle())
			mout.warn("elem has range: ", *it, ", using min index=", it->index);
		path << *it;
	}
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



bool ODIMPathMatcher::matchHead(const rack::ODIMPath & path)  const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	//mout.debug() << "matcher: " << *this);
	mout.debug3( "path:    ", path   );

	rack::ODIMPathMatcher::const_iterator mit = this->begin();
	if (mit->isRoot())
		++mit;

	rack::ODIMPath::const_iterator pit = path.begin();
	if (pit->isRoot())
		++pit;

	while ((mit!=this->end()) && (pit!=path.end())){
		mout.debug3()<<  *mit <<":\t" << *pit;
		if (!mit->test(*pit)){
			mout << '*';
			return false;
		}
		mout << mout.endl;
		++mit, ++pit;
	}

	return (mit==this->end());
}

bool ODIMPathMatcher::matchTail(const rack::ODIMPath & path) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	//mout.debug() << "matcher: " << *this);
	mout.debug3("path: ", path);

	rack::ODIMPathMatcher::const_reverse_iterator mit = this->rbegin();
	rack::ODIMPath::const_reverse_iterator pit = path.rbegin();

	while ((mit!=this->rend()) && (pit!=path.rend())){
		//mout.debug2( *mit, ":\t", *pit;
		if (!mit->test(*pit)){
			mout.debug3( *mit, ":\t no... ", *pit);
			// mout, '*');
			return false;
		}
		//mout, '%');
		mout.debug3(*mit, ":\t YES... ", *pit);
		++mit, ++pit;
	}
	//mout);

	// return true;
	return (mit==this->rend());
}

bool ODIMPathMatcher::matchElem(const rack::ODIMPathElem & elem, bool defaultValue) const {

	for (ODIMPathMatcher::const_iterator it = begin(); it != end(); ++it){
		if (it->is(elem.getType())){
			return it->test(elem);
		}
	}

	return defaultValue;
}

}  // namespace rack



