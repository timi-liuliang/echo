#pragma once

#include "engine/modules/pcg/node/pcg_node.h"
#include "engine/modules/pcg/data/image/pcg_image.h"

namespace Echo
{
	class PCGImageVoronoi : public PCGNode
	{
		ECHO_CLASS(PCGImageVoronoi, PCGNode);

	public:
		enum DistributionType
		{
			Poisson,
			Random,
			Grid,
			Hexagonal
		};

	public:
		PCGImageVoronoi();
		virtual ~PCGImageVoronoi();

		// Set Bounds
		i32 getWidth() const { return m_width; }
		void setWidth(i32 width);

		// height
		i32 getHeight() const { return m_height; }
		void setHeight(i32 height);

		// get result
		PCGImagePtr getResultImage() { return m_resultImage; }

		// Run
		virtual void run() override;

	public:
		// Compute Perlin noise at coordinates x, y
		float voronoi(float x, float y);

		// distribution
		void generateSites();
		void poissonDiscSampling();
		void randomSampling();

	protected:
		i32						m_width = 128;
		i32						m_height = 128;
		DistributionType		m_distributionType = DistributionType::Random;
		i32						m_randomSeed;
		i32						m_randomSamplingCount = 32;
		vector<Vector2>::type	m_sites;
		PCGImagePtr				m_resultImage;
	};
}
