// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Type.h"
#include "WeaponBase.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TESTWEAPONPROJECTILE_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	//table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	UDataTable* WeaponInfoTable = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	UDataTable* ProjectileInfoTable = nullptr;

	UFUNCTION(BlueprintCallable)
	bool GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo);
	UFUNCTION(BlueprintCallable)
	bool GetProjectileInfoByName(FName NameProjectile, FProjectileInfo& OutInfo);
};
