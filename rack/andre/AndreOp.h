/**

    Copyright 2011-2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

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
