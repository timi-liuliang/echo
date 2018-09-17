#include "matrix.h"

namespace nn
{
	Matrix::Matrix()
	{
	}

	Matrix::Matrix(int width, int height)
		: m_width(width)
		, m_height(height)
	{
		m_array.resize(m_width * m_height, 0.0);
	}
}