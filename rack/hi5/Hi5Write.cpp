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


void Writer::writeFile(const std::string &filename, const Hi5Tree &tree){

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL hi5::hi5monitor, __FUNCTION__, __FILE__);

	const hid_t fid = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	if (fid < 0)
		mout.error() << ": H55create failed, file=" << filename << mout.endl;


	/*
	std::vector<std::pair<const char *, double> > v;
	v.resize(3);
	v[0].first  = "key1";
	v[0].second = 12.34;
	v[1].first  = "abcdåäö";
	v[1].second = 56.78;
	Writer::vectorToH5Compound(v, fid, "legend", "index", "coeff");


	/// Ok with float, fails with double
	std::map<int, float> m;
	m[4] = 123.456;
	m[2] = 767.898;
	Writer::mapToH5Compound(m, fid, "legend2");

	//std::map<int, const char *> m;

	std::map<const char *, double> m2;
	m2["eka"] = 1.23;
	m2["toka"] = 4.56;
	Writer::mapToH5Compound(m2, fid, "legend3", "index", "str");
	*/

	treeToH5File(tree, fid, "/");



	int status = H5Fclose(fid);
	if (status < 0)
		mout.warn() << ": H55close failed, file=" << filename << mout.endl;

	//hi5::debug(tree);
}


void Writer::treeToH5File(const Hi5Tree &tree, hid_t fid, const Hi5Tree::path_t &path){

	// hi5::hi5monitor,
	drain::Logger mout("Writer", __FUNCTION__);

	mout.debug(1)  << "path=" << path << mout.endl;

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
		imageToH5DataSet(image, fid, path);
		//drain::Variable d(typeid(std::string));
		if (attributes["CLASS"].toStr() == "PALETTE"){

			mout.warn() << "unimplemented code (palette creation) " << path << mout.endl;
			/*
			dataToH5Attribute(d="RGB",       fid, path, "PAL_COLORMODEL");
			dataToH5Attribute(d="STANDARD8", fid, path, "PAL_TYPE");
			dataToH5Attribute(d="1.2",       fid, path, "PAL_VERSION");
			*/
			//attributes["PAL_COLORMODEL"] = "RGB";
			//attributes["PAL_MINMAXNUMERIC"] = std::string(); // << min , max "0,255";
			//attributes["PAL_TYPE"] = "STANDARD8";
			//attributes["PAL_VERSION"] = "1.2";
		}
		else {
			dataToH5Attribute("IMAGE", fid, path,"CLASS");
			dataToH5Attribute("1.2", fid, path,"IMAGE_VERSION");
			if (attributes["IMAGE_SUBCLASS"].toStr() == "IMAGE_INDEXED"){

				mout.debug() << "future option: linking palette " << path << mout.endl;
				/*
				 TODO: add palette path using "quantity"? (Avoid re-creating quantitie?s)
				// Create (dummy) palette:
				drain::image::Image data;
				data.setType<unsigned char>();
				data.setGeometry(3, 256);
				// TEST data
				for (int i = 0; i < 256; ++i){
					data.put(0, i, i);
					data.put(1, i, 128-abs(i-128));
					data.put(2, i, 255-i);
				}
				const Hi5Tree::path_t palettePath("dataset1/palette");
				hid_t did = imageToH5DataSet(data, fid, palettePath);
				dataToH5Attribute("PALETTE",   fid, palettePath,"CLASS");
				dataToH5Attribute("RGB",       fid, palettePath, "PAL_COLORMODEL");
				dataToH5Attribute("STANDARD8", fid, palettePath, "PAL_TYPE");
				dataToH5Attribute("1.2",       fid, palettePath, "PAL_VERSION");
				// linkToH5Attribute()
				// drain::image::Image & palette = Hi5Base::getPalette(tree(path));
				// THIS data-to-data link WORKS WELL: H5Lcreate_hard(fid, "dataset1/palette", fid, "dataset1/data2/palette-link-test", 0 ,0);
				const hsize_t elements = 1;
				hid_t links[elements];
				links[0] = did;
				const hid_t sid = H5Screate_simple(1, &elements, &elements);
				const hid_t oid = H5Oopen(fid, "/dataset1/data2/data", H5P_DEFAULT);
				const hid_t aid = H5Acreate2(oid, "PALETTE", H5T_STD_REF_OBJ, sid, H5P_DEFAULT, H5P_DEFAULT);
				H5Awrite(aid, H5T_STD_REF_OBJ, links);
				*/
			}
		}
		if (attributes.hasKey("LEGEND")){

		}
		// return;
	}
	else {
		/// ... otherwise handle as H5Group, ie. continue iteration
		//hid_t gid = 0;


		if (path.size() > 1){ // RO
		//if (path != "/"){
			//gid = H5Gcreate(fid,path.c_str(),0);H5P_DEFAULT
			const hid_t gid = H5Gcreate2(fid, static_cast<std::string>(path).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
			if (gid < 0)
				mout.warn() << ": H5Gcreate failed, path=" << path << mout.endl;
			// TODO: close() ?
			int status = H5Gclose(gid);
			if (status < 0)
				mout.error() << ": H5 close failed, path=" << path << mout.endl;
		}

		//const std::string separator = (path == "/") ? "" : "/";
		for (Hi5Tree::const_iterator it = tree.begin(); it != tree.end(); ++it) {
			Hi5Tree::path_t p(path);
			p << it->first;
			treeToH5File(it->second, fid, p);
			//treeToH5File(it->second, fid, path + separator + std::string(it->first));
		}


	}

	/// Copy attributes (group or image)
	for (drain::VariableMap::const_iterator it = attributes.begin(); it != attributes.end(); it++){
		dataToH5Attribute(it->second, fid, path, it->first);
	}

}


