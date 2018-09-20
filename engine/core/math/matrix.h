#pragma once

#include <vector>
#include <iostream>
#include "Vector2.h"

namespace Echo
{
	class Matrix
	{
	public:
		Matrix();
		Matrix(int height, int width);

		// add row
		void addRow(const RealVector& row);

		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }
		int getNumberElements() const { return m_width * m_height; }

		Matrix multiply(const Matrix& m) const;
		Matrix add(const Matrix& m) const;

		// operator []
		RealVector& operator[] (int idx) { return m_array[idx]; }
		const RealVector& operator[] (int idx) const { return m_array[idx]; }

		// apply function
		Matrix applyFunction(Real(*function)(Real)) const;

		// reset
		void reset();

	private:
		int						 m_height;
		int						 m_width;
		vector<RealVector>::type m_array;
	};
}