# VRDemoHelper
A simple tool to make VR demonstration better and easier

## Features
1. Maximize VR game window for demonstration, both win32(not supported now) and x64 games
2. Hide Steam VR popup window automatically(to be fully tested)  
TODO: use overlap window or notification rather than popup window to show the notification?
3. Configurable
4. Minimize performance loss
5. Hotkey support 
6. Show FPS overlay
low priority, for steam already supports it
UPDATE: SteamVR failed to work with Arizona Sunshine, check the cause later.
6. Show warning overlap when steam menu is toogled(you can't easily tell by game screen)  
The issue can be easily solved by turning on Steam Arcade mode, system menu will be blocked.
### TODO 
1. localization for existing strings
2. add default implementation and create one if log4cplus.prop or rule_config.ini doesn't exist
3. hook D3D to show FPS like fraps
4. support win32 games and applications

## Options
### -t  
turn on trace mode, in trace mode it logs hooked message. You can use it to diagnose why a rule doesn't work.  
### -p [port]   
the logserver listening port, default is 8888. You can set one in case of that it is already in used  
NOTE: used only in trace mode  

## Files
### log4cplus.props  
It uses [log4cplus](https://github.com/log4cplus/log4cplus) to log when in trace mode(option -t). There are 2 log files, one for helper itself, and the other for log server of hooked processes 

### settings.ini  
Used to define windows(games and steam vr) to be processed, process to be ignored in trace mode

## FAQ
none for now, if you have one, email me at sunzhuoshi#gmail.com