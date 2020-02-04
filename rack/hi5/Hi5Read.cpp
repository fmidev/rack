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

#include <data/ODIMPath.h>
//#include <drain/util/Log.h>
#include <hi5/Hi5Read.h>
#include <util/Log.h>
#include <util/Variable.h>
#include <map>
#include <utility>
// using namespace std;


namespace hi5 {

//const
const int Reader::ATTRIBUTES(1);
//const
const int Reader::DATASETS(2);


void Reader::readFile(const std::string & filename, Hi5Tree & tree, int mode) {

	drain::Logger mout(hi5::hi5monitor, __FUNCTION__, __FILE__);

	// mout.warn() << "Mode: " << mode << mout.endl;

	tree.clear();

	hid_t fid = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

	if (fid < 0){
		mout.error() << "H5Fopen failed for file=" << filename << mout.endl;
	}
	else
		h5FileToTree(fid, tree, mode);

	int status = H5Fclose(fid);

	if (status < 0){
		mout.warn() << "H5Fclose failed for file=" << filename << mout.endl;
	}

	//tree.dump();
	//hi5::debug(tree);

}


/// Recursive  , const std::string &path
void Reader::h5FileToTree(hid_t file_id, const Hi5Tree::path_t & path, Hi5Tree & tree, int mode){

	//drain::Logger mout(hi5::hi5monitor, __FUNCTION__, __FILE__);
	drain::Logger mout(__FUNCTION__, __FILE__);
	// drain::Logger mout(hi5monitor, "Reader::readFile");

	//static const std::string _func = "Reader::h5FileToTree";
	int status = 0;

	//const std::string separator = (path == "/") ? "" : "/";
	//const std::string separator = (path.empty()) ? "" : "/";

	//if (path=="/"){
	if (path.empty()){
		mout.error() << "path empty" << mout.endl;
		return;
	}

	if (!path.front().isRoot()){
		mout.error() << "path does not start with root: " << path << mout.endl;
		return;
		//mout.warn() << "path empty" << mout.endl;
	}

	if (path.size() == 1){ // ROOT
		// if (path == ""){
		if (mode & ATTRIBUTES)
			H5Aiterate2(file_id, H5_INDEX_NAME,H5_ITER_NATIVE, NULL, &iterate_attribute, &(hi5::NodeHi5 &)tree);
			//herr_t H5Aiterate2( hid_t obj_id, H5_index_t idx_type, H5_iter_order_t order, hsize_t *n, H5A_operator2_t op, void *op_data, )
		// status = H5Giterate(file_id, "/", NULL, &iterate, &tree);
	}
	//else
	status = H5Giterate(file_id, static_cast<std::string>(path).c_str(), NULL, &iterate, &tree);
		//status = H5Giterate(file_id, path.c_str(), NULL, &iterate, &tree);

	if (status < 0)
		mout.warn() << "H5Giterate failed, path=" << path << mout.endl;

	H5G_stat_t info;

	for (Hi5Tree::iterator it = tree.begin(); it != tree.end(); ++it) {
		//const std::string &s = it->first;
		const Hi5Tree::path_t::elem_t &s = it->first;
		Hi5Tree &subtree = it->second;
		//const std::string p = path + separator+ s;
		Hi5Tree::path_t p(path);
		p << s;
		std::string pStr(p);

		//mout.note() << "traversing: " << p << " mode=" << mode << mout.endl;
		mout.debug(2) << "traversing: " << pStr << mout.endl;

		status = H5Gget_objinfo(file_id, pStr.c_str(), false, &info);
		//hi5monitor.note() << _func << ": called  H5Gget_objinfo, path=" << p << hi5monitor.endl;
		if (status < 0)
			mout.warn() << ": H5Gget_objinfo failed, path=" << p << mout.endl;

		hid_t g = 0; // = H5Gopen(file_id,p.c_str()); // check if group
		switch (info.type) {
			case H5G_GROUP:
				g = H5Gopen2(file_id, pStr.c_str(), H5P_DEFAULT);
				if (g < 0)
					mout.warn() << ": H5Gopen failed, path=" << p << mout.endl;
				if (mode & ATTRIBUTES)
					status = H5Aiterate2(g, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, &iterate_attribute,&(hi5::NodeHi5 &)subtree);
					//status = H5Aiterate(g,NULL,&iterate_attribute,&(hi5::NodeHi5 &)subtree);
				if (status < 0)
					mout.warn() << ": H5Aiterate failed, path=" << p << mout.endl;

				// Recursion continues:
				h5FileToTree(file_id, p, subtree, mode);

				status = H5Gclose(g);
				if (status < 0)
					mout.warn() << "H5Gclose failed, path=" << p << mout.endl;

				// Recursion continues:
				//h5FileToTree(file_id, p, subtree, mode);

				break;

			case H5G_DATASET:
				if (mode & DATASETS){
					h5DatasetToImage(file_id,p,((hi5::NodeHi5 &)subtree).dataSet);
				}
				break;
			default:
				mout.warn() << "H5Gget_objinfo, no group or dataset, path=" << p << mout.endl;
				break;
		}
		// TODO: read attributes
		//H5Fget_
		//H5Gclose(g);
	}
}


/// Recursive traversal.

herr_t Reader::iterate(hid_t group_id, const char * member_name, void *operator_data){
	Hi5Tree &tree = *(Hi5Tree *)operator_data;
	tree[member_name];
	return 0;
}


// It would be more elegant to hide this behind H5 class. Arrays not yet supported.const H5A_info_t *ainfo,
herr_t Reader::iterate_attribute(hid_t id, const char * attr_name, const H5A_info_t *ainfo, void *operator_data){

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL hi5::hi5monitor, __FUNCTION__, attr_name);

