#include <vector>
#include <string>

using namespace std;
enum State
{
    STATE_READY,
    STATE_RUNNING,
    STATE_BLOCKED
};

class Instruction
{
public:
    char operation;
    int intArg;
    string stringArg;
};

class Cpu
{
public:
    vector<Instruction> *pProgram;
    int programCounter;
    int value;
    int timeSlice;
    int timeSliceUsed;
};

class PcbEntry
{
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

void avgTTime();
bool createProgram();

vector<PcbEntry> pcbTable;
double avgTurnaroundTime;
int programIndexCounter;
