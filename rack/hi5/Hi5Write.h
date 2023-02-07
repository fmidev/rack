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

#include "Hi5.h" // to define H5_USE_*_API_DEFAULT

#ifndef Hi5WRITE
#define Hi5WRITE

// TODO rename class to H52Tree

#include <hdf5.h>

#include <string>
#include <list>

#include "drain/image/Image.h"
#include "drain/util/TreeOrdered.h"

/*
 *
 * TODO:
 * - classify (hide methods)
 *
 */


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
	void writeFile(const std::string &filename, const Hi5Tree &tree);

	/// Writes Rack's hi5 object to an HDF5 file.
	//static
	//void writeText(std::ostream & ostr, const Hi5Tree &tree);


	/// Conversion from Rack's hi5 to native HDF5.
	/**
	 *   \param tree - input object hi5 structure
	 *   \param fid - output in native HDF5 structure
	 *   \param path - path of a subtree of input (tree).
	 */
	static
	void treeToH5File(const Hi5Tree &tree, hid_t fid, const Hi5Tree::path_t & path);

	/// Converts drain::Variable to an HDF5 attribute of a group or dataset.
	static
	void dataToH5Attribute(const drain::Variable &d, hid_t fid, const Hi5Tree::path_t &path, const std::string &attribute);

	/// Creates and writes scalar (non-std::string) attributes to group.
	/* template <class T>
	static
	void dataToH5AttributeT(const drain::Variable &d, hid_t fid, const std::string &path, const std::string &attribute);
	*/

	/// For std::string and characterArrays.
	static  // ,const std::string &path
	void dataToH5AttributeString(const drain::Variable &d, hid_t fid, const Hi5Tree::path_t &path, const std::string &attribute);

	/// Converts drain::image::Image to an HDF5 dataset.
	static
	hid_t imageToH5DataSet(const drain::image::Image &image, hid_t fid, const Hi5Tree::path_t & path);


	//  Under constr

	/// Write a vector of paired values
	/**
	 *  This method uses std::pair as entries. Each entry consists of two values named \c first and \c second .
	 *
	 *  \tparam K - type of the first element, typically the key associated with the value (the second element)
	 *  \tparam V - type of the second element, typically the value associated with the key (the first element)
	 *
	 *  \param v - vector containing pairs, perhaps obtaining by adding
	 *  \param f - HDF5 file id
	 *  \param path   - location of the target dataset
	 *  \param label1 - label of the first value
	 */
	template <class K, class V>
	static
	void vectorToH5Compound(const std::vector<std::pair<K,V> > & v, hid_t fid, const Hi5Tree::path_t & path, const char *labelFirst="key", const char *labelSecond="value");

	///  TODO:
	// template <class T> static void vectorToH5Compound(const std::vector<T> ...  )

	/// Writes key-value pairs as compound data.
	/**
	 *   Constucts a std::vector or std::pair's , converting strings to const char *, if applied.
	 *   Then, calls vectorToH5Compound()
	 */
	template <class K, class V>
	static
	void mapToH5Compound(const std::map<K,V> & m, hid_t fid, const Hi5Tree::path_t & path, const char *labelFirst="key", const char *labelSecond="value");


	///  Under constr
	// static void dataToH5Compound(const drain::VariableMap & m, hid_t fid, const std::string &path);


	///  Under constr
	static
	void linkToH5Attribute(hid_t lid, hid_t fid, const std::string &path, const std::string &attribute);

	// deprecated (exclude from save marker)
	//static char tempPathSuffix;

protected:

	///
	/**
	 *  \return - rank: 2 for single, 3 for multichannel, or 0 in errors
	 *
	 *  Handles also ZLIB compression (ODIM recommendation: zlib compression level 6)
	 */
	static
	hsize_t deriveDimensions(const drain::image::Geometry & g, std::vector<hsize_t> & dims, std::vector<hsize_t> & chunkDims);

	template <class T>
	class CompoundConv {

	public:

		typedef T conv_t;

		static inline
		const conv_t & conv(const T & x){
			return x;
		}

	};


};

