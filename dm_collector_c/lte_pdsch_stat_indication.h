/*
 * LTE PDSCH Stat Indication
 */

#include "consts.h"
#include "log_packet.h"
#include "log_packet_helper.h"

const Fmt LtePdschStatIndication_Fmt [] = {
    {UINT, "Version", 1},
};

const Fmt LtePdschStatIndication_Payload_v16 [] = {
    {UINT, "Num Records", 1},
    {SKIP, NULL, 2},
};

const Fmt LtePdschStatIndication_Payload_v5 [] = {
    {UINT, "Num Records", 1},
    {SKIP, NULL, 2},
};

const Fmt LtePdschStatIndication_Record_v16_P1 [] = {
    {UINT, "Subframe Num", 2},
    {PLACEHOLDER, "Frame Num", 0},
    {UINT, "Num RBs", 1},
    {UINT, "Num Layers", 1},
    {UINT, "Num Transport Blocks Present", 1},
    {UINT, "Serving Cell Index", 1},
    {PLACEHOLDER, "HSIC Enabled", 0},
};

const Fmt LtePdschStatIndication_Record_v5_P1 [] = {
    {UINT, "Subframe Num", 2},
    {PLACEHOLDER, "Frame Num", 0},
    {UINT, "Num RBs", 1},
    {UINT, "Num Layers", 1},
    {UINT, "Num Transport Blocks Present", 1},
    {UINT, "Serving Cell Index", 1},
};

const ValueName LtePdschStatIndication_Record_ServingCellIndex [] = {
    {0, "PCell"},
};

const Fmt LtePdschStatIndication_Record_TB_v16 [] = {
    {UINT, "HARQ ID", 1},   // 4 bits
    {PLACEHOLDER, "RV", 0}, // 2 bits
    {PLACEHOLDER, "NDI", 0},    // 1 bit
    {PLACEHOLDER, "CRC Result", 0}, // 1 bit
    {UINT, "RNTI Type", 1}, // 4 bits
    {PLACEHOLDER, "TB Index", 0}, // 1 bit
    {PLACEHOLDER, "Discarded reTx Present", 0}, // 1 bit
    {PLACEHOLDER, "Did Recombining", 0}, // 1 bits
    {UINT, "TB Size", 2},
    {UINT, "MCS", 1},
    {UINT, "Num RBs", 1},
    {PLACEHOLDER, "ACK/NACK Decision", 0},
};
const Fmt LtePdschStatIndication_Record_TB_v5 [] = {
    {UINT, "HARQ ID", 1},   // 4 bits
    {PLACEHOLDER, "RV", 0}, // 2 bits
    {PLACEHOLDER, "NDI", 0},    // 1 bit
    {PLACEHOLDER, "CRC Result", 0}, // 1 bit
    {UINT, "RNTI Type", 1}, // 4 bits
    {PLACEHOLDER, "TB Index", 0}, // 1 bit
    {PLACEHOLDER, "Discarded reTx Present", 0}, // 1 bit
    {PLACEHOLDER, "Did Recombining", 0}, // 1 bits
    {UINT, "TB Size", 2},
    {UINT, "MCS", 1},
    {PLACEHOLDER, "Modulation Type", 0},
    {UINT, "Num RBs", 1},
    {PLACEHOLDER, "ACK/NACK Decision", 0},
};
const ValueName LtePdschStatIndication_Record_TB_Modulation [] = {
    {0, "QPSK"},
    {1, "16QAM"},
    {2, "64QAM"},
};

const ValueName LtePdschStatIndication_Record_TB_CrcResult [] = {
    {0, "Fail"},
    {1, "Pass"},
};
const ValueName LtePdschStatIndication_Record_TB_DiscardedReTxPresent [] = {
    {0, "None"},
    {1, "Present"},
};
const ValueName LtePdschStatIndication_Record_TB_DidRecombining [] = {
    {0, "No"},
    {1, "Yes"},
};
const ValueName LtePdschStatIndication_Record_TB_AckNackDecision [] = {
    {0, "NACK"},
    {1, "ACK"},
};

const Fmt LtePdschStatIndication_Record_v16_P2 [] = {
    {UINT, "PMCH ID", 1},
    {UINT, "Area ID", 1},
};
const Fmt LtePdschStatIndication_Record_v5_P2 [] = {
    {UINT, "PMCH ID", 1},
    {UINT, "Area ID", 1},
};


