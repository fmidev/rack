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
#ifndef RACK_KEY_SELECTOR
#define RACK_KEY_SELECTOR

#include <drain/util/StringMatcherList.h>


class FM301KeyMatcher : public drain::StringMatcher {
public:


	FM301KeyMatcher(const std::string & key="", const std::string & standardName="", const std::string & longName="") :
		drain::StringMatcher(key), standard_name(standardName), long_name(longName) {
		// TODO: extract standardName from long? Or vice versa...
	}


	FM301KeyMatcher(const char *key, const char *standardName="", const char *longName="") :
		drain::StringMatcher(key), standard_name(standardName), long_name(longName) {
		// TODO: extract standardName from long? Or vice versa...
	}

	inline
	FM301KeyMatcher(const FM301KeyMatcher & matcher) : drain::StringMatcher(matcher), standard_name(matcher.standard_name), long_name(matcher.long_name){
	}

	inline
	void setStandardName(const std::string & name){
		standard_name = name;
	}

	inline
	const std::string &	getStandardName() const {
		return standard_name;
	}

	inline
	void setLongName(const std::string & name){
		long_name = name;
	}

	inline
	const std::string & getLongName() const {
		return long_name;
	}

protected:

	std::string standard_name;
	std::string long_name;

};

typedef drain::StringMatcherList<FM301KeyMatcher> QuantitySelector;

#endif
