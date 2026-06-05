package com.mcme.ui

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import kotlin.math.atan2
import kotlin.math.hypot
import kotlin.math.min

class JoystickView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : View(context, attrs) {

    var onMove: ((rightAxis: Float, forwardAxis: Float) -> Unit)? = null

    private val outerPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.argb(60, 255, 255, 255)
        style = Paint.Style.FILL
    }
    private val innerPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.argb(140, 255, 255, 255)
        style = Paint.Style.FILL
    }
    private val borderPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.argb(100, 255, 255, 255)
        style = Paint.Style.STROKE
        strokeWidth = 3f
    }

    private var centerX = 0f
    private var centerY = 0f
    private var thumbX  = 0f
    private var thumbY  = 0f
    private var maxRadius = 0f
    private var thumbRadius = 0f
    private var active = false

    override fun onSizeChanged(w: Int, h: Int, oldW: Int, oldH: Int) {
        centerX = w / 2f; centerY = h / 2f
        maxRadius   = min(w, h) / 2f * 0.85f
        thumbRadius = maxRadius * 0.40f
        thumbX = centerX; thumbY = centerY
    }

    override fun onDraw(canvas: Canvas) {
        // Outer ring
        canvas.drawCircle(centerX, centerY, maxRadius, outerPaint)
        canvas.drawCircle(centerX, centerY, maxRadius, borderPaint)
        // Thumb
        canvas.drawCircle(thumbX, thumbY, thumbRadius, innerPaint)
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> { active = true; update(event.x, event.y) }
            MotionEvent.ACTION_MOVE -> if (active) update(event.x, event.y)
            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                active = false
                thumbX = centerX; thumbY = centerY
                onMove?.invoke(0f, 0f)
                invalidate()
            }
        }
        return true
    }

    private fun update(tx: Float, ty: Float) {
        val dx = tx - centerX; val dy = ty - centerY
        val dist = hypot(dx, dy)
        val clamp = if (dist > maxRadius) maxRadius / dist else 1f
        thumbX = centerX + dx * clamp
        thumbY = centerY + dy * clamp
        onMove?.invoke(
            (thumbX - centerX) / maxRadius,
            (thumbY - centerY) / maxRadius
        )
        invalidate()
    }
}
