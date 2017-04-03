# VRDemoHelper
A simple tool to make VR demonstration better and easier

## Features
1. Maximize VR game window for demonstration, both win32(not supported now) and x64 games  

* Hide Steam VR popup window automatically(to be fully tested)  
TODO: use overlap window or notification rather than popup window to show the notification?

* Configurable

* Minimize performance loss

* Hotkey support 

* Show FPS overlay  
low priority, for steam already supports it
UPDATE: SteamVR failed to work with Arizona Sunshine, check the cause later.

* Show warning overlap when steam menu is toogled(you can't easily tell by game screen)  
The issue can be easily solved by turning on Steam Arcade mode, system menu will be blocked.  
Offer a shortcut in VRDemoHelper should be a resonable solution for non-professional steam users.
### TODO 
1. make "pause" hotkey configuable(F8 by default)

* Turn on Steam VR Arcade mode by modifying $(Steam)/config/steamvr.settings and restart SteamVR process   
	
	<pre>
	{
   		"dashboard" : {
      		"arcadeMode" : false
   		}
	}
	</pre>
Rename "Hide Steam VR Notification" to Set SteamVR for Demo(Turn on Arcade Mode, Minimize Steam VR, Hide Steam VR Notifications and etc)  

* localization for existing strings

* add default implementation or create one if log4cplus.prop or settings.ini doesn't exist to make configuration files optional  
   
* hook D3D to show FPS like fraps

* support win32 games and applications

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