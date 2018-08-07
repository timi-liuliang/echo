#include "engine/core/geom/ParamSurface.h"
#include "engine/core/math/Vector3.h"
#include "engine/core/util/Buffer.h"

namespace Echo
{
	ParamSurface::ParamSurface(const Vector2& vDivision, const Vector2& vUpperBound, const Vector2& vTcrdWeight)
	{
		m_vDivision = vDivision;
		m_vUpperBound = vUpperBound;
		m_vTcrdWeight = vTcrdWeight;
		m_vSlice = m_vDivision - Vector2::ONE;
	}

	ParamSurface::~ParamSurface()
	{
	}

	ui32 ParamSurface::getVertexCount() const
	{
		return (ui32)m_vDivision.x * (ui32)m_vDivision.y;
	}

	ui32 ParamSurface::getLineIndexCount() const
	{
		return 4 * (ui32)m_vSlice.x * (ui32)m_vSlice.y;
	}

	ui32 ParamSurface::getTriangleIndexCount() const
	{
		return 6 * (ui32)m_vSlice.x * (ui32)m_vSlice.y;
	}

	void ParamSurface::generateVertices(Buffer& buff, bool bNormal, bool bTexCoord) const
	{
		ui32 vertStride = sizeof(Vector3);
		if (bNormal)
			vertStride += sizeof(Vector3);
		if (bTexCoord)
			vertStride += sizeof(Vector2);

		buff.allocate(getVertexCount() * vertStride);
		buff.getData();

		ui32 offset = 0;
		for(ui32 j = 0; j < m_vDivision.y; ++j)
		{
			for(ui32 i = 0; i < m_vDivision.x; ++i)
			{
				// compute position
				Vector2 vDomain = computeDomain((Real)i, (Real)j);
				Vector3 vRange = evaluate(vDomain);
				memcpy(buff.getData() + offset, &vRange, sizeof(Vector3));
				offset += sizeof(Vector3);

				// compute normal
				if (bNormal)
				{
					float s = (Real)i, t = (Real)j;

					// nudge the point if the normal is indeterminate.
					if (i == 0)
						s += 0.01f;
					if (i == m_vDivision.x - 1)
						s -= 0.01f;
					if (j == 0)
						t += 0.01f;
					if (j == m_vDivision.y - 1)
						t -= 0.01f;

					// compute the tangents and their cross product.
					Vector3 p = evaluate(computeDomain(s, t));
					Vector3 u = evaluate(computeDomain(s + 0.01f, t)) - p;
					Vector3 v = evaluate(computeDomain(s, t + 0.01f)) - p;
					Vector3 vNormal = u.cross(v);
					vNormal.z = -vNormal.z;
					vNormal.normalize();
					if (isNormalInverse(vDomain))
						vNormal = -vNormal;

					memcpy(buff.getData() + offset, &vNormal, sizeof(Vector3));
					offset += sizeof(Vector3);
				}

				// Compute Texture Coordinates
				if (bTexCoord)
				{
					Real s = m_vTcrdWeight.x * i / m_vSlice.x;
					Real t = m_vTcrdWeight.y * j / m_vSlice.y;
					Vector2 vTcrd(s, t);
					memcpy(buff.getData() + offset, &vTcrd, sizeof(Vector2));
					offset += sizeof(Vector2);
				}
			} // for
		} // for
	}

	void ParamSurface::generateLineIndices(Buffer& buff, ui32& stride) const
	{
		stride = (getLineIndexCount() > Math::MAX_WORD)? sizeof(Dword) : sizeof(Word);
		buff.allocate(getLineIndexCount()*stride);
		ui32 offset = 0;
		for(ui32 j = 0, nVert = 0; j < m_vSlice.y; ++j)
		{
			for(ui32 i = 0; i < m_vSlice.x; ++i)
			{
				ui32 next = (i + 1) % (ui32)m_vDivision.x;

				if(stride == sizeof(Word))
				{
					Word wIndices[4];
					wIndices[0] = nVert + i;
					wIndices[1] = nVert + next;
					wIndices[2] = nVert + i;
					wIndices[3] = nVert + (ui32)m_vDivision.x;
					memcpy(buff.getData() + offset, wIndices, 4 * stride);
					offset += 4 * stride;
				}
				else
				{
					Dword dwIndices[4];
					dwIndices[0] = nVert + i;
					dwIndices[1] = nVert + next;
					dwIndices[2] = nVert + i;
					dwIndices[3] = nVert + (ui32)m_vDivision.x;
					memcpy(buff.getData() + offset, dwIndices, 4 * stride);
					offset += 4 * stride;
				}
			}

			nVert += (ui32)m_vDivision.x;
		}
	}

	void ParamSurface::generateTriangleIndices(Buffer& buff, ui32& stride) const
	{
		stride = (getTriangleIndexCount() > Math::MAX_WORD)? sizeof(Dword) : sizeof(Word);
		buff.allocate(getTriangleIndexCount()*stride);
		ui32 offset = 0;
		for(ui32 j = 0, nVert = 0; j < m_vSlice.y; ++j)
		{
			for(ui32 i = 0; i < m_vSlice.x; ++i)
			{
				ui32 next = (i + 1) % (ui32)m_vDivision.x;

				if(stride == sizeof(Word))
				{
					Word wIndices[6];
					wIndices[0] = nVert + i;
					wIndices[1] = nVert + next;
					wIndices[2] = nVert + i + (ui32)m_vDivision.x;
					wIndices[3] = nVert + next;
					wIndices[4] = nVert + next + (ui32)m_vDivision.x;
					wIndices[5] = nVert + i + (ui32)m_vDivision.x;
					memcpy(buff.getData() + offset, wIndices, 6 * stride);
					offset += 6 * stride;
				}
				else
				{
					Dword dwIndices[6];
					dwIndices[0] = nVert + i;
					dwIndices[1] = nVert + next;
					dwIndices[2] = nVert + i + (ui32)m_vDivision.x;
					dwIndices[3] = nVert + next;
					dwIndices[4] = nVert + next + (ui32)m_vDivision.x;
					dwIndices[5] = nVert + i + (ui32)m_vDivision.x;
					memcpy(buff.getData() + offset, dwIndices, 6 * stride);
					offset += 6 * stride;
				}
			}

			nVert += (ui32)m_vDivision.x;
		}
	}

	bool ParamSurface::isNormalInverse(const Vector2& vDomain) const
	{
		return false;
	}

	Vector2 ParamSurface::computeDomain(Real x, Real y) const
	{
		return Vector2(x * m_vUpperBound.x / m_vSlice.x, y * m_vUpperBound.y / m_vSlice.y);
	}

	//---------------------------------------------------------------------------------------

	SphereSurface::SphereSurface(Real radius, const Vector2& vDivision, const Vector2& vUpperBound, const Vector2& vTcrdWeight)
		:ParamSurface(vDivision, vUpperBound, vTcrdWeight)
		,m_radius(radius)
	{
	}

	Vector3 SphereSurface::evaluate(const Vector2& vDomain) const
	{
		Real u = vDomain.x;
		Real v = vDomain.y;
		Real x = m_radius * Math::Sin(u) * Math::Cos(v);
		Real y = m_radius * Math::Cos(u);
		Real z = m_radius * -Math::Sin(u) * Math::Sin(v);
		return Vector3(x, y, z);
	}
}
