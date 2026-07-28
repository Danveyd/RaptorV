package com.danvexteam.raptorv.internal

import android.content.res.AssetManager
import android.view.Surface

internal object RaptorNative {
    init { System.loadLibrary("raptorv") }

    @JvmStatic external fun createEngine(assetManager: AssetManager): Long
    @JvmStatic external fun destroyEngine(engineHandle: Long)
    @JvmStatic external fun onSurfaceCreated(engineHandle: Long, surface: Surface)
    @JvmStatic external fun onSurfaceChanged(engineHandle: Long, width: Int, height: Int)
    @JvmStatic external fun onSurfaceDestroyed(engineHandle: Long)
    @JvmStatic external fun tick(engineHandle: Long, dt: Float)

    @JvmStatic external fun createScene(engineHandle: Long): Long
    @JvmStatic external fun setActiveScene(engineHandle: Long, sceneHandle: Long)
    @JvmStatic external fun destroyScene(sceneHandle: Long)

    @JvmStatic external fun createEntity(sceneHandle: Long, name: String): Long
    @JvmStatic external fun setTransform(sceneHandle: Long, entityId: Long, px: Float, py: Float, pz: Float, rx: Float, ry: Float, rz: Float, sx: Float, sy: Float, sz: Float)
    @JvmStatic external fun getTransformRotation(sceneHandle: Long, entityId: Long): FloatArray

    @JvmStatic external fun loadMesh(engineHandle: Long, path: String): Long
    @JvmStatic external fun attachMesh(sceneHandle: Long, entityId: Long, meshHandle: Long)
    @JvmStatic external fun attachRigidBody(sceneHandle: Long, entityId: Long, type: Int, arg1: Float, arg2: Float, arg3: Float, mass: Float)

    @JvmStatic external fun setColorGradingOptions(
        engineHandle: Long,
        toneMapping: Int, exposure: Float, whiteBalanceTemp: Float, whiteBalanceTint: Float,
        contrast: Float, vibrance: Float, saturation: Float,
        shadowsR: Float, shadowsG: Float, shadowsB: Float, shadowsW: Float,
        midtonesR: Float, midtonesG: Float, midtonesB: Float, midtonesW: Float,
        highlightsR: Float, highlightsG: Float, highlightsB: Float, highlightsW: Float,
        rangeX: Float, rangeY: Float, rangeZ: Float, rangeW: Float,
        slopeX: Float, slopeY: Float, slopeZ: Float,
        offsetX: Float, offsetY: Float, offsetZ: Float,
        powerX: Float, powerY: Float, powerZ: Float
    )

    @JvmStatic external fun setAmbientOcclusionBasic(
        engineHandle: Long, enabled: Boolean, aoType: Int, radius: Float, power: Float, bias: Float,
        intensity: Float, quality: Int, bentNormals: Boolean, resolution: Float
    )

    @JvmStatic external fun setSSCTOptions(
        engineHandle: Long, enabled: Boolean, coneRad: Float, shadowDist: Float, contactDistMax: Float,
        intensity: Float, dirX: Float, dirY: Float, dirZ: Float
    )

    @JvmStatic external fun setGTAOOptions(
        engineHandle: Long, sliceCount: Int, stepsPerSlice: Int, thicknessHeuristic: Float
    )

    @JvmStatic external fun setScreenSpaceRefractionEnabled(engineHandle: Long, enabled: Boolean)
    @JvmStatic external fun setDitheringEnabled(engineHandle: Long, enabled: Boolean)

    @JvmStatic external fun setBloomOptions(
        engineHandle: Long,
        enabled: Boolean, strength: Float, resolution: Int, levels: Int, blendMode: Int,
        threshold: Boolean, highlight: Float, quality: Int, lensFlare: Boolean, starburst: Boolean,
        chromaticAberration: Float, ghostCount: Int, ghostSpacing: Float, ghostThreshold: Float,
        haloThickness: Float, haloRadius: Float, haloThreshold: Float, dirtStrength: Float
    )

