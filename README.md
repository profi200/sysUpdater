## sysUpdater

A quick tool for manually updating a 3DS using CIA files.

This is not intended for installing anything else! You also should strictly avoid updating single titles because this also can lead to bricks!


## Only use this if you know what you do! I'm not responsive for bricks which rely on wrong usage. Use it at your own risk.
## IMPORTANT: Don't run this with Gateway mode (it seem it always use 10.2 FIRM...) or it will brick! This is not my fault.CFW are safe since [TuxSH pull request](https://github.com/profi200/sysUpdater/pull/13). Also,do not use any other patch than signature check! (like FIRM partition writing patche)

### How to use

1. Install this app to your system.
  * It updates whatever NAND it is installed on. Installed on sysNAND for example it updates the build in NAND. Installed on emuNAND it updates emuNAND.
2. Create the dir "updates" in the root of the SD card of your 3DS.
3. Create update CIAs from Nintendos update server or get them from gamecards.
  * With [3DNUS](http://gbatemp.net/threads/3dnus.376488) for example.
4. Place all the created .cia files in the update dir you created in step 2.
5. Start the app and follow the instructions. Downgrade means it uninstalls the title first if
   the installed versions are newer.
