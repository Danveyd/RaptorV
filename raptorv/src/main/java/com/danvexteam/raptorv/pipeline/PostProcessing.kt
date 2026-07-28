package com.danvexteam.raptorv.pipeline

import com.danvexteam.raptorv.Engine
import com.danvexteam.raptorv.types.*

class BloomScope {
    var enabled: Boolean = true
    var strength: Float = 0.10f
    var resolution: Int = 384
    var levels: Int = 6
    var blendMode: BloomBlendMode = BloomBlendMode.ADD
    var threshold: Boolean = true
    var highlight: Float = 1000.0f
    var quality: QualityLevel = QualityLevel.LOW
    var lensFlare: Boolean = false
    var starburst: Boolean = true
    var chromaticAberration: Float = 0.005f
    var dirtStrength: Float = 0.2f

    internal fun build(): BloomOptions = BloomOptions(
        enabled = enabled,
        strength = strength,
        resolution = resolution,
        levels = levels,
        blendMode = blendMode,
        threshold = threshold,
        highlight = highlight,
        quality = quality,
        lensFlare = lensFlare,
        starburst = starburst,
        chromaticAberration = chromaticAberration,
        dirtStrength = dirtStrength
    )
}

class AmbientOcclusionScope {
    var enabled: Boolean = true
    var type: AmbientOcclusionType = AmbientOcclusionType.GTAO
    var radius: Float = 0.5f
    var power: Float = 1.2f
    var bias: Float = 0.0005f
    var intensity: Float = 1.0f
    var quality: QualityLevel = QualityLevel.HIGH
    var bentNormals: Boolean = false
    var resolution: Float = 0.5f
    var gtaoSliceCount: Int = 2
    var gtaoStepsPerSlice: Int = 3

    var enableSSCT: Boolean = true
    var ssctConeRad: Float = 0.8f
    var ssctShadowDistance: Float = 0.3f
    var ssctContactDistanceMax: Float = 0.8f
    var ssctIntensity: Float = 0.8f

    internal fun build(): AmbientOcclusionOptions = AmbientOcclusionOptions(
        enabled = enabled,
        aoType = type,
        radius = radius,
        power = power,
        bias = bias,
        intensity = intensity,
        quality = quality,
        bentNormals = bentNormals,
        resolution = resolution,
        gtaoSliceCount = gtaoSliceCount,
        gtaoStepsPerSlice = gtaoStepsPerSlice,
        ssctEnabled = enableSSCT,
        ssctConeRad = ssctConeRad,
        ssctShadowDistance = ssctShadowDistance,
        ssctContactDistanceMax = ssctContactDistanceMax,
        ssctIntensity = ssctIntensity
    )
}

class FogScope {
    var enabled: Boolean = false
    var distance: Float = 10.0f
    var cutOffDistance: Float = Float.POSITIVE_INFINITY
    var maximumOpacity: Float = 1.0f
    var height: Float = 0.0f
    var heightFalloff: Float = 1.0f
    var color: Vec3 = Vec3(0.8f, 0.85f, 0.9f)
    var density: Float = 0.04f
    var inScatteringStart: Float = 0.0f
    var inScatteringSize: Float = -1.0f

    internal fun build(): FogOptions = FogOptions(
        enabled = enabled,
        distance = distance,
        cutOffDistance = cutOffDistance,
        maximumOpacity = maximumOpacity,
        height = height,
        heightFalloff = heightFalloff,
        color = color,
        density = density,
        inScatteringStart = inScatteringStart,
        inScatteringSize = inScatteringSize
    )
}

class ColorGradingScope {
    var toneMapping: ToneMappingMode = ToneMappingMode.ACES
    var exposure: Float = 0.0f
    var contrast: Float = 1.0f
    var vibrance: Float = 1.0f
    var saturation: Float = 1.0f
    var whiteBalanceTemp: Float = 0.0f
    var whiteBalanceTint: Float = 0.0f

    var shadows: Vec4 = Vec4(1f, 1f, 1f, 0f)
    var midtones: Vec4 = Vec4(1f, 1f, 1f, 0f)
    var highlights: Vec4 = Vec4(1f, 1f, 1f, 0f)

