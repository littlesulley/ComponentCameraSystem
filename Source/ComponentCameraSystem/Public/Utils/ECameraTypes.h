// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Platform.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "WaveOscillatorCameraShakePattern.h"
#include "Misc/EnumRange.h"
#include "Camera/PlayerCameraManager.h"

#include "ECameraTypes.generated.h"

/**************************************************************************************/
/*************************************  Begin macros. *********************************/
/**************************************************************************************/

#ifndef UE50
#define UE50 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0)
#endif

#ifndef UE51
#define UE51 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 1)
#endif

#ifndef UE52
#define UE52 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 2)
#endif

#ifndef UE53
#define UE53 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 3)
#endif

#ifndef UE54
#define UE54 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4)
#endif

#ifndef LATER_THAN_UE50
#define LATER_THAN_UE50 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 0)
#endif

#ifndef LATER_THAN_UE51
#define LATER_THAN_UE51 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
#endif

#ifndef LATER_THAN_UE52
#define LATER_THAN_UE52 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2)
#endif

#ifndef LATER_THAN_UE53
#define LATER_THAN_UE53 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)
#endif

#ifndef LATER_THAN_UE54
#define LATER_THAN_UE54 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
#endif

#ifndef EARLIER_THAN_UE50
#define EARLIER_THAN_UE50 ENGINE_MAJOR_VERSION < 5
#endif

#ifndef EARLIER_THAN_UE51
#define EARLIER_THAN_UE51 (ENGINE_MAJOR_VERSION < 5 || ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 1)
#endif

#ifndef EARLIER_THAN_UE52
#define EARLIER_THAN_UE52 (ENGINE_MAJOR_VERSION < 5 || ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 2)
#endif

#ifndef EARLIER_THAN_UE53
#define EARLIER_THAN_UE53 (ENGINE_MAJOR_VERSION < 5 || ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 3)
#endif

#ifndef EARLIER_THAN_UE54
#define EARLIER_THAN_UE54 (ENGINE_MAJOR_VERSION < 5 || ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 4)
#endif

/**************************************************************************************/
/*************************************  Begin enums. **********************************/
/**************************************************************************************/

/**
* Define headings.
*/
UENUM()
enum class EHeading : uint8
{
	/** World space forward as heading, i.e., (1,0,0). */
	WorldForward,
	/** Target's local space forward as heading. 
	 *  Different components may interpret this differently.
	 *  For example, ControlAim component uses follow target's forward.
	 *  NOTE: this should be used very carefully as it may cause unexpected artifacts. 
	 */
	TargetForward,
	/** A hard specified forward. */
	HardForward,
	/** A soft specified forwad, determined by a source actor and a destination actor. */
	SoftForward
};
 
/**
* Stages in which camera components should be applied.
* Each camera component has a specific stage.
* Order: PreFollow -> Follow -> PostFollow -> PreAim -> Aim -> PostAim -> Noise -> Finalize.
*/
UENUM()
enum class EStage : uint8
{
	/** Executes at the PreFollow stage. */
	PreFollow,
	/** Executes at the Follow stage. */
	Follow,
	/** Executes at the PostFollow stage. */
	PostFollow,
	/** Executes at the PreAim stage. */
	PreAim,
	/** Executes at the Aim stage. */
	Aim,
	/** Executes at the PostAim stage. */
	PostAim,
	/** Executes at the Noise stage. */
	Noise,
	/** Executes at the Finalize stage. */
	Finalize
};
ENUM_RANGE_BY_FIRST_AND_LAST(EStage, EStage::PreFollow, EStage::Finalize)

UENUM()
enum class ESimpleFollowType : uint8
{
	/** Keep a fixed relationship at world space. */
	WorldSpace,
	/** Keep a fixed relationship at follow target's local space. */
	LocalSpace
};

/**
* Methods you can use for trace.
*/
UENUM()
enum class ETraceShape : uint8
{
	/** Uses a simple line trace emitted from camera. */
	Line,
	/** Use a sphere. */
	Sphere
};

/**
* Methods you can use for damping.
*/
UENUM()
enum class EDampMethod : uint8
{
	/** Uses a simple residual-based damping algorithm. */
	Naive,
	/** Naive with continuous optimization. Reference https://sulley.cc/2023/07/08/18/22/#solution-3-continuous-residual. */
	ContinuousNaive,
	/** Naive with restriction optimization. Reference https://sulley.cc/2023/07/08/18/22/#solution-1-imposing-an-invalid-range. */
	RestrictedNaive,
	/** Naive with soft restriction optimization. Reference https://sulley.cc/2023/07/08/18/22/#solution-2-adding-low-pass-filter. */
	SoftRestrictedNaive,
	/** Naive with low-pass filtering optimization. Reference https://sulley.cc/2023/07/08/18/22/#solution-2-adding-low-pass-filter. */
	LowPassNaive,
	/** Splits the given deltaTime into several parts and simulates naive damping in order. */
	Simulate,
	/** Uses spring to damp. */
	Spring,
	/** Uses exact spring damper. */
	ExactSpring
};

/**
* Methods you want to use for calculate group actor location.
*/
UENUM()
enum class EGroupLocationMethod : uint8
{
	/** Group actor location will be the arithmetic average of all target locations. */
	ArithmeticAverage,
	/** Group actor location will be the weighed average of target locations. */
	WeighedAverage,
	/** Group actor location will be dynamically determined based on the distance between the given location and each target actor location.
	 *  This method does not take actor weight into consideration.
	 */
	UnweightedDistanceBased,
	/** Weighted version of UnweightedDistanceBased. */
	WeightedDistanceBased,
	/** The closest target location to the given location will be used as group actor location. */
	Closest,
	/** The farest target location to the given location will be used as group actor location. */
	Farest,
	/** Specified location. */
	Specified
};

