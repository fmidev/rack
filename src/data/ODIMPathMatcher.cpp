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


//bool
void ODIMPathElemMatcher::set(const std::string &s){

	drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.special<LOG_NOTICE>("setting: ", s, " --> ", std::string(s.begin(), it));

	reset();

	std::vector<std::string> args;
	drain::StringTools::split(s, args, '|'); // For now. (Future extension: '/' ?)

	// bool other = false;
	for (std::string & arg: args){
		// other |= !
		add(arg);
	}

	// mout.special<LOG_NOTICE>("set: ", s, " --> ", *this);


	return; // !other;
}


bool ODIMPathMatcher::ensureLimitingSlash(){

	drain::Logger mout(__FILE__, __FUNCTION__);

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
	ODIMPathElemMatcher::idx_range_t indexRange;
	ODIMPathElem::extractIndex(s, indexRange);
	this->index    = indexRange.min;
	this->indexMax = indexRange.max;
}

/*
template <>
char drain::TextReader::scanSegmentToValue(std::istream & istr, const std::string & endChars, ODIMPathElemMatcher::idx_range_t & dst){
}
*/

template <>
void ODIMPathElem::extractIndex(const std::string &s, ODIMPathElemMatcher::idx_range_t & idx){

	//std::stringstream sstr(s);
	//sstr >> idx;
	drain::Logger mout(__FILE__, __FUNCTION__);

	std::stringstream sstr(s);

	idx.set(0, INDEX_MAX);

	char c = drain::TextReader::scanSegmentToValue(sstr, ":", idx.min);
	if (c == ':'){
		sstr.get(); // swallow ':'
		sstr >> idx.max; // can this fail? For example with "1:"
		//mout.warn("extracted maxIndex ", this->indexMax, " c=", c);
	}
	else {
		// mout.warn("max == index == ", this->index);
		idx.max = idx.min;
	}


	if (idx.max < idx.min){
		mout.warn("indexMax(", idx.max, ") < index(", idx.min, "), adjusting indexMax=", idx.min);
		idx.max = idx.min;
		//mout.warn( << " -> setting " << *this);;
	}

	//else this->indexMax = this->index;

}

bool ODIMPathElemMatcher::extractPrefix(const std::string & prefix, bool indexed){

	drain::Logger mout(__FILE__, __FUNCTION__);

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

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!elem.belongsTo(this->flags.value)){
		//if (elem.group != this->group){
		//mout.warn(  " elem type of ",  elem,  '~',  elem.getType(),  " not in ",  flags,  '~',  this->flags.value);
		return false;
	}
	else if (elem.isIndexed()){
		// same group, indexed (DATASET, DATA, QUALITY)
		//mout.debug2();
		//mout.warn();
		// mout.debug2() <<  this->index  << '(' << elem.index <<  ')' <<  this->indexMax << '!';
		if (elem.index < this->index){
			mout.debug2("supplied index=", elem.index, " below ", this->index, "; range [", this->index, ':', this->indexMax, ']');
			//mout.debug2(this->index, '(', elem.index,  ')', this->indexMax, "! below ", this->index, '!');
			return false;
		}
		else if (elem.index > this->indexMax){
			mout.debug2("supplied index=", elem.index, " is above max ", this->indexMax, "; range [", this->index, ':', this->indexMax, ']');
			// mout.debug2(*this, ", see also:");
			// this->toStream(std::cerr);
			return false;
		}
		else
			return true;
	}
	else { // same group, not indexed (WHAT, WHERE, HOW, ARRAY)
		return true;
	}

}





/// Checks if corresponds to a single path, implying that all the index ranges are singletons.
bool ODIMPathMatcher::isLiteral() const {

	for (const ODIMPathElemMatcher & elem: *this){
		if (!elem.isSingle())
			return false;
	}

	return true;
}

/// Extracts a single, "deterministic" path only. TODO: enumerate, extract maximally N branches.
void ODIMPathMatcher::extractPath(ODIMPath & path) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	for (const ODIMPathElemMatcher & elem: *this){
		if (!elem.isSingle())
			mout.warn("elem has range: ", elem, ", using min index=", elem.index);
		path.appendElem(elem);
	}
}


// const rack::ODIMPathMatcher & matcher
bool ODIMPathMatcher::match(const rack::ODIMPath & path) const {

	//drain::Logger mout(__FILE__, __FUNCTION__);

	if (this->empty())
		return true;
	else if (this->front().isRoot())
		return matchHead(path);
	else
		return matchTail(path);
}



bool ODIMPathMatcher::matchHead(const rack::ODIMPath & path)  const {

	drain::Logger mout(__FILE__, __FUNCTION__);

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

	drain::Logger mout(__FILE__, __FUNCTION__);

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

	for (const ODIMPathElemMatcher & elemMatcher: *this){
		if (elemMatcher.is(elem.getType())){
			return elemMatcher.test(elem);
		}
	}

	/*
	for (ODIMPathMatcher::const_iterator it = begin(); it != end(); ++it){
		if (it->is(elem.getType())){
			return it->test(elem);
		}
	}
	*/

	return defaultValue;
}


std::ostream & ODIMPathElemMatcher::toStream(std::ostream & ostr) const {

	// for (dict_t::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it){}
	// ostr << flags; // << flags.value;
	// ostr << " or...";
	static const dict_t & dict = getDictionary();
	// ostr << 'G' << this->group << '=';
	char sep=0;
	for (group_t g: {WHAT, WHERE, HOW}){
		if ((this->group & g) == g){
			if (sep)
				ostr << sep;
			else
				sep = '|'; // Matcher Syntax
			ostr << dict.getKey(g);
		}
	}
	/*
	if (this->isIndexed(group) && this->belongsTo(ATTRIBUTE_GROUPS)){
		ostr << '+';
	}
	*/
	for (group_t g: {DATASET, DATA, QUALITY}){
		if ((this->group & g) == g){
			if (sep)
				ostr << sep;
			else
				sep = '|'; // Matcher Syntax
			ostr << dict.getKey(g);

			if (index > 0){
				ostr << index;
			}
			else if (indexMax == 0){
				ostr << "00:00"; // alert
			}

			if (indexMax != index){
				ostr << ':';
				if (indexMax != INDEX_MAX) // TODO: default not implemented yet
					ostr << indexMax;
			}
		}
	}

	// Returns true if any of the elements is indexed, ie, DATASET, DATA or QUALITY.
	/*
	if (ODIMPathElem::isIndexed(flags.value)){
		ostr << '#';
		ostr << this->index << ':' << this->indexMax;
	}
	*/

	return ostr;
}

}  // namespace rack



