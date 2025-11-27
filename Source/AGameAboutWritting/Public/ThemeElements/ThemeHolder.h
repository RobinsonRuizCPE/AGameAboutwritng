// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractableActor/ActorInteractable.h"
#include "../Items/ItemTheme.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ThemeHolder.generated.h"

UCLASS()
class AGAMEABOUTWRITTING_API AThemeHolder : public AActorInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThemeHolder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	/** Arrow as root */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UArrowComponent* ArrowRoot;

	/** Static mesh attached to root */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
		AItemTheme* AttachedItemThem;

};
