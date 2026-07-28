package com.danvexteam.raptorv

import android.graphics.Bitmap
import com.danvexteam.raptorv.internal.RaptorNative

class Texture internal constructor(
    private val engine: Engine,
    internal val handle: Long
) : AutoCloseable {

    companion object {
        fun fromBitmap(engine: Engine, bitmap: Bitmap): Texture {
            val handle = RaptorNative.createTextureFromBitmap(engine.handle, bitmap)
            return Texture(engine, handle)
        }

        fun cubemapFromBitmaps(engine: Engine, bitmaps: Array<Bitmap>): Texture {
            val handle = RaptorNative.createCubemapFromBitmaps(engine.handle, bitmaps)
            return Texture(engine, handle)
        }

        fun fromHdrBytes(engine: Engine, bytes: ByteArray): Texture {
            val handle = RaptorNative.createTextureFromHdrBytes(engine.handle, bytes, bytes.size)
            return Texture(engine, handle)
        }
    }

    override fun close() {
        RaptorNative.destroyTexture(engine.handle, handle)
    }
}
