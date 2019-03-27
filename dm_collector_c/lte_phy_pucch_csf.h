/*
 * LTE PHY PUCCH CSF
 */

#include "consts.h"
#include "log_packet.h"
#include "log_packet_helper.h"

const Fmt LtePhyPucchCsf_Fmt[] = {
    {UINT, "Version", 1},
};

const Fmt LtePhyPucchCsf_Payload_v43[] = {
    {UINT, "Start System Sub-frame Number", 2},    // 4 bits
    {PLACEHOLDER, "Start System Frame Number", 0}, // 10 bits
    {UINT, "PUCCH Reporting Mode", 1}, // last 2 bits in previous byte + 1 bit
    {PLACEHOLDER, "PUCCH Report Type", 0}, // 4 bit
    {UINT, "Number of Subbands", 4},       // 4 bits
    {PLACEHOLDER, "Rank Index", 0},        // skip 3 bits, 1 bit
    {PLACEHOLDER, "CQI CW0", 0},           // skip 2 bits, 4 bits
    {PLACEHOLDER, "CQI CW1", 0},           // 4 bits
    {PLACEHOLDER, "Wideband PMI", 0},      // 4 bits
    {PLACEHOLDER, "Carrier Index", 0},     // 4 bits
    {PLACEHOLDER, "CSF Tx Mode", 0},       // 4 bits
    {SKIP, NULL, 4},
};

const Fmt LtePhyPucchCsf_Payload_v102[] = {
    {UINT, "Start System Sub-frame Number", 2},    // 4 bits
    {PLACEHOLDER, "Start System Frame Number", 0}, // 10 bits
    {UINT, "PUCCH Reporting Mode", 1}, // last 2 bits in previous byte + 1 bit
    {PLACEHOLDER, "PUCCH Report Type", 0}, // 4 bit
    {UINT, "Number of Subbands", 4},       // 4 bits
    {PLACEHOLDER, "Alt Cqi Table Data", 0},
    {PLACEHOLDER, "CQI CW0", 0},       // skip 2 bits, 4 bits
    {PLACEHOLDER, "CQI CW1", 0},       // 4 bits
    {PLACEHOLDER, "Wideband PMI", 0},  // 4 bits
    {PLACEHOLDER, "Carrier Index", 0}, // 4 bits
    {PLACEHOLDER, "CSF Tx Mode", 0},   // 4 bits
    {UINT, "Num Csirs Ports", 1},
    {UINT, "Csi Meas Set Index", 1},
    {PLACEHOLDER, "Rank Index", 0},
    {SKIP, NULL, 2},
};

