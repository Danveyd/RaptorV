package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative

class SkinningBuffer internal constructor(
    private val engine: Engine,
    internal val handle: Long,
    val boneCount: Int
) : AutoCloseable {

    companion object {
        fun create(engine: Engine, boneCount: Int): SkinningBuffer {
            val handle = RaptorNative.createSkinningBuffer(engine.handle, boneCount)
            return SkinningBuffer(engine, handle, boneCount)
        }
    }

    override fun close() {
        if (handle != 0L) {
            RaptorNative.destroySkinningBuffer(engine.handle, handle)
        }
    }
}
