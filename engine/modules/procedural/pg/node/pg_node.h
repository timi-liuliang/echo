#pragma once

#include "engine/core/base/object.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/modules/procedural/pg/data/pg_data.h"

namespace Echo
{
	class PGNode : public Object
	{
		ECHO_CLASS(PGNode, Object);

	public:
		// pos
		void setPosition(const Vector2& position) { m_position = position; }
		const Vector2& getPosition() { return m_position; }

		// final
		bool isFinal() const { return m_isFinal; }
		void setFinal(bool isFinal);

		// child
		void addChild(PGNode* node);
		void removeChild(PGNode* node);

		// children
		const vector<PGNode*>::type& children() { return m_children; }

		// set parent
		PGNode* getParent() { return m_parent; }
		void setParent(PGNode* parent);

		// is dirty
		bool isDirty() { return m_dirtyFlag; }

		// calculate
		virtual void play(PCGData& data);

	protected:
		PGNode*					m_parent = nullptr;
		vector<PGNode*>::type	m_children;
		bool					m_dirtyFlag = true;
		Vector2					m_position;
		bool					m_isFinal = false;
	};
}