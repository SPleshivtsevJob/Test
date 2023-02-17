// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	BulletCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	BulletCollisionSphere->SetSphereRadius(10.f);
	BulletCollisionSphere->bReturnMaterialOnMove = true;//hit event return physMaterial
	BulletCollisionSphere->SetCanEverAffectNavigation(false);//collision not affect navigation (P keybord on editor)

	RootComponent = BulletCollisionSphere;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Projectile Mesh"));
	BulletMesh->SetupAttachment(RootComponent);
	BulletMesh->SetCanEverAffectNavigation(false);
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileBase::ImpactProjectile()
{
	this->Destroy();
}

void AProjectileBase::InitProjectile(FProjectileInfo InitParam, TArray <FPredictProjectilePathPointData> PathData, FVector FireVector, bool CloseHit)
{
	PathDataMove = PathData;
	ProjectileSetting = InitParam;
	//BulletMesh->SetStaticMesh(InitParam.ProjectileStaticMesh);
	//BulletMesh->SetRelativeTransform(ProjectileSetting.ProjectileStaticMeshOffset);
	InitVisualMeshProjectile_Multicast(InitParam.ProjectileStaticMesh, ProjectileSetting.ProjectileStaticMeshOffset);
	
	BulletCollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileBase::BulletCollisionSphereHit);

	FireDirVector = FireVector;
	StartLocation = GetActorLocation();
	NumArray = PathData.Num();
	CurrentNumArray = 0;
	CloseHitProjectile = CloseHit;
	ProjectileFlight();
}

void AProjectileBase::ProjectileFlight()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ProjectileFlightTimer, this, &AProjectileBase::ProjectileMove, 0.05f, true);
}

void AProjectileBase::ProjectileMove()
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	if (!CloseHitProjectile)
	{
			if (CurrentNumArray < NumArray )
			{
				DrawDebugLine(GetWorld(), RootComponent->GetComponentLocation(), PathDataMove[CurrentNumArray].Location, FColor(255.0f, 100.0f, 100.0f), false, 3.0f, 0, 1);
				
				UE_LOG(LogTemp, Warning, TEXT("CurrentNumArray  %d"), CurrentNumArray);
				UKismetSystemLibrary::MoveComponentTo(RootComponent, PathDataMove[CurrentNumArray].Location, PathDataMove[CurrentNumArray].Location.Rotation(), false, false, 0.05f, true, EMoveComponentAction::Move, LatentInfo);
				CurrentNumArray++;
			}
			else
			{
				if (!((CurrentNumArray - 1) < 0))
				{
					UKismetSystemLibrary::MoveComponentTo(RootComponent, GetActorLocation() + (UKismetMathLibrary::GetForwardVector((PathDataMove[CurrentNumArray - 1].Location - StartLocation).Rotation()) * 100), PathDataMove[CurrentNumArray - 1].Location.Rotation(), false, false, 0.05f, false, EMoveComponentAction::Type::Move, LatentInfo);
				}
			}
	}
	else 
	{
		DrawDebugLine(GetWorld(), RootComponent->GetComponentLocation(), RootComponent->GetComponentLocation() + FireDirVector * 200, FColor(255.0f, 100.0f, 100.0f), false, 3.0f, 0, 1);
		UKismetSystemLibrary::MoveComponentTo(RootComponent, GetActorLocation() + FireDirVector * 200, GetActorRotation(), false, false, 0.05f, true, EMoveComponentAction::Move, LatentInfo);
	}
}

void AProjectileBase::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ProjectileFlightTimer);
	if (ProjectileSetting.HitDecal)
	{
		UMaterialInterface* myMaterial = ProjectileSetting.HitDecal;

		if (myMaterial && OtherComp)
		{
			SpawnHitDecal_Multicast(myMaterial, OtherComp, Hit);
			//UGameplayStatics::SpawnDecalAttached(myMaterial, FVector(20.0f), OtherComp, NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 10.0f);
		}
	}
	if (ProjectileSetting.HitFX)
	{
		UParticleSystem* myParticle = ProjectileSetting.HitFX;
		if (myParticle)
		{
			SpawnHitFx_Multicast(myParticle, Hit);
			//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), myParticle, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint, FVector(1.0f)));
		}
	}
	if (ProjectileSetting.HitSound)
	{
		SpawnHitSound_Multicast(ProjectileSetting.HitSound, Hit);
		//UGameplayStatics::PlaySoundAtLocation(GetWorld(), ProjectileSetting.HitSound, Hit.ImpactPoint);
	}

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ProjectileFlightTimer);
	UGameplayStatics::ApplyDamage(OtherActor, ProjectileDamage, GetInstigatorController(), this, NULL);
	
	ImpactProjectile();
}



void AProjectileBase::InitVisualMeshProjectile_Multicast_Implementation(UStaticMesh* newMesh, FTransform MeshRelative)
{
	BulletMesh->SetStaticMesh(newMesh);
	BulletMesh->SetRelativeTransform(MeshRelative);
}
void AProjectileBase::SpawnHitDecal_Multicast_Implementation(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult HitResult)
{
	UGameplayStatics::SpawnDecalAttached(DecalMaterial, FVector(20.0f), OtherComp, NAME_None, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 10.0f);
}

void AProjectileBase::SpawnHitFx_Multicast_Implementation(UParticleSystem* FxTemplate, FHitResult HitResult)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FxTemplate, FTransform(HitResult.ImpactNormal.Rotation(), HitResult.ImpactPoint, FVector(0.5f)));
}

void AProjectileBase::SpawnHitSound_Multicast_Implementation(USoundBase* HitSound, FHitResult HitResult)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, HitResult.ImpactPoint);
}