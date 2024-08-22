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
/*
 * Time.h
 *
 *  Created on: Aug 31, 2010
 *      Author: mpeura
 */

#include <string>
#include <iostream>
#include <fstream>

#ifndef DRAIN_OUTPUT
#define DRAIN_OUTPUT

#include <drain/Log.h>
#include <drain/util/FilePath.h>

namespace drain {

/// Output utility. Opens stdout with "-". Closes upon destruction.
/**
 *  Note: This class does nout (yet) support format strings of arbitrary size.
 *  (output str length has limited size).
 *
 */
class Output {

public:

	inline
	Output(){};

	inline
	Output(const Output & output){}; // Should this copy something?

	/// Constructor that directly opens a file, or stdout with "-".
	inline
	Output(const std::string & filename){
		open(filename);
	};

	/// Constructor that directly opens a file, or stdout with "-".
	inline
	Output(const char *filename){
		open(filename);
	};

	/// Constructor that directly opens a file.
	inline
	Output(const drain::FilePath & filePath){
		open(filePath.str());
	};

	// Output(std::ostream & ostr);

	/// Closes upon destruction.
	~Output();

	// Opens with std::open. Can be overridden with open(<filename>);
	void open(const std::string & filename);

	operator std::ostream & ();

	inline
	operator std::ofstream & (){
		return ofstr;
	}

	inline
	operator bool (){
		return static_cast<bool>(ofstr);
	};

	inline // std::ofstream &
	std::ostream & getStream(){
		if (ofstr.is_open()){
			return ofstr;
		}
		else {
			return std::cout;
		}
		// return ofstr;
	}


protected:

	//std::ostream   ostr;
	std::ofstream ofstr;

};

template <class T>
inline
Output & operator<<(Output & output, const T& value){
	output.getStream() << value;
	return output;
}

} // drain::


#endif /* DRAIN_OUTPUT_H_ */