	hi5::NodeHi5 &node = *(hi5::NodeHi5 *)operator_data;
	drain::Variable & attribute = node.attributes[attr_name];

	attribute.reset();  // TODO remove - should be unneeded


	const hid_t a = H5Aopen_name(id, attr_name);
	if (a < 0)
		mout.error() << ": H5Aopen_name failed: " << attr_name  << mout.endl;

	/// Get the native data type. (The conversion will not store the original data type.)
	const hid_t datatype  = H5Tget_native_type(H5Aget_type(a), H5T_DIR_DEFAULT);

	const hid_t aspace = H5Aget_space(a);
	const int rank = H5Sget_simple_extent_ndims(aspace);
	//std::cerr << " rank=" << rank << "\n";

	hsize_t elements = 1;
	if (rank > 0) {
		if (rank > 1) {
			mout.warn() << "arrays of rank > 1 are unsupported; ";
			mout << " attribute=" << attr_name << " , rank=" << rank << mout.endl;
			return -1;
		}
		else {
			//hsize_t dims = new hsize_t[2];
			H5Sget_simple_extent_dims(aspace, &elements, NULL);
			mout.debug() << " Reading attribute: " << attr_name;
			mout << "  rank: " << rank;
			mout << "  elements: " << elements << mout.endl;
		}
	}

