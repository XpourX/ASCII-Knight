# ASCII Knight Game

Course Project #10 for Introduction to Programming (Winter 2025/2026)
Faculty of Mathematics and Informatics, Sofia University

## Description

ASCII Knight is a combat game in ASCII graphics where the player controls a hero (@) who can move left and right, jump (including double jump), and attack in four directions.

The goal is to defeat appearing enemies while avoiding their attacks and preserving the hero's health points (HP).

## Features

- **Player Movement**: Move left/right, jump, and double jump
- **Combat System**: Attack in four directions (up, left, down, right)
- **Enemy Types**:
  - E (Basic Walker) - Walks on platforms
  - J (Jumper) - Jumps when close to player
  - F (Flier) - Flies and descends periodically
  - C (Crawler) - Sticks to walls
  - B (Boss) - Takes multiple hits, occupies 3x3 space
- **Wave System**: Enemies appear in waves with increasing difficulty
- **Health System**: Player starts with 5 HP

## Controls

- `A/D` - Move left/right
- `W` - Jump / Double jump
- `I` - Attack upward
- `J` - Attack left
- `K` - Attack downward
- `L` - Attack right

## How to Build and Run

### Using the batch files:
1. Double-click `build_and_run.bat` to compile and run the game
2. Or use `build.bat` to compile, then `run.bat` to execute

### Using Command Line:
```cmd
g++ -Wall -Wextra -std=c++17 main.cpp -o ascii_knight.exe
ascii_knight.exe
```

### Using Makefile:
```cmd
make
make run
```

### Using VSCode:
- Press `Ctrl+Shift+B` to build
- Press `F5` to debug

## Project Structure

```
ascii_knight/
├── main.cpp              # Main game file
├── Makefile              # Build configuration
├── README.md             # This file
├── build.bat             # Windows build script
├── run.bat               # Windows run script
├── build_and_run.bat     # Build and run script
└── .vscode/              # VSCode configuration
    ├── tasks.json
    ├── launch.json
    └── c_cpp_properties.json
```

## Requirements

- GCC compiler (MinGW on Windows)
- Windows OS (for conio.h and windows.h libraries)
- C++17 standard

## Game Rules

1. Start with 5 HP
2. Each enemy hit reduces HP by 1 (Boss may deal more damage)
3. Defeat all enemies in each wave to progress
4. Final wave includes a Boss enemy
5. Game ends when HP reaches 0 (GAME OVER) or Boss is defeated (YOU WIN)

## Development Status

This is a basic project structure. The full game implementation is in progress.

## Author

- **Name**: [Your Name]
- **Faculty Number**: [Your Number]
- **Compiler**: GCC

## License

Educational project for Sofia University, FMI.
