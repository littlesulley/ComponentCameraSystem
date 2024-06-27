// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <vector>
#include "Containers/ContainerAllocationPolicies.h"
#include "ECameraDamper.generated.h"

/**
 * An aggregate damper for an **FVector** input. Consists of three separate sub-dampers for each value.
 * When a damper is used in any component (follow or aim), the following typical steps should be followed:
 *   1) Call `SetInput`, set the value to damp.
 *   2) Call `ApplyDamp`, damp the input value and return the result.
 *   3) Call `SetOutput` (optional), set the damped result from step 2) to a new value, usually updated by the component.
 *   4) Call `PostApplyDamp` (optional), update each sub-damper's internal variables.
 */
UCLASS(DefaultToInstanced, EditInlineNew, Blueprintable, BlueprintType, CollapseCategories, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraDamperVector : public UObject
{
	GENERATED_BODY()

public:	
	UECameraDamperVector();

	UFUNCTION(BlueprintCallable, Category = "CameraDamperVector")
	void SetInput(const FVector& _Input);

	UFUNCTION(BlueprintCallable, Category = "CameraDamperVector")
	void SetOutput(const FVector& _Output);

	UFUNCTION(BlueprintCallable, Category = "CameraDamperVector")
	FVector ApplyDamp(const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "CameraDamperVector")
	void PostApplyDamp();

protected:
	/* Input to damp at this frame. */
	UPROPERTY(BlueprintReadWrite, Category = "CameraDamperVector")
	FVector Input;    

	/* Output after damp at this frame. */
	UPROPERTY(BlueprintReadWrite, Category = "CameraDamperVector")
	FVector Output;

	/** First sub-damper. May be used to damp X component. */
	UPROPERTY(Instanced, BlueprintReadOnly, EditAnywhere, Category = "CameraDamperVector")
	TObjectPtr<UECameraDamper> DamperX;

	/** Second sub-damper. May be used to damp Y component. */
	UPROPERTY(Instanced, BlueprintReadOnly, EditAnywhere, Category = "CameraDamperVector")
	TObjectPtr<UECameraDamper> DamperY;

	/** Third sub-damper. May be used to damp Z component. */
	UPROPERTY(Instanced, BlueprintReadOnly, EditAnywhere, Category = "CameraDamperVector")
	TObjectPtr<UECameraDamper> DamperZ;
};

/**
 * An aggregate damper for a **Rotator** input. Consists of three separate sub-dampers for each value.
 * When a damper is used in any component (follow or aim), the following typical steps should be followed:
 *   1) Call `SetInput`, set the value to damp.
 *   2) Call `ApplyDamp`, damp the input value and return the result.
 *   3) Call `SetOutput` (optional), set the damped result from step 2) to a new value, usually updated by the component.
 *   4) Call `PostApplyDamp` (optional), update each sub-damper's internal variables.
 */
UCLASS(DefaultToInstanced, EditInlineNew, Blueprintable, BlueprintType, CollapseCategories, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraDamperRotator : public UObject
{
	GENERATED_BODY()

public:
	UECameraDamperRotator();

	UFUNCTION(BlueprintCallable, Category = "CameraDamperRotator")
	void SetInput(const FRotator& _Input);

	UFUNCTION(BlueprintCallable, Category = "CameraDamperRotator")
	void SetOutput(const FRotator& _Output);

	UFUNCTION(BlueprintCallable, Category = "CameraDamperRotator")
	FRotator ApplyDamp(const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "CameraDamperRotator")
	void PostApplyDamp();

protected:
	/* Input to damp at this frame. */
	UPROPERTY(BlueprintReadWrite, Category = "CameraDamperRotator")
	FRotator Input;

	/* Output after damp at this frame. */
	UPROPERTY(BlueprintReadWrite, Category = "CameraDamperRotator")
	FRotator Output;

	/** First sub-damper. May be used to damp roll. */
	UPROPERTY(Instanced, BlueprintReadOnly, EditAnywhere, Category = "CameraDamperRotator")
	TObjectPtr<UECameraDamper> DamperR;

	/** Second sub-damper. May be used to damp pitch. */
	UPROPERTY(Instanced, BlueprintReadOnly, EditAnywhere, Category = "CameraDamperRotator")
	TObjectPtr<UECameraDamper> DamperP;

	/** Third sub-damper. May be used to damp yaw. */
	UPROPERTY(Instanced, BlueprintReadOnly, EditAnywhere, Category = "CameraDamperRotator")
	TObjectPtr<UECameraDamper> DamperY;
};

/**
 * Abstract class representing a damper for a single **float** input.
 * To customize your own damper class, use the following two steps:
 *   1) Implement `ApplyDamp`, which defines the actual damping logic.
 *   2) Implememt `PostApplyDamp`, which provides a damper-specific way to allow modifying internal variables.
 *
 * When a damper is used in any component (follow or aim), the following typical steps should be followed:
 *   1) Call `SetInput`, set the value to damp.
 *   2) Call `ApplyDamp`, damp the input value and return the result.
 *   3) Call `SetOutput` (optional), set the damped result from step 2) to a new value, usually updated by the component.
 *   4) Call `PostApplyDamp` (optional), update the damper's internal variables.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraDamper : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CameraDamper")
	void SetInput(const float& _Input) { Input = _Input; }

	UFUNCTION(BlueprintCallable, Category = "CameraDamper")
	void SetOutput(const float& _Output) { Output = _Output; }

	/**
	 * Implement this function to apply damping to the internal variable `Input` with elapsed `DeltaTime`.
	 * This function generally caches the damped value as the internal `Output` value and returns it.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "CameraDamper")
	float ApplyDamp(const float& DeltaTime);
	virtual float ApplyDamp_Implementation(const float& DeltaTime) { return Input; }

	/**
	 * Implement this function to update the damper's internal variables, e.g., cached positions/velocities.
	 * This function provides an ad-hoc interface for customized dampers and is usually called after `SetOutput`.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "CameraDamper")
	void PostApplyDamp();
	virtual void PostApplyDamp_Implementation() { }

protected:
	/* Input to damp at this frame. */
	UPROPERTY(BlueprintReadWrite, Category = "CameraDamper")
	float Input;

	/* Output after damp at this frame. */
	UPROPERTY(BlueprintReadWrite, Category = "CameraDamper")
	float Output;
};

/** Uses a simple residual-based damping algorithm. This is the damper you should choose for most of the time. */
UCLASS(classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UNaiveDamper : public UECameraDamper
{
	GENERATED_BODY()

public:
	UNaiveDamper() {}
	UNaiveDamper(float InDampTime) : DampTime(InDampTime) {}
	UNaiveDamper(float InDampTime, float InResidual) : DampTime(InDampTime), Residual(InResidual) {}

	/** Damp time. X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NaiveDamper", meta = (ClampMin = "0"))
	float DampTime{ 0.2 };

	/** Damp residual after damp time (in percent). X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NaiveDamper", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float Residual{ 0.01 };

	virtual float ApplyDamp_Implementation(const float& DeltaTime) override;
};

/** Splits the given deltaTime into several parts and simulates naive damping in order. */
UCLASS(classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API USimulateDamper : public UECameraDamper
{
	GENERATED_BODY()

public:
	USimulateDamper() {}
	USimulateDamper(float InDampTime, float InResidual, int InCount) : DampTime(InDampTime), Residual(InResidual), SimulateCount(InCount) {}

	/** Damp time. X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SimulateDamper", meta = (ClampMin = "0"))
	float DampTime{ 0.2 };

	/** Damp residual after damp time (in percent). X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SimulateDamper", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float Residual{ 0.01 };

	/** Number of simulations per damping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SimulateDamper", meta = (ClampMin = "1", ClampMax = "10"))
	int SimulateCount{ 5 };

	virtual float ApplyDamp_Implementation(const float& DeltaTime) override;
};

/** Continuous optimization when FPS is unstable. Reference https://sulley.cc/2023/07/08/18/22/#solution-3-continuous-residual. */
UCLASS(classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UContinuousNaiveDamper : public UECameraDamper
{
	GENERATED_BODY()

public:
	UContinuousNaiveDamper() {}
	UContinuousNaiveDamper(float InDampTime, float InResidual, int InOrder) : DampTime(InDampTime), Residual(InResidual), Order(InOrder) {}

	/** Damp time. X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContinuousDamper", meta = (ClampMin = "0"))
	float DampTime{ 0.2 };

	/** Damp residual after damp time (in percent). X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContinuousDamper", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float Residual{ 0.01 };

	/** Orders of derivative you want to use for approximation. Larger means more accurate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContinuousDamper", meta = (ClampMin = "1", ClampMax = "7"))
	int Order{ 5 };

	virtual float ApplyDamp_Implementation(const float& DeltaTime) override;
	virtual void PostApplyDamp_Implementation() override;

private:
	static inline std::vector<float> Factorials = { 1, 1, 2, 6, 24, 120, 720, 5040 };
	float PreviousResidual{ 0.0 };
};

/** Restrict damp velocity when FPS in unstable. Reference https://sulley.cc/2023/07/08/18/22/#solution-1-imposing-an-invalid-range. */
UCLASS(classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API URestrictNaiveDamper : public UECameraDamper
{
	GENERATED_BODY()

public:
	URestrictNaiveDamper() { }
	URestrictNaiveDamper(float InDampTime, float InResidual, float InTolerance, float InPower) : DampTime(InDampTime), Residual(InResidual), Tolerance(InTolerance), Power(InPower) {}

	/** Damp time. X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RestrictDamper", meta = (ClampMin = "0"))
	float DampTime{ 0.2 };

	/** Damp residual after damp time (in percent). X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RestrictDamper", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float Residual{ 0.01 };

	/** Tolerance of restriction range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RestrictDamper", Meta = (ClampMin = "0.0"))
	float Tolerance{ 0.1 };

	/** Controls how aggresively to compact the curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RestrictDamper", Meta = (ClampMin = "0.0"))
	float Power{ 3.0 };

	virtual float ApplyDamp_Implementation(const float& DeltaTime) override;
	virtual void PostApplyDamp_Implementation() override;

private:
	float PreviousResidual{ 0.0 };
};

/** Low-pass filtering optimization when FPS in unstable. Reference https://sulley.cc/2023/07/08/18/22/#solution-2-adding-low-pass-filter. */
UCLASS(classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API ULowpassNaiveDamper : public UECameraDamper
{
	GENERATED_BODY()

public:
	ULowpassNaiveDamper() { }
	ULowpassNaiveDamper(float InDampTime, float InResidual, float InTolerance, float InBeta) : DampTime(InDampTime), Residual(InResidual), Tolerance(InTolerance), Beta(InBeta) {}

	/** Damp time. X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LowpassDamper", meta = (ClampMin = "0"))
	float DampTime{ 0.2 };

	/** Damp residual after damp time (in percent). X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LowpassDamper", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float Residual{ 0.01 };

	/** Tolerance of restriction range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LowpassDamper", Meta = (ClampMin = "0.0"))
	float Tolerance{ 0.1 };

	/** Smaller means smoother. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LowpassDamper", Meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Beta{ 0.001 };

	virtual float ApplyDamp_Implementation(const float& DeltaTime) override;
	virtual void PostApplyDamp_Implementation() override;

private:
	float PreviousResidual{ 0.0 };
	float DeltaResidual{ 0.0 };
};

/** Uses spring to damp. Reference https://sulley.cc/2024/06/18/20/06/. */
UCLASS(classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API USpringDamper : public UECameraDamper
{
	GENERATED_BODY()

public:
	USpringDamper() {}
	USpringDamper(float InFrequency, float InDampRatio) : Frequency(InFrequency), DampRatio(InDampRatio) {}

	/** Used for Spring. Controls the frequency of oscillation and the speed of decay.  X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringDamper", Meta = (ClampMin = "0.0001"))
	float Frequency{ 3.1415926 };

	/** Used for Spring. Damp ratio, controlling whether the spring is undamped (=0), underdamped (<1), critically damped (=1), or overdamped (>1). X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringDamper", Meta = (ClampMin = "0"))
	float DampRatio{ 1.0 };

	virtual float ApplyDamp_Implementation(const float& DeltaTime) override;

private:
	float Velocity{ 0.0 };
};