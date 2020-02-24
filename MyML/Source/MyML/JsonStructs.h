// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JsonStructs.generated.h"

/**
 * 
 */
UCLASS()
class MYML_API UJsonStructs : public UObject
{
	GENERATED_BODY()
	
};
USTRUCT()
struct FAiInput {
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		float RotationPlayer;

};


USTRUCT()
struct FTrainingData {
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		TArray<float> input;

	UPROPERTY()
		TArray<float> output;
};


USTRUCT()
struct FNNetworkLayer
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		int32 n_neurons_pre;

	UPROPERTY()
		int32 n_neurons_pos;

	UPROPERTY()
		TArray<float> weight;
};

USTRUCT()
struct FNNetwork
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		TArray<FNNetworkLayer> NNetwork;
};
