// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeuralNetwork.h"
#include "Utils/ECameraTypes.h"
#include "PCMGNeuralNetwork.generated.h"

/**
 * A helper class used to instantiate a neural network and integrate into UE.
 */
UCLASS(classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UPCMGNeuralNetwork : public UNeuralNetwork
{
	GENERATED_BODY()
	
public:
	UPCMGNeuralNetwork();

public:
	void RunModel(EPCMGModel ModelType, TArray<float>& Input, TArray<float>& Output);

};
