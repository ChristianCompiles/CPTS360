#include <stdlib.h>    // for exit()
#include <stdio.h>     // for the usual printf(), etc.
#include <getopt.h>    // for getopt()
/*
 * ASSIGNMENT
 *
 * - "#include" any other necessary headers (as indicated by "man"
 *    pages)
 */

/*
 * Note the new #include
 */
#include "critical_section.h"
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "eprintf.h"


// To get `getopt_long()` to work, you need to provide a static
// (usually) array of `struct option` structures.  There are four
// members to be filled in:

// 1. `name` is a (char *) string containing the "long" option name
// (e.g. "--help" or "--format=pdf").

// 2. `has_arg` has one of these values that describe the
// corresponding option:
enum {
    NO_ARG  = 0, // the option takes no argument
    REQ_ARG = 1, // the option must have an argument
    OPT_ARG = 2  // the option takes an optional argument
};

// 3. The "flag" is an int pointer that determines how the function
// will return its value. If it is NULL, getopt_long() will return
// "val" (the fourth member) as its function return. If it is not
// NULL, getopt_long() will return 0 and set "*flag" to "val".

// 4. "val" is an int which is either a character to denote a "short"
// (e.g. "-h" or "-f pdf") option or 0, to denote an option which does
// not have a "short" form.

// The array is terminated by an entry with a NULL name (first
// element).

static struct option options[] = {
        // elements are:
        // name       has_arg   flag   val
        { "children", OPT_ARG,  NULL,  'c'},
        { "help",     NO_ARG,   NULL,  'h'},
        { "nosync",   NO_ARG,   NULL,  'n'},
        { "pgid",     NO_ARG,   NULL,  'g'},
        { "ppid",     NO_ARG,   NULL,  'p'},
        { NULL }  // end of options table
};

/*
 * These globals are set by command line options. Here, they are set
 * to their default values.
 */
int showPpids = 0;   // show parent process IDs
int showPgids = 0;   // show process group IDs
int synchronize = 1; // synchronize outputs (don't use until Part 3)


enum { IN_PARENT = -1 }; // must be negative
/*
 * In the parent, this value is IN_PARENT. In the children, it's set
 * to the order in which they were spawned, starting at 0.
 */
int siblingIndex = IN_PARENT;


// This is a global count of signals received.
int signalCount = 0;


void writeLog(char message[], const char *fromWithin)
// print identifying information about the current process to stdout
{
    /*
     * ASSIGNMENT
     *
     * - Insert your previous writeLog() code here with this
     *   modification: If the global `synchronize` flag is set, call
     *   criticalSection_enter() before the first printf() call and
     *   criticalSection_leave() after the last one.
     */
    char processName[256];
    int colonIndent;

    if (siblingIndex == IN_PARENT)
    {
        strcpy(processName, "parent");
        colonIndent = 20;
    }
    else
    {
        snprintf(processName, 256, "child %d", siblingIndex);
        colonIndent = 30;
    }

    printf("%*s: %d\n", colonIndent, "pid", getpid());

    if(showPpids)
    {
        printf("%*s: %d\n", colonIndent, "ppid", getppid());
    }
    if(showPgids)
    {
        printf("%*s: %d\n", colonIndent, "gpid", getpgrp());
    }

    printf("%*s: %d\n", colonIndent, "signalCount", signalCount);

    printf("%*s: %s\n", colonIndent, "message",  message);
    printf("%*s: %s\n", colonIndent, "fromWithin", fromWithin);
    printf("\n");
}


void inChild(int iSibling)
// do everything that's supposed to be done in the child
{
    /*
     * ASSIGNMENT
     *
     * - insert your previous inChild() code here unchanged
     */

    siblingIndex = iSibling;

    writeLog("process is pause()d", __func__);

    while(1)
    {
        pause();
    }
}


void handler(int sigNum)
// handle signal `sigNum`
{
    /*
     * ASSIGNMENT
     *
     * - insert your previous handler() code here unchanged
     */
    signalCount +=1;
    char message[256];

    snprintf(message, 256, "%d %s", sigNum, strsignal(sigNum));
    writeLog(message, __func__);
}


