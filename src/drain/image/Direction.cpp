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


#include "Direction.h"

namespace drain
{
namespace image
{


const Direction::value_t Direction::NONE;
const Direction::value_t Direction::UP;
const Direction::value_t Direction::UP_RIGHT;
const Direction::value_t Direction::RIGHT;
const Direction::value_t Direction::DOWN_RIGHT;
const Direction::value_t Direction::DOWN;
const Direction::value_t Direction::DOWN_LEFT;
const Direction::value_t Direction::LEFT;
const Direction::value_t Direction::UP_LEFT;

const std::map<Direction::value_t,Position> Direction::offset = {
		{NONE,       { 0, 0}},
		{UP,         { 0,-1}},
		{UP_RIGHT,   {+1,-1}},
		{RIGHT,      {+1, 0}},
		{DOWN_RIGHT, {+1,+1}},
		{DOWN,       { 0,+1}},
		{DOWN_LEFT,  {-1,+1}},
		{LEFT,       {-1, 0}},
		{UP_LEFT,    {-1,-1}}
};


const Direction::dict_t Direction::arrows = {
		{NONE,       {"←"}},
		{UP,         {"↑"}},
		{UP_RIGHT,   {"↗"}},
		{RIGHT,      {"→"}},
		{DOWN_RIGHT, {"↘"}},
		{DOWN,       {"↓"}},
		{DOWN_LEFT,  {"↓"}},
		{LEFT,       {"←"}},
		{UP_LEFT,    {"↖"}}
};

} // image::

} // drain::

