// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "Type.generated.h"

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	TSubclassOf<class AWeaponBase> WeaponClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	UStaticMesh* StaticMeshWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	FTransform WeaponStaticMeshOffset = FTransform();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	FTransform ShootLocationOffset = FTransform();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	FTransform AimLocationOffset = FTransform();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	float RateOfFire = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	float ReloadTime = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	int32 MaxRound = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	float Dispersion = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	float RateOfDecreaseDispersion = 0.5f;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponFX")
	USoundBase* SoundFireWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponFX")
	USoundBase* SoundReloadWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponFX")
	UParticleSystem* EffectFireWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponDropMesh")
		UStaticMesh* ShellBulletStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponDropMesh")
		UStaticMesh* AmmoMagazineStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	FName TypeProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileDamage = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileLifeTime = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileSpeed = 2000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileMass = 1.0f;
};

USTRUCT(BlueprintType)
struct FProjectileInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		TSubclassOf<class AProjectileBase> Projectile = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		UStaticMesh* ProjectileStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		FTransform ProjectileStaticMeshOffset = FTransform();

	//material to decal on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		UMaterialInterface* HitDecal;
	//Sound when hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		USoundBase* HitSound = nullptr;
	//fx when hit surface
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		UParticleSystem* HitFX;
};