;;; Directory Local Variables for Raylib Tutorial
;;; Usage: M-x compile to build and run current subproject

((c++-mode
  . ((eval
      . (progn
          (let* ((current-dir (file-name-directory (buffer-file-name)))
                 ;; 找到项目根目录（包含 .git）
                 (project-root
                  (expand-file-name
                   (locate-dominating-file
                    (locate-dominating-file current-dir "CMakeLists.txt")
                    ".git")))
                 ;; 获取相对路径
                 (relative-path (file-relative-name current-dir project-root))
                 ;; 从 CMakeLists.txt 提取可执行文件名
                 (cmake-file (expand-file-name "CMakeLists.txt" current-dir))
                 (executable-name
                  (with-temp-buffer
                    (when (file-exists-p cmake-file)
                      (insert-file-contents cmake-file)
                      (goto-char (point-min))
                      (when (search-forward "add_executable" nil t)
                        (skip-chars-forward " (\t\n")
                        (buffer-substring
                         (point)
                         (progn (skip-chars-forward "^ )\t\n") (point)))))))
                 ;; 判断路径类型并生成对应的可执行文件路径
                 (exe-path
                  (cond
                   ;; 普通 games/xxx 目录
                   ((string-match "^games/\\([^/]+\\)/?$" relative-path)
                    (format "./build/bin/games/%s" executable-name))
                   ;; snake phases 目录
                   ((string-match "^games/snake/phases/\\([^/]+\\)" relative-path)
                    (format "./build/bin/snake-phases/%s" executable-name))
                   ;; chapters 目录
                   ((string-match "^chapters/\\([^/]+\\)" relative-path)
                    (format "./build/bin/chapters/%s" executable-name))
                   ;; 其他情况
                   (t nil))))
            (if (and project-root executable-name exe-path)
                (setq compile-command
                      (format "cd %s && cmake -B build -S . && cmake --build build && %s"
                              (shell-quote-argument project-root)
                              exe-path))
              (when project-root
                (setq compile-command
                      (format "cd %s && cmake -B build -S . && cmake --build build"
                              (shell-quote-argument project-root))))))))))
 (c-mode
  . ((eval
      . (progn
          (let* ((current-dir (file-name-directory (buffer-file-name)))
                 (project-root
                  (expand-file-name
                   (locate-dominating-file
                    (locate-dominating-file current-dir "CMakeLists.txt")
                    ".git")))
                 (relative-path (file-relative-name current-dir project-root))
                 (cmake-file (expand-file-name "CMakeLists.txt" current-dir))
                 (executable-name
                  (with-temp-buffer
                    (when (file-exists-p cmake-file)
                      (insert-file-contents cmake-file)
                      (goto-char (point-min))
                      (when (search-forward "add_executable" nil t)
                        (skip-chars-forward " (\t\n")
                        (buffer-substring
                         (point)
                         (progn (skip-chars-forward "^ )\t\n") (point)))))))
                 (exe-path
                  (cond
                   ((string-match "^games/\\([^/]+\\)/?$" relative-path)
                    (format "./build/bin/games/%s" executable-name))
                   ((string-match "^games/snake/phases/\\([^/]+\\)" relative-path)
                    (format "./build/bin/snake-phases/%s" executable-name))
                   ((string-match "^chapters/\\([^/]+\\)" relative-path)
                    (format "./build/bin/chapters/%s" executable-name))
                   (t nil))))
            (if (and project-root executable-name exe-path)
                (setq compile-command
                      (format "cd %s && cmake -B build -S . && cmake --build build && %s"
                              (shell-quote-argument project-root)
                              exe-path))
              (when project-root
                (setq compile-command
                      (format "cd %s && cmake -B build -S . && cmake --build build"
                              (shell-quote-argument project-root))))))))))
 (nil . ((indent-tabs-mode . nil)
         (tab-width . 4)
         (c-basic-offset . 4)
         (fill-column . 100))))