/**
* Methods you want to use for calculate group actor rotation.
*/
UENUM()
enum class EGroupRotationMethod : uint8
{
	/** Group actor rotation will be the arithmetic average of all target rotations. */
	ArithmeticAverage,
	/** Group actor rotation will be the weighed average of target rotations. */
	WeighedAverage,
	/** Specified rotation. */
	Specified
};

/**
* Methods you want to use to resolve group actor in scren space.
* DistanceOnly and Mix are recommended.
*/
UENUM()
enum class EResolveGroupActorMethod : uint8
{
	/** Only adjust camera FOV. */
	ZoomOnly,
	/** Only adjust camera distance. */
	DistanceOnly,
	/** Mix-adjusting FOV and distance. */
	Mix
};

/** Methods you want to use for RailFollow component. */
UENUM()
enum class ERailFollowType : uint8
{
	/** Camera will get to the position on rail nearest to the follow target. */
	FollowTarget,
	/** Camera will move on rail at a fixed speed, starting at some place you specify. */
	FixedSpeed,
	/** Camera will move according to your manual setup. */
	Manual
};

/** Methods you want to use for CraneFollow component. */
UENUM()
enum class ECraneFollowType : uint8
{
	/** Camera will get to the position on crane nearest to the follow target. DO NOT use a loop spline for this type. */
	FollowTarget,
	/** Camera will move on crane at a fixed speed, starting at some place you specify. */
	FixedSpeed,
	/** Camera will move according to your manual setup. */
	Manual
};

/** Camera shake types. */
UENUM()
enum class ECameraShakeType : uint8
{
	Wave,
	Perlin
};

/** How do you want to preserve keyframes in the actor sequence component.  */
UENUM()
enum class EKeyframePreservationType : uint8
{
	None,
	FirstOnly,
	LastOnly,
	FirstAndLast,
	All
};

/** Models for procedural camera motion generation. */
UENUM()
enum class EPCMGModel : uint8
{
	/** Reinforcement learning - PPO model. Implementation based on https://iclr-blog-track.github.io/2022/03/25/ppo-implementation-details/. */
	PPO           UMETA(DisplayName = "PPO"),
	/** Reinforcement learning - SAC model. Implementation based on https://github.com/XinJingHao/SAC-Continuous-Pytorch/blob/main/SAC.py. */
	SAC           UMETA(DisplayName = "SAC"),
	/** A randomized magnetic field algorithm. */
	Magnetic      UMETA(DisplayName = "MagneticField"),
	/** Function composition, including trigonometric functions, exponential functions, polynomial functions, etc. */
	Function      UMETA(DisplayName = "FunctionComposition")
};

/** Schemes to update mixing camera's weights. */
UENUM()
enum class EMixingCameraWeightUpdateScheme : uint8
{
	/** Weights are equally distributed. */
	Average,
	/** Update weights in blueprint using your own custom scheme. */
	Manual
};

/** Schemes to mix sub-cameras of a mixing camera. */
UENUM()
enum class EMixingCameraMixScheme : uint8
{
	/** Only mix camera's position. */
	PositionOnly,
	/** Only mix camera's rotation. */
	RotationOnly,
	/** Mix both camera's position and rotation. */
	Both
};

/** Methods to mix rotations of a mixing camera. */
UENUM()
enum class EMixingCameraMixRotationMethod : uint8
{
	/** Trivially mix rotations. Simple weighted average. May cause sudden jump.  */
	Trivial,
	/** Matrix decomposition to find the largest eigen-value. Smooth but at a high efficiency cost (involving multiple iterations).
	  * Blog: https://sulley.cc/2024/01/11/20/06/
	  */
	Eigenvalue,
	/** Use smoothed circular mean. Ref: https://sulley.cc/2024/01/11/20/06/#sc-lerp-smoothed-c-lerp */
	Circular
};

/** Strategies to apply camera rolling. */
UENUM()
enum class ERollScheme : uint8
{
	/** Manually set roll using blueprint node. */
	Manual,
	/** Constant speed */
	Constant,
	/** Proportional to follow target's velocity. */
	ProportionalToVelocity
};

UENUM()
enum class EPhotoModeResolution : uint8
{
	HD			UMETA(DisplayName = "720p"),
	FHD			UMETA(DisplayName = "1080p"),
	QHD			UMETA(DisplayName = "1440p (2K)"),
	UHD			UMETA(DisplayName = "2160p (4K)"),
	Cinematic   UMETA(DisplayName = "Cinematic")
};

/** Strategies to recenter camera. */
UENUM()
enum class ERecenterScheme : uint8
{
	/** Auto-recenter camera. */
	Auto,
	/** Manually start and stop camera recentering. */
	Manual
};

UENUM()
enum class ELatentOutputPins : uint8
{
	/** Execute each tick. */
	OnTick,
	/** Execute when the function completes its work. */
	OnComplete,
	/** Execute when the function is interrupted. */
	OnInterrupt
};

/**************************************************************************************/
/********************************* Begin structs.**************************************/
/**************************************************************************************/

/** A set of parameters describing an orbit. */
USTRUCT(BlueprintType)
struct FOrbit
{
	GENERATED_USTRUCT_BODY()

public:
	/** Height relative to follow target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOrbit")
	float Height;

	/** Orbit radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOrbit")
	float Radius;

	FOrbit()
		: Height(10.0f)
		, Radius(30.0f)
	{ }
};

/** A set of parameters describing recentering setting. */
USTRUCT(BlueprintType)
struct FRecenteringParams
{
	GENERATED_USTRUCT_BODY()

public:

