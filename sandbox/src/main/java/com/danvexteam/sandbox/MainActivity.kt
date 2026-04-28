package com.danvexteam.sandbox

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.Choreographer
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import com.danvexteam.raptorv.RaptorNative

class MainActivity : AppCompatActivity() {
    private var playerEntity: Int = -1

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        Handler(Looper.getMainLooper()).postDelayed({
            /*val floor = RaptorNative.createEntity("Floor")
            RaptorNative.setPosition(floor, 0f, -5f, -10f)
            RaptorNative.setScale(floor, 10f, 1f, 10f)
            RaptorNative.addBoxCollider(floor, 10f, 1f, 10f, 0f)*/

            playerEntity = RaptorNative.createEntity("Box")
            RaptorNative.setPosition(playerEntity, 0f, 0f, -10f)
            //RaptorNative.setScale(playerEntity, 0.1f, 0.1f, 0.1f)
            //RaptorNative.addBoxCollider(playerEntity, 1f, 1f, 1f, 1f)
            RaptorNative.loadGlTF(playerEntity, "atlantic_explorer_submarineglb.glb")

            startGameLoop()
        }, 500)
    }

    private fun startGameLoop() {
        Choreographer.getInstance().postFrameCallback(object : Choreographer.FrameCallback {
            override fun doFrame(frameTimeNanos: Long) {
                if (playerEntity != -1) {
                    val currentRot = RaptorNative.getRotation(playerEntity)
                    RaptorNative.setRotation(playerEntity, currentRot[0], currentRot[1] + 0.02f, currentRot[2])
                }
                Choreographer.getInstance().postFrameCallback(this)
            }
        })
    }
}
