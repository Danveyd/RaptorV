package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.Vec3

class Material internal constructor(
    private val sceneHandle: Long,
    private val entityId: Long
) {
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

    var baseColor: Vec3 = Vec3(1f, 1f, 1f)
        set(value) {
            field = value
            RaptorNative.setMaterialParamColor(sceneHandle, entityId, "baseColorFactor", value.x, value.y, value.z)
        }

    fun setFloat(paramName: String, value: Float) {
        RaptorNative.setMaterialParamFloat(sceneHandle, entityId, paramName, value)
    }

    fun setColor(paramName: String, color: Vec3) {
        RaptorNative.setMaterialParamColor(sceneHandle, entityId, paramName, color.x, color.y, color.z)
    }
}
