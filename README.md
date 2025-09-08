# space-invaders-mini
This is a C++ Raylib game. 
It is a clone of space invaders featuring three stages with increasing difficulty.
It has two themes: normal, and dark.
The goal is to dodge the turrets shot by the Enemies, and shoot our own to eliminate the enemies.
Each Enemy turret deals 1-4 HP randomly on each hit.
Each Player turret deals 1 HP to the enemy.

The Stages of the game are given below: 
* Stage 1: This stage is the starting stage. It has 5 Enemies shooting at the player at a cooldown of 300ms per bullet shot by any Enemy. Each enemy in this stage has 2HP.The player has a total of 25HP. This Stage is easy difficulty.
* Stage 2: This stage is the middle stage. It has 10 Enemies shooting at the player at a cooldown of 250ms per bullet shot by any Enemy. Each enemy in this stage has 3HP. The player has a total of 50HP. This Stage is medium difficulty.
* Stage 3: This is the final stage. It has 15 Enemies shooting at the player at a cooldown of 200ms per bullet shot by any Enemy. Each enemy in this stage has 4HP. The player has a total of 100HP. This stage is the hard difficulty.

After eliminating all the Enemies in all stages, the game will show a victory screen.
If you lose all of your health in any stage, you will be shown a you died! screen.
