/*
 * GSM Surround Cell BA List
 */

#include "consts.h"
#include "log_packet.h"
#include "log_packet_helper.h"

const Fmt GsmScbl_Fmt[] = {
    {UINT, "Cell Count", 1},
};

const Fmt GsmScbl_Cell[] = {
    {UINT, "BCCH ARFCN", 2},       // 12 bits
    {PLACEHOLDER, "BCCH Band", 0}, // 4 bits
    {UINT, "RX Power", 2},         // (x - 65536) / 16.0
    {UINT, "BSIC Known", 1},
    {UINT, "BSIC-BCC", 1},        // 3 bits
    {PLACEHOLDER, "BSIC-NCC", 1}, // 3 bits
    {UINT, "Frame Number Offset", 4},
    {UINT, "Time Offset", 2},
};

static int _decode_gsm_scbl_payload(const char *b, int offset, size_t length,
                                    json &result) {
    int start = offset;
    int num_record = _search_result_int(result, "Cell Count");

    double pyfloat;
    int temp;

    json result_record;
    for (int i = 0; i < num_record; i++) {
        json result_record_item;
        offset += _decode_by_fmt(GsmScbl_Cell, ARRAY_SIZE(GsmScbl_Cell, Fmt), b,
                                 offset, length, result_record_item);

        (void)_map_result_field_to_name(
            result_record_item, "BSIC Known", ValueNameTrueOrFalse,
            ARRAY_SIZE(ValueNameTrueOrFalse, ValueName), "(MI)Unknown");

        temp = _search_result_int(result_record_item, "BCCH ARFCN");
        int iArfcn = temp & 4095;      // 12 bits
        int iBand = (temp >> 12) & 15; // 4 bits
        _replace_result_int(result_record_item, "BCCH ARFCN", iArfcn);
        _replace_result_int(result_record_item, "BCCH Band", iBand);
        (void)_map_result_field_to_name(
            result_record_item, "BCCH Band", ValueNameBandClassGSM,
            ARRAY_SIZE(ValueNameBandClassGSM, ValueName), "(MI)Unknown");

        temp = _search_result_int(result_record_item, "RX Power");
        float fRxPower = (temp - 65536) / 16.0;
        pyfloat = fRxPower;
        _replace_result(result_record_item, "RX Power", pyfloat);

        temp = _search_result_int(result_record_item, "BSIC-BCC");
        int iBCC = temp & 7;        // 3 bits
        int iNCC = (temp >> 3) & 7; // 3 bits
        _replace_result_int(result_record_item, "BSIC-BCC", iBCC);
        _replace_result_int(result_record_item, "BSIC-NCC", iNCC);

        result_record.push_back(result_record_item);
    }
    result["Surrounding Cells"] = result_record;
    return offset - start;
}
