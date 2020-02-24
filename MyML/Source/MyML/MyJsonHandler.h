// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "JsonStructs.h"
#include "NeuronalNetWork.h"
#include "Engine.h"
#include "Templates/SharedPointer.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "JsonUtilities/Public/JsonObjectConverter.h"

#include "CoreMinimal.h"


/**
 * 
 */
class MYML_API MyJsonHandler
{
public:
	MyJsonHandler();
	~MyJsonHandler();

	// Neural Network
	static void Write_Network(const NeuronalNetWork& nnetwork, FString path);
	static NeuronalNetWork Load_Network(FString path);

	// Training Data
	static void Write_DataTraining(const TArray<FTrainingData>& data, FString path);
};
