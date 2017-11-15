
package com.inzyme.jmds;

/**
 * DSPhysicalConnectorType contains constants that represent various
 * types of physical connections on a capture card.
 * 
 * @author Mike Schrag
 */
public class DSPhysicalConnectorType {
    public static final int PhysConn_Video_Tuner = 1;
    public static final int PhysConn_Video_Composite = 2;
    public static final int PhysConn_Video_SVideo = 3;
    public static final int PhysConn_Video_RGB = 4;
    public static final int PhysConn_Video_YRYBY = 5;
    public static final int PhysConn_Video_SerialDigital = 6;
    public static final int PhysConn_Video_ParallelDigital = 7;
    public static final int PhysConn_Video_SCSI = 8;
    public static final int PhysConn_Video_AUX = 9;
    public static final int PhysConn_Video_1394 = 10;
    public static final int PhysConn_Video_USB = 11;
    public static final int PhysConn_Video_VideoDecoder = 12;
    public static final int PhysConn_Video_VideoEncoder = 13;
    public static final int PhysConn_Video_SCART = 14;
    public static final int PhysConn_Audio_Tuner = 4096;
    public static final int PhysConn_Audio_Line = 4097;
    public static final int PhysConn_Audio_Mic = 4098;
    public static final int PhysConn_Audio_AESDigital = 4099;
    public static final int PhysConn_Audio_SPDIFDigital = 4100;
    public static final int PhysConn_Audio_SCSI = 4101;
    public static final int PhysConn_Audio_AUX = 4102;
    public static final int PhysConn_Audio_1394 = 4103;
    public static final int PhysConn_Audio_USB = 4104;
    public static final int PhysConn_Audio_AudioDecoder = 4105;

    /**
     * Returns a "pretty" name for the given physical connector type.
     * 
     * @param _physicalType the physical connector type (one pf
     *            DSPhysicalConnectorType.PhysConn_xxx)
     * @return a displayable name for the connector
     */
    public static String getPhysicalTypeName(int _physicalType) {
        switch (_physicalType) {
        case DSPhysicalConnectorType.PhysConn_Video_Tuner:
            return "Video Tuner";
        case DSPhysicalConnectorType.PhysConn_Video_Composite:
            return "Video Composite";
        case DSPhysicalConnectorType.PhysConn_Video_SVideo:
            return "S-Video";
        case DSPhysicalConnectorType.PhysConn_Video_RGB:
            return "Video RGB";
        case DSPhysicalConnectorType.PhysConn_Video_YRYBY:
            return "Video YRYBY";
        case DSPhysicalConnectorType.PhysConn_Video_SerialDigital:
            return "Video Serial Digital";
        case DSPhysicalConnectorType.PhysConn_Video_ParallelDigital:
            return "Video Parallel Digital";
        case DSPhysicalConnectorType.PhysConn_Video_SCSI:
            return "Video SCSI";
        case DSPhysicalConnectorType.PhysConn_Video_AUX:
            return "Video AUX";
        case DSPhysicalConnectorType.PhysConn_Video_1394:
            return "Video 1394";
        case DSPhysicalConnectorType.PhysConn_Video_USB:
            return "Video USB";
        case DSPhysicalConnectorType.PhysConn_Video_VideoDecoder:
            return "Video Decoder";
        case DSPhysicalConnectorType.PhysConn_Video_VideoEncoder:
            return "Video Encoder";

        case DSPhysicalConnectorType.PhysConn_Audio_Tuner:
            return "Audio Tuner";
        case DSPhysicalConnectorType.PhysConn_Audio_Line:
            return "Audio Line";
        case DSPhysicalConnectorType.PhysConn_Audio_Mic:
            return "Audio Microphone";
        case DSPhysicalConnectorType.PhysConn_Audio_AESDigital:
            return "Audio AES/EBU Digital";
        case DSPhysicalConnectorType.PhysConn_Audio_SPDIFDigital:
            return "Audio S/PDIF";
        case DSPhysicalConnectorType.PhysConn_Audio_SCSI:
            return "Audio SCSI";
        case DSPhysicalConnectorType.PhysConn_Audio_AUX:
            return "Audio AUX";
        case DSPhysicalConnectorType.PhysConn_Audio_1394:
            return "Audio 1394";
        case DSPhysicalConnectorType.PhysConn_Audio_USB:
            return "Audio USB";
        case DSPhysicalConnectorType.PhysConn_Audio_AudioDecoder:
            return "Audio Decoder";

        default:
            return "Unknown Type";
        }
    }
}
