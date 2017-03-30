# VRDemoHelper
A simple tool to make VR demonstration better and easier

## Features
1. Maximize VR game window for demonstration, both win32(not supported now) and x64 games
2. Hide Steam VR popup window automatically(to be tested)  
TODO: use overlap window or notification rather than popup window to show the notification?
3. Show warning overlap when steam menu is toogled(you can't tell by game screen)  
TODO: make sure no simple solution exists already
4. Show FPS overlay(low priority, for steam already supports it)
5. Configurable
6. Minimize performance loss 
### TODO 
1. add default implementation and create one if log4cplus.prop or rule_config.ini doesn't exist
2. hook D3D to show FPS like fraps
3. support win32 games and applications

## Options
### -t  
turn on trace mode, in trace mode it logs hooked message. You can use it to diagnose why a rule doesn't work.  
### -p [port]   
the logserver listening port, default is 8888. You can set one in case of that it is already in used  
NOTE: used only in trace mode  

## Files
### log4cplus.props  
It uses [log4cplus](https://github.com/log4cplus/log4cplus) to log when in trace mode(option -t). There are 2 log files, one for helper itself, and the other for log server of hooked processes 

### rule_config.ini  
Used to define windows(games and steam vr) to be processed, process to be ignored in trace mode

## FAQ
none for now, if you have one, email me at sunzhuoshi#gmail.com