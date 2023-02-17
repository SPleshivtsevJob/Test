// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Type.h"
#include "ProjectileBase.h"
#include "WeaponBase.generated.h"

UCLASS()
class TESTWEAPONPROJECTILE_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class USceneComponent* SceneComponent = nullptr;
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UStaticMeshComponent* StaticMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UArrowComponent* ShootLocation = nullptr;
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UArrowComponent* AimLocation = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FWeaponInfo WeaponSetting;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FProjectileInfo ProjectileInfo;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void WeaponInit();
	void Fire(FVector LocationTrace, FVector ForwardVectorTrace, bool isAimFirstView);
};
