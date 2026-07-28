package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.MagFilter
import com.danvexteam.raptorv.types.MinFilter
import com.danvexteam.raptorv.types.Vec2
import com.danvexteam.raptorv.types.Vec3
import com.danvexteam.raptorv.types.Vec4
import com.danvexteam.raptorv.types.WrapMode

enum class CullMode { NONE, FRONT, BACK, FRONT_AND_BACK }

class Material internal constructor(
    private val sceneHandle: Long,
    private val entityId: Long
) {
    var baseColor: Vec4 = Vec4(1f, 1f, 1f, 1f)
        set(value) {
            field = value
            RaptorNative.setMaterialParamColor(sceneHandle, entityId, "baseColorFactor", value.x, value.y, value.z, value.w)
        }

    var roughness: Float = 0.4f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "roughnessFactor", value)
        }

    var metallic: Float = 0.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "metallicFactor", value)
        }

    var reflectance: Float = 0.5f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "reflectance", value)
        }

    var clearCoat: Float = 0.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "clearCoatFactor", value)
        }

    var clearCoatRoughness: Float = 0.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "clearCoatRoughnessFactor", value)
        }

    var anisotropy: Float = 0.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "anisotropy", value)
        }

    var anisotropyDirection: Vec3 = Vec3(1f, 0f, 0f)
        set(value) {
            field = value
            RaptorNative.setMaterialParamVec2(sceneHandle, entityId, "anisotropyDirection", value.x, value.y)
        }

    var sheenColor: Vec3 = Vec3(1f, 1f, 1f)
        set(value) {
            field = value
            RaptorNative.setMaterialParamColor(sceneHandle, entityId, "sheenColorFactor", value.x, value.y, value.z, 1.0f)
        }

    var sheenRoughness: Float = 0.2f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "sheenRoughnessFactor", value)
        }

    var transmission: Float = 0.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "transmissionFactor", value)
        }

    var ior: Float = 1.5f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "ior", value)
        }

    var emissiveColor: Vec4 = Vec4(0f, 0f, 0f, 1f)
        set(value) {
            field = value
            RaptorNative.setMaterialParamColor(sceneHandle, entityId, "emissiveFactor", value.x, value.y, value.z, value.w)
        }

    var cullMode: CullMode = CullMode.BACK
        set(value) {
            field = value
            RaptorNative.setMaterialCullMode(sceneHandle, entityId, value.ordinal)
        }

    var depthWrite: Boolean = true
        set(value) {
            field = value
            RaptorNative.setMaterialDepthWrite(sceneHandle, entityId, value)
        }

    var iridescence: Float = 0.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "iridescenceFactor", value)
        }

    var iridescenceIor: Float = 1.3f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "iridescenceIor", value)
        }

    var iridescenceThicknessMin: Float = 100.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "iridescenceThicknessMinimum", value)
        }

    var iridescenceThicknessMax: Float = 400.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "iridescenceThicknessMaximum", value)
        }

    var dispersion: Float = 0.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "dispersion", value)
        }

    var subsurfaceColor: Vec3 = Vec3(1f, 1f, 1f)
        set(value) {
            field = value
            RaptorNative.setMaterialParamColor(sceneHandle, entityId, "subsurfaceColor", value.x, value.y, value.z, 1.0f)
        }

    var subsurfacePower: Float = 1.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "subsurfacePower", value)
        }

    var thickness: Float = 0.5f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "thickness", value)
        }

    var specularColor: Vec3 = Vec3(1f, 1f, 1f)
        set(value) {
            field = value
            RaptorNative.setMaterialParamColor(sceneHandle, entityId, "specularFactor", value.x, value.y, value.z, 1.0f)
        }

    var glossiness: Float = 1.0f
        set(value) {
            field = value
            RaptorNative.setMaterialParamFloat(sceneHandle, entityId, "glossinessFactor", value)
        }


    fun setFloat(paramName: String, value: Float) {
        RaptorNative.setMaterialParamFloat(sceneHandle, entityId, paramName, value)
    }

    fun setVec2(paramName: String, value: Vec2) {
        RaptorNative.setMaterialParamVec2(sceneHandle, entityId, paramName, value.x, value.y)
    }

    fun setColor(paramName: String, color: Vec3) {
        RaptorNative.setMaterialParamColor(sceneHandle, entityId, paramName, color.x, color.y, color.z, 1.0f)
    }

    fun setColorAlpha(paramName: String, color: Vec4) {
        RaptorNative.setMaterialParamColor(sceneHandle, entityId, paramName, color.x, color.y, color.z, color.w)
    }

    fun setBaseColorTexture(texture: Texture) = setTexture("baseColorMap", texture)
    fun setNormalMap(texture: Texture) = setTexture("normalMap", texture)
    fun setRoughnessMap(texture: Texture) = setTexture("roughnessMap", texture)
    fun setMetallicMap(texture: Texture) = setTexture("metallicMap", texture)
    fun setEmissiveMap(texture: Texture) = setTexture("emissiveMap", texture)

    fun setTexture(
        paramName: String,
        texture: Texture,
        minFilter: MinFilter = MinFilter.LINEAR,
        magFilter: MagFilter = MagFilter.LINEAR,
        wrapS: WrapMode = WrapMode.REPEAT,
        wrapT: WrapMode = WrapMode.REPEAT
    ) {
        RaptorNative.setMaterialParamTexture(
            sceneHandle, entityId, paramName, texture.handle,
            minFilter.ordinal, magFilter.ordinal, wrapS.ordinal, wrapT.ordinal
        )
    }

    fun setSpecularAntiAliasing(enabled: Boolean, variance: Float = 0.15f, threshold: Float = 0.2f) {
        RaptorNative.setMaterialSpecularAntiAliasing(sceneHandle, entityId, enabled, variance, threshold)
    }
}

inline fun Entity.material(block: Material.() -> Unit): Material {
    val mat = this.material
    mat.block()
    return mat
}

inline fun Material.configure(block: Material.() -> Unit): Material {
    this.block()
    return this
}