	/** Whether to enable recentering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams")
	bool bRecentering;

	/** Whether to reset pitch to zero when recentering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams")
	bool bResetPitch;

	/** Heading for recentering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams")
	EHeading Heading;

	/** Scheme to recenter cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams")
	ERecenterScheme RecenterScheme;

	/** How long will it take to start recentering since input is not given. Only used when RecenterScheme is Auto. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams", meta = (EditCondition = "bRecentering ==  true && RecenterScheme == ERecenterScheme::Auto"))
	float WaitTime;

	/** Time to finish recentering. Only used when RecenterScheme is Auto. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams", meta = (EditCondition = "bRecentering ==  true && RecenterScheme == ERecenterScheme::Auto"))
	float RecenteringTime;

	/** Only do recentering when YAW between the camera forward direction and the heading direction is within this range. 
	  * Used when scheme is Auto and heading is TargetForward.
	  **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams", meta = (EditCondition = "Heading == EHeading::TargetForward && RecenterScheme == ERecenterScheme::Auto", ClampMin = "-180.0", ClampMax = "180.0"))
	FVector2D RecenterRange;

	/** Hard specified forward. Will ignore its pitch angle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams", meta = (EditCondition = "bRecentering ==  true && Heading == EHeading::HardForward"))
	FVector HardForward;

	/** Source actor for soft forward. Its position is used as the start position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams", meta = (EditCondition = "bRecentering ==  true && Heading == EHeading::SoftForward"))
	AActor* Source;

	/** Destination actor for soft forward. Its position is used as the end position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRecenteringParams", meta = (EditCondition = "bRecentering ==  true && Heading == EHeading::SoftForward"))
	AActor* Destination;


	FRecenteringParams()
		: bRecentering(false)
		, bResetPitch(false)
		, Heading(EHeading::WorldForward)
		, RecenterScheme(ERecenterScheme::Auto)
		, WaitTime(2.0f)
		, RecenteringTime(2.0f)
		, RecenterRange(FVector2D(-175.f, 175.f))
		, HardForward(FVector(1, 0, 0))
	{ }
};

/** A set of parameters describing occlusion detection. */
USTRUCT(BlueprintType)
struct FOccluderParams
{
	GENERATED_USTRUCT_BODY()

public:
	/** Shape you use for tracing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOccluderParams")
	ETraceShape TraceShape;

	/** If on, only returns the first hit encountered. This can be toggled on to improve performance.
	 *  This should be turned on in most cases.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOccluderParams")
	bool bTraceSingle;

	/** Array of object types to trace occlusion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOccluderParams")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	/** Actor types to ignore. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOccluderParams")
	TArray<TSubclassOf<AActor>> ActorTypesToIgnore;

	/** Actors ignored for trace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOccluderParams")
	TArray<TSoftObjectPtr<AActor>> ActorsToIgnore;

	/** Maximum length of the ray used to trace occlusion along the sight line of camera. Set as 0 to use the actual line length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOccluderParams", meta = (ClampMin = "0.0"))
	float MaximumTraceLength;

	/** Do not take any action until occlusion has lasted this long. Set as 0 to immediately avoid occlusion.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOccluderParams", meta = (ClampMin = "0.0"))
	float MinimumOcclusionTime;

	/** If use Sphere trace type, define its radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOccluderParams", Meta = (EditCondition = "TraceShape == ETraceShape::Sphere", ClampMin = "0.01"))
	float SphereRadius;

	/** Whether to show debug line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOccluderParams")
	bool bShowDebug;

	FOccluderParams()
		: TraceShape(ETraceShape::Line)
		, bTraceSingle(true)
		, MaximumTraceLength(0.0f)
		, MinimumOcclusionTime(0.0f)
		, SphereRadius(0.01f)
		, bShowDebug(false)
	{ }
};

USTRUCT(BlueprintType)
struct FDitherParams
{
	GENERATED_USTRUCT_BODY()

public:
	/** Array of object types of actors that should be taken into account for fade. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	/** Actor types to ignore for fade. Note that this parameter should exclude the type of self, which is usually Pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams")
	TArray<TSubclassOf<AActor>> ActorTypesToIgnore;

	/** Actors ignored for fade. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams")
	TArray<AActor*> ActorsToIgnore;

	/** Sphere radius for tracing objects. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams", meta = (ClampMin = "0.0"))
	float SphereRadius;

	/** Maximum trace length. Set as 0 to use the length from camera to target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams", meta = (ClampMin = "0.0"))
	float MaxTraceLength;

	/** Whether to enable self dither fade. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams")
	bool bEnableSelfDither;

	/** Object type of self, usually Pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams", meta = (EditCondition = "bEnableSelfDither == true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> SelfType;

	/** Trace length for dither fading the controlled pawn (self). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams", meta = (ClampMin = "0.0", EditCondition = "bEnableSelfDither == true"))
	float DitherSelfTraceLength;

	/** Distance determining max dither weight. If camera-hit distance is less than this value, weight of self dither would be the largest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams", meta = (ClampMin = "0.0", EditCondition = "bEnableSelfDither == true"))
	float DitherSelfMaxWeightDistance;

	/** Whether to show debug line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDitherParams")
	bool bShowDebug;

	FDitherParams()
		: SphereRadius(5.0f)
		, MaxTraceLength(0.0f)
		, bEnableSelfDither(true)
		, DitherSelfTraceLength(80.0f)
		, DitherSelfMaxWeightDistance(0.0f)
		, bShowDebug(false)
	{ }
};

USTRUCT(BlueprintType)
struct FFadedObjectInfo
{
	GENERATED_USTRUCT_BODY()	

public:
	/** Whether this actor is currently being faded. Is true when this actor is fading in or remaining faded, but not fading out. */
	bool bFading;
	/** Weight. */
	float Weight;
	/** Distance from camera to the hit point. -1 means infinity. */
	float Distance;
	/** Elapsed fade in time. */
	float ElapsedFadeInTime;
	/** Elapsed fade out time. */
	float ElapsedFadeOutTime;

