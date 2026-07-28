package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.LightDesc
import com.danvexteam.raptorv.types.Vec3

class Light internal constructor(private val engine: Engine, val id: Long) {

    val desc = LightDesc()

    var color: Vec3
        get() = desc.color
        set(value) { desc.color = value; apply() }

    var intensity: Float
        get() = desc.intensity
        set(value) { desc.intensity = value; apply() }

    var position: Vec3
        get() = desc.position
        set(value) { desc.position = value; apply() }

    var direction: Vec3
        get() = desc.direction
        set(value) { desc.direction = value; apply() }

    var castShadows: Boolean
        get() = desc.castShadows
        set(value) { desc.castShadows = value; apply() }

    var shadowMapSize: Int
        get() = desc.shadowMapSize
        set(value) { desc.shadowMapSize = value; apply() }

    fun setPositionAndDirection(position: Vec3, direction: Vec3) {
        desc.position = position
        desc.direction = direction
        RaptorNative.setLightPositionAndDirection(
            engine.handle, id,
            position.x, position.y, position.z,
            direction.x, direction.y, direction.z
        )
    }

    fun setChannel(channelIndex: Int, enable: Boolean) {
        if (channelIndex in 0..7) {
            RaptorNative.setLightChannel(engine.handle, id, channelIndex, enable)
        }
    }

    fun update(desc: LightDesc) {
        this.desc.type = desc.type
        this.desc.color = desc.color
        this.desc.intensity = desc.intensity
        this.desc.direction = desc.direction
        this.desc.position = desc.position
        this.desc.falloffRadius = desc.falloffRadius
        this.desc.castShadows = desc.castShadows
        this.desc.enableContactShadows = desc.enableContactShadows
        this.desc.innerConeAngle = desc.innerConeAngle
        this.desc.outerConeAngle = desc.outerConeAngle
        this.desc.shadowMapSize = desc.shadowMapSize
        this.desc.shadowCascades = desc.shadowCascades
        this.desc.shadowConstantBias = desc.shadowConstantBias
        this.desc.shadowNormalBias = desc.shadowNormalBias
        apply()
    }

    fun apply() {
        RaptorNative.updateLight(
            engine.handle, id, desc.type.ordinal,
            desc.color.x, desc.color.y, desc.color.z, desc.intensity,
            desc.direction.x, desc.direction.y, desc.direction.z,
            desc.position.x, desc.position.y, desc.position.z,
            desc.falloffRadius, desc.castShadows, desc.enableContactShadows,
            desc.innerConeAngle, desc.outerConeAngle,
            desc.shadowMapSize, desc.shadowCascades, desc.shadowConstantBias, desc.shadowNormalBias,
            desc.sunAngularRadius, desc.sunHaloSize, desc.sunHaloFalloff,
            desc.cascadeSplit1, desc.cascadeSplit2, desc.cascadeSplit3
        )
    }
}
