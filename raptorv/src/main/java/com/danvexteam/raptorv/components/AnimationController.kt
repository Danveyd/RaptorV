package com.danvexteam.raptorv.components

import com.danvexteam.raptorv.Component
import com.danvexteam.raptorv.Engine

class AnimationController(
    private val engine: Engine,
    private val meshHandle: Long,
    defaultAnimationIndex: Int = 0,
    var speed: Float = 1.0f,
    var loop: Boolean = true
) : Component() {

    var currentAnimationIndex: Int = defaultAnimationIndex
        private set

    private var currentTime: Float = 0f
    var duration: Float = 0f
        private set

    private var isFading: Boolean = false
    private var prevAnimationIndex: Int = -1
    private var prevTime: Float = 0f
    private var fadeDuration: Float = 0.3f
    private var fadeTimer: Float = 0f

    override fun onCreate() {
        duration = engine.getAnimationDuration(meshHandle, currentAnimationIndex)
    }

    override fun onUpdate(deltaTime: Float) {
        if (duration <= 0f) return

        currentTime += deltaTime * speed

        if (loop) {
            currentTime %= duration
        } else if (currentTime > duration) {
            currentTime = duration
        }

        if (isFading) {
            fadeTimer += deltaTime
            val alpha = (fadeTimer / fadeDuration).coerceIn(0f, 1f)

            entity.applyAnimation(
                animationIndex = currentAnimationIndex,
                timeSeconds = currentTime,
                enableCrossFade = true,
                prevAnimationIndex = prevAnimationIndex,
                prevTimeSeconds = prevTime,
                alpha = 1.0f - alpha
            )

            if (fadeTimer >= fadeDuration) {
                isFading = false
            }
        } else {
            entity.applyAnimation(currentAnimationIndex, currentTime)
        }
    }

    fun play(newAnimationIndex: Int, fadeTimeSeconds: Float = 0.3f, speed: Float = 1.0f, loop: Boolean = true) {
        if (newAnimationIndex == currentAnimationIndex && !isFading) return

        this.prevAnimationIndex = this.currentAnimationIndex
        this.prevTime = this.currentTime
        this.currentAnimationIndex = newAnimationIndex

        this.speed = speed
        this.loop = loop
        this.currentTime = 0f
        this.duration = engine.getAnimationDuration(meshHandle, newAnimationIndex)

        if (fadeTimeSeconds > 0f && prevAnimationIndex >= 0) {
            this.isFading = true
            this.fadeDuration = fadeTimeSeconds
            this.fadeTimer = 0f
        } else {
            this.isFading = false
        }
    }
}
