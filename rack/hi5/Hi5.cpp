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


#include <fstream>

#include "Hi5.h"
#include <drain/util/ValueReader.h>


// using namespace std;

namespace hi5 {



void NodeHi5::writeText(std::ostream &ostr, const std::string & prefix) const {

	if (attributes.empty() && dataSet.isEmpty()){
		if (!prefix.empty())
			ostr << prefix;
		if (noSave)
			ostr << '~';
		ostr << '\n';
		return;
	}

	if (noSave)
		ostr << '~';

	for (drain::VariableMap::const_iterator it = attributes.begin(); it != attributes.end(); it++){
		if (!prefix.empty())
			ostr << prefix << ':'; //'\t';
		ostr << it->first << '=';
		it->second.valueToJSON(ostr);
		/*
		ostr << it->second << ' ';
		*/
		// ostr << ' ';
		// it->second.typeInfo(ostr);
		ostr << '\n';
	}

	//const drain::image::Image &d = n.dataSet;
	if (dataSet.getVolume() > 0){
		if (!prefix.empty())
			ostr << prefix << ':';
		//'\t';
		ostr << "image=" << dataSet.getWidth() << ',' << dataSet.getHeight();
		// ostr  << ' ' << '[' << drain::Type::getTypeChar(dataSet.getType()) << '@' << dataSet.getEncoding().getByteSize() << ']' << '\n';  // like typeInfo above
		ostr << '\n';
	}

}

std::ostream &operator<<(std::ostream &ostr,const hi5::NodeHi5 &n){
	n.writeText(ostr);
	return ostr;
}


//Hi5Error Hi5Base::debug;
//drain::Log Hi5Base::hi5monitor;
//drain::Logger Hi5Base::hi5mout(Hi5Base::hi5monitor,"hi5");
drain::Log hi5monitor;
drain::Logger hi5mout(hi5monitor,"Hi5");


const hid_t & Hi5Base::getH5DataType(const std::type_info &type){


	if (type == typeid(bool)){  // does not work
		hi5mout.warn() << __FUNCTION__ << ": boolean type '" << type.name() << "' currently unsupported" << hi5mout.endl;
		return H5T_NATIVE_HBOOL; // experimental
	}
	else if (type == typeid(char)){
		return H5T_NATIVE_CHAR;
	}
	else if (type == typeid(unsigned char)){
		return H5T_NATIVE_UCHAR;
	}
	else if (type == typeid(short)){
		return H5T_NATIVE_SHORT;
	}
	else if (type == typeid(unsigned short)){  // and sizeof() = 16? H5T_STD_I16LE
		return H5T_NATIVE_USHORT;
	}
	else if (type == typeid(int)){
		return H5T_NATIVE_INT;
	}
	else if (type == typeid(unsigned int)){
		return H5T_NATIVE_UINT;
	}
	else if (type == typeid(long)){
		return H5T_NATIVE_LONG;
	}
	else if (type == typeid(unsigned long)){
		return H5T_NATIVE_ULONG;
	}
#ifdef  STDC99
	else if (type == typeid(long long)){
		return H5T_NATIVE_LLONG;
	}
	else if (type == typeid(unsigned long long)){
		return H5T_NATIVE_ULLONG;
	}
#endif
	else if (type == typeid(float)){
		return H5T_NATIVE_FLOAT;
	}
	else if (type == typeid(double)){
		return H5T_NATIVE_DOUBLE;
	}
	else {
		hi5mout.error() << __FUNCTION__ << ": unsupported data type:'" << type.name() << '\'' << hi5mout.endl;
		return H5T_NATIVE_UCHAR; /// TODO
	}

}


//drain::image::Image &  Hi5Base::getPalette(HI5TREE & dst){
HI5TREE & Hi5Base::getPalette(HI5TREE & dst){

	HI5TREE & palette = dst["palette"];

	drain::image::Image & data = palette.data.dataSet;
	if (data.isEmpty()){
		data.setType<unsigned char>();
		data.setGeometry(3, 256);
		for (drain::image::Image::iterator it = data.begin(); it != data.end(); ++it){
			*it = rand()&0xff;
		}
		drain::VariableMap & attributes = palette.data.attributes;
		attributes["CLASS"] = "PALETTE"; // palette marker for Hi5Writer
		/*
		attributes["PAL_COLORMODEL"] = "RGB";
		// attributes["PAL_MINMAXNUMERIC"] = std::string(); // << min , max "0,255";
		attributes["PAL_TYPE"] = "STANDARD8";
		attributes["PAL_VERSION"] = "1.2";
		 */
	}

	return palette;
	//return data;

}

void Hi5Base::linkPalette(const HI5TREE & palette, HI5TREE & dst){
	drain::VariableMap & attributes = dst["data"].data.attributes;
	attributes["IMAGE_SUBCLASS"] = "IMAGE_INDEXED";
	attributes["PALETTE_ADDR"] = (long unsigned int) & palette; //(long unsigned int);
}

// const HI5TREE &src,
void Hi5Base::writeText(const HI5TREE &src, const std::list<HI5TREE::path_t> & paths, std::ostream & ostr) {


	for (std::list<HI5TREE::path_t>::const_iterator it = paths.begin(); it != paths.end(); ++it) {

		const std::string &key = *it;
		src(key).data.writeText(ostr, key);

	}
}

void Hi5Base::readText(HI5TREE &src, std::istream & istr) {

	std::string line;

	while ( std::getline(istr, line) ){

		if (!line.empty() && (line.at(0)!='#')){

			// const size_t j = line.find('=');
			// if (j == std::string::npos)
			readTextLine(src, line);
			// else readTextLine(src, line.substr(0, j), line.substr(j+1));

		}

	}

}



void Hi5Base::readTextLine(HI5TREE & dst, const std::string & line){

	//drain::Logger mout("Hi5Base", __FUNCTION__);

	HI5TREE::path_t path;
	std::string attrKey;
	drain::Variable v;

	Hi5Base::parsePath(line, path, attrKey, v);

	hi5mout.debug();
	hi5mout << path      << " : ";
	hi5mout << attrKey   << " =";
	hi5mout << v << " | ";
	hi5mout << hi5mout.endl;

	/// Create the node always
	NodeHi5 & n = dst(path).data;

	if (attrKey.empty())
		return;

	if (attrKey == "image"){

		n.dataSet.setType<unsigned char>();
		n.dataSet.setGeometry(v.get<size_t>(0), v.get<size_t>(1));

	}
	else { // non-image

		drain::Variable & a = n.attributes[attrKey];
		a = v;

		if (attrKey == "quantity"){
			if (n.attributes.get("gain", 0.0) == 0.0){
				hi5mout.debug() << "Suggesting --completeODIM to proceed" << hi5mout.endl;
			}
		}

	}

}


/// Split full path string to path object and attribute key.
// consider ValueReader, TextReader instead (skipping attrType)

void Hi5Base::parsePath(const std::string & line, HI5TREE::path_t & path, std::string & attrKey, drain::Variable & v){

	drain::Logger mout("Hi5Base", __FUNCTION__);

	mout.debug() << "line: " << line << mout.endl;

	typedef std::vector<std::string> strVector;

	//static
	//const drain::RegExp pathSyntax("^[/]?([^: ]+)((:.*)?)$");

	strVector p;
	drain::StringTools::split(line, p, ':');

	path = p[0];
	mout.debug() << "path: " << path << mout.endl;

	if (p.size() > 1){

		mout.debug() << "assignment: " << p[1] << mout.endl;

		strVector assignment;
		drain::StringTools::split(p[1], assignment, '=');

		attrKey = assignment[0];
		mout.debug() << "key: " << attrKey << mout.endl;

		if (assignment.size() == 2){

			std::string & value = assignment[1];

			if (value.empty()){
				v.setType(typeid(std::string));
				return;
			}

			/*
			if (value.at(0) == '"'){
				v.setType();
			}
			*/

			// Test array OR type specification...
			size_t i = assignment[1].find('[');

			if ((i==0) || (i==std::string::npos)){
				mout.debug() << "NEW mode: " << mout.endl;
				drain::ValueReader::scanArrayValues(drain::StringTools::trim(assignment[1], "[] \t\n"), v);
			}
			else {
				drain::ValueReader::scanArrayValues(drain::StringTools::trim(assignment[1].substr(0,i-1)), v);
				mout.note() << "discarding old type code: " << assignment[1].substr(i) << ", guessing " << drain::Type::getTypeChar(v.getType()) << mout.endl;
			}

			/*
			std::cout << "VALUE: ";
			v.valueToJSON(std::cout);
			std::cout << '\n';
			*/

		}
		else {
			mout.note() << "incomplete assignment: " << p[1] << mout.endl;
		}

	}

	// mout.note() << "key: " << attrKey << mout.endl;

}




void Hi5Base::deleteNoSave(HI5TREE &src){

	drain::Logger mout("Hi5Base", __FUNCTION__);

	typedef std::list<HI5TREE::path_t::elem_t> path_elem_list_t;
	std::list<std::string> sl;
	path_elem_list_t l;

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it) {
		if (! it->second.data.noSave){ // needed?
			//mout.debug(1) << "delete: " <<  it->first << mout.endl;
			deleteNoSave(it->second);
		}
		else {
			l.push_back(it->first);
		}
	}

	for (path_elem_list_t::iterator it = l.begin(); it != l.end(); ++it){
		HI5TREE::path_t p;
		p << *it;
		mout.debug(1) << "delete group: " <<  *it << mout.endl;
		src.erase(p);
	}

	/*
	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it) {
		if (it->second.data.noSave){
			mout.debug(1) << "delete group: " <<  it->first << mout.endl;
			src.erase(HI5TREE::path_t(it->first));
		}
	}
	*/

}


std::ostream & operator<<(std::ostream &ostr, const HI5TREE & tree){

	tree.dump(ostr);
	return ostr;
}


} // ::hi5


// Rack
// REP // REP