hsize_t Writer::deriveDimensions(const drain::image::Geometry & g, std::vector<hsize_t> & dims, std::vector<hsize_t> & chunkDims){

	drain::Logger mout(__FUNCTION__, __FILE__ );

	hsize_t width    = g.getWidth();
	hsize_t height   = g.getHeight();
	hsize_t channels = g.getChannelCount();

	if (g.getArea() == 0){
		mout.warn() << "empty image, geometry: " << g << mout.endl;
		return 0;
	}

	const hsize_t rank = (channels <= 1) ? 2 : 3;

	dims.resize(rank);
	chunkDims.resize(rank);

	switch (channels){
		case 0:
			mout.warn() << "unsupported image geometry: " << g << mout.endl;
			return 0;
		case 1:
			dims.resize(2);
			dims[0] = height;
			dims[1] = width;
			chunkDims.resize(2);
			chunkDims[0] = std::min(hsize_t(20), height);
			chunkDims[1] = std::min(hsize_t(20), width);
			return 2;
		default:
			dims.resize(3);
			dims[0] = channels;
			dims[1] = height;
			dims[2] = width;
			chunkDims.resize(3);
			chunkDims[0] = std::min(hsize_t(20), channels);
			chunkDims[1] = std::min(hsize_t(20), height);
			chunkDims[2] = std::min(hsize_t(20), width);
			return 3;
	}


}


