// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvQueryGenerator_BlueprintBase.generated.h"

struct FEnvQueryInstance;
class AActor;
class UEnvQueryContext;
class UEnvQueryItemType;

UCLASS(Abstract, Blueprintable)
class AIMODULE_API UEnvQueryGenerator_BlueprintBase : public UEnvQueryGenerator
{
	GENERATED_UCLASS_BODY()

	/** max distance of between point and context */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FEnvFloatParam Radius; 

	/** A short description of what test does, like "Generate pawn named Joe" */
	UPROPERTY(EditAnywhere, Category = Generator)
	FText GeneratorsActionDescription;

	/** context */
	UPROPERTY(EditAnywhere, Category = Generator)
	TSubclassOf<UEnvQueryContext> Context;

	UPROPERTY(EditDefaultsOnly, Category = Generator)
	TSubclassOf<UEnvQueryItemType> GeneratedItemType;

	virtual void PostInitProperties() override;
	virtual UWorld* GetWorld() const;

	UFUNCTION(BlueprintImplementableEvent, Category = Generator)
	virtual void DoItemGeneration(const TArray<FVector>& ContextLocations);

	void GenerateItems(FEnvQueryInstance& QueryInstance); 

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
	
	UFUNCTION(BlueprintCallable, Category = "EQS")
	void AddGeneratedVector(FVector GeneratedVector);

	UFUNCTION(BlueprintCallable, Category = "EQS")
	void AddGeneratedActor(AActor* GeneratedActor);

	UFUNCTION(BlueprintCallable, Category = "EQS")
	UObject* GetQuerier() const;

private:
	/** this is valid and set only within GenerateItems call */
	FEnvQueryInstance* CachedQueryInstance;
};
