section .data
message: db  'hello, world!', 10, 0

section .text

exit:
    mov  rax, 60
    xor  rdi, rdi          
    syscall

string_length:
    mov  rax, rdi
  .counter:
    cmp  byte [rdi], 0
    je   .end
    inc  rdi
    jmp  .counter
  .end:
    sub  rdi, rax
    mov  rax, rdi
    ret

; void print_string(uint8_t* str, uint64_t len)
print_string:
    push rdi
    call string_length
    pop rdi
    mov rdx, rax ; string length
    mov  rsi, rdi ; string pointer
    mov  rax, 1   ; syscall number
    mov  rdi, 1   ; stdout fd
    syscall
    ret

