package com.danvexteam.raptorv

import android.content.Context
import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.CameraDesc
import com.danvexteam.raptorv.types.LightDesc
import com.danvexteam.raptorv.types.RenderSettings

class Engine(context: Context) : AutoCloseable {
    internal val handle: Long = RaptorNative.createEngine(context.assets)

    fun createScene(): Scene = Scene(this, RaptorNative.createScene(handle))

    fun setActiveScene(scene: Scene) {
        RaptorNative.setActiveScene(handle, scene.handle)
    }

    fun loadMesh(path: String): Long {
        return RaptorNative.loadMesh(handle, path)
    }

    fun createCamera(desc: CameraDesc = CameraDesc()): Camera {
        val id = RaptorNative.createCamera(
            handle,
            desc.fovDegrees, desc.nearPlane, desc.farPlane,
            desc.position.x, desc.position.y, desc.position.z,
            desc.target.x, desc.target.y, desc.target.z,
            desc.up.x, desc.up.y, desc.up.z
        )
        return Camera(this, id)
    }

    fun setRenderSettings(settings: RenderSettings) {
        RaptorNative.setRenderSettings(
            handle, settings.bloom, settings.ssao, settings.fxaa, settings.taa,
            settings.clearColor.x, settings.clearColor.y, settings.clearColor.z, settings.clearColor.w
        )
    }

    fun createLight(desc: LightDesc = LightDesc()): Light {
        val id = RaptorNative.createLight(
            handle, desc.type.ordinal,
            desc.color.x, desc.color.y, desc.color.z, desc.intensity,
            desc.direction.x, desc.direction.y, desc.direction.z,
            desc.position.x, desc.position.y, desc.position.z,
            desc.falloffRadius, desc.castShadows
        )
        return Light(this, id)
    }

    internal fun tick(dt: Float) = RaptorNative.tick(handle, dt)

    override fun close() {
        RaptorNative.destroyEngine(handle)
    }
}