	// CONSIDER: iterate with C types,
	if (H5Tequal(datatype, H5T_NATIVE_CHAR)){
		h5AttributeToData<char>(a,datatype,attribute, elements);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_UCHAR)){
		h5AttributeToData<unsigned char>(a,datatype,attribute, elements);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_SHORT)){
		h5AttributeToData<short>(a,datatype,attribute, elements);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_USHORT)){  // or ? H5T_STD_I16LE
		h5AttributeToData<unsigned short>(a,datatype,attribute, elements);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_INT)){
		h5AttributeToData<int>(a,datatype,attribute, elements);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_UINT)){
		h5AttributeToData<unsigned int>(a,datatype,attribute, elements);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_LONG)){
		h5AttributeToData<long>(a,datatype,attribute, elements);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_ULONG)){
		h5AttributeToData<unsigned long>(a,datatype,attribute, elements);
	}
	#ifdef  STDC99
	else if (H5Tequal(datatype, H5T_NATIVE_LLONG), elements){
		h5AttributeToData<long long>(a,datatype,attribute, elements);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_ULLONG)){
		h5AttributeToData<unsigned long long>(a,datatype,attribute, elements);
	}
	#endif
	else if (H5Tequal(datatype, H5T_NATIVE_FLOAT)){
		h5AttributeToData<float>(a,datatype,attribute, elements);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_DOUBLE)){
		h5AttributeToData<double>(a,datatype,attribute, elements);
	}
	else {
		// separate handler for std::strings
		if (H5Tget_class(datatype) == H5T_STRING){

			/*
			char *str = new char[info.data_size+1];
			str[info.data_size] = '\0';
			H5Aread(a, datatype, str);
			attribute = (const char *)str;
			if (H5Tis_variable_str(datatype)){
				mout.warn() << " string variable not H5T_VARIABLE, '" << str << "'" << mout.endl;
			}
			delete str;
			*/

			if (H5Tis_variable_str(datatype)){
				// mout.warn() << " variable-length" << mout.endl;
				char *s = 0;
				//mout.warn() << (long int)s << '\t';
				H5Aread(a, datatype, &s);
				//mout << (long int)s << mout.endl;
				attribute = std::string(s);
			}
			else {
				H5A_info_t info;
				H5Aget_info(a, &info);
				//mout.warn() << " fixed-length [" << info.data_size <<  "]" << mout.endl;
				char *str = new char[info.data_size+1];
				//mout.warn() << (long int)str << '\t';
				str[info.data_size] = '\0';
				H5Aread(a, datatype, str);
				//mout << (long int)str << mout.endl;
				attribute = (const char *)str;
				delete[] str;
			}
			/*
			H5A_info_t info;
			H5Aget_info(a, &info);
			mout.warn() << " '" << attribute << "'" <<"\t [" << info.data_size << "] variable-length=" << (int)H5Tis_variable_str(datatype) << mout.endl;
			*/
		}
		else {
			mout.warn() << ": type not implemented, attr=" << attr_name << " *\n";
			mout << mout.endl;
		}
	}


	int status = H5Aclose(a);
	if (status < 0)
		mout.warn() << "H5Aclose failed for attribute=" << attr_name << mout.endl;

	return 0;
}


