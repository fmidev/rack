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

const std::string & PythonConverter::getType(const drain::Castable & arg){
	const std::type_info & t = arg.getType();
	if (!arg.isValid()){
		static const std::string s("NoneType");
		return s;
	}
	else if (arg.getElementCount() > 1){
		static const std::string s("list");
		return s;
	}
	else if (t == typeid(bool)){
		static const std::string s("bool");
		return s;
	}
	else if (drain::Type::call<drain::typeIsInteger>(t)){
		static const std::string s("int");
		return s;
	}
	else if (drain::Type::call<drain::typeIsFloat>(t)){
		static const std::string s("float");
		return s;
	}
	else {
		static const std::string s("str");
		return s;
	}
}


//std::string PythonConverter::content="return self._make_cmd(locals())";
void PythonConverter::writeTitle(std::ostream & ostr, int indentLevel, const std::string & title) const {
	indent(ostr, indentLevel);
	ostr << title << '\n';
	indent(ostr, indentLevel);
	ostr << std::string(title.size(), '-') << '\n';
};

void PythonConverter::exportCommands(const std::string &name, const drain::CommandBank & commandBank, std::ostream & ostr, int indentLevel) const {

	ostr << '\n';

	write(ostr, indentLevel, "class ", name, ':');

	++indentLevel;

	write(ostr, indentLevel, TRIPLE_HYPHEN, ' ', "Rack automatic");
	write(ostr, indentLevel, TRIPLE_HYPHEN, '\n');

	write(ostr, indentLevel, "# defaultCmdKey=", commandBank.defaultCmdKey);
	write(ostr, indentLevel, "# execFileCmd=", commandBank.execFileCmd);
	write(ostr, indentLevel, "# commandBank.scriptCmd=", commandBank.scriptCmd);
	ostr << '\n';

	// int i = 10;
	for (const auto & entry: commandBank.getMap()){
		const std::string & key = entry.first;
		const drain::Command & command = entry.second->getSource();

		const bool IMPLICIT = (key == commandBank.defaultCmdKey);
		if (key == commandBank.defaultCmdKey){
			write(ostr, indentLevel, "# TODO: key == commandBank.defaultCmdKey...");
			// exportCommand(key, command, ostr, indentLevel+1);
		}
		else if (command.getName() == commandBank.execFileCmd){
			write(ostr, indentLevel, "# NOTE: key == commandBank.execFileCmd TODO...");
			// exportCommand(key, command, ostr, indentLevel+1);
		}
		else if (command.getName() == commandBank.scriptCmd){
			write(ostr, indentLevel, "# NOTE: key == commandBank.scriptCmd  TODO SCRIPT QUOTE check...\n");
			// exportCommand(key, command, ostr, indentLevel+1);
		}
		else {
			//exportCommand(key, command, ostr, indentLevel+1);
		}
		exportCommand(key, command, ostr, indentLevel, IMPLICIT);

		ostr << '\n';
		--counter;
		if (counter==0){
			break;
		}
	}
	// ostr << std::string(indentChar, indent+1) << "pass?";


}

void PythonConverter::exportCommand(const std::string & name, const drain::Command & command, std::ostream & ostr, int indentLevel, bool implicit) const {

	const drain::ReferenceMap & params = command.getParameters();
	const drain::ReferenceMap::unitmap_t & umap = params.getUnitMap();

	drain::PythonSerializer pyser;
	// drain::Output pyDump(filename);
	// std::ostream & pyDump = std::cout; //("dump.py");
	// Signature:

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
		ostr << getType(param);

		if (param.isString()){
			// Force default
			ostr << '=';
			pyser.stringToStream(ostr, param.toStr().c_str());
		}
		else if (!param.empty()){
			ostr << '=';
			const std::type_info & t = param.getType();
			if (param.getElementCount() > 1){
				std::list<double> l;
				param.toSequence(l);
				pyser.iterableToStream(ostr, l, drain::Serializer::LIST);
			}
			else if (t == typeid(bool)){
				pyser.boolToStream(ostr, param);
			}
			else if (drain::Type::call<drain::typeIsFloat>(t)){
				pyser.floatToStream(ostr, param);
			}
			else {
				pyser.toStream(ostr, param);
			}

			/*
			else if (param.isString()){ // why not type?
				pyser.stringToStream(ostr, param.toStr().c_str());
			}
			*/
		}

	}
	ostr << "):\n";  // TODO: -> Command

	write(ostr, indentLevel+1, TRIPLE_HYPHEN, ' ', command.getDescription());
	ostr << '\n';

	writeTitle(ostr, indentLevel+1, "Parameters");
	for (const auto & key: keys){
		const Reference & param = params[key];
		write(ostr, indentLevel+1, key, ':', getType(param));
		drain::ReferenceMap::unitmap_t::const_iterator it = umap.find(key);
		if (it != umap.end()){
			write(ostr, indentLevel+2, it->second);
		}
	}
	write(ostr, indentLevel+1, TRIPLE_HYPHEN);
	ostr << '\n'; // extra newline required in policy

	write(ostr, indentLevel+1, "cmd = self.make_cmd(locals())");
	if (implicit){
		write(ostr, indentLevel+1, "cmd.set_implicit()");
	}
	if ((params.size()>1) && (params.separator != ',')){
		write(ostr, indentLevel+1, "# note: separator '", params.separator, "'");
		write(ostr, indentLevel+1, "cmd.set_separators('", params.separator, "', ',')");
		// write(ostr, indentLevel+1, "return cmd");
	}
	//else {
	write(ostr, indentLevel+1, "return cmd");
	ostr << '\n';

	// ostr << "return self.make_cmd(locals())";
	// }
	// ostr << '\n'; // needed
	/*
	for (const std::string & line: content){
		indent(ostr, indentLevel+1);
		ostr << line;
		ostr << '\n'; // needed
	}
	*/
	//ostr << "pass\n";
}

}
