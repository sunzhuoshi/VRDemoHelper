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

7. Show warning overlap when steam menu is toogled(you can't easily tell by game screen)  
The issue can be easily solved by turning on Steam Arcade mode, system menu will be blocked.  
Offer a shortcut in VRDemoHelper should be a resonable solution for non-professional steam users.
### TODO 
1. make "pause" hotkey configuable(F8 by default)

2. Turn on Steam VR Arcade mode by modifying $(Steam)/config/steamvr.settings and restart SteamVR process   
	
	<pre>
	{
   		"dashboard" : {
      		"arcadeMode" : false
   		}
	}
	</pre>
Rename "Hide Steam VR Notification" to Set SteamVR for Demo(Turn on Arcade Mode, Minimize Steam VR, Hide Steam VR Notifications and etc)  

3. localization for existing strings

4. add default implementation or create one if settings.ini doesn't exist to make configuration files optional  
   
5. hook D3D to show FPS like fraps

6. support win32 games and applications

## Files
### settings.ini  
Used to define windows(games and steam vr) to be processed

## FAQ
none for now, if you have one, email me at sunzhuoshi#gmail.com