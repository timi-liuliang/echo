#include "ai_module.h"
#include "machine_learning/neuron.h"
#include "machine_learning/neural_layer.h"
#include "machine_learning/neural_network.h"
#include "machine_learning/editor/neuron_editor.h"
#include "machine_learning/editor/neural_layer_editor.h"
#include "machine_learning/editor/neural_network_editor.h"

namespace Echo
{
	AIModule::AIModule()
	{
	}

	void AIModule::registerTypes()
	{
		//Class::registerType<Neuron>();
		//Class::registerType<NeuralLayer>();
		//Class::registerType<NeuralNetwork>();

		//REGISTER_OBJECT_EDITOR(Neuron, NeuronEditor)
		//REGISTER_OBJECT_EDITOR(NeuralLayer, NeuralLayerEditor)
		//REGISTER_OBJECT_EDITOR(NeuralNetwork, NeuralNetworkEditor)
	}
}