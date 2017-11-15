
package com.inzyme.jmds;

import java.io.IOException;

import javax.media.Buffer;
import javax.media.Format;
import javax.media.format.VideoFormat;
import javax.media.protocol.BufferTransferHandler;
import javax.media.protocol.ContentDescriptor;
import javax.media.protocol.PushBufferStream;
import javax.media.protocol.SourceStream;

import com.sun.media.vfw.BitMapInfo;


/**
 * DSSourceStream provides an interface to the data stream from the
 * DirectShow capture device.
 * 
 * Note that must stop and disconnect this stream before closing your
 * VM. There is a finalizer on this object, but if your VM process is
 * killed, your capture card can be left in an unstable state. This
 * can effectively turn this class into DSRebootYourComputerSourceStream.
 * 
 * @author Mike Schrag
 */
public class DSSourceStream implements PushBufferStream {
    private DSCapturePin myCapturePin;
    private CaptureRunnable myCaptureRunnable;
    private Thread myCaptureThread;
    private BufferTransferHandler myTransferHandler;
    private boolean myConnected;
    private boolean myStarted;
    private boolean myStopRequested;
    private Format myFormat;
    private Object myRestartLock = new Object();

    /**
     * Constructs a new DSSourceStream.
     * 
     * @param _capturePin the capture pin to connect to
     */
    public DSSourceStream(DSCapturePin _capturePin) {
        myCapturePin = _capturePin;
        myCaptureRunnable = new CaptureRunnable();
    }

    /**
     * Returns the current format for this stream.
     * 
     * @return the current format for this stream
     */
    public Format getFormat() {
        BitMapInfo bmi = new BitMapInfo();
        fillInBitMapInfo(bmi);
        float frameRate = getFrameRate();
        myFormat = bmi.createVideoFormat(Format.byteArray, frameRate);
        return myFormat;
    }

    /**
     * Sets the format of this stream to the specified format index (as returned
     * by this stream's DataSource's FilterControl.getSupportedFormats(). I
     * haven't
     * tried changing formats after the stream is started, so you're on your own
     * there for now.
     * 
     * @param _index the index of the format to set
     */
    public Format setFormat(int _index) {
        setFormat0(_index);
        return getFormat();
    }

    /**
     * Reads from the capture device into the specified buffer.
     * 
     * @param _buffer the buffer to read into
     */
    public void read(Buffer _buffer) throws IOException {
        Object data = _buffer.getData();
        int length = ((VideoFormat) myFormat).getMaxDataLength();

        if (data == null || !(data instanceof byte[]) || ((byte[]) data).length != length) {
            data = new byte[length];
            _buffer.setData(data);
            _buffer.setLength(length);
        }

        int offset = _buffer.getOffset();
        _buffer.setFormat(myFormat);

        int pos = 0;
        while (!myStopRequested && pos < length) {
            pos += fillBuffer((byte[]) data, offset + pos, length - pos);
        }

        _buffer.setData(data);
        _buffer.setOffset(offset);
        _buffer.setLength(length);
        _buffer.setTimeStamp(Buffer.TIME_UNKNOWN);
    }

    public void setTransferHandler(BufferTransferHandler _transferHandler) {
        myTransferHandler = _transferHandler;
    }

    public ContentDescriptor getContentDescriptor() {
        return new ContentDescriptor(ContentDescriptor.RAW);
    }

    public long getContentLength() {
        return SourceStream.LENGTH_UNKNOWN;
    }

    /**
     * Always returns false.
     */
    public boolean endOfStream() {
        return false;
    }

    /**
     * Always returns null.
     */
    public Object getControl(String _controlClass) {
        return null;
    }

    /**
     * Returns an empty array
     */
    public Object[] getControls() {
        return new Object[0];
    }

    /**
     * Starts this stream. You must have connected to this stream prior to
     * attempting
     * to start it.
     * 
     * This will create a capture thread that will poll your capture device,
     * attempting to
     * read frames off as quickly as possible and push them out onto the stream.
     */
    public void start() {
        if (!myConnected) {
            throw new IllegalStateException("You must connect to this stream prior to starting it.");
        }

        // try { new BufferedReader(new InputStreamReader(System.in)).readLine(); } catch (Throwable t) { }
        if (myStarted) {
            return;
        }

        // block on the restart lock in case there is a currently stopping
        // or starting thread
        synchronized (myRestartLock) {
            myStarted = true;
            myStopRequested = false;

            myCaptureThread = new Thread(myCaptureRunnable, "Capture Thread");
            myCaptureThread.start();

            try {
                myRestartLock.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Request for this stream to stop. This is not a blocking call, but the
     * stream
     * will stop as quickly as it can (usually within a single frame).
     */
    public void stop() {
        if (!myStarted) {
            return;
        }

        myStopRequested = true;
    }

    protected void finalize() throws Throwable {
        stop();
        disconnect();
    }

    void connect() {
        if (myConnected) {
            return;
        }
        myConnected = true;
        connect0();
    }

    void disconnect() {
        if (!myConnected) {
            return;
        }
        disconnect0();
        myConnected = false;
    }

    private native void setFormat0(int _index);

    private native void fillInBitMapInfo(BitMapInfo _bitMapInfo);

    private native float getFrameRate();

    private native void connect0();

    private native void start0();

    private native void stop0();

    private native void disconnect0();

    private native int getBufferSize();

    private native int fillBuffer(byte[] _buffer, int _offset, int _length);

    private class CaptureRunnable implements Runnable {
        public void run() {
            start0();

            // notify sleepers that we're done starting up
            synchronized (myRestartLock) {
                myRestartLock.notifyAll();
            }

            // chill until there is data in the buffer 
            while (!myStopRequested && getBufferSize() == 0) {
                try {
                    Thread.sleep(100);
                } catch (Throwable t) {
                    t.printStackTrace();
                }
            }

            // grab frames as fast as possible
            while (!myStopRequested) {
                if (myTransferHandler != null) {
                    myTransferHandler.transferData(DSSourceStream.this);
                } else {
                    // if there's no transfer handler, just wait for a bit and try again
                    try {
                        Thread.sleep(100);
                    } catch (Throwable t) {
                    }
                }

                // This sleep is here because there is the
                // potential that you can starve your PC 
                // while polling the capture card for video.
                try {
                    Thread.sleep(20);
                } catch (Throwable t) {
                }
            }

            stop0();

            // notify waiters that we're done stopping the thread
            synchronized (myRestartLock) {
                myStarted = false;
                myRestartLock.notify();
            }
        }
    }
}
