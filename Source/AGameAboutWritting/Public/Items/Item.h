// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../InteractableActor/ActorInteractable.h"
#include "Engine/EngineTypes.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"

#include "RealtimeMeshComponent.h"
#include "RealtimeMeshSimple.h"

#include "Item.generated.h"

USTRUCT(BlueprintType)
struct FHandTransforms
{
	GENERATED_BODY()

public:
	FHandTransforms() : LeftHandTransform(), RightHandrelativeTransform() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FTransform LeftHandTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FTransform RightHandrelativeTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool use_left_hand = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool use_right_hand = false;
};

UCLASS(Blueprintable)
class AGAMEABOUTWRITTING_API AItem : public AActorInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon")
		FHandTransforms GetHandsSockets() const;

	virtual FHandTransforms GetHandsSockets_Implementation() const;


public:
	/** === Class (static) Getters === */
	UFUNCTION(BlueprintCallable, Category = "Item|Class Accessors", meta = (DisplayName = "Get Item Display Name (Class)"))
		static FText GetItemDisplayNameFromClass(TSubclassOf<AItem> ItemClass);

	UFUNCTION(BlueprintCallable, Category = "Item|Class Accessors", meta = (DisplayName = "Get Item Price (Class)"))
		static int32 GetItemPriceFromClass(TSubclassOf<AItem> ItemClass);

	UFUNCTION(BlueprintCallable, Category = "Item|Class Accessors", meta = (DisplayName = "Get Item Mesh (Class)"))
		static UStaticMesh* GetItemMeshFromClass(TSubclassOf<AItem> ItemClass);

	/** === Instance Getters === */
	UFUNCTION(BlueprintCallable, Category = "Item|Accessors")
		UStaticMesh* GetMeshReference() const {
		return ItemStaticMesh ? ItemStaticMesh->GetStaticMesh() : nullptr;
	};

	UFUNCTION(BlueprintCallable, Category = "Item|Accessors")
		FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintCallable, Category = "Item|Accessors")
		int32 GetPrice() const { return Price; }

	virtual void OnHitByPlayerLaser_Implementation() override;
	virtual void OnNoLongerHitByPlayerLaser_Implementation() override;
	virtual bool Interact_Implementation(AActor* ActorThatInteract) override;
	virtual void StopInteract_Implementation(AActor* ActorStopingInteract) override;
	virtual void UseObject_Implementation(FVector UseDirection, FVector UsePosition, AActor* ActorUsingObject) override;

	UFUNCTION(BlueprintCallable, Category = "Item|Accessors")
		void SetupItemAttachment();

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USceneComponent> AttachPivot;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USceneComponent> AttachOffset;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UArrowComponent> Arrow;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USphereComponent> Sphere;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		TObjectPtr<UStaticMeshComponent> ItemStaticMesh;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		bool IsHeld;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		bool IsOneHanded = false;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		TObjectPtr<APlayerCharacter> PlayerCharacter;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		FHitResult HitInfo;

	/** The display name (can be localized) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Data")
		FText DisplayName;

	/** The price in whatever in-game currency you use */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Data", meta = (ClampMin = "0"))
		int32 Price;

	UPROPERTY()
		mutable bool bHandTransformsCached = false;

	UPROPERTY()
		mutable FHandTransforms CachedHandTransformsLocal;
};

