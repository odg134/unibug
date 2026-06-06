option casemap:none

PUBLIC foobar

.code

foobar PROC
    ;
    ; Raw bytes:
    ;   C5 FC 10 02 C5 FC 11 01
    ;
    ; ASM:
    ;   vmovups ymm0, ymmword ptr [rdx]
    ;   vmovups ymmword ptr [rcx], ymm0
    ;
    ; This passes one 32 byte packet from RDX to RCX.
    ;
    db 0C5h, 0FCh, 010h, 002h
    db 0C5h, 0FCh, 011h, 001h

    ret
foobar ENDP

END
