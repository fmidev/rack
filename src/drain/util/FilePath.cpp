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


#include <drain/Log.h>
#include <iostream>
#include <stdexcept>
//#include <sys/stat.h>

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

/*
FilePath::FilePath(const std::string & s){
	//dir.separator.acceptLeading = true;
	//dir.separator.acceptTrailing = false; //??
	set(s);
}
*/

FilePath::FilePath(const FilePath & p) : dir(p.dir), tail(p.tail), extension(p.extension){
}

// FilePath & FilePath::operator<<(const FilePath & path){
void FilePath::append(const FilePath & path){

	this->dir << path.dir;
	if (!(this->tail.empty() && this->extension.empty())){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.warn(" warning: appending path dropped the trailing filename ", this->tail, " (.",  this->extension,  ")");
	}
	this->tail  = path.tail;
	this->extension = path.extension;
	//this->insert(this->end(), path.begin(), path.end());
	// return *this;
}

/*
void FilePath::append(const FilePath & path){
	this->dir.append(path.dir);
	this->tail  = path.tail;
	this->extension = path.extension;
}
*/


void FilePath::handleBasename(const std::string & basename){

	//if (!drain::StringTools::split2(result[3], this->tail, this->extension, '.')){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// mout.warn("check :", basename);

	const size_t i = basename.find_last_of('.');
	if (i == std::string::npos){
		// mout.warn("appending (no period) :", basename);
		this->dir.append(basename);
	}
	else {
		if (i==0){
			// So called hidden file, starting with '.'
			this->tail = basename;
		}
		else {
			// Split tail to basename and extension
			this->tail.assign(basename,0,i);
			if (i<(basename.size()-1)){
				this->extension.assign(basename,i+1);
			}
			else {
				mout.warn("filename (basename) ending with a period '.' :", basename);
			}
		}
		// this->tail  = result[3];
	}
}


void FilePath::append(const std::string & s){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//this->dir.clear();
	this->tail.clear();
	this->extension.clear();

	if (s.empty()){
		// noi ^
	}
	else {

		/*  volume.h5
		 *  /usr/local/rack
		 *  /usr/local/rack.bin
		 *  /usr/local/rack/
		 *  /usr/local/rack/rack
		 *  local/rack
		 */

		/*
		static const char SEP = path_t::separator.character;
		static const drain::StringBuilder<> sb("^((\\S*)", SEP, ")?([^", SEP, " ]+)(", SEP, "?)$");
		static const RegExp pathRegExp(sb);
		      1 leading part, ending with SEP
		      2 leading part, without SEP
		    1 3 tail (basename), not containing SEP
		    2 4 trailing SEP or empty
		 */
		// static const RegExp pathRegExp("^((\\S*)/)?([^/ ]+)$");
		// static const RegExp pathRegExp("^((\\S*)/)?([^/ ]+)\\.([[:alnum:]]+)$");
		// static const RegExp pathRegExp("^((\\S*)/)?([^/ ]+)$");


		static const char SEP = path_t::separator.character;
		static const drain::StringBuilder<> sb("^((\\S*)", SEP, ")?([^", SEP, " ]+)?$");
		static const RegExp pathRegExp(sb);
		/*
		 	  1 leading part ending with SEP
		      2 trailing part, not containing SEP
		 */
		RegExp::result_t result;

		if (!pathRegExp.execute(s, result)){

			/*
			mout.warn("pathRegExp result, ",result.size(), " elements: ", sprinter(result));
			for (std::size_t i = 1; i < result.size(); ++i)
				mout.warn('\t' , i , "  = \t'" , result[i] , "'" );
			*/
			switch (result.size()) {
			case 4:
				// The last segment not containing SEP
				this->dir.append(result[2]);
				this->handleBasename(result[3]);
				break;
			case 3:
				// The segment until the last SEP
				this->dir.append(result[2]);
				break;
			case 2:
				this->handleBasename(result[1]);
				// this->tail  = result[1];
				break;
			default:
				mout.warn("Result: " ,	sprinter(result) );
				mout.error("odd parsing results for file path: " , s );
			}

			/*
			if (result.size() == 5){ // does not end with separator (like '/')
				//this->dir.append(result[2]);
				if (result[4].empty()){
					this->dir.append(result[2]);
					this->tail  = result[3];
				}
				else {
					this->dir.append(result[2], result[3]);
				}
			}
			else if (result.size() == 3){
				if (result[2].empty()){ // does not end with separator (like '/')
					this->dir.append(result[1]);
				}
				else {
					this->tail  = result[1];
				}
			}
			else {
				mout.warn("Result: " ,	sprinter(result) );
				mout.error("odd parsing results for file path: " , s );
			}
			*/

			//mout.special("Split: ", this->dir, '|', this->basename);

			// Strip extension to dedicated member
			/*
			static const RegExp basenameRegExp("^(.+)\\.([[:alnum:]]+)$");
			if (!basenameRegExp.execute(this->tail, result)){

				//mout.special("Result 2: ", sprinter(result));

				if (result.size() == 3){
					this->tail  = result[1];
					this->extension = result[2];
				}
				else {
					mout.warn("Result: " ,	sprinter(result) );
					mout.error("odd parsing results for file path: " , s );
				}


			}
			*/


		}
		else {
			mout.error("could not parse file path: '" , s , "'" );
		}
		//std::cerr << "Result: " << this->dir << this->basename << this->extension << std::endl;
	}
}


