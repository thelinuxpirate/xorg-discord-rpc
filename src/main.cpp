#include <CLI/CLI.hpp>
#include <string>
#include <iostream>

#include "user.h"
#include "rpc.h"
#include "daemon.h"

#include <X11/Xlib.h>

using namespace std;

static CLI::Validator idChecker() {
    return CLI::Validator(
        [](const string& s) -> string {
            if (s.empty() || !all_of(s.begin(), s.end(), ::isdigit)) {
                return "DISCORD_APPID must be numeric";
            }

            if (s.length() < 17 || s.length() > 20) {
                return "DISCORD_APPID must be 17–20 digits";
            }

            return {};
        },
        ""
    );
}

int main(int argc, char** argv) {
    if (setUp() != 0) {
        return 1;
    }

    XInitThreads();
    CLI::App app{"The Xorg Discord RPC CLI Tool"};

    int64_t id{0};
    int getPid{0};
    int kill{0};
    string path;

    app.add_option("-i,--appid", id, "Runs instance of RPC <APPID>")->check(idChecker());
    app.add_option("-l,--load", path, "Loads a specfic config <TOML_PATH>")->type_name("PATH");
    app.add_option("-p,--pid", getPid, "Prints PID instance if it exists <1>")->expected(1);
    app.add_option("-k,--kill", kill, "Shuts off ongoing instance <1>")->expected(1);

    CLI11_PARSE(app, argc, argv);
    PresenceConfig cfg;

    if (argc == 1) {
        cout << app.help();
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
        } else {
            cerr << "[rpc] no running daemon\n";
            return 1;
        }
    }

    if (kill > 0) {
        killDaemon();
        return 0;
    }

    return 0;
}
