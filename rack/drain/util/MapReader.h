/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef MAPREADER_H_
#define MAPREADER_H_

#include <iostream>
#include <fstream>

#include <map>

#include "RegExp.h"

// using namespace std;


namespace drain
{

// consider changing back *m => &m ?

/// General utility for reading text entries of type KEY=VALUE into a map.  
/**
 *   TODO: separate from maps, allow user to assign map[key]=value
 *   because key maybe an alias of another (longer) key.
 *
 */
template <class K, class V>
class MapReader
{
public:

	/// Leading and trailing characters to be pruned.
	std::string trimChars;

	// If "limited", does not allow adding new entries but changes only.
	bool limited;

	MapReader() : m(defaultMap) {};
	//MapReader(const MapReader &mr) : m(mr.m) {};
	MapReader(std::map<K,V> &target) : m(&target) {

		trimChars = " \t";
		// WAS lazy hyphen in the leading characters
		//std::string regExpString = "^[ \t]*([a-zA-Z][a-zA-Z0-9_]*)[ \t]*=(.*)$";

		// Indices for the essential patterns in the following regexps
		//KEY = 1;
		//VALUE = 3;

		// Mpeura 2009/07
		//configLineParser.setExpression("^[ \t]*([a-zA-Z][a-zA-Z0-9_\\.]*)[ \t]*(=(.*))?$");
		//commandLineParser.setExpression("^\\-\\-?([a-zA-Z][a-zA-Z0-9_\\.]*)(=(.*))?$");  // Must preserve position 1 and 3!
		setKeySyntax("[a-zA-Z][a-zA-Z0-9_\\.]*");

		//std::string regExpString = "^([a-z]*)$";
		//setRegExp(regExpString);
		//cout << lineParser.test("ksm=kks") << endl;
	};

	virtual ~MapReader(){};

	void setMap(std::map<K,V> &target){
		m = &target;
	}

	/// Defines the std::strings recognized (accepted) as keys. By default: [a-zA-Z][a-zA-Z0-9_\\.]*   TODO: make short/long spex strictness: -a --accept
	void setKeySyntax(const std::string & regExp){
		configLineParser.setExpression(std::string("^[ \t]*(")+regExp+")[ \t]*(=(.*))?$");
		commandLineParser.setExpression(std::string("^\\-\\-?(")+regExp+")(=(.*))?$");  // Must preserve position 1 and 3!
	}

	//std::string regexpString;
	//drain::RegExp commandLineOptionParser;

	void trim(std::string &s){
		std::string::size_type pos1 = s.find_first_not_of(trimChars);
		std::string::size_type pos2 = s.find_last_not_of(trimChars);
		if (pos1 != std::string::npos)
			s = s.substr(pos1,pos2-pos1 + 1);
		else
			s = "";
	}


	// PAH
	/*
    bool setRegExp(const std::string &regExpString){
    	return ( configLineParser.setExpression(regExpString) );
    }
	 */

	/**! Read from file, typically a config file.
	 * 
	 * 
	 */
	void read(const std::string &filename){
		std::ifstream ifstr;
		ifstr.open(filename.c_str());
		read(ifstr);
		ifstr.close();
	}

	/**! Read from a stream.
	 * 
	 */
	void read(std::istream &istr = std::cin){
		std::string line;
		while (getline(istr,line)){
			//cout << "MapReader::read istr: " << line << '\n';
			readConfigEntry(line);
		}
	}

	/**! Read a single line, ie. a KEY=VALUE pair.
	 * 
	 */ 
	void readConfigEntry(const std::string &line){ //const char *line){
		readEntry(line,configLineParser);
	}

	/**! Read a single commandLine entry, ie. a --KEY=VALUE pair.
	 *   TODO: --out-file=koe, --out-file koe, -o koe
	 */ 
	void readCommandLineEntry(const std::string &s){ //const char *line){
		readEntry(s,commandLineParser);
	}

	void readPendingCommandLineArgument(const std::string &s){ 
		value = s;
		trim(value);
		(*m)[key] = value;
		argumentRegistered = true;
	}


	/**! Read a single line, ie. a KEY=VALUE pair.
	 * 
	 */ 
	void readEntry(const std::string &line, drain::RegExp &regExp){ //const char *line){

		regExp.execute(line);

		const std::vector<std::string> &result = regExp.result;

		//std::string::size_type s = result.size();

		/*
		cout << "readEntry, size=" << s << '\n';
		for (unsigned int i = 0; i < s; ++i) {
			cout << i << ' ' << result[i] << '\n';
		}
		 */
		//cout << "VALUE=" << VALUE << result[VALUE] << '\n';
		//cout << "KEY=" << KEY << result[KEY] << '\n';
		//cout << "***** readEntry, size=" << s << '\n'; 


		// If matches --xxxxxxx
		if (!result.empty()){

			key = result[1];
			trim(key);

			/*
			if ((limited)&&(m->find(key) != m->end())){
				throw "Closed map has not key " + key;
			}
			 */

			if (!result[2].empty()){// At least '=' found
				value = result[3];
				trim(value);
				(*m)[key] = value;
				argumentRegistered = true;
			}
			else {
				value = "";
				argumentRegistered = false;  // user is hinted to use readPendingArgument now.
			}
		}
		else { // loose argument
			key="";
			value = line;
			trim(value);
			argumentRegistered = true;
		}



	}

	/**! Reads command line arguments
	 *   TODO: if exists (declared) then...  
	 *   TODO: one at the time, calling read(int argc, char **argv, int i){
	 */
	void read(int argc, char **argv){
		for (int i = 1; i < argc; ++i) {
			readCommandLineEntry(argv[i]);
		}

	}



	inline const std::string &getKey(){return key;};
	inline const std::string &getValue(){return value;};

	//inline bool pendingArgument(){return !hasArgument;};
	inline bool hasArgument(){return argumentRegistered;};

protected:
	std::map<K,V> *m;
	std::map<K,V> defaultMap;

	// For configuration file options
	drain::RegExp configLineParser;

	// For command line options
	drain::RegExp commandLineParser;

	std::string key;
	std::string value;

	// change to "pending argument" etc
	bool argumentRegistered;
	//int KEY;
	//int VALUE;

};

} //namespace

#endif /*MAPREADER_H_*/
