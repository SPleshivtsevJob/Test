// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

bool UMyGameInstance::GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo)
{
	bool bIsFind = false;
	FWeaponInfo* WeaponInfoRow;

	if (WeaponInfoTable)
	{
		WeaponInfoRow = WeaponInfoTable->FindRow<FWeaponInfo>(NameWeapon, "", false);
		if (WeaponInfoRow)
		{
			bIsFind = true;
			OutInfo = *WeaponInfoRow;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UTPSGameInstance::GetWeaponInfoByName - WeaponTable -NULL"));
	}
	return bIsFind;
}

bool UMyGameInstance::GetProjectileInfoByName(FName NameProjectile, FProjectileInfo& OutInfo)
{
	bool bIsFind = false;
	FProjectileInfo* ProjectileInfoRow;

	if (ProjectileInfoTable)
	{
		ProjectileInfoRow = ProjectileInfoTable->FindRow<FProjectileInfo>(NameProjectile, "", false);
		if (ProjectileInfoRow)
		{
			bIsFind = true;
			OutInfo = *ProjectileInfoRow;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UTPSGameInstance::GetProjectileInfoByName - ProjectileInfoTable -NULL"));
	}
	return bIsFind;
}
