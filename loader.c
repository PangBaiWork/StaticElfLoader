#include "loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>


static void clear_tls(void)
{
#if defined(__x86_64__)
    #ifndef ARCH_SET_FS
    #define ARCH_SET_FS 0x1002
    #endif
    syscall(SYS_arch_prctl, ARCH_SET_FS, 0);

#elif defined(__i386__)

#elif defined(__aarch64__)
    __asm__ volatile("msr tpidr_el0, xzr");

#elif defined(__arm__)
    #ifdef __NR_set_tls
    syscall(__NR_set_tls, 0);
    #endif

#elif defined(__riscv)

#elif defined(__mips64)


#endif
}


#define PAGE_SIZE  4096
#define PAGE_MASK  (~(uint64_t)(PAGE_SIZE - 1))
#define PAGE_DOWN(x)  ((x) & PAGE_MASK)
#define PAGE_UP(x)    (((x) + PAGE_SIZE - 1) & PAGE_MASK)


static int elf_prot(uint32_t flags)
{
    int prot = 0;
    if (flags & PF_R) prot |= PROT_READ;
    if (flags & PF_W) prot |= PROT_WRITE;
    if (flags & PF_X) prot |= PROT_EXEC;
    return prot;
}

static int check_elf(const Elf_Ehdr *ehdr)
{
 
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "[!] Not an ELF file\n");
        return -1;
    }


    if (ehdr->e_ident[EI_CLASS] != ELF_CLASS) {
        fprintf(stderr, "[!] ELF class mismatch (expect %d, got %d)\n",
                ELF_CLASS, ehdr->e_ident[EI_CLASS]);
        return -1;
    }


    if (ehdr->e_machine != ELF_EXPECTED_MACHINE) {
        fprintf(stderr, "[!] Machine mismatch (expect %d for %s, got %d)\n",
                ELF_EXPECTED_MACHINE, ARCH_NAME, ehdr->e_machine);
        return -1;
    }


    if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN) {
        fprintf(stderr, "[!] Not EXEC or DYN (got %d)\n", ehdr->e_type);
        return -1;
    }

    return 0;
}


int load_elf(const char *path, elf_info_t *info)
{
    int fd, i;
    struct stat st;
    uint8_t *file_buf = MAP_FAILED;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("[!] open");
        return -1;
    }
    if (fstat(fd, &st) < 0) {
        perror("[!] fstat");
        close(fd);
        return -1;
    }


    file_buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_buf == MAP_FAILED) {
        perror("[!] mmap file");
        close(fd);
        return -1;
    }

    Elf_Ehdr *ehdr = (Elf_Ehdr *)file_buf;
    if (check_elf(ehdr) < 0)
        goto fail;

    Elf_Phdr *phdr = (Elf_Phdr *)(file_buf + ehdr->e_phoff);

    printf("[*] ELF:   %s\n", path);
    printf("[*] arch:  %s\n", ARCH_NAME);
    printf("[*] entry: %#lx\n", (unsigned long)ehdr->e_entry);
    printf("[*] phnum: %d\n", ehdr->e_phnum);
    printf("[*] type:  %s\n",
           ehdr->e_type == ET_EXEC ? "EXEC (static)" : "DYN (PIE)");


    uint64_t addr_min = UINT64_MAX;
    uint64_t addr_max = 0;
    int has_load = 0;

    for (i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)
            continue;
        has_load = 1;
        uint64_t start = PAGE_DOWN(phdr[i].p_vaddr);
        uint64_t end   = PAGE_UP(phdr[i].p_vaddr + phdr[i].p_memsz);
        if (start < addr_min) addr_min = start;
        if (end   > addr_max) addr_max = end;
    }

    if (!has_load) {
        fprintf(stderr, "[!] No PT_LOAD segments\n");
        goto fail;
    }

    printf("[*] vaddr: %#lx - %#lx (size %#lx)\n",
           (unsigned long)addr_min,
           (unsigned long)addr_max,
           (unsigned long)(addr_max - addr_min));


    uint64_t base = 0;
    if (ehdr->e_type == ET_DYN) {
        void *reserved = mmap(NULL, addr_max - addr_min,
                              PROT_NONE,
                              MAP_PRIVATE | MAP_ANONYMOUS,
                              -1, 0);
        if (reserved == MAP_FAILED) {
            perror("[!] mmap reserve");
            goto fail;
        }
        base = (uint64_t)reserved - addr_min;
        printf("[*] PIE base: %#lx\n", (unsigned long)base);
    }


    for (i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)
            continue;

        uint64_t seg_start = PAGE_DOWN(phdr[i].p_vaddr);
        uint64_t seg_end   = PAGE_UP(phdr[i].p_vaddr + phdr[i].p_memsz);
        uint64_t map_addr  = base + seg_start;
        uint64_t map_size  = seg_end - seg_start;

        printf("[*] LOAD: vaddr=%#lx memsz=%#lx filesz=%#lx %c%c%c\n",
               (unsigned long)phdr[i].p_vaddr,
               (unsigned long)phdr[i].p_memsz,
               (unsigned long)phdr[i].p_filesz,
               (phdr[i].p_flags & PF_R) ? 'R' : '-',
               (phdr[i].p_flags & PF_W) ? 'W' : '-',
               (phdr[i].p_flags & PF_X) ? 'X' : '-');


        void *seg = mmap((void *)map_addr, map_size,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
                         -1, 0);
        if (seg == MAP_FAILED) {
            fprintf(stderr, "[!] mmap %#lx size %#lx: %s\n",
                    (unsigned long)map_addr,
                    (unsigned long)map_size,
                    strerror(errno));
            goto fail;
        }


        if (phdr[i].p_filesz > 0) {
            memcpy((void *)(base + phdr[i].p_vaddr),
                   file_buf + phdr[i].p_offset,
                   phdr[i].p_filesz);
        }


        if (mprotect((void *)map_addr, map_size, elf_prot(phdr[i].p_flags)) < 0) {
            perror("[!] mprotect");
            goto fail;
        }

        printf("    mapped at %#lx size %#lx\n",
               (unsigned long)map_addr,
               (unsigned long)map_size);
    }


    info->base  = base;
    info->entry = base + ehdr->e_entry;
    info->phnum = ehdr->e_phnum;

    if (ehdr->e_type == ET_EXEC)
        info->phdr_addr = addr_min + ehdr->e_phoff;
    else
        info->phdr_addr = base + ehdr->e_phoff;

    printf("[*] Load complete\n");
    printf("[*] entry:     %#lx\n", (unsigned long)info->entry);
    printf("[*] phdr_addr: %#lx\n", (unsigned long)info->phdr_addr);

    munmap(file_buf, st.st_size);
    close(fd);
    return 0;

