%include "words.inc"
%include "dict.inc"
%include "lib.inc"

section .text
global _start

%define SYS_IOCTL 16
%define FIONREAD 0x541B
%define STDIN 0

_start:
    ; get number of available bytes via ioctl
    push 0

    mov rax, SYS_IOCTL
    mov rdi, STDIN
    mov rsi, FIONREAD
    mov rdx, rsp
    syscall
    pop rax

    call exit
    ; if amount is too large, print error message and exit with error code 1
    ; read input
    ; pass input string to find_word

