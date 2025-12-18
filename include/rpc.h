#ifndef RPC_H
#define RPC_H

#include <sys/types.h>
#include <unistd.h>

constexpr const char* PID_FILE = "/tmp/xorg_discord_presence.pid";

void handleSignal(int sig);
pid_t daemonize();
void killDaemon();

int runDiscordPresence(const long &APP_ID, bool daemon);

#endif
