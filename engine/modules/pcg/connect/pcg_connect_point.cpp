#include "pcg_connect_point.h"

PCGConnectPoint::PCGConnectPoint(PCGNode* InOwner, const String& InSupportTypes)
	: Owner(InOwner)
	, Type(Input)
	, SupportTypes(InSupportTypes)
{
}

PCGConnectPoint::PCGConnectPoint(PCGNode* InOwner, std::shared_ptr<PCGData> InData)
	: Owner(InOwner)
	, Type(Output)
{
	Data = InData;

	if (InData)
	{
		SupportTypes = InData->getType();
	}
}

PCGConnectPoint::~PCGConnectPoint()
{

}

TSharedPtr<PCGData> PCGConnectPoint::GetData()
{ 
	if (Type == Input)
	{
		if (Connects.Num() > 0)
		{
			return Connects[0]->From->GetData();
		}
	}
	else
	{
		return Data;
	}

	return nullptr;
}

void PCGConnectPoint::AddConnect(std::shared_ptr<PCGConnect> InConnect)
{
	Connects.Add(InConnect);
}

void PCGConnectPoint::RemoveConnect(std::shared_ptr<PCGConnect> InConnect)
{
	Connects.Remove(InConnect);
}

std::shared_ptr<PCGConnectPoint> PCGConnectPoint::GetDependEndPoint()
{
	if (Type == Input)
	{
		if (Connects.Num() > 0)
		{
			return Connects[0]->From;
		}
	}

	return nullptr;
}