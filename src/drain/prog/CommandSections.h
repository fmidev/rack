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

#ifndef DRAIN_CMD_SECTIONS_H_
#define DRAIN_CMD_SECTIONS_H_

#include <drain/Log.h>
#include <map>
#include <set>
//#include "drain/util/Debug.h"
//#include "drain/util/ReferenceMap.h"

#include "drain/util/ReferenceMap.h"

//#include "Command.h"
//#include "CommandRegistry.h"

#include "CommandBank.h"


namespace drain {




class CommandSection {


public:

	CommandSection(const std::string & title) : title(title), index(drain::getCommandBank().addSection(title)){
		Logger mout(__FILE__, __FUNCTION__);
		//std::cerr << __FUNCTION__ << ':' << title << '(' << index << ')' << std::endl;
		mout.debug() << __FUNCTION__ << ':' << title << '(' << index << ')' << mout.endl;
	}

	// Consider trying static?

	const std::string title;

	const drain::Flagger::value_t index;

	operator const std::string & () const {
		return title;
	}

	operator drain::Flagger::value_t () const {
		return index;
	}


	/*
	void hello(const std::string & title){
		std::cerr << "installing " << title << '(' << index << ')' << ' ' << '"'<< title << '"' << std::endl;
	}
	*/


protected:

	/**
	 *   This can be used for hidden Sections containing commands for debugging etc.
	 *
	 */
	CommandSection(const std::string & title, drain::Flagger::value_t index) : title(title), index(index){
		if (index > 0)
			drain::getCommandBank().sections.add(title, index);
	}


};

struct HiddenSection : public CommandSection {

	inline
	HiddenSection(): CommandSection("hidden", 0){
	};

};


struct GeneralSection : public CommandSection {

	inline
	GeneralSection(): CommandSection("general"){
	};

};


struct TriggerSection : public CommandSection {

	inline
	TriggerSection(): CommandSection("trigger"){
	};

};






}


/* namespace drain */

#endif /* DRAINLET_H_ */

// Rack
