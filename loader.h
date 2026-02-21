#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>
#include <elf.h>



#if defined(__x86_64__) || defined(__aarch64__) || \
    (defined(__riscv) && __riscv_xlen == 64) || defined(__mips64)

    #define ELF_CLASS    ELFCLASS64
    #define ELF_PHDR_SIZE 56

    typedef Elf64_Ehdr  Elf_Ehdr;
    typedef Elf64_Phdr  Elf_Phdr;
    typedef Elf64_Addr  Elf_Addr;
    typedef uint64_t    elf_ptr_t;   

#elif defined(__i386__) || defined(__arm__) || \
      (defined(__riscv) && __riscv_xlen == 32)

    #define ELF_CLASS    ELFCLASS32
    #define ELF_PHDR_SIZE 32

    typedef Elf32_Ehdr  Elf_Ehdr;
    typedef Elf32_Phdr  Elf_Phdr;
    typedef Elf32_Addr  Elf_Addr;
    typedef uint32_t    elf_ptr_t;

#else
    #error "Unsupported architecture"
#endif



#if defined(__x86_64__)
    #define ELF_EXPECTED_MACHINE EM_X86_64
    #define ARCH_NAME "x86_64"
#elif defined(__i386__)
    #define ELF_EXPECTED_MACHINE EM_386
    #define ARCH_NAME "i386"
#elif defined(__aarch64__)
    #define ELF_EXPECTED_MACHINE EM_AARCH64
    #define ARCH_NAME "aarch64"
#elif defined(__arm__)
    #define ELF_EXPECTED_MACHINE EM_ARM
    #define ARCH_NAME "arm"
#elif defined(__riscv)
    #define ELF_EXPECTED_MACHINE EM_RISCV
    #define ARCH_NAME "riscv"
#elif defined(__mips64)
    #define ELF_EXPECTED_MACHINE EM_MIPS
    #define ARCH_NAME "mips64"
#else
    #error "Unsupported architecture"
#endif



typedef struct {
    uint64_t base;
    uint64_t entry;
    uint64_t phdr_addr;
    uint16_t phnum;

} elf_info_t;



int load_elf(const char *path, elf_info_t *info);

__attribute__((noreturn))
void run_elf(const elf_info_t *info, int argc, char **argv);

extern void launch_entry(elf_ptr_t *sp, elf_ptr_t entry)
    __attribute__((noreturn));

#endif