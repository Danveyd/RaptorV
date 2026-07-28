package com.danvexteam.raptorv.pipeline

import com.danvexteam.raptorv.Engine
import com.danvexteam.raptorv.types.*

data class LumenConfig(
    var giQuality: QualityLevel = QualityLevel.HIGH,
    var enableSSR: Boolean = true,
    var ssrThickness: Float = 0.2f,
    var enableSSCT: Boolean = true,
    var enableFSR: Boolean = true,
    var fsrRenderScale: Float = 0.45f,
    var fsrSharpness: Float = 0.8f
)

fun Engine.enableLumenGI(config: LumenConfig = LumenConfig()) {
    val aoOpts = AmbientOcclusionOptions(
        enabled = true,
        aoType = AmbientOcclusionType.GTAO,
        radius = 0.5f,
        power = 1.2f,
        quality = config.giQuality,
        bentNormals = false,
        resolution = 0.5f,
        gtaoSliceCount = if (config.giQuality == QualityLevel.ULTRA) 4 else 2,
        gtaoStepsPerSlice = 3,

        ssctEnabled = config.enableSSCT,
        ssctConeRad = 0.8f,
        ssctShadowDistance = 0.3f,
        ssctContactDistanceMax = 0.8f,
        ssctIntensity = 0.8f,
        ssctSampleCount = 4,
        ssctRayCount = 1
    )
    setAmbientOcclusion(aoOpts)

    if (config.enableSSR) {
        val ssrOpts = ScreenSpaceReflectionsOptions(
            enabled = true,
            thickness = config.ssrThickness,
            bias = 0.01f,
            maxDistance = 5.0f,
            stride = 2.0f
        )
        setSSR(ssrOpts)
    } else {
        setSSR(ScreenSpaceReflectionsOptions(enabled = false))
    }

    if (config.enableFSR) {
        val dynOpts = DynamicResolutionOptions(
            enabled = true,
            homogeneousScaling = true,
            minScale = Vec2(config.fsrRenderScale, config.fsrRenderScale),
            maxScale = Vec2(1.0f, 1.0f),
            sharpness = config.fsrSharpness,
            quality = QualityLevel.ULTRA
        )
        setDynamicResolution(dynOpts)
    }
}
