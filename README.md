# WiFlyWebServer

## Purpose

This WebServer implementation is meant to be used in constrained environments where you will likely need to constantly reuse send/receive buffers.  Just provide it a request and it will craft a response for you.

## Examples

There are a couple examples provided with this library.  All examples have the following requirements:

* May require at least a MEGA2560.  I haven't tested it on any other platform.
* ArduPilot-Mega 'make' build environment.  To get this, clone the ArduPilot-Mega repository and drop this whole directory into the libraries directory.

run 'make' followed by 'make upload' inside the appropriate example directory to get it on your board.

### WebServerDemo

This is an example WebServer running on FreeRTOS.  It assumes a Roving Networks WiFly module connected to Serial2 on your board.

Set ACCESS_POINT and PASSPHRASE in WebServerDemo.pde to get the WiFly to connect to your router.

### UnitTests

Unity Unit Tests for WebServer and classes.  I recommend you run this on your environment first to know whether not you'll have any luck resource-wise.