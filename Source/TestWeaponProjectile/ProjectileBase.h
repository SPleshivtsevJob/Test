// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Type.h"
#include "ProjectileBase.generated.h"

UCLASS()
class TESTWEAPONPROJECTILE_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UStaticMeshComponent* BulletMesh = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class USphereComponent* BulletCollisionSphere = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FProjectileInfo ProjectileSetting;

	float ProjectileDamage = 0.0f;
	bool bHit;
	int8 NumArray;
	int8 CurrentNumArray;
	TArray <FPredictProjectilePathPointData> PathDataMove;
	FVector StartLocation;
	FVector FireDirVector;
	bool CloseHitProjectile = true;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	FTimerHandle TimerHandle_ProjectileFlightTimer;
	UFUNCTION()
	virtual void ImpactProjectile();
	UFUNCTION()
		virtual void BulletCollisionSphereHit(class UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	void InitProjectile(FProjectileInfo InitParam, TArray<FPredictProjectilePathPointData> PathData, FVector FireVector, bool CloseHit);

	void ProjectileFlight();
	void ProjectileMove();

	UFUNCTION(NetMulticast, Reliable)
		void InitVisualMeshProjectile_Multicast(UStaticMesh* newMesh, FTransform MeshRelative);
	UFUNCTION(NetMulticast, Reliable)
		void SpawnHitDecal_Multicast(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult HitResult);
	UFUNCTION(NetMulticast, Reliable)
		void SpawnHitFx_Multicast(UParticleSystem* FxTemplate, FHitResult HitResult);
	UFUNCTION(NetMulticast, Reliable)
		void SpawnHitSound_Multicast(USoundBase* HitSound, FHitResult HitResult);
};
