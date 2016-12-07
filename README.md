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
* Open the .xcodeproj file
* Edit the Build Settings to your desired Apple Developer Account and edit specs for specific devices you want to run on
* Run simulator or build onto your iOS device
* Enjoy! All code explanations are below

## Usage and Workflow
There are multiple parts to this project. First we will discuss the C code. 
All of the UPNP discovery is done through this part of the code. The implementation was inspired by [UPNPHacks](http://www.upnp-hacks.org/upnp.html) and the [lsupnp](https://github.com/ccoff/lsupnp) cmd line tool. After coding up the upnp discovery where it contains a hostArray that holds all discovered hosts, we call this from Objective-C. 

The rest of the application is implemented in Objective-C. The main brute-force pinging of ICMP protocol is done via Apple's [Simple Ping](https://developer.apple.com/library/content/samplecode/SimplePing/Introduction/Intro.html). From the ICMP and UPNP discoveries we can then use Apple's persistent storage to keep a log of the found devices. This way, when you next open the app we can see if the devices are new or previously found. Another added diagnostic is how reliable the given device is on n pings. For the overall subnet, I have also implemented a speedtest that utilizes the downloading of a file and gaining the roundtrip time of bytesreceived. 

## Challenges and Results
