/*
 * LTE PDSCH Stat Indication
 */

#include "consts.h"
#include "log_packet.h"
#include "log_packet_helper.h"

const Fmt LtePdschStatIndication_Fmt[] = {
    {UINT, "Version", 1},
};

const Fmt LtePdschStatIndication_Payload_v36[] = {
    {UINT, "Num Records", 1},
    {SKIP, NULL, 2},
};

const Fmt LtePdschStatIndication_Payload_v32[] = {
    {UINT, "Num Records", 1},
    {SKIP, NULL, 2},
};

const Fmt LtePdschStatIndication_Payload_v24[] = {
    {UINT, "Num Records", 1},
    {SKIP, NULL, 2},
};

const Fmt LtePdschStatIndication_Payload_v16[] = {
    {UINT, "Num Records", 1},
    {SKIP, NULL, 2},
};

const Fmt LtePdschStatIndication_Payload_v5[] = {
    {UINT, "Num Records", 1},
    {SKIP, NULL, 2},
};

const Fmt LtePdschStatIndication_Record_v36_P1[] = {
    {UINT, "Subframe Num", 2},
    {PLACEHOLDER, "Frame Num", 0},
    {UINT, "Num RBs", 1},
    {UINT, "Num Layers", 1},
    {UINT, "Num Transport Blocks Present", 1},
    {UINT, "Serving Cell Index", 1},
    {PLACEHOLDER, "HSIC Enabled", 0},
    {SKIP, NULL, 6}, // modified
};

const Fmt LtePdschStatIndication_Record_v32_P1[] = {
    {UINT, "Subframe Num", 2},
    {PLACEHOLDER, "Frame Num", 0},
    {UINT, "Num RBs", 1},
    {UINT, "Num Layers", 1},
    {UINT, "Num Transport Blocks Present", 1},
    {UINT, "Serving Cell Index", 1},
    {PLACEHOLDER, "HSIC Enabled", 0},
};

const Fmt LtePdschStatIndication_Record_v24_P1[] = {
    {UINT, "Subframe Num", 2},
    {PLACEHOLDER, "Frame Num", 0},
    {UINT, "Num RBs", 1},
    {UINT, "Num Layers", 1},
    {UINT, "Num Transport Blocks Present", 1},
    {UINT, "Serving Cell Index", 1},
    {PLACEHOLDER, "HSIC Enabled", 0},
};

const Fmt LtePdschStatIndication_Record_v16_P1[] = {
    {UINT, "Subframe Num", 2},
    {PLACEHOLDER, "Frame Num", 0},
    {UINT, "Num RBs", 1},
    {UINT, "Num Layers", 1},
    {UINT, "Num Transport Blocks Present", 1},
    {UINT, "Serving Cell Index", 1},
    {PLACEHOLDER, "HSIC Enabled", 0},
};

const Fmt LtePdschStatIndication_Record_v5_P1[] = {
    {UINT, "Subframe Num", 2},
    {PLACEHOLDER, "Frame Num", 0},
    {UINT, "Num RBs", 1},
    {UINT, "Num Layers", 1},
    {UINT, "Num Transport Blocks Present", 1},
    {UINT, "Serving Cell Index", 1},
};

const Fmt LtePdschStatIndication_Record_TB_v36[] = {
    {UINT, "HARQ ID", 1},                       // 4 bits
    {PLACEHOLDER, "RV", 0},                     // 2 bits
    {PLACEHOLDER, "NDI", 0},                    // 1 bit
    {PLACEHOLDER, "CRC Result", 0},             // 1 bit
    {UINT, "RNTI Type", 1},                     // 4 bits
    {PLACEHOLDER, "TB Index", 0},               // 1 bit
    {PLACEHOLDER, "Discarded reTx Present", 0}, // 1 bit
    {PLACEHOLDER, "Did Recombining", 0},        // 1 bits
    {SKIP, NULL, 2},
    {UINT, "TB Size", 2},
    {UINT, "MCS", 1},
    {UINT, "Num RBs", 1},
    {UINT, "Modulation Type", 1},
    {UINT, "QED2 Interim Status", 1}, // added 2 bit
    {UINT, "QED Iteration", 0},       // added 6 bit
    {SKIP, NULL, 2},                  // modified
    {PLACEHOLDER, "ACK/NACK Decision", 0},
};

