%include "words.inc"
%include "dict.inc"
%include "lib.inc"

section .data

ERR_TOO_LONG: db "String is too long!", 0
ERR_INVALID_INPUT: db "Invalid input!", 0
ERR_NOT_FOUND: db "Element with provided key not found", 0

section .text
global _start

%define SYS_IOCTL 16
%define SYS_READ 0
%define FIONREAD 0x541B
%define STDIN 0
%define STDOUT 1
%define STDERR 2
%define STRING_BUFFER_SIZE 256
%define SIZE_POINTER 8

; r14 - input string length
_start:
    ; get number of available bytes via ioctl
    ; read input
    mov r15, 0xdeadbeafdeadbeaf ; check for stack override
    push r15
    xor r15, r15
    sub rsp, STRING_BUFFER_SIZE ; str buffer, aligned by 16
    mov rdi, rsp
    mov rsi, STRING_BUFFER_SIZE
    call read_key
    test rax, -1
    mov r15, ERR_INVALID_INPUT
    jng .error

    mov rdi, rax         ; pointer to string
    mov rsi, first_word  ; pointer to dictionary
    call find_word
    test rax, -1
    mov r15, ERR_NOT_FOUND
    jz .error

    lea rdi, [rax+rdx+1+SIZE_POINTER]  ; +1 for null terminator; string_length does not count null terminator
                                       ; get value
    mov rsi, STDOUT
    call print_string

    xor rdi, rdi ; reset rdi before call
    call exit

    .error:
        mov rdi, r15
        mov rsi, STDERR
        call print_string
        mov rdi, 1
        call exit
    ; if amount is too large, print error message and exit with error code 1
    ; pass input string to find_word


; rdi - str: u8*
; rsi - size: u64
; internal
; rbx - buffer cursor
; r13 - buffer pointer str: u8*
; r14 - size: u64
; r15 - char buffer
read_key:
    push rbx
    push r13
    push r14
    push r15
    push rdi
    xor rbx, rbx
    mov r13, rdi
    mov r14, rsi ; available le
    xor r15, r15
    .whitespace:
        call read_char
        mov rdi, rax
        mov r15, rax
        call is_space
        ; space -> .whitespace
        test al, -1
        jnz .whitespace
        ; EOF -> .exit
        test r15b, -1
        jz .exit
        ; char -> .word
    .word:
        ; take action
        cmp rbx, r14
        jae .fail ; buffer overflow condition cursor >= length
        mov byte[r13+rbx], r15b
        inc rbx
        ; take new symbol
        call read_char
        mov r15, rax
        ; transit
        ; newline -> .exit
        cmp r15b, `\n`
        je .null_terminator
        ; EOF -> .exit
        test r15b, -1
        jz .null_terminator
        ; char -> .word
        jmp .word
    .null_terminator:
        cmp rbx, r14
        jae .fail ; buffer overflow condition cursor >= length
        mov byte[r13+rbx], 0x0 ; null terninator
                               ; it is not counted as string length
        jmp .exit
    .fail:
        mov r13, 0 ; explicit return of null ptr
        xor rbx, rbx ; reset cursor
    .exit:
        pop rdi
        mov rax, r13
        mov rdx, rbx
        pop r15
        pop r14
        pop r13
        pop rbx
        ret
   




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



