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
#ifndef DRAIN_IMAGE_FILEINFO
#define DRAIN_IMAGE_FILEINFO


//
#include <drain/RegExp.h>

#include "BeanLike.h"
#include "FilePath.h"
#include "Registry.h"


namespace drain
{



class FileInfo {
public:

	FileInfo(const std::string & extRE = "[\\w]+", int flags=REG_ICASE|REG_EXTENDED){
		setExtensionRegExp(extRE, flags);
	}

	void setExtensionRegExp(const std::string & extRE, int flags=REG_ICASE|REG_EXTENDED);

	/// Accepts or rejects a filename extension.
	inline
	bool checkPath(const std::string & path) const {
		if (path == "-")
			return false;
		else
			return checkPath(FilePath(path));
	}

	/// Accepts or rejects the extension extracted from file path.
	/**
	 *  Extracts the extesions in [<directory>/]<filename>.<ext>.
	 *  \param ext - filename extension (without leading period)
	 *  \return - true, if the extension
	 */
	inline
	bool checkPath(const FilePath & filePath) const {
		return checkExtension(filePath.extension);
	}

	/// Accepts or rejects a filename extension.
	/**
	 *  \param ext - filename extension (without leading period)
	 *  \return - true, if the extension
	 */
	inline
	bool checkExtension(const std::string & ext) const {
		return extensionRegexp.test(ext);
	}

	/// Regexp to be set inside parentheses
	drain::RegExp extensionRegexp;


	//static
	//const drain::RegExp noExtension;

};


class FileHandler { // : public BeanLike {

public:

	/*
	inline
	FileHandler(const std::string & name) : BeanLike(name){
	}
	*/
	//static virtual
	//const FileInfo & getFileInfo() = 0;

};

// static
// Registry<FileInfo> & getFileInfoRegistry();


}  // drain

#endif /*FILE_H_*/

// Drain
