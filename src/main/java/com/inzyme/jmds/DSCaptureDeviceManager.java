
package com.inzyme.jmds;

import java.util.ArrayList;
import java.util.List;


/**
 * DSCaptureDeviceManager provides an interface to enumerating DirectShow
 * video capture devices.
 * 
 * @author Mike Schrag
 */
public class DSCaptureDeviceManager {
    static {
        System.loadLibrary("jmds");
    }

    /**
     * Returns an array of capture devices that are connected to your machine.
     * 
     * @return an array of capture devices that are connected to your machine
     */
    public static DSCaptureDeviceInfo[] getCaptureDevices() {
        List devicesList = new ArrayList();
        fillInDevices(devicesList);
        DSCaptureDeviceInfo[] devices = (DSCaptureDeviceInfo[]) devicesList
                .toArray(new DSCaptureDeviceInfo[devicesList.size()]);
        return devices;
    }

    private static native void fillInDevices(List _devicesList);
}
