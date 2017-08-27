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

//#include <drain/image/Image.h>
//#include <drain/prog/Command.h>

#ifndef COMMANDS_IMAGE
#define COMMANDS_IMAGE


#define REGISTRY_IMAGE "image"

#include "image/BasicOps.h"

//#include <drain/prog/Command.h>
//#include "Drainage.h"
#include "Command.h"

namespace drain {

using namespace image;

/// Connects image operators for drainage
class ImageOpAdapter : public Command {

public:


	ImageOpAdapter(ImageOp & op) : imageOp(op), fullName(std::string(__FUNCTION__)+"<"+op.getName()+">") { //
	};

	ImageOpAdapter(const ImageOpAdapter & a) : imageOp(a.imageOp), fullName(std::string(__FUNCTION__)+"<"+a.getName()+">") {
		imageOp.getParameters().updateFromMap(a.imageOp.getParameters());
	}

	~ImageOpAdapter(){};

	ImageOp & imageOp;

	inline
	const std::string & getName() const { return imageOp.getName(); };

	/// Returns the name in format ImageOpAdapter<...> .
	inline
	const std::string & getNameFull() const { return fullName; };

	inline
	const std::string & getDescription() const { return imageOp.getDescription(); };

	virtual
	inline
	const ReferenceMap & getParameters() const { return imageOp.getParameters(); };  // or getParameters

	virtual
	inline
	void run(const std::string & params = ""){
		imageOp.setParameters(params);
		exec();
	}

	virtual
	void exec() const = 0;


protected:

	const std::string fullName;

};



} // drain::

#endif

// Drain
