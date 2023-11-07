#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembly_generator.h"
#include "main.h"

static char *append_int(char *dest, int src);
static char *append_str(char *dest, char *src);
static void tokenize(char *asm_line, char **p_segment, int *p_value);
static char *get_push_instructions(char *vm_line, char *asm_line);
static char *append_push_segment_instructions(char *asm_line, char *segment_addr, int p_value);
static char *get_pop_instructions(char *vm_line, char *asm_line);
static char *append_pop_segment_instructions(char *asm_line, char *segment_addr, int p_value);
static char *get_add_instructions(char *asm_line);
static char *get_sub_instructions(char *asm_line);
static char *get_neg_instructions(char *asm_line);
static char *get_eq_instructions(char *asm_line);
static char *get_lt_instructions(char *asm_line);
static char *get_gt_instructions(char *asm_line);
static char *get_and_instructions(char *asm_line);
static char *get_or_instructions(char *asm_line);
static char *get_not_instructions(char *asm_line);
static char *get_binary_op_instructions(char *asm_line, char *binary_op);
static char *get_comp_instructions(char *asm_line, char *comp, int count);

static int eq_count = 0;
static int lt_count = 0;
static int gt_count = 0;

#define LCL 1
#define ARG 2
#define THIS 3
#define THAT 4
#define TEMP_START 5
#define MAX_STR_LEN 16

static char *push_instructions = ""
    "@s_p\n"
    "a_m=m+1\n"
    "a=a-1\n"
    "m=d\n";

void get_asm_instructions(char *vm_line, char *asm_line) {

  asm_line = append_str(asm_line, "// ");
  asm_line = append_str(asm_line, vm_line);
  asm_line = append_str(asm_line, "\n");
  static char *commands[] = {
    "add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"
  };
  static int num_simple_commands = 9;
  static char *(*get_simple_intructions[])(char *) = {
    get_add_instructions, get_sub_instructions, get_neg_instructions,
    get_eq_instructions,  get_gt_instructions,  get_lt_instructions,
    get_and_instructions, get_or_instructions,  get_not_instructions
  };
  if (strncmp(vm_line, "push", 4) == 0) {
    asm_line = get_push_instructions(vm_line, asm_line);
  } else if (strncmp(vm_line, "pop", 3) == 0) {
    asm_line = get_pop_instructions(vm_line, asm_line);
  } else {
    for (int cmd_num = 0; cmd_num < num_simple_commands; cmd_num++) {
      if (strncmp(vm_line, commands[cmd_num], strlen(commands[cmd_num])) == 0) {
        asm_line = get_simple_intructions[cmd_num](asm_line);
        break;
      }
    }
  }
  asm_line = append_str(asm_line, "\n");
}

static char *append_str(char *str, char *data) {
  strcpy(str, data);
  return str + strlen(data);
}

