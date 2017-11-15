
package com.inzyme.jmds;

import java.util.ArrayList;
import java.util.List;


/**
 * DSCaptureDeviceInfo contains metadata about a particular DirectShow
 * capture device.
 * 
 * @author Mike Schrag
 */
public class DSCaptureDeviceInfo {
    private String myName;
    private String myDevicePath;
    private DSCapturePin[] myPins;

    /**
     * Constructs a DSCaptureDeviceInfo.
     * 
     * @param _devicePath the Windows device path to the capture device
     * @param _name the display name of this device (only used for cosmetics)
     */
    public DSCaptureDeviceInfo(String _devicePath, String _name) {
        myDevicePath = _devicePath;
        myName = _name;
    }

    /**
     * Returns the device path of this capture device.
     * 
     * @return the device path of this capture device
     */
    public String getDevicePath() {
        return myDevicePath;
    }

    /**
     * Returns the displayable name of this capture device.
     * 
     * @return the displayable name of this capture device
     */
    public String getName() {
        return myName;
    }

    /**
     * Returns an array of pins that are available on this capture device.
     * 
     * @return this capture device's pins
     */
    public DSCapturePin[] getPins() {
        if (myPins == null) {
            List pinsList = new ArrayList();
            fillInPins(pinsList);
            myPins = (DSCapturePin[]) pinsList.toArray(new DSCapturePin[pinsList.size()]);
        }
        return myPins;
    }

    private native void fillInPins(List _pinsList);

    public String toString() {
        return "[DSCaptureDeviceInfo: devicePath = " + myDevicePath + "; name = " + myName + "]";
    }
}
