#ifndef ASM_WRITER_H
#define ASM_WRITER_H

void write_asm_instructions(const char *vm_line);
void writer_init(FILE *asm_file, char *root_file_name);

#endif

