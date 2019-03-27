/*
 * LTE PHY PDSCH Decoding Result
 */

#include "consts.h"
#include "log_packet.h"
#include "log_packet_helper.h"

const Fmt LtePhyPdschDecodingResult_Fmt[] = {
    {UINT, "Version", 1},
};

const Fmt LtePhyPdschDecodingResult_Payload_v24[] = {
    {UINT, "Serving Cell ID", 4},                     // 9 btis
    {PLACEHOLDER, "Starting Subframe Number", 0},     // 4 bits
    {PLACEHOLDER, "Starting System Frame Number", 0}, // 10 bits
    {PLACEHOLDER, "UE Category", 0}, // right shift 1 bit, 4 bits
    {PLACEHOLDER, "Num DL HARQ", 0}, // 4 bits
    {UINT, "TM Mode", 1},            // right shift 4 bits, 4 bits
    {SKIP, NULL, 1},
    {UINT, "Carrier Index", 1},            // 3 bits
    {PLACEHOLDER, "Number of Records", 0}, // 5 bits
};

const Fmt LtePhyPdschDecodingResult_Payload_v44[] = {
    {UINT, "Serving Cell ID", 4},                     // 9 btis
    {PLACEHOLDER, "Starting Subframe Number", 0},     // 4 bits
    {PLACEHOLDER, "Starting System Frame Number", 0}, // 10 bits
    {PLACEHOLDER, "UE Category", 0}, // right shift 1 bit, 4 bits
    {PLACEHOLDER, "Num DL HARQ", 0}, // 4 bits
    {UINT, "TM Mode", 1},            // right shift 4 bits, 4 bits
    {SKIP, NULL, 4},
    {UINT, "Carrier Index", 2},            //  right shift 7 bits, 4 bits
    {PLACEHOLDER, "Number of Records", 0}, // 5 bits
};

const Fmt LtePhyPdschDecodingResult_Payload_v126[] = {
    {UINT, "Serving Cell ID", 4},                     // 9 btis
    {PLACEHOLDER, "Starting Subframe Number", 0},     // 4 bits
    {PLACEHOLDER, "Starting System Frame Number", 0}, // 10 bits
    {PLACEHOLDER, "UE Category", 0}, // right shift 1 bit, 4 bits
    {PLACEHOLDER, "Num DL HARQ", 0}, // 4 bits
    {UINT, "TM Mode", 1},            // right shift 4 bits, 4 bits
    {UINT, "C_RNTI", 2},
    {SKIP, NULL, 2},
    {UINT, "Carrier Index", 2},            //  right shift 7 bits, 4 bits
    {PLACEHOLDER, "Number of Records", 0}, // 5 bits
    {SKIP, NULL, 4}};

const Fmt LtePhyPdschDecodingResult_Record_v24[] = {
    {UINT, "Subframe Offset", 2},
    {UINT, "PDSCH Channel ID", 2},
    {UINT, "HARQ ID", 1},                        // 4 bits
    {PLACEHOLDER, "RNTI Type", 0},               // 4 bits
    {UINT, "System Information Msg Number", 2},  // 4 bits
    {PLACEHOLDER, "System Information Mask", 0}, // 12 bits
    {UINT, "HARQ Log Status", 1},                // right shift 3 bit, 2 bits
    {PLACEHOLDER, "Codeword Swap", 0},           // 1 bit
    {PLACEHOLDER, "Number of Streams", 0},       // 2 bits
};

const Fmt LtePhyPdschDecodingResult_Record_v44[] = {
    {UINT, "Subframe Offset", 2},
    {UINT, "PDSCH Channel ID", 2},
    {UINT, "HARQ ID", 1},                        // 4 bits
    {PLACEHOLDER, "RNTI Type", 0},               // 4 bits
    {UINT, "System Information Msg Number", 2},  // 4 bits
    {PLACEHOLDER, "System Information Mask", 0}, // 12 bits
    {UINT, "HARQ Log Status", 1},                // right shift 3 bit, 2 bits
    {PLACEHOLDER, "Codeword Swap", 0},           // 1 bit
    {PLACEHOLDER, "Number of Streams", 0},       // 2 bits
    {BYTE_STREAM, "Demap Sic Status", 2},
    {SKIP, NULL, 2},
};

const Fmt LtePhyPdschDecodingResult_Record_v126[] = {
    {UINT, "Subframe Offset", 2},
    {UINT, "HARQ ID", 1},                         // 4 bits
    {PLACEHOLDER, "RNTI Type", 0},                // 4 bits
    {UINT, "Codeword Swap", 1},                   // 1 bit
    {PLACEHOLDER, "Number of Transport Blks", 0}, // 2 bits
};

