#include <CLI/CLI.hpp>

#include "rpc.h"

/*
 * TODO:
 * turn into cli command + add function to disable RPC / restart
 * trim spaces function?
 * set default to gnu
 * Make a README that explains:
 * Build Setup
 * Application runs on assets
 *
 * IDEA: read toml file with app name += convert string to asset name for png
 * WinTitle = 'neovim' or 'emacs' = convert WinName to = 'editor'
 *
 * if empty-workspace = asset gnu^
*/

int main(int argc, char** argv) {
    CLI::App app{"The Xorg Discord RPC CLI Tool"};

    long id{0};

    app.add_option("-r,--appid", id, "Runs instance of RPC using your APPID")->required();

    CLI11_PARSE(app, argc, argv);

    if (id > 0) {
        runDiscordPresence(id);
    }

    return 0;
}
