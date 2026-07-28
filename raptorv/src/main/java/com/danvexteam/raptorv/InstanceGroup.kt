package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.Transform

class InstanceGroup internal constructor(
    val entity: Entity,
    val instanceCount: Int,
    internal val instanceBufferHandle: Long
) {
    var transform: Transform
        get() = entity.transform
        set(value) { entity.transform = value }

    var isVisible: Boolean
        get() = entity.isVisible
        set(value) { entity.isVisible = value }

    var castShadows: Boolean
        get() = entity.castShadows
        set(value) { entity.castShadows = value }

    var receiveShadows: Boolean
        get() = entity.receiveShadows
        set(value) { entity.receiveShadows = value }

    var priority: Int
        get() = entity.priority
        set(value) { entity.priority = value }

    val material: Material
        get() = entity.material

    fun setTransforms(transforms: List<Transform>) {
        val array = FloatArray(transforms.size * 9)
        for (i in transforms.indices) {
            val t = transforms[i]
            array[i * 9 + 0] = t.position.x
            array[i * 9 + 1] = t.position.y
            array[i * 9 + 2] = t.position.z
            array[i * 9 + 3] = t.rotation.x
            array[i * 9 + 4] = t.rotation.y
            array[i * 9 + 5] = t.rotation.z
            array[i * 9 + 6] = t.scale.x
            array[i * 9 + 7] = t.scale.y
            array[i * 9 + 8] = t.scale.z
        }
        RaptorNative.updateInstanceTransforms(instanceBufferHandle, array, transforms.size)
    }
}
