#include <atomic>
#include <csignal>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "daemon.h"

using namespace std;
namespace fs = filesystem;

atomic<bool> interrupted{false};

string getPidFilePath() {
    if (const char* xdg = getenv("XDG_RUNTIME_DIR")) {
        return string(xdg) + "/xorg_discord_presence.pid";
    }

    string runUser = "/run/user/" + to_string(getuid()); // FALLBACK: /run/user/<uid>
    if (fs::exists(runUser)) {
        return runUser + "/xorg_discord_presence.pid";
    }

    return "/tmp/xorg_discord_presence.pid";
}


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
    auto appPid = getPidFilePath();

    ifstream f(appPid);
    if (!f) {
        cerr << "[rpc] no daemon running (pid file missing)\n";
        return;
    }

    pid_t pid;
    if (!(f >> pid) || pid <= 0) {
        cerr << "[rpc] invalid pid file\n";
        fs::remove(appPid);
        return;
    }

    if (pid == getpid()) {
        cerr << "[rpc] refusing to kill self\n";
        return;
    }

    if (kill(pid, 0) != 0) {
        cerr << "[rpc] stale pid file (process not running)\n";
        fs::remove(appPid);
        return;
    }

    kill(pid, SIGTERM);

    for (int i = 0; i < 10; i++) {
        if (kill(pid, 0) != 0) {
            fs::remove(appPid);
            cout << "[rpc] daemon stopped\n";
            return;
        }
        usleep(100000);
    }

    cerr << "[rpc] daemon did not exit, killing\n";
    kill(pid, SIGKILL);
    fs::remove(appPid);
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
    auto appPid = getPidFilePath();

    ifstream f(appPid);
    pid_t pid;

    if (!(f >> pid)) {
        return -1;
    }

    if (kill(pid, 0) != 0) {
        fs::remove(appPid);
        return -1;
    }

    return pid;
}

bool isDaemonRunning(pid_t &pid) {
    pid = readPidFile();
    return pid > 0;
}
