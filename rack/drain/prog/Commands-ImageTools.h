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
#include "Command.h"

#include "image/Sampler.h"

#ifndef COMMANDS_GENERAL
#define COMMANDS_GENERAL


namespace drain {

using namespace image;

class CmdCoordPolicy : public SimpleCommand<std::string> {

public:
	/*
	const int CoordinatePolicy::UNDEFINED(0);
	const int CoordinatePolicy::LIMIT(1);
	const int CoordinatePolicy::WRAP(2);
	const int CoordinatePolicy::MIRROR(3);
	const int CoordinatePolicy::POLAR(4);
	*/

	CmdCoordPolicy() : SimpleCommand<std::string>(__FUNCTION__, "Coordinate under/overflow policy: 0=UNDEFINED, 1=LIMIT, 2=WRAP, 3=MIRROR, 4=POLAR", "policy", "","<xUF>[,<yUF>[,<xOF>,<yOF>]]") {
	};

	virtual
	void filter(Image & dst) const;

};

/// Base class for sampling image data.
/*
 *  Derived classes may use geographical coordinates etc.
 *
 */
class CmdSampler : public BasicCommand {

public:

	inline
	CmdSampler(Sampler & sampler) : BasicCommand(__FUNCTION__, "Set sampling configuration."), sampler(sampler) {
		setReferences();
	};

	Sampler & sampler; // see separare Sampler for rack

protected:

	inline
	CmdSampler(Sampler & sampler, const std::string & name, const std::string & description) : BasicCommand(name, description), sampler(sampler) {
		setReferences();
	};

private:

	void setReferences();

};


/**
\code
drainage --geometry 256,256,1 --plot 96,96,255 --plot 160,96,208  --plot 128,160,192 $BITS16 -o dots.png
drainage --geometry 256,256,1 --plot 96,96,255 --plot 160,96,208  --plot 128,160,192 $BITS16 -o dots-16b.png
drainage --geometry 256,256,3,1 --plot 96,96,255,64,32,255  --plot 160,96,64,255,32,208  --plot 128,160,64,32,255,192  -o dots-rgba.png
drainage --geometry 256,256,3,1 --plot 96,96,255,64,32,255  --plot 160,96,64,255,32,208  --plot 128,160,64,32,255,192 --target S --copy f,f -o dots-rgba-16b.png
\endcode
 *
 */
class CmdPlot: public SimpleCommand<std::string> {
    public: //re 

	CmdPlot() : SimpleCommand<std::string>(__FUNCTION__, "Set intensity at (i,j) to (f1,f2,f3,...). See 'plotFile' and 'fill'.",
			"value", "0,0,0", "<i>,<j>,<f1>[,f2,f3,alpha]" ) {
	};

	void filter(Image & dst) const;

};


class CmdPlotFile: public SimpleCommand<std::string> {

public:

	CmdPlotFile() : SimpleCommand<std::string>(__FUNCTION__, "Plots a given file.  See 'plot'.",
			"filename", "", "<string>") {
	};

	void filter(Image & dst) const;

};

}  // drain::



#endif

// Drain
