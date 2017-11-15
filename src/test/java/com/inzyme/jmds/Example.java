
package com.inzyme.jmds;

import javax.media.Format;
import javax.media.control.FormatControl;


public class Example {
    public static void main(String[] _args) {
        try {
            DSCaptureDeviceInfo[] devices = DSCaptureDeviceManager.getCaptureDevices();
            for (int deviceNum = 0; deviceNum < devices.length; deviceNum++) {
                System.out.println("Device #" + deviceNum + ": " + devices[deviceNum]);
                DSCapturePin[] pins = devices[deviceNum].getPins();
                for (int pinNum = 0; pinNum < pins.length; pinNum++) {
                    System.out.println("  Pin #" + pinNum + " = " + pins[pinNum]);
                    Format[] formats = pins[pinNum].getFormats();
                    for (int formatNum = 0; formatNum < formats.length; formatNum++) {
                        System.out.println("    Format #" + formatNum + " = " + formats[formatNum]);
                    }

                    System.out.println();

                    DSDataSource dataSource = new DSDataSource(pins[pinNum]);
                    System.out.println("    Connecting to pin ...");
                    dataSource.connect();
                    try {
                        System.out.println("    Attempting to change to each available format ...");
                        FormatControl formatControl = (FormatControl) dataSource.getControl(FormatControl.class.getName());
                        for (int formatNum = 0; formatNum < formats.length; formatNum++) {
                            formatControl.setFormat(formats[formatNum]);
                            System.out.println("    Changed to " + formatControl.getFormat());
                        }

                        System.out.println();
                        System.out.println("    Checking for a crossbar.");
                        if (dataSource.hasCrossBar()) {
                            System.out.println("      Crossbar found.");
                            DSCrossBar crossBar = dataSource.getCrossBar();
                            DSInputCrossBarPin[] inputPins = crossBar.getInputPins();
                            for (int cbInputPinNum = 0; cbInputPinNum < inputPins.length; cbInputPinNum++) {
                                System.out.println("      Crossbar Input Pin #" + cbInputPinNum + ": "
                                                   + inputPins[cbInputPinNum]);
                            }

                            DSOutputCrossBarPin[] outputPins = crossBar.getOutputPins();
                            for (int cbOutputPinNum = 0; cbOutputPinNum < outputPins.length; cbOutputPinNum++) {
                                System.out.println("      Crossbar Output Pin #" + cbOutputPinNum + ": "
                                                   + outputPins[cbOutputPinNum]);
                            }

                            System.out
                                    .println("      Routing the SVideo input to the Video output (this may fail if you don't have SVideo)");
                            try {
                                DSInputCrossBarPin inputPin = crossBar
                                        .getInputPinWithPhysicalType(DSPhysicalConnectorType.PhysConn_Video_SVideo);
                                DSOutputCrossBarPin outputPin = crossBar
                                        .getOutputPinWithPhysicalType(DSPhysicalConnectorType.PhysConn_Video_VideoDecoder);
                                outputPin.routeTo(inputPin);
                            } catch (Throwable t) {
                                t.printStackTrace();
                            }
                        }
                    } finally {
                        dataSource.disconnect();
                    }
                }
            }
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }
}
