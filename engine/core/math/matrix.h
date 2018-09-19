#pragma once

#include <vector>
#include <iostream>
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	typedef Echo::vector<float>::type RealVector;

	class Matrix
	{
	public:
		Matrix();
		Matrix(int width, int height);
		Matrix(const RealVector& array) { m_array = array; }

		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }
		int getNumberElements() const { return m_width * m_height; }

		// operator []
		double operator[] (int idx) const { return m_array[idx]; }

		// apply function
		Matrix applyFunction(double(*function)(double)) const;

	private:
		int			m_width;
		int			m_height;
		RealVector  m_array;
	};
}