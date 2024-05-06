
#include <cctype>     // for toupper()
#include <cstdlib>    // for EXIT_SUCCESS and EXIT_FAILURE
#include <cstring>    // for strerror()
#include <cerrno>     // for errno
#include <deque>      // for deque (used for ready and blocked queues)
#include <fstream>    // for ifstream (used for reading simulated programs)
#include <iostream>   // for cout, endl, and cin
#include <sstream>    // for stringstream (used for parsing simulated programs)
#include <sys/wait.h> // for wait()
#include <unistd.h>   // for pipe(), read(), write(), close(), fork(), and _exit()
#include <vector>     // for vector (used for PCB table)

using namespace std;
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
enum State
{
    STATE_READY,
    STATE_RUNNING,
    STATE_BLOCKED
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
PcbEntry pcbEntry[10];
unsigned int timestamp = 0;
Cpu cpu;
// For the states below, -1 indicates empty (since it is an invalid index).
int runningState = -1;
deque<int> readyState;
deque<int> blockedState;
// In this implementation, we'll never explicitly clear PCB entries and the index in
// the table will always be the process ID. These choices waste memory, but since this
// program is just a simulation it the easiest approach. Additionally, debugging is
// simpler since table slots and process IDs are never re-used.
double cumulativeTimeDiff = 0;
int numTerminatedProcesses = 0;
bool createProgram(const string &filename, vector<Instruction> &program)
{
    ifstream file;
    int lineNum = 0;
    file.open(filename.c_str());
    if (!file.is_open())
    {
        cout << "Error opening file " << filename << endl;
        return false;
    }
    while (file.good()) 
    {
        string line;
        getline(file, line);
        
        trim(line);
        if (line.size() > 0)
        {
            Instruction instruction;
            instruction.operation = toupper(line[0]);
            instruction.stringArg = trim(line.erase(0, 1));
            stringstream argStream(instruction.stringArg);
            switch (instruction.operation)
            {
            case 'S': // Integer argument.
            case 'A': // Integer argument.
            case 'D': // Integer argument.
            case 'F': // Integer argument.
                if (!(argStream >> instruction.intArg))
                {
                    cout << filename << ":" << lineNum
                         << " - Invalid integer argument "
                         << instruction.stringArg << " for "
                         << instruction.operation << " operation"
                         << endl;
                    file.close();
                    return false;
                }
                break;
            case 'B': // No argument.
            case 'E': // No argument.
                break;
            case 'R': // String argument.
                      // Note that since the string is trimmed on both ends, filenames
                      // with leading or trailing whitespace (unlikely) will not work.
                if (instruction.stringArg.size() == 0)
                {
                    cout << filename << ":" << lineNum << " - Missing string argument" << endl;
                    file.close();
                    return false;
                }
                break;
            default:
                cout << filename << ":" << lineNum << " - Invalid operation, "
                     << instruction.operation << endl;
                file.close();
                return false;
            }
            program.push_back(instruction);
        }
        lineNum++;
    }
    file.close();
    return true;
}
// Implements the S operation.
void set(int value)
{

    // TODO: Implement
    // 1. Set the CPU value to the passed-in value.
    cpu.value = value;


}
// Implements the A operation.
void add(int value)
{
    // TODO: Implement
    // 1. Add the passed-in value to the CPU value.
    cpu.value += value;

}
// Implements the D operation.
void decrement(int value)
{
    // TODO: Implement
    // 1. Subtract the integer value from the CPU value.
    cpu.value -= value;
}
// Performs scheduling.
void schedule()
{
    // TODO: Implement
    // 1. Return if there is still a processing running (runningState != -1). 
    //There is no need to schedule if a process is already running (at least until iLab 3)
    if (runningState != -1)
    {
        return;
    }
    // 2. Get a new process to run, if possible, from the ready queue.
    if (!readyState.empty())
    {
        int pcbIndex = readyState.front();
        readyState.pop_front();
    // 3. If we were able to get a new process to run:
    // a. Mark the processing as running (update the new process's PCB state)
 
        runningState = pcbIndex;
        pcbEntry[pcbIndex].state = STATE_RUNNING;
    // b. Update the CPU structure with the PCB entry details(program, program counter, // value, etc.)

        cpu.pProgram = &(pcbEntry[pcbIndex].program); // this
        cpu.programCounter = pcbEntry[pcbIndex].programCounter;
        cpu.value = pcbEntry[pcbIndex].value;
        
    }
}
// Implements the B operation.
void block()
{
    // TODO: Implement
    
    // 1. Add the PCB index of the running process (stored in runningState) to the blocked queue.
    blockedState.push_back(runningState);

    // 2. Update the process's PCB entry
    int pcbIndex = runningState;    
    
    // a. Change the PCB's state to blocked.
    pcbEntry[pcbIndex].state = STATE_BLOCKED;
    // b. Store the CPU program counter in the PCB's program counter.
    pcbEntry[pcbIndex].programCounter = cpu.programCounter;
    // c. Store the CPU's value in the PCB's value.
    pcbEntry[pcbIndex].value = cpu.value;
    // 3. Update the running state to -1 (basically mark no process as running). Note that a new process will be chosen to run later
    runningState = -1;
    // (via theQ command code calling the schedule() function).
}
// Implements the E operation.
void end()
{
    // TODO: Implement
    // 1. Get the PCB entry of the running process.
    int pcbIndex = runningState;
    PcbEntry& process = pcbEntry[runningState]; 
    // 2. Update the cumulative time difference (increment it by
    // timestamp + 1 - start time of the process).
    cumulativeTimeDiff += timestamp + 1- process.startTime;
    // 3. Increment the number of terminated processes.
    numTerminatedProcesses++;
    // 4. Update the running state to -1 (basically mark no process as running).
    runningState = -1;

    // Note that a new process will be chosen to run later (via the Q command code calling the schedule function).
}
// Implements the F operation.
void fork(int value)
{
    // TODO: Implement
    // 1. Get a free PCB index (pcbTable.size())
    vector<PcbEntry> pcbTable;
    int freePcbIndex = pcbTable.size();
    
 
    // 2. Get the PCB entry for the current running process.
    int pcbIndex = runningState;
    PcbEntry& parentProcess = pcbEntry[freePcbIndex];
    // 3. Ensure the passed-in value is not out of bounds.
    if (value < 0 || value >= pcbTable.size())
    {
        cout << "Invalid value for fork operation" << endl;
        return;
    }
    // 4. Populate the PCB entry obtained in #1
    

    // a. Set the process ID to the PCB index obtained in #1.
    parentProcess.processId = freePcbIndex;
    // b. Set the parent process ID to the process ID of the
    parentProcess.parentProcessId = pcbIndex;
    // running process (use the running process's PCB entry to get this).
    // c. Set the program counter to the cpu program counter.
    parentProcess.programCounter = cpu.programCounter;
    // d. Set the value to the cpu value.
    parentProcess.value = cpu.value;
    // e. Set the priority to the same as the parent process's priority.
    parentProcess.priority = pcbEntry[pcbIndex].priority;
    // f. Set the state to the ready state.
    parentProcess.state = STATE_READY;
    // g. Set the start time to the current timestamp
    parentProcess.startTime = timestamp;
    // 5. Add the pcb index to the ready queue.
    readyState.push_back(freePcbIndex);
    // 6. Increment the cpu's program counter by the value read in #3
    cpu.programCounter += value;
}
// Implements the R operation.
void replace(string &argument)
{
    // TODO: Implement
    // 1. Clear the CPU's program (cpu.pProgram->clear()).
    cpu.pProgram->clear();
    // 2. Use createProgram() to read in the filename specified by
    // argument into the CPU (*cpu.pProgram)
    // a. Consider what to do if createProgram fails. I printed an
    // error, incremented the cpu program counter and then returned. Note
    // that createProgram can fail if the file could not be opened or did not exist.
    if (!createProgram(argument, *cpu.pProgram))
    {
        cout << "Error reading program file " << argument << endl;
        cpu.programCounter++;
        return;
    }
    // 3. Set the program counter to 0.
    cpu.programCounter = 0;

}
// Implements the Q command.
void quantum()
{
    Instruction instruction;
    cout << "In quantum";
    if (runningState == -1)
    {
        cout << "No processes are running" << endl;
        ++timestamp;
        return;
    }
    if (cpu.programCounter < cpu.pProgram->size())
    {
        instruction = (*cpu.pProgram)[cpu.programCounter];
        ++cpu.programCounter;
    }
    else
    {
        cout << "End of program reached without E operation" << endl;
        instruction.operation = 'E';
    }
    switch (instruction.operation)
    {
    case 'S':
        set(instruction.intArg);
        cout << "instruction S " << instruction.intArg << endl;
        break;
    case 'A':
        add(instruction.intArg);
        cout << "instruction A " << instruction.intArg << endl;
        break;
    case 'D':
        decrement(instruction.intArg);
        break;
    case 'B':
        block();
        break;
    case 'E':
        end();
        break;
    case 'F':
        fork(instruction.intArg);
        break;
    case 'R':
        replace(instruction.stringArg);
        break;
    }
    ++timestamp;
    schedule();
}
// Implements the U command.
void unblock()
{
    // 1. If the blocked queue contains any processes:
    // a. Remove a process form the front of the blocked queue.
    if (!blockedState.empty())
    {
        int pcbIndex = blockedState.front();
        blockedState.pop_front();
    }
    // b. Add the process to the ready queue.
    readyState.push_back(pcbIndex);
    // c. Change the state of the process to ready (update its PCB entry).
    pcbEntry[pcbIndex].state = STATE_READY;
    // 2. Call the schedule() function to give an unblocked process a
    // chance to run (if possible).
    schedule();}
// Implements the P command.
void print()
{
    cout << "Print command is not implemented until iLab 3" << endl;
    cout << "**************************************************************" << endl;
   cout << "The current system state is as follows:" << endl;
   cout << "**************************************************************" << endl;
   cout << "Time: " << timestamp << endl;
   cout << "Running Process: " << runningState << endl;
   cout << "Ready Queue: ";
   for (int i : readyState)
       cout << i << " ";
       cout << endl;
       cout << "Blocked Queue: ";
       for (int i : blockedState)
           cout << i << " ";
           cout << endl;
           cout << "**************************************************************" << endl;
           cout << "PCB Table:" << endl;
           cout << "**************************************************************" << endl;
           cout << "PID\tPPID\tState\tPriority\tStart Time\tTime Used" << endl;
           for (int i = 0; i < pcbEntry.size(); i++)
           {
               cout << pcbEntry[i].processId << "\t" << pcbEntry[i].parentProcessId << "\t";
               switch (pcbEntry[i].state)
               {
               case STATE_READY:
                   cout << "Ready\t";
                   break;
               case STATE_RUNNING:
                   cout << "Running\t";
                   break;
               case STATE_BLOCKED:
                   cout << "Blocked\t";
                   break;
               }
               cout << pcbEntry[i].priority << "\t" << pcbEntry[i].startTime << "\t" << pcbEntry[i].timeUsed << endl;
           }
           cout << "**************************************************************" << endl;
           cout << "CPU:" << endl;
           cout << "**************************************************************" << endl;
           cout << "Program Counter: " << cpu.programCounter << endl;
           cout << "Value: " << cpu.value << endl;
           cout << "Time Slice: " << cpu.timeSlice << endl;
           cout << "Time Slice Used: " << cpu.timeSliceUsed << endl;
           cout << "**************************************************************" << endl;
// Function that implements the process manager.
int runProcessManager(int fileDescriptor)
{
    // vector<PcbEntry> pcbTable;
    //  Attempt to create the init process.
    if (!createProgram("init", pcbEntry[0].program))
    {
        return EXIT_FAILURE;
    }
    pcbEntry[0].processId = 0;
    pcbEntry[0].parentProcessId = -1;
    pcbEntry[0].programCounter = 0;
    pcbEntry[0].value = 0;
    pcbEntry[0].priority = 0;
    pcbEntry[0].state = STATE_RUNNING;
    pcbEntry[0].startTime = 0;
    pcbEntry[0].timeUsed = 0;
    runningState = 0;
    cpu.pProgram = &(pcbEntry[0].program);
    cpu.programCounter = pcbEntry[0].programCounter;
    cpu.value = pcbEntry[0].value;
    timestamp = 0;
    double avgTurnaroundTime = 0;
    // Loop until a 'T' is read, then terminate.

    char ch;
    do
    {
        // Read a command character from the pipe.
        if (read(fileDescriptor, &ch, sizeof(ch)) != sizeof(ch))
        {
            // Assume the parent process exited, breaking the pipe.
            break;
        }
        // TODO: Write a switch statement
        switch (ch)
        {
        case 'Q':
            quantum();
            break;
        case 'U':
            cout << "You entered U" << endl;
            break;
        case 'P':
            cout << "You entered P" << endl;
            break;
        default:
            cout << "You entered an invalid character!" << endl;
        }
    } while (ch != 'T');
    return EXIT_SUCCESS;
}
int main(int argc, char *argv[])
{
    int pipeDescriptors[2];
    pid_t processMgrPid;
    char ch;
    int result;
    // TODO: Create a pipe
    pipe(pipeDescriptors);
    // USE fork() SYSTEM CALL to create the child process and save the value returned in processMgrPid variable
    if ((processMgrPid = fork()) == -1)
        exit(1); /* FORK FAILED */
    if (processMgrPid == 0)
    {
        // The process manager process is running.
        // Close the unused write end of the pipe for the process manager process.
        close(pipeDescriptors[1]);
        // Run the process manager.
        result = runProcessManager(pipeDescriptors[0]);
        // Close the read end of the pipe for the process manager process (for
        // cleanup purposes).
        close(pipeDescriptors[0]);
        _exit(result);
    }
    else
    {
        // The commander process is running.
        // Close the unused read end of the pipe for the commander process.
        close(pipeDescriptors[0]);
        // Loop until a 'T' is written or until the pipe is broken.
        do
        {
            cout << "Enter Q, P, U or T" << endl;
            cout << "$ ";
            cin >> ch;
            // Pass commands to the process manager process via the pipe.
            if (write(pipeDescriptors[1], &ch, sizeof(ch)) != sizeof(ch))
            {
                // Assume the child process exited, breaking the pipe.
                break;
            }
        } while (ch != 'T');
        write(pipeDescriptors[1], &ch, sizeof(ch));
        // Close the write end of the pipe for the commander process (for
        // cleanup purposes).
        close(pipeDescriptors[1]);
        // Wait for the process manager to exit.
        wait(&result);
    }
    return result;
}
