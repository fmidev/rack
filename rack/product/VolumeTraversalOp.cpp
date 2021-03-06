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
/*
 * ProductOp.h
 *
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#include "VolumeTraversalOp.h" // NEW


namespace rack {

using namespace drain::image;

/// Base class for radar data processors.

/// Base class for radar data processors.
/** Input and output as HDF5 data, which has been converted to internal structure, drain::Tree<NodeH5>.
 *
 *  Basically, there are two kinds of polar processing
 *  - Cumulative: the volume is traversed, each sweep contributing to a single accumulation array, out of which the product layer(s) is extracted.
 *  - Sequential: each sweep generates new layer (/dataset) in the product; typically, the lowest only is applied.
 *
 *  TODO: Raise to RackOp
 */





//template <class M>
void VolumeTraversalOp::processVolume(const Hi5Tree &src, Hi5Tree &dst) const {

	//drain::Logger mout(this->getName()+"(VolumeTraversalOp)", __FUNCTION__);
	drain::Logger mout(__FUNCTION__, __FILE__);
	mout.debug2() << "start" << mout.endl;

	DataSetMap<PolarSrc> srcDataSets;
	DataSetMap<PolarDst> dstDataSets;

	ODIMPathList dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	this->dataSelector.getPaths(src, dataPaths); //, ODIMPathElem::DATA);

	mout.note() << "Initially " << dataPaths.size() << " paths with " << this->dataSelector << mout.endl;

	if (dataPaths.size() == 0)
		mout.note() << "no dataPaths matching selector: "  << this->dataSelector << mout.endl;

	drain::RegExp quantityRegExp(this->dataSelector.quantity); // DataSet objects (further below)


	for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){


		if (it->empty()){
			mout.fail() << "empty path accepted by selector: " <<  this->dataSelector << mout.endl;
			continue;
		}

		mout.ok() << "considering " << *it << mout.endl;

		ODIMPath::const_iterator pit = it->begin();

		/*
		if (!pit->empty()){
			mout.warn() << "path starts by '"<< *pit << "' instead of root, with selector: " <<  this->dataSelector << mout.endl;
		}
		*/

		// User may have modified dataselector so that odd paths appear

		if (!pit->is(ODIMPathElem::DATASET)){
			++pit;
			if (pit == it->end()){
				mout.warn() << "odd 2nd path elem ("<< *pit << "), with selector: " <<  this->dataSelector << mout.endl;
			}
		}

		if (!pit->is(ODIMPathElem::DATASET)){
			mout.warn() << "path does not start with /dataset.. :" << *it  << ", with selector: "<<  this->dataSelector << mout.endl;
		}

		//const ODIMPath & parent = *it;



		mout.debug3() << "parent: " << *pit << mout.endl;

		const drain::VariableMap & attribs = src[*pit][ODIMPathElem::WHERE].data.attributes;
		mout.debug3() << "attribs " << attribs << mout.endl;


		const double elangle = attribs["elangle"];

		if (srcDataSets.find(elangle) == srcDataSets.end()){

			mout.debug2() << "add elangle="  << elangle << ':'  << *pit << mout.endl;

			// Something like: sweeps[elangle] = src[parent] .

			/// its and itd for debugging
			// DataSetMap<PolarSrc>::const_iterator its =
			srcDataSets.insert(DataSetMap<PolarSrc>::value_type(elangle, DataSet<PolarSrc>(src[*pit], quantityRegExp)));

			// DataSetMap<PolarDst>::iterator itd =
			dstDataSets.insert(DataSetMap<PolarDst>::value_type(elangle, DataSet<PolarDst>(dst[*pit], quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .

			// elangles << elangle;

		}
		else {
			mout.debug3() << "already contains elangle="  << elangle << ':'  << *pit << mout.endl;
		}

	}

	mout.debug() << srcDataSets.size() << " datasets, now calling processDataSets() " << mout.endl;

	processDataSets(srcDataSets, dstDataSets);


}


void VolumeTraversalOp::processDataSets(const DataSetMap<PolarSrc> & srcDataSets, DataSetMap<PolarDst> & dstDataSets) const {

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL name+"(DetectorOp)", __FUNCTION__);

	mout.debug2() << "start1" << mout.endl;

	DataSetMap<PolarSrc>::const_iterator its = srcDataSets.begin();
	DataSetMap<PolarDst>::iterator itd = dstDataSets.begin();
	while (its != srcDataSets.end()){

		mout.info() << "processing elangle:" << its->first << mout.endl;

		itd = dstDataSets.find(its->first);

		//if (its->first == itd->first){
		if (itd != dstDataSets.end()){

			const DataSet<PolarSrc> & srcDataSet = its->second;
			DataSet<PolarDst> & dstDataSet = itd->second;

			/// MAIN COMMAND
			processDataSet(srcDataSet, dstDataSet);
			//dstDataSet.updateTree(); // create /what, /where etc.
			//DataTools::updateInternalAttributes(dstProb.tree); // collect attributes from /what, /where to /data:data properties so that srcData.getQualityData() works below.
			// update str trees?


		}
		else {
			mout.warn() << "something went wrong, src=" << its->first << mout.endl;
			return;
		}
		++its;
		//++itd;
	}

	mout.debug2() << "end" << mout.endl;

}


}  // namespace rack


// Rack
 // REP // REP // RE2
 // RE2
 // REP
