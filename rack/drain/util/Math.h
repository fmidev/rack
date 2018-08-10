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
/*
 * Math.h
 *
 *  Created on: Jul 23, 2013
 *      Author: mpeura
 */

#ifndef MATH_H_
#define MATH_H_

#include <vector>
#include <iostream>
#include <stdexcept>

// // using namespace std;

namespace drain {

// MatrixBase
template <class T=double>
class Matrix : protected std::vector<T> {

public:

	Matrix(size_t rows=0, size_t columns = 0) : std::vector<T>(rows*columns), rows(rows), columns(columns) {
	}

	Matrix(const Matrix &m) : std::vector<T>(m), rows(m.rows), columns(m.columns) {
	}

	~Matrix(){};

	virtual
	void setSize(size_t rows, size_t columns){
		this->rows = rows;
		this->columns = columns;
		std::vector<T>::resize(rows * columns);
	}

	inline
	size_t getSize() const { return std::vector<T>::size(); }

	inline
	virtual
	size_t getRows() const {
		return rows;
	}

	inline
	virtual
	size_t getColumns() const {
		return columns;
	}

	inline
	T & operator()(const size_t & i){
		return (*this)[i];
	};

	inline
	const T & operator()(const size_t & i) const {
		return (*this)[i];
	};

	inline
	T & operator()(const size_t & i, const size_t & j){
		//return (*this)[i*rows + j];
		return (*this)[i*columns + j];
	};

	inline
	const T & operator()(const size_t & i, const size_t & j) const {
		//return (*this)[i*rows + j];
		return (*this)[i*columns + j];
	};


	/// Resets the values, leaving geomatry.
	inline
	void fill(const T & x){
		for (typename std::vector<T>::iterator it=this->begin(); it != this->end(); ++it){
			*it = 0;
		}
	}

	inline
	typename std::vector<T>::iterator begin() { return std::vector<T>::begin(); }

	inline
	typename std::vector<T>::iterator end() { return std::vector<T>::end(); }

	inline
	typename std::vector<T>::const_iterator begin() const { return std::vector<T>::begin(); }

	inline
	typename std::vector<T>::const_iterator end() const { return std::vector<T>::end(); }

	template <class T2>
	inline
	Matrix<T> & operator=(const std::vector<T2> &v){
		setSize(v.size(),1);
		std::vector<T>::operator=(v);
		return *this;
	}

	operator std::vector<T> &() const {
		return *this;
	}

	void transpose(Matrix<T> & dst);

	template <class T2, class T3>
	void multiply(const Matrix<T2> & src2, Matrix<T3> & dst);

	/// Computes the inverse of a matrix.
	/**
	 *   Currently, only 2x2 and 3x3 matrices are supported.
	 */
	template <class T2>
	void inverse(Matrix<T2> & dst);

	/// Computes the inverse of a 3-by-3 matrix.
	template <class T2>
	void inverse3(Matrix<T2> & dst);

private:

	size_t rows;
	size_t columns;

};

/*
template <class T=double>
class Matrix : public MatrixBase<T> {

public:
	Matrix(size_t rows=0, size_t columns = 0) : MatrixBase<T>(rows, columns) {
	}

};
*/


template <class T>
void Matrix<T>::transpose(Matrix<T> & dst){

	Matrix<T> tmp;
	const bool SELF = (&dst == this);

	const Matrix<T> & s = SELF ? tmp : *this;
	if (SELF)
		tmp = *this;

	dst.setSize(getColumns(), getRows());
	for (size_t i = 0; i < getRows(); ++i) {
		for (size_t j = 0; j < getColumns(); ++j) {
			dst(j,i) = s(i,j);
			//rand();
		}
	}
}



template <class T>
template <class T2, class T3>
void Matrix<T>::multiply(const Matrix<T2> & src, Matrix<T3> & dst){

	if (getColumns() != src.getRows()){
		std::cerr << "a=" << *this << std::endl;
		std::cerr << getColumns() << std::endl;
		std::cerr << "b=" << src << std::endl;
		std::cerr << getRows() << std::endl;
		throw std::runtime_error("multiply: w1 != h2");
	}

	dst.setSize(getRows(), src.getColumns());

	/*
	std::cout << " multiply\n";
	std::cout << "C=" << dst << std::endl;
	*/

	T3 x;
	for (size_t i = 0; i < getRows(); ++i) {
		for (size_t j = 0; j < src.getColumns(); ++j) {
			//T3 & x = dst(i,j);
			x = 0.0;
			for (size_t k = 0; k < src.getRows(); ++k) {
				x += (*this)(i,k)*src(k,j);
			}
			dst(i,j) = x;
		}
	}

}

template <class T>
template <class T2>
void Matrix<T>::inverse(Matrix<T2> & dst){

	switch (getRows()) {
	case 3:
		inverse3(dst);
		break;
	default:
		std::cerr << getRows() << std::endl;
		throw std::runtime_error("inverse: not implemented for n>3");
		break;
	}
}

/// Computes the inverse of a 3-by-3 matrix.
template <class T>
template <class T2>
void Matrix<T>::inverse3(Matrix<T2> & dst){

	if ((getRows() != 3) || (getColumns() != 3))
		throw std::runtime_error("inverse: matrix not 3x3");

	dst.setSize(3,3);
	//std::cout << dst << std::endl;

	/// Abbreviations of this
	/*   a d g
	 *   b e h
	 *   c f i
	 */
	const T & a = (*this)(0,0);
	const T & b = (*this)(1,0);
	const T & c = (*this)(2,0);

	const T & d = (*this)(0,1);
	const T & e = (*this)(1,1);
	const T & f = (*this)(2,1);

	const T & g = (*this)(0,2);
	const T & h = (*this)(1,2);
	const T & i = (*this)(2,2);

	// Determinant
	const double det = a*(e*i-f*h) - d*(b*i-c*h) + g*(b*f-c*e);
	if (det == 0.0)
		throw std::runtime_error("inverse: zero determinant");

	// Inverse of determinant
	const double detInv = 1.0/det;

    /// Transpose of this matrix
	const T & A = (*this)(0,0);
	const T & B = (*this)(0,1);
	const T & C = (*this)(0,2);

	const T & D = (*this)(1,0);
	const T & E = (*this)(1,1);
	const T & F = (*this)(1,2);

	const T & G = (*this)(2,0);
	const T & H = (*this)(2,1);
	const T & I = (*this)(2,2);

	/*   A D G
	 *   B E H
	 *   C F I
	 */
	dst(0,0) = detInv * +(E*I - F*H);
	dst(0,1) = detInv * -(B*I - C*H);
	dst(0,2) = detInv * +(B*F - C*E);

	dst(1,0) = detInv * -(D*I - F*G);
	dst(1,1) = detInv * +(A*I - C*G);
	dst(1,2) = detInv * -(A*F - C*D);

	dst(2,0) = detInv * +(D*H - E*G);
	dst(2,1) = detInv * -(A*H - B*G);
	dst(2,2) = detInv * +(A*E - B*D);


}


template <class T=double>
class Vector : public Matrix<T> {

public:

