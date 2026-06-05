package com.mcme

import android.os.Bundle
import android.view.View
import android.view.WindowManager
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import com.mcme.databinding.ActivitySettingsBinding

class SettingsActivity : AppCompatActivity() {

    private lateinit var binding: ActivitySettingsBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        makeFullscreen()
        binding = ActivitySettingsBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val prefs = getSharedPreferences("mcme_prefs", MODE_PRIVATE)

        // --- Sensitivity ---
        val savedSens = prefs.getFloat("sensitivity", 0.15f)
        binding.seekSensitivity.progress = (savedSens * 200).toInt().coerceIn(1, 100)
        binding.tvSensValue.text = String.format("%.2f", savedSens)
        binding.seekSensitivity.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(sb: SeekBar?, p: Int, fromUser: Boolean) {
                val v = p / 200f
                binding.tvSensValue.text = String.format("%.2f", v)
                prefs.edit().putFloat("sensitivity", v).apply()
                NativeLib.setSensitivity(v)
            }
            override fun onStartTrackingTouch(sb: SeekBar?) {}
            override fun onStopTrackingTouch(sb: SeekBar?) {}
        })

        // --- Render Distance ---
        val savedRD = prefs.getInt("render_distance", 4)
        binding.seekRenderDist.progress = savedRD - 2
        binding.tvRenderValue.text = "${savedRD} chunks"
        binding.seekRenderDist.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(sb: SeekBar?, p: Int, fromUser: Boolean) {
                val d = p + 2
                binding.tvRenderValue.text = "$d chunks"
                prefs.edit().putInt("render_distance", d).apply()
                NativeLib.setRenderDistance(d)
            }
            override fun onStartTrackingTouch(sb: SeekBar?) {}
            override fun onStopTrackingTouch(sb: SeekBar?) {}
        })

        binding.btnBack.setOnClickListener { finish() }
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

    override fun onResume() { super.onResume(); makeFullscreen() }
}
