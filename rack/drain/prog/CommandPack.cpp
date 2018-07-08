/*

    Copyright 2001 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */
 
/*
 * RackLet.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: mpeura
 */

#include <fstream>

#include "CommandRegistry.h"
#include "CommandPack.h"
//#include "Drainage.h"

namespace drain {

void CmdHelp::helpOnModules(std::ostream & ostr, const std::string & excludeKey) const {

	const CommandRegistry::SectionMap & sections = getRegistry().getSections();

	ostr << "For help on separate modules, type:\n";
	for (CommandRegistry::SectionMap::const_iterator it = sections.begin(); it != sections.end(); ++it){
		if (it->first != excludeKey){
			if (it->first.empty())
				ostr << " --help [<command>]" << '\n';
			else
				ostr << " --help " << it-> first << '\n';
		}
	}
}

void CmdHelp::exec() const {

	drain::Logger mout(name, __FUNCTION__);

	const CommandRegistry r = getRegistry();

	std::ostream & ostr = std::cout;

	/// Strip possible leading hyphens, so both "--help command" and "--help --command" will work
	const size_t keyStart = value.find_first_not_of('-');
	const std::string key = (keyStart==std::string::npos) ? value : value.substr(keyStart);
	//mout.debug() << "key: " << key << mout.endl;


	if (r.has(key)){
		r.help(key, ostr);
	}
	else {

		ostr << title << '\n' << std::endl;

		if (key == "intro"){
			helpOnModules(ostr);
		}
		else {
			const CommandRegistry & r = getRegistry();

			const CommandRegistry::SectionMap & sections = r.getSections();

			if (sections.find(key) != sections.end()){
				r.helpSections(ostr, key);
				helpOnModules(ostr, key);
				return;
			}
			else {
				mout.warn() << "Help for '" << key << "'  (" << value << ") not found." << mout.endl;
				return;
			}

			/*
				for (std::set<std::string>::const_iterator it = commands.begin(); it != commands.end(); ++it){
					r.help(*it, ostr);
					ostr << '\n';
				}
			 */
		}

		//ostr << "For help on general commands, type:\n\t --help " << std::endl;
		//ostr << "For help on image processing, type:" << std::endl;
		//ostr << "\t --help image" << std::endl;

	}

	exit(0); // standard/needed?
}

CommandEntry<CmdFormat> cmdFormat("format");


class CmdFormatFile : public SimpleCommand<std::string> {
    public: //re 

	//std::string filename;

	CmdFormatFile() : SimpleCommand<>(__FUNCTION__, "Read format for metadata dump from a file","filename","","std::string") {
		// parameters.separators.clear();
		// parameters.reference("filename", filename, "");
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout; = resources.mout;

		std::stringstream sstr;
		std::ifstream ifstr;
		ifstr.open(value.c_str(), std::ios::in);
		if (ifstr.good()){
			for (int c = ifstr.get(); !ifstr.eof(); c = ifstr.get()){
				sstr << (char)c;
			}
			ifstr.close();
			cmdFormat.value = sstr.str();

		}
		else
			mout.error() << name << ": opening file '" << value << "' failed." << mout.endl;

	};

};
CommandEntry<CmdFormatFile> cmdFormatFile("formatFile");

void CommandLoader::run(const std::string & params){
	// getRegistry().scriptify(params, script);
	drain::Logger mout(name, __FUNCTION__); // = getDrainage().mout;

	Script script;

	std::string line;
	std::ifstream ifstr;

	const std::string & filename = params;
	mout.note() << "open list: " << filename << mout.endl;

	ifstr.open(params.c_str());

	while ( std::getline(ifstr, line) ){
		if (!line.empty()){
			mout.debug(1) << line << mout.endl;
			/*
				if (line.at(0) != '#'){
					const size_t i = line.find_first_not_of(" \t");
					const size_t j = line.find(" \t",i);
					cmdInputFile.rackLet.filename = line.substr(i,j);
					mout.note() << "reading: " <<  cmdInputFile.rackLet.filename << mout.endl;
					cmdInputFile.rackLet.exec();
				}
			 */
			if (line.at(0) != '#')
				getRegistry().scriptify(line, script); // adds a line
		}
	}
	ifstr.close();

	getRegistry().run(script);

}

//



//} // namespace unnamed

} /* namespace drain */
