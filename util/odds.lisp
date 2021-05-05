; rogue 3.6.3 weapon odds (new! previously unweighted)

(defparameter *weaps*
  '(("mace" . 4) ("long sword" . 4) ("short bow" . 0) ("arrow" . 1)
    ("dagger" . -4) ("rock" . 1) ("two handed sword" . -1) ("sling" . 0)
    ("dart" . -5) ("crossbow" . 0) ("crossbow bolt" . 1) ("spear" . 3)))

(defun running-odds (objadj prefix &optional (odds 100))
  (let ((running 0) (equally (truncate (/ odds (list-length objadj)))))
    (prog1
        (format nil
                "~a {~&~:@{~@{~#[~;~{    ~a // ~a (~a)~%~}~:;~{    ~a, // ~a (~a)~%~}~]~}~}};" prefix
                (loop for obja in objadj
                      for weight = (+ equally (cdr obja)) then (+ equally
                                                                  (cdr obja))
                      do (incf running weight)
                      collect (list running (car obja) weight)))
      (when (not (= running odds)) (error "leftover ~a" (- running odds))))))

(format t "~a" (running-odds *weaps* "int w_chances[MAXWEAPONS] ="))