    internal fun build(): ColorGradingOptions = ColorGradingOptions(
        toneMapping = toneMapping,
        exposure = exposure,
        contrast = contrast,
        vibrance = vibrance,
        saturation = saturation,
        whiteBalanceTemp = whiteBalanceTemp,
        whiteBalanceTint = whiteBalanceTint,
        shadows = shadows,
        midtones = midtones,
        highlights = highlights
    )
}

class SSRScope {
    var enabled: Boolean = false
    var thickness: Float = 0.2f
    var bias: Float = 0.01f
    var maxDistance: Float = 5.0f
    var stride: Float = 2.0f

    internal fun build(): ScreenSpaceReflectionsOptions = ScreenSpaceReflectionsOptions(
        enabled = enabled,
        thickness = thickness,
        bias = bias,
        maxDistance = maxDistance,
        stride = stride
    )
}

class DynamicResolutionScope {
    var enabled: Boolean = true
    var scale: Float = 0.5f
    var sharpness: Float = 0.8f
    var quality: QualityLevel = QualityLevel.ULTRA

    internal fun build(): DynamicResolutionOptions = DynamicResolutionOptions(
        enabled = enabled,
        homogeneousScaling = true,
        minScale = Vec2(scale, scale),
        maxScale = Vec2(1.0f, 1.0f),
        sharpness = sharpness,
        quality = quality
    )
}

class DepthOfFieldScope {
    var enabled: Boolean = false
    var cocScale: Float = 1.0f
    var cocAspectRatio: Float = 1.0f
    var maxApertureDiameter: Float = 0.01f
    var filter: DoFFilter = DoFFilter.MEDIAN

    internal fun build(): DepthOfFieldOptions = DepthOfFieldOptions(
        enabled = enabled,
        cocScale = cocScale,
        cocAspectRatio = cocAspectRatio,
        maxApertureDiameter = maxApertureDiameter,
        filter = filter
    )
}

class VignetteScope {
    var enabled: Boolean = false
    var midPoint: Float = 0.5f
    var roundness: Float = 0.5f
    var feather: Float = 0.5f
    var color: Vec4 = Vec4(0f, 0f, 0f, 1f)

    internal fun build(): VignetteOptions = VignetteOptions(
        enabled = enabled,
        midPoint = midPoint,
        roundness = roundness,
        feather = feather,
        color = color
    )
}

class TAAScope {
    var enabled: Boolean = false
    var feedback: Float = 0.12f
    var lodBias: Float = -1.0f
    var sharpness: Float = 0.0f
    var preventFlickering: Boolean = false
    var historyReprojection: Boolean = true

    internal fun build(): TemporalAntiAliasingOptions = TemporalAntiAliasingOptions(
        enabled = enabled,
        feedback = feedback,
        lodBias = lodBias,
        sharpness = sharpness,
        preventFlickering = preventFlickering,
        historyReprojection = historyReprojection
    )
}

class SoftShadowsScope {
    var penumbraScale: Float = 1.0f
    var penumbraRatioScale: Float = 1.0f

    internal fun build(): SoftShadowOptions = SoftShadowOptions(
        penumbraScale = penumbraScale,
        penumbraRatioScale = penumbraRatioScale
    )
}

class VSMShadowsScope {
    var anisotropy: Int = 0
    var mipmapping: Boolean = false
    var msaaSamples: Int = 1
    var highPrecision: Boolean = false
    var lightBleedReduction: Float = 0.15f

    internal fun build(): VsmShadowOptions = VsmShadowOptions(
        anisotropy = anisotropy,
        mipmapping = mipmapping,
        msaaSamples = msaaSamples,
        highPrecision = highPrecision,
        lightBleedReduction = lightBleedReduction
    )
}

class MSAAScope {
    var enabled: Boolean = false
    var sampleCount: Int = 4
}

