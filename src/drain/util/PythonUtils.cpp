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

#include "PythonUtils.h"
#include "PythonSerializer.h"

namespace drain {

const std::string PythonConverter::TRIPLE_HYPHEN = "\"\"\"";

//std::string PythonConverter::content="return self._make_cmd(locals())";

void PythonConverter::exportCommands(const std::string &name, const drain::CommandBank & commandBank, std::ostream & ostr, int indentLevel) const {

	indent(ostr, indentLevel);
	ostr << "class " << name << ':' << '\n' << '\n';

	indent(ostr, indentLevel+1);
	ostr << "# defaultCmdKey="<< commandBank.defaultCmdKey << "\n";

	indent(ostr, indentLevel+1);
	ostr << "# execFileCmd=" << commandBank.execFileCmd << "\n";

	indent(ostr, indentLevel+1);
	ostr << "# commandBank.scriptCmd="<< commandBank.scriptCmd << "\n";

	// int i = 10;
	for (const auto & entry: commandBank.getMap()){
		const std::string & key = entry.first;
		const drain::Command & command = entry.second->getSource();

		if (key == commandBank.defaultCmdKey){
			indent(ostr, indentLevel+1);
			ostr << "# TODO: key == commandBank.defaultCmdKey...\n";
			// exportCommand(key, command, ostr, indentLevel+1);
		}
		else if (command.getName() == commandBank.execFileCmd){
			indent(ostr, indentLevel+1);
			ostr << "# NOTE: key == commandBank.execFileCmd TODO...\n";
			// exportCommand(key, command, ostr, indentLevel+1);
		}
		else if (command.getName() == commandBank.scriptCmd){
			indent(ostr, indentLevel+1);
			ostr << "# NOTE: key == commandBank.scriptCmd  TODO SCRIPT QUOTE check...\n";
			// exportCommand(key, command, ostr, indentLevel+1);
		}
		else {
			//exportCommand(key, command, ostr, indentLevel+1);
		}
		exportCommand(key, command, ostr, indentLevel+1);

		ostr << '\n';
		--counter;
		if (counter==0){
			break;
		}
	}
	// ostr << std::string(indentChar, indent+1) << "pass?";


}

void PythonConverter::exportCommand(const std::string & name, const drain::Command & command, std::ostream & ostr, int indentLevel) const {

	const drain::ReferenceMap & params = command.getParameters();
	const drain::ReferenceMap::unitmap_t & u = params.getUnitMap();

	drain::PythonSerializer pyser;
	// drain::Output pyDump(filename);
	// std::ostream & pyDump = std::cout; //("dump.py");

	//  Signature:

	indent(ostr, indentLevel);
	ostr << "def " << name << '(';
	ostr << "self";

	char separator = ',';

	drain::ReferenceMap::keylist_t keys = params.getKeyList();

	for (const auto & key: keys){

		const Reference & param = params[key];

		if (separator){
			ostr << separator << '\n';
			indent(ostr, indentLevel+1);
		}
		else {
			separator = ','; // params.separator;
		}

		// String -> plain
		ostr << key << ':';
		const std::type_info & t = param.getType();
		if (param.getElementCount() > 1){
			ostr << "list";
		}
		else if (t == typeid(bool)){
			ostr << "bool";
		}
		else if (drain::Type::call<drain::typeIsInteger>(t)){
			ostr << "int";
		}
		else if (drain::Type::call<drain::typeIsFloat>(t)){
			ostr << "float";
		}
		else {
			ostr << "str";
			// ostr << entry.second;

		}
		// JSONvalueOut?
		if (param.isString()){
			// Force default
			ostr << '=';
			// pyser.stringToStream(ostr, "");
			pyser.stringToStream(ostr, param.toStr().c_str());
		}
		else if (!param.empty()){
			ostr << '=';
			if (param.getElementCount() > 1){
				// std::list<std::string> l;
				std::list<double> l;
				param.toSequence(l);
				pyser.iterableToStream(ostr, l, drain::Serializer::LIST);
			}
			else if (t == typeid(bool)){
				pyser.boolToStream(ostr, param);
			}
			/*
			else if (param.isString()){ // why not type?
				pyser.stringToStream(ostr, param.toStr().c_str());
			}
			*/
			else if (drain::Type::call<drain::typeIsFloat>(t)){
				pyser.floatToStream(ostr, param);
			}
			else {
				pyser.toStream(ostr, param);
			}
		}

	}
	ostr << "):\n";

	indent(ostr, indentLevel+1);
	ostr << TRIPLE_HYPHEN << ' ' << command.getDescription() << '\n';
	ostr << '\n';

	indent(ostr, indentLevel+1);
	ostr << "--- Parameters ---\n";
	for (const auto & key: keys){
		const Reference & param = params[key];
		//for (const auto & entry: m){
		indent(ostr, indentLevel+1);
		ostr << key << ':' << param << '\n';
	}
	indent(ostr, indentLevel+1);
	ostr << TRIPLE_HYPHEN << '\n';

	if ((params.size()>1) && (params.separator != ',')){
		indent(ostr, indentLevel+1);
		ostr << "# note: separator '" << params.separator << "'\n";

		indent(ostr, indentLevel+1);
		ostr << "cmd = self.make_cmd(locals())\n";
		indent(ostr, indentLevel+1);
		ostr << "cmd.setSeparators('"<< params.separator << "', ',')\n";
		indent(ostr, indentLevel+1);
		ostr << "return cmd\n";
	}
	else {
		indent(ostr, indentLevel+1);
		ostr << "return self.make_cmd(locals())";
	}
	ostr << '\n'; // needed
	/*
	for (const std::string & line: content){
		indent(ostr, indentLevel+1);
		ostr << line;
		ostr << '\n'; // needed
	}
	*/
	ostr << '\n';
	//ostr << "pass\n";
}

}
