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

//#include <exception>
#include <fstream>
#include <iostream>



#include "drain/util/Histogram.h"
#include "drain/prog/Command.h"


//#include "fileio.h"
#include "resources.h"
//#include "fileio-read.h"

namespace rack {

struct HistEntry : drain::BeanLike {

	inline
	HistEntry() : drain::BeanLike(__FUNCTION__), index(0), count(0){
		parameters.link("index", index);
		parameters.link("range", binRange.tuple());
		//parameters.link("max", binRange.max);
		parameters.link("count", count);
		parameters.link("label", label);
	};

	drain::Histogram::vect_t::size_type index;
	drain::Range<double> binRange;
	drain::Histogram::count_t count;
	std::string label;

};


/// TODO: generalize to array outfile
class CmdHistogram : public drain::BasicCommand {

public:

	int count;

	drain::Range<double> range;

	std::string attribute;
	std::string filename;

	//	CmdHistogram() : drain::SimpleCommand<int>(__FUNCTION__, "Histogram","slots", 256, "") {
	CmdHistogram() : drain::BasicCommand(__FUNCTION__, std::string("Histogram. Optionally --format using keys ") + histEntryHelper.getParameters().getKeys()) {
		parameters.link("count", count = 0);  // = 256
		parameters.link("range", range.tuple());
		//parameters.link("max", maxValue = +std::numeric_limits<double>::max());
		parameters.link("filename", filename="", "<filename>.txt|-");
		parameters.link("attribute", attribute="histogram", "<attribute_key>");
	};

	CmdHistogram(const CmdHistogram & cmd): drain::BasicCommand(cmd), count(0) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	};
	// virtual	inline const std::string & getDescription() const { return description; };

	typedef std::map<int, std::string> legend;


	void exec() const;

	// OutputPrefix ? included or not?
	// Consider static, called by CmdOutputFile with default hist params?
	void writeHistogram(const drain::Histogram & histogram, const std::string & filename, const legend &leg = legend()) const;


private:

	// Try to set "nodata" and "undetect"
	void setSpecialEntry(legend & leg, double value, const std::string & label) const;

	static
	const HistEntry histEntryHelper;

};





} // namespace rack
