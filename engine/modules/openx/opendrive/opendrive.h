#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class OpenDrive : public Node
	{
		ECHO_CLASS(OpenDrive, Node)

	public:
		OpenDrive();
		virtual ~OpenDrive();

		// Xodr file
		void setXodrRes(const ResourcePath& path);
		const ResourcePath& getXodrRes() { return m_xodrRes; }

	private:
		// Parse
		void parseXodr(const String& content);

		// Update
		virtual void updateInternal() override;

	public:
		ResourcePath	m_xodrRes = ResourcePath("", ".xodr");
	};
}
