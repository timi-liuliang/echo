#include "pcg_image_voronoi.h"
#include "engine/modules/pcg/data/image/pcg_image.h"

namespace Echo
{
	PCGImageVoronoi::PCGImageVoronoi()
	{
		m_resultImage = new PCGImage;
		m_outputs.push_back(new PCGConnectPoint(this, m_resultImage.ptr()));
	}

	PCGImageVoronoi::~PCGImageVoronoi()
	{

	}

	void PCGImageVoronoi::bindMethods()
	{
		CLASS_BIND_METHOD(PCGImageVoronoi, getWidth, DEF_METHOD("getWidth"));
		CLASS_BIND_METHOD(PCGImageVoronoi, setWidth, DEF_METHOD("setWidth"));
		CLASS_BIND_METHOD(PCGImageVoronoi, getHeight, DEF_METHOD("getHeight"));
		CLASS_BIND_METHOD(PCGImageVoronoi, setHeight, DEF_METHOD("setHeight"));

		CLASS_REGISTER_PROPERTY(PCGImageVoronoi, "Width", Variant::Type::Int, "getWidth", "setWidth");
		CLASS_REGISTER_PROPERTY(PCGImageVoronoi, "Height", Variant::Type::Int, "getHeight", "setHeight");
	}

	void PCGImageVoronoi::setWidth(i32 width)
	{
		if (m_width != width)
		{
			m_width = width;
			m_dirtyFlag = true;
		}
	}

	void PCGImageVoronoi::setHeight(i32 height)
	{ 
		if (m_height != height)
		{
			m_height = height;
			m_dirtyFlag = true;
		}
	}

	void PCGImageVoronoi::run()
	{
		if (m_dirtyFlag)
		{
			m_resultImage->set(PF_R8_UINT, m_width, m_height);

			// step 1
			generateSites();

			for (i32 x = 0; x < m_width; x++)
			{
				for (i32 y = 0; y < m_height; y++)
				{
					float grayScale = voronoi(x, y);

					// Apply amplitude
					grayScale = Math::Clamp(grayScale, -1.f, 1.f);

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

	// Constructing a Voronoi partition (Site Vertex Boundary Cell)
	// https://www.khanacademy.org/computing/pixar/pattern/dino/e/constructing-a-voronoi-partition
	float PCGImageVoronoi::voronoi(float x, float y)
	{
		return 1.f;
	}

	void PCGImageVoronoi::generateSites()
	{
		m_sites.clear();

		switch (m_distributionType)
		{
		case DistributionType::Poisson: poissonDiscSampling();	break;
		case DistributionType::Random:	randomSampling();		break;
		default:						poissonDiscSampling();	break;
		}
	}

	// Fast Poisson Disk Sampling in Arbitrary Dimensions
	// https://www.cct.lsu.edu/~fharhad/ganbatte/siggraph2007/CD2/content/sketches/0250.pdf
	// Coding Challenge #33: Poisson-disc Sampling
	// https://www.youtube.com/watch?v=flQgnCUxHlw
	void PCGImageVoronoi::poissonDiscSampling()
	{

	}

	void PCGImageVoronoi::randomSampling()
	{
		std::srand(m_randomSeed);
		for (i32 i = 0; i < m_randomSamplingCount; i++)
		{
			m_sites.push_back(Vector2(std::rand() % m_width, std::rand() % m_height));
		}
	}
}
