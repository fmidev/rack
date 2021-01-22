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

#include "drain/util/Input.h"
#include "CommandBankUtils.h"

namespace drain {




void CmdExecFile::exec() const {

	drain::Logger mout(__FUNCTION__, __FILE__); // = getDrainage().mout( ;

	Script script;

	std::string line;
	drain::Input ifstr(value);

	// std::ifstream ifstr;
	// const std::string & filename = params;
	// mout.note() << "open list: " << filename << mout.endl;
	// ifstr.open(params.c_str());

	while ( std::getline((std::ifstream &)ifstr, line) ){
		if (!line.empty()){
			mout.debug(1) << line << mout.endl;
			if (line.at(0) != '#')
				bank.scriptify(line, script);
				//getRegistry().scriptify(line, script); // adds a line
		}
	}
	//ifstr.close();
	bank.run(script, contextCloner);
	//getRegistry().run(script);

}

/*
void CmdFormat::exec() const {
	Context &ctx = getContext<Context>();
	ctx.formatStr = value;
}


void CmdFormatFile::exec() const {

	drain::Logger mout(__FUNCTION__, __FILE__); // = resources.mout; = resources.mout;

	std::stringstream sstr;

	//drain::Input ifstr(value);
	std::ifstream ifstr;
	ifstr.open(value.c_str(), std::ios::in);
	if (ifstr.good()){
		for (int c = ifstr.get(); !ifstr.eof(); c = ifstr.get()){ // why not getline?
			sstr << (char)c;
		}
		ifstr.close();
		Context &ctx = getContext<Context>();
		//Context &ctx = getContext<>();
		ctx.formatStr = sstr.str(); // SmartContext ?

	}
	else
		mout.error() << name << ": opening file '" << value << "' failed." << mout.endl;

};
*/


} /* namespace drain */

// Rack
