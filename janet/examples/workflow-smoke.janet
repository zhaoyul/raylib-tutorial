(import raylib)
(import workflow)

# Smoke test for workflow.janet without optional deps (ev, spork/netrepl).
# Runs a fixed number of frames and exits cleanly.

(workflow/init "Workflow Smoke Test" 800 450 :fps 60)
(workflow/set-background 18 18 18 255)

(var t 0)

(defn update [dt state]
  (set t (+ t dt)))

(defn draw [state]
  (raylib/draw-text "workflow: smoke test" 20 20 20 230 230 230 255)
  (raylib/draw-text (string "t=" t) 20 50 20 180 180 180 255))

(set workflow/update update)
(set workflow/draw draw)

(workflow/run-frames 120)
(raylib/close-window)

