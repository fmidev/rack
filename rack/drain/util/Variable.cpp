/**

    Copyright 2012-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include "Variable.h"

// // // using namespace std;


namespace drain
{


/*
void Variable::appendToString(const std::string &s) {

	if (isStlString()){
		std::string & d = (std::string *)ptr;
		if (!d.empty() && inputSeparator)
			d.append(inputSeparator);  // really?
		d.append(s);
	}
	else if (isCharArrayString()){

		if (isEmpty()){
			requestSize(1);
			if (isEmpty()){
				throw std::runtime_error("Variable: appendToString: still empty after resize");
			}
			else {
				*getPtr() = '\0';
			}
		}

		const char *c = getCharArray(); // checks null char
		//
		size_t l = strlen(c);

		// Note: current elementCount irrelevant, using strlen!
		// Note: terminal null char must
		if (inputSeparator)
			requestSize(l+1 + 1 + s.length());
		else
			requestSize(l+1 +     s.length());

		/// Store
		std::string::const_iterator sit = s.begin();
		std::vector<char>::iterator  it = _data.begin();

		//while (it != _data.end()){
		while (sit != s.end()){
			//std::cout << (int)*sit << ',';
			*it = *sit;
			++it;
			++sit;
		}
		*it = '\0';  // see resize() above

	}
	std::stringstream sstr;
	toStream(sstr);
	if ((this->inputSeparator != '\0') && (_data.size()>1))
		sstr << this->inputSeparator;
	sstr << x;
	assignToCharArray(sstr.str());

	//std::cerr << "Variable appendString:" << toStr() << " => '"<< s << "'[" << s.length() << ']' << (int)s.at(0) << '\n';
	if (inputSeparator)
		resize(getElementCount() +1 + s.length() + 1);
	else
		resize(getElementCount() + s.length() + 1);

	/// Store
	std::string::const_iterator sit = s.begin();
	std::vector<char>::iterator  it = _data.begin();
	//while (it != _data.end()){
	while (sit != s.end()){
		//std::cout << (int)*sit << ',';
		*it = *sit;
		++it;
		++sit;
	}
	*it = '\0';  // see resize() above

}
*/
//const Variable VariableMap::_empty;

/*
template <>
void Variable::toVector(std::vector<Variable> & v) const {

	//std::cerr << "toVector" << typeid(vField).name() << std::endl;
		if (getType() == typeid(std::string)){
			std::list<std::string> l;
			StringTools::split(toStr(), l, std::string(1,this->separator)," \t\n");
			v.resize(l.size());
			//typename
			std::list<std::string>::const_iterator lit = l.begin();
			std::vector<Variable>::iterator vit = v.begin();
			while (lit != l.end()) {
				*vit = *lit;
				++vit;
				++lit;
			}
		}
		else {
			v.resize(getElementCount());
			CastableIterator it = begin();
			std::vector<Variable>::iterator vit = v.begin();
			while (it != end()) {
				//std::cerr << *vit << "<=" << *it << std::endl;
				*vit = *it;
				++it;
				++vit;
			}
		}
}
*/


} // drain
