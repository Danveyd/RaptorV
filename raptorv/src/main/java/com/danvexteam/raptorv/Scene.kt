package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative

class Scene internal constructor(private val engine: Engine, internal val handle: Long) {
    fun createEntity(name: String = "Entity"): Entity {
        return Entity(this, RaptorNative.createEntity(handle, name))
    }

    fun destroy() {
        RaptorNative.destroyScene(handle)
    }
}
