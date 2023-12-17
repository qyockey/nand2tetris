#ifndef ASM_WRITER_H
#define ASM_WRITER_H

void write_asm_instructions(const char *vm_line);
void writer_init(const char *asm_file_path);
void writer_dispose();
void write_bootstrap();
void set_current_vm_file_name(const char *vm_file_name);

#endif

