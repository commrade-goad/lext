(define sdl (ffi-open "libSDL2.so"))

(define-macro (c-import scheme-name lib-handle c-name ret-type arg-types . nfixed)
  (let ((func-ptr (gensym)))
    `(begin
       ;; 1. Resolve the C function pointer once at compile/load time
       (define ,func-ptr (ffi-sym ,lib-handle ,c-name))
       ;; 2. Create the Scheme function wrapper
       (define (,scheme-name . args)
         ,(if (null? nfixed)
              `(ffi-call ,func-ptr ',ret-type ',arg-types args)
              `(ffi-call ,func-ptr ',ret-type ',arg-types args ,(car nfixed)))))))

;; Constants
(define SDL_RENDERER_ACCELERATED 2)
(define SDL_INIT_VIDEO 32)
(define SDL_QUIT #x100)
(define SDL_KEYDOWN #x300)
(define SDL_MOUSEMOTION #x400)

;; --- Clean C Imports via Macro ---
(c-import sdl-init                 sdl "SDL_Init"               int     (int))
(c-import sdl-create-window        sdl "SDL_CreateWindow"       pointer (string int int int int int))
(c-import sdl-create-renderer      sdl "SDL_CreateRenderer"     pointer (pointer int int))
(c-import sdl-set-render-drawcolor sdl "SDL_SetRenderDrawColor" void    (pointer int int int int))
(c-import sdl-render-clear         sdl "SDL_RenderClear"        void    (pointer))
(c-import sdl-render-present       sdl "SDL_RenderPresent"      void    (pointer))
(c-import sdl-delay                sdl "SDL_Delay"              void    (int))
(c-import sdl-destroy-window       sdl "SDL_DestroyWindow"      void    (pointer))
(c-import sdl-quit                 sdl "SDL_Quit"               void    ())
(c-import sdl-pollevent            sdl "SDL_PollEvent"          int     (pointer))
(c-import sdl-get-ticks            sdl "SDL_GetTicks"           int     ())

;; --- Libc Imports ---
(define libc (ffi-open #f))
(c-import malloc libc "malloc" pointer (int))
(c-import free   libc "free"   void    (pointer))

;; --- Event Specific Struct Definitions ---
;; Keyboard Event (32 bytes)
(ffi-typedef 'SDL_KeyboardEvent
             '(struct (uint32 type)
                      (uint32 timestamp)
                      (uint32 windowID)
                      (uint8 state)
                      (uint8 repeat)
                      (uint8 padding2)
                      (uint8 padding3)
                      (int32 scancode)
                      (int32 sym)
                      (uint16 mod)
                      (uint32 unused)))

;; Mouse Motion Event (36 bytes)
(ffi-typedef 'SDL_MouseMotionEvent
             '(struct (uint32 type)
                      (uint32 timestamp)
                      (uint32 windowID)
                      (uint32 which)
                      (uint32 state)
                      (int32 x)
                      (int32 y)
                      (int32 xrel)
                      (int32 yrel)))

;; Allocate a single reusable event buffer (56 bytes is the max event size in SDL2)
(define event-ptr (malloc 56))

;; --- Game Logic ---

(sdl-init SDL_INIT_VIDEO)

(define win (sdl-create-window "FFI SDL Window" 0 0 640 480 4))
(define ren (sdl-create-renderer win -1 SDL_RENDERER_ACCELERATED))

(define start-time (sdl-get-ticks))

;; Inner function to drain the event queue completely for the current frame
;; Returns #t to continue running, or #f if a QUIT event was encountered
(define (poll-all-events)
  (if (> (sdl-pollevent event-ptr) 0)
      (let ((type (ffi-deref event-ptr 'int)))
        (cond
          ((= type SDL_QUIT)
           (display "Quit event received! Exiting loop...\n")
           #f) ; Signal to stop the game loop
          ((= type SDL_MOUSEMOTION)
           (let ((motion (ffi-deref event-ptr 'SDL_MouseMotionEvent)))
             (format #t "Mouse Motion: x=~A, y=~A, xrel=~A, yrel=~A\n"
                     (cdr (assoc 'x motion))
                     (cdr (assoc 'y motion))
                     (cdr (assoc 'xrel motion))
                     (cdr (assoc 'yrel motion)))
             (poll-all-events))) ; Keep pulling more events
          ((= type SDL_KEYDOWN)
           (let ((key (ffi-deref event-ptr 'SDL_KeyboardEvent)))
             (format #t "Key Down: scancode=~A, sym=~A\n"
                     (cdr (assoc 'scancode key))
                     (cdr (assoc 'sym key)))
             (poll-all-events))) ; Keep pulling more events
          (else
           (poll-all-events)))) ; Ignore unhandled events and keep going
      #t)) ; No more events left in queue for this frame, continue loop

;; Main Engine Loop
(let loop ()
  ;; 1. Handle all input first
  (if (poll-all-events)
      ;; 2. If no quit was triggered, execute frame updates and rendering
      (begin
        (sdl-set-render-drawcolor ren 0 0 255 255) ; clear to blue
        (sdl-render-clear ren)
        (sdl-render-present ren)

        (sdl-delay 16) ; ~60 frames per second limit

        ;; 3. Check master time constraint safely outside event stream
        #;(if (< (- (sdl-get-ticks) start-time) 5000)
            (loop)
            (display "Time limit reached. Exiting...\n")))
      #f))

(free event-ptr)
(sdl-destroy-window win)
(sdl-quit)