const Fmt LtePhyPdschDecodingResult_Stream_v24[] = {
    {UINT, "Transport Block CRC", 4},          // 1 bit
    {PLACEHOLDER, "NDI", 0},                   // 1 bit
    {PLACEHOLDER, "Code Block Size Plus", 0},  // 13 bits
    {PLACEHOLDER, "Num Code Block Plus", 0},   // 4 bits
    {PLACEHOLDER, "Max TDEC Iter", 0},         // 4 bits
    {PLACEHOLDER, "Retransmission Number", 0}, // 3 bits
    {PLACEHOLDER, "RVID", 0},                  // 2 bits
    {PLACEHOLDER, "Companding Stats", 0},      // 2 bits
    {PLACEHOLDER, "HARQ Combining", 0},        // 1 bit
    {PLACEHOLDER, "Decob TB CRC", 0},          // 1 bit
    {UINT, "Num RE", 4},                       // right shift 10 bits, 16 bits
    {PLACEHOLDER, "Codeword Index", 0},        // right shift 27 bits, 4 bits
};

const Fmt LtePhyPdschDecodingResult_Stream_v44[] = {
    {UINT, "Transport Block CRC", 4},          // 1 bit
    {PLACEHOLDER, "NDI", 0},                   // 1 bit
    {PLACEHOLDER, "Code Block Size Plus", 0},  // 13 bits
    {PLACEHOLDER, "Num Code Block Plus", 0},   // 4 bits
    {PLACEHOLDER, "Max TDEC Iter", 0},         // 4 bits
    {PLACEHOLDER, "Retransmission Number", 0}, // 3 bits
    {PLACEHOLDER, "RVID", 0},                  // 2 bits
    {PLACEHOLDER, "Companding Stats", 0},      // 2 bits
    {PLACEHOLDER, "HARQ Combining", 0},        // 1 bit
    {PLACEHOLDER, "Decob TB CRC", 0},          // 1 bit
    {UINT, "Num RE", 4},                       // right shift 10 bits, 16 bits
    {PLACEHOLDER, "Codeword Index", 0},        // right shift 27 bits, 4 bits
    {UINT, "LLR Scale", 1},                    // 4 bits
    {SKIP, NULL, 3},
};

const Fmt LtePhyPdschDecodingResult_TBlks_v126[] = {
    {UINT, "Transport Block CRC", 4},             // 1 bit
    {PLACEHOLDER, "NDI", 0},                      // 1 bit
    {PLACEHOLDER, "Retransmission Number", 0},    // 1 bit
    {PLACEHOLDER, "RVID", 0},                     // 1 bit
    {PLACEHOLDER, "Code Block Size Plus", 0},     // 13 bits
    {PLACEHOLDER, "Num Code Block Plus Data", 0}, // 4 bits
    {PLACEHOLDER, "Num Code Block Plus", 0},      // 4 bits
    {PLACEHOLDER, "Max Half Iter Data", 0},       // 4 bits

    {UINT, "Num Channel Bits", 4},
    {PLACEHOLDER, "CW Idx", 0},                 // 4 bits
    {PLACEHOLDER, "Llr Buf Valid", 0},          // 4 bits
    {PLACEHOLDER, "First Decoded CB Index", 0}, // 4 bits

    {UINT, "First Decoded CB Index Qed Iter2 Data", 4},
    {PLACEHOLDER, "Last Decoded CB Index Qed Iter2 Data", 0}, // 4 bits
    {PLACEHOLDER, "Companding Format", 0},                    // 4 bits
    {SKIP, NULL, 3},

    {UINT, "HARQ Combine Enable", 1},
};

const Fmt LtePhyPdschDecodingResult_EnergyMetric_v24[] = {
    // totally 13
    {UINT, "Energy Metric", 4},              // 21 bits
    {PLACEHOLDER, "Iteration Number", 0},    // 4 bits
    {PLACEHOLDER, "Code Block CRC Pass", 0}, // 1 bit
    {PLACEHOLDER, "Early Termination", 0},   // 1 bit
    {PLACEHOLDER, "HARQ Combine Enable", 0}, // 1 bit
    {PLACEHOLDER, "Deint Decode Bypass", 0}, // 1 bit
};

const Fmt LtePhyPdschDecodingResult_EnergyMetric_v44[] = {
    // totally 13
    {UINT, "Energy Metric", 4},              // 21 bits
    {PLACEHOLDER, "Iteration Number", 0},    // 4 bits
    {PLACEHOLDER, "Code Block CRC Pass", 0}, // 1 bit
    {PLACEHOLDER, "Early Termination", 0},   // 1 bit
    {PLACEHOLDER, "HARQ Combine Enable", 0}, // 1 bit
    {PLACEHOLDER, "Deint Decode Bypass", 0}, // 1 bit
};

