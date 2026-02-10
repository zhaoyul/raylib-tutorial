(import spork/msg)

# Non-interactive smoke client for spork/netrepl.
# Connects, sends one eval request, prints the returned value, and exits.

(def host "127.0.0.1")
(def port "9365")

(defn recv-skip-oob [recv]
  (var msg (recv))
  (while (and msg (> (length msg) 0) (= (get msg 0) 0xFF))
    # Out-of-band flush output from server.
    (set msg (recv)))
  (when (and msg (> (length msg) 0) (= (get msg 0) 0xFE))
    (set msg (string/slice msg 1)))
  msg)

(with [stream (net/connect host port)]
  (def send (msg/make-send stream))
  (def recv (msg/make-recv stream))

  # 1) handshake: client options + name
  (send (string/format "\xFF%j" {:auto-flush true :name "raylib-smoke-client"}))
  (recv-skip-oob recv) # prompt

  # 2) eval: move the demo circle (defined in netrepl-host.janet)
  (send (string/format "\xFF%j" '(do (put demo-state :x 500) (get demo-state :x))))

  (def resp (recv-skip-oob recv))
  (def [ok result] (-> resp parse protect))
  (when (not ok)
    (error (string "eval failed: " result)))
  (print "eval result: " (get result 1)))
