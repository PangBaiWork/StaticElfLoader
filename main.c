#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loader.h"

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <static-elf> [args...]\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s ./static_elf\n", prog);
    fprintf(stderr, "  %s ./static_elf arg1 arg2\n", prog);
    // exit(1);
}

int main(int argc, char **argv)
{
     char *elf_path;
    if (argc < 2){
        usage(argv[0]);
        elf_path = "./static_elf";
    }

    elf_path = argv[1];
    


    int target_argc = argc - 1;
    char **target_argv = &argv[1];

    printf("=============================\n");
    printf(" Static ELF Loader\n");
    printf("=============================\n");
    printf("[*] target: %s\n", elf_path);
    printf("[*] target argc: %d\n", target_argc);
    for (int i = 0; i < target_argc; i++)
        printf("[*] target argv[%d]: %s\n", i, target_argv[i]);
    printf("\n");


    elf_info_t info;
    if (load_elf(elf_path, &info) < 0) {
        fprintf(stderr, "[!] Failed to load %s\n", elf_path);
        return 1;
    }
    printf("\n");


    run_elf(&info, target_argc, target_argv);


    return 0;
}