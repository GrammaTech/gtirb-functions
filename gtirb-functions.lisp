;;; Copyright (C) 2020 GrammaTech, Inc.
;;;
;;; This code is licensed under the MIT license. See the LICENSE file in
;;; the project root for license terms.
;;;
;;; This project is sponsored by the Office of Naval Research, One Liberty
;;; Center, 875 N. Randolph Street, Arlington, VA 22203 under contract #
;;; N68335-17-C-0700.  The content of the information does not necessarily
;;; reflect the position or policy of the Government and no official
;;; endorsement should be inferred.
(defpackage :gtirb-functions/gtirb-functions
  (:nicknames :gtirb-functions)
  (:use :gt :gtirb :graph :stefil)
  (:shadow :size :symbol :copy)
  (:export :func
           :functions
           :entries
           :exits
           :returns
           :test))
(in-package :gtirb-functions/gtirb-functions)
(in-readtable :curry-compose-reader-macros)

(defclass func (gtirb-node)
  ((module :initarg :module :accessor module :type module
           :documentation "Module containing the function.")
   (name :initarg :name :accessor name :type '(or null symbol)
         :documentation "Name of the function, if present.")
   (blocks :initarg :blocks :accessor blocks
           :type '(list (or code-block proxy-block))
           :documentation "Blocks in the function.")
   (entries :initarg :entries :accessor entries
            :type '(list (or code-block proxy-block))
            :documentation "Blocks serving as entry points to the function."))
  (:documentation "A function in a GTIRB instance."))

(defmethod ir ((func func)) (ir (module func)))

(defgeneric exits (object)
  (:documentation "Return the blocks that exit OBJECT.")
  (:method ((obj func))
    (let ((cfg (cfg (gtirb (module obj)))))
      (remove-if-not
       (lambda (node)
         (remove-if «or [{intersection (mapcar #'uuid (entries obj))} #'cdr]
                        [#'not {equal (uuid node)} #'car]»
                    (node-edges cfg (uuid node))))
       (entries obj)))))

(defgeneric returns (object)
  (:documentation "Return the blocks that return from OBJECT.")
  (:method ((obj func))
    (let ((cfg (cfg (gtirb (module obj)))))
      (remove-if-not [{some [{eql :return} #'edge-type {edge-value cfg}]}
                      {node-edges cfg} #'uuid]
                     (exits obj)))))

(defgeneric functions (object)
  (:documentation "Return all functions in OBJECT.")
  (:method ((obj gtirb)) (mappend #'functions (modules obj)))
  (:method ((obj module) &aux results)
    (let ((entries
           (aux-data-data (cdr (assoc "functionEntries"
                                      (aux-data obj) :test #'string=))))
          (blocks
           (aux-data-data (cdr (assoc "functionBlocks"
                                      (aux-data obj) :test #'string=))))
          (names (if-let ((data (cdr (assoc "functionNames"
                                            (aux-data obj) :test #'string=))))
                   (aux-data-data data)
                   (make-hash-table))))
      (maphash
       (lambda (key value)
         (push (make-instance 'func
                 :name (when-let ((uuid (gethash key names)))
                         (get-uuid uuid obj))
                 :module obj
                 :blocks (mapcar {get-uuid _ obj} value)
                 :entries (mapcar {get-uuid _ obj} (gethash key entries)))
               results))
       ;; Map over blocks because no function can exist without at
       ;; least one block--as opposed to entries and names which are
       ;; not guaranteed to exist.
       blocks)
      results)))


;;;; Main test suite.
(defsuite test)
(in-suite test)

(defvar *hello*)

(defvar *base-dir* (nest (make-pathname :directory)
                         (pathname-directory)
                         #.(or *compile-file-truename*
                               *load-truename*
                               *default-pathname-defaults*)))

(defixture hello
  (:setup (setf *hello* (read-gtirb (merge-pathnames "tests/hello.v1.gtirb"
                                                     *base-dir*))))
  (:teardown (setf *hello* nil)))

(deftest functions-are-returned ()
  (with-fixture hello
    (let ((hello-functions (functions *hello*)))
      (is (and (listp hello-functions) (not (emptyp hello-functions))))
      (is (every #'blocks hello-functions))
      (is (every #'entries hello-functions))
      (is (every (lambda (function)
                   (every {member _ (blocks function)} (entries function)))
                 hello-functions))
      (is (some #'exits hello-functions))
      (is (some #'returns hello-functions))
      (is (every «>= [#'length #'exits] [#'length #'returns]»
                 hello-functions)))))