void initSignals(void)
// initialize all signals
{
    /*
     * ASSIGNMENT
     *
     * - insert your previous initSignals() code here unchanged
     */
    for(int i = 1; i < _NSIG; i++)
    {
        if(i == SIGTRAP || i == SIGQUIT || i == SIGKILL || i == 32 || i == 33 )
            continue;  // i == SIGSTOP

        if (signal(i, handler) == SIG_ERR)
        {
            // uncomment below to expand error message
            //char message[20];
            //snprintf(message, 20, "failed to init %d", i);
            //writeLog(message, __func__);
            writeLog(strsignal(i), __func__);
        }
    }
}


void inParent(void)
// do everything that's supposed to be done in the parent
{
    /*
     * ASSIGNMENT
     *
     * - insert your previous inParent() code here unchanged
     */
    writeLog("parent is waiting for child to die", __func__);

    int status;
    int child_pid;

    while(1)
    {
        if ((child_pid = wait(&status)) != -1)
            break;

        if(WIFEXITED(status))
        {
            char* msg = (char*)calloc(1,256);
            snprintf(msg,256, "child exited normally: %d. Status: %d", child_pid, status);
            writeLog(msg, __func__);
            free(msg);
        }
        else
        {
            char* msg = (char*)calloc(1,256);
            snprintf(msg, 256, "child exited abnormally: %d.", child_pid);
            writeLog(msg, __func__);
            free(msg);
        }

        if(WIFSIGNALED(status))
        {
            char* msg = (char*)calloc(1,256);
            snprintf(msg, 256, "child was terminated by %d (%s)", status, strsignal(WTERMSIG(status)));
            writeLog(msg, __func__);
        }
    }

    if(errno == ECHILD)
        writeLog("No children to wait for.", __func__);
    else
        writeLog("wait() failed for unknown reason.", __func__);
}


static void usage(char *progname)
// issue a usage error message
{
    eprintf("usage: %s [{args}]*\n", progname);
    eprintf("%s\n", " {args} are:");
    eprintf("%s",
            "  -c[{arg}] or --children[={arg}]  fork {arg} children (default: 1)\n"
            "  -g or --pgid                     list process group IDs\n"
            "  -n or --nosync                   turn off synchronization\n"
            "  -p or --ppid                     list parent PIDs (default: no)\n"
            "  -h or --help                     help (this message) and exit\n"
    );
    return;
}

int main(int argc, char **argv)
{
    int ch;
    int nSiblings = 0;
    static char *progname = "**UNSET**";

    /*
     * Parse the command line arguments.
     */
    progname = argv[0];
    for (;;) {
        ch = getopt_long(argc, argv, "c::ghnp", options, NULL);
        if (ch == -1)
            break;

        switch (ch) {

            case 'c':
                if (optarg)
                    nSiblings = atoi(optarg);
                else
                    nSiblings = 1;
                break;

            case 'g':
                showPgids = 1;
                break;

            case 'h':
                usage(progname);
                exit(0);

            case 'n':
                synchronize = 0;
                break;

            case 'p':
                showPpids = 1;
                break;

            default:
                printf("?? getopt returned character code 0x%02x ??\n", ch);
                exit(1);
        }
    }
    /*
     * ASSIGNMENT
     *
     * - Insert your previous main() code here unchanged, except that
     *   if the global `synchronize` flag is set, add a call to
     *   criticalSection_init() before the initializeSignals() call.
     */

    if(synchronize)
        criticalSection_init();

    initSignals();
    if(nSiblings == 0)
    {
        writeLog("parent is pause()d for a signal", __func__);
        while(1)
        {
            pause();
        }
    }
    else
    {
        for(int iSibling = 0; iSibling < nSiblings; iSibling++)
        {
            printf("nSibling is %d and iSibling is %d\n", nSiblings, iSibling);
            pid_t par_or_child = fork();

            if(par_or_child == 0) // in child
            {
                inChild(iSibling);
            }
            else
            {
                char msg[40];
                snprintf(msg, 40, "parent forked child %d: %d", iSibling, par_or_child);
                writeLog(msg, __func__);
            }
        }
        inParent();
    return 0;
}