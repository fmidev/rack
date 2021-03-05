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


void CmdLog::exec() const {

	Context & ctx = getContext<Context>();
	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
	drain::StringMapper mapper;

	std::string s = mapper.parse(value).toStr(ctx.getStatusMap());

	if (ctx.logFileStream.is_open()){
		mout.warn() << "log file '" << ctx.logFile << "' closed by '"<< s << "'" << mout.endl;
		ctx.log.setOstr(std::cerr);
		ctx.logFileStream.close();
	}

	ctx.logFile = s;

	ctx.logFileStream.open(ctx.logFile, std::ios::out);

	if (ctx.logFileStream.is_open()){
		ctx.log.setOstr(ctx.logFileStream);
		mout.note() << "thread" << ctx.getId() << mout.endl;
		mout.timestamp("START") << ctx.logFile << mout.endl;
		mout.warn() << ctx.logFile<< mout.endl;
		return;
	}

	ctx.log.setOstr(std::cerr);
	mout.error() << "failed in opening log file: " << ctx.logFile << mout.endl;
}

void CmdStatus::exec() const {

	Context & ctx = getContext<Context>();

	std::ostream & ostr = std::cout; // for now...

	const drain::VariableMap & statusMap = ctx.getStatusMap();

	SprinterLayout layout;
	layout.mapChars.separator = '\n';
	layout.arrayChars.separator = '\n';
	layout.stringChars.separator = '\0';
	SprinterBase::sequenceToStream(ostr, statusMap, layout);
	ostr << '\n';
	//ostr << drain::sprinter(statusMap, layout) << '\n';

	/*
	for (drain::FlexVariableMap::const_iterator it = statusMap.begin(); it != statusMap.end(); ++it){
		ostr << it->first << '=' << it->second << ' ';
		it->second.typeInfo(ostr);
		ostr << '\n';
	}
	*/

	//ostr << "errorFlags: " << ctx.statusFlags << std::endl;

};


/*
void CmdExecFile::exec() const {

	Context & ctx = getContext<>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = getDrainage().mout( ;

	Script script;

	std::string filename;

	drain::StringMapper mapper(value); // todo VALID chars what:source

	mout.note() << "open list: " << filename << mout.endl;

	bank.readFile(value, script);
	//bank.run(script, getCloner<Context>()); //?
	mout.unimplemented() << "bank.run(script, getCloner<Context>())" << mout.endl;



}
*/


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
