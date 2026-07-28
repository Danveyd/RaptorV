package com.danvexteam.raptorv

import android.content.Context
import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.AmbientOcclusionOptions
import com.danvexteam.raptorv.types.AntiAliasingMode
import com.danvexteam.raptorv.types.BloomOptions
import com.danvexteam.raptorv.types.CameraDesc
import com.danvexteam.raptorv.types.ColorGradingOptions
import com.danvexteam.raptorv.types.DepthOfFieldOptions
import com.danvexteam.raptorv.types.DynamicResolutionOptions
import com.danvexteam.raptorv.types.FogOptions
import com.danvexteam.raptorv.types.LightDesc
import com.danvexteam.raptorv.types.QualityLevel
import com.danvexteam.raptorv.types.ScreenSpaceReflectionsOptions
import com.danvexteam.raptorv.types.ShadowType
import com.danvexteam.raptorv.types.SoftShadowOptions
import com.danvexteam.raptorv.types.TemporalAntiAliasingOptions
import com.danvexteam.raptorv.types.VignetteOptions
import com.danvexteam.raptorv.types.VsmShadowOptions

class Engine(context: Context) : AutoCloseable {
    internal val handle: Long = RaptorNative.createEngine(context.assets)

    private var activeScene: Scene? = null

    fun createScene(): Scene = Scene(this, RaptorNative.createScene(handle))

    fun setActiveScene(scene: Scene) {
        activeScene = scene
        RaptorNative.setActiveScene(handle, scene.handle)
    }

    fun loadMesh(path: String): Long {
        return RaptorNative.loadMesh(handle, path)
    }

    fun createCamera(desc: CameraDesc = CameraDesc()): Camera {
        val id = RaptorNative.createCamera(
            handle,
            desc.projectionType.ordinal, desc.fovDegrees, desc.focalLength, desc.nearPlane, desc.farPlane, desc.focusDistance,
            desc.position.x, desc.position.y, desc.position.z,
            desc.target.x, desc.target.y, desc.target.z,
            desc.up.x, desc.up.y, desc.up.z,
            desc.aperture, desc.shutterSpeed, desc.sensitivityISO,
            desc.shiftX, desc.shiftY,
            desc.orthoLeft, desc.orthoRight, desc.orthoBottom, desc.orthoTop
        )
        return Camera(this, id)
    }

    fun createLight(desc: LightDesc = LightDesc()): Light {
        val id = RaptorNative.createLight(
            handle, desc.type.ordinal,
            desc.color.x, desc.color.y, desc.color.z, desc.intensity,
            desc.direction.x, desc.direction.y, desc.direction.z,
            desc.position.x, desc.position.y, desc.position.z,
            desc.falloffRadius, desc.castShadows, desc.enableContactShadows,
            desc.innerConeAngle, desc.outerConeAngle,
            desc.shadowMapSize, desc.shadowCascades, desc.shadowConstantBias, desc.shadowNormalBias,
            desc.sunAngularRadius, desc.sunHaloSize, desc.sunHaloFalloff,
            desc.cascadeSplit1, desc.cascadeSplit2, desc.cascadeSplit3
        )
        return Light(this, id)
    }

    fun setShadowType(type: ShadowType) {
        RaptorNative.setShadowType(handle, type.ordinal)
    }

    fun setVsmShadowOptions(opts: VsmShadowOptions) {
        RaptorNative.setVsmShadowOptions(
            handle,
            opts.anisotropy, opts.mipmapping, opts.msaaSamples, opts.highPrecision, opts.lightBleedReduction
        )
    }

    fun createMesh(vertices: FloatArray, indices: IntArray): Long {
        return RaptorNative.createMesh(handle, vertices, indices)
    }

    internal fun tick(dt: Float) {
        activeScene?.update(dt)
        RaptorNative.tick(handle, dt)
    }

    override fun close() {
        RaptorNative.destroyEngine(handle)
    }

    fun setGuardBand(enabled: Boolean) {
        RaptorNative.setGuardBandOptions(handle, enabled)
    }

    fun setVisibleLayers(select: Int, values: Int) {
        RaptorNative.setViewVisibleLayers(handle, select, values)
    }

    fun setPostProcessingEnabled(enabled: Boolean) {
        RaptorNative.setPostProcessingEnabled(handle, enabled)
    }

    fun setMirrorMode(enabled: Boolean) {
        RaptorNative.setViewFrontFaceWindingInverted(handle, enabled)
    }

    fun pickEntity(x: Int, y: Int): Long {
        return RaptorNative.pickEntityAt(handle, x, y)
    }

    fun setAntiAliasing(mode: AntiAliasingMode) {
        RaptorNative.setAntiAliasing(handle, mode.ordinal)
    }

    fun setNightAdaptation(adaptation: Float) {
        RaptorNative.setNightAdaptation(handle, adaptation)
    }

    fun setMSAA(enabled: Boolean, sampleCount: Int = 4) {
        RaptorNative.setMSAAOptions(handle, enabled, sampleCount)
    }

