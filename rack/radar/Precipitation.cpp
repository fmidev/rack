/*

    Copyright 2015 - 2015  Johannes Karjalainen and Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "Precipitation.h"


namespace rack {


const std::string & Precipitation::getDescription() const {

	if (descriptionExt.empty()){
		std::stringstream s;
		s << description;
		if (!presets.empty()){
			s << "\n presets:";
			for (std::map<std::string,std::string>::const_iterator it = presets.begin(); it != presets.end(); ++it)
				s << "\n\t" << it->first << " => " << it->second;  // no trailing newline
		}
		descriptionExt = s.str();
	}
	return descriptionExt;
}

/*
PrecipitationZ precipZrain("precipZrain");
PrecipitationZ precipZsnow("precipZsnow");
PrecipitationKDP precipKDP;
PrecipitationKDPZDR precipKDPZDR;
PrecipitationZZDR precipZZDR;
*/

}  // namespace rack




