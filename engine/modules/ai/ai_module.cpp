#include "ai_module.h"
#include "machine_learning/neuron.h"
#include "machine_learning/neural_layer.h"
#include "machine_learning/neural_network.h"
#include "machine_learning/editor/neuron_editor.h"
#include "machine_learning/editor/neural_layer_editor.h"
#include "machine_learning/editor/neural_network_editor.h"
#include "behavior_tree/behavior_tree.h"
#include "behavior_tree/editor/behavior_tree_editor.h"
#include "rvo/rvo_agent.h"

namespace Echo
{
	DECLARE_MODULE(AIModule)

	AIModule::AIModule()
	{
		m_rvoSimulator = EchoNew(RvoSimulator);
	}

	AIModule::~AIModule()
	{
		EchoSafeDelete(m_rvoSimulator, RvoSimulator);
	}

	AIModule* AIModule::instance()
	{
		static AIModule* inst = EchoNew(AIModule);
		return inst;
	}

	void AIModule::bindMethods()
	{

	}

	void AIModule::registerTypes()
	{
		Class::registerType<BehaviorTree>();
		//Class::registerType<Neuron>();
		//Class::registerType<NeuralLayer>();
		//Class::registerType<NeuralNetwork>();
		Class::registerType<RvoAgent>();

		CLASS_REGISTER_EDITOR(BehaviorTree, BehaviorTreeEditor)
		//REGISTER_OBJECT_EDITOR(NeuralLayer, NeuralLayerEditor)
		//REGISTER_OBJECT_EDITOR(NeuralNetwork, NeuralNetworkEditor)
	}

	void AIModule::update(float elapsedTime)
	{
		if (m_rvoSimulator)
		{
			m_rvoSimulator->update(elapsedTime);
		}
	}
}