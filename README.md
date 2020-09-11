# Pen Test Apps

This project contains several apps to test pressure-sensitive pen output. They were originally written to investigate claims made for tilt and pressure with the Microsoft Surface Pen. They should all show what values are being sent for tilt (angle and azimuth) and pressure, as well as other data available for the particular API.

The applications are:

**PenPointerTest.exe**

    Uses Windows directly using WM_POINTER messages.
        Introduced with Windows 8.
    Was written from scratch to work similar to TiltTest.
        Using WM_POINTERUPDATE, WM_POINTERUP, WM_POINTERDOWN messages.
    This is probably the best single test of the three.
 
**TiltTest.exe**

    Uses Wacom API based on a WinTab sample from Wacom.
    Needs wintab32.dll.
        Comes from Wacom drivers for a Wacom tablet.
            The Tablet Name will probably be Wacom Tablet.
        On a Suface device can use the NTrig wintab32.dll and DHid.dll versions,
            The Tablet Name will probably be Microsoft device.
    
**MTScratchpadRTStylus.exe**

    Uses Windows RealTime Stylus API.
        Introduced with Windows 7.
    Based on a Windows example program.
    Uses RTSCom.dll (Others don't).

**Installation**

Just unzip the files from a Release into a directory somewhere convenient. Then run the applications from there.
To uninstall, just delete these files.

They can be installed on a USB drive and could potentially be used to test computers you may be considering in a store.

**More Information**

More projects fronm the same author are at https://kennethevans.github.io.

Licensed under the MIT license. (See: https://en.wikipedia.org/wiki/MIT_License)