const Fmt LtePdschStatIndication_Record_TB_v32[] = {
    {UINT, "HARQ ID", 1},                       // 4 bits
    {PLACEHOLDER, "RV", 0},                     // 2 bits
    {PLACEHOLDER, "NDI", 0},                    // 1 bit
    {PLACEHOLDER, "CRC Result", 0},             // 1 bit
    {UINT, "RNTI Type", 1},                     // 4 bits
    {PLACEHOLDER, "TB Index", 0},               // 1 bit
    {PLACEHOLDER, "Discarded reTx Present", 0}, // 1 bit
    {PLACEHOLDER, "Did Recombining", 0},        // 1 bits
    {UINT, "TB Size", 2},
    {UINT, "MCS", 1},
    {UINT, "Num RBs", 1},
    {UINT, "Modulation Type", 1},
    {SKIP, NULL, 1},
    {PLACEHOLDER, "ACK/NACK Decision", 0},
};

const Fmt LtePdschStatIndication_Record_TB_v24[] = {
    {UINT, "HARQ ID", 1},                       // 4 bits
    {PLACEHOLDER, "RV", 0},                     // 2 bits
    {PLACEHOLDER, "NDI", 0},                    // 1 bit
    {PLACEHOLDER, "CRC Result", 0},             // 1 bit
    {UINT, "RNTI Type", 1},                     // 4 bits
    {PLACEHOLDER, "TB Index", 0},               // 1 bit
    {PLACEHOLDER, "Discarded reTx Present", 0}, // 1 bit
    {PLACEHOLDER, "Did Recombining", 0},        // 1 bits
    {UINT, "TB Size", 2},
    {UINT, "MCS", 1},
    {UINT, "Num RBs", 1},
    {UINT, "Modulation Type", 1},
    {SKIP, NULL, 1},
    {PLACEHOLDER, "ACK/NACK Decision", 0},
};

const Fmt LtePdschStatIndication_Record_TB_v16[] = {
    {UINT, "HARQ ID", 1},                       // 4 bits
    {PLACEHOLDER, "RV", 0},                     // 2 bits
    {PLACEHOLDER, "NDI", 0},                    // 1 bit
    {PLACEHOLDER, "CRC Result", 0},             // 1 bit
    {UINT, "RNTI Type", 1},                     // 4 bits
    {PLACEHOLDER, "TB Index", 0},               // 1 bit
    {PLACEHOLDER, "Discarded reTx Present", 0}, // 1 bit
    {PLACEHOLDER, "Did Recombining", 0},        // 1 bits
    {UINT, "TB Size", 2},
    {UINT, "MCS", 1},
    {UINT, "Num RBs", 1},
    {PLACEHOLDER, "ACK/NACK Decision", 0},
};

const Fmt LtePdschStatIndication_Record_TB_v5[] = {
    {UINT, "HARQ ID", 1},                       // 4 bits
    {PLACEHOLDER, "RV", 0},                     // 2 bits
    {PLACEHOLDER, "NDI", 0},                    // 1 bit
    {PLACEHOLDER, "CRC Result", 0},             // 1 bit
    {UINT, "RNTI Type", 1},                     // 4 bits
    {PLACEHOLDER, "TB Index", 0},               // 1 bit
    {PLACEHOLDER, "Discarded reTx Present", 0}, // 1 bit
    {PLACEHOLDER, "Did Recombining", 0},        // 1 bits
    {UINT, "TB Size", 2},
    {UINT, "MCS", 1},
    {PLACEHOLDER, "Modulation Type", 0},
    {UINT, "Num RBs", 1},
    {PLACEHOLDER, "ACK/NACK Decision", 0},
};

const ValueName LtePdschStatIndication_Record_TB_Modulation[] = {
    {0, "QPSK"},
    {1, "16QAM"},
    {2, "64QAM"},
};

const ValueName LtePdschStatIndication_Record_TB_Modulation_v24[] = {
    {2, "QPSK"},
    {4, "16QAM"},
    {6, "64QAM"},
};

const ValueName LtePdschStatIndication_Record_TB_Modulation_v32[] = {
    {2, "QPSK"},
    {4, "16QAM"},
    {6, "64QAM"},
};

const ValueName LtePdschStatIndication_Record_TB_Modulation_v36[] = {
    {2, "QPSK"},
    {4, "16QAM"},
    {6, "64QAM"},
};

