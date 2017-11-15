
package com.inzyme.jmds;

import java.util.ArrayList;
import java.util.List;

import javax.media.Format;

import com.sun.media.vfw.BitMapInfo;


/**
 * Represents a single pin on a DirectShow capture device.
 * 
 * @author Mike Schrag
 */
public class DSCapturePin {
    private DSCaptureDeviceInfo myCaptureDevice;
    private String myPinID;
    private String myName;
    private Format[] myFormats;
    private List myFormatsList;

    /**
     * Constructs a DSCapturePin.
     * 
     * @param _captureDevice the DSCaptureDevice that contains this pin
     * @param _pinID the ID of this pin
     * @param _name the displayable name of this pin
     */
    public DSCapturePin(DSCaptureDeviceInfo _captureDevice, String _pinID, String _name) {
        myCaptureDevice = _captureDevice;
        myPinID = _pinID;
        myName = _name;
    }

    /**
     * Returns this pin's parent capture device.
     * 
     * @return this pin's parent capture device
     */
    public DSCaptureDeviceInfo getCaptureDevice() {
        return myCaptureDevice;
    }

    /**
     * Returns the ID of this pin.
     * 
     * @return the ID of this pin
     */
    public String getPinID() {
        return myPinID;
    }

    /**
     * Returns the name of this pin.
     * 
     * @return the name of this pin
     */
    public String getName() {
        return myName;
    }

    /**
     * Returns an array of Formats that are supported by this pin.
     * 
     * @return an array of Formats that are supported by this pin
     */
    public Format[] getFormats() {
        if (myFormats == null) {
            synchronized (this) {
                myFormatsList = new ArrayList();
                fillInFormats();
                myFormats = (Format[]) myFormatsList.toArray(new Format[myFormatsList.size()]);
            }
        }
        return myFormats;
    }

    private void formatFound(BitMapInfo _bitMapInfo, float _frameRate) {
        Format format = _bitMapInfo.createVideoFormat(Format.byteArray, _frameRate);
        myFormatsList.add(format);
    }

    private native void fillInFormats();

    public String toString() {
        return "[DSCapturePin: captureDevice = " + myCaptureDevice + "; pinID = " + myPinID + "; name = " + myName + "]";
    }
}