template <class K, class V>
void Writer::vectorToH5Compound(const std::vector<std::pair<K,V> > & v, hid_t fid, const Hi5Tree::path_t & path, const char *labelFirst, const char *labelSecond){

	drain::Logger mout(__FUNCTION__, __FILE__);

	typedef std::vector<std::pair<K,V> > vect_t;
	typedef typename vect_t::value_type pair_t;
	typedef typename pair_t::first_type   first_type;
	typedef typename pair_t::second_type second_type;

	herr_t status;

	const hsize_t size = sizeof(pair_t);
	mout.debug() << path << ": " << v.size() << " elements x " << size << "b" << mout.endl;

	const hid_t first_h5t  = Hi5Base::getH5NativeDataType(typeid(first_type));
	const hid_t second_h5t = Hi5Base::getH5NativeDataType(typeid(second_type));

	mout.debug3() << "datatypes: " << first_h5t << ", " << second_h5t << mout.endl;

	// Create the compound datatype for memory.
	hid_t memtype = H5Tcreate (H5T_COMPOUND, size);

	// HOFFSET(pair_t, first)
	status = H5Tinsert (memtype, labelFirst, 0, first_h5t); // Hi5Base::getH5NativeDataType(typeid(int));
	handleStatus(status, "H5Tinsert failed", mout, __LINE__);
	//if (status < 0)	mout.error() << "H5Tinsert failed for pair.first type, path=" << path << mout.endl;

	status = H5Tinsert (memtype, labelSecond, sizeof(first_type), second_h5t); // Hi5Base::getH5NativeDataType(typeid(int));
	handleStatus(status, "H5Tinsert failed", mout, __LINE__);
	//if (status < 0) mout.error() << "H5Tinsert failed for pair.second type, path=" << path << mout.endl;


	const hid_t std_t1 = Hi5Base::getH5StandardType(typeid(first_type));
	const hid_t std_t2 = Hi5Base::getH5StandardType(typeid(second_type));
	mout.debug() << "file types: " << std_t1 << ", " << std_t2 << mout.endl;
	const hsize_t s1 = H5Tget_size(std_t1);
	const hsize_t s2 = H5Tget_size(std_t2);

	hid_t filetype = H5Tcreate (H5T_COMPOUND, s1+s2);  // size wrong if hvt_t ?
	H5Tinsert (filetype, labelFirst, 0,    std_t1);
	H5Tinsert (filetype, labelSecond, s1,  std_t2); // TODO? Hi5Base::getH5StandardType(typeid(int));

	// Create dataspace.  Setting maximum size to NULL sets the maximum size to be the current size.
	hsize_t dims[1]; //
	dims[0] = v.size();
	hid_t space = H5Screate_simple (1, dims, NULL);


	const std::string pathStr(path);
	//static_cast<std::string>(path).c_str();
	mout.note() << "experimental: creating compound at " << pathStr << mout.endl;


	if (H5Lexists(fid, pathStr.c_str(), H5P_DEFAULT)){
		mout.warn() << "compound object exists already (combining is not supported) : " << pathStr << mout.endl;
	}
	else {
		// mout.debug3() << "does not exist, creating " << pathStr << mout.endl;
		hid_t dset = H5Dcreate(fid, pathStr.c_str(), filetype, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		handleStatus(dset, "H5Dcreate failed", mout, __LINE__);
		if (dset >= 0){
			// Create the dataset and write the compound data to it.
			status = H5Dwrite (dset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &v.at(0));
			handleStatus(dset, "H5Dwrite failed", mout, __LINE__);
			//H5Dclose();
			status = H5Dclose(dset);
			handleStatus(status, "H5Dclose failed", mout, __LINE__);
		}
	}

	status = H5Sclose(space);
	handleStatus(status, "H5Sclose failed", mout, __LINE__);

}

/// For some reason HDF5 int fails, but long int does not (Native vs Standard type diff?)
template <>
class Writer::CompoundConv<int> {

public:

	typedef long int conv_t;

	static inline
	conv_t conv(int x){
		return static_cast<conv_t>(x);
	}

};


template <>
class Writer::CompoundConv<std::string> {

public:

	typedef const char * conv_t;

	static inline
	conv_t conv(const std::string & x){
		return x.c_str();
	}

};

template <class K, class V>
void Writer::mapToH5Compound(const std::map<K,V> & m, hid_t fid, const Hi5Tree::path_t & path, const char *labelFirst, const char *labelSecond){

	drain::Logger mout(__FUNCTION__, __FILE__);

	typedef typename CompoundConv<K>::conv_t key_t;
	typedef typename CompoundConv<V>::conv_t val_t;

	typedef std::vector<std::pair<key_t,val_t> > vect_t;
	vect_t v(m.size());
	//v.resize(m.size());

	typename vect_t::iterator vit = v.begin();
	for (typename std::map<K,V>::const_iterator it = m.begin(); it != m.end(); ++it) {
		vit->first  = CompoundConv<K>::conv(it->first);
		vit->second = CompoundConv<V>::conv(it->second);
		mout.debug2() << vit->first << ':' << vit->second << mout.endl;
		++vit;
	}

	Writer::vectorToH5Compound(v, fid, path, labelFirst, labelSecond);

}




/*
template <>
inline
void Writer::dataToH5AttributeT<std::string>(const drain::Variable & data, hid_t fid, const std::string & path, const std::string & attribute){
	dataToH5AttributeString(d, fid, path, attribute);
}
*/

/*
 *

template <class T>
void Writer::dataToH5AttributeT(const drain::Variable &d, hid_t fid, const std::string &path, const std::string &attribute){

	drain::Logger mout(hi5::hi5monitor, "Hi5Write", __FUNCTION__);

	if (d.isCharArrayString() || (d.getType() == typeid(std::string))){
		dataToH5AttributeString(d, fid, path, attribute);
		return;
	}


	int status = 0;

	const hid_t tid = getH5NativeDataType(typeid(T));

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

*/
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
/*
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
*/





} // ::hi5

#endif

// Rack
