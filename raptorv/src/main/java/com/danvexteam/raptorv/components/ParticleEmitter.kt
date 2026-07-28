package com.danvexteam.raptorv.components

import com.danvexteam.raptorv.Component
import com.danvexteam.raptorv.InstanceGroup
import com.danvexteam.raptorv.types.Transform
import com.danvexteam.raptorv.types.Vec3
import kotlin.random.Random

private class ParticleData(
    val transform: Transform = Transform(),
    val velocity: Vec3 = Vec3(),
    var life: Float = 0f,
    var maxLife: Float = 1.0f
)

class ParticleEmitter(
    private val meshHandle: Long,
    private val maxParticles: Int = 100,
    var emitRate: Float = 30f,
    var particleSpeed: Float = 2.0f,
    var particleLifetime: Float = 1.5f,
    var gravity: Vec3 = Vec3(0f, -0.5f, 0f),
    var spread: Vec3 = Vec3(0.5f, 0.5f, 0.5f)
) : Component() {

    private var instanceGroup: InstanceGroup? = null
    private val particles = mutableListOf<ParticleData>()
    private val transformsList = mutableListOf<Transform>()
    private var emitTimer = 0f

    override fun onCreate() {
        if (meshHandle == 0L) return

        for (i in 0 until maxParticles) {
            val p = ParticleData()
            p.transform.scale = Vec3(0f, 0f, 0f)
            particles.add(p)
            transformsList.add(p.transform)
        }

        val scene = entity.material.let { null }
    }

    override fun onUpdate(deltaTime: Float) {
        emitTimer += deltaTime
        val spawnInterval = 1.0f / emitRate

        while (emitTimer >= spawnInterval) {
            emitTimer -= spawnInterval
            spawnParticle()
        }

        val basePos = entity.position

        for (i in particles.indices) {
            val p = particles[i]
            if (p.life > 0f) {
                p.life -= deltaTime

                if (p.life <= 0f) {
                    p.transform.scale = Vec3(0f, 0f, 0f)
                } else {
                    p.velocity.x += gravity.x * deltaTime
                    p.velocity.y += gravity.y * deltaTime
                    p.velocity.z += gravity.z * deltaTime

                    p.transform.position.x += p.velocity.x * deltaTime
                    p.transform.position.y += p.velocity.y * deltaTime
                    p.transform.position.z += p.velocity.z * deltaTime

                    val alpha = p.life / p.maxLife
                    p.transform.scale = Vec3(0.1f * alpha, 0.1f * alpha, 0.1f * alpha)
                }
            }
        }

        instanceGroup?.setTransforms(transformsList)
    }

    private fun spawnParticle() {
        val deadParticle = particles.find { it.life <= 0f } ?: return
        val basePos = entity.position

        deadParticle.life = particleLifetime
        deadParticle.maxLife = particleLifetime

        deadParticle.transform.position = Vec3(
            basePos.x + (Random.nextFloat() - 0.5f) * spread.x,
            basePos.y + (Random.nextFloat() - 0.5f) * spread.y,
            basePos.z + (Random.nextFloat() - 0.5f) * spread.z
        )

        deadParticle.velocity.set(
            (Random.nextFloat() - 0.5f) * particleSpeed,
            Random.nextFloat() * particleSpeed + 1.0f,
            (Random.nextFloat() - 0.5f) * particleSpeed
        )

        deadParticle.transform.scale = Vec3(0.1f, 0.1f, 0.1f)
    }
}
