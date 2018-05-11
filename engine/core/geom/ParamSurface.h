#pragma once

#include "engine/core/Util/Buffer.h"
#include "engine/core/Math/Vector2.h"
#include "engine/core/Math/Vector3.h"

namespace Echo
{
	class ParamSurface
	{
	public:
		ParamSurface(const Vector2& vDivision, const Vector2& vUpperBound, const Vector2& vTcrdWeight);
		virtual ~ParamSurface();

	public:
		ui32			getVertexCount() const;
		ui32			getLineIndexCount() const;
		ui32			getTriangleIndexCount() const;

		void			generateVertices(Buffer& buff, bool bNormal = true, bool bTexCoord = true) const;
		void			generateLineIndices(Buffer& buff, ui32& stride) const;
		void			generateTriangleIndices(Buffer& buff, ui32& stride) const;

	protected:
		virtual Vector3	evaluate(const Vector2& domain) const = 0;
		virtual bool	isNormalInverse(const Vector2& vDomain) const;
		Vector2			computeDomain(Real i, Real j) const;

	protected:
		Vector2	m_vDivision;
		Vector2	m_vUpperBound;
		Vector2	m_vTcrdWeight;
		Vector2	m_vSlice;
	};

	class SphereSurface: public ParamSurface
	{
	public:
		SphereSurface(	Real radius,
			const Vector2& vDivision = Vector2(20, 20),
			const Vector2& vUpperBound = Vector2(Math::PI, Math::PI_2),
			const Vector2& vTcrdWeight = Vector2(20, 35));

	private:
		Vector3		evaluate(const Vector2& vDomain) const;

	private:
		Real		m_radius;
	};
}
