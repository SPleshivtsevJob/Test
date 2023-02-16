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
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UArrowComponent* ShootArrow = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UArrowComponent* ArrowLeft = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UArrowComponent* ArrowRight = nullptr;

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

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ChangePosCameraLeft;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ChangePosCameraRight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ChangeCameraView;*/

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
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponName;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool isAimFirstView = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool FreeCamera = false;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool isAimThirdView = false;

	UPROPERTY(Replicated)
	FVector TraceLocation;
	UPROPERTY(Replicated)
	FVector TraceForwardVector;
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

	void InitWeapon(FName IdWeaponName);

	UFUNCTION()
		void InputAttackPressed(const FInputActionValue& Value);
	UFUNCTION()
		void InputAttackReleased(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
		void UpdateTraceLocAndRot_OnServer(FVector Location, FRotator Rotation);
	UFUNCTION(NetMulticast, Reliable)
		void UpdateTraceLocAndRot_Multicast(FVector Location, FRotator Rotation);

	UFUNCTION(Server, Reliable)
	void MoveCamera_OnServer(ATestWeaponProjectileCharacter* Char, UCameraComponent* Camera, FVector Location);
	UFUNCTION(NetMulticast, Reliable)
	void MoveCamera_Multicast(ATestWeaponProjectileCharacter* Char, UCameraComponent* Camera, FVector Location);


	//UFUNCTION(Server, Reliable)
		//void SetLocRotCamera_OnServer(ATestWeaponProjectileCharacter* Char, UCameraComponent* Camera, FVector Location);
	//UFUNCTION(NetMulticast, Reliable)
		//void SetLocRotCamera_Multicast(ATestWeaponProjectileCharacter* Char, UCameraComponent* Camera, FVector Location);

	UFUNCTION(Server, Reliable)
		void UpdateAimCameraView_OnServer(bool bAimCameraView);
	/*UFUNCTION()
		void ChangePosCameraLeftPressed(const FInputActionValue& Value);
	UFUNCTION()
		void ChangePosCameraLeftReleased(const FInputActionValue& Value);

	UFUNCTION()
		void ChangePosCameraRightPressed(const FInputActionValue& Value);
	UFUNCTION()
		void ChangePosCameraRightReleased(const FInputActionValue& Value);

	UFUNCTION()
		void ChangeCameraViewPressed(const FInputActionValue& Value);*/


	UFUNCTION()
	void AimFirstViewPressed(const FInputActionValue& Value);
	UFUNCTION()
	void AimThirdViewPressed(const FInputActionValue& Value);

	UFUNCTION()
	void GetFreeCameraPressed(const FInputActionValue& Value);
	UFUNCTION()
	void GetFreeCameraReleased(const FInputActionValue& Value);




	UFUNCTION(Server, Reliable)
		void AttackCharEvent_OnServer(bool bIsFiring);


	FVector GetFireEndLocation()const;
	UPROPERTY(Replicated)
	AWeaponBase* CurrentWeapon = nullptr;
	UFUNCTION(BlueprintCallable)
	AWeaponBase* GetCurrentWeapon();

	FTimerHandle TimerHandle_WaitCameraAfterFireTimer;

	//void MoveCameraAfterFireTimer();
	void ChangePosCameraPlayer();
	void ChangePosCamera(bool Right);
	void TraceLeftRight();

	void SetWeaponStateFire(bool bIsFire);

	UFUNCTION(Server, Reliable)
	void Fire_Server();
	void Fire();
};

