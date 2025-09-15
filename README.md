### PC keybinds for skate

## The goal
the goal of this project is to allow skate users on day one of playing skate 4, who don't have access to a controller, to have a lightweight and temporary fix to using a controller!

## flexibility
I will prioritize flexibility in this project by having a config file, however certain things may be out of the scope of this project (mapping the mouse position to analogue for example).


## How to add your own controls.
Edit the "controller_config.json" file! It's that easy. The codes for any non letter keys can be found here: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes. If a non standard key you want to use is not available please add it yourself by editing the mapping.cpp file to include it in the big ugly list of vk keys and then run "make" (or "make build") at the root of this project.


## AI statement
Yeah I had no clue how to use CMake or how to make c program that installs the drivers for you (although I knew that would be awesome). AI was also used to speed up error testing, as I am not a c++ person so whenever I make an error I'm basically lost lmao.


## Troubleshooting
You may run into the following errors:
 + the controller doesnt show up.
    - if that happens please restart your pc.
 + the app launches then closes again.
    - This can occur whenever C++ feels like it honestly, I just re open it a bunch and it all works fine.
 + Your keycode is not supported.
    - Again, refer to how to manual add VK keys to the program, although I feel I have covered most.