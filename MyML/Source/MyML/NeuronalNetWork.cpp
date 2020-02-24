// Fill out your copyright notice in the Description page of Project Settings.


#include "NeuronalNetWork.h"

NeuronalNetWork::NeuronalNetWork()
{
}

NeuronalNetWork::NeuronalNetWork(TArray<int> log)
{
	Init(log);
}

NeuronalNetWork::NeuronalNetWork(FString path)
{
	//NNetwork = MyJsonHandler::Load_Network(path).NNetwork;
}

NeuronalNetWork::~NeuronalNetWork()
{
}

void NeuronalNetWork::Init(TArray<int> log)
{
	NNetwork.Empty();

	for (int i = 0; i < log.Num() - 1; i++) {
		TArray<TArray<float>> layer;
		for (int j = 0; j < log[i]; j++) {
			TArray<float> sublayer;
			for (int k = 0; k < log[i + 1]; k++) {
				sublayer.Add(2.f*FMath::FRand() - 1.f);
			}
			layer.Add(sublayer);
		}
		NNetwork.Add(layer);
	}
}

TArray<float> NeuronalNetWork::forward(TArray<float> data)
{
	for (int i = 0; i < NNetwork.Num(); ++i) {
		TArray<float> output;
		for (int j = 0; j < NNetwork[i].Num(); ++j) {
			output.Add(0);
			for (int k = 0; k < NNetwork[i][0].Num(); ++k) {
				output[j] += data[k] * NNetwork[i][j][k];// data[j]
			}
			output[j] = Sigmoid(output[j]);
		}
		data = output;
	}
	return data;
}
