/*
 * launch_entry 
 *
 * void launch_entry(uint64_t *sp, uint64_t entry);
 *                    arg0          arg1
 */

    .global launch_entry
    .type   launch_entry, @function

launch_entry:


#if defined(__x86_64__)

    mov     %rdi, %rsp
    mov     %rsi, %rax

    xor     %rdx, %rdx
    xor     %rcx, %rcx
    xor     %rbx, %rbx
    xor     %rsi, %rsi
    xor     %rdi, %rdi
    xor     %rbp, %rbp
    xor     %r8,  %r8
    xor     %r9,  %r9
    xor     %r10, %r10
    xor     %r11, %r11
    xor     %r12, %r12
    xor     %r13, %r13
    xor     %r14, %r14
    xor     %r15, %r15

    jmp     *%rax


#elif defined(__i386__)

    mov     4(%esp), %eax       
    mov     8(%esp), %ecx     

    mov     %eax, %esp

    xor     %eax, %eax
    xor     %ebx, %ebx
    xor     %ecx, %ecx         

    mov     8(%esp), %edx     


    mov     4(%esp), %esp      


    pop     %eax                
    pop     %esp                


_i386_start:
    mov     4(%esp), %edx      
    mov     8(%esp), %eax       
    mov     %edx, %esp         

    xor     %edx, %edx
    xor     %ecx, %ecx
    xor     %ebx, %ebx
    xor     %esi, %esi
    xor     %edi, %edi
    xor     %ebp, %ebp

    jmp     *%eax


#elif defined(__aarch64__)

    mov     sp, x0              
    mov     x30, x1            

    mov     x0,  #0
    mov     x1,  #0
    mov     x2,  #0
    mov     x3,  #0
    mov     x4,  #0
    mov     x5,  #0
    mov     x6,  #0
    mov     x7,  #0
    mov     x8,  #0
    mov     x9,  #0
    mov     x10, #0
    mov     x11, #0
    mov     x12, #0
    mov     x13, #0
    mov     x14, #0
    mov     x15, #0
    mov     x16, #0
    mov     x17, #0
    mov     x18, #0
    mov     x19, #0
    mov     x20, #0
    mov     x21, #0
    mov     x22, #0
    mov     x23, #0
    mov     x24, #0
    mov     x25, #0
    mov     x26, #0
    mov     x27, #0
    mov     x28, #0
    mov     x29, #0            

    br      x30                


#elif defined(__arm__)

    mov     sp, r0             
    mov     r12, r1             

    mov     r0,  #0
    mov     r1,  #0
    mov     r2,  #0
    mov     r3,  #0
    mov     r4,  #0
    mov     r5,  #0
    mov     r6,  #0
    mov     r7,  #0
    mov     r8,  #0
    mov     r9,  #0
    mov     r10, #0
    mov     r11, #0            
    mov     lr,  #0

    bx      r12                 


#elif defined(__riscv) && __riscv_xlen == 64

    mv      sp, a0              
    mv      t0, a1             

    li      a0, 0
    li      a1, 0
    li      a2, 0
    li      a3, 0
    li      a4, 0
    li      a5, 0
    li      a6, 0
    li      a7, 0
    li      s0, 0
    li      s1, 0
    li      s2, 0
    li      s3, 0
    li      s4, 0
    li      s5, 0
    li      s6, 0
    li      s7, 0
    li      s8, 0
    li      s9, 0
    li      s10, 0
    li      s11, 0
    li      t1, 0
    li      t2, 0
    li      t3, 0
    li      t4, 0
    li      t5, 0
    li      t6, 0
    li      ra, 0
    li      gp, 0
    li      tp, 0

    jr      t0                  


#elif defined(__mips64)

    move    $sp, $a0           
    move    $t9, $a1         

    move    $a0, $zero
    move    $a1, $zero
    move    $a2, $zero
    move    $a3, $zero
    move    $s0, $zero
    move    $s1, $zero
    move    $s2, $zero
    move    $s3, $zero
    move    $s4, $zero
    move    $s5, $zero
    move    $s6, $zero
    move    $s7, $zero
    move    $fp, $zero
    move    $ra, $zero

    jr      $t9                
    nop                         

#else
    #error "Unsupported architecture"
#endif

    .size   launch_entry, . - launch_entry