fail:
    if (file_buf != MAP_FAILED)
        munmap(file_buf, st.st_size);
    if (fd >= 0)
        close(fd);
    return -1;
}



#ifndef AT_RANDOM
#define AT_RANDOM 25
#endif
#ifndef AT_PHDR
#define AT_PHDR   3
#endif
#ifndef AT_PHENT
#define AT_PHENT  4
#endif
#ifndef AT_PHNUM
#define AT_PHNUM  5
#endif
#ifndef AT_PAGESZ
#define AT_PAGESZ 6
#endif
#ifndef AT_ENTRY
#define AT_ENTRY  9
#endif
#ifndef AT_UID
#define AT_UID    11
#endif
#ifndef AT_EUID
#define AT_EUID   12
#endif
#ifndef AT_GID
#define AT_GID    13
#endif
#ifndef AT_EGID
#define AT_EGID   14
#endif
#ifndef AT_SECURE
#define AT_SECURE 23
#endif
#ifndef AT_NULL
#define AT_NULL   0
#endif

#define PUSH(sp, val)  (*(--(sp)) = (elf_ptr_t)(val))



__attribute__((noreturn))
void run_elf(const elf_info_t *info, int argc, char **argv)
{
    int i;

    size_t stack_size = 0x800000;
    void *stack_mem = mmap(NULL, stack_size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
                           -1, 0);
    if (stack_mem == MAP_FAILED) {
        perror("[!] mmap stack");
        _exit(1);
    }

    elf_ptr_t *sp = (elf_ptr_t *)((uint8_t *)stack_mem + stack_size);


    sp = (elf_ptr_t *)((uint8_t *)sp - 16);
    uint8_t *random_ptr = (uint8_t *)sp;
#ifdef SYS_getrandom
    if (syscall(SYS_getrandom, random_ptr, 16, 0) != 16)
#endif
    {
        for (i = 0; i < 16; i++)
            random_ptr[i] = (uint8_t)(i * 17 + 0x42);
    }


    sp = (elf_ptr_t *)((uintptr_t)sp & ~(uintptr_t)0xF);

    #define N_AUXV 10  
    int total_slots = (N_AUXV + 1) * 2   /* auxv pairs + AT_NULL */
                    + 1                    /* envp NULL */
                    + argc + 1             /* argv + NULL */
                    + 1;                   /* argc */


    if (total_slots % 2 != 0)
        --sp;  /* padding */




    PUSH(sp, 0);                          PUSH(sp, AT_NULL);
    PUSH(sp, (elf_ptr_t)random_ptr);      PUSH(sp, AT_RANDOM);
    PUSH(sp, 0);                          PUSH(sp, AT_SECURE);
    PUSH(sp, (elf_ptr_t)getegid());       PUSH(sp, AT_EGID);
    PUSH(sp, (elf_ptr_t)getgid());        PUSH(sp, AT_GID);
    PUSH(sp, (elf_ptr_t)geteuid());       PUSH(sp, AT_EUID);
    PUSH(sp, (elf_ptr_t)getuid());        PUSH(sp, AT_UID);
    PUSH(sp, (elf_ptr_t)info->entry);     PUSH(sp, AT_ENTRY);
    PUSH(sp, (elf_ptr_t)PAGE_SIZE);       PUSH(sp, AT_PAGESZ);
    PUSH(sp, (elf_ptr_t)info->phnum);     PUSH(sp, AT_PHNUM);
    PUSH(sp, (elf_ptr_t)ELF_PHDR_SIZE);  PUSH(sp, AT_PHENT);
    PUSH(sp, (elf_ptr_t)info->phdr_addr); PUSH(sp, AT_PHDR);


    PUSH(sp, 0);


    PUSH(sp, 0);
    for (i = argc - 1; i >= 0; i--)
        PUSH(sp, (elf_ptr_t)argv[i]);


    PUSH(sp, (elf_ptr_t)argc);

    if ((uintptr_t)sp % 16 != 0) {
        fprintf(stderr, "[!] WARNING: sp=%p not 16-byte aligned!\n", (void*)sp);
    }

    printf("[*] stack sp:  %p\n", (void *)sp);
    printf("[*] argc:      %d\n", argc);
    printf("[*] jumping to %#lx (%s)\n",
           (unsigned long)info->entry, ARCH_NAME);
    printf("[*] ------- target output -------\n");
    fflush(stdout);
    fflush(stderr);

    clear_tls();

    launch_entry(sp, (elf_ptr_t)info->entry);
}