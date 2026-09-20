VOID//SIGNAL

read.me has been made with help of AI because im not good at writing READMEs

A small 2D sci-fi exploration game made with C++ and raylib.

You play as Unit 07, a maintenance robot waking up aboard a damaged crewship that has been pulled toward a strange black-hole-like anomaly.

Most of the crew is missing, the ship is falling apart, and there's a strange signal coming from somewhere inside the anomaly.

Your job is simple:

Explore the ship. Recover the crew's caches. Repair the systems. Figure out what happened. And hopefully, get out alive.

🎮 Gameplay

VOID//SIGNAL is mainly focused on exploration, atmosphere, survival and figuring stuff out, with some combat and crafting mixed in.

You can:

Explore 10 different levels
Find and recover hidden crew caches
Scavenge materials around the ship
Craft items and weapons
Fight corrupted enemy robots
Use a wrench as a melee weapon
Repair damaged ship systems
Deal with hazards like fires, hull breaches and electrical problems
Access damaged terminals
Decode encrypted crew messages
Complete small repair/minigame sections
Discover transmissions left behind by the crew
Progress through the ship and uncover what happened

There are also various procedural effects, including:

Flickering lights
Glowing robot eyes
Animated black-hole effects
Environmental animations
Electrical effects
Other small details made directly with raylib
🕹️ Controls
Key	Action
W A S D	Move
E	Interact / Scavenge
ENTER	Continue / Confirm
0 - 9	Enter cipher codes
ESC	Pause / Cancel
🛠️ Built With
C++
raylib
MinGW / GCC
Procedural 2D graphics

The main gameplay scene doesn't rely on a bunch of external art assets. A lot of the visuals are generated directly with raylib.

📖 Story

Something went very wrong aboard the ship.

The crew detected a repeating signal coming from inside a nearby black-hole-like anomaly.

Before they could understand it, the ship was caught by the anomaly and heavily damaged.

Unit 07 is one of the few systems that manages to restart.

The crew left caches, emergency messages and encrypted information throughout the ship, but their final destination is unknown.

As Unit 07 restores the ship and explores deeper into it, more of the story starts to come together.

Maybe the signal isn't just a signal.

🚧 Current State

VOID//SIGNAL is currently a playable work-in-progress game.

The project has grown quite a bit from its original idea.

Current features
10 playable levels
Exploration system
Enemy robots
Combat
Wrench melee weapon
Weapon crafting
Resource/salvage system
Ship repair tools
Environmental hazards
Interactive terminals
Cipher/decoding sections
Repair minigames
Dialogue
Level progression
Procedural animations and effects
Story/transmission system

I'm mainly using this project to experiment with:

Game loops and state management
Procedural graphics
Animation
Level design
Dialogue systems
Interaction systems
Combat systems
Crafting systems
Building a complete game from scratch

There are definitely still things I want to improve, especially the visuals, audio, transitions, combat and overall atmosphere.

▶️ Building
Requirements

You'll need:

Windows
GCC / MinGW
raylib
A C++ compiler
Compile

From the project directory, run:

g++ main.cpp -o voidsignal.exe -I/c/raylib/raylib/src -L/c/raylib/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm

Then run:

./voidsignal.exe
If main.cpp can't be found

Make sure your terminal is actually inside the project folder before compiling.

For example:

cd /c/PROJECTS/VOID-SIGNAL

Then:

g++ main.cpp -o voidsignal.exe -I/c/raylib/raylib/src -L/c/raylib/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm
📦 Releases

Playable releases are available on the GitHub Releases page.

The latest release is currently VOID//SIGNAL v2.0.0.

🤖 About

A retro-ish sci-fi exploration and decoding game where you explore a damaged ship, fight corrupted robots, repair systems, craft equipment and decode messages from space.

Made by PYROMANIAC404 using C++ and raylib.

And yes, this started as a much smaller project.

It got out of hand.