	FFadedObjectInfo()
		: bFading(true)
		, Weight(0.0f)
		, Distance(-1.0f)
		, ElapsedFadeInTime(0.0f)
		, ElapsedFadeOutTime(0.0f)
	{ }

	FFadedObjectInfo(float InWeight, float InDistance, float InElapsedFadeInTime, float InElapsedFadeOutTime, bool InbFading)
		: bFading(InbFading)
		, Weight(InWeight)
		, Distance(InDistance)
		, ElapsedFadeInTime(InElapsedFadeInTime)
		, ElapsedFadeOutTime(InElapsedFadeOutTime)
	{ }
};


/** A set of parameters describing damping. 
 *  You should always add damping, even though you don't really need it.
 *  If this is the case, set damping as a very small number, such as 0.005.
 */
USTRUCT(BlueprintType)
struct FDampParams
{
	GENERATED_USTRUCT_BODY()

public:
	/** Which damp method you want to use. Select from Naive, Simulate, Spring and Exact Spring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams")
	EDampMethod DampMethod;

	/** Damp time. X/Y/Z or Roll/Pitch/Yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod != EDampMethod::Spring && DampMethod != EDampMethod::ExactSpring"))
	FVector DampTime;

	/** Used for Naive and Simulate. Damp residual after damp time (in percent). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod != EDampMethod::Spring && DampMethod != EDampMethod::ExactSpring"))
	float Residual;

	/** Used for ContinuousNaive. Orders of derivative you want to use for approximation. Larger means more accurate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod == EDampMethod::ContinuousNaive", ClampMin = "1", ClampMax = "7"))
	int Order;

	/** Used for Spring. The spring coefficient controlling how responsive the actor gets back to its rest place. In the order of X, Y and Z axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod == EDampMethod::Spring", ClampMin = "0.0", ClampMax = "1000.0"))
	FVector SpringCoefficient;

	/** Used for Spring. Damp residual after damp time. A larger value leads to a more damp-like effect. A small SpringCoefficient (~150) with a large SpringResidual (~0.95) produces a relaxing spring.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod == EDampMethod::Spring", ClampMin = "0.0", ClampMax = "1.0"))
	float SpringResidual;

	/** Used for Exact Spring. A value of 1 means a critically damped spring, a value <1 means an under-damped spring, and a value of >1 means a over-damped spring. Cannot be negative. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod == EDampMethod::ExactSpring", ClampMin = "0.001", ClampMax = "5.0"))
	FVector DampRatio;
		
	/** Used for Exact Spring. Duration of time used to damp the input value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod == EDampMethod::ExactSpring", ClampMin = "0.0"))
	FVector HalfLife;

	/** Used for RestrictedNaive and SoftRestrictedNaive. Tolerance of restriction range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod == EDampMethod::RestrictedNaive || DampMethod == EDampMethod::SoftRestrictedNaive || DampMethod == EDampMethod::LowPassNaive", ClampMin = "0.0"))
	float Tolerance;

	/** Used for SoftRestrictedNaive. Controls how aggresively to compact the curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod == EDampMethod::SoftRestrictedNaive", ClampMin = "0.0"))
	float Power;

	/** Used for LowPassNaive. Smaller means smoother. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FDampParams", Meta = (EditCondition = "DampMethod == EDampMethod::LowPassNaive", ClampMin = "0.0", ClampMax = "1.0"))
	float LowpassBeta;

	float MaxDeltaSeconds;

	FDampParams()
		: DampMethod(EDampMethod::Naive)
		, DampTime(FVector(0.2f, 0.2f, 0.2f))
		, Residual(0.01f)
		, Order(3)
		, SpringCoefficient(FVector(200.0f, 250.0f, 250.0f))
		, SpringResidual(0.5f)
		, DampRatio(1.0f)
		, HalfLife(0.5f)
		, Tolerance(0.1f)
		, Power(3.0f)
		, LowpassBeta(0.001f)
		, MaxDeltaSeconds(1 / 60.0f)
	{ }

	FDampParams(EDampMethod DampMethod, float Residual)
		: DampMethod(DampMethod)
		, DampTime(FVector(0.2f, 0.2f, 0.2f))
		, Residual(Residual)
		, Order(3)
		, SpringCoefficient(FVector(300.0f, 250.0f, 250.0f))
		, SpringResidual(0.5f)
		, DampRatio(1.0f)
		, HalfLife(0.5f)
		, Tolerance(0.1f)
		, Power(3.0f)
		, LowpassBeta(0.001f)
		, MaxDeltaSeconds(1 / 60.0f)
	{ }
};

/** A set of parameters describing an actor wrapped by a bounding box. */
USTRUCT(BlueprintType)
struct FBoundingWrappedActor
{
	GENERATED_USTRUCT_BODY()

public:
	/** Target actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FBoundingWrappedActor")
	TObjectPtr<AActor> Target;

	/** Weight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FBoundingWrappedActor", meta = (ClampMin = "0.0"))
	float Weight;

	/** (Half) Width of the bounding box on screen space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FBoundingWrappedActor", meta = (ClampMin = "0.0"))
	float Width;

	/** (Half) Height of the bounding box on screen space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FBoundingWrappedActor", meta = (ClampMin = "0.0"))
	float Height;

	/** Whether to exclude this actor (ignore this bounding box) during the resolving stage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FBoundingWrappedActor")
	bool bExcludeBoundingBox;

	FBoundingWrappedActor()
		: Weight(1.0f)
		, Width(0.0f)
		, Height(0.0f)
		, bExcludeBoundingBox(false)
	{ }

	/** Set weight. */
	void SetWeight(float InWeight) { Weight = InWeight; }

	/** Set width. */
	void SetWidth(float InWidth) { Width = InWidth; }

	/** Set height. */
	void SetHeight(float InHeight) { Height = InHeight; }

	/** Set bExcludeBoundingBox. */
	void SetExcludeBoundingBox(bool bInExcludeBoundingBox) { bExcludeBoundingBox = bInExcludeBoundingBox; }
};

/** A set of parameters defining one key point for ERailFollowType::Manual. */
USTRUCT(BlueprintType)
struct FRailManualPointParams
{
	GENERATED_USTRUCT_BODY()

public:
	/** Position on rail, within [0,1]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRailManualPointParams")
	float Position;

	/** Blend time used from previous key point to this one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRailManualPointParams")
	float BlendTime;

	/** Blend function used from previous key point to this one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRailManualPointParams")
	TEnumAsByte<EEasingFunc::Type> BlendFunc;

	/** How long will this key point stay at this position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRailManualPointParams")
	float Duration;

	/** Is this key point going forward from previous key point or backward?
	 *  For example, if previous key point position is 0.5, and this is 0.8,
	 *  forward direction 0.5->0.6->0.7->0.8, and backward is 0.5->0.4->...->1.0->0.9->0.8.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRailManualPointParams")
	bool bForward;

	FRailManualPointParams()
		: Position(0.0f)
		, BlendTime(1.0f)
		, BlendFunc(EViewTargetBlendFunction::VTBlend_Linear)
		, Duration(1.0f)
		, bForward(true)
	{ }
};

/** A set of parameters for ERailFollowType::Manual. */
USTRUCT(BlueprintType)
struct FRailManualParams
{
	GENERATED_USTRUCT_BODY()

public:
	/** Start position on rail, within [0,1]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRailManualParams")
	float StartPosition;

	/** A list of key points on rail. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FRailManualParams")
	TArray<FRailManualPointParams> KeyPoints;

	FRailManualParams()
		: StartPosition(0.0f)
	{ }
};

/** A set of parameters defining one key point for ECraneFollowType::Manual. */
USTRUCT(BlueprintType)
struct FCraneManualPointParams
{
	GENERATED_USTRUCT_BODY()

public:
	/** Position on crane, respectively for arm length, crane yaw and crane pitch, in world space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FCraneManualPointParams")
	FVector Position;

	/** Blend time used from previous key point to this one. Consistent for arm length, yaw and pitch. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FCraneManualPointParams")
	float BlendTime;

	/** Blend function used from previous key point to this one. Consistent for arm length, yaw and pitch. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FCraneManualPointParams")
	TEnumAsByte<EEasingFunc::Type> BlendFunc;

	/** How long will this key point stay at this position. Consistent for arm length, yaw and pitch. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FCraneManualPointParams")
	float Duration;

	/** Is yaw going forward from previous key point or backward? In degree.
	 *  For example, if previous key point position is 30, and this is 50,
	 *  forward direction 30->40->50, and backward is 30->20->...->360->350->...->50.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FCraneManualPointParams")
	bool bForwardYaw;

	/** Is pitch going forward from previous key point or backward? In degree.
	 *  For example, if previous key point position is 30, and this is 50,
	 *  forward direction 30->40->50, and backward is 30->20->...->360->350->...->50.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FCraneManualPointParams")
	bool bForwardPitch;

	FCraneManualPointParams()
		: Position(FVector(100, 0, 0))
		, BlendTime(1.0f)
		, BlendFunc(EViewTargetBlendFunction::VTBlend_Linear)
		, Duration(1.0f)
		, bForwardYaw(true)
		, bForwardPitch(true)
	{ }
};

/** A set of parameters for ECraneFollowType::Manual. */
USTRUCT(BlueprintType)
struct FCraneManualParams
{
	GENERATED_USTRUCT_BODY()

public:
	/** Start position on crane, respectively for arm length, crane yaw and crane pitch, in world space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FCraneManualParams")
	FVector StartPosition;

	/** A list of key points on crane. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FCraneManualParams")
	TArray<FCraneManualPointParams> KeyPoints;

	FCraneManualParams()
		: StartPosition(FVector(100, 0, 0))
	{ }
};

/** A struct consisting of PostProcessSettings and its corresponding weight. */
USTRUCT(BlueprintType)
struct FWeightedPostProcess
{
	GENERATED_USTRUCT_BODY()

public:
	/** Post process settings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedPostProcess")
	FPostProcessSettings PPSetting;

	/** Weight. The amount of influence the volume's properties have. A value of 1 has full effect, while a value of 0 has no effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedPostProcess")
	float Weight;

	/** Time used to blend into this post process effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedPostProcess")
	float BlendInTime;

	/** Duration for this post process effect. O means endlessness. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedPostProcess")
	float Duration;

	/** Time used to blend out of this post process effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedPostProcess")
	float BlendOutTime;

	float ElapsedBlendInTime;
	float ElapsedDurationTime;
	float ElapsedBlendOutTime;
	bool bHasCompleted;

	FWeightedPostProcess()
		: PPSetting(FPostProcessSettings())
		, Weight(1.f)
		, BlendInTime(0.5f)
		, Duration(0.f)
		, BlendOutTime(0.5f)
	{
		ElapsedBlendInTime = 0.f;
		ElapsedDurationTime = 0.f;
		ElapsedBlendOutTime = 0.f;
		bHasCompleted = false;
	}

	FWeightedPostProcess(const FPostProcessSettings& InPPSettings, const float& InWeight, const float& InBlendInTime, const float& InDuration, const float& InBlendOutTime)
		: PPSetting(InPPSettings)
		, Weight(InWeight)
		, BlendInTime(InBlendInTime)
		, Duration(InDuration)
		, BlendOutTime(InBlendOutTime)
	{
		ElapsedBlendInTime = 0.f;
		ElapsedDurationTime = 0.f;
		ElapsedBlendOutTime = 0.f;
		bHasCompleted = false;
	}
};

/** A struct consisting of a blendable and its corresponding weight. */
USTRUCT(BlueprintType)
struct FWeightedBlendableObject
{
	GENERATED_USTRUCT_BODY()

public:
	/** Blendable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedBlendableObject")
	TScriptInterface<IBlendableInterface> InBlendableObject;

	/** Weight. The amount of influence the volume's properties have. A value of 1 has full effect, while a value of 0 has no effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedBlendableObject")
	float Weight;

	/** Time used to blend into this post process effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedBlendableObject")
	float BlendInTime;

	/** Duration for this post process effect. O means endlessness. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedBlendableObject")
	float Duration;

	/** Time used to blend out of this post process effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FWeightedBlendableObject")
	float BlendOutTime;

	float ElapsedBlendInTime;
	float ElapsedDurationTime;
	float ElapsedBlendOutTime;
	bool bHasCompleted;

	FWeightedBlendableObject()
		: Weight(1.f)
		, BlendInTime(0.5f)
		, Duration(0.f)
		, BlendOutTime(0.5f)
	{
		ElapsedBlendInTime = 0.f;
		ElapsedDurationTime = 0.f;
		ElapsedBlendOutTime = 0.f;
		bHasCompleted = false;
	}

	FWeightedBlendableObject(const TScriptInterface<IBlendableInterface>& InBlendable, const float& InWeight, const float& InBlendInTime, const float& InDuration, const float& InBlendOutTime)
		: InBlendableObject(InBlendable)
		, Weight(InWeight)
		, BlendInTime(InBlendInTime)
		, Duration(InDuration)
		, BlendOutTime(InBlendOutTime)
	{
		ElapsedBlendInTime = 0.f;
		ElapsedDurationTime = 0.f;
		ElapsedBlendOutTime = 0.f;
		bHasCompleted = false;
	}
};

/** A struct packing some oscillation parameters.  */
USTRUCT(BlueprintType)
struct FPackedOscillationParams
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	ECameraShakeType ShakeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	float LocationAmplitudeMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	float LocationFrequencyMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	FWaveOscillator X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	FWaveOscillator Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	FWaveOscillator Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	float RotationAmplitudeMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	float RotationFrequencyMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	FWaveOscillator Pitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	FWaveOscillator Yaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	FWaveOscillator Roll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	FWaveOscillator FOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	float BlendInTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FPackedOscillationParams")
	float BlendOutTime;