    fun setAmbientOcclusion(opts: AmbientOcclusionOptions) {
        RaptorNative.setAmbientOcclusionBasic(
            handle, opts.enabled, opts.aoType.ordinal, opts.radius, opts.power, opts.bias,
            opts.intensity, opts.quality.ordinal, opts.bentNormals, opts.resolution
        )
        RaptorNative.setSSCTOptions(
            handle, opts.ssctEnabled, opts.ssctConeRad, opts.ssctShadowDistance, opts.ssctContactDistanceMax,
            opts.ssctIntensity, opts.ssctLightDirection.x, opts.ssctLightDirection.y, opts.ssctLightDirection.z
        )
        RaptorNative.setGTAOOptions(
            handle, opts.gtaoSliceCount, opts.gtaoStepsPerSlice, opts.gtaoThicknessHeuristic
        )
    }

    fun setScreenSpaceRefraction(enabled: Boolean) {
        RaptorNative.setScreenSpaceRefractionEnabled(handle, enabled)
    }

    fun setDithering(enabled: Boolean) {
        RaptorNative.setDitheringEnabled(handle, enabled)
    }

    fun setBloom(opts: BloomOptions) {
        RaptorNative.setBloomOptions(
            handle,
            opts.enabled, opts.strength, opts.resolution, opts.levels, opts.blendMode.ordinal,
            opts.threshold, opts.highlight, opts.quality.ordinal, opts.lensFlare, opts.starburst,
            opts.chromaticAberration, opts.ghostCount, opts.ghostSpacing, opts.ghostThreshold,
            opts.haloThickness, opts.haloRadius, opts.haloThreshold, opts.dirtStrength
        )
    }

    fun setTAA(opts: TemporalAntiAliasingOptions) {
        RaptorNative.setTemporalAntiAliasingOptions(
            handle,
            opts.enabled, opts.feedback, opts.lodBias, opts.sharpness, opts.upscaling,
            opts.filterHistory, opts.filterInput, opts.useYCoCg, opts.hdr,
            opts.boxType.ordinal, opts.boxClipping.ordinal, opts.jitterPattern.ordinal,
            opts.varianceGamma, opts.preventFlickering, opts.historyReprojection
        )
    }

    fun setFog(opts: FogOptions) {
        RaptorNative.setFogOptions(
            handle,
            opts.enabled, opts.distance, opts.cutOffDistance, opts.maximumOpacity,
            opts.height, opts.heightFalloff, opts.color.x, opts.color.y, opts.color.z,
            opts.density, opts.inScatteringStart, opts.inScatteringSize, opts.fogColorFromIbl
        )
    }

    fun setDepthOfField(opts: DepthOfFieldOptions) {
        RaptorNative.setDepthOfFieldOptions(
            handle,
            opts.enabled, opts.cocScale, opts.cocAspectRatio, opts.maxApertureDiameter,
            opts.filter.ordinal, opts.nativeResolution, opts.foregroundRingCount,
            opts.backgroundRingCount, opts.fastGatherRingCount, opts.maxForegroundCOC, opts.maxBackgroundCOC
        )
    }

    fun setVignette(opts: VignetteOptions) {
        RaptorNative.setVignetteOptions(
            handle,
            opts.enabled, opts.midPoint, opts.roundness, opts.feather,
            opts.color.x, opts.color.y, opts.color.z, opts.color.w
        )
    }

    fun setDynamicResolution(opts: DynamicResolutionOptions) {
        RaptorNative.setDynamicResolutionOptions(
            handle,
            opts.enabled, opts.homogeneousScaling, opts.minScale.x, opts.minScale.y,
            opts.maxScale.x, opts.maxScale.y, opts.sharpness, opts.quality.ordinal
        )
    }

    fun setSSR(opts: ScreenSpaceReflectionsOptions) {
        RaptorNative.setScreenSpaceReflectionsOptions(
            handle,
            opts.enabled, opts.thickness, opts.bias, opts.maxDistance, opts.stride
        )
    }

    fun setSoftShadows(opts: SoftShadowOptions) {
        RaptorNative.setSoftShadowOptions(handle, opts.penumbraScale, opts.penumbraRatioScale)
    }

    fun setColorGrading(opts: ColorGradingOptions) {
        RaptorNative.setColorGradingOptions(
            handle,
            opts.toneMapping.ordinal, opts.exposure, opts.whiteBalanceTemp, opts.whiteBalanceTint,
            opts.contrast, opts.vibrance, opts.saturation,
            opts.shadows.x, opts.shadows.y, opts.shadows.z, opts.shadows.w,
            opts.midtones.x, opts.midtones.y, opts.midtones.z, opts.midtones.w,
            opts.highlights.x, opts.highlights.y, opts.highlights.z, opts.highlights.w,
            opts.tonalRanges.x, opts.tonalRanges.y, opts.tonalRanges.z, opts.tonalRanges.w,
            opts.slope.x, opts.slope.y, opts.slope.z,
            opts.offset.x, opts.offset.y, opts.offset.z,
            opts.power.x, opts.power.y, opts.power.z
        )
    }

    fun getAnimationCount(meshHandle: Long): Int {
        return RaptorNative.getAnimationCount(handle, meshHandle)
    }

    fun getAnimationName(meshHandle: Long, index: Int): String {
        return RaptorNative.getAnimationName(handle, meshHandle, index)
    }

    fun getAnimationDuration(meshHandle: Long, index: Int): Float {
        return RaptorNative.getAnimationDuration(handle, meshHandle, index)
    }

    fun compileCustomShader(glslCode: String): Long {
        return RaptorNative.compileMaterialFromGLSL(handle, glslCode)
    }
}
