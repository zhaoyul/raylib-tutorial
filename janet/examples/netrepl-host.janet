(import raylib)
(import workflow)

(def window-width 800)
(def window-height 450)
(def radius 24)
(def netrepl-host "127.0.0.1")
(def netrepl-port 9365)

(workflow/init "Janet NetREPL Host" window-width window-height :fps 60)
(workflow/start-netrepl :host netrepl-host :port netrepl-port)

(print "NetREPL listening on " netrepl-host ":" netrepl-port)

(def demo-state @{:x 200 :y 200 :vx 160 :vy 120 :radius radius})

(defn update [dt state]
  (let [x (+ (get demo-state :x) (* (get demo-state :vx) dt))
        y (+ (get demo-state :y) (* (get demo-state :vy) dt))
        min-x radius
        max-x (- window-width radius)
        min-y radius
        max-y (- window-height radius)
        clamped-x (if (< x min-x) min-x (if (> x max-x) max-x x))
        clamped-y (if (< y min-y) min-y (if (> y max-y) max-y y))]
    (when (or (< x min-x) (> x max-x))
      (put demo-state :vx (- (get demo-state :vx))))
    (when (or (< y min-y) (> y max-y))
      (put demo-state :vy (- (get demo-state :vy))))
    (put demo-state :x clamped-x)
    (put demo-state :y clamped-y)))

(defn draw [state]
  (raylib/draw-text "Connect via netrepl client" 20 20 20 230 230 230 255)
  (raylib/draw-circle (get demo-state :x) (get demo-state :y) (get demo-state :radius) 255 196 0 255))

(set workflow/update update)
(set workflow/draw draw)

(workflow/run)
