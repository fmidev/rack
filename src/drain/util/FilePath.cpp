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
//#include "Path.h"


#include <stdexcept>
#include <iostream>

#include "Log.h"

#include "FilePath.h"

namespace drain {

// Note: inside [ ] no specials like \S \s \W \w ...
// Consider static RegExp extensionRegExp("\\.([[:alnum:]]+)$");
//const RegExp File::pathRegExp("^((\\S*)/)?([^/ ]+)\\.([^\\. ]+)$");
//const RegExp FilePath::pathRegExp("^((\\S*)/)?([^/ ]+)\\.([[:alnum:]]+)$");
/*

0 ==	 drain/examples/RegExp-example.cpp
1  = 	'drain/examples/'
2  = 	'drain/examples'
3  = 	'RegExp-example'
4  = 	'cpp'

*/


/// In Linux and Unix...
//  Note: pathRegExp should be flexible, respectively
//char FilePath::separator('/');

FilePath::FilePath(const std::string & s){
	//dir.separator.acceptLeading = true;
	//dir.separator.acceptTrailing = false; //??
	set(s);
}

FilePath::FilePath(const FilePath & p) : dir(p.dir), basename(p.basename), extension(p.extension){
}


void FilePath::set(const std::string & s){

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL __FILE__, __FUNCTION__);

	this->dir.clear();
	this->basename.clear();
	this->extension.clear();

	if (s.empty()){
		// noi ^
	}
	else {

		//static const RegExp pathRegExp("^((\\S*)/)?([^/ ]+)\\.([[:alnum:]]+)$");
		static const RegExp pathRegExp("^((\\S*)/)?([^/ ]+)$");
		//static const RegExp pathRegExp("^((\\S*)/)?([^/ ]+)$");


		RegExp::result_t result;

		if (!pathRegExp.execute(s, result)){

			//mout.special("Result 1: ", sprinter(result));

			//std::cerr << "Regepp: " << sprinter(result) << '\n';
			// for (std::size_t i = 1; i < result.size(); ++i)
			//   mout.warn('\t' , i , "  = \t'" , result[i] , "'" );

			if (result.size() == 4){
				// this->dir.assign(result[2]);  // excludes trailing separator '/'
				this->dir.set(result[2]);     // includes trailing separator '/'
				this->basename  = result[3];
			}
			else if (result.size() == 2){
				this->basename  = result[1];
			}
			else {
				mout.warn("Result: " ,	sprinter(result) );
				mout.error("odd parsing results for file path: " , s );
			}

			//mout.special("Split: ", this->dir, '|', this->basename);

			static const RegExp basenameRegExp("^(.+)\\.([[:alnum:]]+)$");
			if (!basenameRegExp.execute(this->basename, result)){

				//mout.special("Result 2: ", sprinter(result));

				if (result.size() == 3){
					this->basename  = result[1];
					this->extension = result[2];
				}
				else {
					mout.warn("Result: " ,	sprinter(result) );
					mout.error("odd parsing results for file path: " , s );
				}


			}


		}
		else {
			mout.error("could not parse file path: '" , s , "'" );
		}
		//std::cerr << "Result: " << this->dir << this->basename << this->extension << std::endl;
	}
}


} // drain::
