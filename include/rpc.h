#ifndef RPC_H
#define RPC_H

#include <string>
#include <sys/types.h>
#include <unistd.h>

constexpr const char* PID_FILE = "/tmp/xorg_discord_presence.pid";

std::string to_lower(const std::string &str);
std::string sanitize_asset(std::string s);

void handleSignal(int sig);
pid_t daemonize();
void killDaemon();

int runDiscordPresence(const long &APP_ID, bool daemon);

#endif
