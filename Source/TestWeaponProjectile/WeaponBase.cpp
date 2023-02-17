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
	this->SetReplicates(true);
	this->SetReplicateMovement(true);
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

void AWeaponBase::Fire(FVector LocationTrace, FVector ForwardVectorTrace, bool isAimFirstView)
{
	if (ShootLocation)
	{
		FVector SpawnLocation = ShootLocation->GetComponentLocation();
		FRotator SpawnRotation = ShootLocation->GetComponentRotation();

		if (ProjectileInfo.Projectile)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = GetInstigator();

			AProjectileBase* myProjectile = Cast<AProjectileBase>(GetWorld()->SpawnActor(ProjectileInfo.Projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
			if (myProjectile)
			{
				myProjectile->SetLifeSpan(WeaponSetting.ProjectileLifeTime);
				myProjectile->ProjectileDamage = WeaponSetting.ProjectileDamage;

				FHitResult OutHit;
				FPredictProjectilePathParams PathParams;

				FCollisionQueryParams CollisionParams;
				CollisionParams.AddIgnoredActor(this);
				CollisionParams.AddIgnoredActor(GetAttachParentActor());
				CollisionParams.AddIgnoredActor(myProjectile);

				FVector TraceLocation = LocationTrace;
				FVector TraceForwardVector = ForwardVectorTrace;

				if (!isAimFirstView)
				{
					//Camera shoot
					UE_LOG(LogTemp, Warning, TEXT("Camera shoot"));
					DrawDebugLine(GetWorld(), TraceLocation, (TraceForwardVector * 100 * WeaponSetting.ProjectileSpeed) + TraceLocation, FColor(255, 255, 0), false, 3.0f, 0, 1);

					if (GetWorld()->LineTraceSingleByChannel(OutHit, TraceLocation, (TraceForwardVector * 100 * WeaponSetting.ProjectileSpeed) + TraceLocation, ECC_Visibility, CollisionParams))
					{
						DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 10.0f, 10, FColor(30, 30, 30), false, 3.0f, 0, 1);
						PathParams.LaunchVelocity = UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - ShootLocation->GetComponentLocation()).Rotation()) * WeaponSetting.ProjectileSpeed;
						//UE_LOG(LogTemp, Warning, TEXT("SimFrequency  %f"), FMath::RoundHalfFromZero(((OutHit.ImpactPoint - TraceLocation).Size()) / 500));
						PathParams.SimFrequency = FMath::RoundHalfFromZero(((OutHit.ImpactPoint - TraceLocation).Size()) / 500);
					}
					else
					{
						PathParams.LaunchVelocity = TraceForwardVector * WeaponSetting.ProjectileSpeed;
						PathParams.SimFrequency = 10.0f;
					}
				}
				else
				{
					//Aim shoot
					UE_LOG(LogTemp, Warning, TEXT("Aim shoot"));
					TraceLocation = AimLocation->GetComponentLocation();
					TraceForwardVector = UKismetMathLibrary::GetForwardVector(AimLocation->GetComponentRotation());
					DrawDebugLine(GetWorld(), TraceLocation, (TraceForwardVector * 100 * WeaponSetting.ProjectileSpeed) + AimLocation->GetComponentLocation(), FColor(255, 255, 0), false, 3.0f, 0, 1);

					if (GetWorld()->LineTraceSingleByChannel(OutHit, TraceLocation, TraceForwardVector * 100 * WeaponSetting.ProjectileSpeed + TraceLocation, ECC_Visibility, CollisionParams))
					{
						PathParams.LaunchVelocity = UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - TraceLocation).Rotation()) * WeaponSetting.ProjectileSpeed;
						//UE_LOG(LogTemp, Warning, TEXT("SimFrequency  %f"), FMath::RoundHalfFromZero(((OutHit.ImpactPoint - TraceLocation).Size()) / 500));
						PathParams.SimFrequency = FMath::RoundHalfFromZero(((OutHit.ImpactPoint - TraceLocation).Size()) / 500);
					}
					else
					{
						PathParams.LaunchVelocity = TraceForwardVector * WeaponSetting.ProjectileSpeed;
						PathParams.SimFrequency = 10.0f;
					}
				}

				PathParams.StartLocation = ShootLocation->GetComponentLocation();
				PathParams.bTraceWithCollision = true;
				PathParams.ProjectileRadius = 10.0f;
				PathParams.TraceChannel = ECC_Visibility;
				//PathParams.ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
				//PathParams.ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery2);
				//PathParams.ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);
				PathParams.bTraceComplex = true;
				PathParams.bTraceWithChannel = true;
				PathParams.DrawDebugType = EDrawDebugTrace::None;
				PathParams.DrawDebugTime = 0.0f;
				PathParams.MaxSimTime = 2.0f;
				PathParams.OverrideGravityZ = WeaponSetting.ProjectileMass;
				PathParams.ActorsToIgnore.Add(this);
				PathParams.ActorsToIgnore.Add(GetAttachParentActor());
				PathParams.ActorsToIgnore.Add(myProjectile);

				FPredictProjectilePathResult PathResult;
				UGameplayStatics::PredictProjectilePath(GetWorld(), PathParams, PathResult);
				UE_LOG(LogTemp, Warning, TEXT("OutHit.Distance   %f"), OutHit.Distance);
				myProjectile->InitProjectile(ProjectileInfo, PathResult.PathData, UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - TraceLocation).Rotation()), (OutHit.Distance < 600 && OutHit.Distance > 0));
			}
		}
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundFireWeapon, ShootLocation->GetComponentLocation());
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSetting.EffectFireWeapon, ShootLocation->GetComponentTransform());
	}
}