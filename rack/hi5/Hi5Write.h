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
#ifndef Hi5WRITE
#define Hi5WRITE

// TODO rename class to H52Tree

#include <hdf5.h>

#include <string>
#include <list>


#include <drain/image/Image.h>
//#include <drain/util/String.h>
#include <drain/util/Tree.h>

/*
#include <drain/image/Image.h>
#include <drain/image/ImageView.h>
#include <drain/image/ImageOp.h>
*/


#include "Hi5.h"


/*
 *
 * TODO:
 * - classify (hide methods)
 *
 */
// using namespace std;

namespace hi5 {

/// Converts Rack's h5 structure to native h5 structure applied for file output.
/**
 *
 *   ODIM conventions are purposely not included in the Hi5 utilities.
 */
class Writer : public Hi5Base {

public:

	/// Writes Rack's hi5 object to an HDF5 file.
	static
	void writeFile(const std::string &filename, const HI5TREE &tree);

	/// Writes Rack's hi5 object to an HDF5 file.
	//static
	//void writeText(std::ostream & ostr, const HI5TREE &tree);


	/// Conversion from Rack's hi5 to native HDF5.
	/**
	 *   \param tree - input object hi5 structure
	 *   \param fid - output in native HDF5 structure
	 *   \param path - path of a subtree of input (tree).
	 */
	static
	void treeToH5File(const HI5TREE &tree, hid_t fid, const std::string &path);

	/// Converts drain::Variable to an HDF5 attribute of a group or dataset.
	static
	void dataToH5Attribute(const drain::Variable &d, hid_t fid, const std::string &path, const std::string &attribute);

	/// Creates and writes scalar (non-std::string) attributes to group.
	template <class T>
	static
	void dataToH5AttributeScalar(const drain::Variable &d, hid_t fid, const std::string &path, const std::string &attribute){

		drain::MonitorSource mout(hi5::hi5monitor, "Hi5Write", __FUNCTION__);

		int status = 0;

		const hid_t tid = getH5DataType(typeid(T));

		// New
		const hsize_t elements = d.getElementCount();
		const bool isArray = (elements > 1); // && false;

		//const hid_t sid = isArray ? H5Screate_simple(1, &elements, NULL) : H5Screate(H5S_SCALAR);
		const hid_t sid = isArray ? H5Screate_simple(1, &elements, &elements) : H5Screate(H5S_SCALAR);
		if (sid < 0){
			mout.error() << ": H5Screate failed for attribute, path=" << path;
			mout << ", size=" << elements << mout.endl;
		}
		//H5Sc

		// NEW
		/*
		if (isArray){
			mout.debug()  << ": creating array of size " << elements;
			mout << ", path=" << path << mout.endl;
			status = H5Sset_extent_simple(sid, 1, &elements, &elements); //NULL);
			if (status < 0)
				mout.error()  << ": H5Screate failed for ARRAY, path=" << path << mout.endl;
			return;
		}
		*/

		const hid_t oid = H5Oopen(fid, path.c_str(), H5P_DEFAULT);
		if (oid < 0)
			mout.error()  << "H5Oopen failed, path=" << path << mout.endl;
		//const hid_t gid = H5Gopen2(fid,path.c_str(),H5P_DEFAULT);
		//if (gid < 0)
		//	mout.error()  << ": H5Gopen failed, path=" << path << mout.endl;

		const hid_t aid = H5Acreate2(oid, attribute.c_str(), tid, sid, H5P_DEFAULT, H5P_DEFAULT);
		if (aid < 0)
			mout.error()  << "H5Acreate failed, path=" << path << mout.endl;

		// OLD const T x = d;
		// status = H5Awrite(aid,tid,&x);
		// New
		 status = H5Awrite(aid, tid, d.getPtr());
		if (status < 0)
			mout.error()  << "H5Awrite failed, path=" << path << mout.endl;


		status = H5Aclose(aid);
		if (status < 0)
			mout.error()  << ": H5 close failed, path=" << path << mout.endl;

		//status = H5Gclose(gid);
		status = H5Oclose(oid);
		if (status < 0)
			mout.error()  << ": H5 close failed, path=" << path << mout.endl;

		status = H5Sclose(sid);
		if (status < 0)
			mout.error()  << ": H5 close failed, path=" << path << mout.endl;

	}

	static
	void linkToH5Attribute(hid_t lid, hid_t fid, const std::string &path, const std::string &attribute);

	/// Converts drain::image::Image to an HDF5 dataset.
	static
	void imageToH5DataSet(const drain::image::Image &image ,hid_t fid, const std::string & path);


	static
	char tempPathSuffix;

	//static	const std::string tempPathAttribute;

};





} // ::hi5

#endif

// Rack
