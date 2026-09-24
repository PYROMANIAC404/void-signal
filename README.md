# VOID//SIGNAL

A small 2D sci-fi exploration game made with **C++ and raylib**.

You play as **Unit 07**, a maintenance robot aboard a damaged crewship that has been pulled toward a strange black hole. The crew is missing, the ship is falling apart, and an unknown signal is coming from somewhere inside the anomaly.

Your job is simple:

**Explore. Recover. Repair. Find out what happened. Escape.**

## Description

VOID//SIGNAL is a 2D sci-fi exploration game focused on exploring a damaged spaceship, collecting resources, repairing systems, solving puzzles, and dealing with the hazards left behind after the ship was pulled toward the anomaly.

The game was built from scratch in **C++ using raylib**. Most of the visuals are drawn procedurally in code(ikr prety cool) rather than using large collections of external assets.

You play as Unit 07 and explore different sections of the ship, including the:

* Central Deck
* Lower Engine
* Upper Engine
* Water Treatment
* Cafeteria
* Medical
* Oxygen
* Communications
* Administration
* Archive

Along the way you can find resources, repair damaged systems, interact with terminals, solve puzzles, craft upgrades, and uncover what happened to the missing crew.

There are also hazards throughout the ship, including fires, hull breaches, debris and electrical hazards.

The further you go, the more the strange signal starts to make sense.

### Screenshots

*<img width="989" height="578" alt="Screenshot 2026-09-12 223934" src="https://github.com/user-attachments/assets/a4b64079-264c-4bf7-8e73-2addfd575f54" />
.*

## Getting Started

### Dependencies

You will need:

* Windows
* C++
* A C++ compiler such as MinGW / MSYS2
* raylib
* Git (optional, but recommended)

The project was developed using **MSYS2 UCRT64** and raylib.

### Installing

Clone the repository:

```bash
git clone https://github.com/PYROMANIAC404/void-signal.git
cd void-signal
```

Make sure raylib is installed and that your compiler can access the raylib headers and libraries.

For my setup, raylib is located at:

```text
C:\raylib
```

### Executing program

Compile the game from the project directory:

```bash
g++ main.cpp -o voidsignal.exe -I/c/raylib/raylib/src -L/c/raylib/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm
```

Then run:

```bash
./voidsignal.exe
```

Or open `voidsignal.exe` directly after compiling.

## Controls

| Key   | Action                       |
| ----- | ---------------------------- |
| WASD  | Move                         |
| E     | Interact                     |
| Enter | Confirm / Continue           |
| 0-9   | Enter codes / terminal input |
| Esc   | Pause / Back                 |

Some interactions only become available when you are close enough to the relevant object.

## Gameplay

### Exploration

Explore the ship room by room and look for useful resources, damaged systems, terminals and hidden areas.

### Resources

You can collect resources such as:

* Metal
* Circuits
* Power
* Data
* Bot Parts
* Coolant

These resources are used for repairs and upgrades.

### Repair & Crafting

Unit 07 can repair and upgrade itself using the resources found throughout the ship.

There are also special crafting interactions placed around the ship.

### Puzzles

Several parts of the ship require more than simply walking around.

You may encounter:

* Terminal puzzles
* Wiring puzzles
* Repair systems
* Locked areas
* Environmental problems
* Archive investigation

### Hazards

The ship isn't exactly in perfect condition.

Watch out for:

* Fire
* Hull breaches
* Falling debris
* Electrical hazards

### The Signal

Something is transmitting from the anomaly.

As you progress through the ship and recover information, you slowly begin to understand what the signal is and why it is there.

## Project Structure

The game is currently built primarily around a single C++ source file:

```text
void-signal/
│
├── main.cpp
├── voidsignal.exe
└── README.md
```

The game uses raylib for rendering, input, audio and the window/game loop.

## Help

If the game does not compile, make sure:

1. raylib is installed correctly
2. your compiler can find the raylib headers
3. the raylib library path is correct
4. you are using a compatible MinGW/MSYS2 environment

If the executable is created successfully but does not launch, check that the required raylib libraries and runtime files are available.

If you find a bug, feel free to open an issue with:

* What happened
* What you were doing when it happened
* Any error message
* Steps to reproduce it

## Built With

* **C++**
* **raylib**
* **MSYS2 / MinGW**
* **Git / GitHub**

## Credits

VOID//SIGNAL was made as a personal game development project.

AI was also used during development for assistance wit debuging(as im such an idiot T-T)  . The project itself was built, tested and assembled by me.

## License

This project is available under the license included in this repository.

---

**VOID//SIGNAL**

*The crew is gone.*

*The ship is broken.*

*Something is still transmitting.*
