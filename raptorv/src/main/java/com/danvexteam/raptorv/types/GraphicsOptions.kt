package com.danvexteam.raptorv.types

enum class QualityLevel { LOW, MEDIUM, HIGH, ULTRA }
enum class AmbientOcclusionType { SAO, GTAO }
enum class BloomBlendMode { ADD, INTERPOLATE }
enum class DoFFilter { NONE, UNUSED, MEDIAN }
enum class TAABoxType { AABB, AABB_VARIANCE }
enum class TAABoxClipping { ACCURATE, CLAMP, NONE }
enum class TAAJitterPattern { RGSS_X4, UNIFORM_HELIX_X4, HALTON_23_X8, HALTON_23_X16, HALTON_23_X32 }
enum class AntiAliasingMode { NONE, FXAA }

data class AmbientOcclusionOptions(
    var enabled: Boolean = false,
    var aoType: AmbientOcclusionType = AmbientOcclusionType.SAO,
    var radius: Float = 0.3f,
    var power: Float = 1.0f,
    var bias: Float = 0.0005f,
    var resolution: Float = 0.5f,
    var intensity: Float = 1.0f,
    var bilateralThreshold: Float = 0.05f,
    var quality: QualityLevel = QualityLevel.LOW,
    var lowPassFilter: QualityLevel = QualityLevel.MEDIUM,
    var upsampling: QualityLevel = QualityLevel.LOW,
    var bentNormals: Boolean = false,
    var minHorizonAngleRad: Float = 0.0f,

    var ssctEnabled: Boolean = false,
    var ssctConeRad: Float = 1.0f,
    var ssctShadowDistance: Float = 0.3f,
    var ssctContactDistanceMax: Float = 1.0f,
    var ssctIntensity: Float = 0.8f,
    var ssctLightDirection: Vec3 = Vec3(0f, -1f, 0f),
    var ssctDepthBias: Float = 0.01f,
    var ssctDepthSlopeBias: Float = 0.01f,
    var ssctSampleCount: Int = 4,
    var ssctRayCount: Int = 1,

    var gtaoSliceCount: Int = 4,
    var gtaoStepsPerSlice: Int = 3,
    var gtaoThicknessHeuristic: Float = 0.004f,
    var gtaoUseVisibilityBitmasks: Boolean = false,
    var gtaoConstThickness: Float = 0.5f,
    var gtaoLinearThickness: Boolean = false
)

data class BloomOptions(
    var enabled: Boolean = false,
    var strength: Float = 0.10f,
    var resolution: Int = 384,
    var levels: Int = 6,
    var blendMode: BloomBlendMode = BloomBlendMode.ADD,
    var threshold: Boolean = true,
    var highlight: Float = 1000.0f,
    var quality: QualityLevel = QualityLevel.LOW,
    var lensFlare: Boolean = false,
    var starburst: Boolean = true,
    var chromaticAberration: Float = 0.005f,
    var ghostCount: Int = 4,
    var ghostSpacing: Float = 0.6f,
    var ghostThreshold: Float = 10.0f,
    var haloThickness: Float = 0.1f,
    var haloRadius: Float = 0.4f,
    var haloThreshold: Float = 10.0f,
    var dirtStrength: Float = 0.2f
)

data class TemporalAntiAliasingOptions(
    var enabled: Boolean = false,
    var feedback: Float = 0.12f,
    var lodBias: Float = -1.0f,
    var sharpness: Float = 0.0f,
    var upscaling: Float = 1.0f,
    var filterHistory: Boolean = true,
    var filterInput: Boolean = true,
    var useYCoCg: Boolean = false,
    var hdr: Boolean = true,
    var boxType: TAABoxType = TAABoxType.AABB,
    var boxClipping: TAABoxClipping = TAABoxClipping.ACCURATE,
    var jitterPattern: TAAJitterPattern = TAAJitterPattern.HALTON_23_X16,
    var varianceGamma: Float = 1.0f,
    var preventFlickering: Boolean = false,
    var historyReprojection: Boolean = true
)

data class FogOptions(
    var enabled: Boolean = false,
    var distance: Float = 0.0f,
    var cutOffDistance: Float = Float.POSITIVE_INFINITY,
    var maximumOpacity: Float = 1.0f,
    var height: Float = 0.0f,
    var heightFalloff: Float = 1.0f,
    var color: Vec3 = Vec3(1.0f, 1.0f, 1.0f),
    var density: Float = 0.1f,
    var inScatteringStart: Float = 0.0f,
    var inScatteringSize: Float = -1.0f,
    var fogColorFromIbl: Boolean = false
)

data class DepthOfFieldOptions(
    var enabled: Boolean = false,
    var cocScale: Float = 1.0f,
    var cocAspectRatio: Float = 1.0f,
    var maxApertureDiameter: Float = 0.01f,
    var filter: DoFFilter = DoFFilter.MEDIAN,
    var nativeResolution: Boolean = false,
    var foregroundRingCount: Int = 0,
    var backgroundRingCount: Int = 0,
    var fastGatherRingCount: Int = 0,
    var maxForegroundCOC: Int = 0,
    var maxBackgroundCOC: Int = 0
)

data class VignetteOptions(
    var enabled: Boolean = false,
    var midPoint: Float = 0.5f,
    var roundness: Float = 0.5f,
    var feather: Float = 0.5f,
    var color: Vec4 = Vec4(0f, 0f, 0f, 1f)
)

data class DynamicResolutionOptions(
    var enabled: Boolean = false,
    var homogeneousScaling: Boolean = false,
    var minScale: Vec2 = Vec2(0.5f, 0.5f),
    var maxScale: Vec2 = Vec2(1.0f, 1.0f),
    var sharpness: Float = 0.9f,
    var quality: QualityLevel = QualityLevel.LOW
)

data class ScreenSpaceReflectionsOptions(
    var enabled: Boolean = false,
    var thickness: Float = 0.1f,
    var bias: Float = 0.01f,
    var maxDistance: Float = 3.0f,
    var stride: Float = 2.0f
)

data class SoftShadowOptions(
    var penumbraScale: Float = 1.0f,
    var penumbraRatioScale: Float = 1.0f
)

enum class ToneMappingMode {
    LINEAR,
    ACES,
    ACES_LEGACY,
    FILMIC,
    AGX,
    PBR_NEUTRAL,
    DISPLAY_RANGE
}

data class ColorGradingOptions(
    var toneMapping: ToneMappingMode = ToneMappingMode.ACES,
    var exposure: Float = 0.0f,
    var whiteBalanceTemp: Float = 0.0f,
    var whiteBalanceTint: Float = 0.0f,
    var contrast: Float = 1.0f,
    var vibrance: Float = 1.0f,
    var saturation: Float = 1.0f,

    var shadows: Vec4 = Vec4(1f, 1f, 1f, 0f),
    var midtones: Vec4 = Vec4(1f, 1f, 1f, 0f),
    var highlights: Vec4 = Vec4(1f, 1f, 1f, 0f),
    var tonalRanges: Vec4 = Vec4(0.0f, 0.33f, 0.55f, 1.0f),

    var slope: Vec3 = Vec3(1f, 1f, 1f),
    var offset: Vec3 = Vec3(0f, 0f, 0f),
    var power: Vec3 = Vec3(1f, 1f, 1f)
)

enum class ShadowType { PCF, VSM, DPCF, PCSS }

data class VsmShadowOptions(
    var anisotropy: Int = 0,
    var mipmapping: Boolean = false,
    var msaaSamples: Int = 1,
    var highPrecision: Boolean = false,
    var lightBleedReduction: Float = 0.15f
)
