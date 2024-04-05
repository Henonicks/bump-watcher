# C++ Discord bot made with [D++](https://dpp.dev), Bump Watcher 

This bot watches when someone bumps a guild (Disboard bump) and gives them a dedicated role. Then, removes it in 2 hours.

## Compilation

```bash
cmake -B build
cmake --build build -j 4
```

If DPP is installed in a different location you can specify the root directory to look in while running cmake 

```bash
cmake -B build -DDPP_ROOT_DIR=<your-path>
```

## Running the bot

Create a config.json in the directory above the build directory:

```json
{
    "BOT_TOKEN" : "your bot token here",
    "BUMPER_ROLE_ID" : "ID of the role to be given" 
}
```

Start the bot:
```bash
cd build
./templatebot
```