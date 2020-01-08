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

#ifndef Hi5READ
#define Hi5READ


#include <hdf5.h>

#include <string>
#include <list>


#include <drain/image/Image.h>
//#include <drain/util/StringTools.h>
#include <drain/util/Tree.h>


#include "Hi5.h"


// using namespace std;

namespace hi5 {

//extern const int ATTRIBUTES;
//extern const int DATASETS;


//extern

class Reader : public Hi5Base {

public:


	static
	const int ATTRIBUTES; // = 1;

	//extern
	static
	const int DATASETS; // = 2;

	static
	void readFile(const std::string &filename, Hi5Tree &tree, int mode=3); //(ATTRIBUTES|DATASETS));

	/// Conversion from native HDF5 structure to Rack's hi5 structure.
	/**
	 *  \param fid  - input in native HDF5 structure
	 *  \param path - subpath of input (typically the root, "/")
	 *  \param tree - output in hi5 structure
	 *  \param mode - switch for excluding attributes or datasets.
	 */
	static  // , const std::string &path,
	void h5FileToTree(hid_t fid, const Hi5Tree::path_t &path, Hi5Tree &tree, int mode=3); // ATTRIBUTES|DATASETS));

	/// Conversion from native HDF5 structure to Rack's hi5 structure.
	/**
	 *  \param fid  - input in native HDF5 structure
	 *  \param tree - output in hi5 structure
	 *  \param mode - switch for excluding attributes or datasets.
	 */
	static
	inline
	void h5FileToTree(hid_t fid, Hi5Tree &tree, int mode=3){ //(ATTRIBUTES|DATASETS)){
		//h5FileToTree(fid, "/", tree, mode);
		h5FileToTree(fid, Hi5Tree::path_t(Hi5Tree::path_t::elem_t(Hi5Tree::path_t::elem_t::ROOT)), tree, mode);
		// h5FileToTree(fid, "", tree, mode);
	};

	/*
	template <class T>
	static
	void h5AttributeToData(hid_t aid, hid_t datatype, drain::Variable & attribute){
		T x;
		int status = H5Aread(aid,datatype,&x);
		if (status < 0){
			hi5mout.error() << "h5AttributeToData: read failed " << hi5mout.endl;
		}
		attribute = x;
	}
	 */

	template <class T>
	static
	void h5AttributeToData(hid_t aid, hid_t datatype, drain::Variable & attribute, size_t elements=1){

		attribute.setType(typeid(T));
		attribute.setSize(elements);

		int status = H5Aread(aid, datatype, attribute.getPtr());

		if (status < 0){
			hi5mout.error() << "h5AttributeToData: read failed " << hi5mout.endl;
		}

		//attribute = x;

	}



	static
	void h5DatasetToImage(hid_t id, const Hi5Tree::path_t &path, drain::image::Image &image);

	//void h5DatasetToImage(hid_t id,const std::string &path,drain::image::Image &image);

protected:

	static
	herr_t  iterate_attribute(hid_t id, const char * attr_name, const H5A_info_t *ainfo, void *operator_data);

	static
	herr_t  iterate(hid_t group_id, const char * member_name, void *operator_data);

};






//static
//void debug(const Hi5Tree &src,int level = 0);


} // ::hi5


#endif /* ImageH5_H_ */

// Rack
