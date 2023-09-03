typedef struct node {
  char *symbol;
  unsigned value;
  struct node *next;
} Entry;

static unsigned hash(char *symbol);
static Entry *lookup(char *symbol);
void symbolTableInit();
void symbolTableAdd(char *symbol, unsigned value);
unsigned symbolTableGet(char *symbol);
void symbolTablePrint(void);