static char *append_int(char *str, int data) {
  char *data_str = malloc(MAX_STR_LEN * sizeof(char));
  if (data_str == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  sprintf(data_str, "%d", data);
  strcpy(str, data_str);
  return str + strlen(data_str);
}

static void tokenize(char *vm_line, char **p_segment, int *p_value) {
  strtok(vm_line, " ");
  *p_segment = strtok(NULL, " ");
  char *value_str = strtok(NULL, " ");
  *p_value = atoi(value_str);
}

static char *get_push_instructions(char *vm_line, char *asm_line) {
  char **p_segment = malloc(sizeof(char **));
  if (p_segment == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  int *p_value = malloc(sizeof(int));
  if (p_value == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  tokenize(vm_line, p_segment, p_value);
  if (strcmp(*p_segment, "local") == 0) {
    // push RAM[*segment_pointer + i]
    asm_line = append_push_segment_instructions(asm_line, "l_cL", *p_value);
  } else if (strcmp(*p_segment, "argument") == 0) {
    // push RAM[*segment_pointer + i]
    asm_line = append_push_segment_instructions(asm_line, "a_rG", *p_value);
  } else if (strcmp(*p_segment, "this") == 0) {
    // push RAM[*segment_pointer + i]
    asm_line = append_push_segment_instructions(asm_line, "t_hIS", *p_value);
  } else if (strcmp(*p_segment, "that") == 0) {
    // push RAM[*segment_pointer + i]
    asm_line = append_push_segment_instructions(asm_line, "t_hAT", *p_value);
  } else if (strcmp(*p_segment, "constant") == 0) {
    // push i
    asm_line = append_str(asm_line, "@");
    asm_line = append_int(asm_line, *p_value);
    asm_line = append_str(asm_line, "\n");
    asm_line = append_str(asm_line, "d=a\n");
    asm_line = append_str(asm_line, push_instructions);
  } else if (strcmp(*p_segment, "static") == 0) {
    // push variable foo.i
    asm_line = append_str(asm_line, "@");
    asm_line = append_str(asm_line, get_root_filename());
    asm_line = append_str(asm_line, ".");
    asm_line = append_int(asm_line, *p_value);
    asm_line = append_str(asm_line, "\n");
    asm_line = append_str(asm_line, "d=m\n");
    asm_line = append_str(asm_line, push_instructions);
  } else if (strcmp(*p_segment, "temp") == 0) {
    // push RAM[*(5+i)]
    asm_line = append_str(asm_line, "@r");
    asm_line = append_int(asm_line, TEMP_START + *p_value);
    asm_line = append_str(asm_line, "\n");
    asm_line = append_str(asm_line, "d=m\n");
    asm_line = append_str(asm_line, push_instructions);
  } else if (strcmp(*p_segment, "pointer") == 0) {
    // 0 => push this
    // 1 => push that
    asm_line = append_str(asm_line, "@");
    if (*p_value == 0) {
      asm_line = append_str(asm_line, "t_hIS");
    } else if (*p_value == 1) {
      asm_line = append_str(asm_line, "t_hAT");
    }
    asm_line = append_str(asm_line, ""
        "\n"
        "d=m\n"
    );
    asm_line = append_str(asm_line, push_instructions);
  }
  return asm_line;
}

static char *append_push_segment_instructions(
    char *asm_line,
    char *segment,
    int value
) {
  asm_line = append_str(asm_line, "@");
  asm_line = append_str(asm_line, segment);
  asm_line = append_str(asm_line, ""
      "\n"
      "d=m\n"
      "@"
  );
  asm_line = append_int(asm_line, value);
  asm_line = append_str(asm_line, ""
      "\n"
      "a=d+a\n"
      "d=m\n"
  );
  asm_line = append_str(asm_line, push_instructions);
  return asm_line;
}

static char *get_pop_instructions(char *vm_line, char *asm_line) {
  char **p_segment = malloc(sizeof(char **));
  if (p_segment == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  int *p_value = malloc(sizeof(int));
  if (p_value == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  tokenize(vm_line, p_segment, p_value);
  if (strcmp(*p_segment, "local") == 0) {
    // pop RAM[*segment_pointer + i]
    asm_line = append_pop_segment_instructions(asm_line, "l_cL", *p_value);
  } else if (strcmp(*p_segment, "argument") == 0) {
    // pop RAM[*segment_pointer + i]
    asm_line = append_pop_segment_instructions(asm_line, "a_rG", *p_value);
  } else if (strcmp(*p_segment, "this") == 0) {
    // pop RAM[*segment_pointer + i]
    asm_line = append_pop_segment_instructions(asm_line, "t_hIS", *p_value);
  } else if (strcmp(*p_segment, "that") == 0) {
    // pop RAM[*segment_pointer + i]
    asm_line = append_pop_segment_instructions(asm_line, "t_hAT", *p_value);
  } else if (strcmp(*p_segment, "static") == 0) {
    // pop variable foo.i
    asm_line = append_str(asm_line, ""
        "@s_p\n"
        "a_m=m-1\n"
        "d=m\n"
        "@"
    );
    asm_line = append_str(asm_line, get_root_filename());
    asm_line = append_str(asm_line, ".");
    asm_line = append_int(asm_line, *p_value);
    asm_line = append_str(asm_line, ""
        "\n"
        "m=d\n"
    );
  } else if (strcmp(*p_segment, "temp") == 0) {
    // pop RAM[*(5+i)]
    asm_line = append_str(asm_line, ""
        "@s_p\n"
        "a_m=m-1\n"
        "d=m\n"
        "@r"
    );
    asm_line = append_int(asm_line, TEMP_START + *p_value);
    asm_line = append_str(asm_line, ""
        "\n"
        "m=d\n"
    );
  } else if (strcmp(*p_segment, "pointer") == 0) {
    // 0 => pop this
    // 1 => pop that
    asm_line = append_str(asm_line, ""
        "@s_p\n"
        "a_m=m-1\n"
        "d=m\n"
        "@"
    );
    if (*p_value == 0) {
      asm_line = append_str(asm_line, "t_hIS");
    } else if (*p_value == 1) {
      asm_line = append_str(asm_line, "t_hAT");
    }
    asm_line = append_str(asm_line, ""
        "\n"
        "m=d\n"
    );
  }
  return asm_line;
}

static char *append_pop_segment_instructions(
    char *asm_line,
    char *segment,
    int value
) {
  asm_line = append_str(asm_line, "@");
  asm_line = append_str(asm_line, segment);
  asm_line = append_str(asm_line, ""
      "\n"
      "d=m\n"
      "@"
  );
  asm_line = append_int(asm_line, value);
  asm_line = append_str(asm_line, ""
      "\n"
      "d=a+d\n"
      "@r13\n"
      "m=d\n"
  );
  asm_line = append_str(asm_line, ""
      "@s_p\n"
      "a_m=m-1\n"
      "d=m\n"
      "@r13\n"
      "a=m\n"
      "m=d\n"
  );
  return asm_line;
}

static char *get_add_instructions(char *asm_line) {
  return get_binary_op_instructions(asm_line, "+");
}

static char *get_sub_instructions(char *asm_line) {
  return get_binary_op_instructions(asm_line, "-");
}

static char *get_neg_instructions(char *asm_line) {
  return append_str(asm_line, ""
      "@s_p\n"
      "a=m-1\n"
      "m=-m\n"
  );
}

static char *get_eq_instructions(char *asm_line) {
  return get_comp_instructions(asm_line, "e_q", eq_count++);
}

static char *get_lt_instructions(char *asm_line) {
  return get_comp_instructions(asm_line, "l_t", lt_count++);
}

static char *get_gt_instructions(char *asm_line) {
  return get_comp_instructions(asm_line, "g_t", gt_count++);
}

static char *get_and_instructions(char *asm_line) {
  return get_binary_op_instructions(asm_line, "&");
}

static char *get_or_instructions(char *asm_line) {
  return get_binary_op_instructions(asm_line, "|");
}

static char *get_not_instructions(char *asm_line) {
  return append_str(asm_line,
      "@s_p\n"
      "a=m-1\n"
      "m=!m\n"
  );
}

static char *get_binary_op_instructions(char *asm_line, char *binary_op) {
  asm_line = append_str(asm_line, ""
      "@s_p\n"
      "a_m=m-1\n"
      "d=m\n"
      "a=a-1\n"
      "m=m"
  );
  asm_line = append_str(asm_line, binary_op);
  asm_line = append_str(asm_line, "d\n");
  return asm_line;
}

static char *get_comp_instructions(char *asm_line, char *comp, int count) {
  char *label = malloc(MAX_STR_LEN * sizeof(char));
  if (label == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  char *count_str = malloc(MAX_STR_LEN * sizeof(char));
  if (count_str == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  sprintf(count_str, "%d", count);
  char *label_edit = label;
  label_edit = append_str(label_edit, comp);
  label_edit = append_str(label_edit, "$");
  label_edit = append_str(label_edit, count_str);

  asm_line = append_str(asm_line, ""
      "@s_p\n"
      "a_m=m-1\n"
      "d=m\n"
      "a=a-1\n"
      "d=m-d\n"
      "m=-1\n"
      "@"
  );
  asm_line = append_str(asm_line, label);
  asm_line = append_str(asm_line, ""
      "\n"
      "d;j"
  );
  asm_line = append_str(asm_line, comp);
  asm_line = append_str(asm_line, ""
      "\n"
      "@s_p\n"
      "a=m-1\n"
      "m=0\n"
  );
  asm_line = append_str(asm_line, "(");
  asm_line = append_str(asm_line, label);
  asm_line = append_str(asm_line, ")\n");
  return asm_line;
}

