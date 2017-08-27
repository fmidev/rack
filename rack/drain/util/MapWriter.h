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

// Drain
