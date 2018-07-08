/*

    Copyright 2001 - 2015  Johannes Karjalainen & Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include <drain/util/BeanLike.h>
#include <drain/util/RegExp.h>

#ifndef PRECIPITATION_H_
#define PRECIPITATION_H_

namespace rack{

class Precipitation : public drain::BeanLike  {

public:


	Precipitation(const std::string & name = "", const std::string & description = "") : drain::BeanLike(name, description) { // , parameters(true, ':')
		this->parameters.separator = ',';
	}


	virtual
	inline
	~Precipitation(){};

	/// Redefined such that if argument is a preset, reinvoke with its arguments.
	virtual
	inline
	void setParameters(const std::string &p, char assignmentSymbol='=', char separatorSymbol=0){

		drain::Logger mout(name, __FUNCTION__);

		static drain::RegExp presetKey("^[a-zA-Z]+");
		if (presetKey.test(p)){
			std::map<std::string,std::string>::const_iterator it = presets.find(p);
			if (it != presets.end()){
				mout.info() << "applying presets '" << "'" << mout.endl;
				//setParameters(it->second);
				BeanLike::setParameters(it->second, assignmentSymbol, separatorSymbol);
			}
			else {
				mout.warn() << "no preset found for: '" << p << "'" << mout.endl;
			}
		}
		else {
			BeanLike::setParameters(p, assignmentSymbol, separatorSymbol);
		}

		initParameters();
	};

	virtual
	const std::string & getDescription() const;

protected:

	virtual
	inline
	void initParameters(){};

	virtual
	void setParameterReferences() = 0;

	std::map<std::string,std::string> presets;

private:

	mutable
	std::string descriptionExt;

};


class PrecipitationZ : public Precipitation {
    public: //re 
	inline
    PrecipitationZ(double a=200.0, double b=1.6) :
    	Precipitation(__FUNCTION__, "Precipitation rate from Z (reflectivity)"), a(a), b(b){
		setParameterReferences();
	};

	inline
	PrecipitationZ(const PrecipitationZ & p){
		setParameterReferences();
		BeanLike::copy(p);
	}
	//inline 	~PrecipitationZ(){};

	virtual
	inline
	void initParameters(){
		aInv = 1.0/a;
		bInv = 1.0/b;
	};


	inline
	double rainRate(double dbz) const {
		return pow(aInv*pow10(dbz*0.10), bInv);// Eq.(1)
	}


	double a;
	double b;


protected:

	void setParameterReferences(){
		parameters.reference("a", this->a = a);
		parameters.reference("b", this->b = b);
		presets["Marshall-Palmer"] = "200,1.6";
		initParameters();  // todo lower
	}


	double aInv;
	double bInv;

};

class PrecipitationKDP : public Precipitation {
    public: //re 
//public:

	inline
    PrecipitationKDP(double a=21.0, double b=0.72) : Precipitation(__FUNCTION__, "Precipitation rate from KDP"), a(a), b(b) {
		setParameterReferences();
	};

	PrecipitationKDP(const PrecipitationKDP & p){ // copy name?
		setParameterReferences();
		copy(p);
	};


	inline
	double rainRate(double kdp) const {
		double r = -1.0;
		r = a * pow(kdp, b); // Eq.(3)
		return r;
	}

	double a;
	double b;

protected:

	void setParameterReferences(){
		parameters.reference("a", a);
		parameters.reference("b", b);
		presets["Leinonen2012"] = "21,0.72";
	}

};

class PrecipitationZZDR : public Precipitation {
    public: //re 

	PrecipitationZZDR(double a=0.0122, double b=0.820, double c=-2.28) :
		Precipitation(__FUNCTION__, "Precipitation rate from Z and ZDR"), a(a), b(b), c(c) {
		setParameterReferences();
	};

	inline
    PrecipitationZZDR(const PrecipitationZZDR & p) {
		setParameterReferences();
		copy(p);
	};

	//inline virtual 	~PrecipitationZZDR(){};

	inline
	double rainRate(double dbz, double zdr) const {
		double r = -1.0;
		r = a * pow(dbz, b) * pow(zdr, c);// Eq.(2)
		return r;
	}

	double a;
	double b;
	double c;

protected:

	void setParameterReferences(){
		parameters.reference("a", this->a);
		parameters.reference("b", this->b);
		parameters.reference("c", this->c);
		//presets[""]
	}


};

class PrecipitationKDPZDR : public Precipitation {
    public: //re 
	inline
	PrecipitationKDPZDR(double a=29.7, double b=0.890, double c=-0.927) :
	Precipitation(__FUNCTION__, "Precipitation rate from KDP and ZDR"), a(a), b(b), c(c) {
		setParameterReferences();
	};

	inline
    PrecipitationKDPZDR(const PrecipitationKDPZDR & p) {
		setParameterReferences();
		copy(p);
	};

	// inline virtual 	~PrecipitationKDPZDR(){};
	inline
	double rainRate(double kdp, double zdr) const {
		double r = -1.0;
		r = a * pow(kdp, b) * pow(zdr, c); //Eq. (4)
		return r;
	}

	double a;
	double b;
	double c;

protected:

	void setParameterReferences(){
		parameters.reference("a", a);
		parameters.reference("b", b);
		parameters.reference("c", c);
		//presets[""]
	}
};

}



#endif /* PRECIPITATION_H_ */
