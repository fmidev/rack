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
#include "palette-manager.h"


namespace rack {

	drain::image::PaletteOp::palette_map_t & PaletteManager::getMap(){

		drain::image::PaletteOp::palette_map_t & m = drain::image::PaletteOp::getPaletteMap();

		if (m.empty()){ // Todo: what if future PaletteOp map is non-empty (factory palettes)


			//drain::Logger mout(drain::image::getImgLog(), __FILE__, __FUNCTION__);

			//drain::image::getImgLog().setVerbosity(20);

			/*
			m["ANDRE-CLASS"] =
			#include "palette/palette-ANDRE-CLASS.inc"
			;
			*/


			m["ALT"] =
			#include "palette/palette-ALT.inc"
			;

			m["CLASS"] =
			#include "palette/palette-CLASS.inc"
			;
			// drain::Sprinter::sequenceToStream(std::cout, m["CLASS"], "\n", drain::Sprinter::cppLayout);
			// std::cout << "-------------------------" << '\n';
			// std::cout << m["CLASS"] << '\n';

			m["CLASS-ETOP"] =
			#include "palette/palette-CLASS-ETOP.inc"
			;

			m["COUNT"] =
			#include "palette/palette-COUNT.inc"
			;

			m["DBZHSTDDEV"] =
			#include "palette/palette-DBZHSTDDEV.inc"
			;

			m["DBZH"] =
			#include "palette/palette-DBZH.inc"
			;
			/*
			m.addAlias("DBZ", "DBZH");
			m.addAlias("DBZ", "DBZV");
			m.addAlias("DBZ", "TH");
			m.addAlias("DBZ", "TV");
			m.addAlias("DBZ", "T"); // ?
			*/

			m["DBZ-SLOPE"] =
			#include "palette/palette-DBZ-SLOPE.inc"
			;

			m["FLASH"] =
			#include "palette/palette-FLASH.inc"
			;

			m["HCLASS"] =
			#include "palette/palette-HCLASS.inc"
			;

			m["HGHT"] =
			#include "palette/palette-HGHT.inc"
			;

			m["PROB"] =
			#include "palette/palette-PROB.inc"
			;

			m["QIND"] =
			#include "palette/palette-QIND.inc"
			;

			// Black and white
			m["QIND-BW"] =
			#include "palette/palette-QIND-BW.inc"
			;

			m["RATE"] =
			#include "palette/palette-RATE.inc"
			;

			m["RHOHV"] =
			#include "palette/palette-RHOHV.inc"
			;

			m["RISK"] =
			#include "palette/palette-RISK.inc"
			;

			m["SUNSHINE"] =
			#include "palette/palette-SUNSHINE.inc"
			;

			m["VRAD-DEV"] =
			#include "palette/palette-VRAD-DEV.inc"
			;

			m["VRAD"] =
			#include "palette/palette-VRAD.inc"
			;
			//m.addAlias("VRAD", "VRADH");
			//m.addAlias("VRAD", "VRADV");

			m["ZDR"] =
			#include "palette/palette-ZDR.inc"
			;

			// drain::Sprinter::toStream(std::cout, m["CLASS"], drain::Sprinter::jsonLayout);
		}

		return m;
	}

	drain::image::Palette & PaletteManager::getPalette(const std::string & key){
		return getMap()[key];
	}

	/*
		{
			 	{"title", {"Radar reflectance"} },
				{"nodata", {"nodata", "102,nodata", {255.0,255.0,255.0}} },
				{"undetect", {"undetect", "101,undetect", {0.0,0.0,0.0}} },
				{     -32.0, {"noise", "103,noise", {0.0,0.0,0.0}} },
				{     -24.0, {"", "104,", {60.0,140.0,200.0}} },
				{     -16.0, {"", "105,", {10.0,155.0,225.0}} },
				{      -8.0, {"insects", "106,insects", {5.0,205.0,170.0}} },
				{       0.0, {"drizzle", "107,drizzle", {140.0,230.0,20.0}} },
				{       8.0, {"weak", "108,weak", {240.0,240.0,20.0}} },
				{      16.0, {"rain", "109,rain", {255.0,205.0,20.0}} },
			 	{      24.0, {"moderate", "110,moderate", {255.0,150.0,50.0}} },
				{      32.0, {"intensive", "111,intensive", {255.0,80.0,60.0}} },
				{      40.0, {"hail", "112,hail", {250.0,120.0,255.0}} },
				{      60.0, {"", "113,", {255.0,255.0,255.0}} }
		}
		;
	*/

} /* namespace rack */

