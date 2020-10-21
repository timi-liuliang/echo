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

		// is selected
		bool isSelected() const { return m_isSelected; }
		void setSelected(bool isSelected);

		// child
		void addChild(PGNode* node);
		void removeChild(PGNode* node);

		// get child
		ui32 getChildNum() const { return static_cast<ui32>(m_children.size()); }
		PGNode* getChildByIndex(ui32 idx);

		// link
		bool isLink() const { return m_isLink; }
		void setLink(bool isLink) { m_isLink = isLink; }

		// children
		const vector<PGNode*>::type& children() { return m_children; }

		// set parent
		PGNode* getParent() { return m_parent; }
		void setParent(PGNode* parent);

		// is dirty
		bool isDirty() { return m_dirtyFlag; }

		// calculate
		virtual void play(PCGData& data);

	public:
		// queue free
		virtual void queueFree() override;

	public:
		// save
		void saveXml(void* pugiNode, bool recursive);

		// instance
		static PGNode* instanceNodeTree(void* pugiNode, PGNode* parent);

	protected:
		PGNode*					m_parent = nullptr;
		vector<PGNode*>::type	m_children;
		bool					m_dirtyFlag = true;
		Vector2					m_position;
		bool					m_isFinal = false;
		bool					m_isSelected = false;
		bool					m_isLink = false;
	};
}