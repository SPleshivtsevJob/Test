// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Type.h"
#include "WeaponBase.h"
#include "TestWeaponProjectileCharacter.generated.h"


UCLASS(config=Game)
class ATestWeaponProjectileCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UArrowComponent* ShootArrow = nullptr;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AimFirstView;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AimThirdView;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* GetFreeCamera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

public:
	ATestWeaponProjectileCharacter();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponName;

	UPROPERTY(BlueprintReadOnly)
	bool isAimFirstView = false;
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool isFreeCamera = false;
	UPROPERTY(BlueprintReadOnly)
	bool isAimThirdView = false;

	UPROPERTY()
	FVector TraceLocation;
	UPROPERTY()
	FVector TraceForwardVector;
	UPROPERTY(Replicated)
	AWeaponBase* CurrentWeapon = nullptr;
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	/** Called for looking input */
	void Look(const FInputActionValue& Value);	

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	FTimerHandle TimerHandle_FireRateTimer;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION()
	void InputAttackPressed(const FInputActionValue& Value);
	UFUNCTION()
	void InputAttackReleased(const FInputActionValue& Value);
	UFUNCTION()
	void AimFirstViewPressed(const FInputActionValue& Value);
	UFUNCTION()
	void AimThirdViewPressed(const FInputActionValue& Value);
	UFUNCTION()
	void GetFreeCameraPressed(const FInputActionValue& Value);
	UFUNCTION()
	void GetFreeCameraReleased(const FInputActionValue& Value);


	UFUNCTION(Server, Reliable)
	void SetAimFirstView_OnServer(bool bIsAimFirst);
	UFUNCTION(Server, Reliable)
	void SetAimThirdView_OnServer(bool bIsAimThird);
	UFUNCTION(Server, Reliable)
	void SetIsFreeCamera_OnServer(bool bisFree);
	
	UFUNCTION()
	void MoveCamera(ATestWeaponProjectileCharacter* Char, UCameraComponent* Camera, FVector Location);

	
	UFUNCTION()
	void InitWeapon(FName IdWeaponName);

	
	UFUNCTION(BlueprintCallable)
	AWeaponBase* GetCurrentWeapon();
	UFUNCTION(Server, Reliable)
	void AttackCharEvent(bool bIsFiring);
	void SetWeaponStateFire(bool bIsFire);
	void Fire();
};

