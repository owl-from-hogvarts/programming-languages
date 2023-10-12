%include "words.inc"
%include "dict.inc"
%include "lib.inc"

section .data

ERR_TOO_LONG: dq "String is too long!", 0


section .text
global _start

%define SYS_IOCTL 16
%define SYS_READ 0
%define FIONREAD 0x541B
%define STDIN 0
%define STRING_BUFFER_SIZE 256
%define SIZE_POINTER 8

; r14 - input string length
_start:
    ; get number of available bytes via ioctl
    ; read input
    sub rsp, STRING_BUFFER_SIZE ; str buffer, aligned by 16
    mov rax, SYS_READ
    mov rdi, STDIN
    mov rsi, rsp
    mov rdx, STRING_BUFFER_SIZE
    syscall
    test rax, -1
    js .error

    mov r14, rax
    mov byte[rsp+r14], 0 ; string should be null terminated

    mov rdi, rsp         ; pointer to string
    mov rsi, first_word  ; pointer to dictionary
    call find_word
    test rax, -1
    jz .error

    lea r15, [rax+r14+1+SIZE_POINTER]  ; +1 for null terminator; string_length does not count null terminator
                          ; get value
    call print_newline
    mov rdi, r15
    call print_string

    xor rdi, rdi ; reset rdi before call
    call exit

    .error:
        mov rdi, 15
        call exit
    ; if amount is too large, print error message and exit with error code 1
    ; pass input string to find_word


; rdi - fd to check
; -> ssize - amount of available byte; -1 indicates error

; internal
; rdi - amount of bytes availble
get_availbale_bytes:
    mov rax, SYS_IOCTL
    ; rdi already contains fd
    mov rsi, FIONREAD
    mov rdx, rsp

    push 0
    syscall
    pop rdi

    test rax, -1
    jz .exit
    ret
    .exit:
        mov rax, rdi
        ret



