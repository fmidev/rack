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

#ifndef VOLTRAVOP_H_
#define VOLTRAVOP_H_

#include "PolarProductOp.h"


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




///
/**
 *   \tparam M - ODIM type corresponding to products type (polar, vertical)
 */
class VolumeTraversalOp : public PolarProductOp { //VolumeOp<PolarODIM> {

public:

	VolumeTraversalOp(const std::string & name, const std::string &description="") : PolarProductOp(name, description){
	};

	~VolumeTraversalOp(){};

	virtual
	void processVolume(const HI5TREE &src, HI5TREE &dst) const;

	virtual
	void processDataSets(const DataSetSrcMap & srcVolume, DataSetDstMap<PolarDst> & dstVolume) const;

	virtual
	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		copyPolarGeometry(srcODIM, dstData);
	}

};

/*
template <class M>
void VolumeTraversalOp<M>::processVolume(const HI5TREE &src, HI5TREE &dst) const {

	drain::MonitorSource mout(this->name+"(VolumeTraversalOpOp)", __FUNCTION__);
	mout.debug(1) << "start" << mout.endl;

	DataSetSrcMap srcDataSets;
	DataSetDstMap<PolarDst> dstDataSets;

	std::list<std::string> dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	DataSelector::getPaths(src, this->dataSelector, dataPaths);

	if (dataPaths.size() == 0)
		mout.note() << "no dataPaths matching selector="  << this->dataSelector << mout.endl;

	drain::RegExp quantityRegExp(this->dataSelector.quantity); // DataSet objects (further below)
	//drain::Variable elangles(typeid(double));
	mout.warn() << "regExp: " << quantityRegExp << mout.endl;


	for (std::list<std::string>::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		mout.debug() << "considering" << *it << mout.endl;

		const std::string parent = DataSelector::getParent(*it);
		const double elangle = src(parent)["where"].data.attributes["elangle"];

		if (srcDataSets.find(elangle) == srcDataSets.end()){
			mout.info() << "add "  << elangle << ':'  << parent << mout.endl;

			/// its and itd for debugging
			//DataSetSrcMap::const_iterator its = srcDataSets.insert(srcDataSets.begin(), DataSetSrcMap::value_type(elangle, DataSetSrc<>(src[parent], quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .
			srcDataSets.insert(DataSetSrcMap::value_type(elangle, DataSetSrc<PolarSrc>(src(parent), quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .

			//DataSetDstMap::iterator itd = dstDataSets.begin();
			//itd = dstDataSets.insert(itd, DataSetDstMap::value_type(elangle, DataSetDst<>(dst[parent], quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .
			dstDataSets.insert(DataSetDstMap<>::value_type(elangle, DataSetDst<PolarDst>(dst(parent), quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .

			// elangles << elangle;

		}

	}

	mout.debug() << "ok, calling processDataSets " << mout.endl;

	//processDataSets(srcDataSets, dstDataSets);


}
*/

}  // namespace rack


#endif /* RACKOP_H_ */

// Rack
