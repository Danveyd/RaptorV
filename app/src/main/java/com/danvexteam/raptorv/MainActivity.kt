package com.danvexteam.raptorv

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.danvexteam.raptorv.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        RaptorNative.initEngine()
    }

    companion object {
        init {
            System.loadLibrary("raptorv")
        }
    }
}