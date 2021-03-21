#pragma once

#include "pcg/data/pcg_data.h"

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
	PCGConnectPoint(PCGNode* InOwner, const String& InSupportTypes);
	PCGConnectPoint(PCGNode* InOwner, std::shared_ptr<PCGNode> InData);
	~PCGConnectPoint();

	// Owner
	PCGNode* GetOwner() { return Owner; }

	// Data type
	String GetDataType() const { return Data->GetType(); }

	// Data
	std::shared_ptr<PCGData> GetData();
	void SetData(std::shared_ptr<PCGData> InData) { Data = InData; }

public:
	// Connect
	void AddConnect(TSharedPtr<FPCGConnect> InConnect);
	void RemoveConnect(TSharedPtr<FPCGConnect> InConnect);

	// Depend
	TSharedPtr<PCGConnectPoint> GetDependEndPoint();

protected:
	PCGNode*										Owner = nullptr;
	EType											Type;
	FString											SupportTypes;
	std::shared_ptr<PCGData>						Data;
	std::vector<std::shared_ptr<class PCGConnect>>	Connects;
};