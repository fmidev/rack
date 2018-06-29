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

#ifndef AnDReOP_H_
#define AnDReOP_H_


#include "radar/Geometry.h"
#include "hi5/Hi5.h"
#include "data/ODIM.h"
#include "product/VolumeTraversalOp.h"


using namespace drain::image;

namespace rack {



/// The base class for  removal operators.
//  No more for anomaly detection?
/**
 *
 */
class AndreOp: public VolumeTraversalOp {

public:

	~AndreOp(){};

	/// If true, results of certain detectors will be stored higher and hence applied in a wider group of data(sets).
	//static 	bool universalMode;  use:  Detector::SUPPORT_UNIVERSAL

	//virtual	void processVolume(const HI5TREE &src, HI5TREE &dst) const;



	/// Searches for precomputed quality field up in the hierarchy. Computes if nonexistent, and stores in pathCombinedQuality
	//  Todo: This implementation is clumsy, as it re-searches for the data
	/**
	 *  \param dstRoot   - structure to be traversed
	 *  \param groupPath - path to be searched for and the parent for quality1/<pathCombinedQuality>
	 *
	 *  Called by (at least) RemoverOp
	 */
	// static Image & getCombinedQualityData(HI5TREE &dstRoot, const std::string &groupPath); // todo: support to "universal"

	/// Store combined detection results in .../quality1/(pathCombinedQuality): 'data' implies overwriting, 'data~' implies temp data (unsaved).
	// static std::string pathCombinedQuality;

	virtual
	void processDataSets(const DataSetMap<PolarSrc> & srcVolume, DataSetMap<PolarDst> & dstVolume) const = 0;

protected:

	virtual
	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		copyPolarGeometry(srcODIM, dstData);
	}

	/*
	virtual
	HI5TREE & getDst(HI5TREE & dst) const {
		return dst;
	};
	*/

	/// Constructor for derived classes.
	/**
	 */
	AndreOp(const std::string &name, const std::string & description) : VolumeTraversalOp(name, description){
	};


};


}

#endif /* CorrectoROP_H_ */

// Rack