    @JvmStatic external fun setTemporalAntiAliasingOptions(
        engineHandle: Long,
        enabled: Boolean, feedback: Float, lodBias: Float, sharpness: Float, upscaling: Float,
        filterHistory: Boolean, filterInput: Boolean, useYCoCg: Boolean, hdr: Boolean,
        boxType: Int, boxClipping: Int, jitterPattern: Int, varianceGamma: Float,
        preventFlickering: Boolean, historyReprojection: Boolean
    )

    @JvmStatic external fun setFogOptions(
        engineHandle: Long,
        enabled: Boolean, distance: Float, cutOffDistance: Float, maximumOpacity: Float,
        height: Float, heightFalloff: Float, colorR: Float, colorG: Float, colorB: Float,
        density: Float, inScatteringStart: Float, inScatteringSize: Float, fogColorFromIbl: Boolean
    )

    @JvmStatic external fun setDepthOfFieldOptions(
        engineHandle: Long,
        enabled: Boolean, cocScale: Float, cocAspectRatio: Float, maxApertureDiameter: Float,
        filter: Int, nativeResolution: Boolean, fgRings: Int, bgRings: Int, fastGatherRings: Int,
        maxFgCoc: Int, maxBgCoc: Int
    )

    @JvmStatic external fun setVignetteOptions(
        engineHandle: Long,
        enabled: Boolean, midPoint: Float, roundness: Float, feather: Float,
        colorR: Float, colorG: Float, colorB: Float, colorA: Float
    )

    @JvmStatic external fun setDynamicResolutionOptions(
        engineHandle: Long,
        enabled: Boolean, homogeneousScaling: Boolean, minScaleX: Float, minScaleY: Float,
        maxScaleX: Float, maxScaleY: Float, sharpness: Float, quality: Int
    )

    @JvmStatic external fun setScreenSpaceReflectionsOptions(
        engineHandle: Long,
        enabled: Boolean, thickness: Float, bias: Float, maxDistance: Float, stride: Float
    )

    @JvmStatic external fun setSoftShadowOptions(
        engineHandle: Long,
        penumbraScale: Float, penumbraRatioScale: Float
    )

    @JvmStatic external fun setEnvironment(sceneHandle: Long, iblBytes: ByteArray?, iblSize: Int, skyboxBytes: ByteArray?, skyboxSize: Int)

    @JvmStatic external fun createCamera(
        engineHandle: Long,
        projectionType: Int, fov: Float, focalLength: Float, near: Float, far: Float, focusDist: Float,
        px: Float, py: Float, pz: Float, tx: Float, ty: Float, tz: Float, ux: Float, uy: Float, uz: Float,
        aperture: Float, shutterSpeed: Float, sensitivityISO: Float,
        shiftX: Float, shiftY: Float,
        orthoLeft: Float, orthoRight: Float, orthoBottom: Float, orthoTop: Float
    ): Long

    @JvmStatic external fun updateCamera(
        engineHandle: Long, cameraId: Long,
        projectionType: Int, fov: Float, focalLength: Float, near: Float, far: Float, focusDist: Float,
        px: Float, py: Float, pz: Float, tx: Float, ty: Float, tz: Float, ux: Float, uy: Float, uz: Float,
        aperture: Float, shutterSpeed: Float, sensitivityISO: Float,
        shiftX: Float, shiftY: Float,
        orthoLeft: Float, orthoRight: Float, orthoBottom: Float, orthoTop: Float
    )

    @JvmStatic external fun setMainCamera(engineHandle: Long, cameraId: Long)

    @JvmStatic external fun setIndirectLightIntensity(sceneHandle: Long, intensity: Float)

    @JvmStatic external fun setIndirectLightRotation(sceneHandle: Long, rotationY: Float)

    @JvmStatic external fun createLight(
        engineHandle: Long, type: Int, cx: Float, cy: Float, cz: Float, intensity: Float,
        dx: Float, dy: Float, dz: Float, px: Float, py: Float, pz: Float, falloff: Float, shadows: Boolean, enableContactShadows: Boolean,
        inner: Float, outer: Float,
        shadowMapSize: Int, shadowCascades: Int, shadowConstantBias: Float, shadowNormalBias: Float,
        sunAngularRadius: Float, sunHaloSize: Float, sunHaloFalloff: Float,
        split1: Float, split2: Float, split3: Float
    ): Long

