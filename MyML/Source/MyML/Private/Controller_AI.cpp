// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/Controller_AI.h"

// Sets default values
AController_AI::AController_AI()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AController_AI::BeginPlay()
{
	Super::BeginPlay();
	OurPlayer = UGameplayStatics::GetPlayerController(this, 0);

	if (show_cubes > population) {
		show_cubes = population;
	}

	// Init Score of each population
	this->Score.Init(0, population);

	// Init Neural Network of selected
	int n_selected = (int)(population*population_selection);
	for (int i = 0; i < n_selected; i++)
		selections.Add(NeuronalNetWork());

	// Init positions
	for (int i = 0; i < population + player; i++)
		position.Add(i);

	// Init topology
	topology.Add(InputLayer);
	for (auto&i : HiddenLayer)
		topology.Add(i);
	topology.Add(OutputLayer);

	importance_diversity2 = 1 / (importance_diversity + 1);
	this->Initialize(); // Spawn objects

	Cubes[0]->nnetwork = MyJsonHandler::Load_Network(RelativePath + "../NNdata/test.json");
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("first number:%f"), Cubes[0]->nnetwork.NNetwork[0][0][0]));

	if (!player) {
		OurPlayer->UnPossess();
		OurPlayer->Possess(Cubes[0]);
	}
}

