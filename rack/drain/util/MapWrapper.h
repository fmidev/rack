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
#ifndef MAPWRAPPER_H_
#define MAPWRAPPER_H_

// using namespace std;

#include <map>

namespace drain
{

  template <class K, class V>
  class MapWrapper 
  {
    public:
	  
	  MapWrapper() : m(ownMap) {};
	  
	  MapWrapper(map<K,V> &src): m(&src) {};
	  
	  virtual ~MapWrapper(){};
	  
	  void setMap(map<K,V> &src){
	  	m = &src;
	  }
	  
	  const Data &get(const K &key) const {
	  	typename map<K,V>::iterator iter = this->m->find(key);
	  	if (iter != m.end()){
	  		return Data();
	  	}
	  	else {
	  		return Data(iter->second);
	  	}
	  };
	  
	  template<class T>
	  T get(const K &key, const T &defaultValue) const {
	  	typename map<K,V>::iterator iter = this->m->find(key);
	  	if (iter == m->end()){
	  		return defaultValue;
	  		//return defaultValue;
	  	}
	  	else {
	  		//*iter;
	  		return Data(iter->second);
	  	}
	  };  
	  
	 protected:
	  	map<K,V> *m;
	  	map<K,V> ownMap; 
  };
}

#endif /*MAPWRAPPER_H_*/
