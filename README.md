# MINECALC----Minecraft-for-the-TI-84-Plus-CE
<video src="https://github.com/imatree247/MineCalc----Minecraft-for-the-TI-84-Plus-CE/blob/main/maintrailer.mp4" controls width="100%" poster="assets/thumbnail.png">
  Your browser does not support the video tag.
</video>
As the name sugests, this is a game similar to Minecraft but instead designed to run on the TI 84 Plus CE graphing calculator. It is coded in C++ using the C/C++ TI 84 Plus CE Toolchain (CEdev) as well as a little bit of assembly. It was tested using the CEmu calculator emulator. This code works for the TI 84 Plus CE Python Edition as well.

So, I bet you are wondering, how close to Minecraft is this game? This game, unlike pretty much every other Minecraft game for the calculator, is fully 3D from the first person perspective. It has a render distance of 16 blocks (taxicab/Manhattan distance, so looks like a diamond), and runs at an average of 7 fps, although the fps can vary depending on how much open space you're looking at. Sadly, in order to prioritize speed, it runs at 1/8th resolution (but it is Minecraft, do pixels really matter? :) ).

The game is chunk-based instead of one fixed map — each chunk is 32x32x32 (31x31x31 usable once you account for the barrier blocks), and you can walk between up to 62x62 chunks, with each visited chunk saved to its own appvar. There's a hotbar, crafting system, furnace/smelting, inventory, and a daylight cycle. There are about 28 block types (including glass, doors, beds, sandstone, cactus) and 18 items, including tools/weapons (wood, stone, gold, iron, and diamond rarity pickaxes and swords).

There are now two mobs: sheep and zombies. Sheep wander peacefully and, when killed, drop two wool (used for crafting beds) and one raw mutton (smeltable into cooked mutton). Zombies spawn at night, chase the player, and deal damage on contact; killing one drops rotten flesh. Weapon damage against mobs scales with whatever's in your selected hotbar slot (better swords/tools do more damage). Beds let you skip the night and set your respawn point.

The game is approaching beta but is still a work in progress and I highly recommend playing with a high amount of ram. I also recommend not doing anything too insane (aka breaking every block in a chunk to see what will happen. What will happen, you might ask? 90% it works. Some times, it might refuse to save the chunk if you don't have enough ram free though). There's also currently no automatic archiving of newly-visited chunks until you exit cleanly, so if the game crashes mid-session, any chunk you entered since your last clean save/exit won't be preserved. If you are unable to run it (because it keeps quitting) I suggest archiving all of your stuff and clearing your RAM.

One more thing to mention: in the crafting screen, you have to align all of your crafting recipies to the top left corner of the crafting table because I am NOT going to make like 5 recipies for each item. One other thing to mention: if you play the game for over 17 hours straight, I am pretty sure one of my clock variables will overflow and either the daylight cycle will look really weird or the game will crash. I have no idea. I have not tested it and don't plan to. I don't know why you would play it for that long but please don't.

 If you find a bug that is not in the list of bugs please email me at sneakybiscuit24633@gmail.com. If you fix or know how to fix a bug, you can also email me. The only reasons I won't fix a bug is if it is too hard to fix or makes the game slower.

To run the game, first download TI Connect CE or visit a website like ticalc.link. If your OS is later than 5.5, you need the arTIfiCE jailbreak* (if your OS is between 5.5 and 5.8.4 — if it's 5.8.5, arTIfiCE won't work and you are screwed, well, as of 8/10/26 when this readme was last updated. You should probably check to see if you are still screwed). This will let you run assembly programs. Third, download the MINECALC.8xp file and the MNCFTIMG.8xv assets file from the bin folder and send both to the calculator.

*arTIfiCE is not a jailbreak. It is an exploit. Everyone calls it a jailbreak though so I am.

# Controls in game:

clear --- save and exit the game. This will work from any screen.

2nd   --- move forward

alpha --- move backward

mode  --- jump

XTθN  --- open crafting menu (idk what to call the XTθN key, but it is the key below mode if that helps)

del   --- mine block / attack a mob you're looking at

stat  --- place block / use a block (crafting table, furnace, door, bed, etc.) / use a held item

plus  --- move selected block in hotbar forward 1

minus --- move selected block in hotbar backward 1

math  --- toggle fast look/turn speed

arrow keys --- changing perspective/looking around.


# Controls for crafting menu:

arrow keys --- moving cursor around

**NOTE: THERE IS A SCROLL TO THE ITEM MENU THAT IS HARD TO SEE!!! JUST MOVE THE CURSOR TO THE BOTTOM TO ACTIVATE IT!!!**

plus  --- move your cursor to the next area

minus --- move your cursor to the previous area

2nd   --- select/move block

alpha --- split block (only works on the crafting table section of the inventory screen)

enter --- craft

XTθN  --- exit the inventory screen.


# Controls for the furnace menu:

arrow keys --- moving cursor around

**NOTE: THERE IS A SCROLL TO THE FURNACE MENU THAT IS HARD TO SEE!!! JUST MOVE THE CURSOR TO THE BOTTOM TO ACTIVATE IT!!!**

2nd --- select block to be smelted

alpha --- select block to be used as fuel

XTθN --- exit menu. NOTE: when you exit, it takes all your items out of the furnace and resets fire thingy


As of making this readme, there is one bug I haven't fix yet. And it is important, so listen. **Everytime you open a world, there is a like 10% chance the screen will flicker with the background color. Just restart the game. Close it and reopen it. It will be gone.** 


One last thing: I did use generative AI (free account on Claude) to help with the programming. It didn't do anything too key to the game though ;). 

Another last thing: you are free to use my code for something as long as you provide a link or something AND don't just copy my entire game.