	FPackedOscillationParams()
		: ShakeType(ECameraShakeType::Wave)
		, LocationAmplitudeMultiplier(1.f)
		, LocationFrequencyMultiplier(1.f)
		, X(FWaveOscillator())
		, Y(FWaveOscillator())
		, Z(FWaveOscillator())
		, RotationAmplitudeMultiplier(1.f)
		, RotationFrequencyMultiplier(1.f)
		, Pitch(FWaveOscillator())
		, Yaw(FWaveOscillator())
		, Roll(FWaveOscillator())
		, FOV(FWaveOscillator())
		, Duration(1.f)
		, BlendInTime(0.2f)
		, BlendOutTime(0.2f) { }
};

/** A struct composed of an actor type and an offset vector. */
USTRUCT(BlueprintType)
struct FOffsetActorType
{
	GENERATED_USTRUCT_BODY()

public:
	/** Actor type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOffsetActorType")
	TSubclassOf<AActor> ActorType;

	/** Local space offset applied to actors of this type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FOffsetActorType")
	FVector Offset;

	FOffsetActorType()
		: Offset(FVector()) { }
};

/** A struct used to define aim assist. */
USTRUCT(BlueprintType)
struct FAimAssist
{
	GENERATED_USTRUCT_BODY()

public:
	/** Whether to enable aim assist. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FAimAssist")
	bool bEnableAimAssist;

	/** Time interval to access and update target actors. In seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FAimAssist")
	float TimeInterval;

	/** Types of actors that are taken for aim assist. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FAimAssist")
	TArray<FOffsetActorType> TargetTypes;

	/** Magnetic radius, in screen space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FAimAssist")
	float MagneticRadius;

	/** Magnetic coefficient defining the strength of magnetic force. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FAimAssist")
	float MagneticCoefficient;

	/** If the target actor has a larger distance to camera than this value, it will be ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FAimAssist")
	float MaxDistance;

	FAimAssist()
		: bEnableAimAssist(false)
		, TimeInterval(0.1f)
		, MagneticRadius(50)
		, MagneticCoefficient(1)
		, MaxDistance(800)
	{ }
}; 

/** A set of parameters defining the function used to generate keyframes.
 *  The function is: A1*exp(A2*x+A3) * sin(B1*x+B2) + C1*x^3 + C2*x^2 + C3*x + D.
 */
USTRUCT(BlueprintType)
struct FFunctionParams
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParams")
	float A1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParams")
	float A2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParams")
	float B1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParams")
	float B2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParams")
	float C1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParams")
	float C2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParams")
	float C3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParams")
	float D;

