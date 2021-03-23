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
		PCGConnectPoint(PCGNode* owner, const String& supportTypes);
		PCGConnectPoint(PCGNode* owner, std::shared_ptr<PCGData> data);
		~PCGConnectPoint();

		// Owner
		PCGNode* getOwner() { return m_owner; }

		// Data type
		String getDataType() const { return m_data->getType(); }

		// Data
		std::shared_ptr<PCGData> getData();
		void setData(std::shared_ptr<PCGData> InData) { m_data = InData; }

	public:
		// Connect
		void addConnect(PCGConnect* InConnect);
		void removeConnect(PCGConnect* InConnect);

		// Depend
		PCGConnectPoint* getDependEndPoint();

	protected:
		PCGNode*						m_owner = nullptr;
		Type							m_type;
		String							m_supportTypes;
		std::shared_ptr<PCGData>		m_data;
		std::vector<class PCGConnect*>	m_connects;
	};
}
