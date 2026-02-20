(import raylib)

# Minimal sanity check for the native module.
# Opens a window, draws a few frames, then exits.

(def w 800)
(def h 450)

(raylib/init-window w h "Janet raylib smoke test")
(raylib/set-target-fps 60)
(def started-at (raylib/get-time))

(var frames 0)
(while (and (< frames 120) (not (raylib/window-should-close)))
  (def elapsed (- (raylib/get-time) started-at))
  (def sw (raylib/get-screen-width))
  (def sh (raylib/get-screen-height))
  (raylib/is-key-pressed raylib/KEY_SPACE)
  (raylib/is-key-released raylib/KEY_SPACE)
  (raylib/is-key-down raylib/KEY_W)
  (raylib/is-mouse-button-down raylib/MOUSE_BUTTON_LEFT)
  (raylib/is-mouse-button-pressed raylib/MOUSE_BUTTON_LEFT)
  (raylib/get-mouse-position)
  (raylib/begin-drawing)
  (raylib/clear-background 18 18 18 255)
  (raylib/draw-text "raylib_janet: smoke test" 20 20 20 230 230 230 255)
  (raylib/draw-text (string "size=" sw "x" sh " t=" elapsed) 20 44 20 180 180 180 255)
  (raylib/draw-rectangle 20 60 frames 18 255 196 0 255)
  (raylib/end-drawing)
  (set frames (+ frames 1)))

(raylib/close-window)
