#include <stdio.h>
#include "globals.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#define  MemoryChipSize 1024
#define  MAX_BREAKPOINTS 64

typedef  DataBusType Memory_Chip[MemoryChipSize] ;

extern void BurnInProm () ;
extern void InitializeMemory () ;
extern void InitializePCandStackPointer () ;
extern void ActivateCpu () ;
extern void ActivateMemory () ;
extern void DumpRegisters () ;
extern void GeneratePulse () ;
extern int  Cycle () ;

struct Clock
      {
         int Cycle ;
         int Subcycle ;         /* 0..4 */
      } ;

extern struct Clock Quartz;
extern DataBusType    ProgramCounter ;
extern DataBusType    Accumulator ;
extern DataBusType    StackPointer ;
extern DataBusType    InstructionReg ;
extern DataBusType    TempInstruction ;
extern DataBusType    ARegister ;
extern DataBusType    BRegister ;
extern DataBusType    CRegister ;
extern DataBusType    DRegister ;
extern DataBusType    ERegister ;
extern DataBusType    FRegister ;
extern int            MicroPc;
extern Bit            NBit, ZBit ;

extern Memory_Chip MemoryChip3 ;

int btoi();
char btoc();
int True_ascii_to_mem_ascii();

int  power2[16] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};

int  polled_io = 0;
char input_characters[100][100];
char *inbuf;
int  input_x=0, input_y=0, input_buf=0;

int  original_stdin_channel_flags;
int  nonblock_stdin_channel_flags;

static int breakpoints[MAX_BREAKPOINTS];
static int num_breakpoints = 0;

static int is_breakpoint(int pc)
{
    int i;
    for (i = 0; i < num_breakpoints; i++)
        if (breakpoints[i] == pc) return 1;
    return 0;
}

void Set_blocking_io(){
    if(fcntl(0, F_SETFL, original_stdin_channel_flags) == -1){
        perror("reset fcntl error: ");
        exit(2);
    }
}

void Set_nonblocking_io(){
    if(fcntl(0, F_SETFL, nonblock_stdin_channel_flags) == -1){
        perror("set blocking fcntl error: ");
        exit(2);
    }
}

/* Execute one subcycle. Returns 1 if halted, 0 otherwise. */
static int do_step(AddressBusType Address, DataBusType Data, Bit *ReadBit, Bit *WriteBit)
{
    if (polled_io) {
        Set_nonblocking_io();
        if ((inbuf = fgets(&input_characters[input_buf][0], 99, stdin))) {
            input_buf = (input_buf + 1) % 100;
            polled_io = 0;
            MemoryChip3[1021][14] = '1';
            MemoryChip3[1021][15] = '0';
            Set_blocking_io();
        }
    }

    GeneratePulse();
    ActivateCpu(Address, Data, ReadBit, WriteBit);
    ActivateMemory(Address, Data, *ReadBit, *WriteBit);

    return (*ReadBit == One) && (*WriteBit == One);
}

static void cmd_regs(FILE *dbg_out)
{
    fprintf(dbg_out,
        "REGS PC=%d AC=%d SP=%d IR=%d TI=%d AR=%d BR=%d CR=%d DR=%d ER=%d FR=%d"
        " N=%c Z=%c MPC=%d CYCLE=%d\n",
        btoi(ProgramCounter), btoi(Accumulator), btoi(StackPointer),
        btoi(InstructionReg), btoi(TempInstruction),
        btoi(ARegister), btoi(BRegister), btoi(CRegister),
        btoi(DRegister), btoi(ERegister), btoi(FRegister),
        NBit, ZBit, MicroPc, Cycle());
}

static void cmd_mem(FILE *dbg_out, int addr)
{
    AddressBusType mem_addr;
    DataBusType    mem_loc;
    int i, mem_location = addr;

    if (addr < 0 || addr > 4095) {
        fprintf(dbg_out, "ERR address out of range\n");
        return;
    }

    for (i = 11; i >= 0; i--) {
        if (mem_location >= power2[i]) {
            mem_addr[11-i] = '1';
            mem_location  -= power2[i];
        } else {
            mem_addr[11-i] = '0';
        }
    }
    mem_addr[12] = '\0';
    ActivateMemory(mem_addr, mem_loc, '1', '0');
    mem_loc[16] = '\0';
    fprintf(dbg_out, "MEM %d %s %d\n", addr, mem_loc, btoi(mem_loc));
}


/**** call sequence:   mic1 promfile_name programfile_name pc sp    ****/

