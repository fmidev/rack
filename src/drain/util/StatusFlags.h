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
#ifndef STATUS_FLAGS_H_
#define STATUS_FLAGS_H_


#include <drain/util/EnumFlagger.h>

namespace drain
{


enum Status {UNKNOWN_ERROR=0, INPUT_ERROR=1, DATA_ERROR=2, METADATA_ERROR=4, OUTPUT_ERROR=8, COMMAND_ERROR=16, PARAMETER_ERROR=32 };

template <>
const drain::Enum<Status>::dict_t drain::Enum<Status>::dict;

typedef drain::EnumFlagger<drain::MultiFlagger<Status> > StatusFlags;


/*

struct status_flag_id {
	// For now, just an identity (unique placeholder for the global flag dictionary).
};

class StatusFlags : public GlobalFlags<status_flag_id> {

public:

	static
	const drain::Flagger::ivalue_t INPUT_ERROR; //     = 1;

	static
	const ivalue_t DATA_ERROR; //      = 2;

	static
	const drain::Flagger::ivalue_t METADATA_ERROR; //  = 4;

	static
	const drain::Flagger::ivalue_t OUTPUT_ERROR; //    = 8;

	static
	const drain::Flagger::ivalue_t COMMAND_ERROR;// = 16;

	static
	const drain::Flagger::ivalue_t PARAMETER_ERROR;// = 16;
};
	*/

} // ::drain

#endif