static int _decode_lte_phy_pucch_csf_payload(const char *b, int offset,
                                             size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    int temp;

    switch (pkt_ver) {
    case 43: {
        offset += _decode_by_fmt(LtePhyPucchCsf_Payload_v43,
                                 ARRAY_SIZE(LtePhyPucchCsf_Payload_v43, Fmt), b,
                                 offset, length, result);
        temp = _search_result_int(result, "Start System Sub-frame Number");
        int iSubFN = temp & 15;
        int iSysFN = (temp >> 4) & 1023;
        int ileft = (temp >> 14) & 3;
        temp = _search_result_int(result, "PUCCH Reporting Mode");
        int iPucchReportingMode = ((temp & 1) << 2) | ileft;
        int iPucchReportType = (temp >> 1) & 15;
        unsigned int utemp = _search_result_uint(result, "Number of Subbands");
        int iNumSubbands = utemp & 15;
        int iRankIndex = (utemp >> 7) & 1;
        int iCQI0 = (utemp >> 10) & 15;
        int iCQI1 = (utemp >> 14) & 15;
        int iWidebandPMI = (utemp >> 18) & 15;
        int iCarrierIndex = (utemp >> 22) & 15;
        int iCSFTxMode = (utemp >> 26) & 15;

        _replace_result_int(result, "Start System Sub-frame Number", iSubFN);
        _replace_result_int(result, "Start System Frame Number", iSysFN);
        _replace_result_int(result, "PUCCH Reporting Mode",
                            iPucchReportingMode);
        (void)_map_result_field_to_name(
            result, "PUCCH Reporting Mode", ValueNamePucchReportingMode,
            ARRAY_SIZE(ValueNamePucchReportingMode, ValueName), "(MI)Unknown");
        _replace_result_int(result, "PUCCH Report Type", iPucchReportType);
        (void)_map_result_field_to_name(
            result, "PUCCH Report Type", ValueNamePucchReportType,
            ARRAY_SIZE(ValueNamePucchReportType, ValueName), "(MI)Unknown");
        _replace_result_int(result, "Rank Index", iRankIndex);
        (void)_map_result_field_to_name(
            result, "Rank Index", ValueNameRankIndex,
            ARRAY_SIZE(ValueNameRankIndex, ValueName), "(MI)Unknown");
        _replace_result_int(result, "Number of Subbands", iNumSubbands);
        _replace_result_int(result, "CQI CW0", iCQI0);
        _replace_result_int(result, "CQI CW1", iCQI1);
        _replace_result_int(result, "Wideband PMI", iWidebandPMI);
        _replace_result_int(result, "CSF Tx Mode", iCSFTxMode);
        (void)_map_result_field_to_name(
            result, "CSF Tx Mode", ValueNameCSFTxMode,
            ARRAY_SIZE(ValueNameCSFTxMode, ValueName), "(MI)Unknown");
        _replace_result_int(result, "Carrier Index", iCarrierIndex);
        (void)_map_result_field_to_name(
            result, "Carrier Index", ValueNameCarrierIndex,
            ARRAY_SIZE(ValueNameCarrierIndex, ValueName), "(MI)Unknown");
        return offset - start;
    }
    case 102: {
        offset += _decode_by_fmt(LtePhyPucchCsf_Payload_v102,
                                 ARRAY_SIZE(LtePhyPucchCsf_Payload_v102, Fmt),
                                 b, offset, length, result);
        temp = _search_result_int(result, "Start System Sub-frame Number");
        int iSubFN = temp & 15;
        int iSysFN = (temp >> 4) & 1023;
        int ileft = (temp >> 14) & 3;
        temp = _search_result_int(result, "PUCCH Reporting Mode");
        int iPucchReportingMode = ((temp & 1) << 2) | ileft;
        int iPucchReportType = (temp >> 1) & 15;
        unsigned int utemp = _search_result_uint(result, "Number of Subbands");
        int iNumSubbands = utemp & 15;
        int iAltCqiTableData = (utemp >> 7) & 1;
        int iCQI0 = (utemp >> 10) & 15;
        int iCQI1 = (utemp >> 14) & 15;
        int iWidebandPMI = (utemp >> 18) & 15;
        int iCarrierIndex = (utemp >> 22) & 15;
        int iCSFTxMode = (utemp >> 26) & 15;

        temp = _search_result_int(result, "Num Csirs Ports");
        int iNumCsirsPorts = temp & 15;
        _replace_result_int(result, "Num Csirs Ports", iNumCsirsPorts);

        _replace_result_int(result, "Alt Cqi Table Data", iAltCqiTableData);

        temp = _search_result_int(result, "Csi Meas Set Index");
        int iCsiMeasSetIndex = temp & 3;
        int iRankIndex = (utemp >> 2) & 7;
        _replace_result_int(result, "Csi Meas Set Index", iCsiMeasSetIndex);
        _replace_result_int(result, "Rank Index", iRankIndex);
        (void)_map_result_field_to_name(
            result, "Rank Index", ValueNameRankIndex,
            ARRAY_SIZE(ValueNameRankIndex, ValueName), "(MI)Unknown");

        _replace_result_int(result, "Start System Sub-frame Number", iSubFN);
        _replace_result_int(result, "Start System Frame Number", iSysFN);
        _replace_result_int(result, "PUCCH Reporting Mode",
                            iPucchReportingMode);
        (void)_map_result_field_to_name(
            result, "PUCCH Reporting Mode", ValueNamePucchReportingMode,
            ARRAY_SIZE(ValueNamePucchReportingMode, ValueName), "(MI)Unknown");
        _replace_result_int(result, "PUCCH Report Type", iPucchReportType);
        (void)_map_result_field_to_name(
            result, "PUCCH Report Type", ValueNamePucchReportType,
            ARRAY_SIZE(ValueNamePucchReportType, ValueName), "(MI)Unknown");
        _replace_result_int(result, "Number of Subbands", iNumSubbands);
        _replace_result_int(result, "CQI CW0", iCQI0);
        _replace_result_int(result, "CQI CW1", iCQI1);
        _replace_result_int(result, "Wideband PMI", iWidebandPMI);
        _replace_result_int(result, "CSF Tx Mode", iCSFTxMode);
        (void)_map_result_field_to_name(
            result, "CSF Tx Mode", ValueNameCSFTxMode,
            ARRAY_SIZE(ValueNameCSFTxMode, ValueName), "(MI)Unknown");
        _replace_result_int(result, "Carrier Index", iCarrierIndex);
        (void)_map_result_field_to_name(
            result, "Carrier Index", ValueNameCarrierIndex,
            ARRAY_SIZE(ValueNameCarrierIndex, ValueName), "(MI)Unknown");
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PHY PUCCH CSF version: 0x%x\n", pkt_ver);
        return 0;
    }
}
