#pragma once

#include "engine/modules/pcg/data/pcg_data.h"

namespace Echo
{
	class PCGNode;
	class PCGConnect;
	class PCGConnectPoint
	{
		friend class PCGNode;

	public:
		// Type
		enum Type
		{
			Input,
			Output
		};

	public:
		PCGConnectPoint(PCGNode* owner, const String& supportTypes, Type type=Input);
		PCGConnectPoint(PCGNode* owner, PCGDataPtr data);
		~PCGConnectPoint();

		// Owner
		PCGNode* getOwner() { return m_owner; }

		// idx
		i32 getIdx();

		// type
		Type getType() const { return m_type; }

		// Data type
		String getDataType();
		String getSupportDataTypes() const { return m_supportDataTypes; }

		// connect
		bool isHaveConnect() const { return !m_connects.empty(); }

		// Data
		PCGDataPtr getData();
		void setData(PCGDataPtr InData) { m_data = InData; }

	public:
		// Connect
		void addConnect(PCGConnect* InConnect);
		void removeConnect(PCGConnect* InConnect);

		// Depend
		PCGConnectPoint* getDependEndPoint();

	protected:
		PCGNode*						m_owner = nullptr;
		Type							m_type;
		String							m_supportDataTypes;
		PCGDataPtr						m_data;
		std::vector<class PCGConnect*>	m_connects;
	};
}
