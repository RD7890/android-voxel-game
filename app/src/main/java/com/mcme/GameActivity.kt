package com.mcme

import android.os.Bundle
import android.view.View
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import com.mcme.databinding.ActivityGameBinding

class GameActivity : AppCompatActivity() {

    private lateinit var binding: ActivityGameBinding
    private lateinit var gameView: GameSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        makeFullscreen()

        binding = ActivityGameBinding.inflate(layoutInflater)
        setContentView(binding.root)

        gameView = binding.gameSurface

        binding.btnBack.setOnClickListener { finish() }
        binding.btnJump.setOnClickListener { NativeLib.onJump() }

        // Wire joystick to native
        binding.joystick.onMove = { rx, ry ->
            NativeLib.onJoystick(rx, -ry)
        }

        // Look controls on right side
        binding.lookZone.setOnTouchListener(gameView.lookTouchListener)
    }

    private fun makeFullscreen() {
        window.decorView.systemUiVisibility = (
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            or View.SYSTEM_UI_FLAG_FULLSCREEN
            or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
            or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
        )
    }

    override fun onResume() {
        super.onResume()
        makeFullscreen()
        gameView.onResume()
        NativeLib.onResume()
    }

    override fun onPause() {
        super.onPause()
        gameView.onPause()
        NativeLib.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        NativeLib.destroy()
    }
}
