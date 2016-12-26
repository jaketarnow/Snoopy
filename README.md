# Snoopy
Snoopy is a network wifi scanner to view all devices on your home network and run diagnostics.

## Dependencies
* iOS or MacOS/OS X
* XCode 8.*

## Installation
* Download the zip of this repository or run the code below.
```
git clone https://github.com/jaketarnow/Snoopy
```
* Create a new xcode project and pull in all the files from [SnoopyFinal](https://github.com/jaketarnow/Snoopy/tree/master/Snoopy_Final)
* Edit the Build Settings to your desired Apple Developer Account and edit specs for specific devices you want to run on
* Run simulator or build onto your iOS device
* Enjoy! All code explanations are below

## Usage and Workflow
There are multiple parts to this project. First we will discuss the C code. 
All of the UPNP discovery is done through this part of the code. The implementation was inspired by [UPNPHacks](http://www.upnp-hacks.org/upnp.html) and the [lsupnp](https://github.com/ccoff/lsupnp) cmd line tool. After coding up the upnp discovery where it contains a hostArray that holds all discovered hosts, we call this from Objective-C. 

On the Objective-C side, we used the [ScanLAN](https://github.com/mongizaidi/LAN-Scan) template as implemented by Mongizaidi. As you can see on GitHub, it is the standard template to use when implenting Apple's SimplePing and Apple's Network Interfaces. 

The main brute-force pinging of ICMP protocol is done via Apple's [Simple Ping](https://developer.apple.com/library/content/samplecode/SimplePing/Introduction/Intro.html). From the ICMP and UPNP discoveries we can then use Apple's persistent storage to keep a log of the found devices. This way, when you next open the app we can see if the devices are new or previously found. Another added diagnostic is how reliable the given device is on n pings. For the overall subnet, I have also implemented a speedtest that utilizes the downloading of a file and gaining the roundtrip time of bytesreceived. 

## Challenges and Results
One of the main challenges with this project was the UPNP discovery. Implementing it where all discovered hosts are added to a hostArray that is easily accessabile from Objective-C, led to many memory errors. The use of the strncpy and strncat overruns the buffers if you are not safe and leaves the tmp variable unterminated. It would be much more efficient to allocate memory for tmp in the beginning instead of on every pass through the loop. Even if you set the value of char tmp[] to 100, it will improve the consistency of UPNP discoveries, with lack of segfaults/memory leaks. 

All of the results from this implementation are that we can discover most hosts on a home network with the given SSID names. Yet, on more secure networks such as university campus' or airports for example, we are unlikely to grab the actual SSID name and are left with the host address. When running the application, we are able to receive n amount of devices and then they are stored in NSUserDefaults at the end of the scan/termination of the app. This allows for peristent storage of devices for next run on that specific network. 

## ScreenShots
<img src="https://github.com/jaketarnow/Snoopy/blob/master/Screenshots/AllDevices_withNames.png" width="400" height="600"><img src="https://github.com/jaketarnow/Snoopy/blob/master/Screenshots/Diagnostics_newDiscovery.png" width="400" height="600"><img src="https://github.com/jaketarnow/Snoopy/blob/master/Screenshots/ScanFinished.png" width="400" height="600">


## Future Work
My future work would include working more on the reliability aspect of the app and gathering more diagnostics. The reliability currently is only calculated my number of pings that are returned non-Null to ones that are. Most of the time all of the devices are returned with 100% reliability, yet in the application it shows that sometimes they were previously found, and sometimes they were not. This should go hand-in-hand as that describes the reliability. 
