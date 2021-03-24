#include "pcg_image_perlin_noise.h"
#include "engine/modules/pcg/data/image/pcg_image.h"

namespace Echo
{
	PCGImagePerlinNoise::PCGImagePerlinNoise()
	{
		m_resultImage = new PCGImage;
		m_outputs.push_back(new PCGConnectPoint(this, m_resultImage.ptr()));
	}

	PCGImagePerlinNoise::~PCGImagePerlinNoise()
	{

	}

	void PCGImagePerlinNoise::bindMethods()
	{

	}

	void PCGImagePerlinNoise::setWidth(i32 width)
	{
		if (m_width != width)
		{
			m_width = width;
			m_dirtyFlag = true;
		}
	}

	void PCGImagePerlinNoise::setHeight(i32 height)
	{ 
		if (m_height != height)
		{
			m_height = height;
			m_dirtyFlag = true;
		}
	}

	void PCGImagePerlinNoise::run()
	{
		if (m_dirtyFlag)
		{
			m_resultImage->set(PF_R8_UINT, m_width, m_height);

			for (i32 x = 0; x < m_width; x++)
			{
				for (i32 y = 0; y < m_height; y++)
				{
					float grayScale = perlin((x + m_offset.x) / m_gridSize, (y + m_offset.y) / m_gridSize);

					// Apply amplitude
					grayScale = Math::Clamp(grayScale * m_amplitude, -1.f, 1.f);

					// Mapping from (-1, 1) to (0, 1)
					grayScale = (grayScale + 1.f) * 0.5f;

					// Change image's value
					assert(grayScale >= 0.f && grayScale <= 1.f);
					m_resultImage->setValue(x, y, Color(grayScale, grayScale, grayScale, grayScale));
				}
			}

			m_dirtyFlag = false;
		}
	}

	float PCGImagePerlinNoise::perlin(float x, float y)
	{
		// Determin grid cell coordinates
		i32 x0 = (i32)x;
		i32 x1 = x0 + 1;
		i32 y0 = (i32)y;
		i32 y1 = y0 + 1;

		// Determine interpolation weights
		float sx = x - x0;
		float sy = y - y0;

		// Interpolate between grid point gradients
		float n0, n1, ix0, ix1, value;

		n0 = dotGridGradient(x0, y0, x, y);
		n1 = dotGridGradient(x1, y0, x, y);
		ix0 = interpolate(n0, n1, sx);

		n0 = dotGridGradient(x0, y1, x, y);
		n1 = dotGridGradient(x1, y1, x, y);
		ix1 = interpolate(n0, n1, sx);

		value = interpolate(ix0, ix1, sy);

		return value;
	}

	float PCGImagePerlinNoise::dotGridGradient(i32 ix, i32 iy, float x, float y)
	{
		Vector2 gradient = randomGradient(ix, iy);

		// Compute the distance vector
		float dx = x - float(ix);
		float dy = y - float(iy);

		// Compute the dot-product
		return (dx * gradient.x + dy * gradient.y);
	}

	Vector2 PCGImagePerlinNoise::randomGradient(i32 ix, i32 iy)
	{
		float Random = 2920.f * Math::Sin(ix * 21942.f + iy * 171324.f + 8912.f) * Math::Cos(ix * 23157.f * iy * 217832.f + 9758.f);
		return Vector2(Math::Cos(Random), Math::Sin(Random));
	}

	float PCGImagePerlinNoise::interpolate(float a0, float a1, float w)
	{
		if (m_interpolateType == sCurve)return a0 + sCurveInterpolate(w) * (a1 - a0);
		else							return a0 + w * (a1 - a0);
	}

	// https://adrianb.io/2014/08/09/perlinnoise.html
	float PCGImagePerlinNoise::sCurveInterpolate(float t)
	{
		return 6 * Math::Pow(t, 5.f) - 15 * Math::Pow(t, 4.f) + 10 * Math::Pow(t, 3.f);
	}
}