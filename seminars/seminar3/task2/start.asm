%include "print_hex.inc"

global _start
    _start:
           mov  rdi, 0x1122334455667788
           call print_hex
           call exit