int FilePath::mkdir(const FilePath::path_t & dir, int flags){

	drain::Logger mout(__FILE__, __FUNCTION__);


	if (dir.empty()){
		// Well, does not check if current dir is writable.
		return 0;
	}

	FilePath::path_t p;
	for (const FilePath::path_t::elem_t & elem: dir){

		if ((elem == ".")){
			continue;
		}
		else {
			p << elem;

			std::string pstr = p.str();

			mout.attention<LOG_DEBUG+1>("mkdir: ensure ", pstr);

			struct stat s;
			if (stat(pstr.c_str(), &s) == 0){
				if (s.st_mode & S_IFDIR){
					// it's a directory
					mout.ok<LOG_DEBUG>("dir exits: ", pstr);
				}
				else if (s.st_mode & S_IFREG){
					// it's a file
					mout.fail<LOG_ERR>("path exits, but is a file: ", pstr);
					return -1;
				}
				else {
					// something else
					mout.fail<LOG_ERR>("path exits, but is a not a dir: ", pstr);
					return -1;
				}
			}
			else {
				int result = ::mkdir(pstr.c_str(), flags);
				if (result != 0){
					if( errno == EEXIST ) {
						mout.note("dir exists already:" , pstr);
					} else {
						mout.error("cannot create dir: ", pstr, ", errno: ", strerror(errno));
					}
					return result;
				}
			}

		}

	}

	return 0;

	/*

	if (!path.dir.empty()){
		if ((path.dir.front() != ".") && (path.dir.front() != "..")){
			mout.note("creating dir", path.dir);
			if (mkdir(path.dir.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1){
				if( errno == EEXIST ) {
					mout.note("dir exists already");
				} else {
					// something else
					mout.error("cannot create dir: ", path.dir, ", errno: ", strerror(errno));
					// std::cout << "cannot create sessionnamefolder error:" << strerror(errno) << std::endl;
					// throw std::runtime_error( strerror(errno) );
				}
			}
		}
	}
	*/

}

// const FilePath & filepath,
void FilePath::debug(std::ostream & ostr) const {

	ostr << "full path: " << this->str() << '\n';

	ostr << "dir: \t'";
	if (!this->dir.empty()){
		if (this->dir.front().empty()){
			ostr << '^'; //
		}
	}
	ostr << this->dir << "' (" << this->dir.size() << " elements)\n";
	ostr << "tail:\t'" <<  this->tail  << "'\n";
	ostr << "ext: \t'" <<  this->extension << "'\n";
	for (drain::FilePath::path_t::const_iterator it = this->dir.begin(); it != this->dir.end(); ++it){
		ostr << "\t /" << *it  << '\n';
	}
	ostr << '\n';
}

} // drain::
