
%include "lib.inc"
; find_word loops through the whole dictionary, comparing a given key with
; each key in dictionary. If the record is not found, it returns zero; otherwise it
; returns record address.

; rdi - str: u8[] ; pointer to a null terminated string
; rsi - dict: u64 ; pointer to a last dictionary element ; reused as pointer to current element
find_word:
    .loop:
        push rdi
        push rsi
        add rsi, 8 ; dangerouse move; assume that pointer is always 8 bytes long
        call string_equals
        pop rsi
        pop rdi
        test rax, -1
        jnz .exit ; if 1 (true) strings are equal
        test rsi, -1
        jz .exit 
        mov rsi, [rsi] ; follow pointer
        jmp .loop
    .exit:
        ; rax already contains comparison result
        cmovnz rax, rsi
        ret
        
