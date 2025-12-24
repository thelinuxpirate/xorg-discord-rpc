#include <CLI/CLI.hpp>
#include <string>
#include <iostream>

#include "user.h"
#include "rpc.h"

/*
 * TODO:
 * add function to restart RPC
 * Make a README that explains:
 * Build Setup
 * Application runs on assets
 * why daemon works via PID file instead of systemD
*/

using namespace std;

int main(int argc, char** argv) {
    setUp();

    CLI::App app{"The Xorg Discord RPC CLI Tool"};

    long id{0};
    int kill{0};
    string path;

    app.add_option("-r,--appid", id, "Runs instance of RPC <APPID>"); // TODO: add ->check(idChecker());
    app.add_option("-l,--load", path, "Loads a specfic config <TOML_PATH>");
    app.add_option("-k,--kill", kill, "Shuts off ongoing instance <1>")->expected(1);

    CLI11_PARSE(app, argc, argv);

    if (argc == 1) {
        cout << app.help() << endl;
    }

    if (!path.empty()) { // TODO: double-check
        loadConfig(path);
    }

    if (id > 0) {
        return runDiscordPresence(id, true);
    }

    if (kill > 0) {
        return runDiscordPresence(0, false);
    }

    return 0;
}
