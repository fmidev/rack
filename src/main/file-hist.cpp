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
#include <drain/Log.h>
#include <fstream>
#include <iostream>



#include "drain/util/Output.h"

#include "fileio.h"
#include "resources.h"
#include "file-hist.h"






namespace rack {


const HistEntry CmdHistogram::histEntryHelper;

void CmdHistogram::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // getResources().mout;

	Hi5Tree & currentHi5 = *ctx.currentHi5;


	ODIMPath path;
	DataSelector selector(ODIMPathElemMatcher::DATA);
	//selector.setParameters(ctx.select);
	selector.consumeParameters(ctx.select);
	selector.getPath(currentHi5, path);
	//ctx.select.clear();

	PlainData<BasicDst> dstData(currentHi5(path));

	mout.experimental("data: ", dstData.data );

	mout.experimental("path: ", path, " [", dstData.odim.quantity, ']');

	// NO resources.setCurrentImage(selector);
	// drain::image::Image & img = *ctx.currentImage;
	// mout.warn("computing hist"  );
	const std::type_info & type = dstData.data.getType();

	const drain::ValueScaling & scaling = dstData.data.getScaling();
	/*
	unsigned short bitCount = 8 * drain::Type::call<drain::sizeGetter>(type);
	mout.warn("bitCount: " , bitCount);

	//( TODO: Histogram2 based on ImageCodeMap/Book (HistEntry);
	const int finalCount = count ? count : 1 << bitCount;
	drain::Histogram histogram(finalCount);
	// mout.note("Initial histogram 0: ", histogram);
	//histogram.setSize(count);
	*/

	drain::Histogram histogram;

	if (!range.empty()){
		histogram.setRange(range);
	}
	else {

		if (scaling.isPhysical()){
			mout.note("Image: physical range: ", scaling.getPhysicalRange());
		}
		mout.note("No range given, using scaling of data: ", scaling);
		histogram.deriveScaling(scaling, type);
		//histogram.setScale(s);
	}

	mout.note("Initial histogram 1: ", histogram);

	histogram.compute(dstData.data, type, scaling);

	if (!filename.empty()){

		mout.note("writing ", filename);

		legend leg;
		const drain::VariableMap & dstWhat = dstData.getWhat();
		if (dstWhat.hasKey("legend")){
			mout.note("Found what:legend", dstWhat["legend"], " using it!");
			//typedef std::map<int, std::string> legend;
			//dstWhat["legend"].toMap(leg, ',', ':'); // TOD
			drain::Variable v;
			v.toMap(leg, ',', ':');
			//drain::SmartMapTools::setValues(leg, dstWhat["legend"], '', ':');
		}
		else {
			setSpecialEntry(leg, dstData.odim.nodata,   "nodata");
			setSpecialEntry(leg, dstData.odim.undetect, "undetect");
		}

		writeHistogram(histogram, filename, leg);
	}

	if (!attribute.empty()){
		dstData.getHow()[attribute] = histogram.getVector();
		//dstData.updateTree2();
	}


}


void CmdHistogram::writeHistogram(const drain::Histogram & histogram, const std::string & filename, const legend &leg) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	mout.note("Scaling: ", histogram.scaling);

	drain::Output out((filename == "-") ? filename : ctx.outputPrefix + filename);

	std::ostream & ostr = out;

	drain::StringMapper mapper;
	if (! ctx.formatStr.empty()){
		mapper.parse(ctx.formatStr, true);
	}
	else {
		// Ensure that this uses same names as HistEntry() init.
		// Here \n is newline, so no conevrtEscape=false ok.
		mapper.parse("${index}\t =${count} #${label} [${range}[ \n", false);
	}

	// TODO: check tests
	// NEW
	ostr << "# " << mapper << '\n'; // TODO: pick plain keys
	mout.note("Legend: " , drain::sprinter(leg) );


	// OLD
	// Header
	ostr << "# [0," << histogram.getSize() << "] ";
	if (histogram.scaling.isPhysical())
		ostr << '[' << histogram.scaling.physRange << ']';
	ostr << '\n';

	HistEntry entry;
	const drain::Histogram::vect_t & v = histogram.getVector();

	//if (!leg.empty()){
	if (leg.size() >= 3){ // KLUDGE
		for (legend::const_iterator it=leg.begin(); it!=leg.end(); ++it){
			ostr << "# " << it->first << '=' << it->second << '\n';
		}
		for (legend::const_iterator it=leg.begin(); it!=leg.end(); ++it){
			entry.index = it->first;
			entry.count = v[it->first];
			entry.binRange.min = histogram.scaling.fwd(it->first);
			entry.binRange.max = histogram.scaling.fwd(it->first + 1);
			entry.label = it->second; // or parameters.reference?
			mapper.toStream(ostr, entry.getParameters());
		}
		ostr << '\n';
	}
	else {
		mout.note("No legend supplied, writing all the (", v.size(), ") elements");

		for (std::size_t i=0; i<v.size(); ++i){

			entry.index = i;
			entry.count = v[i];
			entry.binRange.min = histogram.scaling.fwd(i);
			entry.binRange.max = histogram.scaling.fwd(i+1);

			legend::const_iterator it = leg.find(i);
			if (it == leg.end()){
				entry.label.clear();
			}
			else {
				entry.label = it->second;
			}
			/*
			if (i == dstData.odim.nodata)
				entry.label = "nodata";
			else if (i == dstData.odim.undetect)
				entry.label = "undetect";
			else
				entry.label.clear();
			*/
			mapper.toStream(ostr, entry.getParameters());
		}
		ostr << '\n';
	}

	// histogram.dump(out);
}


void CmdHistogram::setSpecialEntry(legend & leg, double value, const std::string & label) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // getResources().mout;

	legend::key_type i = static_cast<legend::key_type>(value);
	if (static_cast<double>(i) != value){
		mout.warn("special code '" , label , "'=" , value , " not integer" );
	}
	/*
	if (i < leg.begin()->first){
		mout.warn("special code '" , label , "'=" , value , " smaller than" );
	}
	*/
	leg[i] = label;

}





} // namespace rack
