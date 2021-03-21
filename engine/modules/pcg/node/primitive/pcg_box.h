#pragma once

#include "pcg/node/pcg_node.h"

namespace Echo
{
	class PCGBox : public PCGNode
	{
		ECHO_CLASS(PCGBox, PCGNode)

	public:
		PCGBox();
		virtual ~PCGBox();

		// width
		void setSize(const Vector3& size);
		const Vector3& getSize() const { return m_size; }

		// calculate
		virtual void run() override;

	private:
		// add plane
		void addPlane(PCGData& data, i32 axis, bool negative);

	protected:
		Vector3		m_size = Vector3::ONE;
	};
}
