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

class PythonIndent {

public:

	std::ostream & ostr = std::cout;
	int indentLevel = 0;
	std::string indentStr = "    ";

	PythonIndent(std::ostream & ostr=std::cout, int indentLevel=0, std::string indentStr="    ") : ostr(ostr), indentLevel(indentLevel), indentStr(indentStr) {
	};

	inline
	void setIndent(int indentLevel){
		if (indentLevel < 0)
			indentLevel = 0;
		this->indentLevel = indentLevel;
	}

	inline
	void operator ++(){
		++this->indentLevel;
	}

	inline
	void operator --(){
		--this->indentLevel;
		if (this->indentLevel < 0)
			this->indentLevel = 0;
	}

	inline
	void indent() const {
		for (int i=0; i<this->indentLevel; ++i){
			ostr << this->indentStr;
		}
	}

	template <typename ... TT>
	void write(TT... args) const {
		this->indent();
		this->flush(args...);
	};


protected:

	template <typename T, typename ... TT>
	void flush(const T & arg, TT... args) const {
		this->ostr << arg;
		this->flush(args...);
	};

	inline
	void flush() const {
		this->ostr << '\n';
	}

};

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

	PythonIndent indent(ostr, indentLevel);
	indent.write(title);
	indent.write(std::string(title.size(), '-'));
};

void PythonConverter::exportCommands(const std::string &name, const drain::CommandBank & commandBank, std::ostream & ostr, int indentLevel) const {

	PythonIndent indent(ostr, indentLevel);

	ostr << '\n';

	indent.write("class ", name, ':');

	++indent;

	indent.write(TRIPLE_HYPHEN, ' ', "Automatic Drain command set export");
	indent.write(TRIPLE_HYPHEN, '\n');

	indent.write( "# defaultCmdKey=", commandBank.defaultCmdKey);
	indent.write("# execFileCmd=", commandBank.execFileCmd);
	indent.write("# commandBank.scriptCmd=", commandBank.scriptCmd);
	ostr << '\n';

	// int i = 10;
	for (const auto & entry: commandBank.getMap()){
		const std::string & key = entry.first;
		const drain::Command & command = entry.second->getSource();

		const bool IMPLICIT = (key == commandBank.defaultCmdKey);
		if (key == commandBank.defaultCmdKey){
			indent.write("# TODO: key == commandBank.defaultCmdKey...");
			// exportCommand(key, command, ostr, indentLevel+1);
		}
		else if (command.getName() == commandBank.execFileCmd){
			indent.write("# NOTE: key == commandBank.execFileCmd TODO...");
			// exportCommand(key, command, ostr, indentLevel+1);
		}
		else if (command.getName() == commandBank.scriptCmd){
			indent.write("# NOTE: key == commandBank.scriptCmd  TODO SCRIPT QUOTE check...\n");
			// exportCommand(key, command, ostr, indentLevel+1);
		}
		else {
			//exportCommand(key, command, ostr, indentLevel+1);
		}
		exportCommand(key, command, ostr, indentLevel+1, IMPLICIT);

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

	drain::PythonSerializer pyser;

	PythonIndent indent(ostr, indentLevel);

	indent.indent();
	//indent.write("def ", name, '(', "self");
	ostr << "def " <<  name << '(' << "self";

	++indent;

	char separator = ',';

	drain::ReferenceMap::keylist_t keys = params.getKeyList();

	for (const auto & key: keys){

		const Reference & param = params[key];

		if (separator){
			ostr << separator << '\n';
			//indent.write(ostr, indentLevel+1);
			indent.indent();
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

	indent.write(TRIPLE_HYPHEN, ' ', command.getDescription());
	ostr << '\n';

	if (!keys.empty()){
		writeTitle(ostr, indentLevel+1, "Parameters");
		const drain::ReferenceMap::unitmap_t & umap = params.getUnitMap();

		for (const auto & key: keys){
			const Reference & param = params[key];
			indent.write(key, ':', getType(param));
			drain::ReferenceMap::unitmap_t::const_iterator it = umap.find(key);
			if (it != umap.end()){
				indent.write("  ", it->second);
			}
		}
	}
	indent.write(TRIPLE_HYPHEN);
	ostr << '\n'; // extra newline required in policy

	indent.write("cmd = self.make_cmd(locals())");
	if (implicit){
		indent.write("cmd.set_implicit()");
	}
	if ((params.size()>1) && (params.separator != ',')){
		indent.write("# note: separator '", params.separator, "'");
		indent.write("cmd.set_separators('", params.separator, "', ',')");
		// indent.write(+1, "return cmd");
	}
	//else {
	indent.write("return cmd");
	ostr << '\n';

	// ostr << "return self.make_cmd(locals())";
	// }
	// ostr << '\n'; // needed
	/*
	for (const std::string & line: content){
		indent.write(ostr, indentLevel+1);
		ostr << line;
		ostr << '\n'; // needed
	}
	*/
	//ostr << "pass\n";
}

}
