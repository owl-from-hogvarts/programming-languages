section .data
codes:
    db      '0123456789ABCDEF'

section .text
global _start
_start:
    ; number 1122... in hexadecimal format
    call print_strange
    call exit

print_strange:
    push 0xff
    push 0xbb
    push 0xaa
    lea rdi, [rsp+16]
    call print_hex
    lea rdi, [rsp+8]
    call print_hex
    lea rdi, [rsp]
    call print_hex
    pop rax
    pop rax
    pop rax
    xor rax, rax
    ret


print_hex:
    mov rax, 1 ; write syscall id
    mov  rdx, 1 ; amount of chars
    mov  rcx, 8 ; amount of bits
    mov rdi, [rdi]

.loop:
    push rdi
    sub  rcx, 4
	; cl is a register, smallest part of rcx
	; rdi -- eax -- ax -- ah + al
	; rcx -- ecx -- cx -- ch + cl
    sar  rdi, cl
    and  rdi, 0xf

    lea  rsi, [codes + rdi]
    mov  rdi, 1

    ; syscall leaves rcx and r11 changed
    push rcx
    syscall
    pop  rcx

    pop rdi
	; test can be used for the fastest 'is it a zero?' check
	; see docs for 'test' command
    test rcx, rcx
    jnz .loop
    ret
    
exit:
    mov  rax, 60            ; invoke 'exit' system call
    xor  rdi, rdi
    xor rsi, rsi
    syscall