static int _decode_lte_pdsch_stat_indication_payload (const char *b,
        int offset, size_t length, PyObject *result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 16:
        {
            offset += _decode_by_fmt(LtePdschStatIndication_Payload_v16,
                    ARRAY_SIZE(LtePdschStatIndication_Payload_v16, Fmt),
                    b, offset, length, result);
            int num_record = _search_result_int(result, "Num Records");

            PyObject *result_record = PyList_New(0);
            for (int i = 0; i < num_record; i++) {
                PyObject *result_record_item = PyList_New(0);
                offset += _decode_by_fmt(LtePdschStatIndication_Record_v16_P1,
                        ARRAY_SIZE(LtePdschStatIndication_Record_v16_P1, Fmt),
                        b, offset, length, result_record_item);
                int iNonDecodeP1_1 = _search_result_int(result_record_item,
                        "Subframe Num");
                int iSubFN = iNonDecodeP1_1 & 15;
                int iFN = (iNonDecodeP1_1 >> 4) & 4095;
                PyObject *old_object = _replace_result_int(result_record_item,
                        "Subframe Num", iSubFN);
                Py_DECREF(old_object);
                old_object = _replace_result_int(result_record_item,
                        "Frame Num", iFN);
                Py_DECREF(old_object);
                int iNonDecodeP1_2 = _search_result_int(result_record_item,
                        "Serving Cell Index");
                int iServCellIdx = iNonDecodeP1_2 & 7; // last 3 bits
                int iHSICEnabled = (iNonDecodeP1_2 >> 3) & 15; // next 4 bits
                old_object = _replace_result_int(result_record_item,
                        "Serving Cell Index", iServCellIdx);
                Py_DECREF(old_object);
                (void) _map_result_field_to_name(result_record_item,
                        "Serving Cell Index",
                        LtePdschStatIndication_Record_ServingCellIndex,
                        ARRAY_SIZE(LtePdschStatIndication_Record_ServingCellIndex,
                            ValueName),
                        "Unknown");
                old_object = _replace_result_int(result_record_item,
                        "HSIC Enabled", iHSICEnabled);
                Py_DECREF(old_object);
                int num_TB = _search_result_int(result_record_item,
                        "Num Transport Blocks Present");
                PyObject *result_record_item_TB_list = PyList_New(0);
                for (int i = 0; i < num_TB; i++) {
                    PyObject *result_record_item_TB_item = PyList_New(0);
                    offset += _decode_by_fmt(LtePdschStatIndication_Record_TB_v16,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_v16, Fmt),
                            b, offset, length, result_record_item_TB_item);
                    int iNonDecodeP2_1 = _search_result_int(
                            result_record_item_TB_item, "HARQ ID");
                    int iHarqId = iNonDecodeP2_1 & 15; // last 4 bits
                    int iRV = (iNonDecodeP2_1 >> 4) & 3; // next 2 bits
                    int iNDI = (iNonDecodeP2_1 >> 6) & 1; // next 1 bit
                    int iCrcResult = (iNonDecodeP2_1 >> 7) & 1; // next 1 bit
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "HARQ ID", iHarqId);
                    Py_DECREF(old_object);
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "RV", iRV);
                    Py_DECREF(old_object);
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "NDI", iNDI);
                    Py_DECREF(old_object);
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "CRC Result", iCrcResult);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "CRC Result",
                            LtePdschStatIndication_Record_TB_CrcResult,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_CrcResult,
                                ValueName),
                            "Unknown");
                    int iNonDecodeP2_2 = _search_result_int(result_record_item_TB_item,
                            "RNTI Type");
                    int iRNTI = iNonDecodeP2_2 & 15; // last 4 bits;
                    int iTbIdx = (iNonDecodeP2_2 >> 4) & 1; // next 1 bit
                    int iDiscardedReTxPresent = (iNonDecodeP2_2 >> 5) & 1; // next 1 bit
                    int iDidRecombining = (iNonDecodeP2_2 >> 6) & 1; // next 1 bit
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "RNTI Type", iRNTI);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "RNTI Type",
                            RNTIType,
                            ARRAY_SIZE(RNTIType, ValueName),
                            "Unknown");
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "TB Index", iTbIdx);
                    Py_DECREF(old_object);
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "Discarded reTx Present", iDiscardedReTxPresent);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "Discarded reTx Present",
                            LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                                ValueName),
                            "Unknown");
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "Did Recombining", iDidRecombining);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "Did Recombining",
                            LtePdschStatIndication_Record_TB_DidRecombining,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_DidRecombining,
                                ValueName),
                            "Unknown");
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "ACK/NACK Decision", iCrcResult);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "ACK/NACK Decision",
                            LtePdschStatIndication_Record_TB_AckNackDecision,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_AckNackDecision,
                                ValueName),
                            "Unknown");
                    PyObject *t3 = Py_BuildValue("(sOs)", "Ignored",
                            result_record_item_TB_item, "dict");
                    PyList_Append(result_record_item_TB_list, t3);
                    Py_DECREF(t3);
                    Py_DECREF(result_record_item_TB_item);
                }
                PyObject *t2 = Py_BuildValue("(sOs)", "Transport Blocks",
                        result_record_item_TB_list, "list");
                PyList_Append(result_record_item, t2);
                Py_DECREF(t2);
                Py_DECREF(result_record_item_TB_list);
                if (num_TB == 1) {
                    offset += 6;
                }
                offset += _decode_by_fmt(LtePdschStatIndication_Record_v16_P2,
                        ARRAY_SIZE(LtePdschStatIndication_Record_v16_P2, Fmt),
                        b, offset, length, result_record_item);

                PyObject *t1 = Py_BuildValue("(sOs)", "Ignored",
                        result_record_item, "dict");
                PyList_Append(result_record, t1);
                Py_DECREF(t1);
                Py_DECREF(result_record_item);
            }
            PyObject *t = Py_BuildValue("(sOs)", "Records",
                    result_record, "list");
            PyList_Append(result, t);
            Py_DECREF(t);
            Py_DECREF(result_record);
            return offset - start;
        }
    case 5:
        {
            offset += _decode_by_fmt(LtePdschStatIndication_Payload_v5,
                    ARRAY_SIZE(LtePdschStatIndication_Payload_v5, Fmt),
                    b, offset, length, result);
            int num_record = _search_result_int(result, "Num Records");

            PyObject *result_record = PyList_New(0);
            for (int i = 0; i < num_record; i++) {
                PyObject *result_record_item = PyList_New(0);
                offset += _decode_by_fmt(LtePdschStatIndication_Record_v5_P1,
                        ARRAY_SIZE(LtePdschStatIndication_Record_v5_P1, Fmt),
                        b, offset, length, result_record_item);
                int iNonDecodeP1_1 = _search_result_int(result_record_item,
                        "Subframe Num");
                int iSubFN = iNonDecodeP1_1 & 15;
                int iFN = (iNonDecodeP1_1 >> 4) & 4095;
                PyObject *old_object = _replace_result_int(result_record_item,
                        "Subframe Num", iSubFN);
                Py_DECREF(old_object);
                old_object = _replace_result_int(result_record_item,
                        "Frame Num", iFN);
                Py_DECREF(old_object);
                int iNonDecodeP1_2 = _search_result_int(result_record_item,
                        "Serving Cell Index");
                int iServCellIdx = iNonDecodeP1_2 & 7; // last 3 bits
                old_object = _replace_result_int(result_record_item,
                        "Serving Cell Index", iServCellIdx);
                Py_DECREF(old_object);
                (void) _map_result_field_to_name(result_record_item,
                        "Serving Cell Index",
                        LtePdschStatIndication_Record_ServingCellIndex,
                        ARRAY_SIZE(LtePdschStatIndication_Record_ServingCellIndex,
                            ValueName),
                        "Unknown");
                int num_TB = _search_result_int(result_record_item,
                        "Num Transport Blocks Present");
                PyObject *result_record_item_TB_list = PyList_New(0);
                for (int i = 0; i < num_TB; i++) {
                    PyObject *result_record_item_TB_item = PyList_New(0);
                    offset += _decode_by_fmt(LtePdschStatIndication_Record_TB_v5,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_v5, Fmt),
                            b, offset, length, result_record_item_TB_item);
                    int iNonDecodeP2_1 = _search_result_int(
                            result_record_item_TB_item, "HARQ ID");
                    int iHarqId = iNonDecodeP2_1 & 15; // last 4 bits
                    int iRV = (iNonDecodeP2_1 >> 4) & 3; // next 2 bits
                    int iNDI = (iNonDecodeP2_1 >> 6) & 1; // next 1 bit
                    int iCrcResult = (iNonDecodeP2_1 >> 7) & 1; // next 1 bit
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "HARQ ID", iHarqId);
                    Py_DECREF(old_object);
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "RV", iRV);
                    Py_DECREF(old_object);
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "NDI", iNDI);
                    Py_DECREF(old_object);
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "CRC Result", iCrcResult);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "CRC Result",
                            LtePdschStatIndication_Record_TB_CrcResult,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_CrcResult,
                                ValueName),
                            "Unknown");
                    int iNonDecodeP2_2 = _search_result_int(result_record_item_TB_item,
                            "RNTI Type");
                    int iRNTI = iNonDecodeP2_2 & 15; // last 4 bits;
                    int iTbIdx = (iNonDecodeP2_2 >> 4) & 1; // next 1 bit
                    int iDiscardedReTxPresent = (iNonDecodeP2_2 >> 5) & 1; // next 1 bit
                    int iDidRecombining = (iNonDecodeP2_2 >> 6) & 1; // next 1 bit
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "RNTI Type", iRNTI);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "RNTI Type",
                            RNTIType,
                            ARRAY_SIZE(RNTIType, ValueName),
                            "Unknown");
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "TB Index", iTbIdx);
                    Py_DECREF(old_object);
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "Discarded reTx Present", iDiscardedReTxPresent);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "Discarded reTx Present",
                            LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_DiscardedReTxPresent,
                                ValueName),
                            "Unknown");
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "Did Recombining", iDidRecombining);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "Did Recombining",
                            LtePdschStatIndication_Record_TB_DidRecombining,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_DidRecombining,
                                ValueName),
                            "Unknown");
                    int iMCS = _search_result_int(result_record_item_TB_item,
                            "MCS");
                    int iModulationType = -1;
                    if (iMCS > 17) {
                        iModulationType = 2;
                    } else if (iMCS > 10 && iMCS < 17) {
                        iModulationType = 1;
                    } else if (iMCS < 10) {
                        iModulationType = 0;
                    }
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "Modulation Type", iModulationType);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "Modulation Type",
                            LtePdschStatIndication_Record_TB_Modulation,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_Modulation,
                                ValueName),
                            "Unknown");
                    old_object = _replace_result_int(result_record_item_TB_item,
                            "ACK/NACK Decision", iCrcResult);
                    Py_DECREF(old_object);
                    (void) _map_result_field_to_name(result_record_item_TB_item,
                            "ACK/NACK Decision",
                            LtePdschStatIndication_Record_TB_AckNackDecision,
                            ARRAY_SIZE(LtePdschStatIndication_Record_TB_AckNackDecision,
                                ValueName),
                            "Unknown");
                    PyObject *t3 = Py_BuildValue("(sOs)", "Ignored",
                            result_record_item_TB_item, "dict");
                    PyList_Append(result_record_item_TB_list, t3);
                    Py_DECREF(t3);
                    Py_DECREF(result_record_item_TB_item);
                }
                PyObject *t2 = Py_BuildValue("(sOs)", "Transport Blocks",
                        result_record_item_TB_list, "list");
                PyList_Append(result_record_item, t2);
                Py_DECREF(t2);
                Py_DECREF(result_record_item_TB_list);
                if (num_TB == 1) {
                    offset += 6;
                }
                offset += _decode_by_fmt(LtePdschStatIndication_Record_v5_P2,
                        ARRAY_SIZE(LtePdschStatIndication_Record_v5_P2, Fmt),
                        b, offset, length, result_record_item);

                PyObject *t1 = Py_BuildValue("(sOs)", "Ignored",
                        result_record_item, "dict");
                PyList_Append(result_record, t1);
                Py_DECREF(t1);
                Py_DECREF(result_record_item);
            }
            PyObject *t = Py_BuildValue("(sOs)", "Records",
                    result_record, "list");
            PyList_Append(result, t);
            Py_DECREF(t);
            Py_DECREF(result_record);
            return offset - start;
        }

    default:
        printf("Unknown LTE PDSCH Stat Indication version: 0x%x\n", pkt_ver);
        return 0;
    }
}