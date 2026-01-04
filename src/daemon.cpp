#include <atomic>
#include <csignal>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include "daemon.h"

using namespace std;

atomic<bool> interrupted{false};

void handleSignal(int sig) {
    if (sig == SIGTERM || sig == SIGINT || sig == SIGHUP) {
        interrupted.store(true);
    }
}

void installSignals() {
    struct sigaction sa{};
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT,  &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr); // handle X session close
}

void killDaemon() {
    // TODO: handle exception if PID is not found
    ifstream f(PID_FILE);
    pid_t pid;

    if (!(f >> pid)) return;

    if (pid == getpid()) {
        cerr << "[rpc] refusing to kill self\n";
        return;
    }

    kill(pid, SIGTERM);

    for (int i = 0; i < 10; i++) {
        if (kill(pid, 0) != 0) {
            unlink(PID_FILE);
            return;
        }
        usleep(100000);
    }

    kill(pid, SIGKILL);
    unlink(PID_FILE);
}


pid_t daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);

    return getpid();
}

pid_t readPidFile() {
    ifstream f(PID_FILE);
    pid_t pid;

    if (!(f >> pid)) {
        return -1;
    }

    if (kill(pid, 0) != 0) {
        unlink(PID_FILE);
        return -1;
    }

    return pid;
}

bool isDaemonRunning(pid_t &pid) {
    pid = readPidFile();
    return pid > 0;
}

void restartDaemon(char** argv) {
    pid_t pid = readPidFile();

    if (pid > 0 && pid != getpid()) {
        kill(pid, SIGTERM);

        for (int i = 0; i < 50; i++) { // 5 sec wait
            if (kill(pid, 0) != 0) {
                unlink(PID_FILE);
                break;
            }
            usleep(100000);
        }
    }

    execvp(argv[0], argv);

    perror("execvp");
    _exit(EXIT_FAILURE);
}
