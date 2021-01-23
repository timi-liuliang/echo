#pragma once

#include "../pg_node.h"

namespace Echo
{
	class PGBox : public PGNode
	{
		ECHO_CLASS(PGBox, PGNode)

	public:
		PGBox();
		virtual ~PGBox();

		// width
		void setSize(const Vector3& size);
		const Vector3& getSize() const { return m_size; }

		// calculate
		virtual void play(PCGData& data) override;

	protected:
		Vector3		m_size = Vector3::ONE;
	};
}
