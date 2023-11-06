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


#include "drain/util/Log.h"


#include "ImageOpBank.h"


namespace drain
{

namespace image
{

ImageOp & ImageOpBank::getComplete(const std::string & query, char assignmentChar, char separatorChar, const drain::SmartMap<std::string> & aliasMap){

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), "ImageOpBank", __FUNCTION__);

	size_t index = query.find(separatorChar);
	const bool PARAMS = (index != std::string::npos);
	const std::string n = PARAMS ? query.substr(0,index) : query;
	const std::string name = aliasMap.get(n, n);  // :-D default value is the key itself.
	const std::string params = PARAMS ? query.substr(index+1) : "";
	mout.debug() << "op: " << name << '\t' << params << mout.endl;
	if (!has(name)){
		mout.warn() << *this << mout.endl;

		mout.error() << "invalid op: '" << name << "' extracted from query: '" << query << "'" << mout.endl;
	}

	ImageOp & op = get(name);
	if (PARAMS)
		op.setParameters(params, assignmentChar, separatorChar);

	return op;
}

std::ostream & operator<<(std::ostream & ostr, const ImageOpBank & bank){
	for (ImageOpBank::map_t::const_iterator it = bank.getMap().begin(); it != bank.getMap().end(); ++it){
		const ImageOp & op = it->second->getSource();
		//ostr << it->first << ':' << '\t' << op.getName() << ',' << op.getClassName() << ',' << op.getParameters() << '\n';
		ostr << it->first << ' ' << '(' << op.getName() << ')' << ':' << ' ' << op.getDescription() << '\n';
		ostr << '\t' << op.getParameters() << '\n';   // consider op.help() ?
	}
	return ostr;
}



/**
 *
 */
//template <class CMD> // rack:: CMD=ImageOpRacklet
class ImageOpInstaller { // example

public:

	ImageOpBank & bank;

	ImageOpInstaller(ImageOpBank & bank): bank(bank){
	}


	/// Add ImageOp command to registry (CommandBank).
	/**
	 * \tparam OP - Class derived from ImageOp
	 */
	template <class OP>
	void add(const std::string & name = OP().getName()){

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		std::string key(name);
		//std::string name(OP().getName());
		drain::CommandBank::deriveCmdName(key, 'i' ); // ImageOpAdapter2<>::getPrefix()
		mout.warn() << name << ">" <<  OP().getName() << '>' << key << '\n';
		try {
		//bank.addOp(key);
			//static OP op;
			//static BeanCommand<OP> cmd(op);
			//drain::Command & cmd = bank.add<ImageOpRacklet<OP> >(key);
			//cmd.section = ImageOpAdapter2<>::getSectionFlag();
		}
		catch (const std::exception &e) {
			mout.warn() << name  << ">" <<  OP().getName()  << '>' << mout.endl;
			mout.fail() << e.what() << mout.endl;
		}
	}



};



ImageOpBank & getImageOpBank() {

	/// Shared ImageOp bank

	//CommandBank spessu;
	static ImageOpBank bank;

	//CommandBank jassu;
	//ImageOpInstaller installer; //(jassu);


	if (bank.getMap().empty()){
		installImageOps(bank);
	}

	// std::cerr << "--------------------------- jassu: " << std::endl;
	// std::cerr << "--------------------------- spessu: " << std::endl;

	return bank;

}



}

}

// Drain
