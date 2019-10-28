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
	void convertAttribute(std::stringstream sstr,drain::Data data){
		data = sstr.toStr();
	};

 */


#include <hdf5.h>

#include "Hi5Write.h"




// using namespace std;


namespace hi5 {

//char Writer::tempPathSuffix = '~';

void Writer::writeFile(const std::string &filename, const HI5TREE &tree){

	drain::Logger mout(hi5::hi5monitor, __FILE__, __FUNCTION__);

	const hid_t fid = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	if (fid < 0)
		mout.error() << ": H55create failed, file=" << filename << mout.endl;

	treeToH5File(tree,fid,"/");

	int status = H5Fclose(fid);
	if (status < 0)
		mout.warn() << ": H55close failed, file=" << filename << mout.endl;

	//hi5::debug(tree);
}


void Writer::treeToH5File(const HI5TREE &tree,hid_t fid, const std::string &path){

	drain::Logger mout(hi5::hi5monitor, "Writer", __FUNCTION__);

	//mout.note() << "tempPathSuffix='" << tempPathSuffix << "' pathSize=" << path.size() << '\n';

	/// Feature: do not convert paths ending with '~'.
	/// DEPRECATED
	/*
	if ((tempPathSuffix!='\0') && (path.size() > 0)){
		//if ((path.at(path.size()-1) == '~') || (path.find('~') != std::string::npos) ){
		if (path.find('~') != std::string::npos){
			mout.debug(1) << "skipping temporary object " << path << mout.endl;
			return;
		}
	}
	*/

	const hi5::NodeHi5 & node = tree.data;
	const drain::VariableMap  & attributes = node.attributes;
	const drain::image::Image & image = node.dataSet;

	if (node.noSave){ // attributes["~tmp~"].getS
		mout.debug(1) << "skipping temporary object " << path << mout.endl;
		return;
	}

	// Handle as H5Data and return, if data exists...
	if (!image.isEmpty()){  // image or palette
		// Notice: no attribute conversion supported.
		//const std::string &name = image.
		imageToH5DataSet(image,fid,path);
		drain::Variable d(typeid(std::string));
		if (attributes["CLASS"].toStr() == "PALETTE"){
			dataToH5Attribute(d="RGB", fid, path,"PAL_COLORMODEL");
			dataToH5Attribute(d="STANDARD8", fid, path,"PAL_TYPE");
			dataToH5Attribute(d="1.2", fid, path,"PAL_VERSION");
			//attributes["PAL_COLORMODEL"] = "RGB";
			//attributes["PAL_MINMAXNUMERIC"] = std::string(); // << min , max "0,255";
			//attributes["PAL_TYPE"] = "STANDARD8";
			//attributes["PAL_VERSION"] = "1.2";
		}
		else {
			dataToH5Attribute(d="IMAGE", fid,path,"CLASS");
			dataToH5Attribute(d="1.2", fid,path,"IMAGE_VERSION");
			if (attributes["IMAGE_SUBCLASS"].toStr() == "IMAGE_INDEXED"){
				// linkToH5Attribute()
				//H5Lcreate_hard();
			}
		}
		// return;
	}
	else {
		/// ... otherwise handle as H5Group, ie. continue iteration
		//hid_t gid = 0;

		if (path != "/"){
			//gid = H5Gcreate(fid,path.c_str(),0);H5P_DEFAULT
			const hid_t gid = H5Gcreate2(fid,path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
			if (gid < 0)
				mout.warn() << ": H5Gcreate failed, path=" << path << mout.endl;
			// TODO: close() ?
			int status = H5Gclose(gid);
			if (status < 0)
				mout.error() << ": H5 close failed, path=" << path << mout.endl;
		}

		const std::string separator = (path == "/") ? "" : "/";
		for (HI5TREE::const_iterator it = tree.begin(); it != tree.end(); ++it) {
			treeToH5File(it->second, fid, path + separator + std::string(it->first));
		}


	}

	/// Copy attributes (group or image)
	for (drain::VariableMap::const_iterator it = attributes.begin(); it != attributes.end(); it++){
		dataToH5Attribute(it->second, fid, path, it->first);
	}

}



void linkToH5Attribute(hid_t lid, hid_t fid, const std::string &path, const std::string &attribute){

	drain::Logger mout(hi5::hi5monitor, "Hi5Write", __FUNCTION__);

	int status = 0;

	const hid_t tid = H5T_STD_REF_OBJ;

	// New
	const hsize_t elements = 1;

	//const hid_t sid = isArray ? H5Screate_simple(1, &elements, NULL) : H5Screate(H5S_SCALAR);
	const hid_t sid = H5Screate_simple(1, &elements, &elements);
	if (sid < 0){
		mout.error() << "H5Screate failed for attribute, path=" << path;
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
		mout.error() << "H5Oopen failed, path=" << path << mout.endl;
	//const hid_t gid = H5Gopen2(fid,path.c_str(),H5P_DEFAULT);
	//if (gid < 0)
	//	mout.error()  << ": H5Gopen failed, path=" << path << mout.endl;

	const hid_t aid = H5Acreate2(oid, attribute.c_str(), tid, sid, H5P_DEFAULT, H5P_DEFAULT);
	if (aid < 0)
		mout.error() << "H5Acreate failed, path=" << path << mout.endl;

	// New
	// status = H5Awrite(aid, tid, lid);   // FIXME!

	if (status < 0)
		mout.error() << "H5Awrite failed, path=" << path << mout.endl;


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


void Writer::imageToH5DataSet(const drain::image::Image &image, hid_t fid, const std::string & path){

	drain::Logger mout(hi5::hi5monitor, "Writer", __FUNCTION__ );

	//static const std::string func = "Writer::imageToH5DataSet";
	static int status;

	mout.debug(2) << ": starting, path=" << path << mout.endl;
	mout.debug(3) << image << mout.endl;
	//std::cerr << ": starting,"<< hi5monitor.getVerbosityLevel() << " path=" << path << '\n';

	/// Currently, only 1-channel data supported!
	//const hsize_t rank = image.getChannelCount() <= 1 ? 2 : 3;
	const hsize_t rank = 2;
	//hsize_t dims[rank];
	hsize_t dims[3];

	switch (rank){
	case 3:
		dims[2] = image.getChannelCount();
		//std::cerr << "imageToH5DataSet: (Warning: range==3 experimental.)";
		mout.warn() << "channels: " << image.getChannelCount() <<  " => rank==3 experimental, path=" << path << mout.endl;
		// no break
	case 2:
		dims[1] = image.getWidth();
		dims[0] = image.getHeight();
		break;
	default:
		mout.error() << ": unsupported image data range, path=" << path << mout.endl;
		//throw std::runtime_error("imageToH5DataSet: unsupported image data range");
	}

	const hid_t sid = H5Screate_simple(rank, dims, NULL);
	const hid_t TID = getH5DataType(image.getType());
	const hid_t tid = H5Tcopy(TID);


	const hid_t pid = H5Pcreate(H5P_DATASET_CREATE);
	if (pid < 0)
		mout.warn() << ": H5Pcreate failed, path=" << path << mout.endl;

	/// ZLIB compression
	//  PolarODIM recommendation: zlib compression level 6
	const hsize_t chunkdim[2] = {std::min(hsize_t(20), dims[0]), std::min(hsize_t(20), dims[1]) };
	/*if (dims[0]*dims[1] < 20*20){
		chunkdim[0] = dims[0];
		chunkdim[1] = dims[1];
	}
	 */


	status = H5Pset_chunk(pid, 2, chunkdim);
	if (status < 0)
		mout.warn() << ": H5Pset_chunk failed, path=" << path << mout.endl;

	status = H5Pset_deflate(pid,6);  // ZLib compression level
	if (status < 0)
		mout.warn() << ": H5Pset_deflate failed, path=" << path << mout.endl;


	H5Tset_order(tid, H5T_ORDER_LE);
	//const hid_t did = H5Dcreate(fid, path.c_str(), tid, sid, H5P_DEFAULT);
	const hid_t did = H5Dcreate2(fid, path.c_str(), tid, sid, H5P_DEFAULT, pid, H5P_DEFAULT);
	if (did < 0)
		mout.warn() << ": H5Dcreate failed, path=" << path << mout.endl;

	/*
	 * Write the data to the dataset using default transfer properties.
	 */
	H5Dwrite(did, TID, H5S_ALL, H5S_ALL, H5P_DEFAULT, image.getBuffer() );  // only 1st channel segment will be read!
	if (status < 0)
		mout.warn() << ": H5Dwrite failed, path=" << path << mout.endl;

	/*  Close resources
	 */
	status = H5Sclose(sid);
	if (status < 0)
		mout.warn() << ": H5S close failed." << mout.endl;

	status = H5Tclose(tid);
	if (status < 0)
		mout.warn() << ": H5T close failed." << mout.endl;

	status = H5Dclose(did);
	if (status < 0)
		mout.warn() << ": H5D close failed." << mout.endl;

	status = H5Pclose(pid);
	if (status < 0)
		mout.warn() << ": H5P close failed." << mout.endl;
}

//void Writer::dataToH5AttributeT<std::string>(const drain::Variable & data, hid_t fid, const std::string & path, const std::string & attribute){
void Writer::dataToH5AttributeString(const drain::Variable & data, hid_t fid, const std::string & path, const std::string & attribute){

	drain::Logger mout(hi5::hi5monitor, "Writer", __FUNCTION__ );

	const std::type_info & type = data.getType();
	if (type != typeid(std::string)){
		mout.note() << "converting attribute '" << attribute << "'=" << data << " (" << type.name() << ")' to string" << mout.endl;
	}
	//std::cerr << "StringTools attribute " << attribute << '\n';

	const std::string dataStr = data.toStr();

	hid_t tid = H5Tcopy(H5T_C_S1);
	H5Tset_size(tid, dataStr.size() +1); // otherwise errors for empty std::strings
	hid_t sid = H5Screate(H5S_SCALAR);

	// Open group or dataset, whichever...
	hid_t oid = H5Oopen(fid,path.c_str(),H5P_DEFAULT);
	hid_t aid = H5Acreate2(oid, attribute.c_str(), tid, sid, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite (aid, tid, dataStr.c_str());

	H5Aclose(aid);
	H5Oclose(oid);
	H5Sclose(sid);
	H5Tclose(tid);

}

/*
struct AttrWriter {

	AttrWriter(const drain::Variable & data, hid_t fid, const std::string &path, const std::string & attribute) :
		data(data), fid(fid), path(path), attribute(attribute)
	{};

	const drain::Variable &data;
	hid_t fid;
	const std::string &path;
	const std::string &attribute;

	template <class T>
	static
	void callback(AttrWriter & w){
		Writer::dataToH5AttributeT<T>(w.data, w.fid, w.path, w.attribute);
	}
};


void Writer::dataToH5Attribute(const drain::Variable &data, hid_t fid, const std::string &path, const std::string &attribute){

	const std::type_info & type = data.getType();
	AttrWriter a(data, fid, path, attribute);
	drain::Type::call<AttrWriter>(a, type);
}
*/

void Writer::dataToH5Attribute(const drain::Variable &d, hid_t fid, const std::string &path, const std::string &attribute){

	drain::Logger mout(hi5::hi5monitor, "Hi5Write", __FUNCTION__);

	if (d.isCharArrayString() || (d.getType() == typeid(std::string))){
		dataToH5AttributeString(d, fid, path, attribute);
		return;
	}

	if (d.getType() == typeid(bool)){
		const drain::Variable d2 = d.toStr();
		dataToH5AttributeString(d2, fid, path, attribute);
		return;
	}


	int status = 0;

	const hid_t tid = getH5DataType(d.getType());    //getH5DataType(typeid(T));

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




} // ::hi5


// Rack
