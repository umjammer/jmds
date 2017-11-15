package com.inzyme.jmds;

/**
 * The base class for input and output crossbar pins.
 * 
 * @author Mike Schrag
 */
public abstract class DSAbstractCrossBarPin {
    private DSCrossBar myCrossBar;
    private int myPinIndex;
    private int myPinIndexRelated;
    private int myPhysicalType;

    /**
     * Constructs a new DSAbstractCrossBarPin.
     * 
     * @param _crossBar the parent crossbar
     * @param _pinIndex the index of this pin
     * @param _pinIndexRelated the index of the related pin
     * @param _physicalType the physical type of this pin
     */
    DSAbstractCrossBarPin(DSCrossBar _crossBar, int _pinIndex, int _pinIndexRelated, int _physicalType) {
        myCrossBar = _crossBar;
        myPinIndex = _pinIndex;
        myPinIndexRelated = _pinIndexRelated;
        myPhysicalType = _physicalType;
    }

    /**
     * Returns the physical connector type of this pin (one of
     * DSPhysicalConnectorType.PhysConn_xxx)
     * 
     * @return the physical connector type of this pin
     */
    public int getPhysicalType() {
        return myPhysicalType;
    }

    int getPinIndex() {
        return myPinIndex;
    }

    int getPinIndexRelated() {
        return myPinIndexRelated;
    }

    DSCrossBar getCrossBar() {
        return myCrossBar;
    }

    /**
     * Returns the related pin for this pin. A related pin would be, for
     * instance, an
     * audio pin for its corresponding video pin.
     * 
     * @return the related pin for this pin
     */
    public DSInputCrossBarPin getRelatedPin() {
        return myCrossBar.getInputPinAt(myPinIndexRelated);
    }

    /**
     * Convenience method to retrieve the name of the physical connector type of
     * this pin.
     * 
     * @return the name of the physical connector type of this pin
     */
    public String getPhysicalTypeName() {
        return DSPhysicalConnectorType.getPhysicalTypeName(myPhysicalType);
    }
}
