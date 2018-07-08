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
#ifndef MAPWRITER_H_
#define MAPWRITER_H_

#include <map>

#include <iterator>
//#include <utility>

//#include "RegExp.h"

// using namespace std;

namespace drain
{

template <class K, class V>
class MapWriter
{
public:
	
	MapWriter(){
		
		trimChars = " \t";
	};
	
	virtual ~MapWriter(){};
	
	std::string separator;
	std::string trimChars;
	
	/**! Read from a stream, for example from std input or config file.
	 * 
	 * */
	void write(const std::map<K,V> &m, std::ostream &ostr = std::cout){

 	 	typename std::map<K,V>::const_iterator iter;
    	for (iter  = m.begin(); iter != m.end(); ++iter){
	       //ostr << escape;
    		ostr << iter->first << '=' << iter->second << '\n';
    	}
    }
 

};

} //namespace

#endif /*MAPWRITER_H_*/
