// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "MyGameInstance.h"
#include "Net/UnrealNetWork.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = SceneComponent;

	StaticMeshWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh "));
	StaticMeshWeapon->SetGenerateOverlapEvents(false);
	StaticMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMeshWeapon->SetupAttachment(RootComponent);
	//RootComponent = StaticMeshWeapon;

	ShootLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	ShootLocation->SetupAttachment(StaticMeshWeapon);
	AimLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("AimLocation"));
	AimLocation->SetupAttachment(StaticMeshWeapon);
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::WeaponInit()
{
	StaticMeshWeapon->SetStaticMesh(WeaponSetting.StaticMeshWeapon);
	StaticMeshWeapon->SetRelativeTransform(WeaponSetting.WeaponStaticMeshOffset);
	ShootLocation->SetRelativeTransform(WeaponSetting.ShootLocationOffset);
	AimLocation->SetRelativeTransform(WeaponSetting.AimLocationOffset);
}

void AWeaponBase::Fire(AProjectileBase* myProjectile, TArray <FPredictProjectilePathPointData> PathData, FVector FireVector, bool CloseHit)
{
	myProjectile->InitProjectile(ProjectileInfo, PathData, FireVector, CloseHit);
}