int main (int argc, char *argv[])
{
    DataBusType    Data;
    AddressBusType Address;
    Bit  ReadBit;
    Bit  WriteBit;
    int  i, j;
    char cmd[256];

    char   promfile[80];
    char   programfile[80];
    int    pc, sp;

    if ((original_stdin_channel_flags = fcntl(0, F_GETFL, 0)) == -1) {
        perror("fcntl failed: ");
        exit(1);
    }
    nonblock_stdin_channel_flags = original_stdin_channel_flags | O_NONBLOCK;

    switch (argc) {
        case 1: promfile[0] = '\0'; programfile[0] = '\0'; pc = -1; sp = -1; break;
        case 2: strcpy(promfile, argv[1]); programfile[0] = '\0'; pc = -1; sp = -1; break;
        case 3: strcpy(promfile, argv[1]); strcpy(programfile, argv[2]); pc = -1; sp = -1; break;
        case 4: strcpy(promfile, argv[1]); strcpy(programfile, argv[2]); pc = atoi(argv[3]); sp = -1; break;
        case 5: strcpy(promfile, argv[1]); strcpy(programfile, argv[2]); pc = atoi(argv[3]); sp = atoi(argv[4]); break;
        default: fprintf(stderr, "Too many command line arguments, aborting\n"); exit(2);
    }

    for (i = 0; i < 100; i++)
        for (j = 0; j < 80; j++) input_characters[i][j] = '\0';

    BurnInProm(promfile);
    InitializeMemory(programfile);
    InitializePCandStackPointer(pc, sp);
    strcpy(Address, "000000000000");
    strcpy(Data,    "0000000000000000");
    ReadBit  = Zero;
    WriteBit = Zero;

    FILE *dbg_in  = fdopen(3, "r");
    FILE *dbg_out = fdopen(4, "w");
    if (!dbg_in || !dbg_out) {
        fprintf(stderr, "Failed to open debugger fds 3 and 4\n");
        exit(1);
    }

    fprintf(dbg_out, "READY\n");
    fflush(dbg_out);

    while (fgets(cmd, sizeof(cmd), dbg_in)) {
        /* strip trailing newline/CR */
        int len = strlen(cmd);
        while (len > 0 && (cmd[len-1] == '\n' || cmd[len-1] == '\r'))
            cmd[--len] = '\0';

        if (strcmp(cmd, "step") == 0) {
            /* one subcycle */
            if (do_step(Address, Data, &ReadBit, &WriteBit))
                fprintf(dbg_out, "HALTED\n");
            else
                fprintf(dbg_out, "OK\n");

        } else if (strcmp(cmd, "run") == 0) {
            /* run until halt or breakpoint */
            for (;;) {
                if (do_step(Address, Data, &ReadBit, &WriteBit)) {
                    fprintf(dbg_out, "HALTED\n");
                    break;
                }
                if (is_breakpoint(btoi(ProgramCounter))) {
                    fprintf(dbg_out, "BREAK %d\n", btoi(ProgramCounter));
                    break;
                }
            }

        } else if (strncmp(cmd, "break ", 6) == 0) {
            if (num_breakpoints >= MAX_BREAKPOINTS) {
                fprintf(dbg_out, "ERR too many breakpoints\n");
            } else {
                breakpoints[num_breakpoints++] = atoi(cmd + 6);
                fprintf(dbg_out, "OK\n");
            }

        } else if (strncmp(cmd, "delbreak ", 9) == 0) {
            int addr = atoi(cmd + 9);
            int found = 0;
            for (i = 0; i < num_breakpoints; i++) {
                if (breakpoints[i] == addr) {
                    breakpoints[i] = breakpoints[--num_breakpoints];
                    found = 1;
                    break;
                }
            }
            fprintf(dbg_out, found ? "OK\n" : "ERR not found\n");

        } else if (strcmp(cmd, "regs") == 0) {
            cmd_regs(dbg_out);

        } else if (strncmp(cmd, "mem ", 4) == 0) {
            cmd_mem(dbg_out, atoi(cmd + 4));

        } else if (strcmp(cmd, "quit") == 0) {
            fprintf(dbg_out, "OK\n");
            fflush(dbg_out);
            break;

        } else {
            fprintf(dbg_out, "ERR unknown command: %s\n", cmd);
        }

        fflush(dbg_out);
    }

    fprintf(stderr, "MIC-1 emulator finishing\n");
    return 0;
}


/* passed an array of bytes of 16 ascii 1s and 0s  */
/* converts to a true binary integer               */
int btoi(char *mem_loc)
{
    int i, result = 0;
    for (i = 0; i < 16; i++)
        if (mem_loc[i] == '1') result += power2[15-i];
    return result;
}

/* converts low 8 bits to a char */
char btoc(char *mem_loc)
{
    int i, result = 0;
    for (i = 8; i < 16; i++)
        if (mem_loc[i] == '1') result += power2[15-i];
    return ((char)result);
}

/* fills low-order 8 bits of a memory location from a char */
int True_ascii_to_mem_ascii(char *mem_location, char *character)
{
    int i;
    for (i = 0; i < 16; i++) mem_location[i] = '0';
    mem_location[16] = '\0';
    for (i = 8; i < 16; i++)
        if (power2[15-i] & (*character)) mem_location[i] = '1';
    return 1;
}
