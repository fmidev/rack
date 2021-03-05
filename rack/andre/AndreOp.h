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
#include "data/EchoClass.h"
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

	//AndreOp(){};

	~AndreOp(){};


	//static	int getClassCode(const std::string & key);

	//static
	//int getClassCode(classtree_t & tr, classtree_t::path_t::const_iterator it, classtree_t::path_t::const_iterator eit);

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
	Hi5Tree & getDst(Hi5Tree & dst) const {
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
