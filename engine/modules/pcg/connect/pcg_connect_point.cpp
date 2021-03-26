#include "pcg_connect.h"
#include "pcg_connect_point.h"
#include "engine/modules/pcg/node/pcg_node.h"

namespace Echo
{
	PCGConnectPoint::PCGConnectPoint(PCGNode* owner, const String& supportTypes, PCGConnectPoint::Type type)
		: m_owner(owner)
		, m_type(type)
		, m_supportTypes(supportTypes)
	{
	}

	PCGConnectPoint::PCGConnectPoint(PCGNode* owner, PCGDataPtr data)
		: m_owner(owner)
		, m_type(Type::Output)
	{
		m_data = data;

		if (data)
		{
			m_supportTypes = data->getType();
		}
	}

	PCGConnectPoint::~PCGConnectPoint()
	{

	}

	String PCGConnectPoint::getDataType()
	{ 
		return m_data->getType(); 
	}

	PCGDataPtr PCGConnectPoint::getData()
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

	void PCGConnectPoint::addConnect(PCGConnect* InConnect)
	{
		m_connects.push_back(InConnect);
	}

	void PCGConnectPoint::removeConnect(PCGConnect* InConnect)
	{
		//m_connects.erase(InConnect);
	}

	PCGConnectPoint* PCGConnectPoint::getDependEndPoint()
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