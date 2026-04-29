package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.LightDesc

class Light internal constructor(private val engine: Engine, val id: Long) {
    fun update(desc: LightDesc) {
        RaptorNative.updateLight(
            engine.handle, id,
            desc.type.ordinal,
            desc.color.x, desc.color.y, desc.color.z, desc.intensity,
            desc.direction.x, desc.direction.y, desc.direction.z,
            desc.position.x, desc.position.y, desc.position.z,
            desc.falloffRadius, desc.castShadows
        )
    }
}
