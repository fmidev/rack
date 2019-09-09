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
		//it->second.valueToJSON(ostr);
		ostr << it->second << ' ';
		ostr << ' ';
		it->second.typeInfo(ostr);
		ostr << '\n';
	}

	//const drain::image::Image &d = n.dataSet;
	if (dataSet.getVolume() > 0){
		if (!prefix.empty())
			ostr << prefix << ':';
		//'\t';
		ostr << "image=" << dataSet.getWidth() << ',' << dataSet.getHeight() << ' ';
		//ostr << d.getImageChannelCount() << ' '; // << d.getAlphaChannelCount() << ' ';
		ostr << '[' << drain::Type::getTypeChar(dataSet.getType()) << '@' << dataSet.getEncoding().getByteSize() << ']' << '\n';  // like typeInfo above

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

	if (type == typeid(char)){
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

/// Split full path string to path object and attribute key.
// consider ValueReader, TextReader instead (skipping attrType)
void Hi5Base::parsePath(const std::string & s, HI5TREE::path_t & path, std::string & attrKey, std::string & attrValue, std::string & attrType){

	drain::Logger mout("Hi5Base", __FUNCTION__);

	path.clear();
	attrKey.clear();
	attrValue.clear();
	attrType.clear();

	if (s.empty()){
		mout.warn() << "empty path given" << mout.endl;
	}
	//static const drain::RegExp pathSyntax("^(.*)(:()) ([\\w]+())$",  REG_EXTENDED);

	enum { PATH, ATTR_KEY, ATTR_VALUE, ATTR_TYPE } mode;
	mode = PATH;
	std::string::size_type iMax = s.length() - 1; // >= 0
	std::string::size_type i    = 0;
	std::string::size_type iStart   = 0;
	std::string::size_type iEnd     = 0; // for ATTR_VALUE only, potentially containing spaces
	char c;
	bool END;

	for (std::string::size_type i = 0; i < s.length(); ++i) {

		c = s.at(i);
		END = (i == iMax);

		//mout.warn() << "MODE=" << (int)mode << ", '" << s.substr(i) << "' END=" << END << mout.endl;

		switch (mode) {

		case PATH:
			if (c == ':'){
				path = s.substr(iStart, i-iStart);
				mode = ATTR_KEY;
				iStart = i+1;
			}
			else if (c == ' '){
				path = s.substr(iStart, i-iStart);
				mode = ATTR_TYPE;
				iStart = i+1;
			}
			else if (END){
				path = s.substr(iStart);
				return;
			}
			break;

		case ATTR_KEY:
			if (c == '='){
				attrKey = s.substr(iStart, i-iStart);
				mode = ATTR_VALUE;
				iStart = i+1;
				iEnd   = iStart;
			}
			else if (c == ' '){
				attrKey = s.substr(iStart, i-iStart);
				mode = ATTR_TYPE;
				iStart = i+1;
			}
			else if (END){
				attrKey = s.substr(iStart);
				return;
			}
			break;

		case ATTR_VALUE:
			if (END){
				// mout.warn() << "extracting value... " << s.substr(iStart) << mout.endl;
				attrValue = s.substr(iStart, (iEnd+1)-iStart);
				return;
			}
			else if (c == '['){
				// end reading value, proceed to read type
				attrValue = s.substr(iStart, iEnd-iStart);
				mode = ATTR_TYPE;
				iStart = i+1;
			}
			else if (c == ' '){
				// Scan, but do not update anchor (iEnd)
				// mout.warn() << "space" << mout.endl;
			}
			else {
				// mout.warn() << "read on" << mout.endl;
				iEnd = i+1;
			}

			break;

		case ATTR_TYPE:
			if ((c == '[') || (c == ' ')){
				iStart = i+1;
			}
			else if (c == ']'){
				attrType = s.substr(iStart, i-iStart);
				return;
			}
			else if (END){
				attrType = s.substr(iStart);
				mout.warn() << "missing closing brace ']' for attribute type specification (" <<  attrType << ")" << mout.endl;
				return;
			}
			break;

		default:
			break;
		}

	}

}


void Hi5Base::readTextLine(HI5TREE & dst, const std::string & line){

	drain::Logger mout("Hi5Base", __FUNCTION__);

	HI5TREE::path_t path;
	std::string attrKey;
	std::string attrValue;
	std::string attrType;

	// consider ValueReader, TextReader instead (skipping attrType)
	Hi5Base::parsePath(line, path, attrKey, attrValue, attrType);

	mout.debug(1);
	mout << path      << " : ";
	mout << attrKey   << " =";
	mout << attrValue << " | ";
	mout << attrType  ;
	mout << mout.endl;

	/// Create the node always
	NodeHi5 & n = dst(path).data;

	if (attrKey.empty())
		return;

	char typeCode = 0;

	if (!attrType.empty())
		typeCode = attrType.at(0);

	if (attrKey == "image"){

		/// Set type
		if (typeCode)
			n.dataSet.setType(typeCode); // what about <string>!?!
		//n.dataSet.setType(drain::Type::getType(typeCode)); // what about <string>!?!
		else
			n.dataSet.setType<unsigned char>();

		/// Set geometry (unless..?)
		if (!attrValue.empty()){
			drain::Variable g;
			g.setType(typeid(size_t));
			g.setSize(2);
			g.setSeparator(',');
			g = attrValue;
			switch (g.getElementCount()){
			case 2:
				n.dataSet.setGeometry(g.get<size_t>(0), g.get<size_t>(1));
				break;
			case 1:
				n.dataSet.setGeometry(g.get<size_t>(0), g.get<size_t>(0));
				break;
			case 0:
				break;
			default:
				hi5mout.error() << "Illegal number of data geometry arguments:'" << g << '\'';
			}
			n.dataSet.setGeometry(g.get<size_t>(0), g.get<size_t>(1));
		}
		//std::cerr << "!IMAGE:" << n.dataSet << std::endl;
	}
	else { // non-image

		drain::Variable & a = n.attributes[attrKey];

		static
		const char stringTypeCode = drain::Type::getTypeChar(typeid(std::string));

		if ((typeCode) && (typeCode != stringTypeCode)){
			//mout.warn() << "typeCode=" << typeCode << mout.endl;
			a.setType(drain::Type::getTypeInfo(typeCode));
			//mout.warn() << "typeCode=" << typeCode << " => " << drain::Type::getTypeChar(a.getType()) << mout.endl;
		}
		else if (! a.typeIsSet())
			a.setType(typeid(std::string));

		a = attrValue;
		// mout.warn() << attrKey << "=" << a << " => " << drain::Type::getTypeChar(a.getType())<< mout.endl;

		if (attrKey == "quantity"){
			if (n.attributes.get("gain", 0.0) == 0.0){
				hi5mout.debug() << "Suggesting --completeODIM to proceed" << hi5mout.endl;
			}
		}

	}

}

void Hi5Base::readTextLine(HI5TREE & dst, const HI5TREE::path_t & path, const std::string & key, const std::string & value){

	drain::Logger mout("Hi5Base", __FUNCTION__);

	mout.warn() << "unchecked function" << mout.endl;

	/// Create the node, even if the rest fails, ie. (i == std::string::npos).
	NodeHi5 & n = dst(path).data;
	//std::cerr << "Created: " << path << std::endl;

	if (key.empty())
		return;

	//const std::string key(keyPath, i+1);
	//std::cerr << "Created: " << path << '|' << key << std::endl;


	const size_t l = value.rfind(']');
	const size_t k = value.rfind('[', l);

	static const char stringTypeCode = drain::Type::getTypeChar(typeid(std::string));
	char typeCode;
	//if (!type.empty())
	if (k != std::string::npos)
		typeCode = value.at(k+1);
	else
		typeCode = 0;

	// std::cerr << " l: " << l << ", k:" << k << '\n';
	// std::cerr << " keyPath: " << keyPath << ", key:" << key << ", value=" << value << ", typeCode=" << (int)typeCode  << '\n';
	if (key == "image"){

		/// Set type
		if (typeCode)
			n.dataSet.setType(typeCode); // what about <string>!?!
		//n.dataSet.setType(drain::Type::getType(typeCode)); // what about <string>!?!
		else
			n.dataSet.setType<unsigned char>();

		/// Set geometry (unless)
		if (!value.empty() && (value.at(0) != '[')){
			drain::Variable g;
			g.setType(typeid(size_t));
			g.setSize(2);
			g.setSeparator(',');
			g = value.substr(0, k-1);
			switch (g.getElementCount()){
			case 2:
				n.dataSet.setGeometry(g.get<size_t>(0), g.get<size_t>(1));
				break;
			case 1:
				n.dataSet.setGeometry(g.get<size_t>(0), g.get<size_t>(0));
				break;
			case 0:
				break;
			default:
				hi5mout.error() << "Illegal number of data geometry arguments:'" << g << '\'';
			}
			n.dataSet.setGeometry(g.get<size_t>(0), g.get<size_t>(1));
		}
		//std::cerr << "!IMAGE:" << n.dataSet << std::endl;
	}
	else { // non-image

		drain::Variable & a = n.attributes[key];



		if ((typeCode) && (typeCode != stringTypeCode)){
			//mout.warn() << "typeCode=" << typeCode << mout.endl;
			a.setType(drain::Type::getTypeInfo(typeCode));
			//mout.warn() << "typeCode=" << typeCode << " => " << drain::Type::getTypeChar(a.getType()) << mout.endl;
		}
		else if (! a.typeIsSet())
			a.setType(typeid(std::string));

		/// Trim trailing spaces
		const size_t k2 = value.find_last_not_of(" \t\r\n", k-1);
		if (k2 == std::string::npos)
			a = value.substr(0, k-1);
		else
			a = value.substr(0, k2+1);

		/*
		if (typeCode){
			mout.warn() << "typeCode=" << typeCode << " >> " << drain::Type::getTypeChar(a.getType()) << mout.endl;
		}
		 */
		//n.attributes[key]

	}
	// std::cerr << n.attributes << std::endl;
	// std::cerr << dst << std::endl;
}


void Hi5Base::deleteNoSave(HI5TREE &src){

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it) {
		if (it->second.data.noSave){
			src.erase(HI5TREE::path_t(it->first));
		}
	}

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it) {
		if (! it->second.data.noSave){ // needed?
			deleteNoSave(it->second);
		}
	}

}


std::ostream & operator<<(std::ostream &ostr, const HI5TREE & tree){

	tree.dump(ostr);
	return ostr;
}


} // ::hi5


// Rack
// REP // REP