    @JvmStatic external fun updateLight(
        engineHandle: Long, lightId: Long, type: Int, cx: Float, cy: Float, cz: Float, intensity: Float,
        dx: Float, dy: Float, dz: Float, px: Float, py: Float, pz: Float, falloff: Float, shadows: Boolean, enableContactShadows: Boolean,
        inner: Float, outer: Float,
        shadowMapSize: Int, shadowCascades: Int, shadowConstantBias: Float, shadowNormalBias: Float,
        sunAngularRadius: Float, sunHaloSize: Float, sunHaloFalloff: Float,
        split1: Float, split2: Float, split3: Float
    )

    @JvmStatic external fun setTransformBuffer(sceneHandle: Long, byteBuffer: java.nio.ByteBuffer?)

    @JvmStatic external fun setEntityParent(sceneHandle: Long, childId: Long, parentId: Long)

    @JvmStatic external fun setSkyboxColor(sceneHandle: Long, r: Float, g: Float, b: Float, a: Float)

    @JvmStatic external fun createTextureFromBitmap(engineHandle: Long, bitmap: android.graphics.Bitmap): Long
    @JvmStatic external fun destroyTexture(engineHandle: Long, textureHandle: Long)
    @JvmStatic external fun setMaterialParamTexture(
        sceneHandle: Long, entityId: Long, name: String, textureHandle: Long,
        minFilter: Int, magFilter: Int, wrapS: Int, wrapT: Int
    )

    @JvmStatic external fun setMaterialParamFloat(sceneHandle: Long, entityId: Long, name: String, value: Float)
    @JvmStatic external fun setMaterialParamVec2(sceneHandle: Long, entityId: Long, name: String, x: Float, y: Float)
    @JvmStatic external fun setMaterialParamColor(sceneHandle: Long, entityId: Long, name: String, r: Float, g: Float, b: Float, a: Float)
    @JvmStatic external fun setMaterialCullMode(sceneHandle: Long, entityId: Long, cullMode: Int)
    @JvmStatic external fun setMaterialDepthWrite(sceneHandle: Long, entityId: Long, enable: Boolean)

    @JvmStatic external fun createMesh(engineHandle: Long, vertices: FloatArray, indices: IntArray): Long

    @JvmStatic external fun setEntityCastShadows(sceneHandle: Long, entityId: Long, castShadows: Boolean)
    @JvmStatic external fun setEntityReceiveShadows(sceneHandle: Long, entityId: Long, receiveShadows: Boolean)
    @JvmStatic external fun setEntityPriority(sceneHandle: Long, entityId: Long, priority: Int)
    @JvmStatic external fun setEntityCulling(sceneHandle: Long, entityId: Long, culling: Boolean)
    @JvmStatic external fun setEntityLayerMask(sceneHandle: Long, entityId: Long, select: Int, value: Int)

    @JvmStatic external fun getAnimationCount(engineHandle: Long, meshHandle: Long): Int
    @JvmStatic external fun getAnimationName(engineHandle: Long, meshHandle: Long, index: Int): String
    @JvmStatic external fun getAnimationDuration(engineHandle: Long, meshHandle: Long, index: Int): Float
    @JvmStatic external fun applyAnimation(
        sceneHandle: Long, entityId: Long,
        animationIndex: Int, timeSeconds: Float,
        enableCrossFade: Boolean = false, prevAnimationIndex: Int = 0, prevTimeSeconds: Float = 0f, alpha: Float = 0f
    )

    @JvmStatic external fun setShadowType(engineHandle: Long, shadowType: Int)
    @JvmStatic external fun setVsmShadowOptions(
        engineHandle: Long,
        anisotropy: Int, mipmapping: Boolean, msaaSamples: Int, highPrecision: Boolean, lightBleedReduction: Float
    )