///const Hi5Tree::path_t &path
// h5DatasetToImage(hid_t id, const std::string &path, drain::image::Image &image){
void Reader::h5DatasetToImage(hid_t id, const Hi5Tree::path_t & path, drain::image::Image &image){

	drain::Logger mout(hi5::hi5monitor, __FUNCTION__, __FILE__);
	herr_t status = 0;

	mout.debug() << "opening " << path << mout.endl;

	std::string pathStr(path);

	const hid_t dataset = H5Dopen2(id, pathStr.c_str(), H5P_DEFAULT); // H5P_DATASET_ACCESS????
	if (dataset < 0){
		mout.error() << "opening failed for dataset=" << path << mout.endl;
		return;
	}

	if (H5Tget_class(H5Dget_type(dataset)) == H5T_COMPOUND){
		mout.warn() << "skipping compound data at: " << path << mout.endl;
		status = H5Dclose(dataset);
		handleStatus(status, "H5Dget_type failed", mout, __LINE__);
		return;
	}

	const hid_t filespace = H5Dget_space(dataset);




	/// Get the native data type. (The conversion will not store the original data type.)
	//const hid_t datatype  = H5Tget_native_type(dataset, H5T_DIR_DEFAULT);
	const hid_t datatype  = H5Tget_native_type(H5Dget_type(dataset), H5T_DIR_DEFAULT);


	hsize_t rank = H5Sget_simple_extent_ndims(filespace);
	mout.debug(1) << "rank=" << rank << mout.endl;
	if (rank < 2)
		mout.warn() << "H5Sget_simple_extent_dims, problems expected with rank=" << rank << mout.endl;
	if (rank > 3){
		mout.error() << "H5Sget_simple_extent_dims, rank over 3 unsupported, rank=" << rank << mout.endl;
	}
	hsize_t dims[3];
	dims[0] = 0;
	dims[1] = 0;
	dims[2] = 0;


	status = H5Sget_simple_extent_dims(filespace, dims, NULL);
	if (status < 0)
		mout.error() << "H5Sget_simple_extent_dims failed at: " << path << mout.endl;
		//throw std::runtime_error(_func + " error with simple extent dims at " + path);

    // Define the memory space to read dataset.
	const hid_t memspace = H5Screate_simple(rank,dims,NULL);
	if (memspace < 0)
		mout.error() << "opening memspace failed at: " << path << mout.endl;
		//throw std::runtime_error(_func + " opening memspace failed at " + path);


	/// For reading images with several channels
	/*
	 *  single-channel: dims = {height, width}
	 *   multi-channel: dims = {channels,height, width}
	 */
	const bool MULTICHANNEL = (dims[2] > 0);
	if (MULTICHANNEL){
		mout.warn() << "experimental: support for multidimensional data, path=" << path << mout.endl;
	}

	const hsize_t channels = MULTICHANNEL ? dims[0] : 1;        // NEW
	const hsize_t height =   MULTICHANNEL ? dims[1] : dims[0];  // NEW
	const hsize_t width  =   MULTICHANNEL ? dims[2] : dims[1];  // NEW


	//std::cerr << width << "x" << height << '\n';

	//H5Tequal()
	bool typeOk = true;


	//if (H5Tequal(datatype, H5T_NATIVE_UCHAR) || H5Tequal(datatype, H5T_STD_U8BE)){

	// TODO: iterate, use Type::
	if (H5Tequal(datatype, H5T_NATIVE_UCHAR) ){
		image.initialize(typeid(unsigned char), width,height, channels);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_CHAR)){
		image.initialize(typeid(char), width,height, channels);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_USHORT)){
		image.initialize(typeid(unsigned short), width,height, channels);
	}
	else if (H5Tequal(datatype, H5T_STD_I16LE) && (sizeof(signed short) == 2) ){ // H5T_STD_I16LE
		image.initialize(typeid(signed short), width,height, channels);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_INT)){
		image.initialize(typeid(int), width,height, channels);
	}
	/// Added 2016 for sclutter
	else if (H5Tequal(datatype, H5T_NATIVE_LONG)){
		image.initialize(typeid(long int), width,height, channels);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_FLOAT)){
		image.initialize(typeid(float), width,height, channels);
	}
	else if (H5Tequal(datatype, H5T_NATIVE_DOUBLE)){
		image.initialize(typeid(double), width,height, channels);
	}
	// #ifdef  STDC99
	// #endif
	else {
		image.initialize(typeid(char), 0,0);

		//image.initialize(typeid(char>(width,height);
		typeOk = false;
		mout.warn() << "image type (" << datatype << ") not implemented, path=" << path << mout.endl;
		//std::cerr << "Warning: not implemented " << height << '\n';
	}

	//image.initialize(typeid(char>(1,1); // FOR valgrind
	mout.debug(1) << "allocated image: " << image << mout.endl;

    // koe kooe
	if ((image.getGeometry().getVolume() > 0) && typeOk){
		mout.debug(2) << "calling H5Dread" << mout.endl;
		H5O_info_t info;
		H5Oget_info(dataset, &info);

		status = H5Dread(dataset, datatype, memspace, filespace, H5P_DEFAULT, (void *)image.getBuffer()); // valgrind?
		if (status < 0)
			mout.warn() << "H5Dread() failed " << mout.endl;

		image.setName(path);
		mout.debug(2) << "IMAGE: " << image << mout.endl;
		//mout.debug(2) << "IMAGE: " << image.getWidth() << '*' << image.getHeight();
		mout << '*' << image.getChannelCount() << '=' << image.getGeometry().getVolume() << '\n';
		mout << '*' << image.getGeometry() << '\n';
		mout << image << mout.endl;
		//image.toOStr(std::cout);
	}
	else if (typeOk) {
		mout.warn() << " trying to read empty image " << image.getGeometry() << mout.endl;
	}

	//if (drain::Debug > 2)

	/// Todo: status check
	status = H5Dclose(dataset);
	if (status < 0)
		mout.warn() << "H5Dclose() failed " << mout.endl;

	status = H5Tclose(datatype);
	if (status < 0)
			mout.warn() << "H5Tclose() failed " << mout.endl;

	H5Sclose(memspace);
	if (status < 0)
				mout.warn() << "H5Sclose(memspace) failed " << mout.endl;

	H5Sclose(filespace);
	if (status < 0)
		mout.warn() << "H5Sclose(filespace) failed " << mout.endl;

}

} // ::H5


// Rack
 // REP
