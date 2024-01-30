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
void VolumeTraversalOp::traverseVolume(const Hi5Tree &src, Hi5Tree &dst) const {

	//drain::Logger mout(this->getName()+"(VolumeTraversalOp)", __FUNCTION__);
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.debug2("start" );

	/*
	DataSetMap<PolarSrc> srcDataSets;
	DataSetMap<PolarDst> dstDataSets;
	*/
	DataSetMap<PolarSrc> srcDataSets;
	DataSetMap<PolarDst> dstDataSets;

	ODIMPathList dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	this->dataSelector.getPaths(src, dataPaths); //, ODIMPathElem::DATA);

	mout.note("Initially " , dataPaths.size() , " paths with " , this->dataSelector );

	if (dataPaths.size() == 0)
		mout.note("no dataPaths matching selector: "  , this->dataSelector );

	drain::RegExp quantityRegExp(this->dataSelector.getQuantity()); // DataSet objects (further below)


	//std::stringstream key;

	//for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){
	for (ODIMPath & path: dataPaths){


		if (path.empty()){
			mout.fail("empty path accepted by selector: ", this->dataSelector);
			continue;
		}

		mout.ok("considering ", path);

		//ODIMPath::const_iterator pit = it->begin();
		// User may have modified dataselector so that odd paths appear

		if (!path.front().is(ODIMPathElem::DATASET)){
			mout.warn("Stripping trailing non-DATASET path elem (", path,"), obtained with selector: ", this->dataSelector);
			path.pop_front();
			if (path.empty()){
				mout.warn("odd 2nd path elem (...), with selector: ", this->dataSelector);
				continue;
			}
		}

		const ODIMPathElem & parent = path.front();

		if (!parent.is(ODIMPathElem::DATASET)){
			mout.warn("path does not start with /dataset.. :", path, ", with selector: ", this->dataSelector);
			continue;
		}
		/*
		if (!pit->is(ODIMPathElem::DATASET)){
			++pit;
			if (pit == it->end()){
				mout.warn("odd 2nd path elem (", *pit , "), with selector: " ,  this->dataSelector );
			}
		}

		if (!pit->is(ODIMPathElem::DATASET)){
			mout.warn("path does not start with /dataset.. :" , *it  , ", with selector: ",  this->dataSelector );
		}
		*/

		//const ODIMPath & parent = *it;



		mout.debug3("parent: " , parent );

		const drain::VariableMap & what = src[parent][ODIMPathElem::WHAT].data.attributes;
		std::string datetime = what["startdate"].toStr() + what["starttime"].toStr();


		const drain::VariableMap & where = src[parent][ODIMPathElem::WHERE].data.attributes;
		mout.debug3("attribs " , where );
		const double elangle = where["elangle"];

		//if (srcDataSets.find(elangle) == srcDataSets.end()){
		if (srcDataSets.find(datetime) == srcDataSets.end()){

			mout.debug2("add elangle="  , elangle, " time="  , datetime , ':'  , parent );

			// Something like: sweeps[elangle] = src[parent] .

			/// its and itd for debugging
			// srcDataSets.insert(DataSetMap<PolarSrc>::value_type(elangle, DataSet<PolarSrc>(src[*pit], quantityRegExp)));
			// dstDataSets.insert(DataSetMap<PolarDst>::value_type(elangle, DataSet<PolarDst>(dst[*pit], quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .

			srcDataSets.insert(DataSetMap<PolarSrc>::value_type(datetime, DataSet<PolarSrc>(src[parent], quantityRegExp)));

			dstDataSets.insert(DataSetMap<PolarDst>::value_type(datetime, DataSet<PolarDst>(dst[parent], quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .

			// elangles << elangle;

		}
		else {
			mout.debug3("already contains datetime=", datetime, ':', parent);
		}

	}

	mout.debug(srcDataSets.size(), " datasets, now calling computeSingleProduct() ");

	computeProducts(srcDataSets, dstDataSets);
	//(srcDataSets, dstDataSets);

}


/*
void VolumeTraversalOp::computeSingleProduct(const DataSetMap<PolarSrc> & srcDataSets, DataSetMap<PolarDst> & dstDataSets) const {
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented("TODO...");
	mout.error("TODO...");
}
*/

void VolumeTraversalOp::computeProducts(const DataSetMap<PolarSrc> & srcDataSets, DataSetMap<PolarDst> & dstDataSets) const {

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL name+"(DetectorOp)", __FUNCTION__);

	mout.debug2("start1");

	// DataSetMap<PolarSrc>::const_iterator its = srcDataSets.begin();
	// DataSetMap<PolarDst>::iterator itd = dstDataSets.begin();
	DataSetMap<PolarSrc>::const_iterator its = srcDataSets.begin();
	DataSetMap<PolarDst>::iterator itd = dstDataSets.begin();

	while (its != srcDataSets.end()){

		mout.attention("processing: ", its->first);

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
			mout.warn("something went wrong, src=", its->first );
			return;
		}
		++its;
		//++itd;
	}

	//mout.debug2("end" );

}


}  // namespace rack