    @JvmStatic external fun createInstancedEntity(
        sceneHandle: Long, name: String, meshHandle: Long, transforms: FloatArray?, instanceCount: Int
    ): LongArray

    @JvmStatic external fun updateInstanceTransforms(
        instanceBufferHandle: Long, transforms: FloatArray, count: Int
    )

    @JvmStatic external fun setLightPositionAndDirection(
        engineHandle: Long, lightId: Long,
        px: Float, py: Float, pz: Float, dx: Float, dy: Float, dz: Float
    )

    @JvmStatic external fun setLightChannel(engineHandle: Long, lightId: Long, channel: Int, enable: Boolean)

    @JvmStatic external fun createCubemapFromHdrBytes(engineHandle: Long, bytes: ByteArray, size: Int): Long
    @JvmStatic external fun createCubemapFromEquirectangularBitmap(engineHandle: Long, bitmap: android.graphics.Bitmap): Long
    @JvmStatic external fun setSkyboxTexture(sceneHandle: Long, textureHandle: Long, intensity: Float)
    @JvmStatic external fun createCubemapFromBitmaps(engineHandle: Long, bitmaps: Array<android.graphics.Bitmap>): Long
    @JvmStatic external fun createTextureFromHdrBytes(engineHandle: Long, bytes: ByteArray, size: Int): Long
    @JvmStatic external fun compileMaterialFromGLSL(engineHandle: Long, glslCode: String): Long

    @JvmStatic external fun setEntityLightChannel(sceneHandle: Long, entityId: Long, channel: Int, enable: Boolean)
    @JvmStatic external fun setMorphTargetWeights(sceneHandle: Long, entityId: Long, weights: FloatArray)
    @JvmStatic external fun setGuardBandOptions(engineHandle: Long, enabled: Boolean)

    @JvmStatic external fun createSkinningBuffer(engineHandle: Long, boneCount: Int): Long
    @JvmStatic external fun destroySkinningBuffer(engineHandle: Long, skinningBufferHandle: Long)
    @JvmStatic external fun setEntitySkinningBuffer(sceneHandle: Long, entityId: Long, skinningBufferHandle: Long, count: Int, offset: Int)

    @JvmStatic external fun setEntityMaterialParamAt(sceneHandle: Long, entityId: Long, primitiveIndex: Int, paramName: String, value: Float)
    @JvmStatic external fun setViewVisibleLayers(engineHandle: Long, select: Int, values: Int)
    @JvmStatic external fun setEntityBlendOrderAt(sceneHandle: Long, entityId: Long, primitiveIndex: Int, order: Int)
    @JvmStatic external fun setEntityBoundingBox(sceneHandle: Long, entityId: Long, cx: Float, cy: Float, cz: Float, ex: Float, ey: Float, ez: Float)

    @JvmStatic external fun setIndirectLightRotation3D(sceneHandle: Long, rx: Float, ry: Float, rz: Float)

    @JvmStatic external fun setEntityFogEnabled(sceneHandle: Long, entityId: Long, enabled: Boolean)
    @JvmStatic external fun setEntityPolygonOffset(sceneHandle: Long, entityId: Long, factor: Float, units: Float)
    @JvmStatic external fun setPostProcessingEnabled(engineHandle: Long, enabled: Boolean)
    @JvmStatic external fun setSkyboxShowSun(sceneHandle: Long, showSun: Boolean)
    @JvmStatic external fun setViewFrontFaceWindingInverted(engineHandle: Long, inverted: Boolean)

    @JvmStatic external fun pickEntityAt(engineHandle: Long, x: Int, y: Int): Long

    @JvmStatic external fun createRenderTarget(engineHandle: Long, width: Int, height: Int): LongArray
    @JvmStatic external fun destroyRenderTarget(engineHandle: Long, rtHandle: Long, colorTexHandle: Long, depthTexHandle: Long)
    @JvmStatic external fun createOffscreenView(engineHandle: Long, sceneHandle: Long, cameraId: Long, rtHandle: Long, width: Int, height: Int): Long
    @JvmStatic external fun destroyOffscreenView(engineHandle: Long, viewHandle: Long)
}

