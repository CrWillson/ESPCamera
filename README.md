# ESP32 Camera

Program to take a photo with the ESP32 camera and save it to the attached SD card.

Written by: Caleb Willson

## Usage Instructions
This program is written to take a photo on boot up and write it to the SD card. 

The file names follow the pattern: `IMAGE{image_number}.BIN`. The image number will automatically increment and is stored on the SD card in the `CONFIG.TXT` file. If the config file is deleted, a new one will be created and the numbering will start at 0. 

## Yellow Tint Issue
When images are captured soon after the ESP32 boots up, they have a strange yellow tint to them, likely due to the camera not being warmed up yet. Obviously, this colour inaccuracy leads to problems with color calibration. To fix this, the program actually takes 10 photos in rapid succession then saves the 11th photo to the SD card. This helps midigate the yellow tint. If some yellow tint is still visible, try increasing the number of "throwaway" frames taken.

## Installation Instructions
### Requirements
This project was tested using:

| **Dependency**               	| **Version**          	| **Download/Instructions** 	
|:------------------------------|:----------------------|:-
| **gcc**                      	| 14.2.0            	| [Instructions](https://www.freecodecamp.org/news/how-to-install-c-and-cpp-compiler-on-windows/)
| **cmake**                    	| 3.24.0            	| [Download](https://cmake.org/download/)
| **git**                      	| 2.39.2.windows.1  	| [Download](https://git-scm.com/download/win) 
| **ESP-IDF VSCode extension** 	| v1.8.1            	| [Installer](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension)
| **ESP-IDF Tools**            	| v5.3.1            	| [Download](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html)

Note that this project may work with older or newer versions of these dependencies. 
These are only the versions project was tested with.

**The C++ and CMake VSCode extensions are also necessary.**

> GCC, Cmake, and git should be installed with the default options to ensure the greatest chance of compatability.
> Any custom settings in these programs have not been tested or documented.

### ESP-IDF Tools Setup
To set up the ESP-IDF toolchain:

1. Download the ESP-IDF Tools Installer listed above.
2. Scroll down to where you see `ESP-IDF Tools Installer` and click on the link labled `Windows Installer Download`.
3. Click the top download link labled `Universal Online Installer` and run the installer.
4. Continue through the installer and keep the default options and filepaths.
    > When you reach the **Version of ESP-IDF** selection, make sure **v5.3.1 (release version -.zip archive download)** is selected.
    
    > When you reach the **Select Components** page, make sure **Full installation** is selected.

5. Finish the installation.

If all of the options were kept as the default, there should be two important filepaths to remember:
```(bash)
C:\Espressif\frameworks\esp-idf-v5.3.1-2
C:\Users\USERNAME\.espressif
```

### ESP-IDF VSCode Extension Setup
To set up the ESP-IDF VSCode extension:

1. Install the ESP-IDF extension from the link above or VSCode's extension manager.
2. If the configure page does not automatically open, select the new ESP-IDF icon on the lefthand column then select `Configure ESP-IDF Extension` in the lefthand commands menu. 
3. Select `Express`.
4. Enter the following options:
    * Select download server: **Github**
    * Select ESP-IDFF version: **Find ESP-IDF in your system**
    * Enter ESP-IDF directory (IDF_PATH): **C:\Espressif\frameworks\esp-idf-v5.3.1-2**
    * Enter ESP-IDF Tools directory (IDF_TOOLS_PATH): **C:\Users\USERNAME\\.espressif**
4. Select `Install` and wait until you see a menu saying `Quick actions`.
5. Make sure the ESP32-Cam is plugged into the computer.
6. On the settings bar along the bottom of the window, do the following:
    1. Select the icon of a plug and chose the USB port that the ESP32-Cam is plugged into. It should auto detect the ESP32 and suggest which one to use but it might not.
    2. Select the star icon and choose the `UART` flash method. 
    3. Select the wrench icon to begin the initial build.

 If the project fails to build, try the following:
 * Check that CMake is using the right generator:
   * Enter the VSCode command menu (Default Key: F1)
   * Enter the command: `CMake: Open Cmake Tools Extension Settings`
   * Search for the setting: `CMake: Generator`
   * Enter `Ninja` into the setting.
   * Press enter to save the setting.
* Delete the build directory:
    * You can either do this by selecting the trash can icon on the bottom settings bar or by manually deleting the build folder in the top level directory.
    * In general, if the project fails to build, this is usually a good first step.

Settle in. *The first build will take a while*.

Once the first build has finished, it can be flashed to the ESP32-Cam using the lightning bolt button on the bottom settings bar. 