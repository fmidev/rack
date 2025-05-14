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
 
#include <drain/Log.h>
#include <unistd.h>

#include "Context.h"

namespace drain {


/// Defines unique id.
long int Context::counter(0);

// drain::TypeName<Context>::get(),

Context::Context(const std::string & basename) : basename(basename), SCRIPT_DEFINED(false), id(++counter), name(drain::StringBuilder<>(basename, '-', id)){
	init();
}

Context::Context(const Context & ctx) : basename(ctx.basename), SCRIPT_DEFINED(false), id(++counter), name(drain::StringBuilder<>(ctx.name, '.', id)){  //  ++counter + 100*ctx.id
	log.setVerbosity(ctx.log.getVerbosity());
	init();
}

void Context::init(){
	statusMap["ID"] = id;        // "constant"
	statusMap["PID"] = getpid(); // "constant"
	statusMap["CTX"] = name;     // "constant"
	statusMap["statusFlags.dictionary"] = drain::sprinter(statusFlags.getDict(), drain::Sprinter::cppLayout).str(); // WHY?
}

void Context::updateStatus() const {
	// void updateStatus(bool update=true) const {
	// if (update){
	statusMap["statusFlags"] = statusFlags.value;
	statusMap["statusKeys"] = statusFlags.str();  //getKeys()
	//}
};


}  // drain::
