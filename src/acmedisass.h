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

void create_datamap(virtual_file assembly, int mode, int pc_start, int pc_end);
virtual_file read_file(char *filename, int skipbytes);
int is_in_array(int needle, int haystack[], int haystack_len);
int is_in_mode(int opcode, int mode);
int get_pc(char *filename, int skipbytes);
char *newstr(char *initial_str);
void print_bits(unsigned int x);
void print_instruction(opcode opcode, int operand);
void print_mode(int mode);
void print_indent(int n);
void print_info();
void print_help();

#endif // ACMEDISASS_H_