class PostProcessingScope {
    private var bloomScope: BloomScope? = null
    private var aoScope: AmbientOcclusionScope? = null
    private var fogScope: FogScope? = null
    private var colorGradingScope: ColorGradingScope? = null
    private var ssrScope: SSRScope? = null
    private var dynamicResScope: DynamicResolutionScope? = null
    private var dofScope: DepthOfFieldScope? = null
    private var vignetteScope: VignetteScope? = null
    private var taaScope: TAAScope? = null
    private var softShadowsScope: SoftShadowsScope? = null
    private var vsmShadowsScope: VSMShadowsScope? = null
    private var msaaScope: MSAAScope? = null

    var shadowType: ShadowType? = null
    var dithering: Boolean? = null
    var screenSpaceRefraction: Boolean? = null
    var nightAdaptation: Float? = null
    var antiAliasing: AntiAliasingMode? = null

    fun fsr(scale: Float = 0.5f, sharpness: Float = 0.8f) {
        dynamicResScope = DynamicResolutionScope().apply {
            this.enabled = true
            this.scale = scale
            this.sharpness = sharpness
        }
    }

    fun bloom(block: BloomScope.() -> Unit) { bloomScope = BloomScope().apply(block) }
    fun ambientOcclusion(block: AmbientOcclusionScope.() -> Unit) { aoScope = AmbientOcclusionScope().apply(block) }
    fun ao(block: AmbientOcclusionScope.() -> Unit) { ambientOcclusion(block) }
    fun fog(block: FogScope.() -> Unit) { fogScope = FogScope().apply(block) }
    fun colorGrading(block: ColorGradingScope.() -> Unit) { colorGradingScope = ColorGradingScope().apply(block) }
    fun ssr(block: SSRScope.() -> Unit) { ssrScope = SSRScope().apply(block) }
    fun dynamicResolution(block: DynamicResolutionScope.() -> Unit) { dynamicResScope = DynamicResolutionScope().apply(block) }
    fun depthOfField(block: DepthOfFieldScope.() -> Unit) { dofScope = DepthOfFieldScope().apply(block) }
    fun dof(block: DepthOfFieldScope.() -> Unit) { depthOfField(block) }
    fun vignette(block: VignetteScope.() -> Unit) { vignetteScope = VignetteScope().apply(block) }
    fun taa(block: TAAScope.() -> Unit) { taaScope = TAAScope().apply(block) }
    fun softShadows(block: SoftShadowsScope.() -> Unit) { softShadowsScope = SoftShadowsScope().apply(block) }
    fun vsmShadows(block: VSMShadowsScope.() -> Unit) { vsmShadowsScope = VSMShadowsScope().apply(block) }
    fun msaa(enabled: Boolean = true, sampleCount: Int = 4) {
        msaaScope = MSAAScope().apply {
            this.enabled = enabled
            this.sampleCount = sampleCount
        }
    }

    internal fun applyTo(engine: Engine) {
        bloomScope?.let { engine.setBloom(it.build()) }
        aoScope?.let { engine.setAmbientOcclusion(it.build()) }
        fogScope?.let { engine.setFog(it.build()) }
        colorGradingScope?.let { engine.setColorGrading(it.build()) }
        ssrScope?.let { engine.setSSR(it.build()) }
        dynamicResScope?.let { engine.setDynamicResolution(it.build()) }
        dofScope?.let { engine.setDepthOfField(it.build()) }
        vignetteScope?.let { engine.setVignette(it.build()) }
        taaScope?.let { engine.setTAA(it.build()) }
        softShadowsScope?.let { engine.setSoftShadows(it.build()) }
        vsmShadowsScope?.let { engine.setVsmShadowOptions(it.build()) }
        msaaScope?.let { engine.setMSAA(it.enabled, it.sampleCount) }

        shadowType?.let { engine.setShadowType(it) }
        dithering?.let { engine.setDithering(it) }
        screenSpaceRefraction?.let { engine.setScreenSpaceRefraction(it) }
        nightAdaptation?.let { engine.setNightAdaptation(it) }
        antiAliasing?.let { engine.setAntiAliasing(it) }
    }
}

fun Engine.postProcessing(block: PostProcessingScope.() -> Unit) {
    val scope = PostProcessingScope().apply(block)
    scope.applyTo(this)
}
