# void-signal
# VOID//SIGNAL
read.me has been made with help of AI as im not good at it

A small 2D sci-fi exploration game made with C++ and raylib.

You play as Unit 07, a maintenance robot waking up aboard a damaged
crews­hip that has been pulled toward a strange black hole. Most of the
crew is missing, the ship is falling apart, and there's a signal coming
from somewhere inside the anomaly.

Your job is simple:

Explore the ship.
Recover the crew's caches.
Repair the systems.
Figure out what happened.
And hopefully, get out alive.

## 🎮 Gameplay

VOID//SIGNAL is focused on exploration and atmosphere rather than
complicated combat.

You can:

- Explore different sections of the ship
- Find and recover hidden caches
- Scavenge materials
- Repair ship systems
- Access damaged terminals
- Decode encrypted crew messages
- Progress through 5 levels
- Uncover the story through recovered transmissions

There are also small environmental effects like flickering lights,
an animated black hole, glowing robot eyes and other procedural effects.

## 🕹️ Controls

| Key | Action |
|-----|--------|
| W A S D | Move |
| E | Interact / Scavenge |
| ENTER | Continue / Confirm |
| 0 - 9 | Enter cipher codes |
| ESC | Pause / Cancel |

## 🛠️ Built With

- C++
- raylib
- MinGW / GCC
- Procedural 2D graphics

The game currently doesn't rely on external art for the main gameplay
scene. A lot of the visuals are generated directly with raylib.

## 📖 Story

Something went very wrong aboard the ship.

The crew detected a repeating signal coming from inside a nearby
black-hole-like anomaly. Before they could understand it, the ship was
caught by the anomaly and heavily damaged.

Unit 07 is one of the few systems that manages to restart.

The crew left caches and emergency messages throughout the ship, but
their final destination is unknown.

As Unit 07 restores the ship, more of the story starts to come together.

Maybe the signal isn't just a signal.

## 🚧 Current State

This is a playable 5-level demo and is still a work in progress.

I'm mainly using this project to experiment with:

- Game loops and state management
- Procedural graphics
- Animation
- Level progression
- Dialogue systems
- Interaction systems
- Building a complete game from scratch
- 
## 🚧 Current State(as of 19/09/202^)
- added 5 more levels ( so 10 now)
- added a crafting system
- enemy bots
- a wrench meele weapon
- tools to fix hazards and damages on the ship
- cool minigames( trust me bro)
- 
There are definitely things I want to improve, especially the visuals,
audio, transitions and overall atmosphere.

## ▶️ Building

You'll need:

- GCC / MinGW
- raylib

Compile with:

```bash
g++ main.cpp -o voidsignal.exe -I/c/raylib/raylib/src -L/c/raylib/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm
