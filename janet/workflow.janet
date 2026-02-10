(import ev)
(import raylib)
(import spork/netrepl)

(def state @{})
(def entities @[])
(def update (fn [_ _] nil))
(def draw (fn [_] nil))
(def default-background (tuple 18 18 18 255))
(def frame-fiber nil)
(def step-fn nil)
(def default-netrepl-port "9365")
(def netrepl-state @{:server nil :host "127.0.0.1" :port default-netrepl-port})

(defn init [title width height &named fps]
  (raylib/init-window width height title)
  (raylib/set-target-fps (or fps 60))
  (put state :title title)
  (put state :size (tuple width height))
  (put state :background default-background)
  state)

(defn set-background [r g b a]
  (put state :background (tuple r g b a)))

(defn add-entity [entity]
  (array/push entities entity)
  entity)

(defn clear-entities []
  (array/clear entities))

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
  (raylib/end-drawing)
  (when (get netrepl-state :server)
    (ev/sleep 0)))

(set step-fn step)

(defn run-frames [frames]
  (dotimes [_ frames]
    (when (raylib/window-should-close)
      (break))
    (step)))

(defn run []
  (while (not (raylib/window-should-close))
    (step))
  (stop-netrepl)
  (raylib/close-window))

(defn reset-loop []
  (set step-fn step)
  (set frame-fiber
    (fiber (fn []
      (while (not (raylib/window-should-close))
        (step-fn)
        (yield :frame)))))
  frame-fiber)

(defn step-frame []
  (when (nil? frame-fiber)
    (reset-loop))
  (resume frame-fiber))

(defn start-netrepl [&named host port]
  (def server-host (or host (get netrepl-state :host)))
  (def port-value (or port (get netrepl-state :port)))
  (def server-port (if (string? port-value) port-value (string port-value)))
  (def env (fiber/getenv (fiber/current)))
  (put netrepl-state :host server-host)
  (put netrepl-state :port server-port)
  (put netrepl-state :server
       (try
         (netrepl/server server-host server-port env)
         (catch err
           (error (string "Failed to start netrepl server: " err)))))
  netrepl-state)

(defn stop-netrepl []
  (when (get netrepl-state :server)
    (:close (get netrepl-state :server))
    (put netrepl-state :server nil))
  netrepl-state)

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
