
#include <vector>
#include <deque>
#include <string>


using namespace std;

class Instruction {
public:
    char operation;
    int intArg;
    string stringArg;
};

class Cpu {
public:
    vector<Instruction> *pProgram;
    int programCounter;
    int value;
    int timeSlice;
    int timeSliceUsed;
};

enum State {
    STATE_READY,
    STATE_RUNNING,
    STATE_BLOCKED
};

class PcbEntry {
public:
    int processId;
    int parentProcessId;
    vector<Instruction> program;
    unsigned int programCounter;
    int value;
    unsigned int priority;
    State state;
    unsigned int startTime;
    unsigned int timeUsed;
};

bool createProgram(const string &filename, vector<Instruction> &program);
void set(int value);
void add(int value);
void decrement(int value);
void block();
void end();
void fork(int value);
void replace(string &argument);
void quantum();
void unblock();
void print();
void schedule();
int runProcessManager(int fileDescriptor);

vector<PcbEntry> pcbTable;

