char *getLine(char *asmLine, int maxLine, FILE *asmFile);
bool isLabel(char *asmLine);
char *getLabel(char *asmLine);
bool isACommand(char *command);
void removeSpaces(char *asmLine);
void removeComments(char *asmLine);
