enum instruction_type {
    Run,
    Sleep,
    Allocate,
    Free,
    Read,
    Write
};

enum process_state {
    Ready,
    Waiting,
    Running,
    Terminated
};

typedef struct {
    enum instruction_type type;
    int args[2];
} Instruction;

typedef struct {
    int pid;
    int pc;
    int ic;
    Instruction *instructions;
    //enum process_state state;
} Process;

typedef struct tnode {
    Process *process;
    struct tnode *left;
    struct tnode *right; 
} Tnode;

typedef struct plist {
    int time_stamp;
    int sleep_time;
    Process *process;
    struct plist *next;
} Plist;

Process *read_process_info(int pid);
Tnode *talloc(Process *process);
Tnode *tadd(Tnode *tp, Process *process);
Plist *palloc(int time_stamp, int sleep_time, Process *process, Plist *next);