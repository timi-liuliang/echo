#include "pcg_connect.h"
#include "pcg_connect_point.h"
#include "engine/modules/pcg/node/pcg_node.h"

namespace Echo
{
	PCGConnectPoint::PCGConnectPoint(PCGNode* owner, const String& supportTypes)
		: m_owner(owner)
		, m_type(Type::Input)
		, m_supportTypes(supportTypes)
	{
	}

	PCGConnectPoint::PCGConnectPoint(PCGNode* owner, std::shared_ptr<PCGNode> data)
		: m_owner(owner)
		, m_type(Type::Output)
	{
		//m_data = data;

		if (data)
		{
			m_supportTypes = data->getType();
		}
	}

	PCGConnectPoint::~PCGConnectPoint()
	{

	}

	std::shared_ptr<PCGData> PCGConnectPoint::GetData()
	{
		if (m_type == Input)
		{
			if (m_connects.size() > 0)
			{
				//return m_connects[0]->m_from->GetData();
			}
		}
		else
		{
			return m_data;
		}

		return nullptr;
	}

	void PCGConnectPoint::AddConnect(std::shared_ptr<PCGConnect> InConnect)
	{
		m_connects.push_back(InConnect);
	}

	void PCGConnectPoint::RemoveConnect(std::shared_ptr<PCGConnect> InConnect)
	{
		//m_connects.erase(InConnect);
	}

	std::shared_ptr<PCGConnectPoint> PCGConnectPoint::GetDependEndPoint()
	{
		if (m_type == Input)
		{
			if (m_connects.size() > 0)
			{
				//return m_connects[0]->m_from;
			}
		}

		return nullptr;
	}
}