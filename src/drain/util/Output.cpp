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


//#include "Log.h"
#include <stdexcept>
#include "Output.h"


namespace drain {

/*
Output::Output(const std::string & filename){ // : filename(filename){

	//drain::Logger mout(__FILE__, __FUNCTION__);

	if (filename.empty())
		throw std::runtime_error("drain::Output(...):  filename empty (use '-' for stdout)");
		//mout.error("filename empty (use '-' for stdout)" );

	if (filename == "-"){
		//mout.debug("opening standard output" );
	}
	else {
		ofstr.open(filename.c_str(), std::ios::out);
		if (!ofstr.is_open()){//"drain::Output(...):  filename empty (use '-' for stdout)"
			throw std::runtime_error(filename + ": drain::Output(filename) opening failed");
			//mout.error("opening '" , filename , "' failed" );
		}
	}
}
*/

Output::~Output(){
	// drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.debug("closing... " );
	ofstr.close();
}

void Output::open(const std::string & filename){

	if (filename.empty())
		throw std::runtime_error("drain::Output(...):  filename empty (use '-' for stdout)");

	if (filename == "-"){
		// if (ofstr.isOpen...)
		ofstr.close(); // close std?
		//mout.debug("opening standard output" );
	}
	else {
		ofstr.open(filename.c_str(), std::ios::out);
		if (!ofstr.is_open()){
			throw std::runtime_error(filename + ": drain::Output(filename) opening failed");
		}
	}
}



Output::operator std::ostream & (){
	// drain::Logger mout(__FILE__, __FUNCTION__);

	if (ofstr.is_open()){
		return ofstr;
	}
	else {
		return std::cout;
	}

}



} // drain
