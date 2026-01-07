#ifndef DAEMON_H
#define DAEMON_H

#include <atomic>
#include <sys/types.h>
#include <unistd.h>
#include <string>

extern std::atomic<bool> interrupted;

std::string getPidFilePath();
void handleSignal(int sig);
void installSignals();
pid_t daemonize();
pid_t readPidFile();
void killDaemon();
bool isDaemonRunning(pid_t &pid);

#endif
