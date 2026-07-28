package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative

class RenderTarget internal constructor(
    private val engine: Engine,
    internal val handle: Long,
    val texture: Texture,
    private val depthTextureHandle: Long
) : AutoCloseable {

    companion object {
        fun create(engine: Engine, width: Int, height: Int): RenderTarget {
            val handles = RaptorNative.createRenderTarget(engine.handle, width, height)
            val rtHandle = handles[0]
            val colorTexHandle = handles[1]
            val depthTexHandle = handles[2]

            val texture = Texture(engine, colorTexHandle)
            return RenderTarget(engine, rtHandle, texture, depthTexHandle)
        }
    }

    override fun close() {
        RaptorNative.destroyRenderTarget(engine.handle, handle, texture.handle, depthTextureHandle)
    }
}
