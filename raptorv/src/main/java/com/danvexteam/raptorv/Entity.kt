package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.ColliderType
import com.danvexteam.raptorv.types.RigidBodyDesc
import com.danvexteam.raptorv.types.Transform
import com.danvexteam.raptorv.types.Vec3

class Entity internal constructor(private val scene: Scene, val id: Long) {

    private val scripts = mutableListOf<Component>()

    internal val cachedTransform = Transform()

    var transform: Transform
        get() = cachedTransform
        set(t) {
            cachedTransform.position.set(t.position.x, t.position.y, t.position.z)
            cachedTransform.rotation.set(t.rotation.x, t.rotation.y, t.rotation.z)
            cachedTransform.scale.set(t.scale.x, t.scale.y, t.scale.z)
            RaptorNative.setTransform(scene.handle, id,
                t.position.x, t.position.y, t.position.z,
                t.rotation.x, t.rotation.y, t.rotation.z,
                t.scale.x, t.scale.y, t.scale.z)
        }

    var parent: Entity? = null
        set(value) {
            field = value
            if (value != null) {
                RaptorNative.setEntityParent(scene.handle, id, value.id)
            }
        }

    val material: Material by lazy {
        Material(scene.handle, id)
    }
    var castShadows: Boolean = true
        set(value) {
            field = value
            RaptorNative.setEntityCastShadows(scene.handle, id, value)
        }

    var receiveShadows: Boolean = true
        set(value) {
            field = value
            RaptorNative.setEntityReceiveShadows(scene.handle, id, value)
        }

    var priority: Int = 4
        set(value) {
            field = value
            RaptorNative.setEntityPriority(scene.handle, id, value)
        }

    var culling: Boolean = true
        set(value) {
            field = value
            RaptorNative.setEntityCulling(scene.handle, id, value)
        }
    var isVisible: Boolean = true
        set(value) {
            field = value
            setLayerMask(1, if (value) 1 else 0)
        }

    var position: Vec3
        get() = cachedTransform.position
        set(value) {
            cachedTransform.position.set(value.x, value.y, value.z)
            applyTransform()
        }

    var rotation: Vec3
        get() = cachedTransform.rotation
        set(value) {
            cachedTransform.rotation.set(value.x, value.y, value.z)
            applyTransform()
        }

    var scale: Vec3
        get() = cachedTransform.scale
        set(value) {
            cachedTransform.scale.set(value.x, value.y, value.z)
            applyTransform()
        }

    var enableFog: Boolean = true
        set(value) {
            field = value
            RaptorNative.setEntityFogEnabled(scene.handle, id, value)
        }

    fun setAsDecal(factor: Float = -1.0f, units: Float = -1.0f) {
        RaptorNative.setEntityPolygonOffset(scene.handle, id, factor, units)
    }

    fun setPosition(x: Float, y: Float, z: Float) {
        cachedTransform.position.set(x, y, z)
        applyTransform()
    }

    fun setRotation(x: Float, y: Float, z: Float) {
        cachedTransform.rotation.set(x, y, z)
        applyTransform()
    }

    fun setScale(x: Float, y: Float, z: Float) {
        cachedTransform.scale.set(x, y, z)
        applyTransform()
    }

    fun move(dx: Float, dy: Float, dz: Float) {
        cachedTransform.position.x += dx
        cachedTransform.position.y += dy
        cachedTransform.position.z += dz
        applyTransform()
    }

    private fun applyTransform() {
        RaptorNative.setTransform(
            scene.handle, id,
            cachedTransform.position.x, cachedTransform.position.y, cachedTransform.position.z,
            cachedTransform.rotation.x, cachedTransform.rotation.y, cachedTransform.rotation.z,
            cachedTransform.scale.x, cachedTransform.scale.y, cachedTransform.scale.z
        )
    }

    fun setLayerMask(select: Int, value: Int) {
        RaptorNative.setEntityLayerMask(scene.handle, id, select, value)
    }

    fun <T : Component> addScript(script: T): T {
        script.entity = this
        scripts.add(script)
        script.onCreate()
        return script
    }

    internal fun updateScripts(deltaTime: Float) {
        for (i in scripts.indices) {
            scripts[i].onUpdate(deltaTime)
        }
    }

    internal fun destroy() {
        for (i in scripts.indices) {
            scripts[i].onDestroy()
        }
        scripts.clear()
    }

    fun setTransformRaw(px: Float, py: Float, pz: Float,
                        rx: Float, ry: Float, rz: Float,
                        sx: Float = 1f, sy: Float = 1f, sz: Float = 1f) {
        RaptorNative.setTransform(scene.handle, id, px, py, pz, rx, ry, rz, sx, sy, sz)
    }

    fun setSkinningBuffer(buffer: SkinningBuffer, offset: Int = 0) {
        if (buffer.handle != 0L) {
            RaptorNative.setEntitySkinningBuffer(scene.handle, id, buffer.handle, buffer.boneCount, offset)
        }
    }

    fun setLightChannel(channelIndex: Int, enable: Boolean) {
        if (channelIndex in 0..7) {
            RaptorNative.setEntityLightChannel(scene.handle, id, channelIndex, enable)
        }
    }

    fun setMorphTargetWeights(weights: FloatArray) {
        RaptorNative.setMorphTargetWeights(scene.handle, id, weights)
    }

    fun getRotation(): Vec3 {
        val rot = RaptorNative.getTransformRotation(scene.handle, id)
        return Vec3(rot[0], rot[1], rot[2])
    }

    fun setMaterialParamAt(primitiveIndex: Int, paramName: String, value: Float) {
        RaptorNative.setEntityMaterialParamAt(scene.handle, id, primitiveIndex, paramName, value)
    }

    fun setBlendOrderAt(primitiveIndex: Int, order: Int) {
        RaptorNative.setEntityBlendOrderAt(scene.handle, id, primitiveIndex, order)
    }

    fun setBoundingBox(center: Vec3, halfExtent: Vec3) {
        RaptorNative.setEntityBoundingBox(
            scene.handle, id,
            center.x, center.y, center.z,
            halfExtent.x, halfExtent.y, halfExtent.z
        )
    }

    fun attachMesh(meshHandle: Long) {
        RaptorNative.attachMesh(scene.handle, id, meshHandle)
    }

    fun attachRigidBody(desc: RigidBodyDesc = RigidBodyDesc()) {
        val typeInt = desc.type.ordinal
        var arg1 = 0f
        var arg2 = 0f
        var arg3 = 0f

        when (desc.type) {
            ColliderType.BOX -> {
                arg1 = desc.halfExtent.x
                arg2 = desc.halfExtent.y
                arg3 = desc.halfExtent.z
            }
            ColliderType.SPHERE -> {
                arg1 = desc.radius
            }
            ColliderType.CAPSULE -> {
                arg1 = desc.halfHeight
                arg2 = desc.radius
            }
        }

        RaptorNative.attachRigidBody(scene.handle, id, typeInt, arg1, arg2, arg3, desc.mass)
    }

    fun applyAnimation(
        animationIndex: Int, timeSeconds: Float,
        enableCrossFade: Boolean = false, prevAnimationIndex: Int = 0, prevTimeSeconds: Float = 0f, alpha: Float = 0f
    ) {
        RaptorNative.applyAnimation(
            scene.handle, id,
            animationIndex, timeSeconds,
            enableCrossFade, prevAnimationIndex, prevTimeSeconds, alpha
        )
    }
}
