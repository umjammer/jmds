
package com.inzyme.jmds;

/**
 * Provides an interface to an input pin on a crossbar.
 * 
 * @author Mike Schrag
 */
public class DSInputCrossBarPin extends DSAbstractCrossBarPin {
    /**
     * Constructs a new DSInputCrossBarPin.
     * 
     * @param _crossBar the parent crossbar
     * @param _pinIndex the index of this pin
     * @param _pinIndexRelated the index of the related pin
     * @param _physicalType the physical type of this pin
     */
    public DSInputCrossBarPin(DSCrossBar _crossBar, int _pinIndex, int _pinIndexRelated, int _physicalType) {
        super(_crossBar, _pinIndex, _pinIndexRelated, _physicalType);
    }

    public String toString() {
        return "[DSInputCrossBarPin: physicalType = " + getPhysicalTypeName() + " (" + getPhysicalType() + "); pinIndex = "
               + getPinIndex() + "; pinIndexRelated = " + getPinIndexRelated() + "]";
    }
}