//, const std::string & path
hid_t Writer::imageToH5DataSet(const drain::image::Image &image, hid_t fid, const Hi5Tree::path_t & path){

	// hi5::hi5monitor,
	drain::Logger mout(__FUNCTION__, __FILE__ );


	mout.debug(2) << ": starting, path=" << path << mout.endl;
	mout.debug(3) << image << mout.endl;
	//std::cerr << ": starting,"<< hi5monitor.getVerbosityLevel() << " path=" << path << '\n';

	std::vector<hsize_t> dims;
	std::vector<hsize_t> chunkDims;

	const hsize_t rank = deriveDimensions(image.getGeometry(), dims, chunkDims);

	if (!rank){
		mout.error() << ": unsupported image data range, path=" << path << mout.endl;
		return 0;
	}

	int status;


	/*
	/// Currently, only 1-channel data supported!
	const hsize_t rank = (image.getChannelCount() <= 1) ? 2 : 3;
	//const hsize_t rank = 2;
	//hsize_t dims[rank];
	hsize_t dims[3];

	/// ZLIB compression
	//  PolarODIM recommendation: zlib compression level 6
	const hsize_t  chunkdim2[2] = {std::min(hsize_t(20), dims[0]), std::min(hsize_t(20), dims[1]) };
	const hsize_t  chunkdim3[3] = {std::min(hsize_t(20), dims[0]), std::min(hsize_t(20), dims[1]), std::min(hsize_t(20), dims[2])};
	const hsize_t *chunkdim;

	switch (rank){
	case 3:
		dims[0] = image.getChannelCount();
		dims[1] = image.getHeight();
		dims[2] = image.getWidth();
		chunkdim = chunkdim3;
		//std::cerr << "imageToH5DataSet: (Warning: range==3 experimental.)";
		mout.warn() << "channels: " << image.getChannelCount() <<  " => rank==3 experimental, path=" << path << mout.endl;
		break;
		// no break
	case 2:
		dims[0] = image.getHeight();
		dims[1] = image.getWidth();
		dims[2] = 0;
		chunkdim = chunkdim2;
		break;
	default:
		mout.error() << ": unsupported image data range, path=" << path << mout.endl;
		//throw std::runtime_error("imageToH5DataSet: unsupported image data range");
	}
	*/

	const hid_t sid = H5Screate_simple(rank, &dims[0], NULL);
	const hid_t TID = getH5NativeDataType(image.getType());
	const hid_t tid = H5Tcopy(TID);


	const hid_t pid = H5Pcreate(H5P_DATASET_CREATE);
	if (pid < 0)
		mout.warn() << ": H5Pcreate failed, path=" << path << mout.endl;

	/// ZLIB compression
	//  PolarODIM recommendation: zlib compression level 6
	//const hsize_t chunkdim[2] = {std::min(hsize_t(20), dims[0]), std::min(hsize_t(20), dims[1]) };
	/*if (dims[0]*dims[1] < 20*20){
		chunkdim[0] = dims[0];
		chunkdim[1] = dims[1];
	}
	 */
	//status = H5Pset_chunk(pid, 2, chunkdim);
	status = H5Pset_chunk(pid, rank, &chunkDims[0]);
	if (status < 0)
		mout.warn() << ": H5Pset_chunk failed, path=" << path << mout.endl;

	status = H5Pset_deflate(pid,6);  // ZLib compression level
	if (status < 0)
		mout.warn() << ": H5Pset_deflate failed, path=" << path << mout.endl;


	H5Tset_order(tid, H5T_ORDER_LE);
	//const hid_t did = H5Dcreate(fid, path.c_str(), tid, sid, H5P_DEFAULT);
	const hid_t did = H5Dcreate2(fid, static_cast<std::string>(path).c_str(), tid, sid, H5P_DEFAULT, pid, H5P_DEFAULT);
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

	return did;
}

