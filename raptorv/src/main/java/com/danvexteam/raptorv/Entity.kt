package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.RigidBodyDesc
import com.danvexteam.raptorv.types.Transform
import com.danvexteam.raptorv.types.Vec3

class Entity internal constructor(private val scene: Scene, val id: Long) {

    var transform: Transform
        get() = Transform(rotation = getRotation())
        set(t) {
            RaptorNative.setTransform(scene.handle, id,
                t.position.x, t.position.y, t.position.z,
                t.rotation.x, t.rotation.y, t.rotation.z,
                t.scale.x, t.scale.y, t.scale.z)
        }

    fun setTransformRaw(px: Float, py: Float, pz: Float,
                        rx: Float, ry: Float, rz: Float,
                        sx: Float = 1f, sy: Float = 1f, sz: Float = 1f) {
        RaptorNative.setTransform(scene.handle, id, px, py, pz, rx, ry, rz, sx, sy, sz)
    }

    fun getRotation(): Vec3 {
        val rot = RaptorNative.getTransformRotation(scene.handle, id)
        return Vec3(rot[0], rot[1], rot[2])
    }

    fun attachMesh(meshHandle: Long) {
        RaptorNative.attachMesh(scene.handle, id, meshHandle)
    }

    fun attachRigidBody(desc: RigidBodyDesc = RigidBodyDesc()) {
        RaptorNative.attachRigidBody(scene.handle, id, desc.halfExtent.x, desc.halfExtent.y, desc.halfExtent.z, desc.mass)
    }
}
