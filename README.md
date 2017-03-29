# VRDemoHelper
A simple tool to make VR demonstration better and easier

## Features


1. Maximize VR game window for demonstration, both win32(not supported now) and x64 games
2. Hide Steam VR popup window automatically(developing)  
TODO: use overlap window or notification rather than popup window to show the notification?
3. show FPS overlay
4. Minimize performance loss 
### TODO 
1. support "search" trigger type to deal with Steam popup window(it seems not to be hooked)
2. add default implementation and create one if log4cplus.prop or rule_config.ini don't exist
3. hook D3D to show FPS like fraps
4. support win32 games

## Options
### -t  
turn on trace mode, in trace mode it logs hooked message. You can use it to diagnose why a rule doesn't work.  
### -p [port]   
the logserver listening port, default is 8888. You can set one in case of that it is already in used

## Files
### log4cplus.props  
It uses [log4cplus](https://github.com/log4cplus/log4cplus) to log when in trace mode(option -t). There are 2 log files, one for helper itself, and the other for log server of hooked process 

### rule_config.ini  
Used to define windows(games and steam vr) to be processed, process to be ignored in trace mode

## FAQ
none for now, if you have one, email me at sunzhuoshi#gmail.com