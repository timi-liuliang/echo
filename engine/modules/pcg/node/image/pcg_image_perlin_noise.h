#pragma once

#include "engine/modules/pcg/node/pcg_node.h"
#include "engine/modules/pcg/data/image/pcg_image.h"

namespace Echo
{
	class PCGImagePerlinNoise : public PCGNode
	{
		ECHO_CLASS(PCGImagePerlinNoise, PCGNode);

	public:
		PCGImagePerlinNoise();
		virtual ~PCGImagePerlinNoise();

		// Set Bounds
		i32 getWidth() const { return m_width; }
		void setWidth(i32 width);

		// height
		i32 getHeight() const { return m_height; }
		void setHeight(i32 height);

		// amplitude
		float getAmplitude() const { return m_amplitude; }
		void setAmplitude(float InAmplitude);

		// grid size
		float getGridSize() const { return m_gridSize; }
		void setGridSize(float InGridSize);

		// offset
		const Vector2& getOffset() const { return m_offset; }
		void setOffset(const Vector2& offset) { m_offset = offset; }

		// get result
		PCGImagePtr getResultImage() { return m_resultImage; }

		// Run
		virtual void run() override;

	public:
		// Compute Perlin noise at coordinates x, y
		float perlin(float x, float y);

		// Computes the dot product of the distance and gradient vector
		float dotGridGradient(i32 ix, i32 iy, float x, float y);

		// Create random gradient vector
		Vector2 randomGradient(i32 ix, i32 iy);

		// Interpolate between a0 and a1
		float interpolate(float a0, float a1, float w);

		// Interpolate between a0 and a1
		float sCurveInterpolate(float t);

	protected:
		i32				m_width = 128;
		i32				m_height = 128;
		float			m_amplitude = 1.f;
		float			m_gridSize = 32.f;
		Vector2			m_offset = Vector2::ZERO;
		PCGImagePtr		m_resultImage;
	};
}