const ValueName LtePdschStatIndication_Record_TB_CrcResult[] = {
    {0, "Fail"},
    {1, "Pass"},
};
const ValueName LtePdschStatIndication_Record_TB_DiscardedReTxPresent[] = {
    {0, "None"},
    {1, "Present"},
};
const ValueName LtePdschStatIndication_Record_TB_DidRecombining[] = {
    {0, "No"},
    {1, "Yes"},
};
const ValueName LtePdschStatIndication_Record_TB_AckNackDecision[] = {
    {0, "NACK"},
    {1, "ACK"},
};

const Fmt LtePdschStatIndication_Record_v36_P2[] = {
    {UINT, "PMCH ID", 1},
    {UINT, "Area ID", 1},
    {SKIP, NULL, 2},
};

const Fmt LtePdschStatIndication_Record_v32_P2[] = {
    {UINT, "PMCH ID", 1},
    {UINT, "Area ID", 1},
};

const Fmt LtePdschStatIndication_Record_v24_P2[] = {
    {UINT, "PMCH ID", 1},
    {UINT, "Area ID", 1},
};

const Fmt LtePdschStatIndication_Record_v16_P2[] = {
    {UINT, "PMCH ID", 1},
    {UINT, "Area ID", 1},
};

const Fmt LtePdschStatIndication_Record_v5_P2[] = {
    {UINT, "PMCH ID", 1},
    {UINT, "Area ID", 1},
};

