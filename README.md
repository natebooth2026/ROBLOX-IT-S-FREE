# ROBLOX: IT'S FREE
To run the executable:
1. Download the Executable & Dependencies Directory
2. Run the .exe file :)
   
To run my code:
1. The VS2022 project needs to be setup to support the following includes:
   
	`#include <GL/glut.h>
	#include <GL/freeglut.h>
	#include <windows.h>
	#include <mmsystem.h>
	#include <iostream>
	#include <stdio.h>
	#include <cstdlib>
	#include <vector>
	#include <irrKlang/irrKlang.h>
	using namespace irrklang;`

2. Place the provided `audio`, `include`, & `lib` directories as well as main.cpp into the project directory
3. Place the provided `.dll` files into the project directory
4. Go to Project Properties --> C/C++ --> General --> Additional Include Directories and insert `./include`
5. Go to Linker --> General --> Additional Library Directories and insert `./lib`
6. Go to Linker --> Input --> Additional Dependencies and insert `freeglut.lib; opengl32.lib; winmm.lib; irrklang.lib`
7. Save all changes
8. Run the code :)

SOUND SOURCES:
“Roblox Death Sound - Sound Effect (HD).” Www.youtube.com, www.youtube.com/watch?v=3w-2gUSus34.

Gunslinger. “ROBLOX Music - Badliz - the Great Strategy (ROBLOX Theme Song).” YouTube, 23 May 2017, www.youtube.com/watch?v=S-XBFN6LR0Y.

“Desert Eagle Single Shot Gunshot Sound Effect.” Www.youtube.com, www.youtube.com/watch?v=bdjxBg55mRk.

Mr Vibe SFX. “Bullet Impact, Body, Concrete, Metal, Fly by - Play Once - Free SFX - No Copyright.” YouTube, 2 Jan. 2023, www.youtube.com/watch?v=n49bNiFBvgU.

‌
‌

‌
