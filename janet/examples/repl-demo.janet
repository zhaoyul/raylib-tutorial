(import raylib)
(import workflow)

(workflow/init "Janet REPL Demo" 800 450 :fps 60)

(def demo-state @{:x 200 :y 200 :vx 160 :vy 120 :radius 24})

(defn update [dt state]
  (let [x (+ (get demo-state :x) (* (get demo-state :vx) dt))
        y (+ (get demo-state :y) (* (get demo-state :vy) dt))]
    (when (or (< x 24) (> x 776))
      (set demo-state :vx (- (get demo-state :vx))))
    (when (or (< y 24) (> y 426))
      (set demo-state :vy (- (get demo-state :vy))))
    (set demo-state :x x)
    (set demo-state :y y)))

(defn draw [state]
  (raylib/draw-text "Edit update/draw in REPL!" 20 20 20 230 230 230 255)
  (raylib/draw-circle (get demo-state :x) (get demo-state :y) (get demo-state :radius) 255 196 0 255))

(set workflow/update update)
(set workflow/draw draw)

(workflow/run)
