#pragma once

#include <vector>
#include <iostream>

namespace nn
{
	class Matrix
	{
		typedef std::vector<double> DoubleArray;

	public:
		Matrix();
		Matrix(int width, int height);

		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }
		int getNumberElements() const { return m_width * m_height; }

		// operator []
		float operator[] (int idx) const { return m_array[idx]; }

	private:
		int			m_width;
		int			m_height;
		DoubleArray m_array;
	};
}