#include "ai_module.h"
#include "machine_learning/neuron.h"
#include "machine_learning/neural_network.h"

namespace Echo
{
	AIModule::AIModule()
	{
	}

	void AIModule::registerTypes()
	{
		Class::registerType<Neuron>();
		Class::registerType<NeuralNetwork>();
	}
}