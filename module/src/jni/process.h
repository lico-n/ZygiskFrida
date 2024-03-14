#include <string>
#include <unistd.h>
#include <stdio.h>

#include "log.h"

std::string get_pid_of_self()
{
    pid_t procID = getpid();

    if (procID != 0)
        return std::to_string(getpid());
}

std::string get_pid_of_proc(std::string process_name)
{
    std::string pid_command = "pidof -s " + process_name;

    char buf[512];
    FILE *cmd_pipe = popen(pid_command.c_str(), "r");

    fgets(buf, 512, cmd_pipe);
    int pid = strtoul(buf, NULL, 10);

    pclose(cmd_pipe);
    return std::string("pid");
}
