#include "systemcalls.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/

bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/  
    bool result = false;
    if (cmd == NULL) {
        result = false;
        // If command is NULL, then a nonzero value if a shell is
        // available, or 0 if no shell is available.
    }

    int sy = system(cmd);
    if (sy == -1){
        result = false;
        // If a child process could not be created, or its status could
        //   not be retrieved, the return value is -1 and errno is set to
        //   indicate the error.
    }

    else if (sy == 127){
        result = false;
        // If a shell could not be executed in the child process, then
        //   the return value is as though the child shell terminated by
        //   calling _exit(2) with the status 127.
    }

    else if (sy == 0){
        result = true;
    }

    return result;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;

    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    
    int kid = fork();
    if (kid ==-1) {
        abort();        
    }

    else if (kid ==0) {
        if (execv(command[0], command) == 0) {
            exit(0);
        }
        else {
            abort();
        }        
    }

    else {
        int wstatus;
        waitpid(kid, &wstatus, 0);
        if(WIFEXITED(wstatus)) {
        if (WEXITSTATUS(wstatus)) {
            return false;
        }
        } 
        else {
            return false;
        }        
    }

        return true;

    va_end(args);
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0666);
    if (fd < 0) { 
        perror("open"); 
        abort(); 
    }
    int kid= fork();
		if (kid == -1) {
            abort();
        }
		else if (kid == 0)  {
            if (dup2(fd, 1) < 0) {
                abort();
            }
            close(fd);
            execvp(command[0], command); 
            abort();            
        }

		else {
            int wstatus;
            waitpid(kid, &wstatus, 0);
            if(WIFEXITED(wstatus) ) {
                if (WEXITSTATUS(wstatus)) {
                    return false;
                }
            } 
            else {
                return false;
            }            
        }

    va_end(args);
    return true;
}