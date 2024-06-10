

read_char:
    push 0
    mov rax, 0 ; read syscall
    mov rdi, 0 ; stdin
    lea rsi, [rsp] ; buffer pointer
    mov rdx, 1 ; amount of bytes to read
    syscall
    pop rax
    ret

global _start
_start:
    xor r15, r15 ; total
    xor r13,r13
    xor r14,r14 ; amount of digits
    jmp space

space:
    call read_char
    cmp rax, ' '
    je space
    jmp digit

digit:
    cmp rax, 48
    jb end
    cmp rax, 57
    ja end
    sub rax, 48 ; substract base
    push rax
    inc r14
    call read_char
    jmp digit

process_digits:
    mov rax, 1
    xor rdi, rdi
    mov rsi, 10
extent:
    cmp rdi, r13
    jae adding
    mul rsi
    inc rdi
    jmp extent
adding:
    pop rdi
    mul rdi
    add r15, rax
    inc r13
    cmp r14, r13
    ja process_digits
    jmp exit

end:
    jmp process_digits
exit:
    mov rdi, r15
    mov rax, 60
    syscall

