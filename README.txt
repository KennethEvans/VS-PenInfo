This is a directory of applications to test pen opertion.

The applications are:

TiltTest.exe
    Uses Wacom API based on a WinTab sample from Wacom.
    Needs wintab32.dll.
        Comes from Wacom drivers for a Wacom tablet.
            The Tablet Name will probably be Wacom Tablet.
        On a Suface device can use the NTrig wintab32.dll and DHid.dll versions,
            The Tablet Name will probably be Microsoft device.
    
PenPointerTest.exe
    Uses Windows directly using WM_POINTER messages.
        Introduced with Windows 8.
    Was written from scratch to work similar to TiltTest.
        Using WM_POINTERUPDATE, WM_POINTERUP, WM_POINTERDOWN messages.
    This is probably the best single test of the three.
 
MTScratchpadRTStylus.exe
    Uses Windows RealTime Stylus API.
        Introduced with Windows 7.
    Based on a Windows example program.
    Uses RTSCom.dll (Others don't).
    
The programs in this directory are designed to be run from a USB drive so they can be tested on various computers.

They must be run from this directory.
