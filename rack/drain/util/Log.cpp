/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

*/
/*
 * Debug.h
 *
 *  Created on: Aug 20, 2010
 *      Author: mpeura
 */

#include "Log.h"

namespace drain {

//Log monitor;


Log & getLog(){

	static Log log;
	return log;

}

namespace image {

Log & getImgLog(){

	static Log log;
	return log;

}

}



Logger::oper Logger::endl;

Logger & Logger::timestamp(const std::string & label){
	init(LOG_INFO);
	*(this) << "TIME#" << label << Logger::endl;
	return *this;
}

Logger & Logger::timestamp(){
	init(LOG_INFO);
	*(this) << "TIME#";
	return *this;
}

}
