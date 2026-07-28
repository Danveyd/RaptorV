package com.danvexteam.raptorv

import com.danvexteam.raptorv.types.Vec2
import com.danvexteam.raptorv.types.Vec3

enum class PrimitiveType {
    TRIANGLES,
    POINTS,
    LINES,
    LINE_STRIP
}

class MeshBuilder {
    private val vertices = mutableListOf<Float>()
    private val indices = mutableListOf<Int>()
    private var vertexCount = 0

    fun addVertex(
        pos: Vec3,
        uv: Vec2 = Vec2(0f, 0f),
        normal: Vec3 = Vec3(0f, 1f, 0f)
    ): MeshBuilder {
        vertices.add(pos.x); vertices.add(pos.y); vertices.add(pos.z)
        vertices.add(uv.x);  vertices.add(uv.y)
        vertices.add(normal.x); vertices.add(normal.y); vertices.add(normal.z)
        vertexCount++
        return this
    }

    fun addIndex(index: Int): MeshBuilder {
        indices.add(index)
        return this
    }

    fun addTriangle(i0: Int, i1: Int, i2: Int): MeshBuilder {
        indices.add(i0); indices.add(i1); indices.add(i2)
        return this
    }

    fun build(engine: Engine): Long {
        if (vertices.isEmpty() || indices.isEmpty()) return 0L
        return engine.createMesh(vertices.toFloatArray(), indices.toIntArray())
    }

    companion object {
        fun createPlane(engine: Engine, width: Float = 10f, depth: Float = 10f): Long {
            val hw = width / 2f; val hd = depth / 2f
            val builder = MeshBuilder()
            builder.addVertex(Vec3(-hw, 0f, -hd), Vec2(0f, 0f), Vec3(0f, 1f, 0f))
            builder.addVertex(Vec3(hw, 0f, -hd),  Vec2(1f, 0f), Vec3(0f, 1f, 0f))
            builder.addVertex(Vec3(hw, 0f, hd),   Vec2(1f, 1f), Vec3(0f, 1f, 0f))
            builder.addVertex(Vec3(-hw, 0f, hd),  Vec2(0f, 1f), Vec3(0f, 1f, 0f))
            builder.addTriangle(0, 1, 2)
            builder.addTriangle(0, 2, 3)
            return builder.build(engine)
        }

        fun createPointCloud(engine: Engine, pointCount: Int = 1000, radius: Float = 5.0f): Long {
            val builder = MeshBuilder()
            for (i in 0 until pointCount) {
                val px = (Math.random().toFloat() - 0.5f) * radius * 2f
                val py = (Math.random().toFloat() - 0.5f) * radius * 2f
                val pz = (Math.random().toFloat() - 0.5f) * radius * 2f
                builder.addVertex(Vec3(px, py, pz))
                builder.addIndex(i)
            }
            return builder.build(engine)
        }
    }
}
