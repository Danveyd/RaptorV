package com.danvexteam.raptorv.pipeline

import com.danvexteam.raptorv.Engine
import com.danvexteam.raptorv.types.*

class BloomScope {
    var enabled: Boolean = true
    var strength: Float = 0.10f
    var lensFlare: Boolean = false
    var starburst: Boolean = true
    var threshold: Boolean = true
    var highlight: Float = 1000.0f
    var quality: QualityLevel = QualityLevel.LOW

    internal fun build(): BloomOptions = BloomOptions(
        enabled = enabled,
        strength = strength,
        lensFlare = lensFlare,
        starburst = starburst,
        threshold = threshold,
        highlight = highlight,
        quality = quality
    )
}

class AmbientOcclusionScope {
    var enabled: Boolean = true
    var type: AmbientOcclusionType = AmbientOcclusionType.GTAO
    var radius: Float = 0.5f
    var power: Float = 1.2f
    var quality: QualityLevel = QualityLevel.HIGH
    var bentNormals: Boolean = true
    var resolution: Float = 0.5f
    var enableSSCT: Boolean = true
    var ssctIntensity: Float = 0.8f

    internal fun build(): AmbientOcclusionOptions = AmbientOcclusionOptions(
        enabled = enabled,
        aoType = type,
        radius = radius,
        power = power,
        quality = quality,
        bentNormals = bentNormals,
        resolution = resolution,
        ssctEnabled = enableSSCT,
        ssctIntensity = ssctIntensity
    )
}

class FogScope {
    var enabled: Boolean = false
    var distance: Float = 10.0f
    var color: Vec3 = Vec3(0.8f, 0.85f, 0.9f)
    var density: Float = 0.04f

    internal fun build(): FogOptions = FogOptions(
        enabled = enabled,
        distance = distance,
        color = color,
        density = density
    )
}

class ColorGradingScope {
    var toneMapping: ToneMappingMode = ToneMappingMode.ACES
    var exposure: Float = 0.0f
    var contrast: Float = 1.0f
    var vibrance: Float = 1.0f
    var saturation: Float = 1.0f
    var whiteBalanceTemp: Float = 0.0f

    internal fun build(): ColorGradingOptions = ColorGradingOptions(
        toneMapping = toneMapping,
        exposure = exposure,
        contrast = contrast,
        vibrance = vibrance,
        saturation = saturation,
        whiteBalanceTemp = whiteBalanceTemp
    )
}

class SSRScope {
    var enabled: Boolean = false
    var thickness: Float = 0.2f
    var maxDistance: Float = 4.0f
    var stride: Float = 3.0f

    internal fun build(): ScreenSpaceReflectionsOptions = ScreenSpaceReflectionsOptions(
        enabled = enabled,
        thickness = thickness,
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

class PostProcessingScope {
    private var bloomScope: BloomScope? = null
    private var aoScope: AmbientOcclusionScope? = null
    private var fogScope: FogScope? = null
    private var colorGradingScope: ColorGradingScope? = null
    private var ssrScope: SSRScope? = null
    private var dynamicResScope: DynamicResolutionScope? = null

    fun fsr(scale: Float = 0.5f, sharpness: Float = 0.8f) {
        dynamicResScope = DynamicResolutionScope().apply {
            this.enabled = true
            this.scale = scale
            this.sharpness = sharpness
        }
    }

    fun dynamicResolution(block: DynamicResolutionScope.() -> Unit) {
        dynamicResScope = DynamicResolutionScope().apply(block)
    }

    fun bloom(block: BloomScope.() -> Unit) {
        bloomScope = BloomScope().apply(block)
    }

    fun ambientOcclusion(block: AmbientOcclusionScope.() -> Unit) {
        aoScope = AmbientOcclusionScope().apply(block)
    }

    fun fog(block: FogScope.() -> Unit) {
        fogScope = FogScope().apply(block)
    }

    fun colorGrading(block: ColorGradingScope.() -> Unit) {
        colorGradingScope = ColorGradingScope().apply(block)
    }

    fun ssr(block: SSRScope.() -> Unit) {
        ssrScope = SSRScope().apply(block)
    }

    internal fun applyTo(engine: Engine) {
        bloomScope?.let { engine.setBloom(it.build()) }
        aoScope?.let { engine.setAmbientOcclusion(it.build()) }
        fogScope?.let { engine.setFog(it.build()) }
        colorGradingScope?.let { engine.setColorGrading(it.build()) }
        ssrScope?.let { engine.setSSR(it.build()) }
        dynamicResScope?.let { engine.setDynamicResolution(it.build()) }
    }
}

fun Engine.postProcessing(block: PostProcessingScope.() -> Unit) {
    val scope = PostProcessingScope().apply(block)
    scope.applyTo(this)
}
