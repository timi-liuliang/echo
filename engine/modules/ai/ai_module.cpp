#include "ai_module.h"
#include "machine_learning/neuron.h"
#include "machine_learning/neural_layer.h"
#include "machine_learning/neural_network.h"
#include "machine_learning/editor/neuron_editor.h"
#include "machine_learning/editor/neural_layer_editor.h"
#include "machine_learning/editor/neural_network_editor.h"
#include "behavior_tree/behavior_tree.h"
#include "behavior_tree/editor/behavior_tree_editor.h"

namespace Echo
{
	DECLARE_MODULE(AIModule, __FILE__)

	AIModule::AIModule()
	{
	}

	AIModule::~AIModule()
	{
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

		CLASS_REGISTER_EDITOR(BehaviorTree, BehaviorTreeEditor)
		//CLASS_REGISTER_EDITOR(NeuralLayer, NeuralLayerEditor)
		//CLASS_REGISTER_EDITOR(NeuralNetwork, NeuralNetworkEditor)
	}

	void AIModule::update(float elapsedTime)
	{
	}
}