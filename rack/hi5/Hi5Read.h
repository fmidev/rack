/**

    Copyright 2001 - 2010  Markus Peura,
    Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

    Created on: Jul 19, 2010
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

class Reader : public Hi5Base {

public:

	static
	void readFile(const std::string &filename,HI5TREE &tree, int mode=(ATTRIBUTES|DATASETS));

	/// Conversion from native HDF5 structure to Rack's hi5 structure.
	/**
	 *  \param fid  - input in native HDF5 structure
	 *  \param path - subpath of input (typically the root, "/")
	 *  \param tree - output in hi5 structure
	 *  \param mode - switch for excluding attributes or datasets.
	 */
	static
	void h5FileToTree(hid_t fid, const std::string &path, HI5TREE &tree, int mode=(ATTRIBUTES|DATASETS));

	/// Conversion from native HDF5 structure to Rack's hi5 structure.
	/**
	 *  \param fid  - input in native HDF5 structure
	 *  \param tree - output in hi5 structure
	 *  \param mode - switch for excluding attributes or datasets.
	 */
	static
	inline
	void h5FileToTree(hid_t fid, HI5TREE &tree, int mode=(ATTRIBUTES|DATASETS)){
		h5FileToTree(fid, "/", tree, mode);
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
		attribute.resize(elements);

		int status = H5Aread(aid,datatype, attribute.getPtr());

		if (status < 0){
			hi5mout.error() << "h5AttributeToData: read failed " << hi5mout.endl;
		}

		//attribute = x;

	}



	static
	void h5DatasetToImage(hid_t id,const std::string &path,drain::image::Image &image);


	//extern
	static
	const int ATTRIBUTES = 1;

	//extern
	static
	const int DATASETS = 2;

protected:

	static
	herr_t  iterate_attribute(hid_t id, const char * attr_name, const H5A_info_t *ainfo, void *operator_data);

	static
	herr_t  iterate(hid_t group_id, const char * member_name, void *operator_data);

};







//static
//void debug(const HI5TREE &src,int level = 0);


} // ::hi5


#endif /* ImageH5_H_ */
