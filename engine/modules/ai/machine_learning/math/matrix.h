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

	private:
		int			m_width;
		int			m_height;
		DoubleArray m_array;
	};
}