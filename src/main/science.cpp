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
//#include <stddef.h>

#include <drain/prog/CommandInstaller.h>
#include <string>

#include "drain/prog/CommandBank.h"
#include "radar/Precipitation.h"
#include "product/RainRateOp.h"

#include "science.h"

namespace rack {




/*
template <class C=drain::Command>
class ScienceCmdWrapper : public drain::CommandWrapper<C,0, ScienceSection> {
public:

	ScienceCmdWrapper(char alias = 0): drain::CommandWrapper<C,0,ScienceSection>(alias) {};

	//ImageCmdWrapper(const std::string & name, char alias = 0): drain::CommandWrapper<C,0,RACK_SEC_IMG>(name, alias) {};

};
*/

struct ScienceSection : public drain::CommandSection {

	inline	ScienceSection(): CommandSection("science"){
		//hello(__FUNCTION__);
		// drain::CommandBank::trimWords().insert("Op");
	};

};

ScienceModule::ScienceModule(){ // : CommandSection("science"){

	//ScienceCmdWrapper<>::setSectionTitle("science");
	drain::Flagger::value_t section = drain::Static::get<ScienceSection>().index;

	//const ScienceModule & mod = drain::Static::get<ScienceModule>();

	drain::CommandBank & cmdBank = drain::getCommandBank();

	drain::BeanRefCommand<PrecipZrain> precipZrain(RainRateOp::precipZrain);
	cmdBank.addExternal(precipZrain).section = section;

	drain::BeanRefCommand<PrecipZsnow> precipZsnow(RainRateOp::precipZsnow);
	cmdBank.addExternal(precipZsnow).section = section;

	drain::BeanRefCommand<PrecipKDP> precipKDP(RainRateOp::precipKDP);
	cmdBank.addExternal(precipKDP).section = section;

	drain::BeanRefCommand<PrecipZZDR> precipZZDR(RainRateOp::precipZZDR);
	cmdBank.addExternal(precipZZDR).section = section;

	drain::BeanRefCommand<PrecipKDPZDR> precipKDPZDR(RainRateOp::precipKDPZDR);
	cmdBank.addExternal(precipKDPZDR).section = section;

	drain::BeanRefCommand<FreezingLevel> freezingLevel(RainRateOp::freezingLevel);
	cmdBank.addExternal(freezingLevel).section = section;

};


} // namespace rack

