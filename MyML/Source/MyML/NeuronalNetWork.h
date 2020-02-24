// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "Json.h"
#include "JsonStructs.h"
#include "Engine.h"
#include "Templates/SharedPointer.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "JsonUtilities/Public/JsonObjectConverter.h"
#include "CoreMinimal.h"


class MYML_API NeuronalNetWork
{
public:
	NeuronalNetWork();
	NeuronalNetWork(TArray<int> log);
	NeuronalNetWork(FString);
	
	~NeuronalNetWork();

	void Init(TArray<int> log);

	TArray<float> forward(TArray<float> data);

	TArray<TArray<TArray<float>>> NNetwork;

private:

	float Sigmoid(float x) { return -1 + 2 / (1 + FGenericPlatformMath::Exp(-x)); }
};
