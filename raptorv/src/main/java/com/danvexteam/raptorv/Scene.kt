package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative

class Scene internal constructor(
    private val engine: Engine,
    internal val handle: Long
) {
    private val entities = mutableListOf<Entity>()

    fun createEntity(name: String = "Entity"): Entity {
        val entity = Entity(this, RaptorNative.createEntity(handle, name))
        entities.add(entity)
        return entity
    }

    internal fun update(deltaTime: Float) {
        for (i in entities.indices) {
            entities[i].updateScripts(deltaTime)
        }
    }

    fun destroy() {
        for (i in entities.indices) {
            entities[i].destroy()
        }
        entities.clear()
        RaptorNative.destroyScene(handle)
    }
}
