package com.danvexteam.raptorv

import android.content.Context
import android.graphics.Bitmap
import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.FogOptions
import com.danvexteam.raptorv.types.Transform
import com.danvexteam.raptorv.types.Vec3
import com.danvexteam.raptorv.types.Vec4
import java.io.File
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer

class Scene internal constructor(
    private val engine: Engine,
    internal val handle: Long
) {
    private val entities = mutableListOf<Entity>()

    private val transformBuffer: ByteBuffer = ByteBuffer.allocateDirect(1000 * 10 * 4).order(ByteOrder.nativeOrder())
    private val floatBuffer: FloatBuffer = transformBuffer.asFloatBuffer()

    var backgroundColor: Vec4 = Vec4(0f, 0f, 0.02f, 1f)
        set(value) {
            field = value
            RaptorNative.setSkyboxColor(handle, value.x, value.y, value.z, value.w)
        }

    var iblIntensity: Float = 30000.0f
        set(value) {
            field = value
            RaptorNative.setIndirectLightIntensity(handle, value)
        }

    var iblRotation: Float = 0.0f
        set(value) {
            field = value
            RaptorNative.setIndirectLightRotation(handle, value)
        }

    var iblRotation3D: Vec3 = Vec3(0f, 0f, 0f)
        set(value) {
            field = value
            RaptorNative.setIndirectLightRotation3D(handle, value.x, value.y, value.z)
        }

    var showSunOnSkybox: Boolean = false
        set(value) {
            field = value
            RaptorNative.setSkyboxShowSun(handle, value)
        }

    var fog: FogOptions = FogOptions()
        set(value) {
            field = value
            engine.setFog(value)
        }

    init {
        RaptorNative.setTransformBuffer(handle, transformBuffer)
    }

    fun createEntity(name: String = "Entity"): Entity {
        val entity = Entity(this, RaptorNative.createEntity(handle, name))
        entities.add(entity)
        return entity
    }

    fun setEnvironment(context: Context, iblPath: String, skyboxPath: String) {
        val iblBytes = readBytes(context, iblPath)
        val skyboxBytes = readBytes(context, skyboxPath)

        RaptorNative.setEnvironment(
            handle,
            iblBytes, iblBytes?.size ?: 0,
            skyboxBytes, skyboxBytes?.size ?: 0
        )
    }

    private fun readBytes(context: Context, path: String): ByteArray? {
        return try {
            if (path.startsWith("/")) {
                File(path).readBytes()
            } else {
                context.assets.open(path).use { it.readBytes() }
            }
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }

    fun setIblIntensity(intensity: Float) {
        this.iblIntensity = intensity
    }

    fun setIblRotation(rotationY: Float) {
        this.iblRotation = rotationY
    }

    fun setSkyboxCubemap(bitmaps: Array<Bitmap>, intensity: Float = 15000.0f) {
        val cubemapTexture = Texture.cubemapFromBitmaps(engine, bitmaps)
        RaptorNative.setSkyboxTexture(handle, cubemapTexture.handle, intensity)
    }

    fun setSkyboxHdr(hdrFileBytes: ByteArray, intensity: Float = 15000.0f) {
        val cubemapHandle = RaptorNative.createCubemapFromHdrBytes(engine.handle, hdrFileBytes, hdrFileBytes.size)
        if (cubemapHandle != 0L) {
            RaptorNative.setSkyboxTexture(handle, cubemapHandle, intensity)
        }
    }

    fun setSkyboxPanorama(bitmap: Bitmap, intensity: Float = 15000.0f) {
        val cubemapHandle = RaptorNative.createCubemapFromEquirectangularBitmap(engine.handle, bitmap)
        if (cubemapHandle != 0L) {
            RaptorNative.setSkyboxTexture(handle, cubemapHandle, intensity)
        }
    }

    fun createInstanceGroup(
        name: String = "InstanceGroup",
        meshHandle: Long,
        transforms: List<Transform>
    ): InstanceGroup? {
        if (meshHandle == 0L) {
            android.util.Log.e("RaptorV", "Cannot create instance group: meshHandle is 0")
            return null
        }

        val count = transforms.size
        val array = FloatArray(count * 9)
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

        val handles = RaptorNative.createInstancedEntity(handle, name, meshHandle, array, count)
            ?: return null

        val entity = Entity(this, handles[0])
        entities.add(entity)

        return InstanceGroup(entity, count, handles[1])
    }

    internal fun update(deltaTime: Float) {
        for (i in entities.indices) {
            entities[i].updateScripts(deltaTime)
        }

        floatBuffer.position(0)
        if (floatBuffer.remaining() > 0) {
            val entityCount = floatBuffer.get().toInt()
            for (i in 0 until entityCount) {
                if (floatBuffer.remaining() < 10) break
                val id = floatBuffer.get().toLong()
                val px = floatBuffer.get()
                val py = floatBuffer.get()
                val pz = floatBuffer.get()
                val rx = floatBuffer.get()
                val ry = floatBuffer.get()
                val rz = floatBuffer.get()
                val sx = floatBuffer.get()
                val sy = floatBuffer.get()
                val sz = floatBuffer.get()

                val entity = entities.find { it.id == id }
                entity?.let {
                    it.cachedTransform.position.set(px, py, pz)
                    it.cachedTransform.rotation.set(rx, ry, rz)
                    it.cachedTransform.scale.set(sx, sy, sz)
                }
            }
        }
    }

    fun destroy() {
        RaptorNative.setTransformBuffer(handle, null)
        for (i in entities.indices) {
            entities[i].destroy()
        }
        entities.clear()
        RaptorNative.destroyScene(handle)
    }
}
