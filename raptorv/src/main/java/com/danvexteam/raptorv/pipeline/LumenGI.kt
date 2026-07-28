package com.danvexteam.raptorv.pipeline

import com.danvexteam.raptorv.Engine
import com.danvexteam.raptorv.types.AmbientOcclusionOptions
import com.danvexteam.raptorv.types.AmbientOcclusionType
import com.danvexteam.raptorv.types.DynamicResolutionOptions
import com.danvexteam.raptorv.types.QualityLevel
import com.danvexteam.raptorv.types.ScreenSpaceReflectionsOptions
import com.danvexteam.raptorv.types.Vec2

data class LumenConfig(
    var giQuality: QualityLevel = QualityLevel.HIGH,
    var enableSSR: Boolean = true,
    var ssrThickness: Float = 0.1f,
    var enableSSCT: Boolean = true,
    var enableFSR: Boolean = true,
    var fsrRenderScale: Float = 0.75f,
    var fsrSharpness: Float = 0.8f
)

fun Engine.enableLumenGI(config: LumenConfig = LumenConfig()) {
    val aoOpts = AmbientOcclusionOptions(
        enabled = true,
        aoType = AmbientOcclusionType.GTAO,
        radius = 0.5f,
        power = 1.2f,
        quality = config.giQuality,
        bentNormals = true,
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

    val ssrOpts = ScreenSpaceReflectionsOptions(
        enabled = config.enableSSR,
        thickness = 0.3f,
        bias = 0.01f,
        maxDistance = 15.0f,
        stride = 2.0f
    )
    setSSR(ssrOpts)

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
