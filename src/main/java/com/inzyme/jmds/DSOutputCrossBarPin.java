
package com.inzyme.jmds;

/**
 * Provides an interface to an output pin of a crossbar.
 * 
 * @author Mike Schrag
 */
public class DSOutputCrossBarPin extends DSAbstractCrossBarPin {
    private int myRoutedToIndex;

    /**
     * Constructs a new DSOutputCrossBarPin.
     * 
     * @param _crossBar the parent crossbar
     * @param _pinIndex the index of this pin
     * @param _pinIndexRelated the index of the related pin
     * @param _physicalType the physical type of this pin
     * @param _routedToIndex the index of the input pin that this pin is routed
     *            to
     */
    public DSOutputCrossBarPin(DSCrossBar _crossBar,
            int _pinIndex,
            int _pinIndexRelated,
            int _physicalType,
            int _routedToIndex) {
        super(_crossBar, _pinIndex, _pinIndexRelated, _physicalType);
        myRoutedToIndex = _routedToIndex;
    }

    int getRoutedToIndex() {
        return myRoutedToIndex;
    }

    void setRoutedToIndex(int _routedToIndex) {
        myRoutedToIndex = _routedToIndex;
    }

    /**
     * Routes this output pin to the specified input pin.
     * 
     * @param _inputPin the input pin to route to
     */
    public void routeTo(DSInputCrossBarPin _inputPin) {
        getCrossBar().route(this, _inputPin);
    }

    /**
     * Returns the input pin that this output pin is routed to
     * 
     * @return the input pin that this output pin is routed to (or null if it's
     *         not routed)
     */
    public DSInputCrossBarPin getRoutedToPin() {
        return getCrossBar().getInputPinAt(myRoutedToIndex);
    }

    public String toString() {
        return "[DSOutputCrossBarPin: physicalType = " + getPhysicalTypeName() + " (" + getPhysicalType() + "); pinIndex = "
               + getPinIndex() + "; pinIndexRelated = " + getPinIndexRelated() + "; routedToIndex = " + myRoutedToIndex + "]";
    }
}
