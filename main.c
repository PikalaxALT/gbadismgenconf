#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "elf.h"

int main(int argc, char ** argv)
{
    // Usage: gbadismgenconf ELF
    if (argc < 2) {
        FATAL_ERROR("insufficient arguments\n");
    }

    FILE * elfFile = fopen(argv[1], "rb");
    if (elfFile == NULL) {
        FATAL_ERROR("unable to open file %s for reading\n", argv[1]);
    }

    Elf32_Ehdr ehdr;
    fread(ehdr.e_ident, 1, sizeof(ehdr), elfFile);
    if (memcmp(ELFMAG, ehdr.e_ident, SELFMAG) != 0) {
        FATAL_ERROR("ELF magic does not match\n");
    }
    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
        FATAL_ERROR("ELF must be 32-bit\n");
    }
    if (ehdr.e_machine != EM_ARM) {
        FATAL_ERROR("ELF must be a valid ARM binary\n");
    }

    Elf32_Shdr * shdr = malloc(sizeof(*shdr) * ehdr.e_shnum);
    if (shdr == NULL) {
        FATAL_ERROR("failed to allocate section headers\n");
    }
    fseek(elfFile, ehdr.e_shoff, SEEK_SET);
    fread(shdr, 1, sizeof(*shdr) * ehdr.e_shnum, elfFile);

    Elf32_Shdr * strtab = NULL, * symtab = NULL, * shstrtab = NULL;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) symtab = &shdr[i];
        else if (shdr[i].sh_type == SHT_STRTAB) {
            if (i == ehdr.e_shstrndx) shstrtab = &shdr[i];
            else strtab = &shdr[i];
        }
    }
    if (!(strtab && symtab && shstrtab)) {
        FATAL_ERROR("could not find all of symtab, strtab, and shstrtab\n");
    }
    char cur_name[50];
    Elf32_Sym * sym = malloc(symtab->sh_size);
    if (sym == NULL) {
        FATAL_ERROR("Failed to allocate symtab\n");
    }
    fseek(elfFile, symtab->sh_offset, SEEK_SET);
    fread(sym, 1, symtab->sh_size, elfFile);
    for (int i = 0; i < symtab->sh_size / sizeof(*sym); i++) {
        if (ELF32_ST_TYPE(sym[i].st_info) != STT_FUNC) continue;
        int functype = sym[i].st_value & 1;
        fseek(elfFile, strtab->sh_offset + sym[i].st_name, SEEK_SET);
        fgets(cur_name, 50, elfFile);
        printf(
            "%s 0x%08x %s\n",
            (const char *[]){
                "arm_func",
                "thumb_func"
            }[functype],
            sym[i].st_value & ~1,
            cur_name
        );
    }
    free(sym);
    free(shdr);
    fclose(elfFile);
    return 0;
}
