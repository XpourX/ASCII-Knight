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


## Game Rules

1. Start with 5 HP
2. Each enemy hit reduces HP by 1 (Boss may deal more damage)
3. Defeat all enemies in each wave to progress
4. Final wave includes a Boss enemy
5. Game ends when HP reaches 0 (GAME OVER) or Boss is defeated (YOU WIN)

## Author

- **Name**: Yusmen Ismail Osman
- **Faculty Number**: 5MI0600666
- **Compiler**: GCC


