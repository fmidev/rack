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

#include "drain/util/BeanLike.h"
#include "drain/util/RegExp.h"

#ifndef PRECIPITATION_H_
#define PRECIPITATION_H_

namespace rack{

class Precip : public drain::BeanLike  {

public:


	Precip(const std::string & name = "", const std::string & description = "") : drain::BeanLike(name, description) { // , parameters(true, ':')
		this->parameters.separator = ',';
	}


	virtual
	inline
	~Precip(){};

	/// Redefined such that if argument is a preset, reinvoke with its arguments.
	/*
	virtual
	inline
	void setParameters(const std::string &p, char assignmentSymbol='=', char separatorSymbol=0){

		drain::Logger mout(__FILE__, __FUNCTION__);

		static const drain::RegExp presetKey("^[a-zA-Z]+");

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
	*/


	// virtual
	// const std::string & getDescription() const;

protected:

	//virtual inline
	//void initParameters(){};

	// virtual void setParameterReferences() = 0;

	std::map<std::string,std::string> presets;

private:

	//mutable
	//std::string descriptionExt;

};

class SingleParamPrecip : public Precip {

public:

	virtual
	double rainRate(double p) const = 0;

protected:

	inline
	SingleParamPrecip(const std::string & name = "", const std::string & description = ""):
		Precip(name,description) {

	}


};

class DoubleParamPrecip : public Precip {

public:

	virtual
	double rainRate(double p1, double p2) const = 0;

protected:

	inline
	DoubleParamPrecip(const std::string & name = "", const std::string & description = ""):
		Precip(name,description) {
	}

};




class PrecipZ : public SingleParamPrecip {

public:

	inline
    PrecipZ(const std::string  & name=__FUNCTION__, double a=200.0, double b=1.6) :
    	SingleParamPrecip(name, "Precipitation rate from Z (reflectivity)"), a(a), b(b){
		parameters.link("a", this->a = a);
		parameters.link("b", this->b = b);
		updateBean();
	};

	inline
	PrecipZ(const PrecipZ & p) : SingleParamPrecip(p) {
		parameters.copyStruct(p.getParameters(), p, *this);
		updateBean();
	}

	virtual
	inline
	void updateBean() const {
		aInv = 1.0/a;
		bInv = 1.0/b;
		//drain::Logger mout(__FILE__, __FUNCTION__);
		//mout.warn("coeff:", aInv, bInv);
	};


	inline
	double rainRate(double dbz) const {
		return pow(aInv * pow(10.0, dbz*0.10), bInv);// Eq.(1)
	}


	double a;
	double b;


protected:


	mutable double aInv;
	mutable double bInv;

};

class PrecipZrain : public PrecipZ {

public:

	PrecipZrain(double a=200.0, double b=1.6) : PrecipZ(__FUNCTION__, a ,b){
		presets["Marshall-Palmer"] = "200,1.6";
	}
};

class PrecipZsnow : public PrecipZ {

public:

	PrecipZsnow(	double a=222.2, double b=1.1111) : PrecipZ(__FUNCTION__, a ,b){
		//presets["Marshall-Palmer"] = "200,1.6";
	}

};


class PrecipKDP : public SingleParamPrecip {

public:

	inline
    PrecipKDP(double a=21.0, double b=0.72) : SingleParamPrecip(__FUNCTION__, "Precip rate from KDP"), a(a), b(b) {
		//setParameterReferences();
		parameters.link("a", a);
		parameters.link("b", b);
		presets["Leinonen2012"] = "21,0.72";
	};

	PrecipKDP(const PrecipKDP & p) : SingleParamPrecip(p) { // copy name?
		parameters.copyStruct(p.getParameters(), p, *this);
		//setParameterReferences();
		//copy(p);
	};


	inline
	double rainRate(double kdp) const {
		double r = -1.0;
		r = a * ::pow(kdp, b); // Eq.(3)
		return r;
	}

	double a;
	double b;


};

class PrecipZZDR : public DoubleParamPrecip {

public:

	PrecipZZDR(double a=0.0122, double b=0.820, double c=-2.28) :
		DoubleParamPrecip(__FUNCTION__, "Precipitation rate from Z and ZDR"), a(a), b(b), c(c) {
		parameters.link("a", this->a);
		parameters.link("b", this->b);
		parameters.link("c", this->c);
		// setParameterReferences();
	};

	inline
    PrecipZZDR(const PrecipZZDR & p) : DoubleParamPrecip(p) {
		parameters.copyStruct(p.getParameters(), p, *this);
		//setParameterReferences();
		// copy(p);
	};

	//inline virtual 	~PrecipZZDR(){};

	inline
	double rainRate(double dbz, double zdr) const {
		double r = -1.0;
		r = a * ::pow(dbz, b) * pow(zdr, c);// Eq.(2)
		return r;
	}

	double a;
	double b;
	double c;


};

class PrecipKDPZDR : public DoubleParamPrecip {

public:

	inline
	PrecipKDPZDR(double a=29.7, double b=0.890, double c=-0.927) :
	DoubleParamPrecip(__FUNCTION__, "Precipitation rate from KDP and ZDR"), a(a), b(b), c(c) {
		parameters.link("a", a);
		parameters.link("b", b);
		parameters.link("c", c);
	};

	inline
    PrecipKDPZDR(const PrecipKDPZDR & p) : DoubleParamPrecip(p) {
		parameters.copyStruct(p.getParameters(), p, *this);
	};

	// inline virtual 	~PrecipKDPZDR(){};
	inline
	double rainRate(double kdp, double zdr) const {
		double r = -1.0;
		r = a * ::pow(kdp, b) * ::pow(zdr, c); //Eq. (4)
		return r;
	}

	double a;
	double b;
	double c;

};

}  // rack




#endif /* PRECIPITATION_H_ */
