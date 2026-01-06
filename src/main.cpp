#include <CLI/CLI.hpp>
#include <string>
#include <iostream>

#include "user.h"
#include "rpc.h"
#include "daemon.h"

#include <X11/Xlib.h>

/*
 * TODO:
 * rename user.h to toml.h
 * add function to restart RPC | TODO fix
 *
 * Make a README that explains:
 * Build Setup
 * Application runs on assets
 * why program works via PID file instead of systemD (not every distro runs same init)
*/

using namespace std;

int main(int argc, char** argv) {
    setUp();
    XInitThreads();

    CLI::App app{"The Xorg Discord RPC CLI Tool"};

    int64_t id{0};
    int getPid{0};
    int restart{0};
    int kill{0};
    string path;

    app.add_option("-i,--appid", id, "Runs instance of RPC <APPID>"); // TODO: add ->check(idChecker()); | checks for a valid DISCORD_APPID
    app.add_option("-l,--load", path, "Loads a specfic config <TOML_PATH>");
    app.add_option("-p,--pid", getPid, "Prints PID instance if it exists <1>")->expected(1); // flag?
    app.add_option("-r,--restart", restart, "Restarts ongoing instance <1>")->expected(1);
    app.add_option("-k,--kill", kill, "Shuts off ongoing instance <1>")->expected(1);

    CLI11_PARSE(app, argc, argv);

    PresenceConfig cfg;

    if (argc == 1) {
        cout << app.help() << endl;
        return 0;
    }

    if (!path.empty()) {
        cfg = loadConfig(path);
    } else {
        cfg = loadConfig(getUserConfig().configFile);
    }

    if (id > 0) {
        return runDiscordPresence(id, true, cfg);
    }

    if (getPid > 0) {
        pid_t pid = readPidFile();
        if (pid > 0) {
            cout << pid << endl;
            return 0;
        }
        cerr << "[rpc] no running daemon\n";
        return 1;
    }

    if (restart > 0) { // TODO: fix
        restartDaemon(argv);
        return 0;
    }

    if (kill > 0) {
        killDaemon();
        return 0;
    }

    return 0;
}
