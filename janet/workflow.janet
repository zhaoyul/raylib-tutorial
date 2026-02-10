(import raylib)

(def state @{})
(def entities @[])
(def update (fn [_ _] nil))
(def draw (fn [_] nil))
(def default-background (tuple 18 18 18 255))
(def game-fiber nil)

(defn init [title width height &opt fps]
  (raylib/init-window width height title)
  (raylib/set-target-fps (or fps 60))
  (set state :title title)
  (set state :size (tuple width height))
  (set state :background default-background)
  state)

(defn set-background [r g b a]
  (set state :background (tuple r g b a)))

(defn add-entity [entity]
  (array/push entities entity)
  entity)

(defn clear-entities []
  (set entities @[]))

(defn step []
  (raylib/begin-drawing)
  (apply raylib/clear-background (get state :background default-background))
  (def dt (raylib/get-frame-time))
  (update dt state)
  (each entity entities
    (when (get entity :update)
      ((get entity :update) dt state entity)))
  (draw state)
  (each entity entities
    (when (get entity :draw)
      ((get entity :draw) state entity)))
  (raylib/end-drawing))

(defn run-frames [frames]
  (dotimes [_ frames]
    (when (raylib/window-should-close)
      (break))
    (step)))

(defn run []
  (while (not (raylib/window-should-close))
    (step))
  (raylib/close-window))

(defn reset-loop []
  (set game-fiber
    (fiber (fn []
      (while (not (raylib/window-should-close))
        (step)
        (yield :frame)))))
  game-fiber)

(defn step-frame []
  (when (nil? game-fiber)
    (reset-loop))
  (resume game-fiber))

(def template
  (string
    "(import raylib)\n"
    "(import workflow)\n\n"
    "(workflow/init \"My Janet Game\" 800 450 :fps 60)\n\n"
    "(defn update [dt state]\n"
    "  nil)\n\n"
    "(defn draw [state]\n"
    "  (raylib/draw-text \"Hello Janet\" 20 20 20 255 255 255 255))\n\n"
    "(set workflow/update update)\n"
    "(set workflow/draw draw)\n\n"
    "(workflow/run)\n"))

(defn save-template [path]
  (spit path template))
