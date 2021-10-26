#ifndef ACMEDISASS_H_
#define ACMEDISASS_H_

#define VERSION         "1.0"
#define MAX_FILESIZE    0xFFFF
#define DEFAULT_INDENT  20

typedef struct
{
    char name[128];
    int data[MAX_FILESIZE];
    int length;
} virtual_file;

typedef struct
{
    char name[3];
    int bytes;
    int cycles;
    int addressing_mode;
} opcode;

typedef struct
{
    int pc_start;
    int pc_end;     // make sure to set this to pc *after* the last instruction
    int type;       // DATATYPE_DATA or DATATYPE_CODE
} datablock;

void create_datamap();
void create_labelmap();
void fill_datablocks();
int get_pc(char *filename, int skipbytes);
int is_in_array(int needle, int haystack[], int haystack_len);
int is_in_mode(int opcode);
void print_bits(unsigned int x);
void print_disassembly();
void print_help();
void print_indent();
void print_info();
void print_instruction(opcode opcode, int operand, int pc);
void print_mode();
char *newstr(char *initial_str);
virtual_file read_file(char *filename, int skipbytes);

#endif // ACMEDISASS_H_
