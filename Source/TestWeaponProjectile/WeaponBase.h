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
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UArrowComponent* ShootLocation = nullptr;
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UArrowComponent* AimLocation = nullptr;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FWeaponInfo WeaponSetting;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FProjectileInfo ProjectileInfo;
	FVector CurrentDispersion = FVector(0.0f, 0.0f, 0.0f);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	FTimerHandle TimerHandle_FireRateTimer;
	FTimerHandle TimerHandle_RateDecreaseDispersionTimer;

	//true = trace camera, false = trace shoot location on weapon
	bool* TraceCamera;
	UArrowComponent** ShootLocationCharacterCamera;

	
	void DispersionWeapon();
	void DecreaseDispersion();
	void WeaponInit();
	void Fire();



	void Fire2(AProjectileBase* myProjectile, TArray <FPredictProjectilePathPointData> PathData, FVector FireVector, bool CloseHit);
	void SetWeaponStateFire(bool bIsFire);
};
