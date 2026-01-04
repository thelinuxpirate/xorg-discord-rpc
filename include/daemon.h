#ifndef DAEMON_H
#define DAEMON_H

#include <atomic>
#include <sys/types.h>
#include <unistd.h>
#include <string>

extern std::atomic<bool> interrupted;
// TODO: move to $XDG_RUNTIME_DIR
constexpr const char* PID_FILE = "/tmp/xorg_discord_presence.pid";

void handleSignal(int sig);
void installSignals();
pid_t daemonize();
pid_t readPidFile();
void killDaemon();
bool isDaemonRunning(pid_t &pid);
void restartDaemon(char** argv);

#endif
