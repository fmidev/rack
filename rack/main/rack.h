/**


    Copyright 2006 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

   Created on: Sep 30, 2010
*/

#ifndef __RACK_API__
#define __RACK_API__

#define __RACK__ "Rack fmi.fi"
#define __RACK_VERSION__ "4.3"

/*
 *  Todo:
 *  - clear proj def in accumula
 */

namespace rack {

/*!  Main program - renamed to facilitate calls from outside, esp. JNI calls.
 *   The standard main() will call this directly.
 */
int process(int argc, const char **argv);

} // rack::



#endif