	Vector(size_t size=0, bool vertical=true) : Matrix<T>(vertical?size:1, vertical?1:size), vertical(vertical) {
	}

	Vector(const Vector<T> &v) : Matrix<T>(v.getRows(), v.getColumns()), vertical(v.vertical) {
	}
	/*
	void setSize(size_t rows, size_t columns){
		if ((rows > 1) && (columns > 1)){
			throw std::runtime_error("Vector::setSize: rows or colums != 1");
		}
		Matrix<T>::setSize(rows, columns);
		vertical = (rows >= columns);
	}*/

	template <class T2>
	inline
	Vector<T> & operator=(const std::vector<T2> &v){
		// Vector<T>::
		// Matrix<T>::
		Matrix<T>::setSize(v.size(), size_t(1));
		std::vector<T>::operator=(v);
		return *this;
	}

	template <class T2>
	T innerProduct(Vector<T2> & src){
		if (src.size() != std::vector<T>::size()){
			std::cerr << (*this) << std::vector<T>::size() << std::endl;
			std::cerr << src << src.size() << std::endl;
			throw std::runtime_error("Vector::innerProduct: different sizes");
		}
		T result = 0;
		//std::cout << "innerBegin" << flush;
		typename std::vector<T>::const_iterator  it  = std::vector<T>::begin();
		typename std::vector<T2>::const_iterator it2 = src.begin();
		while (it != std::vector<T>::end()){
			result += *it * *it2;
			++it;
			++it2;
		}
		//std::cout << "innerEnd" << std::endl;
		return result;
	}

protected:
	bool vertical;

};


/// Fits a 2nd order polynomial to a curve.
/**
 *  If matrix inversion fails (probablt due to undetectValue determinant), the std::runtime_error thrown by inverse() is passed through.
 */
//void fitCurve(const std::vector<double> &src, const std::vector<double> &weight, drain::Vector<double> &coeff) const;
template <class T>
void polynomialFit2nd(const T &src, const T &weight, std::vector<double> &coeff){

	coeff.resize(3);

	//const size_t n = src.size();

	double x = 0.0;
	drain::Matrix<double> XTX(3,3);
	drain::Matrix<double> XTXinv(3,3);
	double sumX0, sumX1, sumX2, sumX3, sumX4;
	sumX0 = sumX1 = sumX2 = sumX3 = sumX4 = 0.0;


	double y;
	drain::Matrix<double> XTy(3,1);
	double & sumY   = XTy(0);
	double & sumYX  = XTy(1);
	double & sumYXX = XTy(2);
	sumY = sumYX = sumYXX = 0.0;

	double w;

	//for (size_t i = 0; i < n; ++i) {
	typename T::const_iterator its = src.begin();
	typename T::const_iterator itw = weight.begin();

	while (its != src.end()){

		x += 1.0;
		y = *its;
		w = *itw;

		sumX0 += w* 1.0;
		sumX1 += w* x;
		sumX2 += w* x*x;
		sumX3 += w* x*x*x;
		sumX4 += w* x*x*x*x; // (optimise)

		sumY   += w* y;
		sumYX  += w* y*x;
		sumYXX += w* y*x*x;

		++its;
		++itw;
	}

	XTX(0,0) = static_cast<double>(sumX0);
	XTX(0,1) = XTX(1,0) = sumX1;
	XTX(0,2) = XTX(1,1) = XTX(2,0) = sumX2;
	XTX(1,2) = XTX(2,1) = sumX3;
	XTX(2,2) = sumX4;

	XTX.inverse(XTXinv);  // Throws a runtime error, if fails.

	drain::Vector<double> coeffVector(3);
	XTXinv.multiply( XTy, coeffVector);
	coeff = (const std::vector<double> &)coeffVector;


}



template <class T>
std::ostream & operator<<(std::ostream &ostr, const Matrix<T> & m){
	ostr << '[';
	for (size_t i = 0; i < m.getRows(); ++i) {
		for (size_t j = 0; j < m.getColumns(); ++j) {
			ostr << ' ' <<  m(i,j) ;
		}
		ostr << ';' << '\n';
	}
	ostr << ']';
	ostr << " (" << m.getSize() << " elements)";
	ostr << '\n';
	return ostr;
}


} /* namespace drain */
#endif /* MATH_H_ */

// Rack
