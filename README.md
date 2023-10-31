# C++ Discord bot made with [D++](https://dpp.dev), Bump Watcher 

This bot watches when someone bumps a guild (Disboard bump) and gives them a dedicated role. Then, removes it in 2 hours.

## Compilation

    mkdir build
    cd build
    cmake ..
    make -j

If DPP is installed in a different location you can specify the root directory to look in while running cmake 

    cmake .. -DDPP_ROOT_DIR=<your-path>

## Running the bot

Create a config.json in the directory above the build directory:

```json
{
    "BOT_TOKEN" : "your bot token here",
    "BUMPER_ROLE_ID" : "ID of the role to be given" 
}
```

Start the bot:

    cd build
    ./templatebot

## Extending the bot

You can add as many header files and .cpp files into the src and include folders as you wish. All .cpp files in the src directory will be linked together into the bot's executable.

## Renaming the bot

To rename the bot, search and replace "bumpwatcher" in the `CMakeLists.txt` with your new bots name and then rename the bumpwatcher folder in include. Rerun `cmake ..` from the `build` directory and rebuild. You might need to re-create the `build` directory.
