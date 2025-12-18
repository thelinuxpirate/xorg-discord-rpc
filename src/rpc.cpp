// TODO: replace cout via syslog()
#define _CRT_SECURE_NO_WARNINGS
#include <atomic>
#include <csignal>
#include <chrono>
#include <iostream>
#include <string>

#include <thread>
#include <fstream>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "discord.h"
#include "rpc.h"
#include "user.h"
#include "xTools.h"

using namespace std;

atomic<bool> interrupted{false};

void handleSignal(int sig) {
    if (sig == SIGTERM || sig == SIGINT)
        interrupted = true;
}

void killDaemon() {
    ifstream f(PID_FILE);
    pid_t pid;
    if (f >> pid) {
        kill(pid, SIGTERM);
    }
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

int runDiscordPresence(const long &APP_ID, bool daemon) {
    // rerun 'signal' twice because they reset after pid forks sometimes
    signal(SIGTERM, handleSignal);
    signal(SIGINT, handleSignal);

    if (daemon) {
        pid_t daemonPid = daemonize();
        ofstream(PID_FILE) << daemonPid;
    } else {
        killDaemon();
        return 0;
    }

    signal(SIGTERM, handleSignal);
    signal(SIGINT, handleSignal);

    discord::Core* core{};
    auto result = discord::Core::Create(APP_ID, DiscordCreateFlags_Default, &core);
    if (!core) {
        cerr << "Failed to instantiate discord core!\n";
        return -1;
    }
    unique_ptr<discord::Core> stateCore(core);

    // setup activity once
    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);
    activity.GetAssets().SetLargeImage(getWindowManagerName().c_str());
    activity.GetAssets().SetLargeText(getWindowManagerName().c_str());
    activity.GetTimestamps().SetStart(time(nullptr));

    string lastTitle;
    string lastName;

    auto lastCheck = chrono::steady_clock::now();

    // runs Discord callbacks every 16ms, checks window every 500ms
    while (!interrupted) {
        stateCore->RunCallbacks();

        auto now = chrono::steady_clock::now();
        if (now - lastCheck >= chrono::milliseconds(500)) {
            lastCheck = now;

            string title = getWindowTitle();
            string name  = getWindowName();

            if (title != lastTitle || name != lastName) {
                lastTitle = title;
                lastName  = name;

                // TODO: remove
                if (title == "unknown" || name == "unknown") {
                    title = "empty workspace";
                    name = "empty workspace";
                } else if (title == "org.wezfurlong.wezterm") {
                    title = "wezterm";
                    name = "wezterm";
                }

                string asset = sanitize_asset(title);

                activity.SetDetails(title.c_str());
                activity.GetAssets().SetSmallImage(asset.c_str());
                activity.GetAssets().SetSmallText(name.c_str());

                /*cout << endl
                          << "Current Window = "
                          << to_lower(title).c_str()
                          << endl;

                cout <<  "Current WinName = "
                          << to_lower(name).c_str()
                          << endl
                          << endl;*/

                stateCore->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
                    cout << ((result == discord::Result::Ok) ? "Updated Status!\n" : "Status Failed!\n");
                });
            }
        }

        this_thread::sleep_for(chrono::milliseconds(16));
    }

    unlink(PID_FILE);
    return 0;
}
