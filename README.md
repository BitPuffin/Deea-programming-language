# Deea-programming-language.
Deea is an atempt in making my first programming language because why not?

# How to compile
Just run: make then inside a terminal run ./deea 

# Usage examples

(define square (lambda(x) (* x x)))
(square 2)
(square 6)

((lambda (x) (-x 4)) 8)

(define func1 (lambda (x) (lambda (y) (+ x y))))
(define func2 (func1 12))
(func2 12)

(+ 2 2)
(- 2 2)
(* 3 3)
(/ 9 3)
(+ 1 (+ 1 1))

(quote foo)
(define foo 69)
foo

(define foo (quote bar))
foo