void Writer::dataToH5Attribute(const drain::Variable &d, hid_t fid, const Hi5Tree::path_t &path, const std::string &attribute){

	// hi5::hi5monitor,

	drain::Logger mout("Writer", __FUNCTION__);

	mout.debug(1)  << "path=" << path << mout.endl;

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

	const hid_t tid = getH5NativeDataType(d.getType());    //getH5NativeDataType(typeid(T));

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
	const hid_t oid = H5Oopen(fid, static_cast<std::string>(path).c_str(), H5P_DEFAULT);
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


// ,const std::string &path
void Writer::dataToH5AttributeString(const drain::Variable & data, hid_t fid, const Hi5Tree::path_t & path, const std::string & attribute){

	// hi5::hi5monitor,
	drain::Logger mout("Writer", __FUNCTION__ );

	mout.debug(1)  << "path=" << path << mout.endl;

	if (!data.isString()){
		//mout.note() << "converting attribute " << attribute << "='" << data << "'  to string" << mout.endl;
		mout.error() << "attribute " << attribute << "='" << data << "'  not string" << mout.endl;
		return;
	}

	//const std::string dataStr(data.toStr());

	hid_t tid = H5Tcopy(H5T_C_S1);
	H5Tset_size(tid, data.getElementCount()); // dataStr.size() +1   otherwise errors for empty std::strings
	hid_t sid = H5Screate(H5S_SCALAR);

	// Open group or dataset, whichever...
	hid_t oid = H5Oopen(fid, static_cast<std::string>(path).c_str(), H5P_DEFAULT);
	hid_t aid = H5Acreate2(oid, attribute.c_str(), tid, sid, H5P_DEFAULT, H5P_DEFAULT);
	//H5Awrite (aid, tid, dataStr.c_str());
	H5Awrite (aid, tid, data.getCharArray());

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

size_t createCompound(const drain::VariableMap & m, hid_t obj = 0){

	drain::Logger mout(hi5::hi5monitor, "Writer", __FUNCTION__);

	herr_t status;

	size_t address = 0;

	for (drain::VariableMap::const_iterator it=m.begin(); it!=m.end(); ++it){

		const char * key = it->first.c_str();
		const drain::Variable &v = it->second;

		hsize_t s = 0;

		if (v.isString()){

			///  Variable-length string datatype.
			s = sizeof (hvl_t);

			if (obj){
				hid_t h5type = H5Tcopy(H5T_C_S1);
				status = H5Tset_size(h5type, H5T_VARIABLE);
				status = H5Tinsert(obj,  key, address, h5type);
				if (status < 0)
					mout.warn() << "allocating segment for string '"  << key << "'=" << v << " failed, status=" << status << mout.endl;
			}
		}
		else {
			s = v.getSize();
			if (obj){
				hid_t h5type = Hi5Base::getH5NativeDataType(v.getType());
				status = H5Tinsert(obj,  key, address, h5type);
				if (status < 0)
					mout.warn() << "allocating segment for '"  << key << "'=" << v << " failed, status=" << status << mout.endl;
			}
		}

		mout.warn() << address << "\t=> "  << it->first << "': " << s <<  " bytes" << mout.endl;

		address += s;

	}

	return address; // size
}



struct test_struct {
	int index;
	const char *label;
};


/// UNDER CONSTR. whole VariableMap in one struct.
void Writer::dataToH5Compound(const drain::VariableMap & m, hid_t fid, const std::string &path){

	// High-Level lib hdf5_hl  https://support.hdfgroup.org/HDF5/Tutor/h5table.html
	//

	drain::Logger mout(hi5::hi5monitor, "Writer", __FUNCTION__);

	herr_t status;



    // Compute size
    //hsize_t size = createCompound(m, 0);

	// Create variable-length string datatype.
    hid_t strtype = H5Tcopy (H5T_C_S1);
    status = H5Tset_size(strtype, H5T_VARIABLE);
    if (status < 0)
    	mout.error() << "H5Tset_size failed, " << path << mout.endl;
    //mout.warn() << "allocating segment of '"  << size << " bytes" << mout.endl;

	// Create the compound datatype for memory.
    hid_t memtype = H5Tcreate (H5T_COMPOUND, sizeof(test_struct));
    status = H5Tinsert (memtype, "index", HOFFSET(test_struct, index), H5T_NATIVE_INT); // Hi5Base::getH5NativeDataType(typeid(int));
    if (status < 0)
    	mout.error() << "H5Tinsert failed" << path << mout.endl;

    status = H5Tinsert (memtype, "label", HOFFSET(test_struct, label), strtype); // Hi5Base::getH5NativeDataType(typeid(int));
    if (status < 0)
    	mout.error() << "H5Tinsert 2 failed" << mout.endl;


    //H5Tcreate (H5T_COMPOUND, size);
    //createCompound(m, memtype);

    /*
     * Create the compound datatype for the file.  Because the standard
     * types we are using for the file may have different sizes than
     * the corresponding native types, we must manually calculate the
     * offset of each member.
     */
    hid_t filetype = H5Tcreate (H5T_COMPOUND, 8 + sizeof(hvl_t)); // 64bit
    status = H5Tinsert (filetype, "index", 0, H5T_STD_I64BE); // Hi5Base::getH5NativeDataType(typeid(int));
    status = H5Tinsert (filetype, "label", 8, strtype); // Hi5Base::getH5NativeDataType(typeid(int));
    //H5Tcreate (H5T_COMPOUND, size);
    //createCompound(m, filetype);

	std::vector<test_struct> legend;
	legend.resize(5);
    // keijo esim;
    legend[0].index = 123;
    legend[0].label = "mika";
    legend[1].index = 456;
    legend[2].label = "toka";

    /*
     * Create dataspace.  Setting maximum size to NULL sets the maximum
     * size to be the current size.
     */
    hsize_t dims[1]; // = {1}; // Note: whole VariableMap in one struct
    dims[0] = legend.size();

    hid_t space = H5Screate_simple (1, dims, NULL);

    mout.warn() << "creating compound" << mout.endl;

    hid_t dset = H5Dcreate (fid, path.c_str(), filetype, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    /*
     * Create the dataset and write the compound data to it.
     */
    status = H5Dwrite (dset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &legend.at(0));

}


// UNDER CONSTR.
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

} // ::hi5


// Rack