// Called every frame
void AController_AI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	delta = DeltaTime;
	sumDelta += DeltaTime;

	if (sumDelta > refresh_frecuency) {
		RefreshCarPosition();
		CheckHit();
		if (player) {
			FTrainingData temp;
			TArray<float> output{ Cubes[population]->RotationPlayer };
			TArray<float> input = Cubes[population]->Input;
			temp.input = input;
			temp.output = output;
			PlayerTrace.Add(temp);
		}
		sumDelta = 0;
	}

}
void AController_AI::Initialize(bool learn)
{
	FVector Location = GetActorLocation();
	this->init_target = OurTrack->CalcNearestPoint(Location);
	FRotator Rotation = GetActorRotation();
	FTransform transform(Rotation, Location);
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = Instigator;


	for (int i = 0; i < population; i++) {
		Cubes.Add(GetWorld()->SpawnActorDeferred<AMyCube>(OurSpawningObject, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	}
	if (player) {
		Cubes.Add(GetWorld()->SpawnActorDeferred<AMyCube>(OurSpawningObject, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
		Cubes[population]->isPossessed = true;
		OurPlayer->UnPossess();
		OurPlayer->Possess(Cubes[population]);
		Cubes[population]->lastTarget = init_target;
		Cubes[population]->InitNet(topology);
		Cubes[population]->OurVisibleActor->SetVisibility(true);
		Cubes[population]->FinishSpawning(transform);
	}

	for (int i = 0; i < population; i++) {
		Cubes[i]->lastTarget = init_target;
		Cubes[i]->InitNet(topology);
		if (i >= show_cubes) {
			Cubes[i]->OurVisibleActor->SetVisibility(false);
		}
		Cubes[i]->FinishSpawning(transform);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("first number:%f"), best.NN[0][0][0]));

}

void AController_AI::ReInitialize()
{
	Probability();
	GeneticAlgorithm();
	for (int i = 0; i < population + player; i++) {
		Cubes[i]->ResetMovement(GetActorTransform(), init_target);
	}
	if (player) {
		MyJsonHandler::Write_DataTraining(PlayerTrace, RelativePath + "../NNdata/training.json");
	}
}

void AController_AI::Probability()
{
	CalcFitness();
	CalcDiversity();
	if (population == 0)
		return;
	Score[0] = (Cubes[0]->fitness + Cubes[0]->diversity);
	for (int i = 1; i < Score.Num(); i++) {
		Score[i] = Score[i - 1] + (Cubes[i]->fitness + Cubes[i]->diversity);
		UE_LOG(LogTemp, Warning, TEXT("scores[%i]: %f + %f => %f"), i, Cubes[i]->fitness, Cubes[i]->diversity, Score[i]);
	}
}

void AController_AI::CalcPosition()
{
	if (Cubes.Num() == 0)
		return;
	int k;
	for (int i = 0; i < show_cubes; i++) {
		for (int j = i + 1; j < population; j++) {
			if (Cubes[position[i]]->percentage < Cubes[position[j]]->percentage) {
				//GEngine->AddOnScreenDebugMessage(-1, delta, FColor::Green, FString::Printf(TEXT("shown %i"), j));
				k = position[i];
				position[i] = position[j];
				position[j] = k;
			}
		}
	}
}

void AController_AI::CheckHit() {
	int Death = 0;
	int finished = 0;
	for (int i = 0; i < population + player; i++) {
		if (Cubes[i]->hit) {
			Death++;
		}
		else {
			if (Cubes[i]->laps > this->laps) {
				finished++;
			}
		}
	}
	if (finished == population + (int)player - Death) {
		ReInitialize();
		GEngine->AddOnScreenDebugMessage(-1, refresh_frecuency, FColor::Green, FString::Printf(TEXT("Winner: #%i"), position[0]));
		GEngine->AddOnScreenDebugMessage(-1, refresh_frecuency, FColor::Green, FString::Printf(TEXT("population: %i || death: %i || finished: %i"), population, Death, finished));

		// TODO winner and position
	}
	if (Death == population + player) {
		ReInitialize();
	}
}

void AController_AI::RefreshCarPosition()
{
	int count = 0;
	for (int i = 0; i < population + player; i++) {
		if (!Cubes[i]->hit) {
			if (OurTrack->UpdatePoint(Cubes[i]->GetActorLocation(), Cubes[i]->lastTarget)) {
				Cubes[i]->laps++;
				if (Cubes[i]->laps > this->laps)
					this->laps++;
			}
			GEngine->AddOnScreenDebugMessage(-1, refresh_frecuency, FColor::Green, FString::Printf(TEXT("Agent #%i target: %i"), i, Cubes[i]->lastTarget));
			Cubes[i]->percentage = Cubes[i]->laps + OurTrack->CalcRectPosition(Cubes[i]->GetActorLocation(), Cubes[i]->lastTarget);// last implementation
			//GEngine->AddOnScreenDebugMessage(-1, refresh_frecuency, FColor::Green, FString::Printf(TEXT("distance: %f / %f"), Cubes[i]->percentage*OurTrack->TotalDistance, OurTrack->TotalDistance));
		}

		// Draw line and show mesh only if the car is between the 6 firsts
		if (i != population || player != 1) {
			if (i < show_cubes) {
				Cubes[position[i]]->OurVisibleActor->SetVisibility(true);
				Cubes[position[i]]->drawLine = true;
				//GEngine->AddOnScreenDebugMessage(-1, delta, FColor::Green, FString::Printf(TEXT("shown %i"), position[count]));
			}
			else {

				Cubes[position[i]]->drawLine = false;
				Cubes[position[i]]->OurVisibleActor->SetVisibility(false);
			}
		}
	}
	CalcPosition();
	if (!player) {
		if (last_best != position[count]) {
			OurPlayer->SetViewTargetWithBlend(Cubes[position[count]], 0.4f);
			OurPlayer->UnPossess();
			OurPlayer->Possess(Cubes[position[count]]);
			last_best = position[count];

			//Cubes[position[count]]->OurCamera->Activate();
			//Cubes[last_best]->StopPossessing();
			//Cubes[position[count]]->StartPossessing();
		}
	}
}

void AController_AI::CalcDiversity()
{
	this->TotalDiversity = 0;
	for (int i = 0; i < population; i++)
		Cubes[i]->diversity = 0;

	float t = 0;
	for (int i = 0; i < population; i++) {
		for (int j = i + 1; j < population; j++) {
			for (int x = 0; x < topology.Num() - 1; x++) {
				for (int y = 0; y < topology[x]; y++) {
					for (int z = 0; z < topology[x + 1]; z++) {
						float diversity = abs(Cubes[j]->nnetwork.NNetwork[x][y][z] - Cubes[i]->nnetwork.NNetwork[x][y][z]);
						Cubes[i]->diversity += diversity;
						Cubes[j]->diversity += diversity;
					}
				}
			}
		}
		if (Cubes[i]->percentage == 0) // Remove the ones that does not reach the deadline
			Cubes[i]->diversity = 0;
		TotalDiversity += Cubes[i]->diversity;
	}
	TotalDiversity = 1 / TotalDiversity * importance_diversity;
	for (int i = 0; i < population; i++) {
		Cubes[i]->diversity *= TotalDiversity;
	}
}

void AController_AI::CalcFitness()
{
	RefreshCarPosition();
	this->TotalFitness = 0;
	float avarage = 0;
	for (int i = 0; i < population; i++) {
		avarage += Cubes[i]->percentage;
	}
	avarage = avarage * deadline / population;
	for (int i = 0; i < population; i++) {
		if (Cubes[i]->percentage < avarage)// Remove the ones which are the worst
			Cubes[i]->percentage = 0;
		TotalFitness += Cubes[i]->percentage;
	}
	TotalFitness = 1 / TotalFitness * (1 - importance_diversity);
	for (int i = 0; i < population; i++) {
		Cubes[i]->fitness = Cubes[i]->percentage * TotalFitness;
	}
}

void AController_AI::GeneticAlgorithm()
{
	if (selections.Num() == 0)
		return;
	GA_Selection();
	GA_Crossover();
	GA_Mutation();
	UE_LOG(LogTemp, Warning, TEXT("first number:%f"), Cubes[0]->nnetwork.NNetwork[0][0][0]);
}

void AController_AI::GA_Selection()
{
	if (population == 0)
		return;
	// Select the best
	int k = 0;
	for (int j = 1; j < population; j++) {
		if (Cubes[position[0]]->percentage < Cubes[position[j]]->percentage) {
			//GEngine->AddOnScreenDebugMessage(-1, delta, FColor::Green, FString::Printf(TEXT("shown %i"), j));
			k = position[0];
			position[0] = position[j];
			position[j] = k;
		}
	}

	if (position[0] == population) {
		selections[0].NNetwork = Cubes[position[1]]->nnetwork.NNetwork;
		UE_LOG(LogTemp, Warning, TEXT("car_selected__:%i"), position[1]);
		MyJsonHandler::Write_Network(Cubes[position[1]]->nnetwork, RelativePath + "../NNdata/test.json");
	}
	else {
		selections[0].NNetwork = Cubes[position[0]]->nnetwork.NNetwork;
		UE_LOG(LogTemp, Warning, TEXT("car_selected:%i"), position[0]);
		MyJsonHandler::Write_Network(Cubes[position[0]]->nnetwork, RelativePath + "../NNdata/test.json");
	}

	float n;
	for (int i = 1; i < selections.Num(); i++) {
		n = FMath::FRand();
		for (int j = 0; j < Score.Num(); j++) {
			if (n < Score[j]) {
				selections[i].NNetwork = Cubes[j]->nnetwork.NNetwork;
				break;
			}
		}
	}

	Cubes[0]->nnetwork.NNetwork = selections[0].NNetwork;
	for (int i = 1; i < selections.Num() * 2; i++) {
		Cubes[i]->nnetwork.NNetwork = selections[i%selections.Num()].NNetwork;
	}
}

void AController_AI::GA_Crossover()
{
	if (!crossover)
		return;

	int a, b, c;
	for (int i = selections.Num() * 2; i < population; i++) {
		a = FMath::RandRange(0, selections.Num() - 1);
		b = FMath::RandRange(0, selections.Num() - 1);
		c = FMath::RandRange(0, population - 1);

		for (int x = 0; x < topology.Num() - 1; x++) {
			for (int y = 0; y < topology[x]; y++) {
				for (int z = 0; z < topology[x + 1]; z++) {
					if (FMath::FRand() > crossover_rate) {
						Cubes[i]->nnetwork.NNetwork[x][y][z] = selections[a].NNetwork[x][y][z];
					}
					else {
						Cubes[i]->nnetwork.NNetwork[x][y][z] = selections[b].NNetwork[x][y][z];
					}
				}
			}
		}
	}
}

void AController_AI::GA_Mutation()
{
	for (int i = selections.Num(); i < population; i++) {
 		for (auto& a : Cubes[i]->nnetwork.NNetwork) {
			for (auto&b : a) {
				for (auto&c : b) {
					if (FMath::FRand() < mutation_rate)
						c += (FMath::FRand() * 2 - 1)*mutation_change;
				}
			}
		}
	}
}


