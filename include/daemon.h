#ifndef DAEMON_H
#define DAEMON_H

#include <atomic>
#include <sys/types.h>
#include <unistd.h>

extern std::atomic<bool> interrupted;
constexpr const char* PID_FILE = "/tmp/xorg_discord_presence.pid";

void handleSignal(int sig);
void installSignals();
pid_t daemonize();
void killDaemon();

#endif
