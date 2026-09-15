[BITS 32]
global context_switch

; void context_switch(uint32_t *old_esp, uint32_t new_esp)
context_switch:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi

    mov eax, [ebp + 8]
    mov [eax], esp

    mov esp, [ebp + 12]

    pop edi
    pop esi
    pop ebx

    pop ebp
    ret
