
package com.inzyme.jmds;

import java.util.NoSuchElementException;


/**
 * Provides an interface to controlling a DirectShow Crossbar filter.
 * 
 * @author Mike Schrag
 */
public class DSCrossBar {
    private DSDataSource myDataSource;
    private DSInputCrossBarPin[] myInputPins;
    private DSOutputCrossBarPin[] myOutputPins;

    /**
     * Constructs a new DSCrossBar.
     * 
     * @param _dataSource the parent DataSource
     */
    public DSCrossBar(DSDataSource _dataSource) {
        myDataSource = _dataSource;
    }

    /**
     * Returns the set of input pins for this CrossBar.
     * 
     * @return the set of input pins for this CrossBar
     */
    public DSInputCrossBarPin[] getInputPins() {
        if (myInputPins == null) {
            fillInCrossBarPins();
        }
        return myInputPins;
    }

    /**
     * Returns the set of output pins for this CrossBar.
     * 
     * @return the set of output pins for this CrossBar
     */
    public DSOutputCrossBarPin[] getOutputPins() {
        if (myOutputPins == null) {
            fillInCrossBarPins();
        }
        return myOutputPins;
    }

    /**
     * Routes the specified output pin to the specified input pin. Currently
     * this will not change the route of the related pin automatically.
     * 
     * @param _outputPin the output pin to route from
     * @param _inputPin the input pin to route to
     */
    public void route(DSOutputCrossBarPin _outputPin, DSInputCrossBarPin _inputPin) {
        route0(_outputPin.getPinIndex(), _inputPin.getPinIndex());

        // force a reload now that we've changed things (this could clearly be optimized)
        myInputPins = null;
        myOutputPins = null;
    }

    /**
     * Returns the input pin that has the given physical connector type.
     * 
     * @param _physicalType the physical connector type to retrieve (one of
     *            DSPhysicalConnectorType.PhysConn_xxx)
     * @return the matching input pin (or null if there is no match)
     */
    public DSInputCrossBarPin getInputPinWithPhysicalType(int _physicalType) {
        DSInputCrossBarPin matchingInputPin = null;
        DSInputCrossBarPin[] inputPins = getInputPins();
        for (int i = 0; matchingInputPin == null && i < inputPins.length; i++) {
            if (inputPins[i].getPhysicalType() == _physicalType) {
                matchingInputPin = inputPins[i];
            }
        }

        if (matchingInputPin == null) {
            throw new NoSuchElementException("There is no input pin with the type "
                                             + DSPhysicalConnectorType.getPhysicalTypeName(_physicalType));
        }

        return matchingInputPin;
    }

    /**
     * Returns the output pin that has the given physical connector type.
     * 
     * @param _physicalType the physical connector type to retrieve (one of
     *            DSPhysicalConnectorType.PhysConn_xxx)
     * @return the matching output pin (or null if there is no match)
     */
    public DSOutputCrossBarPin getOutputPinWithPhysicalType(int _physicalType) {
        DSOutputCrossBarPin matchingOutputPin = null;
        DSOutputCrossBarPin[] outputPins = getOutputPins();
        for (int i = 0; matchingOutputPin == null && i < outputPins.length; i++) {
            if (outputPins[i].getPhysicalType() == _physicalType) {
                matchingOutputPin = outputPins[i];
            }
        }

        if (matchingOutputPin == null) {
            throw new NoSuchElementException("There is no output pin with the type "
                                             + DSPhysicalConnectorType.getPhysicalTypeName(_physicalType));
        }

        return matchingOutputPin;
    }

    /**
     * Returns the input pin at the given index.
     * 
     * @param _index the index of the input pin to retrieve
     * @return the input pin at the given index
     */
    public DSInputCrossBarPin getInputPinAt(int _index) {
        DSInputCrossBarPin pin;
        if (_index == -1) {
            pin = null;
        } else {
            pin = getInputPins()[_index];
        }
        return pin;
    }

    /**
     * Returns the output pin at the given index.
     * 
     * @param _index the index of the output pin to retrieve
     * @return the output pin at the given index
     */
    public DSOutputCrossBarPin getOutputPinAt(int _index) {
        DSOutputCrossBarPin pin;
        if (_index == -1) {
            pin = null;
        } else {
            pin = getOutputPins()[_index];
        }
        return pin;
    }

    private native void route0(int _outputPinIndex, int _inputPinIndex);

    private native void fillInCrossBarPins();
}
