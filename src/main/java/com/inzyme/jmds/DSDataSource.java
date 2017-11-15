
package com.inzyme.jmds;

import java.awt.Component;
import java.io.IOException;

import javax.media.Format;
import javax.media.Owned;
import javax.media.Time;
import javax.media.control.FormatControl;
import javax.media.protocol.PushBufferDataSource;
import javax.media.protocol.PushBufferStream;


/**
 * Provides a DataSource onto a particular pin of a capture device.
 * 
 * @author Mike Schrag
 */
public class DSDataSource extends PushBufferDataSource {
    private DSSourceStream myStream;
    private DSCapturePin myCapturePin;
    private boolean myCrossBarLoaded;
    private DSCrossBar myCrossBar;
    private FormatControl myFormatControl;

    /**
     * Constructs a new DSDataSource.
     * 
     * @param _capturePin the capture pin to attach to
     */
    public DSDataSource(DSCapturePin _capturePin) {
        myCapturePin = _capturePin;
        myStream = new DSSourceStream(_capturePin);
        myFormatControl = new DSDataSourceFormatControl();
    }

    /**
     * Returns a list of available streams for this data source.
     * 
     * @return a list of available streams for this data source
     */
    public PushBufferStream[] getStreams() {
        return new PushBufferStream[] {
            myStream
        };
    }

    /**
     * Returns the content type of the underlying stream.
     * 
     * @reutrn the content type of the underlying stream
     */
    public String getContentType() {
        return myStream.getContentDescriptor().getContentType();
    }

    /**
     * Connects to this DataSource.
     */
    public void connect() throws IOException {
        myStream.connect();
    }

    /**
     * Disconnects from this DataSource.
     */
    public void disconnect() {
        myStream.disconnect();
    }

    /**
     * Starts the capture device stream for this pin.
     */
    public void start() throws IOException {
        myStream.start();
    }

    /**
     * Stops the capture device stream for this pin.
     */
    public void stop() throws IOException {
        myStream.stop();
    }

    /**
     * Returns the set of available controls for this
     * DataSource. Currently the only available control
     * is the FormatControl.
     * 
     * @return the set of available controls
     */
    public Object[] getControls() {
        return new Object[] {
            myFormatControl
        };
    }

    /**
     * Returns the control for the particular class name. Currently
     * only FormatControl is supported.
     * 
     * @param _controlClass the class name of the control to retrieve
     * @return the control for the particular class name (or null if not
     *         available)
     */
    public Object getControl(String _controlClass) {
        Object control = null;
        if (FormatControl.class.getName().equals(_controlClass)) {
            control = myFormatControl;
        }
        return control;
    }

    /**
     * Returns the duration of this stream (which is TIME_UNKNOWN)
     */
    public Time getDuration() {
        return Time.TIME_UNKNOWN;
    }

    /**
     * Returns whether or not this DataSource has a crossbar. This can
     * only be called after connecting to the DataSource.
     * 
     * @return whether or not this DataSource has a crossbar
     */
    public boolean hasCrossBar() {
        ensureCrossBarLoaded();
        return myCrossBar != null;
    }

    /**
     * Returns the crossbar for this device (or null if there isn't one).
     * This can only be called after connecting to the DataSource.
     * 
     * @return the crossbar for this device (or null if there isn't one)
     */
    public DSCrossBar getCrossBar() {
        ensureCrossBarLoaded();
        return myCrossBar;
    }

    private void ensureCrossBarLoaded() {
        if (!myCrossBarLoaded) {
            myCrossBar = getCrossBar0();
            myCrossBarLoaded = true;
        }
    }

    private native DSCrossBar getCrossBar0();

    public class DSDataSourceFormatControl implements FormatControl, Owned {
        public Component getControlComponent() {
            return null;
        }

        public Format[] getSupportedFormats() {
            return myCapturePin.getFormats();
        }

        public boolean isEnabled() {
            return true;
        }

        public void setEnabled(boolean _enabled) {
        }

        public Format getFormat() {
            return myStream.getFormat();
        }

        public Object getOwner() {
            return DSDataSource.this;
        }

        /**
         * Currently you can't request a format that is not explicitly
         * supported by the device.
         */
        public Format setFormat(Format _format) {
            int matchingIndex = -1;
            Format[] formats = getSupportedFormats();
            for (int i = 0; matchingIndex == -1 && i < formats.length; i++) {
                if (formats[i].matches(_format)) {
                    matchingIndex = i;
                }
            }
            if (matchingIndex == -1) {
                return myStream.getFormat();
            } else {
                return myStream.setFormat(matchingIndex);
            }
        }
    }
}