const Fmt LtePhyPdschDecodingResult_EnergyMetric_v126[] = {
    // totally 13
    {UINT, "Energy Metric", 4},      // 21 bits
    {PLACEHOLDER, "Min Abs LLR", 0}, // 4 bits

    {UINT, "Half Iter Run Data", 4},         // 21 bits
    {PLACEHOLDER, "Half Iter Run", 0},       // 4 bits
    {PLACEHOLDER, "Code Block CRC Pass", 0}, // 1 bit
};

const Fmt LtePhyPdschDecodingResult_Hidden_Energy_Metrics_v126[] = {
    // totally 13
    {UINT, "Hidden Energy Metric First Half", 4},
    {UINT, "Hidden Energy Metric Second Half", 4},
};

static int _decode_lte_phy_pdsch_decoding_result_payload(const char *b,
                                                         int offset,
                                                         size_t length,
                                                         json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 24: {
        offset += _decode_by_fmt(
            LtePhyPdschDecodingResult_Payload_v24,
            ARRAY_SIZE(LtePhyPdschDecodingResult_Payload_v24, Fmt), b, offset,
            length, result);
        unsigned int temp = _search_result_uint(result, "Serving Cell ID");
        int iServingCellId = temp & 511;                      // 9 bits
        int iStartingSubframeNumber = (temp >> 9) & 15;       // 4 bits
        int iStartingSystemFrameNumber = (temp >> 13) & 1023; // 10 bits
        int iUECategory = (temp >> 24) & 15;                  // 4 bits
        int iNumDlHarq = (temp >> 28) & 15;                   // 4 bits
        temp = _search_result_int(result, "TM Mode");
        int iTmMode = (temp >> 4) & 15;
        temp = _search_result_int(result, "Carrier Index");
        int iCarrierIndex = temp & 7;      // 3 bits
        int num_record = (temp >> 3) & 31; // 5 bits

        _replace_result_int(result, "Number of Records", num_record);
        _replace_result_int(result, "Serving Cell ID", iServingCellId);
        _replace_result_int(result, "Starting Subframe Number",
                            iStartingSubframeNumber);
        _replace_result_int(result, "Starting System Frame Number",
                            iStartingSystemFrameNumber);
        _replace_result_int(result, "UE Category", iUECategory);
        _replace_result_int(result, "Num DL HARQ", iNumDlHarq);
        _replace_result_int(result, "TM Mode", iTmMode);
        _replace_result_int(result, "Carrier Index", iCarrierIndex);
        (void)_map_result_field_to_name(
            result, "Carrier Index", ValueNameCarrierIndex,
            ARRAY_SIZE(ValueNameCarrierIndex, ValueName), "(MI)Unknown");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePhyPdschDecodingResult_Record_v24,
                ARRAY_SIZE(LtePhyPdschDecodingResult_Record_v24, Fmt), b,
                offset, length, result_record_item);
            temp = _search_result_int(result_record_item, "HARQ ID");
            int iHarqId = temp & 15;          // 4 bits
            int iRNTIType = (temp >> 4) & 15; // 4 bits
            temp = _search_result_int(result_record_item,
                                      "System Information Msg Number");
            int iSystemInformationMsgNumber = temp & 15;     // 4 bits
            int iSystemInformationMask = (temp >> 4) & 4095; // 12 bits
            temp = _search_result_int(result_record_item, "HARQ Log Status");
            int iHarqLogStatus = (temp >> 3) & 3; // 3 + 2 bits
            int iCodewordSwap = (temp >> 5) & 1;  // 1 bit
            int num_stream = (temp >> 6) & 3;     // 2 bit

            _replace_result_int(result_record_item, "HARQ ID", iHarqId);
            _replace_result_int(result_record_item, "RNTI Type", iRNTIType);
            (void)_map_result_field_to_name(
                result_record_item, "RNTI Type", ValueNameRNTIType,
                ARRAY_SIZE(ValueNameRNTIType, ValueName), "(MI)Unknown");
            _replace_result_int(result_record_item,
                                "System Information Msg Number",
                                iSystemInformationMsgNumber);
            _replace_result_int(result_record_item, "System Information Mask",
                                iSystemInformationMask);
            _replace_result_int(result_record_item, "HARQ Log Status",
                                iHarqLogStatus);
            (void)_map_result_field_to_name(
                result_record_item, "HARQ Log Status", ValueNameHARQLogStatus,
                ARRAY_SIZE(ValueNameHARQLogStatus, ValueName), "(MI)Unknown");
            _replace_result_int(result_record_item, "Codeword Swap",
                                iCodewordSwap);
            _replace_result_int(result_record_item, "Number of Streams",
                                num_stream);

            json result_record_stream;
            for (int j = 0; j < num_stream; j++) {
                json result_record_stream_item;
                offset += _decode_by_fmt(
                    LtePhyPdschDecodingResult_Stream_v24,
                    ARRAY_SIZE(LtePhyPdschDecodingResult_Stream_v24, Fmt), b,
                    offset, length, result_record_stream_item);

                temp = _search_result_uint(result_record_stream_item,
                                           "Transport Block CRC");
                int iTransportBlockCRC = temp & 1;            // 1 bit
                int iNDI = (temp >> 1) & 1;                   // 1 bit
                int iCodeBlockSizePlus = (temp >> 2) & 8191;  // 13 bits
                int iNumCodeBlockPlus = (temp >> 15) & 15;    // 4 bits
                int iMaxTdecIter = (temp >> 19) & 15;         // 4 bits
                int iRetransmissionNumber = (temp >> 23) & 7; // 3 bits
                int iRVID = (temp >> 26) & 3;                 // 2 bits
                int iCompandingStats = (temp >> 28) & 3;      // 2 bits
                int iHarqCombining = (temp >> 30) & 1;        // 1 bit
                int iDecobTbCRC = (temp >> 31) & 1;           // 1 bit
                temp = _search_result_uint(result_record_stream_item, "Num RE");
                int iNumRE = (temp >> 10) & 65535;      // 10 + 6 bits
                int iCodewordIndex = (temp >> 27) & 15; // 27 + 4 bits

                _replace_result_int(result_record_stream_item,
                                    "Transport Block CRC", iTransportBlockCRC);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "Transport Block CRC",
                    ValueNamePassOrFail,
                    ARRAY_SIZE(ValueNamePassOrFail, ValueName), "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "NDI", iNDI);
                _replace_result_int(result_record_stream_item,
                                    "Code Block Size Plus", iCodeBlockSizePlus);
                _replace_result_int(result_record_stream_item,
                                    "Num Code Block Plus", iNumCodeBlockPlus);
                _replace_result_int(result_record_stream_item, "Max TDEC Iter",
                                    iMaxTdecIter);
                _replace_result_int(result_record_stream_item,
                                    "Retransmission Number",
                                    iRetransmissionNumber);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "Retransmission Number",
                    ValueNameNumber, ARRAY_SIZE(ValueNameNumber, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "RVID", iRVID);
                _replace_result_int(result_record_stream_item,
                                    "Companding Stats", iCompandingStats);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "Companding Stats",
                    ValueNameCompandingStats,
                    ARRAY_SIZE(ValueNameCompandingStats, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "HARQ Combining",
                                    iHarqCombining);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "HARQ Combining",
                    ValueNameEnableOrDisable,
                    ARRAY_SIZE(ValueNameEnableOrDisable, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "Decob TB CRC",
                                    iDecobTbCRC);
                _replace_result_int(result_record_stream_item, "Num RE",
                                    iNumRE);
                _replace_result_int(result_record_stream_item, "Codeword Index",
                                    iCodewordIndex);
                int num_energy_metric = iNumCodeBlockPlus;

                json result_energy_metric;
                for (int k = 0; k < num_energy_metric; k++) {
                    json result_energy_metric_item;
                    offset += _decode_by_fmt(
                        LtePhyPdschDecodingResult_EnergyMetric_v24,
                        ARRAY_SIZE(LtePhyPdschDecodingResult_EnergyMetric_v24,
                                   Fmt),
                        b, offset, length, result_energy_metric_item);
                    temp = _search_result_uint(result_energy_metric_item,
                                               "Energy Metric");
                    int iEnergyMetric = temp & 2097151;        // 21 bits
                    int iIterationNumber = (temp >> 21) & 15;  // 4 bits
                    int iCodeBlockCRCPass = (temp >> 25) & 1;  // 1 bit
                    int iEarlyTermination = (temp >> 26) & 1;  // 1 bit
                    int iHarqCombineEnable = (temp >> 27) & 1; // 1 bit
                    int iDeintDecodeBypass = (temp >> 28) & 1; // 1 bit
                    _replace_result_int(result_energy_metric_item,
                                        "Energy Metric", iEnergyMetric);
                    _replace_result_int(result_energy_metric_item,
                                        "Iteration Number", iIterationNumber);
                    _replace_result_int(result_energy_metric_item,
                                        "Code Block CRC Pass",
                                        iCodeBlockCRCPass);
                    (void)_map_result_field_to_name(
                        result_energy_metric_item, "Code Block CRC Pass",
                        ValueNamePassOrFail,
                        ARRAY_SIZE(ValueNamePassOrFail, ValueName),
                        "(MI)Unknown");
                    _replace_result_int(result_energy_metric_item,
                                        "Early Termination", iEarlyTermination);
                    (void)_map_result_field_to_name(
                        result_energy_metric_item, "Early Termination",
                        ValueNameYesOrNo,
                        ARRAY_SIZE(ValueNameYesOrNo, ValueName), "(MI)Unknown");
                    _replace_result_int(result_energy_metric_item,
                                        "HARQ Combine Enable",
                                        iHarqCombineEnable);
                    (void)_map_result_field_to_name(
                        result_energy_metric_item, "HARQ Combine Enable",
                        ValueNameEnableOrDisable,
                        ARRAY_SIZE(ValueNameEnableOrDisable, ValueName),
                        "(MI)Unknown");
                    _replace_result_int(result_energy_metric_item,
                                        "Deint Decode Bypass",
                                        iDeintDecodeBypass);

                    result_energy_metric.push_back(result_energy_metric_item);
                }
                offset += (13 - num_energy_metric) * 4;

                result_record_stream_item["Energy Metrics"] =
                    result_energy_metric;

                result_record_stream.push_back(result_record_stream_item);
            }

            result_record_item["Streams"] = result_record_stream;

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    case 44: {
        offset += _decode_by_fmt(
            LtePhyPdschDecodingResult_Payload_v44,
            ARRAY_SIZE(LtePhyPdschDecodingResult_Payload_v44, Fmt), b, offset,
            length, result);
        int temp = _search_result_uint(result, "Serving Cell ID");
        int iServingCellId = temp & 511;                      // 9 bits
        int iStartingSubframeNumber = (temp >> 9) & 15;       // 4 bits
        int iStartingSystemFrameNumber = (temp >> 13) & 1023; // 10 bits
        int iUECategory = (temp >> 24) & 15;                  // 4 bits
        int iNumDlHarq = (temp >> 28) & 15;                   // 4 bits
        temp = _search_result_int(result, "TM Mode");
        int iTmMode = (temp >> 4) & 15;
        temp = _search_result_int(result, "Carrier Index");
        int iCarrierIndex = (temp >> 7) & 15;
        int num_record = (temp >> 11) & 31;

        _replace_result_int(result, "Number of Records", num_record);
        _replace_result_int(result, "Serving Cell ID", iServingCellId);
        _replace_result_int(result, "Starting Subframe Number",
                            iStartingSubframeNumber);
        _replace_result_int(result, "Starting System Frame Number",
                            iStartingSystemFrameNumber);
        _replace_result_int(result, "UE Category", iUECategory);
        _replace_result_int(result, "Num DL HARQ", iNumDlHarq);
        _replace_result_int(result, "TM Mode", iTmMode);
        _replace_result_int(result, "Carrier Index", iCarrierIndex);
        (void)_map_result_field_to_name(
            result, "Carrier Index", ValueNameCarrierIndex,
            ARRAY_SIZE(ValueNameCarrierIndex, ValueName), "(MI)Unknown");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePhyPdschDecodingResult_Record_v44,
                ARRAY_SIZE(LtePhyPdschDecodingResult_Record_v44, Fmt), b,
                offset, length, result_record_item);
            temp = _search_result_int(result_record_item, "HARQ ID");
            int iHarqId = temp & 15;          // 4 bits
            int iRNTIType = (temp >> 4) & 15; // 4 bits
            temp = _search_result_int(result_record_item,
                                      "System Information Msg Number");
            int iSystemInformationMsgNumber = temp & 15;     // 4 bits
            int iSystemInformationMask = (temp >> 4) & 4095; // 12 bits
            temp = _search_result_int(result_record_item, "HARQ Log Status");
            int iHarqLogStatus = (temp >> 3) & 3; // 3 + 2 bits
            int iCodewordSwap = (temp >> 5) & 1;  // 1 bit
            int num_stream = (temp >> 6) & 3;     // 2 bit

            _replace_result_int(result_record_item, "HARQ ID", iHarqId);
            _replace_result_int(result_record_item, "RNTI Type", iRNTIType);
            (void)_map_result_field_to_name(
                result_record_item, "RNTI Type", ValueNameRNTIType,
                ARRAY_SIZE(ValueNameRNTIType, ValueName), "(MI)Unknown");
            _replace_result_int(result_record_item,
                                "System Information Msg Number",
                                iSystemInformationMsgNumber);
            _replace_result_int(result_record_item, "System Information Mask",
                                iSystemInformationMask);
            _replace_result_int(result_record_item, "HARQ Log Status",
                                iHarqLogStatus);
            (void)_map_result_field_to_name(
                result_record_item, "HARQ Log Status", ValueNameHARQLogStatus,
                ARRAY_SIZE(ValueNameHARQLogStatus, ValueName), "(MI)Unknown");
            _replace_result_int(result_record_item, "Codeword Swap",
                                iCodewordSwap);
            _replace_result_int(result_record_item, "Number of Streams",
                                num_stream);

            json result_record_stream;
            for (int j = 0; j < num_stream; j++) {
                json result_record_stream_item;
                offset += _decode_by_fmt(
                    LtePhyPdschDecodingResult_Stream_v44,
                    ARRAY_SIZE(LtePhyPdschDecodingResult_Stream_v44, Fmt), b,
                    offset, length, result_record_stream_item);

                temp = _search_result_uint(result_record_stream_item,
                                           "Transport Block CRC");
                int iTransportBlockCRC = temp & 1;            // 1 bit
                int iNDI = (temp >> 1) & 1;                   // 1 bit
                int iCodeBlockSizePlus = (temp >> 2) & 8191;  // 13 bits
                int iNumCodeBlockPlus = (temp >> 15) & 15;    // 4 bits
                int iMaxTdecIter = (temp >> 19) & 15;         // 4 bits
                int iRetransmissionNumber = (temp >> 23) & 7; // 3 bits
                int iRVID = (temp >> 26) & 3;                 // 2 bits
                int iCompandingStats = (temp >> 28) & 3;      // 2 bits
                int iHarqCombining = (temp >> 30) & 1;        // 1 bit
                int iDecobTbCRC = (temp >> 31) & 1;           // 1 bit
                temp = _search_result_uint(result_record_stream_item, "Num RE");
                int iNumRE = (temp >> 10) & 65535;      // 10 + 6 bits
                int iCodewordIndex = (temp >> 27) & 15; // 27 + 4 bits
                temp =
                    _search_result_uint(result_record_stream_item, "LLR Scale");
                int iLLRScale = temp & 15; // 4 bits

                _replace_result_int(result_record_stream_item,
                                    "Transport Block CRC", iTransportBlockCRC);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "Transport Block CRC",
                    ValueNamePassOrFail,
                    ARRAY_SIZE(ValueNamePassOrFail, ValueName), "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "NDI", iNDI);
                _replace_result_int(result_record_stream_item,
                                    "Code Block Size Plus", iCodeBlockSizePlus);
                _replace_result_int(result_record_stream_item,
                                    "Num Code Block Plus", iNumCodeBlockPlus);
                _replace_result_int(result_record_stream_item, "Max TDEC Iter",
                                    iMaxTdecIter);
                _replace_result_int(result_record_stream_item,
                                    "Retransmission Number",
                                    iRetransmissionNumber);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "Retransmission Number",
                    ValueNameNumber, ARRAY_SIZE(ValueNameNumber, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "RVID", iRVID);
                _replace_result_int(result_record_stream_item,
                                    "Companding Stats", iCompandingStats);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "Companding Stats",
                    ValueNameCompandingStats,
                    ARRAY_SIZE(ValueNameCompandingStats, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "HARQ Combining",
                                    iHarqCombining);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "HARQ Combining",
                    ValueNameEnableOrDisable,
                    ARRAY_SIZE(ValueNameEnableOrDisable, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "Decob TB CRC",
                                    iDecobTbCRC);
                _replace_result_int(result_record_stream_item, "Num RE",
                                    iNumRE);
                _replace_result_int(result_record_stream_item, "Codeword Index",
                                    iCodewordIndex);
                _replace_result_int(result_record_stream_item, "LLR Scale",
                                    iLLRScale);
                int num_energy_metric = iNumCodeBlockPlus;

                json result_energy_metric;
                for (int k = 0; k < num_energy_metric; k++) {
                    json result_energy_metric_item;
                    offset += _decode_by_fmt(
                        LtePhyPdschDecodingResult_EnergyMetric_v44,
                        ARRAY_SIZE(LtePhyPdschDecodingResult_EnergyMetric_v44,
                                   Fmt),
                        b, offset, length, result_energy_metric_item);
                    temp = _search_result_uint(result_energy_metric_item,
                                               "Energy Metric");
                    int iEnergyMetric = temp & 2097151;        // 21 bits
                    int iIterationNumber = (temp >> 21) & 15;  // 4 bits
                    int iCodeBlockCRCPass = (temp >> 25) & 1;  // 1 bit
                    int iEarlyTermination = (temp >> 26) & 1;  // 1 bit
                    int iHarqCombineEnable = (temp >> 27) & 1; // 1 bit
                    int iDeintDecodeBypass = (temp >> 28) & 1; // 1 bit
                    _replace_result_int(result_energy_metric_item,
                                        "Energy Metric", iEnergyMetric);
                    _replace_result_int(result_energy_metric_item,
                                        "Iteration Number", iIterationNumber);
                    _replace_result_int(result_energy_metric_item,
                                        "Code Block CRC Pass",
                                        iCodeBlockCRCPass);
                    (void)_map_result_field_to_name(
                        result_energy_metric_item, "Code Block CRC Pass",
                        ValueNamePassOrFail,
                        ARRAY_SIZE(ValueNamePassOrFail, ValueName),
                        "(MI)Unknown");
                    _replace_result_int(result_energy_metric_item,
                                        "Early Termination", iEarlyTermination);
                    (void)_map_result_field_to_name(
                        result_energy_metric_item, "Early Termination",
                        ValueNameYesOrNo,
                        ARRAY_SIZE(ValueNameYesOrNo, ValueName), "(MI)Unknown");
                    _replace_result_int(result_energy_metric_item,
                                        "HARQ Combine Enable",
                                        iHarqCombineEnable);
                    (void)_map_result_field_to_name(
                        result_energy_metric_item, "HARQ Combine Enable",
                        ValueNameEnableOrDisable,
                        ARRAY_SIZE(ValueNameEnableOrDisable, ValueName),
                        "(MI)Unknown");
                    _replace_result_int(result_energy_metric_item,
                                        "Deint Decode Bypass",
                                        iDeintDecodeBypass);

                    result_energy_metric.push_back(result_energy_metric_item);
                }
                offset += (13 - num_energy_metric) * 4;

                result_record_stream_item["Energy Metrics"] =
                    result_energy_metric;

                result_record_stream.push_back(result_record_stream_item);
            }

            result_record_item["Streams"] = result_record_stream;

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    case 126: {
        offset += _decode_by_fmt(
            LtePhyPdschDecodingResult_Payload_v126,
            ARRAY_SIZE(LtePhyPdschDecodingResult_Payload_v126, Fmt), b, offset,
            length, result);
        int temp = _search_result_uint(result, "Serving Cell ID");
        int iServingCellId = temp & 511;                      // 9 bits
        int iStartingSubframeNumber = (temp >> 9) & 7;        // 3 bits
        int iStartingSystemFrameNumber = (temp >> 13) & 2047; // 11 bits
        int iUECategory = (temp >> 24) & 15;                  // 4 bits
        int iNumDlHarq = (temp >> 28) & 15;                   // 4 bits
        temp = _search_result_int(result, "TM Mode");
        int iTmMode = (temp >> 4) & 15; // 4 bit
        temp = _search_result_int(result, "Carrier Index");
        int iCarrierIndex = (temp >> 7) & 15; // 4 bit
        int num_record = (temp >> 11) & 31;   //

        _replace_result_int(result, "Number of Records", num_record);
        _replace_result_int(result, "Serving Cell ID", iServingCellId);
        _replace_result_int(result, "Starting Subframe Number",
                            iStartingSubframeNumber);
        _replace_result_int(result, "Starting System Frame Number",
                            iStartingSystemFrameNumber);
        _replace_result_int(result, "UE Category", iUECategory);
        _replace_result_int(result, "Num DL HARQ", iNumDlHarq);
        _replace_result_int(result, "TM Mode", iTmMode);
        _replace_result_int(result, "Carrier Index", iCarrierIndex);
        (void)_map_result_field_to_name(
            result, "Carrier Index", ValueNameCarrierIndex,
            ARRAY_SIZE(ValueNameCarrierIndex, ValueName), "(MI)Unknown");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePhyPdschDecodingResult_Record_v126,
                ARRAY_SIZE(LtePhyPdschDecodingResult_Record_v126, Fmt), b,
                offset, length, result_record_item);
            temp = _search_result_int(result_record_item, "HARQ ID");
            int iHarqId = temp & 15;          // 4 bits
            int iRNTIType = (temp >> 4) & 15; // 4 bits

            temp = _search_result_int(result_record_item, "Codeword Swap");
            int iCodewordSwap = temp & 1;      // 1 bits
            int iNumofTBlks = (temp >> 1) & 7; // 3 bits

            _replace_result_int(result_record_item, "HARQ ID", iHarqId);
            _replace_result_int(result_record_item, "RNTI Type", iRNTIType);
            (void)_map_result_field_to_name(
                result_record_item, "RNTI Type", ValueNameRNTIType,
                ARRAY_SIZE(ValueNameRNTIType, ValueName), "(MI)Unknown");
            _replace_result_int(result_record_item, "Codeword Swap",
                                iCodewordSwap);
            _replace_result_int(result_record_item, "Number of Transport Blks",
                                iNumofTBlks);

            // iNumofTBlks=0;
            offset += 8;
            json result_record_stream;
            for (int j = 0; j < iNumofTBlks; j++) {
                json result_record_stream_item;
                offset += _decode_by_fmt(
                    LtePhyPdschDecodingResult_TBlks_v126,
                    ARRAY_SIZE(LtePhyPdschDecodingResult_TBlks_v126, Fmt), b,
                    offset, length, result_record_stream_item);

                temp = _search_result_uint(result_record_stream_item,
                                           "Transport Block CRC");
                int iTransportBlockCRC = temp & 1;           // 1 bit
                int iNDI = (temp >> 1) & 1;                  // 1 bit
                int iRetransmissionNumber = (temp >> 2) & 7; // 3 bits
                int iRVID = (temp >> 8) & 3;                 // 2 bits

                int iCodeBlockSizePlus = (temp >> 10) & 16383; // 14 bits

                int iNumCodeBlockPlusData = (temp >> 23) & 3; // 2 bits
                int iNumCodeBlockPlus = iNumCodeBlockPlusData + 1;

                int iMaxHalfIterData = (temp >> 28) & 15; // 4 bits

                temp = _search_result_uint(result_record_stream_item,
                                           "Num Channel Bits");
                int iNumChannelBits = temp & 1023;           // 10 bits
                int iCwIdx = (temp >> 6) & 3;                // 2 bits
                int iLlrBufValid = (temp >> 9) & 3;          // 2 bits
                int iFirstDecodedCBIndex = (temp >> 11) & 7; // 2 bits

                temp = _search_result_uint(
                    result_record_stream_item,
                    "First Decoded CB Index Qed Iter2 Data");
                int iFirstDecodedCBIndexQedIter2Data = (temp >> 6) & 63;

                int iLastDecodedCBIndexQedIter2Data = (temp >> 12) & 63;

                int iCompandingFormat = (temp >> 18) & 31; // 4 bits

                temp = _search_result_uint(result_record_stream_item,
                                           "HARQ Combine Enable");
                int iHarqCombineEnable = (temp >> 1) & 7;

                _replace_result_int(result_record_stream_item,
                                    "Transport Block CRC", iTransportBlockCRC);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "Transport Block CRC",
                    ValueNamePassOrFail,
                    ARRAY_SIZE(ValueNamePassOrFail, ValueName), "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "NDI", iNDI);
                _replace_result_int(result_record_stream_item,
                                    "Retransmission Number",
                                    iRetransmissionNumber);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "Retransmission Number",
                    ValueNameNumber, ARRAY_SIZE(ValueNameNumber, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_stream_item, "RVID", iRVID);
                _replace_result_int(result_record_stream_item,
                                    "Code Block Size Plus", iCodeBlockSizePlus);
                _replace_result_int(result_record_stream_item,
                                    "Num Code Block Plus Data",
                                    iNumCodeBlockPlusData);
                _replace_result_int(result_record_stream_item,
                                    "Num Code Block Plus", iNumCodeBlockPlus);

                _replace_result_int(result_record_stream_item,
                                    "Max Half Iter Data", iMaxHalfIterData);

                _replace_result_int(result_record_stream_item,
                                    "Num Channel Bits", iNumChannelBits);

                _replace_result_int(result_record_stream_item, "CW Idx",
                                    iCwIdx);

                _replace_result_int(result_record_stream_item, "Llr Buf Valid",
                                    iLlrBufValid);

                _replace_result_int(result_record_stream_item,
                                    "First Decoded CB Index",
                                    iFirstDecodedCBIndex);

                _replace_result_int(result_record_stream_item,
                                    "First Decoded CB Index Qed Iter2 Data",
                                    iFirstDecodedCBIndexQedIter2Data);

                _replace_result_int(result_record_stream_item,
                                    "Last Decoded CB Index Qed Iter2 Data",
                                    iLastDecodedCBIndexQedIter2Data);

                _replace_result_int(result_record_stream_item,
                                    "Companding Format", iCompandingFormat);
                (void)_map_result_field_to_name(
                    result_record_stream_item, "Companding Format",
                    ValueNameCompandingStats,
                    ARRAY_SIZE(ValueNameCompandingStats, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_stream_item,
                                    "HARQ Combine Enable", iHarqCombineEnable);

                for (int k = 0; k < iNumCodeBlockPlus; k++) {
                    offset += 8;
                }

                int count_temp = 0;
                while ((unsigned int)(offset - start) < length) {
                    json result_hidden_record;
                    offset += _decode_by_fmt(
                        LtePhyPdschDecodingResult_Hidden_Energy_Metrics_v126,
                        ARRAY_SIZE(
                            LtePhyPdschDecodingResult_Hidden_Energy_Metrics_v126,
                            Fmt),
                        b, offset, length, result_hidden_record);

                    int first_half_temp =
                        _search_result_int(result_hidden_record,
                                           "Hidden Energy Metric First Half");
                    int second_half_temp =
                        _search_result_int(result_hidden_record,
                                           "Hidden Energy Metric Second Half");
                    if (first_half_temp != 0 or second_half_temp != 0) {
                        break;
                    }
                    count_temp = count_temp + 1;
                }

                offset = offset - 8;

                result_record_stream.push_back(result_record_stream_item);
            }

            result_record_item["Streams"] = result_record_stream;

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PHY PDSCH Decoding Result version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}
