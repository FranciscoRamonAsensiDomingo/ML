// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "../Classes/Components/SplineComponent.h"
#include "../MyJsonHandler.h"
#include "MyCube.h"
#include "AITrack.h"
#include "../JsonStructs.h"
#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Controller_AI.generated.h"

UCLASS()
class MYML_API AController_AI : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AController_AI();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
		int show_cubes = 6; // Number of cubes' mesh that are shown

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
		int population = 30; // Number of individuals

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
		float importance_diversity = 0.2; // Probability to be choosen = fitness * (1 - importance_diversity) + diversity * importance_diversity

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
		bool crossover = true; // Do we use crossover?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm", meta = (UIMin = "0.0", UIMax = "1.0"))
		float crossover_rate = 0.2; // the probability to experience crossover

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm", meta = (UIMin = "0.0", UIMax = "1.0"))
		float deadline = 0.2; // Avarage percentage that we select

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm", meta = (UIMin = "0.0", UIMax = "0.49"))
		float population_selection = 0.1; // Number of indivuals that we select to create the next generation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm", meta = (UIMin = "0.0", UIMax = "1.0"))
		float mutation_rate = 0.5; // the probability to be mutated

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm", meta = (UIMin = "0.0", UIMax = "1.0"))
		float mutation_change = 0.2; // Maximum change that can experience a weight during mutation



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Network")
		int InputLayer = 4; // Number of input neurons

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Network")
		TArray<int> HiddenLayer = { 3 }; // Topology of hidden layer

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Neural Network")
		int OutputLayer = 1; // Number of outputs (fixed) turn right or left



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Controller")
		bool player = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI Controller")
		TSubclassOf<class AMyCube> OurSpawningObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Controller")
		AAITrack *OurTrack;

	APlayerController* OurPlayer;

	void Initialize(bool learn = false);
	void ReInitialize();

	TArray<AMyCube*> Cubes;
	TArray<float>Score;

	void Probability();

	TArray<int> position;
	int last_best = 0;
	void CalcPosition();

	float refresh_frecuency = 0.2;
private:
	TArray<FTrainingData> PlayerTrace;

	float importance_diversity2;

	FString RelativePath = FPaths::ProjectContentDir();

	TArray<int> topology;

	float delta;
	float sumDelta = 0;

	int init_target;
	int current_target;

	int laps = 0;
	int maxLaps = 1;
	void CheckHit();
	void RefreshCarPosition();

	float TotalDiversity = 0;
	void CalcDiversity();
	float TotalFitness = 0;
	void CalcFitness();

	void GeneticAlgorithm();
	void GA_Selection(); TArray<NeuronalNetWork> selections;
	void GA_Crossover();
	void GA_Mutation();
};
