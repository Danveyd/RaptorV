package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative

class OffscreenView internal constructor(
    private val engine: Engine,
    internal val handle: Long
) : AutoCloseable {

    companion object {
        fun create(
            engine: Engine,
            scene: Scene,
            camera: Camera,
            renderTarget: RenderTarget,
            width: Int,
            height: Int
        ): OffscreenView {
            val handle = RaptorNative.createOffscreenView(
                engine.handle, scene.handle, camera.id, renderTarget.handle, width, height
            )
            return OffscreenView(engine, handle)
        }
    }

    override fun close() {
        RaptorNative.destroyOffscreenView(engine.handle, handle)
    }
}
