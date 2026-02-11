;;; Directory Local Variables for Raylib Tutorial
;;; Usage: M-x compile to build and run current subproject

((c++-mode
  . ((eval
      . (progn
          (let* ((current-dir (file-name-directory (buffer-file-name)))
                 ;; 找到包含顶层 CMakeLists.txt 的项目根目录
                 (project-root
                  (expand-file-name
                   (locate-dominating-file
                    (locate-dominating-file current-dir "CMakeLists.txt")
                    ".git")))
                 ;; 获取相对于项目根目录的路径
                 (relative-path (file-relative-name current-dir project-root))
                 ;; 判断是在 chapters 还是 games 下
                 (subproject-match
                  (string-match "^\\(chapters\\|games\\)/\\([^/]+\\)" relative-path))
                 (category (when subproject-match (match-string 1 relative-path)))
                 (subproject (when subproject-match (match-string 2 relative-path)))
                 ;; 从当前目录的 CMakeLists.txt 中提取可执行文件名
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
                         (progn (skip-chars-forward "^ )\t\n") (point))))))))
            (if (and project-root category subproject executable-name)
                (setq compile-command
                      (format "cd %s && cmake -B build -S . && cmake --build build && ./build/bin/%s/%s"
                              (shell-quote-argument project-root)
                              category
                              executable-name))
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
                 (subproject-match
                  (string-match "^\\(chapters\\|games\\)/\\([^/]+\\)" relative-path))
                 (category (when subproject-match (match-string 1 relative-path)))
                 (subproject (when subproject-match (match-string 2 relative-path)))
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
                         (progn (skip-chars-forward "^ )\t\n") (point))))))))
            (if (and project-root category subproject executable-name)
                (setq compile-command
                      (format "cd %s && cmake -B build -S . && cmake --build build && ./build/bin/%s/%s"
                              (shell-quote-argument project-root)
                              category
                              executable-name))
              (when project-root
                (setq compile-command
                      (format "cd %s && cmake -B build -S . && cmake --build build"
                              (shell-quote-argument project-root))))))))))
 (nil . ((indent-tabs-mode . nil)
         (tab-width . 4)
         (c-basic-offset . 4)
         (fill-column . 100))))
