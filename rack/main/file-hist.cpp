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


/*
#include "drain/util/Log.h"
#include "drain/util/FilePath.h"
#include "drain/util/Output.h"
#include "drain/util/StringMapper.h"
#include "drain/util/Tree.h"
#include "drain/util/Variable.h"
#include "drain/image/File.h"
#include "drain/image/Image.h"
#include "drain/image/Sampler.h"
#include "drain/imageops/ImageModifierPack.h"

#include "drain/prog/Command.h"
#include "drain/prog/CommandBankUtils.h"
#include "drain/prog/CommandInstaller.h"

#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/DataTools.h"
#include "data/ODIMPath.h"
#include "data/PolarODIM.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"
#include "product/ProductOp.h"
#include "radar/FileGeoTIFF.h"
#include "radar/RadarDataPicker.h"
 */


#include "drain/util/Log.h"
#include "drain/util/Output.h"

#include "fileio.h"
#include "resources.h"
#include "file-hist.h"






namespace rack {

/*
struct HistEntry : drain::BeanLike {

	inline
	HistEntry() : drain::BeanLike(__FUNCTION__), index(0), count(0){
		parameters.link("index", index);
		parameters.link("min", binRange.min);
		parameters.link("max", binRange.max);
		parameters.link("count", count);
		parameters.link("label", label);
	};

	drain::Histogram::vect_t::size_type index;
	drain::Range<double> binRange;
	drain::Histogram::count_t count;
	std::string label;

};
static HistEntry histEntryHelper;
 */

const HistEntry CmdHistogram::histEntryHelper;

void CmdHistogram::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // getResources().mout;

	Hi5Tree & currentHi5 = *ctx.currentHi5;

	DataSelector selector(ODIMPathElemMatcher::DATA);
	//selector.pathMatcher.clear();
	//selector.pathMatcher << ODIMPathElemMatcher(ODIMPathElemMatcher::DATA);
	selector.setParameters(ctx.select);

	ODIMPath path;
	selector.getPath3(currentHi5, path);
	ctx.select.clear();



	PlainData<BasicDst> dstData(currentHi5(path));

	mout.warn() << "data: " << dstData.data << mout.endl;

	mout.note() << "path: " << path << " [" << dstData.odim.quantity << ']' << mout.endl;

	// NO resources.setCurrentImage(selector);
	// drain::image::Image & img = *ctx.currentImage;
	// mout.warn() << "computing hist"  << mout.endl;

	drain::Histogram histogram(256);
	histogram.setScale(dstData.data.getScaling());
	histogram.compute(dstData.data, dstData.data.getType());


	if (!filename.empty()){

		mout.warn() << "writing " << filename << mout.endl;

		legend leg;
		const drain::VariableMap & dstWhat = dstData.getWhat();
		if (dstWhat.hasKey("legend")){
			mout.note() << "Using what:legend" <<  dstWhat["legend"] << mout.endl;
			//typedef std::map<int, std::string> legend;
			dstWhat["legend"].toMap(leg, ',', ':'); // TOD
		}
		else {
			setSpecialEntry(leg, dstData.odim.nodata,   "nodata");
			setSpecialEntry(leg, dstData.odim.undetect, "undetect");
		}

		writeHistogram(histogram, filename, leg);
	}

	if (!store.empty()){
		dstData.getHow()[store] = histogram.getVector();
		//dstData.updateTree2();
	}


}

void CmdHistogram::setSpecialEntry(legend & leg, double value, const std::string & label) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // getResources().mout;

	legend::key_type i = static_cast<legend::key_type>(value);
	if (static_cast<double>(i) != value){
		mout.warn() << "special code '" << label << "'=" << value << " not integer" << mout;
	}
	/*
	if (i < leg.begin()->first){
		mout.warn() << "special code '" << label << "'=" << value << " smaller than" << mout;
	}
	*/
	leg[i] = label;

}

void CmdHistogram::writeHistogram(const drain::Histogram & histogram, const std::string & filename, const legend &leg) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	drain::Output out((filename == "-") ? filename : ctx.outputPrefix + filename);

	std::ostream & ostr = out;

	drain::StringMapper mapper;
	if (! ctx.formatStr.empty()){
		mapper.parse(ctx.formatStr, true);
	}
	else
		mapper.parse("${count} # '${label}' (${index}) [${min}, ${max}] \n", false); // here \n IS newline...

	// TODO: check tests
	// NEW
	ostr << "# " << mapper << '\n'; // TODO: pick plain keys
	mout.note() << "Legend: " << drain::sprinter(leg) << mout.endl;


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
		mout.note() << "No legend supplied, writing all elements" << mout.endl;

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







} // namespace rack
