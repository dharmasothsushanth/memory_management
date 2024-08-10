# memory_management
-> My code works correctly on windows machine but fails in Linux (Ubuntu in my case) due to the differance in both the OS's in interpreting the endline, string end operators.

-> ERROR CASES
    1. If the given file doesn't exits then my program prints " <filename> could not be loaded - file does not exist".
    2. If the given pid doesn't exits in current physical and virtual memory in run command then my program prints "Process doesn't exist".
    3. If the virual memory get's full and we want to swapout a process then my program prints "swapout notdone: no space in virtual memory".
    4. If the LRU method fails while doing run or swapin then my program prints "lru failed".
    5. If we accessing anything out of bounds of the virtual address of a process then my program prints "Invalid Memory Address <addr> specified for process id <pid>".
    6. If we are accessing any physical address which out of range of our physical memory then my program prints "physical memory out of bounds".
    7. If the given command doesn't have 11 arguments including the exe then program prints "wrong command".
    8. It the process to be killed is not in main memory and virtual memory the printing "no process to kill".
    9. If a process is not in any memory and asking pte command the prints "process not there in memory so no pte"

-> Assumptions
    1. When swapout is called if virtual memory is not sufficient then I am just giving an error as there was nothing given if we can remove process from virtual memory.