	FFunctionParams()
		: A1(1.0)
		, A2(0.2)
		, B1(3.14159 / 2.0)
		, B2(0.0)
		, C1(0.0)
		, C2(0.0)
		, C3(0.0)
		, D(0.0)
	{ }
};

/** A collection of FFunctionParams for position and rotation. */
USTRUCT(BlueprintType)
struct FFunctionParamsCollection
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParamsCollection")
	FFunctionParams XFuncParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParamsCollection")
	FFunctionParams YFuncParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParamsCollection")
	FFunctionParams ZFuncParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParamsCollection")
	FFunctionParams RollFuncParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParamsCollection")
	FFunctionParams PitchFuncParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FFunctionParamsCollection")
	FFunctionParams YawFuncParams;

	FFunctionParamsCollection()
		: XFuncParams(FFunctionParams())
		, YFuncParams(FFunctionParams())
		, ZFuncParams(FFunctionParams())
		, RollFuncParams(FFunctionParams())
		, PitchFuncParams(FFunctionParams())
		, YawFuncParams(FFunctionParams())
	{ }
};

USTRUCT(BlueprintType)
struct FPCMGRangeParams
{
	GENERATED_USTRUCT_BODY()

public:
	/** Velocity range for X axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-200.0", ClampMax = "200.0"))
	FVector2D XVelocityRange;

	/** Velocity range for Y axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-200.0", ClampMax = "200.0"))
	FVector2D YVelocityRange;

	/** Velocity range for Z axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-200.0", ClampMax = "200.0"))
	FVector2D ZVelocityRange;

	/** Velocity range for roll. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-45.0", ClampMax = "45.0"))
	FVector2D RollVelocityRange;

	/** Velocity range for pitch. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-45.0", ClampMax = "45.0"))
	FVector2D PitchVelocityRange;

	/** Velocity range for yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-45.0", ClampMax = "45.0"))
	FVector2D YawVelocityRange;

	/** Acceleration range for X axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-400.0", ClampMax = "400.0"))
	FVector2D XAccelerationRange;

	/** Acceleration range for Y axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-400.0", ClampMax = "400.0"))
	FVector2D YAccelerationRange;

	/** Acceleration range for Z axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-400.0", ClampMax = "400.0"))
	FVector2D ZAccelerationRange;

	/** Acceleration range for roll. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	FVector2D RollAccelerationRange;

	/** Acceleration range for pitch. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	FVector2D PitchAccelerationRange;

	/** Acceleration range for yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGRangeParams", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	FVector2D YawAccelerationRange;

	FPCMGRangeParams()
		: XVelocityRange(FVector2D(-200.0, 200.0))
		, YVelocityRange(FVector2D(-200.0, 200.0))
		, ZVelocityRange(FVector2D(-200.0, 200.0))
		, RollVelocityRange(FVector2D(-45, 45))
		, PitchVelocityRange(FVector2D(-45, 45))
		, YawVelocityRange(FVector2D(-45, 45))
		, XAccelerationRange(FVector2D(-0, 0))
		, YAccelerationRange(FVector2D(-0, 0))
		, ZAccelerationRange(FVector2D(-0, 0))
		, RollAccelerationRange(FVector2D(-0, 0))
		, PitchAccelerationRange(FVector2D(-0, 0))
		, YawAccelerationRange(FVector2D(-0, 0))
	{ }
};

/** A set of parameters controlling the generation of camera motions (Procedural Camera Motion Generation). 
 *  You can use different models to generate camera motions in light of your input constraints.
 */
USTRUCT(BlueprintType)
struct FPCMGParams
{
	GENERATED_USTRUCT_BODY()

public:
	/** Which model do you want to use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams")
	EPCMGModel Model;

	/** Set of params to define the functions for generating keyframes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (EditCondition = "Model == EPCMGModel::Function"))
	FFunctionParamsCollection FunctionParams;

	/** How turbulent (i.e., tolerance of outliers) the genetraed camera trajectory would be. A value of 0 indicates a similar normality to normal distribution. 
	 *  Can be negative. The larger this value is, more outliers there will be in the data set. Used for position.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "-20.0", ClampMax = "20.0", EditCondition = "Model != EPCMGModel::Function"))
	float PosTurbulence;

	/** How turbulent (i.e., tolerance of outliers) the genetraed camera trajectory would be. A value of 0 indicates a similar normality to normal distribution.
	 *  Can be negative. The larger this value is, more outliers there will be in the data set. Used for rotation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "-20.0", ClampMax = "20.0", EditCondition = "Model != EPCMGModel::Function"))
	float RotTurbulence;

	/** How symmetric will the generated camera trajectory be. A value of 0 indicates absolutely symmetric. Negative and positive imply asymmetry. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "-3.0", ClampMax = "3.0", EditCondition = "Model != EPCMGModel::Function"))
	float PosSymmetry;

	/** How symmetric will the generated camera rotation be. A value of 0 indicates absolutely symmetric. Negative and positive imply asymmetry.
	 *  Respectively for roll, pitch and yaw.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "-3.0", ClampMax = "3.0", EditCondition = "Model != EPCMGModel::Function"))
	float RotSymmetry;

	/** How would you like the camera trajectory to be monotonically increasing for position. A maximum value of 1 indicates strictly increase. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "Model != EPCMGModel::Function"))
	FVector PosIncreaseMonotonicity;

	/** How would you like the camera trajectory to be monotonically decreasing for position. A maximum value of 1 indicates strictly decrease. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "Model != EPCMGModel::Function"))
	FVector PosDecreaseMonotonicity;

	/** How would you like the camera trajectory to be monotonically increasing for rotation. A maximum value of 1 indicates strictly increase. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "Model != EPCMGModel::Function"))
	FVector RotIncreaseMonotonicity;

	/** How would you like the camera trajectory to be monotonically decreasing for rotation. A maximum value of 1 indicates strictly decrease. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "Model != EPCMGModel::Function"))
	FVector RotDecreaseMonotonicity;

	/** Spearman coefficient for position. +1 means monotonically increasing, and -1 means monotonically decreasing. Respectively for X, Y and Z axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "-1.0", ClampMax = "1.0", EditCondition = "Model != EPCMGModel::Function"))
	FVector PosSpearman;

	/** Spearman coefficient for rotation. +1 means monotonically increasing, and -1 means monotonically decreasing. Respectively for roll, pitch and yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (ClampMin = "-1.0", ClampMax = "1.0", EditCondition = "Model != EPCMGModel::Function"))
	FVector RotSpearman;

	/** A variaty of ranges to bound the tangent and velocity of position and rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FPCMGParams", meta = (EditCondition = "Model != EPCMGModel::Function"))
	FPCMGRangeParams Ranges;

	FPCMGParams()
		: Model(EPCMGModel::SAC)
		, FunctionParams(FFunctionParamsCollection())
		, PosTurbulence(0)
		, RotTurbulence(0)
		, PosSymmetry(1)
		, RotSymmetry(1)
		, PosIncreaseMonotonicity(FVector(0.5, 0.5, 0.5))
		, PosDecreaseMonotonicity(FVector(0.5, 0.5, 0.5))
		, RotIncreaseMonotonicity(FVector(0.5, 0.5, 0.5))
		, RotDecreaseMonotonicity(FVector(0.5, 0.5, 0.5))
		, Ranges(FPCMGRangeParams())
	{ }
};

USTRUCT(BlueprintType)
struct FECameraClassWithTargets
{
	GENERATED_USTRUCT_BODY()

public:
	/** Camera class. You cannot place a AEMixingCamera here!  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargets")
	TSubclassOf<class AECameraBase> CameraClass;

	/** Optional soft follow target. Specified in blueprint or instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargets")
	TSoftObjectPtr<AActor> FollowTarget;

	/** Optional follow target type. If FollowTarget is null, try to find actor of this type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargets")
	TSubclassOf<AActor> FollowTargetType;

	/** Optional soft aim target. Specified in blueprint or instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargets")
	TSoftObjectPtr<AActor> AimTarget;

	/** Optional aim target type. If AimTarget is null, try to find actor of this type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargets")
	TSubclassOf<AActor> AimTargetType;
};

USTRUCT(BlueprintType)
struct FECameraClassWithTargetsAndBlending
{
	GENERATED_USTRUCT_BODY()

public:
	/** Camera class. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	TSubclassOf<class AECameraBase> CameraClass;

	/** Optional soft follow target. Specified in blueprint or instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	TSoftObjectPtr<AActor> FollowTarget;

	/** Optional follow target type. If FollowTarget is null, try to find actor of this type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	TSubclassOf<AActor> FollowTargetType;

	/** Optional soft aim target. Specified in blueprint or instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	TSoftObjectPtr<AActor> AimTarget;

	/** Optional aim target type. If AimTarget is null, try to find actor of this type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	TSubclassOf<AActor> AimTargetType;

	/** Optional follow target socket. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	FName FollowSocket;

	/** Optional aim target socket. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	FName AimSocket;

	/** Duration of this camera. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	float Duration;

	/** Blend time from previous camera. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	float BlendTime;

	/** Blend function. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	TEnumAsByte<EViewTargetBlendFunction> BlendFunc;

	/** Blend exponential. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	float BlendExp;

	/** If true, lock outgoing viewtarget to last frame's camera position for the remainder of the blend. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	bool bLockOutgoing;

	/** Whether should preserve state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FECameraClassWithTargetsAndBlending")
	bool bPreserveState;

	FECameraClassWithTargetsAndBlending()
		: Duration(2.0f)
		, BlendTime(1.0f)
		, BlendFunc(EViewTargetBlendFunction::VTBlend_Linear)
		, BlendExp(1.0f)
		, bLockOutgoing(true)
		, bPreserveState(false)
	{ }
};

/** A set of parameters controlling camera zooming. For ScreenFollow only. */
USTRUCT(BlueprintType)
struct FZoomSettings
{
	GENERATED_USTRUCT_BODY()

public:
	/** Whether to enable zooming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FZoomSettings")
	bool bEnableZoom;

	/** The input action to receive zoom input, can be mouse wheels. Ensure this action is correct. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ECamera|FZoomSettings", meta = (EditCondition = "bEnableZoom == true"))
	class UInputAction* ZoomAction;

	/** The camera distance bounds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FZoomSettings", meta = (EditCondition = "bEnableZoom == true"))
	FVector2D DistanceBounds;

	/** Zoom speed (per second) of camera distance when zooming in or out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FZoomSettings", meta = (EditCondition = "bEnableZoom == true"))
	float Speed;

	/** Zoom damp time. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|FZoomSettings", meta = (EditCondition = "bEnableZoom == true"))
	float DampTime;

	FZoomSettings()
		: bEnableZoom(false)
		, ZoomAction(nullptr)
		, DistanceBounds(FVector2D(20.0, 1000.0))
		, Speed(100.f)
		, DampTime(0.2f)
	{ }
};