static int _decode_lte_pdsch_stat_indication_payload(const char *b, int offset,
                                                     size_t length,
                                                     json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 36: {
        offset +=
            _decode_by_fmt(LtePdschStatIndication_Payload_v36,
                           ARRAY_SIZE(LtePdschStatIndication_Payload_v36, Fmt),
                           b, offset, length, result);
        int num_record = _search_result_int(result, "Num Records");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v36_P1,
                ARRAY_SIZE(LtePdschStatIndication_Record_v36_P1, Fmt), b,
                offset, length, result_record_item);
            int iNonDecodeP1_1 =
                _search_result_int(result_record_item, "Subframe Num");
            int iSubFN = iNonDecodeP1_1 & 15;
            int iFN = (iNonDecodeP1_1 >> 4) & 4095;
            _replace_result_int(result_record_item, "Subframe Num", iSubFN);
            _replace_result_int(result_record_item, "Frame Num", iFN);
            int iNonDecodeP1_2 =
                _search_result_int(result_record_item, "Serving Cell Index");
            int iServCellIdx = iNonDecodeP1_2 & 7;         // last 3 bits
            int iHSICEnabled = (iNonDecodeP1_2 >> 3) & 15; // next 4 bits
            _replace_result_int(result_record_item, "Serving Cell Index",
                                iServCellIdx);
            (void)_map_result_field_to_name(
                result_record_item, "Serving Cell Index", ValueNameCellIndex,
                ARRAY_SIZE(ValueNameCellIndex, ValueName), "(MI)Unknown");
            _replace_result_int(result_record_item, "HSIC Enabled",
                                iHSICEnabled);
            (void)_map_result_field_to_name(
                result_record_item, "HSIC Enabled", ValueNameEnableOrDisable,
                ARRAY_SIZE(ValueNameEnableOrDisable, ValueName), "(MI)Unknown");
            int num_TB = _search_result_int(result_record_item,
                                            "Num Transport Blocks Present");
            json result_record_item_TB_list;
            for (int i = 0; i < num_TB; i++) {
                json result_record_item_TB_item;
                offset += _decode_by_fmt(
                    LtePdschStatIndication_Record_TB_v36,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_v36, Fmt), b,
                    offset, length, result_record_item_TB_item);
                int iNonDecodeP2_1 =
                    _search_result_int(result_record_item_TB_item, "HARQ ID");
                int iHarqId = iNonDecodeP2_1 & 15;          // last 4 bits
                int iRV = (iNonDecodeP2_1 >> 4) & 3;        // next 2 bits
                int iNDI = (iNonDecodeP2_1 >> 6) & 1;       // next 1 bit
                int iCrcResult = (iNonDecodeP2_1 >> 7) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "HARQ ID",
                                    iHarqId);
                _replace_result_int(result_record_item_TB_item, "RV", iRV);
                _replace_result_int(result_record_item_TB_item, "NDI", iNDI);
                _replace_result_int(result_record_item_TB_item, "CRC Result",
                                    iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "CRC Result",
                    LtePdschStatIndication_Record_TB_CrcResult,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_CrcResult,
                               ValueName),
                    "(MI)Unknown");
                int iNonDecodeP2_2 =
                    _search_result_int(result_record_item_TB_item, "RNTI Type");
                int iRNTI = iNonDecodeP2_2 & 15;        // last 4 bits;
                int iTbIdx = (iNonDecodeP2_2 >> 4) & 1; // next 1 bit
                int iDiscardedReTxPresent =
                    (iNonDecodeP2_2 >> 5) & 1;                   // next 1 bit
                int iDidRecombining = (iNonDecodeP2_2 >> 6) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "RNTI Type",
                                    iRNTI);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "RNTI Type", RNTIType,
                    ARRAY_SIZE(RNTIType, ValueName), "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item, "TB Index",
                                    iTbIdx);
                _replace_result_int(result_record_item_TB_item,
                                    "Discarded reTx Present",
                                    iDiscardedReTxPresent);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Discarded reTx Present",
                    LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                    ARRAY_SIZE(
                        LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                        ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "Did Recombining", iDidRecombining);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Did Recombining",
                    LtePdschStatIndication_Record_TB_DidRecombining,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_DidRecombining,
                               ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Modulation Type",
                    LtePdschStatIndication_Record_TB_Modulation_v36,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_Modulation_v36,
                               ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "ACK/NACK Decision", iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "ACK/NACK Decision",
                    LtePdschStatIndication_Record_TB_AckNackDecision,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_AckNackDecision,
                               ValueName),
                    "(MI)Unknown");

                int qedInterimStatus = _search_result_int(
                    result_record_item_TB_item, "QED2 Interim Status");

                int qedIterations = (qedInterimStatus >> 2) & 47;
                qedInterimStatus = qedInterimStatus & 3;

                _replace_result_int(result_record_item_TB_item,
                                    "QED2 Interim Status", qedInterimStatus);

                _replace_result_int(result_record_item_TB_item, "QED Iteration",
                                    qedIterations);

                result_record_item_TB_list.push_back(
                    result_record_item_TB_item);
            }
            result_record_item["Transport Blocks"] = result_record_item_TB_list;
            if (num_TB == 1) {
                offset += 12; // v32, TB is 8 bytes
            }
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v36_P2,
                ARRAY_SIZE(LtePdschStatIndication_Record_v36_P2, Fmt), b,
                offset, length, result_record_item);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    case 32: {
        offset +=
            _decode_by_fmt(LtePdschStatIndication_Payload_v32,
                           ARRAY_SIZE(LtePdschStatIndication_Payload_v32, Fmt),
                           b, offset, length, result);
        int num_record = _search_result_int(result, "Num Records");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v32_P1,
                ARRAY_SIZE(LtePdschStatIndication_Record_v32_P1, Fmt), b,
                offset, length, result_record_item);
            int iNonDecodeP1_1 =
                _search_result_int(result_record_item, "Subframe Num");
            int iSubFN = iNonDecodeP1_1 & 15;
            int iFN = (iNonDecodeP1_1 >> 4) & 4095;
            _replace_result_int(result_record_item, "Subframe Num", iSubFN);
            _replace_result_int(result_record_item, "Frame Num", iFN);
            int iNonDecodeP1_2 =
                _search_result_int(result_record_item, "Serving Cell Index");
            int iServCellIdx = iNonDecodeP1_2 & 7;         // last 3 bits
            int iHSICEnabled = (iNonDecodeP1_2 >> 3) & 15; // next 4 bits
            _replace_result_int(result_record_item, "Serving Cell Index",
                                iServCellIdx);
            (void)_map_result_field_to_name(
                result_record_item, "Serving Cell Index", ValueNameCellIndex,
                ARRAY_SIZE(ValueNameCellIndex, ValueName), "(MI)Unknown");
            _replace_result_int(result_record_item, "HSIC Enabled",
                                iHSICEnabled);
            (void)_map_result_field_to_name(
                result_record_item, "HSIC Enabled", ValueNameEnableOrDisable,
                ARRAY_SIZE(ValueNameEnableOrDisable, ValueName), "(MI)Unknown");
            int num_TB = _search_result_int(result_record_item,
                                            "Num Transport Blocks Present");
            json result_record_item_TB_list;
            for (int i = 0; i < num_TB; i++) {
                json result_record_item_TB_item;
                offset += _decode_by_fmt(
                    LtePdschStatIndication_Record_TB_v32,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_v32, Fmt), b,
                    offset, length, result_record_item_TB_item);
                int iNonDecodeP2_1 =
                    _search_result_int(result_record_item_TB_item, "HARQ ID");
                int iHarqId = iNonDecodeP2_1 & 15;          // last 4 bits
                int iRV = (iNonDecodeP2_1 >> 4) & 3;        // next 2 bits
                int iNDI = (iNonDecodeP2_1 >> 6) & 1;       // next 1 bit
                int iCrcResult = (iNonDecodeP2_1 >> 7) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "HARQ ID",
                                    iHarqId);
                _replace_result_int(result_record_item_TB_item, "RV", iRV);
                _replace_result_int(result_record_item_TB_item, "NDI", iNDI);
                _replace_result_int(result_record_item_TB_item, "CRC Result",
                                    iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "CRC Result",
                    LtePdschStatIndication_Record_TB_CrcResult,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_CrcResult,
                               ValueName),
                    "(MI)Unknown");
                int iNonDecodeP2_2 =
                    _search_result_int(result_record_item_TB_item, "RNTI Type");
                int iRNTI = iNonDecodeP2_2 & 15;        // last 4 bits;
                int iTbIdx = (iNonDecodeP2_2 >> 4) & 1; // next 1 bit
                int iDiscardedReTxPresent =
                    (iNonDecodeP2_2 >> 5) & 1;                   // next 1 bit
                int iDidRecombining = (iNonDecodeP2_2 >> 6) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "RNTI Type",
                                    iRNTI);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "RNTI Type", RNTIType,
                    ARRAY_SIZE(RNTIType, ValueName), "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item, "TB Index",
                                    iTbIdx);
                _replace_result_int(result_record_item_TB_item,
                                    "Discarded reTx Present",
                                    iDiscardedReTxPresent);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Discarded reTx Present",
                    LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                    ARRAY_SIZE(
                        LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                        ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "Did Recombining", iDidRecombining);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Did Recombining",
                    LtePdschStatIndication_Record_TB_DidRecombining,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_DidRecombining,
                               ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Modulation Type",
                    LtePdschStatIndication_Record_TB_Modulation_v32,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_Modulation_v32,
                               ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "ACK/NACK Decision", iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "ACK/NACK Decision",
                    LtePdschStatIndication_Record_TB_AckNackDecision,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_AckNackDecision,
                               ValueName),
                    "(MI)Unknown");
                result_record_item_TB_list.push_back(
                    result_record_item_TB_item);
            }
            result_record_item["Transport Blocks"] = result_record_item_TB_list;
            if (num_TB == 1) {
                offset += 8; // v32, TB is 8 bytes
            }
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v32_P2,
                ARRAY_SIZE(LtePdschStatIndication_Record_v32_P2, Fmt), b,
                offset, length, result_record_item);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    case 24: {
        offset +=
            _decode_by_fmt(LtePdschStatIndication_Payload_v24,
                           ARRAY_SIZE(LtePdschStatIndication_Payload_v24, Fmt),
                           b, offset, length, result);
        int num_record = _search_result_int(result, "Num Records");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v24_P1,
                ARRAY_SIZE(LtePdschStatIndication_Record_v24_P1, Fmt), b,
                offset, length, result_record_item);
            int iNonDecodeP1_1 =
                _search_result_int(result_record_item, "Subframe Num");
            int iSubFN = iNonDecodeP1_1 & 15;
            int iFN = (iNonDecodeP1_1 >> 4) & 4095;
            _replace_result_int(result_record_item, "Subframe Num", iSubFN);
            _replace_result_int(result_record_item, "Frame Num", iFN);
            int iNonDecodeP1_2 =
                _search_result_int(result_record_item, "Serving Cell Index");
            int iServCellIdx = iNonDecodeP1_2 & 7;         // last 3 bits
            int iHSICEnabled = (iNonDecodeP1_2 >> 3) & 15; // next 4 bits
            _replace_result_int(result_record_item, "Serving Cell Index",
                                iServCellIdx);
            (void)_map_result_field_to_name(
                result_record_item, "Serving Cell Index", ValueNameCellIndex,
                ARRAY_SIZE(ValueNameCellIndex, ValueName), "(MI)Unknown");
            _replace_result_int(result_record_item, "HSIC Enabled",
                                iHSICEnabled);
            (void)_map_result_field_to_name(
                result_record_item, "HSIC Enabled", ValueNameEnableOrDisable,
                ARRAY_SIZE(ValueNameEnableOrDisable, ValueName), "(MI)Unknown");
            int num_TB = _search_result_int(result_record_item,
                                            "Num Transport Blocks Present");
            json result_record_item_TB_list;
            for (int i = 0; i < num_TB; i++) {
                json result_record_item_TB_item;
                offset += _decode_by_fmt(
                    LtePdschStatIndication_Record_TB_v24,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_v24, Fmt), b,
                    offset, length, result_record_item_TB_item);
                int iNonDecodeP2_1 =
                    _search_result_int(result_record_item_TB_item, "HARQ ID");
                int iHarqId = iNonDecodeP2_1 & 15;          // last 4 bits
                int iRV = (iNonDecodeP2_1 >> 4) & 3;        // next 2 bits
                int iNDI = (iNonDecodeP2_1 >> 6) & 1;       // next 1 bit
                int iCrcResult = (iNonDecodeP2_1 >> 7) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "HARQ ID",
                                    iHarqId);
                _replace_result_int(result_record_item_TB_item, "RV", iRV);
                _replace_result_int(result_record_item_TB_item, "NDI", iNDI);
                _replace_result_int(result_record_item_TB_item, "CRC Result",
                                    iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "CRC Result",
                    LtePdschStatIndication_Record_TB_CrcResult,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_CrcResult,
                               ValueName),
                    "(MI)Unknown");
                int iNonDecodeP2_2 =
                    _search_result_int(result_record_item_TB_item, "RNTI Type");
                int iRNTI = iNonDecodeP2_2 & 15;        // last 4 bits;
                int iTbIdx = (iNonDecodeP2_2 >> 4) & 1; // next 1 bit
                int iDiscardedReTxPresent =
                    (iNonDecodeP2_2 >> 5) & 1;                   // next 1 bit
                int iDidRecombining = (iNonDecodeP2_2 >> 6) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "RNTI Type",
                                    iRNTI);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "RNTI Type", RNTIType,
                    ARRAY_SIZE(RNTIType, ValueName), "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item, "TB Index",
                                    iTbIdx);
                _replace_result_int(result_record_item_TB_item,
                                    "Discarded reTx Present",
                                    iDiscardedReTxPresent);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Discarded reTx Present",
                    LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                    ARRAY_SIZE(
                        LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                        ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "Did Recombining", iDidRecombining);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Did Recombining",
                    LtePdschStatIndication_Record_TB_DidRecombining,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_DidRecombining,
                               ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Modulation Type",
                    LtePdschStatIndication_Record_TB_Modulation_v24,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_Modulation_v24,
                               ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "ACK/NACK Decision", iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "ACK/NACK Decision",
                    LtePdschStatIndication_Record_TB_AckNackDecision,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_AckNackDecision,
                               ValueName),
                    "(MI)Unknown");
                result_record_item_TB_list.push_back(
                    result_record_item_TB_item);
            }
            result_record_item["Transport Blocks"] = result_record_item_TB_list;
            if (num_TB == 1) {
                offset += 8; // v24, TB is 8 bytes
            }
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v24_P2,
                ARRAY_SIZE(LtePdschStatIndication_Record_v24_P2, Fmt), b,
                offset, length, result_record_item);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }

    case 16: {
        offset +=
            _decode_by_fmt(LtePdschStatIndication_Payload_v16,
                           ARRAY_SIZE(LtePdschStatIndication_Payload_v16, Fmt),
                           b, offset, length, result);
        int num_record = _search_result_int(result, "Num Records");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v16_P1,
                ARRAY_SIZE(LtePdschStatIndication_Record_v16_P1, Fmt), b,
                offset, length, result_record_item);
            int iNonDecodeP1_1 =
                _search_result_int(result_record_item, "Subframe Num");
            int iSubFN = iNonDecodeP1_1 & 15;
            int iFN = (iNonDecodeP1_1 >> 4) & 4095;
            _replace_result_int(result_record_item, "Subframe Num", iSubFN);
            _replace_result_int(result_record_item, "Frame Num", iFN);
            int iNonDecodeP1_2 =
                _search_result_int(result_record_item, "Serving Cell Index");
            int iServCellIdx = iNonDecodeP1_2 & 7;         // last 3 bits
            int iHSICEnabled = (iNonDecodeP1_2 >> 3) & 15; // next 4 bits
            _replace_result_int(result_record_item, "Serving Cell Index",
                                iServCellIdx);
            (void)_map_result_field_to_name(
                result_record_item, "Serving Cell Index", ValueNameCellIndex,
                ARRAY_SIZE(ValueNameCellIndex, ValueName), "(MI)Unknown");
            _replace_result_int(result_record_item, "HSIC Enabled",
                                iHSICEnabled);
            (void)_map_result_field_to_name(
                result_record_item, "HSIC Enabled", ValueNameEnableOrDisable,
                ARRAY_SIZE(ValueNameEnableOrDisable, ValueName), "(MI)Unknown");
            int num_TB = _search_result_int(result_record_item,
                                            "Num Transport Blocks Present");
            json result_record_item_TB_list;
            for (int i = 0; i < num_TB; i++) {
                json result_record_item_TB_item;
                offset += _decode_by_fmt(
                    LtePdschStatIndication_Record_TB_v16,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_v16, Fmt), b,
                    offset, length, result_record_item_TB_item);
                int iNonDecodeP2_1 =
                    _search_result_int(result_record_item_TB_item, "HARQ ID");
                int iHarqId = iNonDecodeP2_1 & 15;          // last 4 bits
                int iRV = (iNonDecodeP2_1 >> 4) & 3;        // next 2 bits
                int iNDI = (iNonDecodeP2_1 >> 6) & 1;       // next 1 bit
                int iCrcResult = (iNonDecodeP2_1 >> 7) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "HARQ ID",
                                    iHarqId);
                _replace_result_int(result_record_item_TB_item, "RV", iRV);
                _replace_result_int(result_record_item_TB_item, "NDI", iNDI);
                _replace_result_int(result_record_item_TB_item, "CRC Result",
                                    iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "CRC Result",
                    LtePdschStatIndication_Record_TB_CrcResult,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_CrcResult,
                               ValueName),
                    "(MI)Unknown");
                int iNonDecodeP2_2 =
                    _search_result_int(result_record_item_TB_item, "RNTI Type");
                int iRNTI = iNonDecodeP2_2 & 15;        // last 4 bits;
                int iTbIdx = (iNonDecodeP2_2 >> 4) & 1; // next 1 bit
                int iDiscardedReTxPresent =
                    (iNonDecodeP2_2 >> 5) & 1;                   // next 1 bit
                int iDidRecombining = (iNonDecodeP2_2 >> 6) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "RNTI Type",
                                    iRNTI);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "RNTI Type", RNTIType,
                    ARRAY_SIZE(RNTIType, ValueName), "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item, "TB Index",
                                    iTbIdx);
                _replace_result_int(result_record_item_TB_item,
                                    "Discarded reTx Present",
                                    iDiscardedReTxPresent);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Discarded reTx Present",
                    LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                    ARRAY_SIZE(
                        LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                        ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "Did Recombining", iDidRecombining);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Did Recombining",
                    LtePdschStatIndication_Record_TB_DidRecombining,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_DidRecombining,
                               ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "ACK/NACK Decision", iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "ACK/NACK Decision",
                    LtePdschStatIndication_Record_TB_AckNackDecision,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_AckNackDecision,
                               ValueName),
                    "(MI)Unknown");
                result_record_item_TB_list.push_back(
                    result_record_item_TB_item);
            }
            result_record_item["Transport Blocks"] = result_record_item_TB_list;
            if (num_TB == 1) {
                offset += 6;
            }
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v16_P2,
                ARRAY_SIZE(LtePdschStatIndication_Record_v16_P2, Fmt), b,
                offset, length, result_record_item);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    case 5: {
        offset +=
            _decode_by_fmt(LtePdschStatIndication_Payload_v5,
                           ARRAY_SIZE(LtePdschStatIndication_Payload_v5, Fmt),
                           b, offset, length, result);
        int num_record = _search_result_int(result, "Num Records");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v5_P1,
                ARRAY_SIZE(LtePdschStatIndication_Record_v5_P1, Fmt), b, offset,
                length, result_record_item);
            int iNonDecodeP1_1 =
                _search_result_int(result_record_item, "Subframe Num");
            int iSubFN = iNonDecodeP1_1 & 15;
            int iFN = (iNonDecodeP1_1 >> 4) & 4095;
            _replace_result_int(result_record_item, "Subframe Num", iSubFN);
            _replace_result_int(result_record_item, "Frame Num", iFN);
            int iNonDecodeP1_2 =
                _search_result_int(result_record_item, "Serving Cell Index");
            int iServCellIdx = iNonDecodeP1_2 & 7; // last 3 bits
            _replace_result_int(result_record_item, "Serving Cell Index",
                                iServCellIdx);
            (void)_map_result_field_to_name(
                result_record_item, "Serving Cell Index", ValueNameCellIndex,
                ARRAY_SIZE(ValueNameCellIndex, ValueName), "(MI)Unknown");
            int num_TB = _search_result_int(result_record_item,
                                            "Num Transport Blocks Present");
            json result_record_item_TB_list;
            for (int i = 0; i < num_TB; i++) {
                json result_record_item_TB_item;
                offset += _decode_by_fmt(
                    LtePdschStatIndication_Record_TB_v5,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_v5, Fmt), b,
                    offset, length, result_record_item_TB_item);
                int iNonDecodeP2_1 =
                    _search_result_int(result_record_item_TB_item, "HARQ ID");
                int iHarqId = iNonDecodeP2_1 & 15;          // last 4 bits
                int iRV = (iNonDecodeP2_1 >> 4) & 3;        // next 2 bits
                int iNDI = (iNonDecodeP2_1 >> 6) & 1;       // next 1 bit
                int iCrcResult = (iNonDecodeP2_1 >> 7) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "HARQ ID",
                                    iHarqId);
                _replace_result_int(result_record_item_TB_item, "RV", iRV);
                _replace_result_int(result_record_item_TB_item, "NDI", iNDI);
                _replace_result_int(result_record_item_TB_item, "CRC Result",
                                    iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "CRC Result",
                    LtePdschStatIndication_Record_TB_CrcResult,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_CrcResult,
                               ValueName),
                    "(MI)Unknown");
                int iNonDecodeP2_2 =
                    _search_result_int(result_record_item_TB_item, "RNTI Type");
                int iRNTI = iNonDecodeP2_2 & 15;        // last 4 bits;
                int iTbIdx = (iNonDecodeP2_2 >> 4) & 1; // next 1 bit
                int iDiscardedReTxPresent =
                    (iNonDecodeP2_2 >> 5) & 1;                   // next 1 bit
                int iDidRecombining = (iNonDecodeP2_2 >> 6) & 1; // next 1 bit
                _replace_result_int(result_record_item_TB_item, "RNTI Type",
                                    iRNTI);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "RNTI Type", RNTIType,
                    ARRAY_SIZE(RNTIType, ValueName), "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item, "TB Index",
                                    iTbIdx);
                _replace_result_int(result_record_item_TB_item,
                                    "Discarded reTx Present",
                                    iDiscardedReTxPresent);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Discarded reTx Present",
                    LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                    ARRAY_SIZE(
                        LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                        ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "Did Recombining", iDidRecombining);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Did Recombining",
                    LtePdschStatIndication_Record_TB_DidRecombining,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_DidRecombining,
                               ValueName),
                    "(MI)Unknown");
                int iMCS =
                    _search_result_int(result_record_item_TB_item, "MCS");
                int iModulationType = -1;
                if (iMCS > 17) {
                    iModulationType = 2;
                } else if (iMCS > 10 && iMCS < 17) {
                    iModulationType = 1;
                } else if (iMCS < 10) {
                    iModulationType = 0;
                }
                _replace_result_int(result_record_item_TB_item,
                                    "Modulation Type", iModulationType);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "Modulation Type",
                    LtePdschStatIndication_Record_TB_Modulation,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_Modulation,
                               ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_record_item_TB_item,
                                    "ACK/NACK Decision", iCrcResult);
                (void)_map_result_field_to_name(
                    result_record_item_TB_item, "ACK/NACK Decision",
                    LtePdschStatIndication_Record_TB_AckNackDecision,
                    ARRAY_SIZE(LtePdschStatIndication_Record_TB_AckNackDecision,
                               ValueName),
                    "(MI)Unknown");
                result_record_item_TB_list.push_back(
                    result_record_item_TB_item);
            }
            result_record_item["Transport Blocks"] = result_record_item_TB_list;
            if (num_TB == 1) {
                offset += 6;
            }
            offset += _decode_by_fmt(
                LtePdschStatIndication_Record_v5_P2,
                ARRAY_SIZE(LtePdschStatIndication_Record_v5_P2, Fmt), b, offset,
                length, result_record_item);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }

    default:
        printf("(MI)Unknown LTE PDSCH Stat Indication version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}
