(import raylib)

# Optional deps:
# - ev: used for yielding/sleeping when running netrepl
# - spork/netrepl: REPL-over-socket support
#
# Homebrew's janet package may ship without these modules. The workflow core
# (init/step/run/run-frames) should still work without them.

(var have-ev (not (nil? (get (fiber/getenv (fiber/current)) (quote ev/sleep)))))

(var have-netrepl false)
(try
  (do
    (require "spork/netrepl")
    (set have-netrepl true))
  ([_e]
    (set have-netrepl false)))

(def state @{})
(def entities @[])
(var update (fn [_ _] nil))
(var draw (fn [_] nil))
(def default-background (tuple 18 18 18 255))
(var frame-fiber nil)
(var step-fn nil)
(def default-netrepl-port 9365)
(def netrepl-state
  @{:server nil :host "127.0.0.1" :port (string default-netrepl-port)})

(defn stop-netrepl []
  (when (get netrepl-state :server)
    (:close (get netrepl-state :server))
    (put netrepl-state :server nil))
  netrepl-state)

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
  (when (and have-ev (get netrepl-state :server))
    (ev/sleep 0)))

(set step-fn step)

(defn run-frames [frames]
  (var i 0)
  (while (< i frames)
    (when (raylib/window-should-close)
      (break))
    (step)
    (set i (+ i 1))))

(defn run []
  (while (not (raylib/window-should-close))
    (step))
  (stop-netrepl)
  (raylib/close-window))

(defn reset-loop []
  (set step-fn step)
  (set frame-fiber
    (fiber/new
      (fn []
        (while (not (raylib/window-should-close))
          (step-fn)
          (yield :frame)))
      :i))
  frame-fiber)

(defn step-frame []
  (when (nil? frame-fiber)
    (reset-loop))
  (resume frame-fiber))

(defn start-netrepl [&named host port]
  (when (not have-netrepl)
    (error "spork/netrepl is not available (module not found in JANET_PATH)"))
  (def server-host (or host (get netrepl-state :host)))
  (def port-value (or port (get netrepl-state :port)))
  (def server-port (if (string? port-value) port-value (string port-value)))
  (def env (fiber/getenv (fiber/current)))
  (def netrepl-env (require "spork/netrepl"))
  (def server-entry (get netrepl-env 'server))
  (def server-fn (get server-entry :value))
  (when (nil? server-fn)
    (error "spork/netrepl missing expected binding: server"))
  (put netrepl-state :host server-host)
  (put netrepl-state :port server-port)
  (put netrepl-state :server
       (try
         (server-fn server-host server-port env)
         ([err]
           (error (string "Failed to start netrepl server on "
                          server-host ":" server-port ": " err)))))
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
