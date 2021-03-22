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


#include "resources.h"
#include "fileio.h"




namespace rack {

void CmdOutputFile::writeProfile(const Hi5Tree & src, const std::string & filename) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);


	/// Currently designed only for vertical profiles produced by VerticalProfileOp (\c --pVerticalProfile )
	/// TODO: modify DataSet such that the quantities appear in desired order.

	mout.info() << "File format: .mat (text dump of data)" << mout.endl;

	DataSelector selector;
	selector.consumeParameters(ctx.select);
	//ctx.select.clear();

	mout.debug2() << selector << mout.endl;
	const drain::RegExp quantityRegExp(selector.quantity);
	const bool USE_COUNT = quantityRegExp.test("COUNT");
	// mout.debug3() << "use count" << static_cast<int>(USE_COUNT) << mout.endl;

	const DataSet<PolarSrc> product(src["dataset1"], drain::RegExp(selector.quantity));
	//const DataSet<> product((*ctx.currentHi5), selector);

	std::string mainQuantity; // = product.getFirstData().odim.quantity;

	const drain::RegExp mainQuantityRegExp("HGHT");

	for (DataSet<PolarSrc>::const_iterator it = product.begin(); it != product.end(); ++it){
		if ((it->second.data.getWidth()!=1) && (it->second.data.getHeight()!=1)){
			mout.warn() << "skipping non-1D data, quantity: " << it->first << mout.endl;
			continue;
		}
		if (mainQuantity.empty()){
			mainQuantity = it->first;
		}
		else if (mainQuantityRegExp.test(it->first)){
			mainQuantity = it->first;
			mout.debug3() << "picked main quantity: " << mainQuantity << mout.endl;
			break;
		}
	}

	mout.debug2() << "main quantity: " << mainQuantity << mout.endl;


	const Data<PolarSrc> & srcMainData = product.getData(mainQuantity); // intervals//product.getData("HGHT"); // intervals
	if (srcMainData.data.isEmpty()){
		ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
		mout.warn() << "zero dimension data (for " << mainQuantity << "), giving up." << mout.endl;
		return;
	}

	// drain::Output out((filename == "-") ? filename : ctx.outputPrefix + filename);
	drain::Output output(filename);
	//std::string outFileName = ctx.outputPrefix + value;
	//std::ofstream ofstr(outFileName.c_str(), std::ios::out);
	std::ofstream & ofstr = output;

	/// Step 1: create header
	//  ofstr << "## " << (*ctx.currentHi5)["where"].data.attributes << '\n';
	//  ofstr << "## " << product.tree["where"].data.attributes << '\n';
	ofstr << "# " << mainQuantity << '\t';
	for (DataSet<PolarSrc>::const_iterator it = product.begin(); it != product.end(); ++it){
		const std::string & quantity = it->first;
		const Data<PolarSrc> & srcData = it->second;
		if ((srcData.data.getWidth()!=1) && (srcData.data.getHeight()!=1))
			mout.warn() << "Skipping" << mout.endl;
		//mout.warn() << "Skipping non 1-dimensional data, un supported for .mat format. Geometry=" << srcData.data.getGeometry() << " ." << mout.endl;
		else if (quantity != mainQuantity){
			ofstr << it->first << '\t'; // TO
			if (USE_COUNT){
				if (srcData.hasQuality("COUNT"))
					ofstr << "COUNT\t"; // it->first <<
			}
		}
	}
	ofstr << '\n';


	const size_t n = std::max(srcMainData.data.getWidth(), srcMainData.data.getHeight());
	if (n == 0){
		mout.warn() << "zero dimension data, giving up." << mout.endl;
		return;
	}

	/// Step 2: Main loop
	//  Notice: here turned downside up, because the images (~profile arrays) are upside down.
	for (int i = n-1; i >= 0; --i) {
		/// Data rows. (This loop is somewhat slow due to [*it] .)
		/// HGHT
		ofstr << srcMainData.odim.scaleForward(srcMainData.data.get<double>(i)) << '\t';  ///
		/// others
		for (DataSet<PolarSrc>::const_iterator it = product.begin(); it != product.end(); ++it){
			if (it->first != mainQuantity){
				const Data<PolarSrc> & srcData = it->second;
				if ((srcData.data.getWidth()==1) || (srcData.data.getHeight()==1)){

					ofstr << srcData.odim.scaleForward(srcData.data.get<double>(i)) << '\t';  ///
					//const std::string countStr(it->first+"_COUNT");
					if (USE_COUNT){
						const PlainData<PolarSrc> & count = srcData.getQualityData("COUNT");
						if (!count.data.isEmpty()){
							ofstr <<  count.odim.scaleForward(count.data.get<unsigned int>(i)) << '\t'; // TO
						}
					}
				}
			}
		}
		ofstr << '\n';
		//std::cerr << "i=" << i << std::endl;
	}

	//ofstr.close();

}


} // namespace rack
