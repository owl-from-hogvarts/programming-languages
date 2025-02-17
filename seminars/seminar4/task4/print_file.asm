%define O_RDONLY 0 
%define PROT_READ 0x1
%define MAP_PRIVATE 0x2
%define SYS_WRITE 1
%define SYS_OPEN 2
%define SYS_MMAP 9
%define SYS_MUNMAP 11
%define SYS_CLOSE 3
%define SYS_FSTAT 5
%define FD_STDOUT 1

%define PAGE_SIZE 4096

section .data
    ; This is the file name. You are free to change it.
    fname: db 'hello.txt', 0

section .text
global print_file
global print_string

; use exit system call to shut down correctly
exit:
    mov  rax, 60
    xor  rdi, rdi
    syscall

; These functions are used to print a null terminated string
; rdi holds a string pointer
print_string:
    push rdi
    call string_length
    pop  rsi
    mov  rdx, rax 
    mov  rax, SYS_WRITE
    mov  rdi, FD_STDOUT
    syscall
    ret

string_length:
    xor  rax, rax
.loop:
    cmp  byte [rdi+rax], 0
    je   .end 
    inc  rax
    jmp .loop 
.end:
    ret

; This function is used to print a substring with given length
; rdi holds a string pointer
; rsi holds a substring length
print_substring:
    mov  rdx, rsi 
    mov  rsi, rdi
    mov  rax, SYS_WRITE
    mov  rdi, FD_STDOUT
    syscall
    ret

%define STAT_SIZE 144
%define STAT_FILESIZE_OFFSET 48
; rdi - fd
; returns - file size in bytes
get_file_size:
    mov rax, SYS_FSTAT
    sub rsp, STAT_SIZE
    mov rsi, rsp
    syscall
    mov rax, [rsp+STAT_FILESIZE_OFFSET]
    add rsp, STAT_SIZE
    ret

; rdi - pointer to null terminated file name
; internal
; r15 - memory region address
; r14 - fd
; r13 - file size
print_file:
    push r13
    push r14
    push r15
    ; Вызовите open и откройте fname в режиме read only.
    mov  rax, SYS_OPEN
    ; rdi is already present
    mov  rsi, O_RDONLY    ; Open file read only
    mov  rdx, 0 	  ; We are not creating a file
                          ; so this argument has no meaning
    syscall
    ; rax holds the opened file descriptor now
    mov r14, rax ; fd

    ; Вызовите mmap c правильными аргументами
    ; Дайте операционной системе самой выбрать, куда отобразить файл
    ; Размер области возьмите в размер страницы 
    ; Область не должна быть общей для нескольких процессов 
    ; и должна выделяться только для чтения.

    mov rax, SYS_MMAP
    mov rdi, 0 ; auto select address
    mov rsi, PAGE_SIZE ; page size in bytes
    mov rdx, PROT_READ
    mov r10, MAP_PRIVATE
    mov r8, r14
    mov r9, 0
    syscall
    mov r15, rax

    mov rdi, r14
    call get_file_size

    mov rdi, r15
    mov rsi, rax
    call print_substring

    mov rax, SYS_MUNMAP
    mov rdi, r15
    mov rsi, PAGE_SIZE
    syscall

    pop r15
    pop r14
    pop r13

    ret


