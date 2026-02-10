(import spork/msg)

# One-shot netrepl client: connect to localhost:9365 and set the demo circle color to red
# by hot-replacing the host's draw function.

(def host "127.0.0.1")
(def port "9365")

(defn recv-skip-oob [recv]
  (var msg (recv))
  (while (and msg (> (length msg) 0) (= (get msg 0) 0xFF))
    (set msg (recv)))
  (when (and msg (> (length msg) 0) (= (get msg 0) 0xFE))
    (set msg (string/slice msg 1)))
  msg)

(with [stream (net/connect host port)]
  (def send (msg/make-send stream))
  (def recv (msg/make-recv stream))

  (send (string/format "\xFF%j" {:auto-flush true :name "raylib-set-red"}))
  (recv-skip-oob recv)

  (send
    (string/format
      "\xFF%j"
      '(do
         (defn draw [state]
           (raylib/draw-text "Connect via netrepl client" 20 20 20 230 230 230 255)
           (raylib/draw-circle (get demo-state :x) (get demo-state :y) (get demo-state :radius) 255 0 0 255))
         (set workflow/draw draw)
         :ok)))

  (def resp (recv-skip-oob recv))
  (def [ok result] (-> resp parse protect))
  (when (not ok)
    (error (string "eval failed: " result)))
  (print "eval result: " (get result 1)))

