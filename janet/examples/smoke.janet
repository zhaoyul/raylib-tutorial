(import raylib)

# Minimal sanity check for the native module.
# Opens a window, draws a few frames, then exits.

(def w 800)
(def h 450)

(raylib/init-window w h "Janet raylib smoke test")
(raylib/set-target-fps 60)

(var frames 0)
(while (and (< frames 120) (not (raylib/window-should-close)))
  (raylib/begin-drawing)
  (raylib/clear-background 18 18 18 255)
  (raylib/draw-text "raylib_janet: smoke test" 20 20 20 230 230 230 255)
  (raylib/draw-rectangle 20 60 frames 18 255 196 0 255)
  (raylib/end-drawing)
  (set frames (+ frames 1)))

(raylib/close-window)
