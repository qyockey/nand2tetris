#ifndef ASM_WRITER_H
#define ASM_WRITER_H

void write_asm_instructions(const char *vm_line, const char *vm_file_name);
void writer_init(const char *asm_file_path);
void writer_dispose();
void write_bootstrap();

#endif

