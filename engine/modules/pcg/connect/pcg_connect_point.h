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
		PCGConnectPoint(PCGNode* owner, std::shared_ptr<PCGNode> data);
		~PCGConnectPoint();

		// Owner
		PCGNode* GetOwner() { return m_owner; }

		// Data type
		String GetDataType() const { return m_data->getType(); }

		// Data
		std::shared_ptr<PCGData> GetData();
		void SetData(std::shared_ptr<PCGData> InData) { m_data = InData; }

	public:
		// Connect
		void AddConnect(std::shared_ptr<PCGConnect> InConnect);
		void RemoveConnect(std::shared_ptr<PCGConnect> InConnect);

		// Depend
		std::shared_ptr<PCGConnectPoint> GetDependEndPoint();

	protected:
		PCGNode*										m_owner = nullptr;
		Type											m_type;
		String											m_supportTypes;
		std::shared_ptr<PCGData>						m_data;
		std::vector<std::shared_ptr<class PCGConnect>>	m_connects;
	};
}
