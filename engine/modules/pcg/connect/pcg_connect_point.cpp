#include "pcg_connect.h"
#include "pcg_connect_point.h"
#include "engine/modules/pcg/node/pcg_node.h"

namespace Echo
{
	PCGConnectPoint::PCGConnectPoint(PCGNode* owner, const String& supportTypes, PCGConnectPoint::Type type)
		: m_owner(owner)
		, m_type(type)
		, m_supportDataTypes(supportTypes)
	{
	}

	PCGConnectPoint::PCGConnectPoint(PCGNode* owner, PCGDataPtr data)
		: m_owner(owner)
		, m_type(Type::Output)
	{
		m_data = data;

		if (data)
		{
			m_supportDataTypes = data->getType();
		}
	}

	PCGConnectPoint::~PCGConnectPoint()
	{

	}

	i32 PCGConnectPoint::getIdx()
	{
		if (m_owner)
		{
			const std::vector<PCGConnectPoint*>& connectPoints = m_type == Type::Input ? m_owner->getInputs() : m_owner->getOutputs();
			for (size_t i = 0; i < connectPoints.size(); i++)
			{
				if (this == connectPoints[i])
					return i32(i);
			}
		}

		return -1;
	}

	String PCGConnectPoint::getDataType()
	{ 
		return m_data ? m_data->getType() : StringUtil::BLANK; 
	}

	PCGDataPtr PCGConnectPoint::getData()
	{
		if (m_type == Input)
		{
			if (m_connects.size() > 0)
			{
				return m_connects[0]->getFrom()->getData();
			}
		}
		else
		{
			return m_data;
		}

		return nullptr;
	}

	void PCGConnectPoint::addConnect(PCGConnect* InConnect)
	{
		m_connects.push_back(InConnect);
	}

	void PCGConnectPoint::removeConnect(PCGConnect* connect)
	{
		m_connects.erase(std::remove(m_connects.begin(), m_connects.end(), connect), m_connects.end());
	}

	PCGConnectPoint* PCGConnectPoint::getDependEndPoint()
	{
		if (m_type == Input)
		{
			if (m_connects.size() > 0)
			{
				return m_connects[0]->getFrom();
			}
		}

		return nullptr;
	}
}