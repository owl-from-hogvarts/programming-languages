section .text
 
; syscall register
; rax - syscall id
; rdi
; rsi
; rdx
; r10 - regular functions uses rcx!
; r8
; r9

; return - rax
; rcx and r11 are modified!!!
 

; callee saved register:
; rbx
; rbp
; rsp
; r12-r15


; Принимает код возврата и завершает текущий процесс
exit: 
    mov rax, 60
    syscall     ; forward rdi redgister to syscall
    ret

; Принимает указатель на нуль-терминированную строку, возвращает её длину
; rdi - char*
string_length:
    xor rax, rax ; clear return register
    loop:
        test byte[rdi+rax], 0xff ; use rax as offset
        jz end
        inc rax
        jmp loop
    end:
        ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
; rdi - char*
; modifies:
; rdx, rsi, rdi
print_string:
    ; str_len = string_length(str)
    push rdi
    call string_length
    ; string_length already returns length without null
    ; terminator
    mov rdx, rax ; count
    pop rdi
    mov rsi, rdi ; strPointer
    mov rdi, 1 ; stdout
    mov rax, 1 ; syscall write
    syscall
    ret

; Принимает код символа и выводит его в stdout
; rdi - char
; modifies:
; rdx, rsi, rdi
print_char:
    push rdi     ; make four byte buffer in stack
    mov rsi, rsp ; load address of "buffer"
    mov rdi, 1   ; sysout
    mov rdx, 1   ; amount of bytes to print
    mov rax, 1   ; syscall write
    syscall
    pop rdi
    ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    ; uses backtickets to allow character escapes
    ; see https://nasm.us/doc/nasmdoc3.html chapter 3.4.2
    mov rdi, `\n`
    call print_char
    ret

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
; rdi - u64
; internal:
; rbx - string cursor
; r13 - address of string buffer
print_uint:
    push r13
    sub rsp, 80 ; init string buffer
    mov r13, rsp
    mov rax, rdi ; store number in rax; divide it until zero
    mov rdi, 10
    xor rbx, rbx
    .loop: ; until number is zero
        xor rdx, rdx ; rdx:rax / src
        ; divide number by 10
        ; reminder is digit ; quotient is number for next cycle
        div rdi
        ; add 48 to modulo
        add rdx, 48
        mov byte[r13+rbx], dl
        inc rbx
        test rax, -1
        je .print
        jmp loop
    .print:
        dec rbx
        push rbx
        mov rdi, [r13+rbx]
        call print_char
        pop rbx
        test rbx, -1
        jz .exit
        jmp .print
    .exit:
        add rsp, 80
        pop r13
        ret

; computes power of number
; rdi - u64 - number
; rsi - extend
; internal:
; r13 - extend counter
power:
    push r13
    xor r13, r13 ; extent counter
    mov rax, 1
    .power:
        cmp r13, rsi
        jae .exit
        mul rdi
        inc r13
        jmp .power
    .exit:
        pop r13
        ret ; rax already should contain powered number
; Выводит знаковое 8-байтовое число в десятичном формате 
print_int:
    xor rax, rax
    ret

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
; rdi - str1
; rsi - str2
; internal:
; rbx - first string length ; string cursor within loop
; r14 - single byte buffer within loop
string_equals:
    ; check lengths - strings equals only when their lengths are equal
    ; loop and compare each character
    push r14         ; callee saved
    push rbx         ; callee saved
    push rdi
    push rsi
    call string_length
    mov rbx, rax     ; first string length
    mov rdi, [rsp]   ; second string pointer
    call string_length
    xor rax, rbx
    pop rsi
    pop rdi
    jne .fail

    .compare:
        mov rax, 1
        xor rbx, rbx ; reuse; string cursor

        .loop:
            mov r14b, [rdi+rbx]
            test r14b, 0xff ; test for null terminator
            je .exit
            xor r14b, byte[rsi+rbx]
            jne .fail
            inc rbx
            jmp .loop

    .fail:
        xor rax, rax
    .exit:
        pop rbx      ; restore callee saved rbx
        pop r14
        ret


; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    ; allocate single char buffer on stack
    ; read it lol ;)
    ; check returned code
    ; if zero, dealocate stack and return zero
    xor rax, rax ; set to zero to simplify end of stream condition
    push rax
    mov rax, 0   ; syscall read id
    mov rdi, 0   ; stdin fd
    mov rsi, rsp ; buffer pointer
    mov rdx, 1   ; amount of bytes
    syscall
    pop rax
    ret 

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
    ret
 

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось

; rdi - str: u8*
; internal:
; rbx - string cursor
; r14 - char buffer | current digit
; r15 - accumulate number here
parse_uint:
    push rbx ; explicit reset required
    push r14 ; reset on every new symbol arrival
    push r15 ; explicit reset required
    xor rbx, rbx
    xor r15, r15

    .loop:
        mov r14b, byte[rdi+rbx] ; load char
        test r14b, 0xff ; check for null terminator
        jz .exit        ; explicit check for *zero*
        sub r14b, 48 ; conver ascii code point into actual number
        jl .fail
        cmp r14b, 9 ; check if within digit range
        ja .fail
        push rdi
        mov rdi, 10
        mov rsi, rbx ; cursor acts as extent
                     ; rbx is callee saved
        call power
        ; adding
        mul r14b
        add r15, rax
        inc rbx
        pop rdi
        jmp .loop

    .fail:
        xor rbx, rbx
    .exit:
        mov rdx, rbx
        mov rax, r15
        pop r15
        pop r14
        pop rbx
        ret




; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
    xor rax, rax
    ret 

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
; rdi - str:    u8*
; rsi - buffer: u8*
; rdx - str_length: u64
; internal:
; rbx - string cursor
; r14 - char buffer
string_copy:
    ; get string length
    push rbx
    push r14
    xor rbx, rbx ; string cursor

    .loop:
        mov r14b, [rdi+rbx]
        mov [rsi+rbx], r14b ; copy including null terminator
        test r14b, 0xff ; test for null terminator
        je .success
        inc rbx
        cmp rbx, rdx ; cursor < buffer_length
        jle .loop
        xor rax, rax
        jmp .exit

    .success:
        mov rax, rbx
    .exit:
        pop r14
        pop rbx
        ret
