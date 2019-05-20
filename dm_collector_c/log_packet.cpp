/* log_packet.cpp
 * Author: Jiayao Li, Jie Zhao
 * Implements log packet message decoding.
 */

#include <arpa/inet.h>
#include <cstring>
//#include <datetime.h>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "1xev_connected_state_search_info.h"
#include "1xev_connection_attempt.h"
#include "1xev_connection_release.h"
#include "1xev_rx_partial_multirlp_packet.h"
#include "1xev_signaling_control_channel_broadcast.h"
#include "1xevdo_multi_carrier_pilot_sets.h"
#include "cdma_paging_channel_msg.h"
#include "consts.h"
#include "gsm_dsds_rr_cell_information.h"
#include "gsm_dsds_rr_cell_reselection_param.h"
#include "gsm_dsds_rr_signaling_message.h"
#include "gsm_rr_cell_information.h"
#include "gsm_rr_cell_reselection_meas.h"
#include "gsm_rr_cell_reselection_param.h"
#include "gsm_rr_signaling_message.h"
#include "gsm_surround_cell_ba_list.h"
#include "log_packet.h"
#include "log_packet_helper.h"
#include "lte_pdcp_dl_cipher_data_pdu.h"
#include "lte_pdcp_ul_cipher_data_pdu.h"
#include "lte_pdsch_stat_indication.h"
#include "lte_phy_bplmn_cell_confirm.h"
#include "lte_phy_bplmn_cell_request.h"
#include "lte_phy_cdrx_events_info.h"
#include "lte_phy_connected_neighbor_cell_meas.h"
#include "lte_phy_idle_neighbor_cell_meas.h"
#include "lte_phy_pdcch_decoding_result.h"
#include "lte_phy_pdsch_decoding_result.h"
#include "lte_phy_pucch_csf.h"
#include "lte_phy_pucch_tx_report.h"
#include "lte_phy_pusch_csf.h"
#include "lte_phy_pusch_tx_report.h"
#include "lte_phy_rlm_report.h"
#include "lte_phy_serving_cell_com_loop.h"
#include "lte_phy_system_scan_results.h"
#include "srch_tng_1x_searcher_dump.h"
#include "wcdma_rrc_states.h"
#include "wcdma_search_cell_reselection_rank.h"
#include "binary_decode.h"

// #ifdef __ANDROID__
// #include <android/log.h>
// #define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "TAG",
// __VA_ARGS__); #endif

// Yuanjie: In-phone version cannot locate std::to_string
namespace patch {
template <typename T> std::string to_string(const T &n) {
    std::ostringstream stm;
    stm << n;
    return stm.str();
}
} // namespace patch

/*
 * The decoding result is represented using a Python list object (called result
 * list in the source code).
 * All elements in a result list is a 3-element tuple: (name, value, type).
 * "name" is a C string, and "value" is a reference to a Python object.
 * "type" is a C string that can be one of the following values:
 * - "" (empty string): an ordinary field. The "value" could be a Python
 *      integer, str, etc.
 * - "list": The "value" is another result list, and each element is of the same
 *      "type".
 * - "dict": The "value" is another result list, and each element can be of
 *      different "types".
 * - "raw_msg/TYPE": The value is a binary string that contains raw messages.
 */

// TODO: split this .cpp to multiple files.

static int _decode_wcdma_signaling_messages(const char *b, int offset,
                                            size_t length, json &result) {
    (void)length;
    int start = offset;
    int ch_num = _search_result_int(result, "Channel Type");
    const char *ch_name = search_name(
        WcdmaSignalingMsgChannelType,
        ARRAY_SIZE(WcdmaSignalingMsgChannelType, ValueName), ch_num);

    if (ch_name == NULL) { // not found
        printf("(MI)Unknown WCDMA Signalling Messages Channel Type: 0x%x\n",
               ch_num);
        return 0;
    }

    int pdu_length = _search_result_int(result, "Message Length");

    if (0 == strncmp(ch_name, "RRC_COMPLETE_SIB", 16)) {
        offset += _decode_by_fmt(
            WcdmaSignalingMessagesFmtExtraSIBType,
            ARRAY_SIZE(WcdmaSignalingMessagesFmtExtraSIBType, Fmt), b, offset,
            length, result);
        pdu_length--;
        int ch_num = _search_result_int(result, "Extra SIB Type");
        ch_name = search_name(ValueNameWcdmaExtraSIBType,
                              ARRAY_SIZE(ValueNameWcdmaExtraSIBType, ValueName),
                              ch_num);
        if (ch_name == NULL) { // not found
            printf("(MI)Unknown WCDMA Signalling Messages RRC Complete SIB "
                   "Type: 0x%x\n",
                   ch_num);
            return 0;
        }
    } else if (0 == strncmp(ch_name, "Extension SIB", 13)) {
        offset += _decode_by_fmt(
            WcdmaSignalingMessagesFmtExtensionSIBType,
            ARRAY_SIZE(WcdmaSignalingMessagesFmtExtensionSIBType, Fmt), b,
            offset, length, result);
        pdu_length--;
        int ch_num = _search_result_int(result, "Extension SIB Type");
        ch_name = search_name(
            ValueNameWcdmaExtensionSIBType,
            ARRAY_SIZE(ValueNameWcdmaExtensionSIBType, ValueName), ch_num);
        if (ch_name == NULL) { // not found
            printf("(MI)Unknown WCDMA Signalling Messages RRC Extension SIB "
                   "Type: 0x%x\n",
                   ch_num);
            return 0;
        }
    }

    std::string type_str = "raw_msg/";
    type_str += ch_name;
    result["Msg"] =
        std::string(type_str.c_str()) + make_string(b + offset, pdu_length);
    return offset - start;
}

static int _decode_umts_nas_gmm_state(const char *b, int offset, size_t length,
                                      json &result) {
    (void)b;
    (void)offset;
    (void)length;
    (void)_map_result_field_to_name(
        result, "GMM State", UmtsNasGmmState_GmmState,
        ARRAY_SIZE(UmtsNasGmmState_GmmState, ValueName), "(MI)Unknown");
    (void)_map_result_field_to_name(
        result, "GMM Substate", UmtsNasGmmState_GmmSubstate,
        ARRAY_SIZE(UmtsNasGmmState_GmmSubstate, ValueName), "(MI)Unknown");
    (void)_map_result_field_to_name(
        result, "GMM Update Status", UmtsNasGmmState_GmmUpdateStatus,
        ARRAY_SIZE(UmtsNasGmmState_GmmUpdateStatus, ValueName), "(MI)Unknown");
    return 0;
}

static int _decode_umts_nas_mm_state(const char *b, int offset, size_t length,
                                     json &result) {
    (void)b;
    (void)offset;
    (void)length;
    (void)_map_result_field_to_name(
        result, "MM State", UmtsNasMmState_MmState,
        ARRAY_SIZE(UmtsNasMmState_MmState, ValueName), "(MI)Unknown");
    (void)_map_result_field_to_name(
        result, "MM Substate", UmtsNasMmState_MmSubstate,
        ARRAY_SIZE(UmtsNasMmState_MmSubstate, ValueName), "(MI)Unknown");
    (void)_map_result_field_to_name(
        result, "MM Update Status", UmtsNasMmState_MmUpdateStatus,
        ARRAY_SIZE(UmtsNasMmState_MmUpdateStatus, ValueName), "(MI)Unknown");
    return 0;
}

static int _decode_umts_nas_ota(const char *b, int offset, size_t length,
                                json &result) {
    (void)length;
    // int start = offset;
    (void)_map_result_field_to_name(
        result, "Message Direction", UmtsNasOtaFmt_MessageDirection,
        ARRAY_SIZE(UmtsNasOtaFmt_MessageDirection, ValueName), "(MI)Unknown");

    int pdu_length = _search_result_int(result, "Message Length");
    result["Msg"] =
        std::string("raw_msg/NAS") + make_string(b + offset, pdu_length);
    return pdu_length;
}

static int _decode_lte_rrc_ota(const char *b, int offset, size_t length,
                               json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Pkt Version");

    switch (pkt_ver) {
    case 2:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt_v2,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt_v2, Fmt), b,
                                 offset, length, result);
        break;
    case 4:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt_v4,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt_v4, Fmt), b,
                                 offset, length, result);
        break;
    case 7:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt_v7,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt_v7, Fmt), b,
                                 offset, length, result);
        break;
    case 8:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt_v8,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt_v8, Fmt), b,
                                 offset, length, result);
        break;
    case 9:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt_v9,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt_v9, Fmt), b,
                                 offset, length, result);
        break;
    case 12:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt_v12,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt_v12, Fmt), b,
                                 offset, length, result);
        break;
    case 13:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt_v13,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt_v13, Fmt), b,
                                 offset, length, result);
        break;
    case 15:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt_v15,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt_v15, Fmt), b,
                                 offset, length, result);
        break;
    case 20:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt_v20,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt_v20, Fmt), b,
                                 offset, length, result);
        break;
    default:
        printf("(MI)Unknown LTE RRC OTA packet version: %d\n", pkt_ver);
        return 0;
    }

    if (pkt_ver >= 15) {
        int pdu_number = _search_result_int(result, "PDU Number");
        int pdu_length = _search_result_int(result, "Msg Length");
        const char *type_name = search_name(
            LteRrcOtaPduType_v15, ARRAY_SIZE(LteRrcOtaPduType_v15, ValueName),
            pdu_number);

        if (type_name == NULL) { // not found
            printf("(MI)Unknown LTE RRC PDU Type: 0x%x\n", pdu_number);
            return 0;
        } else {
			int type_code = 0;
            //std::string type_str = "raw_msg/";
            //type_str += type_name;
            //result["Msg"] = std::string(type_str.c_str()) +
            //                make_string(b + offset, pdu_length);
			//                        "LTE-RRC_PCCH": 200,
// 57         "LTE-RRC_DL_DCCH": 201,
// 58         "LTE-RRC_UL_DCCH": 202,
// 59         "LTE-RRC_BCCH_DL_SCH": 203,
// 60         "LTE-RRC_DL_CCCH": 204,
// 61         "LTE-RRC_UL_CCCH": 205,
			int type_len = std::string(type_name).size();
			switch(type_len){
				case 12: // RRC_PCCH
					type_code = 200;
					break;
				case 19: // RRC_BCCH_DL_SCH
					type_code = 203;
					break;
				case 15:
					switch(type_name[8]){
						case 'D':
							switch (type_name[11]){
								case 'D': // RRC_DL_DCCH
									type_code = 201;
									break;
								case 'C': // RRC_DL_CCCH
									type_code = 204;
									break;
							}
							break;
						case 'U':
							switch (type_name[11]){
								case 'D': // RRC_UL_DCCH
									type_code = 202;
									break;
								case 'C': // RRC_UL_CCCH
									type_code = 205;
									break;
							}
							break;
						default:
							type_code = 0;
					}
					break;
				default:
					type_code = 0;
			}
            result["Msg"] = binary_decode(type_code, b + offset, pdu_length);
            return (offset - start) + pdu_length;
        }
    } else {
        int pdu_number = _search_result_int(result, "PDU Number");
        int pdu_length = _search_result_int(result, "Msg Length");
        if (pdu_number > 8) { // Hack. This is not confirmed.
            pdu_number -= 7;
        }
        const char *type_name =
            search_name(LteRrcOtaPduType,
                        ARRAY_SIZE(LteRrcOtaPduType, ValueName), pdu_number);

        if (type_name == NULL) { // not found
            printf("(MI)Unknown LTE RRC PDU Type: 0x%x\n", pdu_number);
            return 0;
        } else {
			unsigned int type_code = 0;
            //std::string type_str = "raw_msg/";
            //type_str += type_name;
           // result["Msg"] = std::string(type_str.c_str()) +
             //               make_string(b + offset, pdu_length);
			int type_len = std::string(type_name).size();
			switch(type_len){
				case 12: // RRC_PCCH
					type_code = 200;
					break;
				case 19: // RRC_BCCH_DL_SCH
					type_code = 203;
					break;
				case 15:
					switch(type_name[8]){
						case 'D':
							switch (type_name[11]){
								case 'D': // RRC_DL_DCCH
									type_code = 201;
									break;
								case 'C': // RRC_DL_CCCH
									type_code = 204;
									break;
							}
							break;
						case 'U':
							switch (type_name[11]){
								case 'D': // RRC_UL_DCCH
									type_code = 202;
									break;
								case 'C': // RRC_UL_CCCH
									type_code = 205;
									break;
							}
							break;
						default:
							type_code = 0;
					}
					break;
				default:
					type_code = 0;
			}
            result["Msg"] = binary_decode(0xca, b + offset, pdu_length);
            return (offset - start) + pdu_length;
        }
    }
}

static int _decode_lte_rrc_mib(const char *b, int offset, size_t length,
                               json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 1:
        offset +=
            _decode_by_fmt(LteRrcMibMessageLogPacketFmt_v1,
                           ARRAY_SIZE(LteRrcMibMessageLogPacketFmt_v1, Fmt), b,
                           offset, length, result);
        break;
    case 2:
        offset +=
            _decode_by_fmt(LteRrcMibMessageLogPacketFmt_v2,
                           ARRAY_SIZE(LteRrcMibMessageLogPacketFmt_v2, Fmt), b,
                           offset, length, result);
        break;
    default:
        printf("(MI)Unknown LTE RRC MIB version: 0x%x\n", pkt_ver);
        return 0;
    }

    return offset - start;
}

static int _decode_lte_rrc_serv_cell_info(const char *b, int offset,
                                          size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 2:
        offset +=
            _decode_by_fmt(LteRrcServCellInfoLogPacketFmt_v2,
                           ARRAY_SIZE(LteRrcServCellInfoLogPacketFmt_v2, Fmt),
                           b, offset, length, result);
        break;
    case 3:
        offset +=
            _decode_by_fmt(LteRrcServCellInfoLogPacketFmt_v3,
                           ARRAY_SIZE(LteRrcServCellInfoLogPacketFmt_v3, Fmt),
                           b, offset, length, result);
        break;
    default:
        printf("(MI)Unknown LTE RRC Serving Cell Info packet version: %d\n",
               pkt_ver);
        return 0;
    }

    return offset - start;
}

static size_t _decode_lte_nas_plain(const char *b, int offset, size_t length,
                                    json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Pkt Version");

    switch (pkt_ver) {
    case 1:
        offset += _decode_by_fmt(LteNasPlainFmt_v1,
                                 ARRAY_SIZE(LteNasPlainFmt_v1, Fmt), b, offset,
                                 length, result);
        break;
    default:
        printf("(MI)Unknown LTE NAS version: 0x%x\n", pkt_ver);
        return 0;
    }

    size_t pdu_length = length - offset;
    //result["Msg"] = std::string("raw_msg/LTE-NAS_EPS_PLAIN") +
    //                make_string(b + offset, pdu_length);
	//"LTE-NAS_EPS_PLAIN": 250,
	result["Msg"] = binary_decode(250, b + offset, pdu_length);
    return length - start;
}

static int _decode_lte_nas_esm_state(const char *b, int offset, size_t length,
                                     json &result) {

    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    switch (pkt_ver) {

    case 1: {
        offset += _decode_by_fmt(LteNasEsmStateFmt_v1,
                                 ARRAY_SIZE(LteNasEsmStateFmt_v1, Fmt), b,
                                 offset, length, result);

        break;
    }

    default:
        printf("(MI)Unknown LTE NAS ESM State version: 0x%x\n", pkt_ver);
        return 0;
    }

    return offset - start;
}

static int _decode_lte_nas_emm_state(const char *b, int offset, size_t length,
                                     json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 2: {
        offset += _decode_by_fmt(LteNasEmmStateFmt_v2,
                                 ARRAY_SIZE(LteNasEmmStateFmt_v2, Fmt), b,
                                 offset, length, result);
        int emm_state_id = _map_result_field_to_name(
            result, "EMM State", LteNasEmmState_v2_EmmState,
            ARRAY_SIZE(LteNasEmmState_v2_EmmState, ValueName), "(MI)Unknown");
        // Replace the value of "EMM Substate"
        const ValueName *table = NULL;
        int table_size = 0;
        switch (emm_state_id) {
        case 1: // EMM_DEREGISTERED
            table = LteNasEmmState_v2_EmmSubstate_Deregistered;
            table_size = ARRAY_SIZE(LteNasEmmState_v2_EmmSubstate_Deregistered,
                                    ValueName);
            break;

        case 2: // EMM_REGISTERED_INITIATED
            table = LteNasEmmState_v2_EmmSubstate_Registered_Initiated;
            table_size = ARRAY_SIZE(
                LteNasEmmState_v2_EmmSubstate_Registered_Initiated, ValueName);
            break;

        case 3: // EMM_REGISTERED
        case 4: // EMM_TRACKING_AREA_UPDATING_INITIATED
        case 5: // EMM_SERVICE_REQUEST_INITIATED
            table = LteNasEmmState_v2_EmmSubstate_Registered;
            table_size =
                ARRAY_SIZE(LteNasEmmState_v2_EmmSubstate_Registered, ValueName);
            break;

        case 0: // EMM_NULL
        case 6: // EMM_DEREGISTERED_INITIATED
        default:
            // No Substate
            break;
        }
        if (table != NULL && table_size > 0) {
            (void)_map_result_field_to_name(result, "EMM Substate", table,
                                            table_size, "(MI)Unknown");
        } else {
            std::string pystr = "Undefined";
            _replace_result(result, "EMM Substate", pystr);
        }
        break;
    } // End of v2

    default:
        printf("(MI)Unknown LTE NAS EMM State version: 0x%x\n", pkt_ver);
        return 0;
    }

    return offset - start;
}

static int _decode_lte_phy_pdsch_demapper_config(const char *b, int offset,
                                                 size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 23: {
        offset +=
            _decode_by_fmt(LtePhyPdschDemapperConfigFmt_v23,
                           ARRAY_SIZE(LtePhyPdschDemapperConfigFmt_v23, Fmt), b,
                           offset, length, result);

        const unsigned int SFN_RSHIFT = 5, SFN_MASK = (1 << 10) - 1;
        const unsigned int SUBFRAME_RSHIFT = 1, SUBFRAME_MASK = (1 << 4) - 1;
        int tmp = _search_result_int(result, "System Frame Number");
        int sfn = (tmp >> SFN_RSHIFT) & SFN_MASK;
        int subframe = (tmp >> SUBFRAME_RSHIFT) & SUBFRAME_MASK;
        int serv_cell = _search_result_int(result, "Serving Cell ID");
        serv_cell += (tmp & 0x1) << 8;

        _replace_result_int(result, "Serving Cell ID", serv_cell);
        _replace_result_int(result, "System Frame Number", sfn);
        _replace_result_int(result, "Subframe Number", subframe);

        // # antennas
        tmp = _search_result_int(result, "Number of Tx Antennas(M)");
        int iRNTIType = tmp & 15;
        int iSpatialRank = (tmp >> 14) & 3;
        tmp = (tmp >> 8) & 0x0f;
        int M = tmp & 0x3;
        M = (M != 3 ? (1 << M) : -1);
        int N = (tmp >> 2) & 0x1;
        N = (1 << N);

        _replace_result_int(result, "PDSCH RNTI Type", iRNTIType);
        (void)_map_result_field_to_name(
            result, "PDSCH RNTI Type", ValueNameRNTIType,
            ARRAY_SIZE(ValueNameRNTIType, ValueName), "(MI)Unknown");

        _replace_result_int(result, "Number of Tx Antennas(M)", M);
        _replace_result_int(result, "Number of Rx Antennas(N)", N);
        _replace_result_int(result, "Spatial Rank", iSpatialRank);
        (void)_map_result_field_to_name(
            result, "Spatial Rank", ValueNameRankIndex,
            ARRAY_SIZE(ValueNameRankIndex, ValueName), "(MI)Unknown");

        tmp = _search_result_int(result, "Frequency Selective PMI");
        int iFrequencySelectivePMI = (tmp >> 1) & 3;
        int iPMIIndex = (tmp >> 4) & 15;
        tmp = _search_result_int(result, "Transmission Scheme");
        int iTransmissionScheme = tmp & 15;

        _replace_result_int(result, "Frequency Selective PMI",
                            iFrequencySelectivePMI);
        (void)_map_result_field_to_name(
            result, "Frequency Selective PMI", ValueNameFrequencySelectivePMI,
            ARRAY_SIZE(ValueNameFrequencySelectivePMI, ValueName),
            "(MI)Unknown");
        _replace_result_int(result, "PMI Index", iPMIIndex);
        _replace_result_int(result, "Transmission Scheme", iTransmissionScheme);
        (void)_map_result_field_to_name(
            result, "Transmission Scheme", ValueNameTransmissionScheme,
            ARRAY_SIZE(ValueNameTransmissionScheme, ValueName), "(MI)Unknown");

        // modulation & ratio
        tmp = _search_result_int(result, "MCS 0");
        int mod_stream0 = (tmp >> 1) & 0x3;
        float ratio = float((tmp >> 3) & 4095) / 256.0;
        tmp = _search_result_int(result, "MCS 1");
        int mod_stream1 = (tmp >> 1) & 0x3;
        int carrier_index = (tmp >> 9) & 0xf;

        _replace_result_int(result, "MCS 0", mod_stream0);
        (void)_map_result_field_to_name(
            result, "MCS 0", LtePhyPdschDemapperConfig_v23_Modulation,
            ARRAY_SIZE(LtePhyPdschDemapperConfig_v23_Modulation, ValueName),
            "(MI)Unknown");

        double pyfloat = ratio;
        // FIXME: Traffic to Pilot Ratio is incorrect. Refine its calculation
        _replace_result(result, "Traffic to Pilot Ratio", pyfloat);

        _replace_result_int(result, "MCS 1", mod_stream1);
        (void)_map_result_field_to_name(
            result, "MCS 1", LtePhyPdschDemapperConfig_v23_Modulation,
            ARRAY_SIZE(LtePhyPdschDemapperConfig_v23_Modulation, ValueName),
            "(MI)Unknown");

        // carrier index
        _replace_result_int(result, "Carrier Index", carrier_index);
        (void)_map_result_field_to_name(
            result, "Carrier Index",
            LtePhyPdschDemapperConfig_v23_Carrier_Index,
            ARRAY_SIZE(LtePhyPdschDemapperConfig_v23_Carrier_Index, ValueName),
            "(MI)Unknown");
        break;
    }
    case 103: {
        offset +=
            _decode_by_fmt(LtePhyPdschDemapperConfigFmt_v103,
                           ARRAY_SIZE(LtePhyPdschDemapperConfigFmt_v103, Fmt),
                           b, offset, length, result);

        const unsigned int SFN_RSHIFT = 5, SFN_MASK = (1 << 10) - 1;
        const unsigned int SUBFRAME_RSHIFT = 1, SUBFRAME_MASK = (1 << 4) - 1;
        int tmp = _search_result_int(result, "System Frame Number");
        int sfn = (tmp >> SFN_RSHIFT) & SFN_MASK;
        int subframe = (tmp >> SUBFRAME_RSHIFT) & SUBFRAME_MASK;
        int serv_cell = _search_result_int(result, "Serving Cell ID");
        serv_cell += (tmp & 0x1) << 8;

        _replace_result_int(result, "Serving Cell ID", serv_cell);
        _replace_result_int(result, "System Frame Number", sfn);
        _replace_result_int(result, "Subframe Number", subframe);

        // # antennas
        tmp = _search_result_int(result, "Number of Tx Antennas(M)");
        int iRNTIType = tmp & 15;
        int iSpatialRank = (tmp >> 14) & 3;
        tmp = (tmp >> 8) & 0x0f;
        int M = tmp & 0x3;
        M = (M != 3 ? (1 << M) : -1);
        int N = (tmp >> 2) & 0x1;
        N = (1 << N);

        _replace_result_int(result, "PDSCH RNTI Type", iRNTIType);
        (void)_map_result_field_to_name(
            result, "PDSCH RNTI Type", ValueNameRNTIType,
            ARRAY_SIZE(ValueNameRNTIType, ValueName), "(MI)Unknown");

        _replace_result_int(result, "Number of Tx Antennas(M)", M);
        _replace_result_int(result, "Number of Rx Antennas(N)", N);
        _replace_result_int(result, "Spatial Rank", iSpatialRank);
        (void)_map_result_field_to_name(
            result, "Spatial Rank", ValueNameRankIndex,
            ARRAY_SIZE(ValueNameRankIndex, ValueName), "(MI)Unknown");

        tmp = _search_result_int(result, "Frequency Selective PMI");
        int iFrequencySelectivePMI = (tmp >> 1) & 3;
        int iPMIIndex = (tmp >> 4) & 15;
        tmp = _search_result_int(result, "Transmission Scheme");
        int iTransmissionScheme = tmp & 15;

        _replace_result_int(result, "Frequency Selective PMI",
                            iFrequencySelectivePMI);
        (void)_map_result_field_to_name(
            result, "Frequency Selective PMI", ValueNameFrequencySelectivePMI,
            ARRAY_SIZE(ValueNameFrequencySelectivePMI, ValueName),
            "(MI)Unknown");
        _replace_result_int(result, "PMI Index", iPMIIndex);
        _replace_result_int(result, "Transmission Scheme", iTransmissionScheme);
        (void)_map_result_field_to_name(
            result, "Transmission Scheme", ValueNameTransmissionScheme,
            ARRAY_SIZE(ValueNameTransmissionScheme, ValueName), "(MI)Unknown");

        // modulation & ratio
        tmp = _search_result_int(result, "MCS 0");
        int mod_stream0 = (tmp >> 2) & 0x3;
        float ratio = float((tmp >> 4) & 0x1fff) / 256.0;
        tmp = _search_result_int(result, "MCS 1");
        int mod_stream1 = (tmp >> 2) & 0x3;
        int carrier_index = (tmp >> 9) & 0xf;

        _replace_result_int(result, "MCS 0", mod_stream0);
        (void)_map_result_field_to_name(
            result, "MCS 0", LtePhyPdschDemapperConfig_v23_Modulation,
            ARRAY_SIZE(LtePhyPdschDemapperConfig_v23_Modulation, ValueName),
            "(MI)Unknown");

        double pyfloat = ratio;
        _replace_result(result, "Traffic to Pilot Ratio", pyfloat);

        _replace_result_int(result, "MCS 1", mod_stream1);
        (void)_map_result_field_to_name(
            result, "MCS 1", LtePhyPdschDemapperConfig_v23_Modulation,
            ARRAY_SIZE(LtePhyPdschDemapperConfig_v23_Modulation, ValueName),
            "(MI)Unknown");

        // carrier index
        _replace_result_int(result, "Carrier Index", carrier_index);
        (void)_map_result_field_to_name(
            result, "Carrier Index",
            LtePhyPdschDemapperConfig_v23_Carrier_Index,
            ARRAY_SIZE(LtePhyPdschDemapperConfig_v23_Carrier_Index, ValueName),
            "(MI)Unknown");
        break;
    }

    case 104:
    case 123: {
        offset +=
            _decode_by_fmt(LtePhyPdschDemapperConfigFmt_v104,
                           ARRAY_SIZE(LtePhyPdschDemapperConfigFmt_v104, Fmt),
                           b, offset, length, result);

        const unsigned int SFN_RSHIFT = 5, SFN_MASK = (1 << 10) - 1;
        const unsigned int SUBFRAME_RSHIFT = 1, SUBFRAME_MASK = (1 << 4) - 1;
        int tmp = _search_result_int(result, "System Frame Number");
        int sfn = (tmp >> SFN_RSHIFT) & SFN_MASK;
        int subframe = (tmp >> SUBFRAME_RSHIFT) & SUBFRAME_MASK;
        int serv_cell = _search_result_int(result, "Serving Cell ID");
        serv_cell += (tmp & 0x1) << 8;

        _replace_result_int(result, "Serving Cell ID", serv_cell);
        _replace_result_int(result, "System Frame Number", sfn);
        _replace_result_int(result, "Subframe Number", subframe);

        // # antennas
        tmp = _search_result_int(result, "Number of Tx Antennas(M)");
        int iRNTIType = tmp & 15;
        int iSpatialRank = (tmp >> 14) & 3;
        tmp = (tmp >> 8) & 0x0f;
        int M = tmp & 0x3;
        M = (M != 3 ? (1 << M) : -1);
        int N = (tmp >> 2) & 0x1;
        N = (1 << N);

        _replace_result_int(result, "PDSCH RNTI Type", iRNTIType);
        (void)_map_result_field_to_name(
            result, "PDSCH RNTI Type", ValueNameRNTIType,
            ARRAY_SIZE(ValueNameRNTIType, ValueName), "(MI)Unknown");

        _replace_result_int(result, "Number of Tx Antennas(M)", M);
        _replace_result_int(result, "Number of Rx Antennas(N)", N);
        _replace_result_int(result, "Spatial Rank", iSpatialRank);
        (void)_map_result_field_to_name(
            result, "Spatial Rank", ValueNameRankIndex,
            ARRAY_SIZE(ValueNameRankIndex, ValueName), "(MI)Unknown");

        tmp = _search_result_int(result, "Frequency Selective PMI");
        int iFrequencySelectivePMI = (tmp >> 1) & 3;
        int iPMIIndex = (tmp >> 4) & 15;
        tmp = _search_result_int(result, "Transmission Scheme");
        int iTransmissionScheme = tmp & 15;

        _replace_result_int(result, "Frequency Selective PMI",
                            iFrequencySelectivePMI);
        (void)_map_result_field_to_name(
            result, "Frequency Selective PMI", ValueNameFrequencySelectivePMI,
            ARRAY_SIZE(ValueNameFrequencySelectivePMI, ValueName),
            "(MI)Unknown");
        _replace_result_int(result, "PMI Index", iPMIIndex);
        _replace_result_int(result, "Transmission Scheme", iTransmissionScheme);
        (void)_map_result_field_to_name(
            result, "Transmission Scheme", ValueNameTransmissionScheme,
            ARRAY_SIZE(ValueNameTransmissionScheme, ValueName), "(MI)Unknown");

        // modulation & ratio
        tmp = _search_result_int(result, "MCS 0");
        int mod_stream0 = (tmp >> 2) & 0x3;
        float ratio = float((tmp >> 4) & 0x1fff) / 256.0;
        tmp = _search_result_int(result, "MCS 1");
        int mod_stream1 = (tmp >> 2) & 0x3;
        int carrier_index = (tmp >> 9) & 0xf;

        _replace_result_int(result, "MCS 0", mod_stream0);
        (void)_map_result_field_to_name(
            result, "MCS 0", LtePhyPdschDemapperConfig_v23_Modulation,
            ARRAY_SIZE(LtePhyPdschDemapperConfig_v23_Modulation, ValueName),
            "(MI)Unknown");

        double pyfloat = ratio;
        _replace_result(result, "Traffic to Pilot Ratio", pyfloat);

        _replace_result_int(result, "MCS 1", mod_stream1);
        (void)_map_result_field_to_name(
            result, "MCS 1", LtePhyPdschDemapperConfig_v23_Modulation,
            ARRAY_SIZE(LtePhyPdschDemapperConfig_v23_Modulation, ValueName),
            "(MI)Unknown");

        // carrier index
        _replace_result_int(result, "Carrier Index", carrier_index);
        (void)_map_result_field_to_name(
            result, "Carrier Index",
            LtePhyPdschDemapperConfig_v23_Carrier_Index,
            ARRAY_SIZE(LtePhyPdschDemapperConfig_v23_Carrier_Index, ValueName),
            "(MI)Unknown");
        break;
    }

    default:
        printf(
            "(MI)Unknown LTE PHY PDSCH Demapper Configuration version: 0x%x\n",
            pkt_ver);
        return 0;
    }

    return offset - start;
}

static int _decode_lte_phy_cmlifmr(const char *b, int offset, size_t length,
                                   json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    (void)_map_result_field_to_name(
        result, "Serving Cell Index", ValueNameCellIndex,
        ARRAY_SIZE(ValueNameCellIndex, ValueName), "(MI)Unknown");

    switch (pkt_ver) {
    case 3: {
        offset += _decode_by_fmt(LtePhyCmlifmrFmt_v3_Header,
                                 ARRAY_SIZE(LtePhyCmlifmrFmt_v3_Header, Fmt), b,
                                 offset, length, result);
        int n_neighbor_cells =
            _search_result_int(result, "Number of Neighbor Cells");
        int n_detected_cells =
            _search_result_int(result, "Number of Detected Cells");

        // decode "Neighbor Cells"
        json result_allcells;
        for (int i = 0; i < n_neighbor_cells; i++) {
            json result_cell;
            offset += _decode_by_fmt(
                LtePhyCmlifmrFmt_v3_Neighbor_Cell,
                ARRAY_SIZE(LtePhyCmlifmrFmt_v3_Neighbor_Cell, Fmt), b, offset,
                length, result_cell);
            result_allcells.push_back(result_cell);
        }
        result["Neighbor Cells"] = result_allcells;

        // decode "Detected Cells"
        for (int i = 0; i < n_detected_cells; i++) {
            json result_cell;
            offset += _decode_by_fmt(
                LtePhyCmlifmrFmt_v3_Detected_Cell,
                ARRAY_SIZE(LtePhyCmlifmrFmt_v3_Detected_Cell, Fmt), b, offset,
                length, result_cell);
            result_allcells.push_back(result_cell);
        }
        result["Detected Cells"] = result_allcells;

        return offset - start;
    }
    case 4: {
        offset += _decode_by_fmt(LtePhyCmlifmrFmt_v4_Header,
                                 ARRAY_SIZE(LtePhyCmlifmrFmt_v4_Header, Fmt), b,
                                 offset, length, result);
        int n_neighbor_cells =
            _search_result_int(result, "Number of Neighbor Cells");
        int n_detected_cells =
            _search_result_int(result, "Number of Detected Cells");

        // decode "Neighbor Cells"
        json result_allcells;
        for (int i = 0; i < n_neighbor_cells; i++) {
            json result_cell;
            offset += _decode_by_fmt(
                LtePhyCmlifmrFmt_v4_Neighbor_Cell,
                ARRAY_SIZE(LtePhyCmlifmrFmt_v4_Neighbor_Cell, Fmt), b, offset,
                length, result_cell);
            result_allcells.push_back(result_cell);
        }
        result["Neighbor Cells"] = result_allcells;

        // decode "Detected Cells"
        for (int i = 0; i < n_detected_cells; i++) {
            json result_cell;
            offset += _decode_by_fmt(
                LtePhyCmlifmrFmt_v4_Detected_Cell,
                ARRAY_SIZE(LtePhyCmlifmrFmt_v4_Detected_Cell, Fmt), b, offset,
                length, result_cell);
            result_allcells.push_back(result_cell);
        }
        result["Detected Cells"] = result_allcells;

        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PHY CMLIFMR version: 0x%x\n", pkt_ver);
        return 0;
    }
}

static int _decode_lte_phy_subpkt(const char *b, int offset, size_t length,
                                  json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Number of SubPackets");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            // Decode subpacket header
            offset +=
                _decode_by_fmt(LtePhySubpktFmt_v1_SubpktHeader,
                               ARRAY_SIZE(LtePhySubpktFmt_v1_SubpktHeader, Fmt),
                               b, offset, length, result_subpkt);
            // Decode payload
            int subpkt_id = _search_result_int(result_subpkt, "SubPacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "SubPacket Version");
            const char *type_name = search_name(
                LtePhySubpkt_SubpktType,
                ARRAY_SIZE(LtePhySubpkt_SubpktType, ValueName), subpkt_id);
            if (type_name == NULL) { // not found
                printf("(MI)Unknown LTE PHY Subpacket ID: 0x%x\n", subpkt_id);
            } else {
                bool success = false;
                if (strcmp(type_name, "Serving_Cell_Measurement_Result") == 0) {
                    switch (subpkt_ver) {
                    case 4: {
                        offset += _decode_by_fmt(
                            LtePhySubpktFmt_v1_Scmr_v4,
                            ARRAY_SIZE(LtePhySubpktFmt_v1_Scmr_v4, Fmt), b,
                            offset, length, result_subpkt);
                        int temp = _search_result_int(result_subpkt,
                                                      "Physical Cell ID");
                        int iPhyCellId = temp & 511;           // 9 bits
                        int iServingCellIdx = (temp >> 9) & 7; // 3 bits
                        _replace_result_int(result_subpkt, "Physical Cell ID",
                                            iPhyCellId);
                        _replace_result_int(result_subpkt, "Serving Cell Index",
                                            iServingCellIdx);
                        (void)_map_result_field_to_name(
                            result_subpkt, "Serving Cell Index",
                            ValueNameCellIndex,
                            ARRAY_SIZE(ValueNameCellIndex, ValueName),
                            "(MI)Unknown");

                        temp = _search_result_int(result_subpkt, "Current SFN");
                        int iSysFN = temp & 1023;       // 10 bits
                        int iSubFN = (temp >> 10) & 15; // 4 bits
                        _replace_result_int(result_subpkt, "Current SFN",
                                            iSysFN);
                        _replace_result_int(result_subpkt,
                                            "Current Subframe Number", iSubFN);

                        unsigned int utemp;
                        double pyfloat;

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[0]");
                        float RSRP0 = float((utemp >> 10) & 4095);
                        RSRP0 = RSRP0 * 0.0625 - 180.0;
                        pyfloat = RSRP0;
                        _replace_result(result_subpkt, "RSRP Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[1]");
                        float RSRP1 = float((utemp >> 12) & 4095);
                        RSRP1 = RSRP1 * 0.0625 - 180.0;
                        pyfloat = RSRP1;
                        _replace_result(result_subpkt, "RSRP Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSRP");
                        float RSRP = float((utemp >> 12) & 4095);
                        RSRP = RSRP * 0.0625 - 180.0;
                        pyfloat = RSRP;
                        _replace_result(result_subpkt, "RSRP", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[0]");
                        float RSRQ0 = float((utemp >> 12) & 1023);
                        RSRQ0 = RSRQ0 * 0.0625 - 30.0;
                        pyfloat = RSRQ0;
                        _replace_result(result_subpkt, "RSRQ Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[1]");
                        float RSRQ1 = float(utemp & 1023);
                        RSRQ1 = RSRQ1 * 0.0625 - 30.0;
                        pyfloat = RSRQ1;
                        _replace_result(result_subpkt, "RSRQ Rx[1]", pyfloat);

                        float RSRQ = float((utemp >> 20) & 1023);
                        RSRQ = RSRQ * 0.0625 - 30.0;
                        pyfloat = RSRQ;
                        _replace_result(result_subpkt, "RSRQ", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSSI Rx[0]");
                        float RSSI0 = float((utemp >> 10) & 2047);
                        RSSI0 = RSSI0 * 0.0625 - 110.0;
                        pyfloat = RSSI0;
                        _replace_result(result_subpkt, "RSSI Rx[0]", pyfloat);

                        float RSSI1 = float((utemp >> 21) & 2047);
                        RSSI1 = RSSI1 * 0.0625 - 110.0;
                        pyfloat = RSSI1;
                        _replace_result(result_subpkt, "RSSI Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSSI");
                        float RSSI = float(utemp & 2047);
                        RSSI = RSSI * 0.0625 - 110.0;
                        pyfloat = RSSI;
                        _replace_result(result_subpkt, "RSSI", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "FTL SNR Rx[0]");
                        float SNR0 = float(utemp & 511);
                        SNR0 = SNR0 * 0.1 - 20.0;
                        pyfloat = SNR0;
                        _replace_result(result_subpkt, "FTL SNR Rx[0]",
                                        pyfloat);

                        float SNR1 = float((utemp >> 9) & 511);
                        SNR1 = SNR1 * 0.1 - 20.0;
                        pyfloat = SNR1;
                        _replace_result(result_subpkt, "FTL SNR Rx[1]",
                                        pyfloat);

                        success = true;
                        break;
                    }
                    case 7: {
                        offset += _decode_by_fmt(
                            LtePhySubpktFmt_v1_Scmr_v7,
                            ARRAY_SIZE(LtePhySubpktFmt_v1_Scmr_v7, Fmt), b,
                            offset, length, result_subpkt);
                        int temp = _search_result_int(result_subpkt,
                                                      "Physical Cell ID");
                        int iPhyCellId = temp & 511;           // 9 bits
                        int iServingCellIdx = (temp >> 9) & 7; // 3 bits
                        _replace_result_int(result_subpkt, "Physical Cell ID",
                                            iPhyCellId);
                        _replace_result_int(result_subpkt, "Serving Cell Index",
                                            iServingCellIdx);
                        (void)_map_result_field_to_name(
                            result_subpkt, "Serving Cell Index",
                            ValueNameCellIndex,
                            ARRAY_SIZE(ValueNameCellIndex, ValueName),
                            "(MI)Unknown");

                        temp = _search_result_int(result_subpkt, "Current SFN");
                        int iSysFN = temp & 1023;       // 10 bits
                        int iSubFN = (temp >> 10) & 15; // 4 bits
                        _replace_result_int(result_subpkt, "Current SFN",
                                            iSysFN);
                        _replace_result_int(result_subpkt,
                                            "Current Subframe Number", iSubFN);

                        unsigned int utemp;
                        double pyfloat;

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[0]");
                        float RSRP0 = float((utemp >> 10) & 4095);
                        RSRP0 = RSRP0 * 0.0625 - 180.0;
                        pyfloat = RSRP0;
                        _replace_result(result_subpkt, "RSRP Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[1]");
                        float RSRP1 = float((utemp >> 12) & 4095);
                        RSRP1 = RSRP1 * 0.0625 - 180.0;
                        pyfloat = RSRP1;
                        _replace_result(result_subpkt, "RSRP Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSRP");
                        float RSRP = float((utemp >> 12) & 4095);
                        RSRP = RSRP * 0.0625 - 180.0;
                        pyfloat = RSRP;
                        _replace_result(result_subpkt, "RSRP", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[0]");
                        float RSRQ0 = float((utemp >> 12) & 1023);
                        RSRQ0 = RSRQ0 * 0.0625 - 30.0;
                        pyfloat = RSRQ0;
                        _replace_result(result_subpkt, "RSRQ Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[1]");
                        float RSRQ1 = float(utemp & 1023);
                        RSRQ1 = RSRQ1 * 0.0625 - 30.0;
                        pyfloat = RSRQ1;
                        _replace_result(result_subpkt, "RSRQ Rx[1]", pyfloat);

                        float RSRQ = float((utemp >> 20) & 1023);
                        RSRQ = RSRQ * 0.0625 - 30.0;
                        pyfloat = RSRQ;
                        _replace_result(result_subpkt, "RSRQ", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSSI Rx[0]");
                        float RSSI0 = float((utemp >> 10) & 2047);
                        RSSI0 = RSSI0 * 0.0625 - 110.0;
                        pyfloat = RSSI0;
                        _replace_result(result_subpkt, "RSSI Rx[0]", pyfloat);

                        float RSSI1 = float((utemp >> 21) & 2047);
                        RSSI1 = RSSI1 * 0.0625 - 110.0;
                        pyfloat = RSSI1;
                        _replace_result(result_subpkt, "RSSI Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSSI");
                        float RSSI = float(utemp & 2047);
                        RSSI = RSSI * 0.0625 - 110.0;
                        pyfloat = RSSI;
                        _replace_result(result_subpkt, "RSSI", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "FTL SNR Rx[0]");
                        float SNR0 = float(utemp & 511);
                        SNR0 = SNR0 * 0.1 - 20.0;
                        pyfloat = SNR0;
                        _replace_result(result_subpkt, "FTL SNR Rx[0]",
                                        pyfloat);

                        float SNR1 = float((utemp >> 9) & 511);
                        SNR1 = SNR1 * 0.1 - 20.0;
                        pyfloat = SNR1;
                        _replace_result(result_subpkt, "FTL SNR Rx[1]",
                                        pyfloat);

                        success = true;
                        break;
                    }
                    case 18: {
                        offset += _decode_by_fmt(
                            LtePhySubpktFmt_v1_Scmr_v18,
                            ARRAY_SIZE(LtePhySubpktFmt_v1_Scmr_v18, Fmt), b,
                            offset, length, result_subpkt);
                        int temp = _search_result_int(result_subpkt,
                                                      "Physical Cell ID");
                        int iPhyCellId = temp & 511;           // 9 bits
                        int iServingCellIdx = (temp >> 9) & 7; // 3 bits
                        _replace_result_int(result_subpkt, "Physical Cell ID",
                                            iPhyCellId);
                        _replace_result_int(result_subpkt, "Serving Cell Index",
                                            iServingCellIdx);
                        (void)_map_result_field_to_name(
                            result_subpkt, "Serving Cell Index",
                            ValueNameCellIndex,
                            ARRAY_SIZE(ValueNameCellIndex, ValueName),
                            "(MI)Unknown");

                        temp = _search_result_int(result_subpkt, "Current SFN");
                        int iSysFN = temp & 1023;       // 10 bits
                        int iSubFN = (temp >> 10) & 15; // 4 bits
                        _replace_result_int(result_subpkt, "Current SFN",
                                            iSysFN);
                        _replace_result_int(result_subpkt,
                                            "Current Subframe Number", iSubFN);

                        unsigned int utemp;
                        double pyfloat;

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[0]");
                        float RSRP0 = float((utemp >> 2) & 2047);
                        RSRP0 = RSRP0 * 0.0625 - 180.0;
                        pyfloat = RSRP0;
                        _replace_result(result_subpkt, "RSRP Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[1]");
                        float RSRP1 = float((utemp >> 4) & 2047);
                        RSRP1 = RSRP1 * 0.0625 - 180.0;
                        pyfloat = RSRP1;
                        _replace_result(result_subpkt, "RSRP Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSRP");
                        float RSRP = float((utemp >> 4) & 2047);
                        RSRP = RSRP * 0.0625 - 180.0;
                        pyfloat = RSRP;
                        _replace_result(result_subpkt, "RSRP", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[0]");
                        float RSRQ0 = float((utemp >> 4) & 511);
                        RSRQ0 = RSRQ0 * 0.0625 - 30.0;
                        pyfloat = RSRQ0;
                        _replace_result(result_subpkt, "RSRQ Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[1]");
                        float RSRQ1 = float((utemp >> 2) & 1023);
                        RSRQ1 = RSRQ1 * 0.0625 - 30.0;
                        pyfloat = RSRQ1;
                        _replace_result(result_subpkt, "RSRQ Rx[1]", pyfloat);

                        float RSRQ = float((utemp >> 12) & 1023);
                        RSRQ = RSRQ * 0.0625 - 30.0;
                        pyfloat = RSRQ;
                        _replace_result(result_subpkt, "RSRQ", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSSI Rx[0]");
                        float RSSI0 = float((utemp >> 2) & 2047);
                        RSSI0 = RSSI0 * 0.0625 - 110.0;
                        pyfloat = RSSI0;
                        _replace_result(result_subpkt, "RSSI Rx[0]", pyfloat);

                        float RSSI1 = float((utemp >> 13) & 2047);
                        RSSI1 = RSSI1 * 0.0625 - 110.0;
                        pyfloat = RSSI1;
                        _replace_result(result_subpkt, "RSSI Rx[1]", pyfloat);

                        // utemp = _search_result_uint(result_subpkt,
                        //         "RSSI");
                        float RSSI = float((utemp >> 13) & 2047);
                        RSSI = RSSI * 0.0625 - 110.0;
                        pyfloat = RSSI;
                        _replace_result(result_subpkt, "RSSI", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "FTL SNR Rx[0]");
                        float SNR0 = float(utemp & 511);
                        SNR0 = SNR0 * 0.1 - 20.0;
                        pyfloat = SNR0;
                        _replace_result(result_subpkt, "FTL SNR Rx[0]",
                                        pyfloat);

                        float SNR1 = float((utemp >> 9) & 511);
                        SNR1 = SNR1 * 0.1 - 20.0;
                        pyfloat = SNR1;
                        _replace_result(result_subpkt, "FTL SNR Rx[1]",
                                        pyfloat);

                        success = true;
                        break;
                    }
                    case 19: {
                        offset += _decode_by_fmt(
                            LtePhySubpktFmt_v1_Scmr_v19,
                            ARRAY_SIZE(LtePhySubpktFmt_v1_Scmr_v19, Fmt), b,
                            offset, length, result_subpkt);
                        int temp = _search_result_int(result_subpkt,
                                                      "Physical Cell ID");
                        int iPhyCellId = temp & 511;           // 9 bits
                        int iServingCellIdx = (temp >> 9) & 7; // 3 bits
                        int iIsServingCell = (temp >> 12) & 1; // 1 bit
                        _replace_result_int(result_subpkt, "Physical Cell ID",
                                            iPhyCellId);
                        _replace_result_int(result_subpkt, "Serving Cell Index",
                                            iServingCellIdx);
                        (void)_map_result_field_to_name(
                            result_subpkt, "Serving Cell Index",
                            ValueNameCellIndex,
                            ARRAY_SIZE(ValueNameCellIndex, ValueName),
                            "(MI)Unknown");
                        _replace_result_int(result_subpkt, "Is Serving Cell",
                                            iIsServingCell);

                        temp = _search_result_int(result_subpkt, "Current SFN");
                        int iSysFN = temp & 1023;       // 10 bits
                        int iSubFN = (temp >> 10) & 15; // 4 bits
                        _replace_result_int(result_subpkt, "Current SFN",
                                            iSysFN);
                        _replace_result_int(result_subpkt,
                                            "Current Subframe Number", iSubFN);

                        unsigned int utemp;
                        double pyfloat;

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[0]");
                        float RSRP0 = float((utemp >> 10) & 4095);
                        RSRP0 = RSRP0 * 0.0625 - 180.0;
                        pyfloat = RSRP0;
                        _replace_result(result_subpkt, "RSRP Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[1]");
                        float RSRP1 = float((utemp >> 12) & 4095);
                        RSRP1 = RSRP1 * 0.0625 - 180.0;
                        pyfloat = RSRP1;
                        _replace_result(result_subpkt, "RSRP Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSRP");
                        float RSRP = float((utemp >> 12) & 4095);
                        RSRP = RSRP * 0.0625 - 180.0;
                        pyfloat = RSRP;
                        _replace_result(result_subpkt, "RSRP", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[0]");
                        float RSRQ0 = float((utemp >> 12) & 1023);
                        RSRQ0 = RSRQ0 * 0.0625 - 30.0;
                        pyfloat = RSRQ0;
                        _replace_result(result_subpkt, "RSRQ Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[1]");
                        float RSRQ1 = float(utemp & 1023);
                        RSRQ1 = RSRQ1 * 0.0625 - 30.0;
                        pyfloat = RSRQ1;
                        _replace_result(result_subpkt, "RSRQ Rx[1]", pyfloat);

                        float RSRQ = float((utemp >> 20) & 1023);
                        RSRQ = RSRQ * 0.0625 - 30.0;
                        pyfloat = RSRQ;
                        _replace_result(result_subpkt, "RSRQ", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSSI Rx[0]");
                        float RSSI0 = float((utemp >> 10) & 2047);
                        RSSI0 = RSSI0 * 0.0625 - 110.0;
                        pyfloat = RSSI0;
                        _replace_result(result_subpkt, "RSSI Rx[0]", pyfloat);

                        float RSSI1 = float((utemp >> 21) & 2047);
                        RSSI1 = RSSI1 * 0.0625 - 110.0;
                        pyfloat = RSSI1;
                        _replace_result(result_subpkt, "RSSI Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSSI");
                        float RSSI = float(utemp & 2047);
                        RSSI = RSSI * 0.0625 - 110.0;
                        pyfloat = RSSI;
                        _replace_result(result_subpkt, "RSSI", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "FTL SNR Rx[0]");
                        float SNR0 = float(utemp & 511);
                        SNR0 = SNR0 * 0.1 - 20.0;
                        pyfloat = SNR0;
                        _replace_result(result_subpkt, "FTL SNR Rx[0]",
                                        pyfloat);

                        float SNR1 = float((utemp >> 9) & 511);
                        SNR1 = SNR1 * 0.1 - 20.0;
                        pyfloat = SNR1;
                        _replace_result(result_subpkt, "FTL SNR Rx[1]",
                                        pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "Post IC RSRQ");
                        float postICRSRQ = float(utemp * 0.0625 - 30);
                        pyfloat = postICRSRQ;
                        _replace_result(result_subpkt, "Post IC RSRQ", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "Projected SIR");
                        int SIR = 0;
                        if (utemp & (1 << 31)) {
                            SIR = utemp - 4294967296;
                        } else {
                            SIR = utemp;
                        }
                        SIR = SIR / 16;
                        _replace_result_int(result_subpkt, "Projected SIR",
                                            SIR);

                        success = true;
                        break;
                    }
                    case 22: {
                        offset += _decode_by_fmt(
                            LtePhySubpktFmt_v1_Scmr_v22,
                            ARRAY_SIZE(LtePhySubpktFmt_v1_Scmr_v22, Fmt), b,
                            offset, length, result_subpkt);
                        int temp = _search_result_int(result_subpkt,
                                                      "Physical Cell ID");
                        int iPhyCellId = temp & 511;           // 9 bits
                        int iServingCellIdx = (temp >> 9) & 7; // 3 bits
                        int iIsServingCell = (temp >> 12) & 1; // 1 bit
                        _replace_result_int(result_subpkt, "Physical Cell ID",
                                            iPhyCellId);
                        _replace_result_int(result_subpkt, "Serving Cell Index",
                                            iServingCellIdx);
                        (void)_map_result_field_to_name(
                            result_subpkt, "Serving Cell Index",
                            ValueNameCellIndex,
                            ARRAY_SIZE(ValueNameCellIndex, ValueName),
                            "(MI)Unknown");
                        _replace_result_int(result_subpkt, "Is Serving Cell",
                                            iIsServingCell);

                        temp = _search_result_int(result_subpkt, "Current SFN");
                        int iSysFN = temp & 1023;       // 10 bits
                        int iSubFN = (temp >> 10) & 15; // 4 bits
                        _replace_result_int(result_subpkt, "Current SFN",
                                            iSysFN);
                        _replace_result_int(result_subpkt,
                                            "Current Subframe Number", iSubFN);

                        unsigned int utemp;
                        double pyfloat;

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[0]");
                        float RSRP0 = float((utemp >> 10) & 4095);
                        RSRP0 = RSRP0 * 0.0625 - 180.0;
                        pyfloat = RSRP0;
                        _replace_result(result_subpkt, "RSRP Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[1]");
                        float RSRP1 = float((utemp >> 12) & 4095);
                        RSRP1 = RSRP1 * 0.0625 - 180.0;
                        pyfloat = RSRP1;
                        _replace_result(result_subpkt, "RSRP Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSRP");
                        float RSRP = float((utemp >> 12) & 4095);
                        RSRP = RSRP * 0.0625 - 180.0;
                        pyfloat = RSRP;
                        _replace_result(result_subpkt, "RSRP", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[0]");
                        float RSRQ0 = float((utemp >> 12) & 1023);
                        RSRQ0 = RSRQ0 * 0.0625 - 30.0;
                        pyfloat = RSRQ0;
                        _replace_result(result_subpkt, "RSRQ Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[1]");
                        float RSRQ1 = float(utemp & 1023);
                        RSRQ1 = RSRQ1 * 0.0625 - 30.0;
                        pyfloat = RSRQ1;
                        _replace_result(result_subpkt, "RSRQ Rx[1]", pyfloat);

                        float RSRQ = float((utemp >> 10) & 1023);
                        RSRQ = RSRQ * 0.0625 - 30.0;
                        pyfloat = RSRQ;
                        _replace_result(result_subpkt, "RSRQ", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSSI Rx[0]");
                        float RSSI0 = float((utemp >> 10) & 2047);
                        RSSI0 = RSSI0 * 0.0625 - 110.0;
                        pyfloat = RSSI0;
                        _replace_result(result_subpkt, "RSSI Rx[0]", pyfloat);

                        float RSSI1 = float((utemp >> 21) & 2047);
                        RSSI1 = RSSI1 * 0.0625 - 110.0;
                        pyfloat = RSSI1;
                        _replace_result(result_subpkt, "RSSI Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSSI");
                        float RSSI = float(utemp & 2047);
                        RSSI = RSSI * 0.0625 - 110.0;
                        pyfloat = RSSI;
                        _replace_result(result_subpkt, "RSSI", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "FTL SNR Rx[0]");
                        float SNR0 = float(utemp & 511);
                        SNR0 = SNR0 * 0.1 - 20.0;
                        pyfloat = SNR0;
                        _replace_result(result_subpkt, "FTL SNR Rx[0]",
                                        pyfloat);

                        float SNR1 = float((utemp >> 9) & 511);
                        SNR1 = SNR1 * 0.1 - 20.0;
                        pyfloat = SNR1;
                        _replace_result(result_subpkt, "FTL SNR Rx[1]",
                                        pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "Post IC RSRQ");
                        float postICRSRQ = float(utemp * 0.0625 - 30);
                        pyfloat = postICRSRQ;
                        _replace_result(result_subpkt, "Post IC RSRQ", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "Projected SIR");
                        int SIR = 0;
                        if (utemp & (1 << 31)) {
                            SIR = utemp - 4294967296;
                        } else {
                            SIR = utemp;
                        }
                        SIR = SIR / 16;
                        _replace_result_int(result_subpkt, "Projected SIR",
                                            SIR);

                        success = true;
                        break;
                    }
                    case 35: {
                        offset += _decode_by_fmt(
                            LtePhySubpktFmt_v1_Scmr_v35,
                            ARRAY_SIZE(LtePhySubpktFmt_v1_Scmr_v35, Fmt), b,
                            offset, length, result_subpkt);
                        int temp = _search_result_int(result_subpkt,
                                                      "Physical Cell ID");
                        int iPhyCellId = temp & 511;           // 9 bits
                        int iServingCellIdx = (temp >> 9) & 7; // 3 bits
                        int iIsServingCell = (temp >> 12) & 1; // 1 bit
                        _replace_result_int(result_subpkt, "Physical Cell ID",
                                            iPhyCellId);
                        _replace_result_int(result_subpkt, "Serving Cell Index",
                                            iServingCellIdx);
                        (void)_map_result_field_to_name(
                            result_subpkt, "Serving Cell Index",
                            ValueNameCellIndex,
                            ARRAY_SIZE(ValueNameCellIndex, ValueName),
                            "(MI)Unknown");
                        _replace_result_int(result_subpkt, "Is Serving Cell",
                                            iIsServingCell);

                        temp = _search_result_int(result_subpkt, "Current SFN");
                        int iSysFN = temp & 1023;       // 10 bits
                        int iSubFN = (temp >> 10) & 15; // 4 bits
                        _replace_result_int(result_subpkt, "Current SFN",
                                            iSysFN);
                        _replace_result_int(result_subpkt,
                                            "Current Subframe Number", iSubFN);

                        unsigned int utemp;
                        double pyfloat;

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[0]");
                        float RSRP0 = float((utemp >> 10) & 4095);
                        RSRP0 = RSRP0 * 0.0625 - 180.0;
                        pyfloat = RSRP0;
                        _replace_result(result_subpkt, "RSRP Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[1]");
                        float RSRP1 = float((utemp >> 12) & 4095);
                        RSRP1 = RSRP1 * 0.0625 - 180.0;
                        pyfloat = RSRP1;
                        _replace_result(result_subpkt, "RSRP Rx[1]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[2]");
                        float RSRP2 = float((utemp >> 12) & 4095);
                        RSRP2 = RSRP2 * 0.0625 - 180.0;
                        pyfloat = RSRP2;
                        _replace_result(result_subpkt, "RSRP Rx[2]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[3]");
                        float RSRP3 = float(utemp & 4095);
                        RSRP3 = RSRP3 * 0.0625 - 180.0;
                        pyfloat = RSRP3;
                        _replace_result(result_subpkt, "RSRP Rx[3]", pyfloat);

                        float RSRP = float((utemp >> 12) & 4095);
                        RSRP = (RSRP + 640) * 0.0625 - 180.0;
                        pyfloat = RSRP;
                        _replace_result(result_subpkt, "RSRP", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "Filtered RSRP");
                        float fRSRP = float((utemp >> 12) & 4095);
                        fRSRP = fRSRP * 0.0625 - 180.0;
                        pyfloat = fRSRP;
                        _replace_result(result_subpkt, "Filtered RSRP",
                                        pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[0]");
                        float RSRQ0 = float((utemp)&1023);
                        RSRQ0 = RSRQ0 * 0.0625 - 30.0;
                        pyfloat = RSRQ0;
                        _replace_result(result_subpkt, "RSRQ Rx[0]", pyfloat);

                        float RSRQ1 = float((utemp >> 20) & 1023);
                        RSRQ1 = RSRQ1 * 0.0625 - 30.0;
                        pyfloat = RSRQ1;
                        _replace_result(result_subpkt, "RSRQ Rx[1]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[2]");
                        float RSRQ2 = float((utemp >> 10) & 1023);
                        RSRQ2 = RSRQ2 * 0.0625 - 30.0;
                        pyfloat = RSRQ2;
                        _replace_result(result_subpkt, "RSRQ Rx[2]", pyfloat);

                        float RSRQ3 = float((utemp >> 20) & 1023);
                        RSRQ3 = RSRQ3 * 0.0625 - 30.0;
                        pyfloat = RSRQ3;
                        _replace_result(result_subpkt, "RSRQ Rx[3]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSRQ");
                        float RSRQ = float((utemp)&1023);
                        RSRQ = RSRQ * 0.0625 - 30.0;
                        pyfloat = RSRQ;
                        _replace_result(result_subpkt, "RSRQ", pyfloat);

                        float fRSRQ = float((utemp >> 20) & 4095);
                        fRSRQ = fRSRQ * 0.0625 - 30.0;
                        pyfloat = fRSRQ;
                        _replace_result(result_subpkt, "Filtered RSRQ",
                                        pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSSI Rx[0]");
                        float RSSI0 = float(utemp & 2047);
                        RSSI0 = RSSI0 * 0.0625 - 110.0;
                        pyfloat = RSSI0;
                        _replace_result(result_subpkt, "RSSI Rx[0]", pyfloat);

                        float RSSI1 = float((utemp >> 11) & 2047);
                        RSSI1 = RSSI1 * 0.0625 - 110.0;
                        pyfloat = RSSI1;
                        _replace_result(result_subpkt, "RSSI Rx[1]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSSI Rx[2]");
                        float RSSI2 = float(utemp & 2047);
                        RSSI2 = RSSI2 * 0.0625 - 110.0;
                        pyfloat = RSSI2;
                        _replace_result(result_subpkt, "RSSI Rx[2]", pyfloat);

                        float RSSI3 = float((utemp >> 11) & 2047);
                        RSSI3 = RSSI3 * 0.0625 - 110.0;
                        pyfloat = RSSI3;
                        _replace_result(result_subpkt, "RSSI Rx[3]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSSI");
                        float RSSI = float(utemp & 2047);
                        RSSI = RSSI * 0.0625 - 110.0;
                        pyfloat = RSSI;
                        _replace_result(result_subpkt, "RSSI", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "FTL SNR Rx[0]");
                        float SNR0 = float(utemp & 511);
                        SNR0 = SNR0 * 0.1 - 20.0;
                        pyfloat = SNR0;
                        _replace_result(result_subpkt, "FTL SNR Rx[0]",
                                        pyfloat);

                        float SNR1 = float((utemp >> 9) & 511);
                        SNR1 = SNR1 * 0.1 - 20.0;
                        pyfloat = SNR1;
                        _replace_result(result_subpkt, "FTL SNR Rx[1]",
                                        pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "FTL SNR Rx[2]");
                        float SNR2 = float(utemp & 511);
                        SNR2 = SNR2 * 0.1 - 20.0;
                        pyfloat = SNR2;
                        _replace_result(result_subpkt, "FTL SNR Rx[2]",
                                        pyfloat);

                        float SNR3 = float((utemp >> 9) & 511);
                        SNR3 = SNR3 * 0.1 - 20.0;
                        pyfloat = SNR3;
                        _replace_result(result_subpkt, "FTL SNR Rx[3]",
                                        pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "Post IC RSRQ");
                        float postICRSRQ = float(utemp * 0.0625 - 30);
                        pyfloat = postICRSRQ;
                        _replace_result(result_subpkt, "Post IC RSRQ", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "Projected SIR");
                        int SIR = 0;
                        if (utemp & (1 << 31)) {
                            SIR = utemp - 4294967296;
                        } else {
                            SIR = utemp;
                        }
                        SIR = SIR / 16;
                        _replace_result_int(result_subpkt, "Projected SIR",
                                            SIR);

                        success = true;
                        break;
                    }
                    case 36: {
                        offset += _decode_by_fmt(
                            LtePhySubpktFmt_v1_Scmr_v36,
                            ARRAY_SIZE(LtePhySubpktFmt_v1_Scmr_v36, Fmt), b,
                            offset, length, result_subpkt);
                        int temp = _search_result_int(result_subpkt,
                                                      "Physical Cell ID");
                        int iPhyCellId = temp & 511;           // 9 bits
                        int iServingCellIdx = (temp >> 9) & 7; // 3 bits
                        int iIsServingCell = (temp >> 12) & 1; // 1 bit
                        _replace_result_int(result_subpkt, "Physical Cell ID",
                                            iPhyCellId);
                        _replace_result_int(result_subpkt, "Serving Cell Index",
                                            iServingCellIdx);
                        (void)_map_result_field_to_name(
                            result_subpkt, "Serving Cell Index",
                            ValueNameCellIndex,
                            ARRAY_SIZE(ValueNameCellIndex, ValueName),
                            "(MI)Unknown");
                        _replace_result_int(result_subpkt, "Is Serving Cell",
                                            iIsServingCell);

                        temp = _search_result_int(result_subpkt, "Current SFN");
                        int iSysFN = temp & 1023;       // 10 bits
                        int iSubFN = (temp >> 10) & 15; // 4 bits
                        _replace_result_int(result_subpkt, "Current SFN",
                                            iSysFN);
                        _replace_result_int(result_subpkt,
                                            "Current Subframe Number", iSubFN);

                        unsigned int utemp;
                        double pyfloat;

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[0]");
                        float RSRP0 = float((utemp >> 10) & 4095);
                        RSRP0 = RSRP0 * 0.0625 - 180.0;
                        pyfloat = RSRP0;
                        _replace_result(result_subpkt, "RSRP Rx[0]", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRP Rx[1]");
                        float RSRP1 = float((utemp >> 12) & 4095);
                        RSRP1 = RSRP1 * 0.0625 - 180.0;
                        pyfloat = RSRP1;
                        _replace_result(result_subpkt, "RSRP Rx[1]", pyfloat);

                        utemp = _search_result_uint(result_subpkt, "RSRP");
                        float RSRP = float((utemp)&4095);
                        RSRP = RSRP * 0.0625 - 180.0;
                        pyfloat = RSRP;
                        _replace_result(result_subpkt, "RSRP", pyfloat);

                        utemp =
                            _search_result_uint(result_subpkt, "RSRQ Rx[0]");
                        float RSRQ0 = float((utemp)&1023);
                        RSRQ0 = RSRQ0 * 0.0625 - 30.0;
                        pyfloat = RSRQ0;
                        _replace_result(result_subpkt, "RSRQ Rx[0]", pyfloat);

                        float RSRQ1 = float((utemp >> 20) & 1023);
                        RSRQ1 = RSRQ1 * 0.0625 - 30.0;
                        pyfloat = RSRQ1;
                        _replace_result(result_subpkt, "RSRQ Rx[1]", pyfloat);

                        float RSRQ = float((utemp)&1023);
                        RSRQ = RSRQ * 0.0625 - 30.0;
                        pyfloat = RSRQ;
                        _replace_result(result_subpkt, "RSRQ", pyfloat);

                        success = true;
                        break;
                    }

                    default:
                        break;
                    }
                }
                // TODO: replace type ID to name.

                if (success) {
                    result_allpkts.push_back(result_subpkt);
                } else {
                    printf("(MI)Unknown LTE PHY Subpacket version: 0x%x - %d\n",
                           subpkt_id, subpkt_ver);
                }
            }
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PHY packet version: 0x%x\n", pkt_ver);
        return 0;
    }
}

static int _decode_lte_phy_irat_cdma_subpkt(const char *b, int offset,
                                            size_t length, json &result) {

    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Subpacket count");

    switch (pkt_ver) {
    case 1:
        for (int i = 0; i < n_subpkt; i++) {

            json result_subpkt;
            // Decode subpacket header
            offset += _decode_by_fmt(LtePhyIratSubPktFmt,
                                     ARRAY_SIZE(LtePhyIratSubPktFmt, Fmt), b,
                                     offset, length, result_subpkt);
            // Decode payload
            // int subpkt_id = _search_result_int(result_subpkt, "Subpacket
            // ID"); int subpkt_ver = _search_result_int(result_subpkt,
            // "Version"); int subpkt_size = _search_result_int(result_subpkt,
            // "Subpacket size");

            json result_subpkt_cdma;
            offset += _decode_by_fmt(LtePhyIratCDMACellFmt,
                                     ARRAY_SIZE(LtePhyIratCDMACellFmt, Fmt), b,
                                     offset, length, result_subpkt_cdma);
            int n_pilot =
                _search_result_int(result_subpkt_cdma, "Number of Pilots");
            int band = _search_result_int(result_subpkt_cdma, "Band");
            for (int j = 0; j < n_pilot; j++) {
                json result_subpkt_cdma_pilot;
                offset +=
                    _decode_by_fmt(LtePhyIratCDMACellPilotFmt,
                                   ARRAY_SIZE(LtePhyIratCDMACellPilotFmt, Fmt),
                                   b, offset, length, result_subpkt_cdma_pilot);
                char name[64];
                sprintf(name, "Pilot_%d", j);
                result_subpkt_cdma[name] = result_subpkt_cdma_pilot;
            }
            char name[64];
            sprintf(name, "Band_%d", band);
            result_subpkt["CDMA"] = result_subpkt_cdma;

            char name2[64];
            sprintf(name2, "Subpacket_%d", i);
            result[name2] = result_subpkt;
        }
        break;

    default:
        break;
    }

    return offset - start;
}

static int _decode_lte_phy_irat_subpkt(const char *b, int offset, size_t length,
                                       json &result) {

    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Subpacket count");

    switch (pkt_ver) {
    case 1:
        for (int i = 0; i < n_subpkt; i++) {

            json result_subpkt;
            // Decode subpacket header
            offset += _decode_by_fmt(LtePhyIratSubPktFmt,
                                     ARRAY_SIZE(LtePhyIratSubPktFmt, Fmt), b,
                                     offset, length, result_subpkt);
            // Decode payload
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            // int subpkt_ver = _search_result_int(result_subpkt, "Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket size");
            switch (subpkt_id) {
            case LtePhyIratType_WCDMA: {
                json result_subpkt_wcdma;
                offset += _decode_by_fmt(LtePhyIratWCDMAFmt,
                                         ARRAY_SIZE(LtePhyIratWCDMAFmt, Fmt), b,
                                         offset, length, result_subpkt_wcdma);
                int n_freq = _search_result_int(result_subpkt_wcdma,
                                                "Number of frequencies");
                for (int j = 0; j < n_freq; j++) {
                    json result_subpkt_wcdma_freq;
                    offset += _decode_by_fmt(
                        LtePhyIratWCDMACellMetaFmt,
                        ARRAY_SIZE(LtePhyIratWCDMACellMetaFmt, Fmt), b, offset,
                        length, result_subpkt_wcdma_freq);
                    // int freq = _search_result_int(result_subpkt_wcdma_freq,
                    // "Frequency");
                    int n_cell = _search_result_int(result_subpkt_wcdma_freq,
                                                    "Number of cells");
                    for (int k = 0; k < n_cell; k++) {
                        json result_subpkt_wcdma_cell;
                        offset += _decode_by_fmt(
                            LtePhyIratWCDMACellFmt,
                            ARRAY_SIZE(LtePhyIratWCDMACellFmt, Fmt), b, offset,
                            length, result_subpkt_wcdma_cell);

                        char name[64];
                        sprintf(name, "CellPacket_%d", k);
                        result_subpkt_wcdma_freq[name] =
                            result_subpkt_wcdma_cell;
                    }
                    char name[64];
                    sprintf(name, "FrequencyPacket_%d", j);
                    result_subpkt_wcdma[name] = result_subpkt_wcdma_freq;
                }

                result_subpkt["WCDMA"] = result_subpkt_wcdma;
                break;
            }

            default: {
                offset += subpkt_size - 4;
                break;
            }
            }

            char name[64];
            sprintf(name, "Subpacket_%d", i);
            result[name] = result_subpkt;
        }

    default:
        break;
    }

    return offset - start;
}

static int _decode_lte_pdcp_dl_srb_integrity_data_pdu(const char *b, int offset,
                                                      size_t length,
                                                      json &result) {
    (void)length;
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 1:
        break;
    default:
        printf("(MI)Unknown LTE PDCP DL SRB Integrity Data PDU version: %d\n",
               pkt_ver);
        return 0;
    }
    int pktCount = _search_result_int(result, "Num SubPkt");
    if (pktCount != 1) {
        printf(
            "(MI)Unknown LTE PDCP DL SRB Integrity Data PDU Num SubPkt: %d\n",
            pktCount);
        return 0;
    }

    int pdu_length = _search_result_int(result, "PDU Size");

    //result["Msg"] = std::string("raw_msg/LTE-PDCP_DL_SRB") +
    //                make_string(b + offset, pdu_length);
	//"LTE-PDCP_DL_SRB": 300
	result["Msg"] = binary_decode(300, b + offset, pdu_length);
    return (offset - start) + pdu_length;
}

static int _decode_lte_pdcp_ul_srb_integrity_data_pdu(const char *b, int offset,
                                                      size_t length,
                                                      json &result) {
    (void)length;
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 1:
        break;
    default:
        printf("(MI)Unknown LTE PDCP UL SRB Integrity Data PDU version: %d\n",
               pkt_ver);
        return 0;
    }
    int pktCount = _search_result_int(result, "Num SubPkt");
    if (pktCount != 1) {
        printf(
            "(MI)Unknown LTE PDCP UL SRB Integrity Data PDU Num SubPkt: %d\n",
            pktCount);
        return 0;
    }

    int pdu_length = _search_result_int(result, "PDU Size");

    //result["Msg"] = std::string("raw_msg/LTE-PDCP_UL_SRB") +
    //                make_string(b + offset, pdu_length);
	//"LTE-PDCP_UL_SRB": 301
	result["Msg"] = binary_decode(301, b + offset, pdu_length);
    return (offset - start) + pdu_length;
}

//------------------------------------------------------
// Jie
static int _decode_lte_mac_configuration_subpkt(const char *b, int offset,
                                                size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num SubPkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            // Decode subpacket header
            int start_subpkt = offset;
            offset += _decode_by_fmt(
                LteMacConfiguration_SubpktHeader,
                ARRAY_SIZE(LteMacConfiguration_SubpktHeader, Fmt), b, offset,
                length, result_subpkt);
            // Decode payload
            int subpkt_id = _search_result_int(result_subpkt, "SubPacket ID");
            int subpkt_ver = _search_result_int(result_subpkt, "Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "SubPacket Size");
            const char *type_name = search_name(
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                subpkt_id);
            (void)_map_result_field_to_name(
                result_subpkt, "SubPacket ID",
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                "(MI)Unknown");

            if (type_name == NULL) { // not found
                printf("(MI)Unknown LTE MAC Configuration Subpacket ID: 0x%x\n",
                       subpkt_id);
            } else {
                bool success = false;
                switch (subpkt_id) {
                case 0: // Config Type Subpacket
                    if (subpkt_ver == 1) {
                        offset += _decode_by_fmt(
                            LteMacConfigurationSubpkt_ConfigType,
                            ARRAY_SIZE(LteMacConfigurationSubpkt_ConfigType,
                                       Fmt),
                            b, offset, length, result_subpkt);
                        (void)_map_result_field_to_name(
                            result_subpkt, "Config reason",
                            LteMacConfigurationConfigType_ConfigReason,
                            ARRAY_SIZE(
                                LteMacConfigurationConfigType_ConfigReason,
                                ValueName),
                            "NORMAL");
                        success = true;
                    }
                    break;
                case 1: // DL Config Subpacket
                    if (subpkt_ver == 1) {
                        offset += _decode_by_fmt(
                            LteMacConfigurationSubpkt_DLConfig,
                            ARRAY_SIZE(LteMacConfigurationSubpkt_DLConfig, Fmt),
                            b, offset, length, result_subpkt);
                        success = true;
                    }
                    break;
                case 2: // UL Config Subpacket
                    if (subpkt_ver == 1) {
                        offset += _decode_by_fmt(
                            LteMacConfigurationSubpkt_ULConfig,
                            ARRAY_SIZE(LteMacConfigurationSubpkt_ULConfig, Fmt),
                            b, offset, length, result_subpkt);
                        success = true;
                    }
                    break;
                case 3: // RACH Config Subpacket
                    if (subpkt_ver == 1) {
                        offset += _decode_by_fmt(
                            LteMacConfigurationSubpkt_RACHConfig,
                            ARRAY_SIZE(LteMacConfigurationSubpkt_RACHConfig,
                                       Fmt),
                            b, offset, length, result_subpkt);
                        success = true;
                    }
                    break;
                case 4: // LC Config Subpacket
                    if (subpkt_ver == 1) {
                        offset += _decode_by_fmt(
                            LteMacConfigurationSubpkt_LCConfig,
                            ARRAY_SIZE(LteMacConfigurationSubpkt_LCConfig, Fmt),
                            b, offset, length, result_subpkt);
                        int num_LC = _search_result_int(
                            result_subpkt, "Number of added/modified LC");
                        int start_LC = offset;

                        for (int j = 0; j < num_LC; j++) {
                            json result_subpkt_LC;
                            offset += _decode_by_fmt(
                                LteMacConfiguration_LCConfig_LC,
                                ARRAY_SIZE(LteMacConfiguration_LCConfig_LC,
                                           Fmt),
                                b, offset, length, result_subpkt_LC);
                            result_subpkt["added/modified LC"] =
                                result_subpkt_LC;
                        }
                        offset += 290 - (offset - start_LC);
                        success = true;
                    }
                    break;
                case 13: // eMBMBS Config SubPacket
                    if (subpkt_ver == 1) {
                        offset += _decode_by_fmt(
                            LteMacConfigurationSubpkt_eMBMSConfig,
                            ARRAY_SIZE(LteMacConfigurationSubpkt_eMBMSConfig,
                                       Fmt),
                            b, offset, length, result_subpkt);
                        success = true;
                    }
                    break;
                case 14: // All Rach Config SubPacket
                    if (subpkt_ver == 1) {
                        offset += _decode_by_fmt(
                            LteMacConfigurationSubpkt_All_Rach_Config,
                            ARRAY_SIZE(
                                LteMacConfigurationSubpkt_All_Rach_Config, Fmt),
                            b, offset, length, result_subpkt);
                        success = true;
                    }
                    break;
                case 18:
                    if (subpkt_ver == 1) {
                        offset += _decode_by_fmt(
                            LteMacConfigurationSubpkt_ELS,
                            ARRAY_SIZE(LteMacConfigurationSubpkt_ELS, Fmt), b,
                            offset, length, result_subpkt);
                        success = true;
                    }
                    break;
                default:
                    break;
                }
                // TODO: replace type ID to name.

                if (success) {
                    result_allpkts.push_back(result_subpkt);
                } else {
                    printf("(MI)Unknown LTE MAC Configuration Subpacket "
                           "version: 0x%x - %d\n",
                           subpkt_id, subpkt_ver);
                }
            }
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE MAC Configuration packet version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}

//------------------------------------------------------
// TODO: Jie
static int _decode_lte_mac_ul_transportblock_subpkt(const char *b, int offset,
                                                    size_t length,
                                                    json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num SubPkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            // Decode subpacket header
            offset += _decode_by_fmt(
                LteMacULTransportBlock_SubpktHeaderFmt,
                ARRAY_SIZE(LteMacULTransportBlock_SubpktHeaderFmt, Fmt), b,
                offset, length, result_subpkt);
            // Decode payload
            int subpkt_id = _search_result_int(result_subpkt, "SubPacket ID");
            int subpkt_ver = _search_result_int(result_subpkt, "Version");
            int subpkt_nsample =
                _search_result_int(result_subpkt, "Num Samples");
            const char *type_name = search_name(
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                subpkt_id);
            (void)_map_result_field_to_name(
                result_subpkt, "SubPacket ID",
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                "(MI)Unknown");

            if (type_name == NULL) { // not found
                printf("(MI)Unknown LTE MAC Uplink Transport Block Subpacket "
                       "ID: 0x%x\n",
                       subpkt_id);
            } else {
                bool success = false;
                json result_sample_list;
                switch (subpkt_ver) {
                case 1: {
                    // UL Transport Block Subpacket V1
                    for (int j = 0; j < subpkt_nsample; j++) {
                        json result_subpkt_sample;
                        offset += _decode_by_fmt(
                            LteMacULTransportBlock_SubpktV1_SampleFmt,
                            ARRAY_SIZE(
                                LteMacULTransportBlock_SubpktV1_SampleFmt, Fmt),
                            b, offset, length, result_subpkt_sample);
                        (void)_map_result_field_to_name(
                            result_subpkt_sample, "BSR event", BSREvent,
                            ARRAY_SIZE(BSREvent, ValueName), "(MI)Unknown");
                        (void)_map_result_field_to_name(
                            result_subpkt_sample, "BSR trig", BSRTrig,
                            ARRAY_SIZE(BSRTrig, ValueName), "(MI)Unknown");
                        (void)_map_result_field_to_name(
                            result_subpkt_sample, "RNTI Type",
                            ValueNameRNTIType,
                            ARRAY_SIZE(ValueNameRNTIType, ValueName),
                            "(MI)Unknown");
                        int temp =
                            _search_result_int(result_subpkt_sample, "Sub-FN");
                        int iSubFN = temp & 15; // 4 bits
                        int iSFN = (temp >> 4);
                        _replace_result_int(result_subpkt_sample, "Sub-FN",
                                            iSubFN);
                        _replace_result_int(result_subpkt_sample, "SFN", iSFN);
                        offset +=
                            _search_result_int(result_subpkt_sample, "HDR LEN");
                        result_sample_list.push_back(result_subpkt_sample);
                    }
                    success = true;
                    break;
                }
                case 2: {
                    // UL Transport Block Subpacket V2
                    for (int j = 0; j < subpkt_nsample; j++) {
                        json result_subpkt_sample;
                        offset += _decode_by_fmt(
                            LteMacULTransportBlock_SubpktV2_SampleFmt,
                            ARRAY_SIZE(
                                LteMacULTransportBlock_SubpktV2_SampleFmt, Fmt),
                            b, offset, length, result_subpkt_sample);
                        (void)_map_result_field_to_name(
                            result_subpkt_sample, "BSR event", BSREvent,
                            ARRAY_SIZE(BSREvent, ValueName), "(MI)Unknown");
                        (void)_map_result_field_to_name(
                            result_subpkt_sample, "BSR trig", BSRTrig,
                            ARRAY_SIZE(BSRTrig, ValueName), "(MI)Unknown");
                        (void)_map_result_field_to_name(
                            result_subpkt_sample, "RNTI Type",
                            ValueNameRNTIType,
                            ARRAY_SIZE(ValueNameRNTIType, ValueName),
                            "(MI)Unknown");
                        int temp =
                            _search_result_int(result_subpkt_sample, "Sub-FN");
                        int iSubFN = temp & 15; // 4 bits
                        int iSFN = (temp >> 4);
                        _replace_result_int(result_subpkt_sample, "Sub-FN",
                                            iSubFN);
                        _replace_result_int(result_subpkt_sample, "SFN", iSFN);
                        offset +=
                            _search_result_int(result_subpkt_sample, "HDR LEN");
                        result_sample_list.push_back(result_subpkt_sample);
                    }
                    success = true;
                    break;
                }
                default:
                    break;
                }
                if (success) {
                    result_subpkt["Samples"] = result_sample_list;
                } else {
                    printf("(MI)Unknown LTE MAC Uplink Transport Block "
                           "Subpacket version: 0x%x - %d\n",
                           subpkt_id, subpkt_ver);
                }
                result_allpkts.push_back(result_subpkt);
            }
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf(
            "(MI)Unknown LTE MAC Uplink Transport Block packet version: 0x%x\n",
            pkt_ver);
        return 0;
    }
}

//------------------------------------------------------
// Jie
static int _decode_lte_mac_dl_transportblock_subpkt(const char *b, int offset,
                                                    size_t length,
                                                    json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num SubPkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            // Decode subpacket header
            offset += _decode_by_fmt(
                LteMacDLTransportBlock_SubpktHeaderFmt,
                ARRAY_SIZE(LteMacDLTransportBlock_SubpktHeaderFmt, Fmt), b,
                offset, length, result_subpkt);
            // Decode payload
            int subpkt_id = _search_result_int(result_subpkt, "SubPacket ID");
            int subpkt_ver = _search_result_int(result_subpkt, "Version");
            int subpkt_nsample =
                _search_result_int(result_subpkt, "Num Samples");
            const char *type_name = search_name(
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                subpkt_id);
            (void)_map_result_field_to_name(
                result_subpkt, "SubPacket ID",
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                "(MI)Unknown");

            if (type_name == NULL) { // not found
                printf("(MI)Unknown LTE MAC Downlink Transport Block Subpacket "
                       "ID: 0x%x\n",
                       subpkt_id);
            } else {
                bool success = false;
                json result_sample_list;
                switch (subpkt_ver) {
                case 2: {
                    // DL Transport Block Subpacket
                    for (int j = 0; j < subpkt_nsample; j++) {
                        json result_subpkt_sample;
                        offset += _decode_by_fmt(
                            LteMacDLTransportBlock_SubpktV2_SampleFmt,
                            ARRAY_SIZE(
                                LteMacDLTransportBlock_SubpktV2_SampleFmt, Fmt),
                            b, offset, length, result_subpkt_sample);
                        (void)_map_result_field_to_name(
                            result_subpkt_sample, "RNTI Type",
                            ValueNameRNTIType,
                            ARRAY_SIZE(ValueNameRNTIType, ValueName),
                            "(MI)Unknown");
                        int temp =
                            _search_result_int(result_subpkt_sample, "Sub-FN");
                        int iSubFN = temp & 15; // 4 bits
                        int iSFN = (temp >> 4);
                        _replace_result_int(result_subpkt_sample, "Sub-FN",
                                            iSubFN);
                        _replace_result_int(result_subpkt_sample, "SFN", iSFN);
                        offset +=
                            _search_result_int(result_subpkt_sample, "HDR LEN");
                        result_sample_list.push_back(result_subpkt_sample);
                    }
                    success = true;
                    break;
                }
                case 4: {
                    // DL Transport Block Subpacket
                    for (int j = 0; j < subpkt_nsample; j++) {
                        json result_subpkt_sample;
                        offset += _decode_by_fmt(
                            LteMacDLTransportBlock_SubpktV4_SampleFmt,
                            ARRAY_SIZE(
                                LteMacDLTransportBlock_SubpktV4_SampleFmt, Fmt),
                            b, offset, length, result_subpkt_sample);
                        (void)_map_result_field_to_name(
                            result_subpkt_sample, "RNTI Type",
                            ValueNameRNTIType,
                            ARRAY_SIZE(ValueNameRNTIType, ValueName),
                            "(MI)Unknown");
                        int temp =
                            _search_result_int(result_subpkt_sample, "Sub-FN");
                        int iSubFN = temp & 15; // 4 bits
                        int iSFN = (temp >> 4);
                        _replace_result_int(result_subpkt_sample, "Sub-FN",
                                            iSubFN);
                        _replace_result_int(result_subpkt_sample, "SFN", iSFN);
                        offset +=
                            _search_result_int(result_subpkt_sample, "HDR LEN");
                        result_sample_list.push_back(result_subpkt_sample);
                    }
                    success = true;
                    break;
                }

                default:
                    break;
                }
                if (success) {
                    result_subpkt["Samples"] = result_sample_list;
                } else {
                    printf("(MI)Unknown LTE MAC Downlink Transport Block "
                           "Subpacket version: 0x%x - %d\n",
                           subpkt_id, subpkt_ver);
                }
                result_allpkts.push_back(result_subpkt);
            }
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE MAC Downlink Transport Block packet version: "
               "0x%x\n",
               pkt_ver);
        return 0;
    }
}

//------------------------------------------------------
// Jie
static int _decode_lte_mac_ul_bufferstatusinternal_subpkt(const char *b,
                                                          int offset,
                                                          size_t length,
                                                          json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num SubPkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            // Decode subpacket header
            offset += _decode_by_fmt(
                LteMacULBufferStatusInternal_SubpktHeaderFmt,
                ARRAY_SIZE(LteMacDLTransportBlock_SubpktHeaderFmt, Fmt), b,
                offset, length, result_subpkt);
            // Decode payload
            int subpkt_id = _search_result_int(result_subpkt, "SubPacket ID");
            int subpkt_ver = _search_result_int(result_subpkt, "Version");
            int subpkt_nsample =
                _search_result_int(result_subpkt, "Num Samples");
            const char *type_name = search_name(
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                subpkt_id);
            (void)_map_result_field_to_name(
                result_subpkt, "SubPacket ID",
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                "(MI)Unknown Subpkt Type");

            if (type_name == NULL) { // not found
                printf("(MI)Unknown LTE MAC Uplink Buffer Status Internel "
                       "Subpacket ID: 0x%x\n",
                       subpkt_id);
            } else {
                bool success = false;
                switch (subpkt_ver) {
                case 3: {
                    // UL Buffer Status SubPacket v3
                    json result_subpkt_allsamples;

                    for (int j = 0; j < subpkt_nsample; j++) {
                        json result_subpkt_sample;
                        offset += _decode_by_fmt(
                            LteMacULBufferStatusInternal_ULBufferStatusSubPacket_SampleFmt,
                            ARRAY_SIZE(
                                LteMacULBufferStatusInternal_ULBufferStatusSubPacket_SampleFmt,
                                Fmt),
                            b, offset, length, result_subpkt_sample);
                        int num_active_lcid = _search_result_int(
                            result_subpkt_sample, "Number of active LCID");
                        int temp =
                            _search_result_int(result_subpkt_sample, "Sub FN");
                        int iSubFN = temp & 15;
                        int iSysFN = (temp >> 4) & 1023;
                        _replace_result_int(result_subpkt_sample, "Sub FN",
                                            iSubFN);
                        _replace_result_int(result_subpkt_sample, "Sys FN",
                                            iSysFN);

                        json result_subpkt_sample_alllcids;

                        for (int k = 0; k < num_active_lcid; k++) {
                            json result_subpkt_sample_lcid;
                            offset += _decode_by_fmt(
                                LteMacULBufferStatusInternal_ULBufferStatusSubPacket_LCIDFmt,
                                ARRAY_SIZE(
                                    LteMacULBufferStatusInternal_ULBufferStatusSubPacket_LCIDFmt,
                                    Fmt),
                                b, offset, length, result_subpkt_sample_lcid);
                            result_subpkt_sample_alllcids.push_back(
                                result_subpkt_sample_lcid);
                        }
                        result_subpkt_sample["LCIDs"] =
                            result_subpkt_sample_alllcids;

                        result_subpkt_allsamples.push_back(
                            result_subpkt_sample);
                    }
                    result_subpkt["Samples"] = result_subpkt_allsamples;

                    success = true;
                    break;
                }
                case 24: {
                    json result_subpkt_allsamples;
                    for (int j = 0; j < subpkt_nsample; j++) {
                        json result_subpkt_sample;
                        offset += _decode_by_fmt(
                            LteMacULBufferStatusInternal_ULBufferStatusSubPacket_SampleFmt_v24,
                            ARRAY_SIZE(
                                LteMacULBufferStatusInternal_ULBufferStatusSubPacket_SampleFmt_v24,
                                Fmt),
                            b, offset, length, result_subpkt_sample);
                        int num_active_lcid = _search_result_int(
                            result_subpkt_sample, "Number of active LCID");
                        int temp =
                            _search_result_int(result_subpkt_sample, "Sub FN");
                        int iSubFN = temp & 15;
                        int iSysFN = (temp >> 4) & 1023;
                        _replace_result_int(result_subpkt_sample, "Sub FN",
                                            iSubFN);
                        _replace_result_int(result_subpkt_sample, "Sys FN",
                                            iSysFN);

                        json result_subpkt_sample_alllcids;
                        for (int k = 0; k < num_active_lcid; k++) {
                            json result_subpkt_sample_lcid;
                            offset += _decode_by_fmt(
                                LteMacULBufferStatusInternal_ULBufferStatusSubPacket_LCIDFmt_v24,
                                ARRAY_SIZE(
                                    LteMacULBufferStatusInternal_ULBufferStatusSubPacket_LCIDFmt_v24,
                                    Fmt),
                                b, offset, length, result_subpkt_sample_lcid);
                            unsigned int iNewUncompressedBytes =
                                _search_result_uint(result_subpkt_sample_lcid,
                                                    "New Uncompressed Bytes");
                            unsigned int iNewCompressedBytes =
                                _search_result_uint(result_subpkt_sample_lcid,
                                                    "New Compressed Bytes");
                            unsigned int iRetxBytes = _search_result_uint(
                                result_subpkt_sample_lcid, "Retx bytes");
                            int iCtrlBytes = _search_result_int(
                                result_subpkt_sample_lcid, "Ctrl bytes");
                            int iTotalBytes = (int)iNewUncompressedBytes +
                                              (int)iNewCompressedBytes +
                                              (int)iRetxBytes + iCtrlBytes;
                            _replace_result_int(result_subpkt_sample_lcid,
                                                "Total Bytes", iTotalBytes);
                            result_subpkt_sample_alllcids.push_back(
                                result_subpkt_sample_lcid);
                        }
                        result_subpkt_sample["LCIDs"] =
                            result_subpkt_sample_alllcids;

                        result_subpkt_allsamples.push_back(
                            result_subpkt_sample);
                    }
                    result_subpkt["Samples"] = result_subpkt_allsamples;

                    success = true;
                    break;
                }
                default:
                    break;
                }
                if (success) {
                    result_allpkts["MAC Subpacket"] = result_subpkt;
                } else {
                    printf("(MI)Unknown LTE MAC Uplink Buffer Status Internel "
                           "Subpacket version: 0x%x - %d\n",
                           subpkt_id, subpkt_ver);
                }
            }
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE MAC Uplink Buffer Status Internal packet "
               "version: 0x%x\n",
               pkt_ver);
        return 0;
    }

    return 0;
}

//------------------------------------------------------
// Jie
static int _decode_lte_mac_ul_txstatistics_subpkt(const char *b, int offset,
                                                  size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num SubPkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            // Decode subpacket header
            offset += _decode_by_fmt(
                LteMacULTxStatistics_SubpktHeaderFmt,
                ARRAY_SIZE(LteMacULTxStatistics_SubpktHeaderFmt, Fmt), b,
                offset, length, result_subpkt);
            // Decode payload
            int subpkt_id = _search_result_int(result_subpkt, "SubPacket ID");
            int subpkt_ver = _search_result_int(result_subpkt, "Version");
            const char *type_name = search_name(
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                subpkt_id);
            (void)_map_result_field_to_name(
                result_subpkt, "SubPacket ID",
                LteMacConfigurationSubpkt_SubpktType,
                ARRAY_SIZE(LteMacConfigurationSubpkt_SubpktType, ValueName),
                "(MI)Unknown Subpkt Type");

            if (type_name == NULL) { // not found
                printf("(MI)Unknown LTE MAC Uplink Tx Statistics Subpacket ID: "
                       "0x%x\n",
                       subpkt_id);
            } else {
                bool success = false;
                switch (subpkt_ver) {
                case 1: // UL Tx Stats SubPacket version 1
                {
                    json result_subpkt_sample;
                    offset += _decode_by_fmt(
                        LteMacULTxStatistics_ULTxStatsSubPacketFmt,
                        ARRAY_SIZE(LteMacULTxStatistics_ULTxStatsSubPacketFmt,
                                   Fmt),
                        b, offset, length, result_subpkt_sample);

                    result_subpkt["Sample"] = result_subpkt_sample;
                    success = true;
                    break;
                }
                case 2: // UL Tx Stats SubPacket version 2
                {
                    json result_subpkt_sample;
                    offset += _decode_by_fmt(
                        LteMacULTxStatistics_ULTxStatsSubPacketFmtV2,
                        ARRAY_SIZE(LteMacULTxStatistics_ULTxStatsSubPacketFmtV2,
                                   Fmt),
                        b, offset, length, result_subpkt_sample);

                    result_subpkt["Sample"] = result_subpkt_sample;
                    success = true;
                    break;
                }

                default:
                    break;
                }
                if (success) {
                    result_allpkts["MAC Subpacket"] = result_subpkt;
                } else {
                    printf("(MI)Unknown LTE MAC Uplink Tx Statistics Subpacket "
                           "version: 0x%x - %d\n",
                           subpkt_id, subpkt_ver);
                }
            }
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf(
            "(MI)Unknown LTE MAC Uplink Tx Statistics packet version: 0x%x\n",
            pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_rlc_ul_config_log_packet_subpkt(const char *b,
                                                       int offset,
                                                       size_t length,
                                                       json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num SubPkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // Decode subpacket header
            offset += _decode_by_fmt(
                LteRlcUlConfigLogPacket_SubpktHeader,
                ARRAY_SIZE(LteRlcUlConfigLogPacket_SubpktHeader, Fmt), b,
                offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 69 && subpkt_ver == 1) {
                // 69 means lte rlc ul config log packet
                offset += _decode_by_fmt(
                    LteRlcUlConfigLogPacket_SubpktPayload,
                    ARRAY_SIZE(LteRlcUlConfigLogPacket_SubpktPayload, Fmt), b,
                    offset, length, result_subpkt);
                (void)_map_result_field_to_name(
                    result_subpkt, "Reason",
                    LteRlcUlConfigLogPacket_Subpkt_Reason,
                    ARRAY_SIZE(LteRlcUlConfigLogPacket_Subpkt_Reason,
                               ValueName),
                    "(MI)Unknown");
                int maxSizeRbs =
                    _search_result_int(result_subpkt, "Max Size RBs");

                // Released RB Struct
                int start_ReleasedRBStruct = offset;
                offset += _decode_by_fmt(
                    LteRlcUlConfigLogPacket_Subpkt_ReleasedRB_Header,
                    ARRAY_SIZE(LteRlcUlConfigLogPacket_Subpkt_ReleasedRB_Header,
                               Fmt),
                    b, offset, length, result_subpkt);
                int num_ReleasedRB =
                    _search_result_int(result_subpkt, "Number of Released RBs");
                json result_ReleasedRB;
                for (int j = 0; j < num_ReleasedRB; j++) {
                    json result_ReleasedRB_item;
                    offset += _decode_by_fmt(
                        LteRlcUlConfigLogPacket_Subpkt_ReleasedRB_Fmt,
                        ARRAY_SIZE(
                            LteRlcUlConfigLogPacket_Subpkt_ReleasedRB_Fmt, Fmt),
                        b, offset, length, result_ReleasedRB_item);
                    result_ReleasedRB.push_back(result_ReleasedRB_item);
                }
                result_subpkt["Released RBs"] = result_ReleasedRB;
                offset +=
                    1 + maxSizeRbs * 1 - (offset - start_ReleasedRBStruct);

                // Added Modified RB Struct
                int start_AddedModifiedRBStruct = offset;
                offset += _decode_by_fmt(
                    LteRlcUlConfigLogPacket_Subpkt_AddedModifiedRB_Header,
                    ARRAY_SIZE(
                        LteRlcUlConfigLogPacket_Subpkt_AddedModifiedRB_Header,
                        Fmt),
                    b, offset, length, result_subpkt);
                int num_AddedModifiedRB = _search_result_int(
                    result_subpkt, "Number of Added/Modified RBs");
                json result_AddedModifiedRB;
                for (int j = 0; j < num_AddedModifiedRB; j++) {
                    json result_AddedModifiedRB_item;
                    offset += _decode_by_fmt(
                        LteRlcUlConfigLogPacket_Subpkt_AddedModifiedRB_Fmt,
                        ARRAY_SIZE(
                            LteRlcUlConfigLogPacket_Subpkt_AddedModifiedRB_Fmt,
                            Fmt),
                        b, offset, length, result_AddedModifiedRB_item);
                    (void)_map_result_field_to_name(
                        result_AddedModifiedRB_item, "Action",
                        LteRlcUlConfigLogPacket_Subpkt_AddedModifiedRB_Action,
                        ARRAY_SIZE(
                            LteRlcUlConfigLogPacket_Subpkt_AddedModifiedRB_Action,
                            ValueName),
                        "(MI)Unknown");
                    result_AddedModifiedRB.push_back(
                        result_AddedModifiedRB_item);
                }
                result_subpkt["Added/Modified RBs"] = result_AddedModifiedRB;
                offset +=
                    1 + maxSizeRbs * 2 - (offset - start_AddedModifiedRBStruct);

                // Active RB info
                offset += _decode_by_fmt(
                    LteRlcUlConfigLogPacket_Subpkt_ActiveRB_Header,
                    ARRAY_SIZE(LteRlcUlConfigLogPacket_Subpkt_ActiveRB_Header,
                               Fmt),
                    b, offset, length, result_subpkt);
                int num_ActiveRB =
                    _search_result_int(result_subpkt, "Number of Active RBs");
                json result_ActiveRB;
                for (int j = 0; j < num_ActiveRB; j++) {
                    json result_ActiveRB_item;
                    offset += _decode_by_fmt(
                        LteRlcUlConfigLogPacket_Subpkt_ActiveRB_Fmt,
                        ARRAY_SIZE(LteRlcUlConfigLogPacket_Subpkt_ActiveRB_Fmt,
                                   Fmt),
                        b, offset, length, result_ActiveRB_item);
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RB Mode",
                        LteRlcUlConfigLogPacket_Subpkt_ActiveRB_RBMode,
                        ARRAY_SIZE(
                            LteRlcUlConfigLogPacket_Subpkt_ActiveRB_RBMode,
                            ValueName),
                        "(MI)Unknown");
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RB Type",
                        LteRlcUlConfigLogPacket_Subpkt_ActiveRB_RBType,
                        ARRAY_SIZE(
                            LteRlcUlConfigLogPacket_Subpkt_ActiveRB_RBType,
                            ValueName),
                        "(MI)Unknown");
                    std::string pystr = "RLCUL CFG";
                    _replace_result(result_ActiveRB_item, "RLCUL CFG", pystr);

                    _replace_result_int(result_ActiveRB_item, "SN Length", 10);

                    result_ActiveRB.push_back(result_ActiveRB_item);
                }
                result_subpkt["Active RBs"] = result_ActiveRB;
            } else {
                printf("(MI)Unknown LTE RLC UL Config Log Packet subpkt id"
                       "and version: 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE RLC UL Config Log Packet version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_rlc_dl_config_log_packet_subpkt(const char *b,
                                                       int offset,
                                                       size_t length,
                                                       json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num SubPkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // Decode subpacket header
            offset += _decode_by_fmt(
                LteRlcDlConfigLogPacket_SubpktHeader,
                ARRAY_SIZE(LteRlcDlConfigLogPacket_SubpktHeader, Fmt), b,
                offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 64 && subpkt_ver == 1) {
                // 64 means lte rlc dl config log packet
                offset += _decode_by_fmt(
                    LteRlcDlConfigLogPacket_SubpktPayload,
                    ARRAY_SIZE(LteRlcDlConfigLogPacket_SubpktPayload, Fmt), b,
                    offset, length, result_subpkt);
                (void)_map_result_field_to_name(
                    result_subpkt, "Reason",
                    LteRlcDlConfigLogPacket_Subpkt_Reason,
                    ARRAY_SIZE(LteRlcDlConfigLogPacket_Subpkt_Reason,
                               ValueName),
                    "(MI)Unknown");
                int maxSizeRbs =
                    _search_result_int(result_subpkt, "Max Size RBs");

                // Released RB Struct
                int start_ReleasedRBStruct = offset;
                offset += _decode_by_fmt(
                    LteRlcDlConfigLogPacket_Subpkt_ReleasedRB_Header,
                    ARRAY_SIZE(LteRlcDlConfigLogPacket_Subpkt_ReleasedRB_Header,
                               Fmt),
                    b, offset, length, result_subpkt);
                int num_ReleasedRB =
                    _search_result_int(result_subpkt, "Number of Released RBs");
                json result_ReleasedRB;
                for (int j = 0; j < num_ReleasedRB; j++) {
                    json result_ReleasedRB_item;
                    offset += _decode_by_fmt(
                        LteRlcDlConfigLogPacket_Subpkt_ReleasedRB_Fmt,
                        ARRAY_SIZE(
                            LteRlcDlConfigLogPacket_Subpkt_ReleasedRB_Fmt, Fmt),
                        b, offset, length, result_ReleasedRB_item);
                    result_ReleasedRB.push_back(result_ReleasedRB_item);
                }
                result_subpkt["Released RBs"] = result_ReleasedRB;
                offset +=
                    1 + maxSizeRbs * 1 - (offset - start_ReleasedRBStruct);

                // Added Modified RB Struct
                int start_AddedModifiedRBStruct = offset;
                offset += _decode_by_fmt(
                    LteRlcDlConfigLogPacket_Subpkt_AddedModifiedRB_Header,
                    ARRAY_SIZE(
                        LteRlcDlConfigLogPacket_Subpkt_AddedModifiedRB_Header,
                        Fmt),
                    b, offset, length, result_subpkt);
                int num_AddedModifiedRB = _search_result_int(
                    result_subpkt, "Number of Added/Modified RBs");
                json result_AddedModifiedRB;
                for (int j = 0; j < num_AddedModifiedRB; j++) {
                    json result_AddedModifiedRB_item;
                    offset += _decode_by_fmt(
                        LteRlcDlConfigLogPacket_Subpkt_AddedModifiedRB_Fmt,
                        ARRAY_SIZE(
                            LteRlcDlConfigLogPacket_Subpkt_AddedModifiedRB_Fmt,
                            Fmt),
                        b, offset, length, result_AddedModifiedRB_item);
                    (void)_map_result_field_to_name(
                        result_AddedModifiedRB_item, "Action",
                        LteRlcDlConfigLogPacket_Subpkt_AddedModifiedRB_Action,
                        ARRAY_SIZE(
                            LteRlcDlConfigLogPacket_Subpkt_AddedModifiedRB_Action,
                            ValueName),
                        "(MI)Unknown");
                    result_AddedModifiedRB.push_back(
                        result_AddedModifiedRB_item);
                }
                result_subpkt["Added/Modified RBs"] = result_AddedModifiedRB;
                offset +=
                    1 + maxSizeRbs * 2 - (offset - start_AddedModifiedRBStruct);

                // Active RB info
                offset += _decode_by_fmt(
                    LteRlcDlConfigLogPacket_Subpkt_ActiveRB_Header,
                    ARRAY_SIZE(LteRlcDlConfigLogPacket_Subpkt_ActiveRB_Header,
                               Fmt),
                    b, offset, length, result_subpkt);
                int num_ActiveRB =
                    _search_result_int(result_subpkt, "Number of Active RBs");
                json result_ActiveRB;
                for (int j = 0; j < num_ActiveRB; j++) {
                    json result_ActiveRB_item;
                    offset += _decode_by_fmt(
                        LteRlcDlConfigLogPacket_Subpkt_ActiveRB_Fmt,
                        ARRAY_SIZE(LteRlcDlConfigLogPacket_Subpkt_ActiveRB_Fmt,
                                   Fmt),
                        b, offset, length, result_ActiveRB_item);
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RB Mode",
                        LteRlcDlConfigLogPacket_Subpkt_ActiveRB_RBMode,
                        ARRAY_SIZE(
                            LteRlcDlConfigLogPacket_Subpkt_ActiveRB_RBMode,
                            ValueName),
                        "(MI)Unknown");
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RB Type",
                        LteRlcDlConfigLogPacket_Subpkt_ActiveRB_RBType,
                        ARRAY_SIZE(
                            LteRlcDlConfigLogPacket_Subpkt_ActiveRB_RBType,
                            ValueName),
                        "(MI)Unknown");
                    std::string pystr = "RLCDL CFG";
                    _replace_result(result_ActiveRB_item, "RLCDL CFG", pystr);

                    _replace_result_int(result_ActiveRB_item, "SN Length", 10);

                    result_ActiveRB.push_back(result_ActiveRB_item);
                }
                result_subpkt["Active RBs"] = result_ActiveRB;
            } else {
                printf("(MI)Unknown LTE RLC DL Config Log Packet subpkt id"
                       "and version: 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE RLC DL Config Log Packet version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_rlc_ul_am_all_pdu_subpkt(const char *b, int offset,
                                                size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Number of Subpackets");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            offset +=
                _decode_by_fmt(LteRlcUlAmAllPdu_SubpktHeader,
                               ARRAY_SIZE(LteRlcUlAmAllPdu_SubpktHeader, Fmt),
                               b, offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 70 && subpkt_ver == 3) {
                // 70 means LTE RLC UL AM All PDU
                offset += _decode_by_fmt(
                    LteRlcUlAmAllPdu_SubpktPayload,
                    ARRAY_SIZE(LteRlcUlAmAllPdu_SubpktPayload, Fmt), b, offset,
                    length, result_subpkt);
                int rb_cfg_idx =
                    _search_result_int(result_subpkt, "RB Cfg Idx");
                (void)_map_result_field_to_name(
                    result_subpkt, "RB Mode", LteRlcUlAmAllPdu_Subpkt_RBMode,
                    ARRAY_SIZE(LteRlcUlAmAllPdu_Subpkt_RBMode, ValueName),
                    "(MI)Unknown");
                int enabled_pdu = _search_result_int(result_subpkt,
                                                     "Enabled PDU Log Packets");
                // Need to check bit by bit
                std::string strEnabledPDU = "";
                if ((enabled_pdu) & (1 << (1)))
                    strEnabledPDU += "RLCUL Config (0xB091), ";
                if ((enabled_pdu) & (1 << (2)))
                    strEnabledPDU += "RLCUL AM ALL PDU (0xB092), ";
                if ((enabled_pdu) & (1 << (3)))
                    strEnabledPDU += "RLCUL AM CONTROL PDU (0xB093), ";
                if ((enabled_pdu) & (1 << (4)))
                    strEnabledPDU += "RLCUL AM POLLING PDU (0xB094), ";
                if ((enabled_pdu) & (1 << (5)))
                    strEnabledPDU += "RLCUL AM SIGNALING PDU (0xB095), ";
                if ((enabled_pdu) & (1 << (6)))
                    strEnabledPDU += "RLCUL UM DATA PDU (0xB096), ";
                if ((enabled_pdu) & (1 << (7)))
                    strEnabledPDU += "RLCUL STATISTICS (0xB097), ";
                if ((enabled_pdu) & (1 << (8)))
                    strEnabledPDU += "RLCUL AM STATE (0xB098), ";
                if ((enabled_pdu) & (1 << (9)))
                    strEnabledPDU += "RLCUL UM STATE (0xB099), ";
                if (strEnabledPDU.length() > 0) {
                    strEnabledPDU =
                        strEnabledPDU.substr(0, strEnabledPDU.length() - 2);
                    strEnabledPDU += ".";
                }
                std::string pystr = strEnabledPDU.c_str();
                _replace_result(result_subpkt, "Enabled PDU Log Packets",
                                pystr);

                int n_pdu = _search_result_int(result_subpkt, "Number of PDUs");
                json result_pdu;
                for (int j = 0; j < n_pdu; j++) {
                    json result_pdu_item;
                    offset += _decode_by_fmt(
                        LteRlcUlAmAllPdu_Subpkt_PDU_Basic,
                        ARRAY_SIZE(LteRlcUlAmAllPdu_Subpkt_PDU_Basic, Fmt), b,
                        offset, length, result_pdu_item);
                    // Update rb_cfg_idx from previous content
                    _replace_result_int(result_pdu_item, "rb_cfg_idx",
                                        rb_cfg_idx);
                    int DCLookAhead =
                        _search_result_int(result_pdu_item, "D/C LookAhead");
                    int iNonDecodeFN =
                        _search_result_int(result_pdu_item, "sys_fn");
                    int iLoggedBytes =
                        _search_result_int(result_pdu_item, "logged_bytes");
                    // D/C LookAhead and SN (or Ack_SN) has already been parsed.
                    iLoggedBytes -= 2;

                    // Handle fn
                    const unsigned int SFN_RSHIFT = 4, SFN_MASK = (1 << 12) - 1;
                    const unsigned int SUBFRAME_RSHIFT = 0,
                                       SUBFRAME_MASK = (1 << 4) - 1;
                    int sys_fn = (iNonDecodeFN >> SFN_RSHIFT) & SFN_MASK;
                    int sub_fn =
                        (iNonDecodeFN >> SUBFRAME_RSHIFT) & SUBFRAME_MASK;
                    _replace_result_int(result_pdu_item, "sys_fn", sys_fn);
                    _replace_result_int(result_pdu_item, "sub_fn", sub_fn);
                    // Decide PDU type
                    int iNonDecodeSN =
                        _search_result_int(result_pdu_item, "SN");
                    if (DCLookAhead < 16) {
                        // Type = Control
                        pystr = "RLCUL CTRL";
                        _replace_result(result_pdu_item, "PDU TYPE", pystr);
                        // Modify ACK_SN
                        std::string strAckSN = "ACK_SN = ";
                        int iAckSN = DCLookAhead * 64 + iNonDecodeSN / 4;
                        strAckSN += SSTR(iAckSN);
                        int iHeadFromPadding = (iNonDecodeSN & 1) * 512;
                        pystr = strAckSN.c_str();
                        _replace_result(result_pdu_item, "SN", pystr);
                        // Update other info
                        offset += _decode_by_fmt(
                            LteRlcUlAmAllPdu_Subpkt_PDU_Control,
                            ARRAY_SIZE(LteRlcUlAmAllPdu_Subpkt_PDU_Control,
                                       Fmt),
                            b, offset, length, result_pdu_item);
                        pystr = "STATUS(0)";
                        _replace_result(result_pdu_item, "cpt", pystr);

                        if (iLoggedBytes > 0) {
                            // Decode NACK
                            int numNack = iLoggedBytes / 1.5;
                            int iAllign = 0;
                            int iHeadFromAllign = 0;

                            json result_pdu_nack;
                            for (int indexNack = 0; indexNack < numNack;
                                 indexNack++) {
                                json result_pdu_nack_item;
                                if (iAllign == 0) {
                                    iAllign = 1;
                                    offset += _decode_by_fmt(
                                        LteRlcUlAmAllPdu_Subpkt_PDU_NACK_ALLIGN,
                                        ARRAY_SIZE(
                                            LteRlcUlAmAllPdu_Subpkt_PDU_NACK_ALLIGN,
                                            Fmt),
                                        b, offset, length,
                                        result_pdu_nack_item);
                                    iLoggedBytes -= 2;
                                    int iNonDecodeNACK = _search_result_int(
                                        result_pdu_nack_item, "NACK_SN");
                                    int iPart3 = iNonDecodeNACK / 4096;
                                    int iPart4 =
                                        (iNonDecodeNACK - iPart3 * 4096) / 256;
                                    int iPart1 =
                                        (iNonDecodeNACK - iPart3 * 4096 -
                                         iPart4 * 256) /
                                        16;
                                    int iPart2 = iNonDecodeNACK -
                                                 iPart3 * 4096 - iPart4 * 256 -
                                                 iPart1 * 16;
                                    int iNack = iPart1 * 32 + iPart2 * 2 +
                                                iPart3 / 8 + iHeadFromPadding;
                                    iHeadFromAllign =
                                        iPart4 + (iPart3 & 1) * 16;
                                    int iE2 = iPart3 & 2;
                                    if (iE2 == 2) {
                                        indexNack = numNack - 1;
                                    }
                                    _replace_result_int(result_pdu_nack_item,
                                                        "NACK_SN", iNack);
                                } else {
                                    iAllign = 0;
                                    offset += _decode_by_fmt(
                                        LteRlcUlAmAllPdu_Subpkt_PDU_NACK_PADDING,
                                        ARRAY_SIZE(
                                            LteRlcUlAmAllPdu_Subpkt_PDU_NACK_PADDING,
                                            Fmt),
                                        b, offset, length,
                                        result_pdu_nack_item);
                                    iLoggedBytes -= 1;
                                    int iNonDecodeNACK = _search_result_int(
                                        result_pdu_nack_item, "NACK_SN");
                                    int iNack = iHeadFromAllign * 32 +
                                                iNonDecodeNACK / 8;
                                    int iE2 = iNonDecodeNACK & 2;
                                    if (iE2 == 2) {
                                        indexNack = numNack - 1;
                                    }
                                    iHeadFromPadding =
                                        (iNonDecodeNACK & 1) * 512;
                                    _replace_result_int(result_pdu_nack_item,
                                                        "NACK_SN", iNack);
                                }
                                result_pdu_nack.push_back(result_pdu_nack_item);
                            }
                            result_pdu_item["RLC CTRL NACK"] = result_pdu_nack;
                        }
                    } else {
                        // Type = DATA
                        pystr = "RLCUL DATA";
                        _replace_result(result_pdu_item, "PDU TYPE", pystr);
                        // Update other info
                        std::string strRF = "", strP = "", strFI = "",
                                    strE = "";
                        if ((DCLookAhead) & (1 << (6))) {
                            strRF += "1";
                        } else {
                            strRF += "0";
                        }
                        if ((DCLookAhead) & (1 << (5))) {
                            strP += "1";
                        } else {
                            strP += "0";
                        }
                        if ((DCLookAhead) & (1 << (4))) {
                            strFI += "1";
                        } else {
                            strFI += "0";
                        }
                        if ((DCLookAhead) & (1 << (3))) {
                            strFI += "1";
                        } else {
                            strFI += "0";
                        }
                        if ((DCLookAhead) & (1 << (2))) {
                            strE += "1";
                        } else {
                            strE += "0";
                        }
                        // update SN (need to check last two bits in
                        // DCLookAhead)
                        if ((DCLookAhead) & (1 << (1))) {
                            iNonDecodeSN += 512;
                        }
                        if ((DCLookAhead) & (1)) {
                            iNonDecodeSN += 256;
                        }
                        _replace_result_int(result_pdu_item, "SN",
                                            iNonDecodeSN);

                        offset += _decode_by_fmt(
                            LteRlcUlAmAllPdu_Subpkt_PDU_DATA,
                            ARRAY_SIZE(LteRlcUlAmAllPdu_Subpkt_PDU_DATA, Fmt),
                            b, offset, length, result_pdu_item);
                        pystr = (strRF.c_str());
                        _replace_result(result_pdu_item, "RF", pystr);
                        pystr = (strP.c_str());
                        _replace_result(result_pdu_item, "P", pystr);
                        pystr = (strFI.c_str());
                        _replace_result(result_pdu_item, "FI", pystr);
                        pystr = (strE.c_str());
                        _replace_result(result_pdu_item, "E", pystr);

                        if (strRF == "1") {
                            // decode LSF and SO
                            iLoggedBytes -= 2;

                            offset += _decode_by_fmt(
                                LteRlcUlAmAllPdu_Subpkt_PDU_LSF_SO,
                                ARRAY_SIZE(LteRlcUlAmAllPdu_Subpkt_PDU_LSF_SO,
                                           Fmt),
                                b, offset, length, result_pdu_item);
                            int temp =
                                _search_result_int(result_pdu_item, "LSF");
                            int iLSF = temp >> 7;
                            int iSO = _search_result_int(result_pdu_item, "SO");
                            iSO += (temp & 127) * 256;
                            _replace_result_int(result_pdu_item, "LSF", iLSF);
                            _replace_result_int(result_pdu_item, "SO", iSO);
                        }

                        if (strE == "1") {
                            // Decode LI
                            int numLI = iLoggedBytes / 1.5;
                            iLoggedBytes = 0;
                            int iAllign = 0;
                            int iHeadFromAllign = 0;

                            json result_pdu_li;
                            for (int indexLI = 0; indexLI < numLI; indexLI++) {
                                json result_pdu_li_item;
                                if (iAllign == 0) {
                                    iAllign = 1;
                                    offset += _decode_by_fmt(
                                        LteRlcUlAmAllPdu_Subpkt_PDU_LI_ALLIGN,
                                        ARRAY_SIZE(
                                            LteRlcUlAmAllPdu_Subpkt_PDU_LI_ALLIGN,
                                            Fmt),
                                        b, offset, length, result_pdu_li_item);
                                    int iNonDecodeLI = _search_result_int(
                                        result_pdu_li_item, "LI");
                                    int iPart3 = iNonDecodeLI / 4096;
                                    int iPart4 =
                                        (iNonDecodeLI - iPart3 * 4096) / 256;
                                    int iPart1 = (iNonDecodeLI - iPart3 * 4096 -
                                                  iPart4 * 256) /
                                                 16;
                                    int iPart2 = iNonDecodeLI - iPart3 * 4096 -
                                                 iPart4 * 256 - iPart1 * 16;
                                    int iLI = (iPart1 % 8) * 256 + iPart2 * 16 +
                                              iPart3;
                                    iHeadFromAllign = (iPart4 % 8) * 256;
                                    _replace_result_int(result_pdu_li_item,
                                                        "LI", iLI);
                                } else {
                                    iAllign = 0;
                                    offset += _decode_by_fmt(
                                        LteRlcUlAmAllPdu_Subpkt_PDU_LI_PADDING,
                                        ARRAY_SIZE(
                                            LteRlcUlAmAllPdu_Subpkt_PDU_LI_PADDING,
                                            Fmt),
                                        b, offset, length, result_pdu_li_item);
                                    int iNonDecodeLI = _search_result_int(
                                        result_pdu_li_item, "LI");
                                    int iLI = iHeadFromAllign + iNonDecodeLI;
                                    _replace_result_int(result_pdu_li_item,
                                                        "LI", iLI);
                                }
                                result_pdu_li.push_back(result_pdu_li_item);
                            }
                            result_pdu_item["RLC DATA LI"] = result_pdu_li;
                        }
                    }
                    offset += iLoggedBytes;
                    //result_pdu[("RLCUL PDU[" + SSTR(i) + "]").c_str()] =
                    //    result_pdu_item;
					result_pdu.push_back(result_pdu_item);
                }
                result_subpkt["RLCUL PDUs"] = result_pdu;
            } else {
                printf("Unkown LTE RLC UL AM ALL PDU subpkt id and version"
                       ": 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("Unkown LTE RLC UL AM ALL PDU version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_rlc_dl_am_all_pdu_subpkt(const char *b, int offset,
                                                size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Number of Subpackets");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            offset +=
                _decode_by_fmt(LteRlcDlAmAllPdu_SubpktHeader,
                               ARRAY_SIZE(LteRlcDlAmAllPdu_SubpktHeader, Fmt),
                               b, offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 65 && subpkt_ver == 3) {
                // 65 means LTE RLC DL AM All PDU
                offset += _decode_by_fmt(
                    LteRlcDlAmAllPdu_SubpktPayload,
                    ARRAY_SIZE(LteRlcDlAmAllPdu_SubpktPayload, Fmt), b, offset,
                    length, result_subpkt);
                int rb_cfg_idx =
                    _search_result_int(result_subpkt, "RB Cfg Idx");
                (void)_map_result_field_to_name(
                    result_subpkt, "RB Mode", LteRlcDlAmAllPdu_Subpkt_RBMode,
                    ARRAY_SIZE(LteRlcDlAmAllPdu_Subpkt_RBMode, ValueName),
                    "(MI)Unknown");
                int enabled_pdu = _search_result_int(result_subpkt,
                                                     "Enabled PDU Log Packets");
                // Need to check bit by bit
                std::string strEnabledPDU = "";
                if ((enabled_pdu) & (1 << (1)))
                    strEnabledPDU += "RLCDL Config (0xB081), ";
                if ((enabled_pdu) & (1 << (2)))
                    strEnabledPDU += "RLCDL AM ALL PDU (0xB082), ";
                if ((enabled_pdu) & (1 << (3)))
                    strEnabledPDU += "RLCDL AM CONTROL PDU (0xB083), ";
                if ((enabled_pdu) & (1 << (4)))
                    strEnabledPDU += "RLCDL AM POLLING PDU (0xB084), ";
                if ((enabled_pdu) & (1 << (5)))
                    strEnabledPDU += "RLCDL AM SIGNALING PDU (0xB085), ";
                if ((enabled_pdu) & (1 << (6)))
                    strEnabledPDU += "RLCDL UM DATA PDU (0xB086), ";
                if ((enabled_pdu) & (1 << (7)))
                    strEnabledPDU += "RLCDL STATISTICS (0xB087), ";
                if ((enabled_pdu) & (1 << (8)))
                    strEnabledPDU += "RLCDL AM STATE (0xB088), ";
                if ((enabled_pdu) & (1 << (9)))
                    strEnabledPDU += "RLCDL UM STATE (0xB089), ";
                if (strEnabledPDU.length() > 0) {
                    strEnabledPDU =
                        strEnabledPDU.substr(0, strEnabledPDU.length() - 2);
                    strEnabledPDU += ".";
                }
                std::string pystr = strEnabledPDU.c_str();
                _replace_result(result_subpkt, "Enabled PDU Log Packets",
                                pystr);

                int n_pdu = _search_result_int(result_subpkt, "Number of PDUs");
                json result_pdu;
                for (int j = 0; j < n_pdu; j++) {
                    json result_pdu_item;
                    offset += _decode_by_fmt(
                        LteRlcDlAmAllPdu_Subpkt_PDU_Basic,
                        ARRAY_SIZE(LteRlcDlAmAllPdu_Subpkt_PDU_Basic, Fmt), b,
                        offset, length, result_pdu_item);
                    // Update rb_cfg_idx from previous content
                    _replace_result_int(result_pdu_item, "rb_cfg_idx",
                                        rb_cfg_idx);
                    int DCLookAhead =
                        _search_result_int(result_pdu_item, "D/C LookAhead");
                    int iNonDecodeFN =
                        _search_result_int(result_pdu_item, "sys_fn");
                    int iLoggedBytes =
                        _search_result_int(result_pdu_item, "logged_bytes");
                    // D/C LookAhead and SN (or Ack_SN) has already been parsed.
                    iLoggedBytes -= 2;

                    // Handle fn
                    const unsigned int SFN_RSHIFT = 4, SFN_MASK = (1 << 12) - 1;
                    const unsigned int SUBFRAME_RSHIFT = 0,
                                       SUBFRAME_MASK = (1 << 4) - 1;
                    int sys_fn = (iNonDecodeFN >> SFN_RSHIFT) & SFN_MASK;
                    int sub_fn =
                        (iNonDecodeFN >> SUBFRAME_RSHIFT) & SUBFRAME_MASK;
                    _replace_result_int(result_pdu_item, "sys_fn", sys_fn);
                    _replace_result_int(result_pdu_item, "sub_fn", sub_fn);
                    // Decide PDU type
                    int iNonDecodeSN =
                        _search_result_int(result_pdu_item, "SN");
                    if (DCLookAhead < 16) {
                        // Type = Control
                        pystr = "RLCDL CTRL";
                        _replace_result(result_pdu_item, "PDU TYPE", pystr);
                        pystr = "PDU CTRL";
                        _replace_result(result_pdu_item, "Status", pystr);
                        // Modify ACK_SN
                        std::string strAckSN = "ACK_SN = ";
                        int iAckSN = DCLookAhead * 64 + iNonDecodeSN / 4;
                        strAckSN += SSTR(iAckSN);
                        int iHeadFromPadding = (iNonDecodeSN & 1) * 512;
                        pystr = strAckSN.c_str();
                        _replace_result(result_pdu_item, "SN", pystr);
                        // Update other info
                        offset += _decode_by_fmt(
                            LteRlcDlAmAllPdu_Subpkt_PDU_Control,
                            ARRAY_SIZE(LteRlcDlAmAllPdu_Subpkt_PDU_Control,
                                       Fmt),
                            b, offset, length, result_pdu_item);
                        pystr = "STATUS(0)";
                        _replace_result(result_pdu_item, "cpt", pystr);

                        if (iLoggedBytes > 0) {
                            // Decode NACK
                            int numNack = iLoggedBytes / 1.5;
                            int iAllign = 0;
                            int iHeadFromAllign = 0;

                            json result_pdu_nack;
                            for (int indexNack = 0; indexNack < numNack;
                                 indexNack++) {
                                json result_pdu_nack_item;
                                if (iAllign == 0) {
                                    iAllign = 1;
                                    offset += _decode_by_fmt(
                                        LteRlcDlAmAllPdu_Subpkt_PDU_NACK_ALLIGN,
                                        ARRAY_SIZE(
                                            LteRlcDlAmAllPdu_Subpkt_PDU_NACK_ALLIGN,
                                            Fmt),
                                        b, offset, length,
                                        result_pdu_nack_item);
                                    iLoggedBytes -= 2;
                                    int iNonDecodeNACK = _search_result_int(
                                        result_pdu_nack_item, "NACK_SN");
                                    int iPart3 = iNonDecodeNACK / 4096;
                                    int iPart4 =
                                        (iNonDecodeNACK - iPart3 * 4096) / 256;
                                    int iPart1 =
                                        (iNonDecodeNACK - iPart3 * 4096 -
                                         iPart4 * 256) /
                                        16;
                                    int iPart2 = iNonDecodeNACK -
                                                 iPart3 * 4096 - iPart4 * 256 -
                                                 iPart1 * 16;
                                    int iNack = iPart1 * 32 + iPart2 * 2 +
                                                iPart3 / 8 + iHeadFromPadding;
                                    iHeadFromAllign =
                                        iPart4 + (iPart3 & 1) * 16;
                                    int iE2 = iPart3 & 2;
                                    if (iE2 == 2) {
                                        indexNack = numNack - 1;
                                    }
                                    _replace_result_int(result_pdu_nack_item,
                                                        "NACK_SN", iNack);
                                } else {
                                    iAllign = 0;
                                    offset += _decode_by_fmt(
                                        LteRlcDlAmAllPdu_Subpkt_PDU_NACK_PADDING,
                                        ARRAY_SIZE(
                                            LteRlcDlAmAllPdu_Subpkt_PDU_NACK_PADDING,
                                            Fmt),
                                        b, offset, length,
                                        result_pdu_nack_item);
                                    iLoggedBytes -= 1;
                                    int iNonDecodeNACK = _search_result_int(
                                        result_pdu_nack_item, "NACK_SN");
                                    int iNack = iHeadFromAllign * 32 +
                                                iNonDecodeNACK / 8;
                                    int iE2 = iNonDecodeNACK & 2;
                                    if (iE2 == 2) {
                                        indexNack = numNack - 1;
                                    }
                                    iHeadFromPadding =
                                        (iNonDecodeNACK & 1) * 512;
                                    _replace_result_int(result_pdu_nack_item,
                                                        "NACK_SN", iNack);
                                }
                                result_pdu_nack.push_back(result_pdu_nack_item);
                            }
                            result_pdu_item["RLC CTRL NACK"] = result_pdu_nack;
                        }
                    } else {
                        // Type = DATA
                        pystr = "RLCDL DATA";
                        _replace_result(result_pdu_item, "PDU TYPE", pystr);
                        pystr = "PDU DATA";
                        _replace_result(result_pdu_item, "Status", pystr);
                        // Update other info
                        std::string strRF = "", strP = "", strFI = "",
                                    strE = "";
                        if ((DCLookAhead) & (1 << (6))) {
                            strRF += "1";
                        } else {
                            strRF += "0";
                        }
                        if ((DCLookAhead) & (1 << (5))) {
                            strP += "1";
                        } else {
                            strP += "0";
                        }
                        if ((DCLookAhead) & (1 << (4))) {
                            strFI += "1";
                        } else {
                            strFI += "0";
                        }
                        if ((DCLookAhead) & (1 << (3))) {
                            strFI += "1";
                        } else {
                            strFI += "0";
                        }
                        if ((DCLookAhead) & (1 << (2))) {
                            strE += "1";
                        } else {
                            strE += "0";
                        }
                        // update SN (need to check last two bits in
                        // DCLookAhead)
                        if ((DCLookAhead) & (1 << (1))) {
                            iNonDecodeSN += 512;
                        }
                        if ((DCLookAhead) & (1)) {
                            iNonDecodeSN += 256;
                        }
                        _replace_result_int(result_pdu_item, "SN",
                                            iNonDecodeSN);
                        offset += _decode_by_fmt(
                            LteRlcDlAmAllPdu_Subpkt_PDU_DATA,
                            ARRAY_SIZE(LteRlcDlAmAllPdu_Subpkt_PDU_DATA, Fmt),
                            b, offset, length, result_pdu_item);
                        pystr = (strRF.c_str());
                        _replace_result(result_pdu_item, "RF", pystr);
                        pystr = (strP.c_str());
                        _replace_result(result_pdu_item, "P", pystr);
                        pystr = (strFI.c_str());
                        _replace_result(result_pdu_item, "FI", pystr);
                        pystr = (strE.c_str());
                        _replace_result(result_pdu_item, "E", pystr);

                        if (strRF == "1") {
                            // decode LSF and SO
                            iLoggedBytes -= 2;

                            offset += _decode_by_fmt(
                                LteRlcDlAmAllPdu_Subpkt_PDU_LSF_SO,
                                ARRAY_SIZE(LteRlcDlAmAllPdu_Subpkt_PDU_LSF_SO,
                                           Fmt),
                                b, offset, length, result_pdu_item);
                            int temp =
                                _search_result_int(result_pdu_item, "LSF");
                            int iLSF = temp >> 7;
                            int iSO = _search_result_int(result_pdu_item, "SO");
                            iSO += (temp & 127) * 256;
                            _replace_result_int(result_pdu_item, "LSF", iLSF);
                            _replace_result_int(result_pdu_item, "SO", iSO);
                        }

                        if (strE == "1") {
                            // Decode LI
                            int numLI = iLoggedBytes / 1.5;
                            iLoggedBytes = 0;
                            int iAllign = 0;
                            int iHeadFromAllign = 0;

                            json result_pdu_li;
                            for (int indexLI = 0; indexLI < numLI; indexLI++) {
                                json result_pdu_li_item;
                                if (iAllign == 0) {
                                    iAllign = 1;
                                    offset += _decode_by_fmt(
                                        LteRlcDlAmAllPdu_Subpkt_PDU_LI_ALLIGN,
                                        ARRAY_SIZE(
                                            LteRlcDlAmAllPdu_Subpkt_PDU_LI_ALLIGN,
                                            Fmt),
                                        b, offset, length, result_pdu_li_item);
                                    int iNonDecodeLI = _search_result_int(
                                        result_pdu_li_item, "LI");
                                    int iPart3 = iNonDecodeLI / 4096;
                                    int iPart4 =
                                        (iNonDecodeLI - iPart3 * 4096) / 256;
                                    int iPart1 = (iNonDecodeLI - iPart3 * 4096 -
                                                  iPart4 * 256) /
                                                 16;
                                    int iPart2 = iNonDecodeLI - iPart3 * 4096 -
                                                 iPart4 * 256 - iPart1 * 16;
                                    int iLI = (iPart1 % 8) * 256 + iPart2 * 16 +
                                              iPart3;
                                    iHeadFromAllign = (iPart4 % 8) * 256;
                                    _replace_result_int(result_pdu_li_item,
                                                        "LI", iLI);
                                } else {
                                    iAllign = 0;
                                    offset += _decode_by_fmt(
                                        LteRlcDlAmAllPdu_Subpkt_PDU_LI_PADDING,
                                        ARRAY_SIZE(
                                            LteRlcDlAmAllPdu_Subpkt_PDU_LI_PADDING,
                                            Fmt),
                                        b, offset, length, result_pdu_li_item);
                                    int iNonDecodeLI = _search_result_int(
                                        result_pdu_li_item, "LI");
                                    int iLI = iHeadFromAllign + iNonDecodeLI;
                                    _replace_result_int(result_pdu_li_item,
                                                        "LI", iLI);
                                }
                                result_pdu_li.push_back(result_pdu_li_item);
                            }
                            result_pdu_item["RLC DATA LI"] = result_pdu_li;
                        }
                    }
                    offset += iLoggedBytes;
                    //result_pdu[("RLCDL PDU[" + SSTR(i) + "]").c_str()] =
                    //    result_pdu_item;
					result_pdu.push_back(result_pdu_item);
                }
                result_subpkt["RLCDL PDUs"] = result_pdu;
            } else {
                printf("Unkown LTE RLC DL AM ALL PDU subpkt id and version"
                       ": 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("Unkown LTE RLC DL AM ALL PDU version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------

static int _decode_lte_mac_rach_trigger_subpkt(const char *b, int offset,
                                               size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Number of Subpackets");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            offset +=
                _decode_by_fmt(LteMacRachTrigger_SubpktHeader,
                               ARRAY_SIZE(LteMacRachTrigger_SubpktHeader, Fmt),
                               b, offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 3 && subpkt_ver == 2) {
                // RACH Config Subpacket
                offset += _decode_by_fmt(
                    LteMacRachTrigger_RachConfigSubpktPayload,
                    ARRAY_SIZE(LteMacRachTrigger_RachConfigSubpktPayload, Fmt),
                    b, offset, length, result_subpkt);
                _replace_result_int(result_subpkt, "Preamble Format", 0);
            } else if (subpkt_id == 3 && subpkt_ver == 4) {
                offset += _decode_by_fmt(
                    LteMacRachTrigger_RachConfigSubpktPayload_v4,
                    ARRAY_SIZE(LteMacRachTrigger_RachConfigSubpktPayload_v4,
                               Fmt),
                    b, offset, length, result_subpkt);
                int num_Cells =
                    _search_result_int(result_subpkt, "Num Active Cells");

                json result_Cells;
                for (int j = 0; j < num_Cells; j++) {
                    json result_Cell_item;
                    offset += _decode_by_fmt(
                        LteMacRachTrigger_RachConfigSubpktPayload_v4_cell,
                        ARRAY_SIZE(
                            LteMacRachTrigger_RachConfigSubpktPayload_v4_cell,
                            Fmt),
                        b, offset, length, result_Cell_item);
                    _replace_result_int(result_Cell_item, "Preamble Format", 0);

                    result_Cells.push_back(result_Cell_item);
                }
                result_subpkt["Cells"] = result_Cells;

            } else if (subpkt_id == 5 && subpkt_ver == 1) {
                offset += _decode_by_fmt(
                    LteMacRachTrigger_RachReasonSubpktPayload,
                    ARRAY_SIZE(LteMacRachTrigger_RachReasonSubpktPayload, Fmt),
                    b, offset, length, result_subpkt);
                (void)_map_result_field_to_name(
                    result_subpkt, "Rach reason",
                    LteMacRachTrigger_RachReasonSubpkt_RachReason,
                    ARRAY_SIZE(LteMacRachTrigger_RachReasonSubpkt_RachReason,
                               ValueName),
                    "(MI)Unknown");
                std::string strRachContention =
                    "Contention Based RACH procedure";
                std::string pystr = strRachContention.c_str();
                _replace_result(result_subpkt, "RACH Contention", pystr);
            } else if (subpkt_id == 5 && subpkt_ver == 2) {
                offset += _decode_by_fmt(
                    LteMacRachTrigger_RachReasonSubpktPayload_v2,
                    ARRAY_SIZE(LteMacRachTrigger_RachReasonSubpktPayload_v2,
                               Fmt),
                    b, offset, length, result_subpkt);
                (void)_map_result_field_to_name(
                    result_subpkt, "Rach reason",
                    LteMacRachTrigger_RachReasonSubpkt_RachReason,
                    ARRAY_SIZE(LteMacRachTrigger_RachReasonSubpkt_RachReason,
                               ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_subpkt, "RACH Contention", ValueNameRachContention,
                    ARRAY_SIZE(ValueNameRachContention, ValueName),
                    "(MI)Unknown");
            } else {
                printf("(MI)Unknown LTE MAC RACH Trigger subpkt id and "
                       "version: 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE MAC RACH Trigger Packet Version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------

static int _decode_lte_mac_rach_attempt_subpkt(const char *b, int offset,
                                               size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Number of Subpackets");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            offset +=
                _decode_by_fmt(LteMacRachAttempt_SubpktHeader,
                               ARRAY_SIZE(LteMacRachAttempt_SubpktHeader, Fmt),
                               b, offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 6 && subpkt_ver == 2) {
                offset += _decode_by_fmt(
                    LteMacRachAttempt_SubpktPayload,
                    ARRAY_SIZE(LteMacRachAttempt_SubpktPayload, Fmt), b, offset,
                    length, result_subpkt);
                int iRachMsgBMasks =
                    _search_result_int(result_subpkt, "Rach msg bmasks");
                (void)_map_result_field_to_name(
                    result_subpkt, "Rach result",
                    LteMacRachAttempt_Subpkt_RachResult,
                    ARRAY_SIZE(LteMacRachAttempt_Subpkt_RachResult, ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_subpkt, "Contention procedure",
                    LteMacRachAttempt_Subpkt_ContentionProcedure,
                    ARRAY_SIZE(LteMacRachAttempt_Subpkt_ContentionProcedure,
                               ValueName),
                    "(MI)Unknown");
                if ((iRachMsgBMasks) & (1 << 2)) {
                    // Msg1
                    json result_subpkt_msg1;
                    offset += _decode_by_fmt(
                        LteMacRachAttempt_Subpkt_Msg1,
                        ARRAY_SIZE(LteMacRachAttempt_Subpkt_Msg1, Fmt), b,
                        offset, length, result_subpkt_msg1);
                    result_subpkt["Msg1"] = result_subpkt_msg1;
                }
                if ((iRachMsgBMasks) & (1 << 1)) {
                    // Msg2
                    json result_subpkt_msg2;
                    offset += _decode_by_fmt(
                        LteMacRachAttempt_Subpkt_Msg2,
                        ARRAY_SIZE(LteMacRachAttempt_Subpkt_Msg2, Fmt), b,
                        offset, length, result_subpkt_msg2);
                    (void)_map_result_field_to_name(
                        result_subpkt_msg2, "Result",
                        LteMacRachAttempt_Subpkt_Msg2_Result,
                        ARRAY_SIZE(LteMacRachAttempt_Subpkt_Msg2_Result,
                                   ValueName),
                        "False");
                    result_subpkt["Msg2"] = result_subpkt_msg2;
                }
                if ((iRachMsgBMasks) & (1)) {
                    // Msg3
                    json result_subpkt_msg3;
                    offset += _decode_by_fmt(
                        LteMacRachAttempt_Subpkt_Msg3,
                        ARRAY_SIZE(LteMacRachAttempt_Subpkt_Msg3, Fmt), b,
                        offset, length, result_subpkt_msg3);
                    int iGrantBytes =
                        _search_result_int(result_subpkt_msg3, "Grant");
                    json result_MACPDUs;
                    for (int j = 0; j < iGrantBytes + 1; j++) {
                        json result_MACPDU_item;
                        offset += _decode_by_fmt(
                            LteMacRachAttempt_Subpkt_Msg3_MACPDU,
                            ARRAY_SIZE(LteMacRachAttempt_Subpkt_Msg3_MACPDU,
                                       Fmt),
                            b, offset, length, result_MACPDU_item);
                        result_MACPDUs.push_back(result_MACPDU_item);
                    }
                    // add pdu list
                    result_subpkt_msg3["MAC PDUs"] = result_MACPDUs;
                    // add Msg3 dict
                    result_subpkt["Msg3"] = result_subpkt_msg3;
                }

            } else {
                printf("(MI)Unknown LTE MAC RACH Attempt Subpkt id and Version"
                       ": 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE MAC RACH Attempt Packet Version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pdcp_dl_config_subpkt(const char *b, int offset,
                                             size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num Subpkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // decode subpacket header
            offset +=
                _decode_by_fmt(LtePdcpDlConfig_SubpktHeader,
                               ARRAY_SIZE(LtePdcpDlConfig_SubpktHeader, Fmt), b,
                               offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if ((subpkt_id == 192 && subpkt_ver == 2) ||
                (subpkt_id == 192 && subpkt_ver == 24)) {
                // PDCP DL Config 0xC0
                offset += _decode_by_fmt(
                    LtePdcpDlConfig_SubpktPayload,
                    ARRAY_SIZE(LtePdcpDlConfig_SubpktPayload, Fmt), b, offset,
                    length, result_subpkt);
                (void)_map_result_field_to_name(
                    result_subpkt, "Reason", LtePdcpDlConfig_Subpkt_Reason,
                    ARRAY_SIZE(LtePdcpDlConfig_Subpkt_Reason, ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_subpkt, "SRB Cipher Algorithm",
                    LtePdcpDlConfig_Subpkt_CipherAlgo,
                    ARRAY_SIZE(LtePdcpDlConfig_Subpkt_CipherAlgo, ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_subpkt, "DRB Cipher Algorithm",
                    LtePdcpDlConfig_Subpkt_CipherAlgo,
                    ARRAY_SIZE(LtePdcpDlConfig_Subpkt_CipherAlgo, ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_subpkt, "SRB Integrity Algorithm",
                    LtePdcpDlConfig_Subpkt_IntegAlgo,
                    ARRAY_SIZE(LtePdcpDlConfig_Subpkt_IntegAlgo, ValueName),
                    "(MI)Unknown");
                int iArraySize =
                    _search_result_int(result_subpkt, "Array size");

                // Released RB
                int start_ReleasedRBStruct = offset;
                offset += _decode_by_fmt(
                    LtePdcpDlConfig_Subpkt_ReleaseRB_Header,
                    ARRAY_SIZE(LtePdcpDlConfig_Subpkt_ReleaseRB_Header, Fmt), b,
                    offset, length, result_subpkt);
                int num_ReleasedRB =
                    _search_result_int(result_subpkt, "Number of Released RBs");
                json result_ReleasedRB;
                for (int j = 0; j < num_ReleasedRB; j++) {
                    json result_ReleasedRB_item;
                    offset += _decode_by_fmt(
                        LtePdcpDlConfig_Subpkt_ReleaseRB_Fmt,
                        ARRAY_SIZE(LtePdcpDlConfig_Subpkt_ReleaseRB_Fmt, Fmt),
                        b, offset, length, result_ReleasedRB_item);
                    result_ReleasedRB.push_back(result_ReleasedRB_item);
                }
                result_subpkt["Released RBs"] = result_ReleasedRB;
                offset +=
                    1 + iArraySize * 1 - (offset - start_ReleasedRBStruct);

                // Added/Modified RB
                int start_AddedModifiedRBStruct = offset;
                offset += _decode_by_fmt(
                    LtePdcpDlConfig_Subpkt_AddedModifiedRB_Header,
                    ARRAY_SIZE(LtePdcpDlConfig_Subpkt_AddedModifiedRB_Header,
                               Fmt),
                    b, offset, length, result_subpkt);
                int num_AddedModifiedRB = _search_result_int(
                    result_subpkt, "Number of Added/Modified RBs");
                json result_AddedModifiedRB;
                for (int j = 0; j < num_AddedModifiedRB; j++) {
                    json result_AddedModifiedRB_item;
                    offset += _decode_by_fmt(
                        LtePdcpDlConfig_Subpkt_AddedModifiedRB_Fmt,
                        ARRAY_SIZE(LtePdcpDlConfig_Subpkt_AddedModifiedRB_Fmt,
                                   Fmt),
                        b, offset, length, result_AddedModifiedRB_item);
                    (void)_map_result_field_to_name(
                        result_AddedModifiedRB_item, "Action",
                        LtePdcpDlConfig_Subpkt_AddedModifiedRB_Action,
                        ARRAY_SIZE(
                            LtePdcpDlConfig_Subpkt_AddedModifiedRB_Action,
                            ValueName),
                        "(MI)Unknown");
                    result_AddedModifiedRB.push_back(
                        result_AddedModifiedRB_item);
                }
                result_subpkt["Added/Modified RBs"] = result_AddedModifiedRB;
                offset +=
                    1 + iArraySize * 2 - (offset - start_AddedModifiedRBStruct);

                // Active RB
                // int start_ActiveRBStruct = offset;
                offset += _decode_by_fmt(
                    LtePdcpDlConfig_Subpkt_ActiveRB_Header,
                    ARRAY_SIZE(LtePdcpDlConfig_Subpkt_ActiveRB_Header, Fmt), b,
                    offset, length, result_subpkt);
                int num_ActiveRB =
                    _search_result_int(result_subpkt, "Number of active RBs");
                json result_ActiveRB;
                for (int j = 0; j < num_ActiveRB; j++) {
                    json result_ActiveRB_item;
                    offset += _decode_by_fmt(
                        LtePdcpDlConfig_Subpkt_ActiveRB_Fmt,
                        ARRAY_SIZE(LtePdcpDlConfig_Subpkt_ActiveRB_Fmt, Fmt), b,
                        offset, length, result_ActiveRB_item);
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RB mode",
                        LtePdcpDlConfig_Subpkt_ActiveRB_RBmode,
                        ARRAY_SIZE(LtePdcpDlConfig_Subpkt_ActiveRB_RBmode,
                                   ValueName),
                        "(MI)Unknown");
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RB type",
                        LtePdcpDlConfig_Subpkt_ActiveRB_RBtype,
                        ARRAY_SIZE(LtePdcpDlConfig_Subpkt_ActiveRB_RBtype,
                                   ValueName),
                        "(MI)Unknown");
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "Status report",
                        LtePdcpDlConfig_Subpkt_ActiveRB_StatusReport,
                        ARRAY_SIZE(LtePdcpDlConfig_Subpkt_ActiveRB_StatusReport,
                                   ValueName),
                        "YES");
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RoHC Enabled",
                        LtePdcpDlConfig_Subpkt_ActiveRB_RoHCEnabled,
                        ARRAY_SIZE(LtePdcpDlConfig_Subpkt_ActiveRB_RoHCEnabled,
                                   ValueName),
                        "true");
                    result_ActiveRB.push_back(result_ActiveRB_item);
                }
                result_subpkt["Active RBs"] = result_ActiveRB;
            } else {
                printf("(MI)Unknown LTE PDCP DL Config subpkt id and version:"
                       " 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PDCP DL Config Log Packet version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pdcp_ul_config_subpkt(const char *b, int offset,
                                             size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num Subpkt");
    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // decode subpacket header
            offset +=
                _decode_by_fmt(LtePdcpUlConfig_SubpktHeader,
                               ARRAY_SIZE(LtePdcpUlConfig_SubpktHeader, Fmt), b,
                               offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if ((subpkt_id == 193 && subpkt_ver == 2) ||
                (subpkt_id == 193 && subpkt_ver == 24)) {
                // PDCP UL Config 0xC1
                offset += _decode_by_fmt(
                    LtePdcpUlConfig_SubpktPayload,
                    ARRAY_SIZE(LtePdcpUlConfig_SubpktPayload, Fmt), b, offset,
                    length, result_subpkt);
                (void)_map_result_field_to_name(
                    result_subpkt, "Reason", LtePdcpUlConfig_Subpkt_Reason,
                    ARRAY_SIZE(LtePdcpUlConfig_Subpkt_Reason, ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_subpkt, "SRB Cipher Algorithm",
                    LtePdcpUlConfig_Subpkt_CipherAlgo,
                    ARRAY_SIZE(LtePdcpUlConfig_Subpkt_CipherAlgo, ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_subpkt, "DRB Cipher Algorithm",
                    LtePdcpUlConfig_Subpkt_CipherAlgo,
                    ARRAY_SIZE(LtePdcpUlConfig_Subpkt_CipherAlgo, ValueName),
                    "(MI)Unknown");
                (void)_map_result_field_to_name(
                    result_subpkt, "SRB Integrity Algorithm",
                    LtePdcpUlConfig_Subpkt_IntegAlgo,
                    ARRAY_SIZE(LtePdcpUlConfig_Subpkt_IntegAlgo, ValueName),
                    "(MI)Unknown");
                int iArraySize =
                    _search_result_int(result_subpkt, "Array size");

                // Released RB
                int start_ReleasedRBStruct = offset;
                offset += _decode_by_fmt(
                    LtePdcpUlConfig_Subpkt_ReleaseRB_Header,
                    ARRAY_SIZE(LtePdcpUlConfig_Subpkt_ReleaseRB_Header, Fmt), b,
                    offset, length, result_subpkt);
                int num_ReleasedRB =
                    _search_result_int(result_subpkt, "Number of Released RBs");
                json result_ReleasedRB;
                for (int j = 0; j < num_ReleasedRB; j++) {
                    json result_ReleasedRB_item;
                    offset += _decode_by_fmt(
                        LtePdcpUlConfig_Subpkt_ReleaseRB_Fmt,
                        ARRAY_SIZE(LtePdcpUlConfig_Subpkt_ReleaseRB_Fmt, Fmt),
                        b, offset, length, result_ReleasedRB_item);
                    result_ReleasedRB.push_back(result_ReleasedRB_item);
                }
                result_subpkt["Released RBs"] = result_ReleasedRB;
                offset +=
                    1 + iArraySize * 1 - (offset - start_ReleasedRBStruct);

                // Added/Modified RB
                int start_AddedModifiedRBStruct = offset;
                offset += _decode_by_fmt(
                    LtePdcpUlConfig_Subpkt_AddedModifiedRB_Header,
                    ARRAY_SIZE(LtePdcpUlConfig_Subpkt_AddedModifiedRB_Header,
                               Fmt),
                    b, offset, length, result_subpkt);
                int num_AddedModifiedRB = _search_result_int(
                    result_subpkt, "Number of Added/Modified RBs");
                json result_AddedModifiedRB;
                for (int j = 0; j < num_AddedModifiedRB; j++) {
                    json result_AddedModifiedRB_item;
                    offset += _decode_by_fmt(
                        LtePdcpUlConfig_Subpkt_AddedModifiedRB_Fmt,
                        ARRAY_SIZE(LtePdcpUlConfig_Subpkt_AddedModifiedRB_Fmt,
                                   Fmt),
                        b, offset, length, result_AddedModifiedRB_item);
                    (void)_map_result_field_to_name(
                        result_AddedModifiedRB_item, "Action",
                        LtePdcpUlConfig_Subpkt_AddedModifiedRB_Action,
                        ARRAY_SIZE(
                            LtePdcpUlConfig_Subpkt_AddedModifiedRB_Action,
                            ValueName),
                        "(MI)Unknown");
                    result_AddedModifiedRB.push_back(
                        result_AddedModifiedRB_item);
                }
                result_subpkt["Added/Modified RBs"] = result_AddedModifiedRB;
                offset +=
                    1 + iArraySize * 2 - (offset - start_AddedModifiedRBStruct);

                // Active RB
                // int start_ActiveRBStruct = offset;
                offset += _decode_by_fmt(
                    LtePdcpUlConfig_Subpkt_ActiveRB_Header,
                    ARRAY_SIZE(LtePdcpUlConfig_Subpkt_ActiveRB_Header, Fmt), b,
                    offset, length, result_subpkt);
                int num_ActiveRB =
                    _search_result_int(result_subpkt, "Number of active RBs");
                json result_ActiveRB;
                for (int j = 0; j < num_ActiveRB; j++) {
                    json result_ActiveRB_item;
                    offset += _decode_by_fmt(
                        LtePdcpUlConfig_Subpkt_ActiveRB_Fmt,
                        ARRAY_SIZE(LtePdcpUlConfig_Subpkt_ActiveRB_Fmt, Fmt), b,
                        offset, length, result_ActiveRB_item);
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RB mode",
                        LtePdcpUlConfig_Subpkt_ActiveRB_RBmode,
                        ARRAY_SIZE(LtePdcpUlConfig_Subpkt_ActiveRB_RBmode,
                                   ValueName),
                        "(MI)Unknown");
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RB type",
                        LtePdcpUlConfig_Subpkt_ActiveRB_RBtype,
                        ARRAY_SIZE(LtePdcpUlConfig_Subpkt_ActiveRB_RBtype,
                                   ValueName),
                        "(MI)Unknown");
                    (void)_map_result_field_to_name(
                        result_ActiveRB_item, "RoHC Enabled",
                        LtePdcpUlConfig_Subpkt_ActiveRB_RoHCEnabled,
                        ARRAY_SIZE(LtePdcpUlConfig_Subpkt_ActiveRB_RoHCEnabled,
                                   ValueName),
                        "true");
                    result_ActiveRB.push_back(result_ActiveRB_item);
                }
                result_subpkt["Active RBs"] = result_ActiveRB;
            } else {
                printf("(MI)Unknown LTE PDCP UL Config subpkt id and version:"
                       " 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PDCP UL Config Log Packet version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pdcp_ul_data_pdu_subpkt(const char *b, int offset,
                                               size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num Subpkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // decode subpacket header
            offset +=
                _decode_by_fmt(LtePdcpUlDataPdu_SubpktHeader,
                               ARRAY_SIZE(LtePdcpUlDataPdu_SubpktHeader, Fmt),
                               b, offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 194 && subpkt_ver == 1) {
                // PDCP UL Data PDU 0xC2
                offset += _decode_by_fmt(
                    LtePdcpUlDataPdu_SubpktPayload,
                    ARRAY_SIZE(LtePdcpUlDataPdu_SubpktPayload, Fmt), b, offset,
                    length, result_subpkt);
                (void)_map_result_field_to_name(
                    result_subpkt, "Mode", LtePdcpUlDataPdu_Subpkt_Mode,
                    ARRAY_SIZE(LtePdcpUlDataPdu_Subpkt_Mode, ValueName),
                    "(MI)Unknown");

                // PDU
                // int start_PDU = offset;
                offset += _decode_by_fmt(
                    LtePdcpUlDataPdu_Subpkt_PDU_Header,
                    ARRAY_SIZE(LtePdcpUlDataPdu_Subpkt_PDU_Header, Fmt), b,
                    offset, length, result_subpkt);
                int num_PDU = _search_result_int(result_subpkt, "Num PDUs");
                json result_PDU;
                for (int j = 0; j < num_PDU; j++) {
                    json result_PDU_item;
                    offset += _decode_by_fmt(
                        LtePdcpUlDataPdu_Subpkt_PDU_Fmt,
                        ARRAY_SIZE(LtePdcpUlDataPdu_Subpkt_PDU_Fmt, Fmt), b,
                        offset, length, result_PDU_item);
                    int iNonDecodeFN = _search_result_int(
                        result_PDU_item, "System Frame Number");
                    // Handle fn
                    const unsigned int SFN_RSHIFT = 4, SFN_MASK = (1 << 12) - 1;
                    const unsigned int SUBFRAME_RSHIFT = 0,
                                       SUBFRAME_MASK = (1 << 4) - 1;
                    int sys_fn = (iNonDecodeFN >> SFN_RSHIFT) & SFN_MASK;
                    int sub_fn =
                        (iNonDecodeFN >> SUBFRAME_RSHIFT) & SUBFRAME_MASK;
                    _replace_result_int(result_PDU_item, "System Frame Number",
                                        sys_fn);
                    _replace_result_int(result_PDU_item, "Subframe Number",
                                        sub_fn);

                    result_PDU.push_back(result_PDU_item);
                }
                result_subpkt["PDCP UL Data PDU"] = result_PDU;
            } else {
                printf("(MI)Unknown LTE PDCP UL Data PDU subpkt id and version:"
                       " 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PDCP UL Data PDU version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pdcp_dl_stats_subpkt(const char *b, int offset,
                                            size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num Subpkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // decode subpacket header
            offset +=
                _decode_by_fmt(LtePdcpDlStats_SubpktHeader,
                               ARRAY_SIZE(LtePdcpDlStats_SubpktHeader, Fmt), b,
                               offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 196 && subpkt_ver == 2) {
                // PDCP DL Stats: 0xC2
                offset += _decode_by_fmt(
                    LtePdcpDlStats_SubpktPayload,
                    ARRAY_SIZE(LtePdcpDlStats_SubpktPayload, Fmt), b, offset,
                    length, result_subpkt);
                // RBs
                int num_RB = _search_result_int(result_subpkt, "Num RBs");
                json result_RB;
                for (int j = 0; j < num_RB; j++) {
                    json result_RB_item;
                    offset += _decode_by_fmt(
                        LtePdcpDlStats_Subpkt_RB_Fmt,
                        ARRAY_SIZE(LtePdcpDlStats_Subpkt_RB_Fmt, Fmt), b,
                        offset, length, result_RB_item);
                    (void)_map_result_field_to_name(
                        result_RB_item, "Mode", LtePdcpDlStats_Subpkt_RB_Mode,
                        ARRAY_SIZE(LtePdcpDlStats_Subpkt_RB_Mode, ValueName),
                        "(MI)Unknown");
                    result_RB.push_back(result_RB_item);
                }
                result_subpkt["RBs"] = result_RB;
            } else {
                printf("(MI)Unknown LTE PDCP DL Stats subpkt id and version:"
                       " 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PDCP DL Stats version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pdcp_ul_stats_subpkt(const char *b, int offset,
                                            size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num Subpkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // decode subpacket header
            offset +=
                _decode_by_fmt(LtePdcpUlStats_SubpktHeader,
                               ARRAY_SIZE(LtePdcpUlStats_SubpktHeader, Fmt), b,
                               offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 197 && subpkt_ver == 1) {
                // PDCP UL Stats: 0xC5
                offset += _decode_by_fmt(
                    LtePdcpUlStats_SubpktPayload_v1,
                    ARRAY_SIZE(LtePdcpUlStats_SubpktPayload_v1, Fmt), b, offset,
                    length, result_subpkt);
                // RBs
                int num_RB = _search_result_int(result_subpkt, "Num RBs");
                json result_RB;
                for (int j = 0; j < num_RB; j++) {
                    json result_RB_item;
                    offset += _decode_by_fmt(
                        LtePdcpUlStats_Subpkt_RB_Fmt_v1,
                        ARRAY_SIZE(LtePdcpUlStats_Subpkt_RB_Fmt_v1, Fmt), b,
                        offset, length, result_RB_item);
                    (void)_map_result_field_to_name(
                        result_RB_item, "Mode", LtePdcpUlStats_Subpkt_RB_Mode,
                        ARRAY_SIZE(LtePdcpUlStats_Subpkt_RB_Mode, ValueName),
                        "(MI)Unknown");
                    result_RB.push_back(result_RB_item);
                }
                result_subpkt["RBs"] = result_RB;
            } else if (subpkt_id == 197 && subpkt_ver == 2) {
                // PDCP UL Stats: 0xC5
                offset += _decode_by_fmt(
                    LtePdcpUlStats_SubpktPayload_v2,
                    ARRAY_SIZE(LtePdcpUlStats_SubpktPayload_v2, Fmt), b, offset,
                    length, result_subpkt);
                // RBs
                int num_RB = _search_result_int(result_subpkt, "Num RBs");
                json result_RB;
                for (int j = 0; j < num_RB; j++) {
                    json result_RB_item;
                    offset += _decode_by_fmt(
                        LtePdcpUlStats_Subpkt_RB_Fmt_v2,
                        ARRAY_SIZE(LtePdcpUlStats_Subpkt_RB_Fmt_v2, Fmt), b,
                        offset, length, result_RB_item);
                    (void)_map_result_field_to_name(
                        result_RB_item, "Mode", LtePdcpUlStats_Subpkt_RB_Mode,
                        ARRAY_SIZE(LtePdcpUlStats_Subpkt_RB_Mode, ValueName),
                        "(MI)Unknown");
                    result_RB.push_back(result_RB_item);
                }
                result_subpkt["RBs"] = result_RB;
            } else if (subpkt_id == 197 && subpkt_ver == 26) {
                // PDCP UL Stats: 0xC5
                offset += _decode_by_fmt(
                    LtePdcpUlStats_SubpktPayload_v26,
                    ARRAY_SIZE(LtePdcpUlStats_SubpktPayload_v26, Fmt), b,
                    offset, length, result_subpkt);
                // RBs
                int num_RB = _search_result_int(result_subpkt, "Num RBs");
                json result_RB;
                for (int j = 0; j < num_RB; j++) {
                    json result_RB_item;
                    offset += _decode_by_fmt(
                        LtePdcpUlStats_Subpkt_RB_Fmt_v26,
                        ARRAY_SIZE(LtePdcpUlStats_Subpkt_RB_Fmt_v26, Fmt), b,
                        offset, length, result_RB_item);

                    (void)_map_result_field_to_name(
                        result_RB_item, "Mode", LtePdcpUlStats_Subpkt_RB_Mode,
                        ARRAY_SIZE(LtePdcpUlStats_Subpkt_RB_Mode, ValueName),
                        "(MI)Unknown");

                    (void)_map_result_field_to_name(
                        result_RB_item, "UDC Comp State",
                        LtePdcpUlStats_Subpkt_UDC_Comp_state,
                        ARRAY_SIZE(LtePdcpUlStats_Subpkt_UDC_Comp_state,
                                   ValueName),
                        "(MI)Unknown");

                    result_RB.push_back(result_RB_item);
                }
                result_subpkt["RBs"] = result_RB;
            } else {
                printf("(MI)Unknown LTE PDCP UL Stats subpkt id and version:"
                       " 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PDCP UL Stats version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_rlc_ul_stats_subpkt(const char *b, int offset,
                                           size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num Subpkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // decode subpacket header
            offset +=
                _decode_by_fmt(LteRlcUlStats_SubpktHeader,
                               ARRAY_SIZE(LteRlcUlStats_SubpktHeader, Fmt), b,
                               offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 71 && subpkt_ver == 1) {
                // LTE RLC UL Stats: 0x47
                offset +=
                    _decode_by_fmt(LteRlcUlStats_SubpktPayload,
                                   ARRAY_SIZE(LteRlcUlStats_SubpktPayload, Fmt),
                                   b, offset, length, result_subpkt);
                // RBs
                int num_RB = _search_result_int(result_subpkt, "Num RBs");
                json result_RB;
                for (int j = 0; j < num_RB; j++) {
                    json result_RB_item;
                    offset += _decode_by_fmt(
                        LteRlcUlStats_Subpkt_RB_Fmt,
                        ARRAY_SIZE(LteRlcUlStats_Subpkt_RB_Fmt, Fmt), b, offset,
                        length, result_RB_item);
                    (void)_map_result_field_to_name(
                        result_RB_item, "Mode", LteRlcUlStats_Subpkt_RB_Mode,
                        ARRAY_SIZE(LteRlcUlStats_Subpkt_RB_Mode, ValueName),
                        "(MI)Unknown");
                    result_RB.push_back(result_RB_item);
                }
                result_subpkt["RBs"] = result_RB;
            } else {
                printf("(MI)Unknown LTE RLC UL Stats subpkt id and version:"
                       " 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE RLC UL Stats version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_rlc_dl_stats_subpkt(const char *b, int offset,
                                           size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num Subpkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // decode subpacket header
            offset +=
                _decode_by_fmt(LteRlcDlStats_SubpktHeader,
                               ARRAY_SIZE(LteRlcDlStats_SubpktHeader, Fmt), b,
                               offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 66 && subpkt_ver == 3) {
                // LTE RLC DL Stats: 0x42
                offset += _decode_by_fmt(
                    LteRlcDlStats_SubpktPayload_v3,
                    ARRAY_SIZE(LteRlcDlStats_SubpktPayload_v3, Fmt), b, offset,
                    length, result_subpkt);
                // RBs
                int num_RB = _search_result_int(result_subpkt, "Num RBs");
                json result_RB;
                for (int j = 0; j < num_RB; j++) {
                    json result_RB_item;
                    offset += _decode_by_fmt(
                        LteRlcDlStats_Subpkt_RB_Fmt_v3,
                        ARRAY_SIZE(LteRlcDlStats_Subpkt_RB_Fmt_v3, Fmt), b,
                        offset, length, result_RB_item);
                    (void)_map_result_field_to_name(
                        result_RB_item, "Mode", LteRlcDlStats_Subpkt_RB_Mode,
                        ARRAY_SIZE(LteRlcDlStats_Subpkt_RB_Mode, ValueName),
                        "(MI)Unknown");
                    result_RB.push_back(result_RB_item);
                }
                result_subpkt["RBs"] = result_RB;
            } else if (subpkt_id == 66 && subpkt_ver == 2) {
                // LTE RLC DL Stats: 0x42
                offset += _decode_by_fmt(
                    LteRlcDlStats_SubpktPayload_v2,
                    ARRAY_SIZE(LteRlcDlStats_SubpktPayload_v2, Fmt), b, offset,
                    length, result_subpkt);
                // RBs
                int num_RB = _search_result_int(result_subpkt, "Num RBs");
                json result_RB;
                for (int j = 0; j < num_RB; j++) {
                    json result_RB_item;
                    offset += _decode_by_fmt(
                        LteRlcDlStats_Subpkt_RB_Fmt_v2,
                        ARRAY_SIZE(LteRlcDlStats_Subpkt_RB_Fmt_v2, Fmt), b,
                        offset, length, result_RB_item);
                    (void)_map_result_field_to_name(
                        result_RB_item, "Mode", LteRlcDlStats_Subpkt_RB_Mode,
                        ARRAY_SIZE(LteRlcDlStats_Subpkt_RB_Mode, ValueName),
                        "(MI)Unknown");
                    result_RB.push_back(result_RB_item);
                }
                result_subpkt["RBs"] = result_RB;
            } else {
                printf("(MI)Unknown LTE RLC DL Stats subpkt id and version:"
                       " 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE RLC DL Stats version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pdcp_dl_ctrl_pdu_subpkt(const char *b, int offset,
                                               size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num Subpkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // decode subpacket header
            offset +=
                _decode_by_fmt(LtePdcpDlCtrlPdu_SubpktHeader,
                               ARRAY_SIZE(LtePdcpDlCtrlPdu_SubpktHeader, Fmt),
                               b, offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 194 && subpkt_ver == 1) {
                // PDCP DL Ctrl PDU 0xC2
                offset += _decode_by_fmt(
                    LtePdcpDlCtrlPdu_SubpktPayload,
                    ARRAY_SIZE(LtePdcpDlCtrlPdu_SubpktPayload, Fmt), b, offset,
                    length, result_subpkt);
                (void)_map_result_field_to_name(
                    result_subpkt, "Mode", LtePdcpDlCtrlPdu_Subpkt_Mode,
                    ARRAY_SIZE(LtePdcpDlCtrlPdu_Subpkt_Mode, ValueName),
                    "(MI)Unknown");

                // PDU
                // int start_PDU = offset;
                offset += _decode_by_fmt(
                    LtePdcpDlCtrlPdu_Subpkt_PDU_Header,
                    ARRAY_SIZE(LtePdcpDlCtrlPdu_Subpkt_PDU_Header, Fmt), b,
                    offset, length, result_subpkt);
                int num_PDU = _search_result_int(result_subpkt, "Num PDUs");
                json result_PDU;
                for (int j = 0; j < num_PDU; j++) {
                    json result_PDU_item;
                    offset += _decode_by_fmt(
                        LtePdcpDlCtrlPdu_Subpkt_PDU_Fmt,
                        ARRAY_SIZE(LtePdcpDlCtrlPdu_Subpkt_PDU_Fmt, Fmt), b,
                        offset, length, result_PDU_item);
                    int iNonDecodeFN = _search_result_int(
                        result_PDU_item, "System Frame Number");
                    // Handle fn
                    const unsigned int SFN_RSHIFT = 4, SFN_MASK = (1 << 12) - 1;
                    const unsigned int SUBFRAME_RSHIFT = 0,
                                       SUBFRAME_MASK = (1 << 4) - 1;
                    int sys_fn = (iNonDecodeFN >> SFN_RSHIFT) & SFN_MASK;
                    int sub_fn =
                        (iNonDecodeFN >> SUBFRAME_RSHIFT) & SUBFRAME_MASK;
                    _replace_result_int(result_PDU_item, "System Frame Number",
                                        sys_fn);
                    _replace_result_int(result_PDU_item, "Subframe Number",
                                        sub_fn);
                    // type = STATUS REPORT
                    std::string strType = "STATUS REPORT";
                    std::string pystr = strType.c_str();
                    _replace_result(result_PDU_item, "type", pystr);

                    result_PDU.push_back(result_PDU_item);
                }
                result_subpkt["PDCP DL Ctrl PDU"] = result_PDU;
            } else {
                printf("(MI)Unknown LTE PDCP DL Ctrl PDU subpkt id and version:"
                       " 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PDCP DL Ctrl PDU version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pdcp_ul_ctrl_pdu_subpkt(const char *b, int offset,
                                               size_t length, json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");
    int n_subpkt = _search_result_int(result, "Num Subpkt");

    switch (pkt_ver) {
    case 1: {
        json result_allpkts;
        for (int i = 0; i < n_subpkt; i++) {
            json result_subpkt;
            int start_subpkt = offset;
            // decode subpacket header
            offset +=
                _decode_by_fmt(LtePdcpUlCtrlPdu_SubpktHeader,
                               ARRAY_SIZE(LtePdcpUlCtrlPdu_SubpktHeader, Fmt),
                               b, offset, length, result_subpkt);
            int subpkt_id = _search_result_int(result_subpkt, "Subpacket ID");
            int subpkt_ver =
                _search_result_int(result_subpkt, "Subpacket Version");
            int subpkt_size =
                _search_result_int(result_subpkt, "Subpacket Size");
            if (subpkt_id == 194 && subpkt_ver == 1) {
                // PDCP UL Ctrl PDU 0xC2
                offset += _decode_by_fmt(
                    LtePdcpUlCtrlPdu_SubpktPayload,
                    ARRAY_SIZE(LtePdcpUlCtrlPdu_SubpktPayload, Fmt), b, offset,
                    length, result_subpkt);
                (void)_map_result_field_to_name(
                    result_subpkt, "Mode", LtePdcpUlCtrlPdu_Subpkt_Mode,
                    ARRAY_SIZE(LtePdcpUlCtrlPdu_Subpkt_Mode, ValueName),
                    "(MI)Unknown");

                // PDU
                // int start_PDU = offset;
                offset += _decode_by_fmt(
                    LtePdcpUlCtrlPdu_Subpkt_PDU_Header,
                    ARRAY_SIZE(LtePdcpUlCtrlPdu_Subpkt_PDU_Header, Fmt), b,
                    offset, length, result_subpkt);
                int num_PDU = _search_result_int(result_subpkt, "Num PDUs");
                json result_PDU;
                for (int j = 0; j < num_PDU; j++) {
                    json result_PDU_item;
                    offset += _decode_by_fmt(
                        LtePdcpUlCtrlPdu_Subpkt_PDU_Fmt,
                        ARRAY_SIZE(LtePdcpUlCtrlPdu_Subpkt_PDU_Fmt, Fmt), b,
                        offset, length, result_PDU_item);
                    int iNonDecodeFN = _search_result_int(
                        result_PDU_item, "System Frame Number");
                    // Handle fn
                    const unsigned int SFN_RSHIFT = 4, SFN_MASK = (1 << 12) - 1;
                    const unsigned int SUBFRAME_RSHIFT = 0,
                                       SUBFRAME_MASK = (1 << 4) - 1;
                    int sys_fn = (iNonDecodeFN >> SFN_RSHIFT) & SFN_MASK;
                    int sub_fn =
                        (iNonDecodeFN >> SUBFRAME_RSHIFT) & SUBFRAME_MASK;
                    _replace_result_int(result_PDU_item, "System Frame Number",
                                        sys_fn);
                    _replace_result_int(result_PDU_item, "Subframe Number",
                                        sub_fn);
                    // type = STATUS REPORT
                    std::string strType = "STATUS REPORT";
                    std::string pystr = strType.c_str();
                    _replace_result(result_PDU_item, "type", pystr);

                    result_PDU.push_back(result_PDU_item);
                }
                result_subpkt["PDCP DL Ctrl PDU"] = result_PDU;
            } else {
                printf("(MI)Unknown LTE PDCP UL Ctrl PDU subpkt id and version:"
                       " 0x%x - %d\n",
                       subpkt_id, subpkt_ver);
            }
            result_allpkts.push_back(result_subpkt);
            offset += subpkt_size - (offset - start_subpkt);
        }
        result["Subpackets"] = result_allpkts;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PDCP UL Ctrl PDU version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pucch_power_control_payload(const char *b, int offset,
                                                   size_t length,
                                                   json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 24: {
        offset += _decode_by_fmt(LtePucchPowerControl_Fmt_v24,
                                 ARRAY_SIZE(LtePucchPowerControl_Fmt_v24, Fmt),
                                 b, offset, length, result);
        int num_record = _search_result_int(result, "Number of Records");
        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePucchPowerControl_Record_Fmt_v24,
                ARRAY_SIZE(LtePucchPowerControl_Record_Fmt_v24, Fmt), b, offset,
                length, result_record_item);
            unsigned int iNonDecodeSFN =
                _search_result_uint(result_record_item, "SFN");
            int iSFN = iNonDecodeSFN & 1023;         // last 10 bits
            int iSubFN = (iNonDecodeSFN >> 10) & 15; // next 4 bits

            int iPower = (iNonDecodeSFN >> 14) & 255; // next 8 bits

            if ((iPower & 0x80) != 0) {
                iPower = iPower | 0xffffff00;
            }

            int iDCI = (iNonDecodeSFN >> 22) & 15;   // next 4 bits
            int iPUCCH = (iNonDecodeSFN >> 26) & 7;  // next 3 bits
            int iN_HARQ = (iNonDecodeSFN >> 29) & 1; // next 1 bit
            _replace_result_int(result_record_item, "SFN", iSFN);
            _replace_result_int(result_record_item, "Sub-FN", iSubFN);
            _replace_result_int(result_record_item, "PUCCH Tx Power (dBm)",
                                iPower);
            _replace_result_int(result_record_item, "DCI Format", iDCI);
            (void)_map_result_field_to_name(
                result_record_item, "DCI Format",
                LtePucchPowerControl_Record_v4_DCI_Format,
                ARRAY_SIZE(LtePucchPowerControl_Record_v4_DCI_Format,
                           ValueName),
                "(MI)Unknown");
            _replace_result_int(result_record_item, "PUCCH Format", iPUCCH);
            (void)_map_result_field_to_name(
                result_record_item, "PUCCH Format",
                LtePucchPowerControl_Record_v4_PUCCH_Format,
                ARRAY_SIZE(LtePucchPowerControl_Record_v4_PUCCH_Format,
                           ValueName),
                "(MI)Unknown");
            _replace_result_int(result_record_item, "N_HARQ", iN_HARQ);

            unsigned int iNonDecodeTPC =
                _search_result_uint(result_record_item, "TPC Command");

            int iTPC = iNonDecodeTPC & 63;           // last 6 bits
            int iN_CQI = (iNonDecodeTPC >> 6) & 31;  // next 5 bits
            int iLoss = (iNonDecodeTPC >> 11) & 255; // next 8 bits
            _replace_result_int(result_record_item, "TPC Command", iTPC);
            if (iTPC == 31 || iTPC == 63) {
                (void)_map_result_field_to_name(
                    result_record_item, "TPC Command",
                    LtePucchPowerControl_Record_v4_TPC,
                    ARRAY_SIZE(LtePucchPowerControl_Record_v4_TPC, ValueName),
                    "(MI)Unknown");
            }
            _replace_result_int(result_record_item, "N_CQI", iN_CQI);
            _replace_result_int(result_record_item, "DL Path Loss", iLoss);

            int iNonDecodeGi = _search_result_int(result_record_item, "g(i)");
            short iGi = (short)iNonDecodeGi;
            _replace_result_int(result_record_item, "g(i)", iGi);

            int iPucchActualTxPower =
                _search_result_int(result_record_item, "PUCCH Actual Tx Power");

            if ((iPucchActualTxPower & 0x80) != 0) {
                iPucchActualTxPower = iPucchActualTxPower | 0xffffff00;
            }

            _replace_result_int(result_record_item, "PUCCH Actual Tx Power",
                                iPucchActualTxPower);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }

    case 4: {
        offset += _decode_by_fmt(LtePucchPowerControl_Fmt_v4,
                                 ARRAY_SIZE(LtePucchPowerControl_Fmt_v4, Fmt),
                                 b, offset, length, result);
        int num_record = _search_result_int(result, "Number of Records");
        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePucchPowerControl_Record_Fmt_v4,
                ARRAY_SIZE(LtePucchPowerControl_Record_Fmt_v4, Fmt), b, offset,
                length, result_record_item);
            unsigned int iNonDecodeSFN =
                _search_result_uint(result_record_item, "SFN");
            int iSFN = iNonDecodeSFN & 1023;          // last 10 bits
            int iSubFN = (iNonDecodeSFN >> 10) & 15;  // next 4 bits
            int iPower = (iNonDecodeSFN >> 14) & 255; // next 8 bits
            int iDCI = (iNonDecodeSFN >> 22) & 15;    // next 4 bits
            int iPUCCH = (iNonDecodeSFN >> 26) & 7;   // next 3 bits
            int iN_HARQ = (iNonDecodeSFN >> 29) & 1;  // next 1 bit
            _replace_result_int(result_record_item, "SFN", iSFN);
            _replace_result_int(result_record_item, "Sub-FN", iSubFN);
            _replace_result_int(result_record_item, "PUCCH Tx Power (dBm)",
                                iPower);
            _replace_result_int(result_record_item, "DCI Format", iDCI);
            (void)_map_result_field_to_name(
                result_record_item, "DCI Format",
                LtePucchPowerControl_Record_v4_DCI_Format,
                ARRAY_SIZE(LtePucchPowerControl_Record_v4_DCI_Format,
                           ValueName),
                "(MI)Unknown");
            _replace_result_int(result_record_item, "PUCCH Format", iPUCCH);
            (void)_map_result_field_to_name(
                result_record_item, "PUCCH Format",
                LtePucchPowerControl_Record_v4_PUCCH_Format,
                ARRAY_SIZE(LtePucchPowerControl_Record_v4_PUCCH_Format,
                           ValueName),
                "(MI)Unknown");
            _replace_result_int(result_record_item, "N_HARQ", iN_HARQ);

            unsigned int iNonDecodeTPC =
                _search_result_uint(result_record_item, "TPC Command");
            int iTPC = iNonDecodeTPC & 63;           // last 6 bits
            int iN_CQI = (iNonDecodeTPC >> 6) & 31;  // next 5 bits
            int iLoss = (iNonDecodeTPC >> 11) & 127; // next 8 bits
            _replace_result_int(result_record_item, "TPC Command", iTPC);
            if (iTPC == 31 || iTPC == 63) {
                (void)_map_result_field_to_name(
                    result_record_item, "TPC Command",
                    LtePucchPowerControl_Record_v4_TPC,
                    ARRAY_SIZE(LtePucchPowerControl_Record_v4_TPC, ValueName),
                    "(MI)Unknown");
            }
            _replace_result_int(result_record_item, "N_CQI", iN_CQI);
            _replace_result_int(result_record_item, "DL Path Loss", iLoss);

            int iNonDecodeGi = _search_result_int(result_record_item, "g(i)");
            int iGi = iNonDecodeGi - 65535;
            _replace_result_int(result_record_item, "g(i)", iGi);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PUCCH Power Control version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pusch_power_control_payload(const char *b, int offset,
                                                   size_t length,
                                                   json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {

    case 25: {
        offset += _decode_by_fmt(LtePuschPowerControl_Fmt_v25,
                                 ARRAY_SIZE(LtePuschPowerControl_Fmt_v25, Fmt),
                                 b, offset, length, result);
        int num_record = _search_result_int(result, "Number of Records");
        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePuschPowerControl_Record_Fmt_v25,
                ARRAY_SIZE(LtePuschPowerControl_Record_Fmt_v25, Fmt), b, offset,
                length, result_record_item);
            unsigned int iNonDecodeSFN =
                _search_result_uint(result_record_item, "SFN");
            // int iSFN = iNonDecodeSFN & 1023; // last 10 bits
            // printf("iNonDecodeSFN=%#04x\n",iNonDecodeSFN);
            int iSFN = (ntohl(iNonDecodeSFN) >> 16 & 3) |
                       (((ntohl(iNonDecodeSFN) >> 20) & 255) << 2);
            int iSubFN = (iNonDecodeSFN >> 10) & 15;       // next 4 bits
            int iPower = (ntohl(iNonDecodeSFN) >> 9) & 31; // next 8 bits
            int iDCI = (iNonDecodeSFN >> 22) & 15;         // next 4 bits
            int iTxType = (iNonDecodeSFN >> 26) & 3;       // next 3 bits
            _replace_result_int(result_record_item, "SFN", iSFN);
            _replace_result_int(result_record_item, "Sub-FN", iSubFN);
            _replace_result_int(result_record_item, "PUSCH Tx Power (dBm)",
                                iPower);
            _replace_result_int(result_record_item, "DCI Format", iDCI);
            (void)_map_result_field_to_name(
                result_record_item, "DCI Format",
                LtePuschPowerControl_Record_v5_DCI_Format,
                ARRAY_SIZE(LtePuschPowerControl_Record_v5_DCI_Format,
                           ValueName),
                "(MI)Unknown");
            _replace_result_int(result_record_item, "Tx Type", iTxType);
            (void)_map_result_field_to_name(
                result_record_item, "Tx Type",
                LtePuschPowerControl_Record_v5_TxType,
                ARRAY_SIZE(LtePuschPowerControl_Record_v5_TxType, ValueName),
                "(MI)Unknown");

            unsigned int iNonDecodeNRB =
                _search_result_uint(result_record_item, "Num RBs");
            int iNRB = iNonDecodeNRB & 255;          // last 8 bits
            int iTBS = (iNonDecodeNRB >> 8) & 16383; // next 14 bits
            int iLoss = (iNonDecodeNRB >> 22) & 255; // next 8 bits
            _replace_result_int(result_record_item, "Num RBs", iNRB);
            _replace_result_int(result_record_item, "Transport Block Size",
                                iTBS);
            _replace_result_int(result_record_item, "DL Path Loss", iLoss);

            unsigned int iNonDecodeFi =
                _search_result_uint(result_record_item, "F(i)");
            int iFi = iNonDecodeFi & 1023; // last 10 bits
            if (iFi >= 512) {
                iFi -= 1024;
            }
            int iTPC = (iNonDecodeFi >> 10) & 31; // next 5 bits
            int iActualPower = (iNonDecodeFi >> 15) & 255;
            _replace_result_int(result_record_item, "F(i)", iFi);
            _replace_result_int(result_record_item, "TPC", iTPC);
            if (iTPC == 15 || iTPC == 31) {
                (void)_map_result_field_to_name(
                    result_record_item, "TPC",
                    LtePuschPowerControl_Record_v5_TPC,
                    ARRAY_SIZE(LtePuschPowerControl_Record_v5_TPC, ValueName),
                    "(MI)Unknown");
            }
            _replace_result_int(result_record_item, "PUSCH Actual Tx Power",
                                iActualPower);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    case 5: {
        offset += _decode_by_fmt(LtePuschPowerControl_Fmt_v5,
                                 ARRAY_SIZE(LtePuschPowerControl_Fmt_v5, Fmt),
                                 b, offset, length, result);
        int num_record = _search_result_int(result, "Number of Records");
        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePuschPowerControl_Record_Fmt_v5,
                ARRAY_SIZE(LtePuschPowerControl_Record_Fmt_v5, Fmt), b, offset,
                length, result_record_item);
            unsigned int iNonDecodeSFN =
                _search_result_uint(result_record_item, "SFN");
            int iSFN = iNonDecodeSFN & 1023;          // last 10 bits
            int iSubFN = (iNonDecodeSFN >> 10) & 15;  // next 4 bits
            int iPower = (iNonDecodeSFN >> 14) & 255; // next 8 bits
            int iDCI = (iNonDecodeSFN >> 22) & 15;    // next 4 bits
            int iTxType = (iNonDecodeSFN >> 26) & 3;  // next 3 bits
            _replace_result_int(result_record_item, "SFN", iSFN);
            _replace_result_int(result_record_item, "Sub-FN", iSubFN);
            _replace_result_int(result_record_item, "PUSCH Tx Power (dBm)",
                                iPower);
            _replace_result_int(result_record_item, "DCI Format", iDCI);
            (void)_map_result_field_to_name(
                result_record_item, "DCI Format",
                LtePuschPowerControl_Record_v5_DCI_Format,
                ARRAY_SIZE(LtePuschPowerControl_Record_v5_DCI_Format,
                           ValueName),
                "(MI)Unknown");
            _replace_result_int(result_record_item, "Tx Type", iTxType);
            (void)_map_result_field_to_name(
                result_record_item, "Tx Type",
                LtePuschPowerControl_Record_v5_TxType,
                ARRAY_SIZE(LtePuschPowerControl_Record_v5_TxType, ValueName),
                "(MI)Unknown");

            unsigned int iNonDecodeNRB =
                _search_result_uint(result_record_item, "Num RBs");
            int iNRB = iNonDecodeNRB & 255;          // last 8 bits
            int iTBS = (iNonDecodeNRB >> 8) & 16383; // next 14 bits
            int iLoss = (iNonDecodeNRB >> 22) & 255; // next 8 bits
            _replace_result_int(result_record_item, "Num RBs", iNRB);
            _replace_result_int(result_record_item, "Transport Block Size",
                                iTBS);
            _replace_result_int(result_record_item, "DL Path Loss", iLoss);

            unsigned int iNonDecodeFi =
                _search_result_uint(result_record_item, "F(i)");
            int iFi = iNonDecodeFi & 1023; // last 10 bits
            if (iFi >= 512) {
                iFi -= 1024;
            }
            int iTPC = (iNonDecodeFi >> 10) & 31; // next 5 bits
            int iActualPower = (iNonDecodeFi >> 15) & 255;
            _replace_result_int(result_record_item, "F(i)", iFi);
            _replace_result_int(result_record_item, "TPC", iTPC);
            if (iTPC == 15 || iTPC == 31) {
                (void)_map_result_field_to_name(
                    result_record_item, "TPC",
                    LtePuschPowerControl_Record_v5_TPC,
                    ARRAY_SIZE(LtePuschPowerControl_Record_v5_TPC, ValueName),
                    "(MI)Unknown");
            }
            _replace_result_int(result_record_item, "PUSCH Actual Tx Power",
                                iActualPower);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    case 4: {
        offset += _decode_by_fmt(LtePuschPowerControl_Fmt_v4,
                                 ARRAY_SIZE(LtePuschPowerControl_Fmt_v4, Fmt),
                                 b, offset, length, result);
        int num_record = _search_result_int(result, "Number of Records");
        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePuschPowerControl_Record_Fmt_v4,
                ARRAY_SIZE(LtePuschPowerControl_Record_Fmt_v4, Fmt), b, offset,
                length, result_record_item);
            unsigned int iNonDecodeSFN =
                _search_result_uint(result_record_item, "SFN");
            int iSFN = iNonDecodeSFN & 1023;          // last 10 bits
            int iSubFN = (iNonDecodeSFN >> 10) & 15;  // next 4 bits
            int iPower = (iNonDecodeSFN >> 14) & 255; // next 8 bits
            int iDCI = (iNonDecodeSFN >> 22) & 15;    // next 4 bits
            int iTxType = (iNonDecodeSFN >> 26) & 3;  // next 3 bits
            _replace_result_int(result_record_item, "SFN", iSFN);
            _replace_result_int(result_record_item, "Sub-FN", iSubFN);
            _replace_result_int(result_record_item, "PUSCH Tx Power (dBm)",
                                iPower);
            _replace_result_int(result_record_item, "DCI Format", iDCI);
            (void)_map_result_field_to_name(
                result_record_item, "DCI Format",
                LtePuschPowerControl_Record_v5_DCI_Format,
                ARRAY_SIZE(LtePuschPowerControl_Record_v5_DCI_Format,
                           ValueName),
                "(MI)Unknown");
            _replace_result_int(result_record_item, "Tx Type", iTxType);
            (void)_map_result_field_to_name(
                result_record_item, "Tx Type",
                LtePuschPowerControl_Record_v5_TxType,
                ARRAY_SIZE(LtePuschPowerControl_Record_v5_TxType, ValueName),
                "(MI)Unknown");

            unsigned int iNonDecodeTBS =
                _search_result_uint(result_record_item, "Transport Block Size");
            int iTBS = iNonDecodeTBS & 16383;        // last 14 bits
            int iLoss = (iNonDecodeTBS >> 14) & 255; // next 8 bits
            int iFi = (iNonDecodeTBS >> 22) & 1023;  // next 10 bits
            if (iFi >= 512) {
                iFi -= 1024;
            }
            _replace_result_int(result_record_item, "Transport Block Size",
                                iTBS);
            _replace_result_int(result_record_item, "DL Path Loss", iLoss);
            _replace_result_int(result_record_item, "F(i)", iFi);

            unsigned int iNonDecodeTPC =
                _search_result_uint(result_record_item, "TPC");
            int iTPC = (iNonDecodeTPC)&31;                 // last 5 bits
            int iActualPower = (iNonDecodeTPC >> 5) & 255; // next 8 bits
            _replace_result_int(result_record_item, "TPC", iTPC);
            if (iTPC == 15 || iTPC == 31) {
                (void)_map_result_field_to_name(
                    result_record_item, "TPC",
                    LtePuschPowerControl_Record_v5_TPC,
                    ARRAY_SIZE(LtePuschPowerControl_Record_v5_TPC, ValueName),
                    "(MI)Unknown");
            }
            _replace_result_int(result_record_item, "PUSCH Actual Tx Power",
                                iActualPower);

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    default:
        printf("(MI)Unknown LTE PUSCH Power Control version: 0x%x\n", pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------
static int _decode_lte_pdcch_phich_indication_report_payload(const char *b,
                                                             int offset,
                                                             size_t length,
                                                             json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    switch (pkt_ver) {
    case 5: {
        offset += _decode_by_fmt(
            LtePdcchPhichIndicationReport_Fmt_v5,
            ARRAY_SIZE(LtePdcchPhichIndicationReport_Fmt_v5, Fmt), b, offset,
            length, result);
        int iNonDecodeDuplexMode = _search_result_int(result, "Duplex Mode");
        int iDuplexMode = iNonDecodeDuplexMode & 3; // last 2 bits
        _replace_result_int(result, "Duplex Mode", iDuplexMode);
        int num_record = _search_result_int(result, "Number of Records");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePdcchPhichIndicationReport_Record_v5_p1,
                ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_p1, Fmt), b,
                offset, length, result_record_item);
            unsigned int iNonDecodeP1_1 =
                _search_result_uint(result_record_item, "Num PDCCH Results");
            int iNumPdcch = iNonDecodeP1_1 & 7;               // last 3 bits
            int iPdcchSFN = (iNonDecodeP1_1 >> 3) & 1023;     // next 10 bits
            int iPdcchSubFN = (iNonDecodeP1_1 >> 13) & 15;    // next 4 bits
            int iPhichIncluded = (iNonDecodeP1_1 >> 17) & 1;  // next 1 bit
            int iPhich1Included = (iNonDecodeP1_1 >> 18) & 1; // next 1 bit
            int iPhichSFN = (iNonDecodeP1_1 >> 19) & 1023;    // next 10 bits
            // reserved 3 bits
            unsigned int iNonDecodeP1_2 =
                _search_result_uint(result_record_item, "PHICH Timing Sub-FN");
            int iPhichSubFN = iNonDecodeP1_2 & 15;        // next 4 bits
            int iPhichValue = (iNonDecodeP1_2 >> 4) & 1;  // next 1 bit
            int iPhich1Value = (iNonDecodeP1_2 >> 5) & 1; // next 1 bit
            _replace_result_int(result_record_item, "Num PDCCH Results",
                                iNumPdcch);
            _replace_result_int(result_record_item, "PDCCH Timing SFN",
                                iPdcchSFN);
            _replace_result_int(result_record_item, "PDCCH Timing Sub-FN",
                                iPdcchSubFN);
            _replace_result_int(result_record_item, "PHICH Included",
                                iPhichIncluded);
            (void)_map_result_field_to_name(
                result_record_item, "PHICH Included",
                LtePdcchPhichIndicationReport_Record_v5_Included,
                ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_Included,
                           ValueName),
                "(MI)Unknown");
            _replace_result_int(result_record_item, "PHICH 1 Included",
                                iPhich1Included);
            (void)_map_result_field_to_name(
                result_record_item, "PHICH 1 Included",
                LtePdcchPhichIndicationReport_Record_v5_Included,
                ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_Included,
                           ValueName),
                "(MI)Unknown");
            if (iPhichIncluded == 1) {
                _replace_result_int(result_record_item, "PHICH Timing SFN",
                                    iPhichSFN);
                _replace_result_int(result_record_item, "PHICH Timing Sub-FN",
                                    iPhichSubFN);
                _replace_result_int(result_record_item, "PHICH Value",
                                    iPhichValue);
                (void)_map_result_field_to_name(
                    result_record_item, "PHICH Value",
                    LtePdcchPhichIndicationReport_Record_v5_Value,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_Value,
                               ValueName),
                    "(MI)Unknown");
            }
            if (iPhich1Included == 1) {
                _replace_result_int(result_record_item, "PHICH 1 Value",
                                    iPhich1Value);
                (void)_map_result_field_to_name(
                    result_record_item, "PHICH 1 Value",
                    LtePdcchPhichIndicationReport_Record_v5_Value,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_Value,
                               ValueName),
                    "(MI)Unknown");
            }

            json result_pdcch;
            for (int j = 0; j < iNumPdcch; j++) {
                json result_pdcch_item;
                offset += _decode_by_fmt(
                    LtePdcchPhichIndicationReport_Record_v5_p2,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_p2, Fmt),
                    b, offset, length, result_pdcch_item);
                int iNonDecodeP2_1 =
                    _search_result_int(result_pdcch_item, "Serv Cell Idx");
                int iServCellIdx = iNonDecodeP2_1 & 7;          // last 3 bits
                int iRNTI = (iNonDecodeP2_1 >> 3) & 15;         // next 4 bits
                int iPayloadSize = (iNonDecodeP2_1 >> 7) & 127; // next 7 bits
                int iAggLv = (iNonDecodeP2_1 >> 14) & 3;        // next 2 bits
                _replace_result_int(result_pdcch_item, "Serv Cell Idx",
                                    iServCellIdx);
                _replace_result_int(result_pdcch_item, "RNTI Type", iRNTI);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "RNTI Type", RNTIType,
                    ARRAY_SIZE(RNTIType, ValueName), "(MI)Unknown");
                _replace_result_int(result_pdcch_item, "Payload Size",
                                    iPayloadSize);
                _replace_result_int(result_pdcch_item, "Aggregation Level",
                                    iAggLv);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "Aggregation Level",
                    LtePdcchPhichIndicationReport_Record_v5_AggLv,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_AggLv,
                               ValueName),
                    "(MI)Unknown");

                int iNonDecodeP2_2 =
                    _search_result_int(result_pdcch_item, "Search Space");
                int iSearchSpace = iNonDecodeP2_2 & 1; // last 1 bit
                // SPS Grant Type should take this 3 bits
                int iNewDLTx = (iNonDecodeP2_2 >> 4) & 1;     // next 1 bit
                int iNumDLTrblks = (iNonDecodeP2_2 >> 5) & 3; // next 2 bits
                _replace_result_int(result_pdcch_item, "Search Space",
                                    iSearchSpace);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "Search Space",
                    LtePdcchPhichIndicationReport_Record_v5_SS,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_SS,
                               ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_pdcch_item, "New DL Tx", iNewDLTx);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "New DL Tx",
                    LtePdcchPhichIndicationReport_Record_v5_NewDLTx,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_NewDLTx,
                               ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_pdcch_item, "Num DL Trblks",
                                    iNumDLTrblks);

                result_pdcch.push_back(result_pdcch_item);
            }
            result_record_item["PDCCH Info"] = result_pdcch;

            for (int k = 0; k < 8 - iNumPdcch; k++) {
                offset += _decode_by_fmt(
                    LtePdcchPhichIndicationReport_Record_v5_p3,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v5_p3, Fmt),
                    b, offset, length, result_record_item);
            }

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    case 25: {
        offset += _decode_by_fmt(
            LtePdcchPhichIndicationReport_Fmt_v25,
            ARRAY_SIZE(LtePdcchPhichIndicationReport_Fmt_v25, Fmt), b, offset,
            length, result);
        int iNonDecodeDuplexMode = _search_result_int(result, "Duplex Mode");
        int iDuplexMode = iNonDecodeDuplexMode & 3; // last 2 bits
        _replace_result_int(result, "Duplex Mode", iDuplexMode);
        int num_record = _search_result_int(result, "Number of Records");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePdcchPhichIndicationReport_Record_v25_p1,
                ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v25_p1, Fmt), b,
                offset, length, result_record_item);
            unsigned int iNonDecodeP1_1 =
                _search_result_uint(result_record_item, "Num PDCCH Results");
            int iNumPdcch = iNonDecodeP1_1 & 7;            // last 3 bits
            int iNumPhich = (iNonDecodeP1_1 >> 3) & 7;     // next 3 bits
            int iPdcchSFN = (iNonDecodeP1_1 >> 6) & 1023;  // next 10 bits
            int iPdcchSubFN = (iNonDecodeP1_1 >> 16) & 15; // next 4 bits
            _replace_result_int(result_record_item, "Num PDCCH Results",
                                iNumPdcch);
            _replace_result_int(result_record_item, "Num PHICH Results",
                                iNumPhich);
            _replace_result_int(result_record_item, "PDCCH Timing SFN",
                                iPdcchSFN);
            _replace_result_int(result_record_item, "PDCCH Timing Sub-FN",
                                iPdcchSubFN);

            json result_phich;
            for (int j = 0; j < iNumPhich; j++) {
                json result_phich_item;
                offset += _decode_by_fmt(
                    LtePdcchPhichIndicationReport_Record_v25_phich,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v25_phich,
                               Fmt),
                    b, offset, length, result_phich_item);
                unsigned int iNonDecode_phich =
                    _search_result_uint(result_phich_item, "Cell Index");
                int iCellIndex = iNonDecode_phich & 7;             // 3 bits
                int iPhichIncluded = (iNonDecode_phich >> 3) & 1;  // next 1 bit
                int iPhich1Included = (iNonDecode_phich >> 4) & 1; // next 1 bit
                int iPhichValue = (iNonDecode_phich >> 5) & 1;     // next 1 bit
                int iPhich1Value = (iNonDecode_phich >> 6) & 1;    // next 1 bit

                _replace_result_int(result_phich_item, "Cell Index",
                                    iCellIndex);
                _replace_result_int(result_phich_item, "PHICH Included",
                                    iPhichIncluded);
                (void)_map_result_field_to_name(
                    result_phich_item, "PHICH Included", ValueNameYesOrNo,
                    ARRAY_SIZE(ValueNameYesOrNo, ValueName), "(MI)Unknown");
                _replace_result_int(result_phich_item, "PHICH 1 Included",
                                    iPhich1Included);
                (void)_map_result_field_to_name(
                    result_phich_item, "PHICH 1 Included", ValueNameYesOrNo,
                    ARRAY_SIZE(ValueNameYesOrNo, ValueName), "(MI)Unknown");
                _replace_result_int(result_phich_item, "PHICH Value",
                                    iPhichValue);
                (void)_map_result_field_to_name(
                    result_phich_item, "PHICH Value", ValueNameAckOrNack,
                    ARRAY_SIZE(ValueNameAckOrNack, ValueName), "(MI)Unknown");
                _replace_result_int(result_phich_item, "PHICH 1 Value",
                                    iPhich1Value);
                (void)_map_result_field_to_name(
                    result_phich_item, "PHICH 1 Value", ValueNameAckOrNack,
                    ARRAY_SIZE(ValueNameAckOrNack, ValueName), "(MI)Unknown");

                result_phich.push_back(result_phich_item);
            }
            result_record_item["PHICH"] = result_phich;

            // totally 12 bytes for all phich
            offset += 12 - iNumPhich * 4;

            json result_pdcch;
            for (int j = 0; j < iNumPdcch; j++) {
                json result_pdcch_item;
                offset += _decode_by_fmt(
                    LtePdcchPhichIndicationReport_Record_v25_pdcch,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v25_pdcch,
                               Fmt),
                    b, offset, length, result_pdcch_item);
                int iNonDecodePdcch_1 =
                    _search_result_int(result_pdcch_item, "Serv Cell Idx");
                int iServCellIdx = iNonDecodePdcch_1 & 7;  // last 3 bits
                int iRNTI = (iNonDecodePdcch_1 >> 3) & 15; // next 4 bits
                int iPayloadSize =
                    (iNonDecodePdcch_1 >> 7) & 127;         // next 7 bits
                int iAggLv = (iNonDecodePdcch_1 >> 14) & 3; // next 2 bits
                _replace_result_int(result_pdcch_item, "Serv Cell Idx",
                                    iServCellIdx);
                _replace_result_int(result_pdcch_item, "RNTI Type", iRNTI);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "RNTI Type", ValueNameRNTIType,
                    ARRAY_SIZE(ValueNameRNTIType, ValueName), "(MI)Unknown");
                _replace_result_int(result_pdcch_item, "Payload Size",
                                    iPayloadSize);
                _replace_result_int(result_pdcch_item, "Aggregation Level",
                                    iAggLv);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "Aggregation Level",
                    ValueNameAggregationLevel,
                    ARRAY_SIZE(ValueNameAggregationLevel, ValueName),
                    "(MI)Unknown");

                int iNonDecodePdcch_2 =
                    _search_result_int(result_pdcch_item, "Search Space");
                int iSearchSpace = iNonDecodePdcch_2 & 1; // last 1 bit
                // SPS Grant Type should take this 3 bits
                int iNewDLTx = (iNonDecodePdcch_2 >> 4) & 1;     // next 1 bit
                int iNumDLTrblks = (iNonDecodePdcch_2 >> 5) & 3; // next 2 bits
                _replace_result_int(result_pdcch_item, "Search Space",
                                    iSearchSpace);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "Search Space", ValueNameSearchSpaceType,
                    ARRAY_SIZE(ValueNameSearchSpaceType, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_pdcch_item, "New DL Tx", iNewDLTx);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "New DL Tx", ValueNameTrueOrFalse,
                    ARRAY_SIZE(ValueNameTrueOrFalse, ValueName), "(MI)Unknown");
                _replace_result_int(result_pdcch_item, "Num DL Trblks",
                                    iNumDLTrblks);

                result_pdcch.push_back(result_pdcch_item);
            }
            result_record_item["PDCCH Info"] = result_pdcch;

            // at most 64 bytes for all pdcch
            offset += 64 - iNumPdcch * 8;

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }
    case 33: {
        offset += _decode_by_fmt(
            LtePdcchPhichIndicationReport_Fmt_v33,
            ARRAY_SIZE(LtePdcchPhichIndicationReport_Fmt_v33, Fmt), b, offset,
            length, result);
        int iNonDecodeDuplexMode = _search_result_int(result, "Duplex Mode");
        int iDuplexMode = iNonDecodeDuplexMode & 3; // last 2 bits
        _replace_result_int(result, "Duplex Mode", iDuplexMode);
        int num_record = _search_result_int(result, "Number of Records");

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(
                LtePdcchPhichIndicationReport_Record_v33_p1,
                ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v33_p1, Fmt), b,
                offset, length, result_record_item);
            unsigned int iNonDecodeP1_1 =
                _search_result_uint(result_record_item, "Num PDCCH Results");
            int iNumPdcch = iNonDecodeP1_1 & 7;            // last 3 bits
            int iNumPhich = (iNonDecodeP1_1 >> 3) & 7;     // next 3 bits
            int iPdcchSFN = (iNonDecodeP1_1 >> 6) & 1023;  // next 10 bits
            int iPdcchSubFN = (iNonDecodeP1_1 >> 16) & 15; // next 4 bits
            _replace_result_int(result_record_item, "Num PDCCH Results",
                                iNumPdcch);
            _replace_result_int(result_record_item, "Num PHICH Results",
                                iNumPhich);
            _replace_result_int(result_record_item, "PDCCH Timing SFN",
                                iPdcchSFN);
            _replace_result_int(result_record_item, "PDCCH Timing Sub-FN",
                                iPdcchSubFN);

            json result_phich;
            for (int j = 0; j < iNumPhich; j++) {
                json result_phich_item;
                offset += _decode_by_fmt(
                    LtePdcchPhichIndicationReport_Record_v25_phich,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v25_phich,
                               Fmt),
                    b, offset, length, result_phich_item);
                unsigned int iNonDecode_phich =
                    _search_result_uint(result_phich_item, "Cell Index");
                int iCellIndex = iNonDecode_phich & 7;             // 3 bits
                int iPhichIncluded = (iNonDecode_phich >> 3) & 1;  // next 1 bit
                int iPhich1Included = (iNonDecode_phich >> 4) & 1; // next 1 bit
                int iPhichValue = (iNonDecode_phich >> 5) & 1;     // next 1 bit
                int iPhich1Value = (iNonDecode_phich >> 6) & 1;    // next 1 bit

                _replace_result_int(result_phich_item, "Cell Index",
                                    iCellIndex);
                _replace_result_int(result_phich_item, "PHICH Included",
                                    iPhichIncluded);
                (void)_map_result_field_to_name(
                    result_phich_item, "PHICH Included", ValueNameYesOrNo,
                    ARRAY_SIZE(ValueNameYesOrNo, ValueName), "(MI)Unknown");
                _replace_result_int(result_phich_item, "PHICH 1 Included",
                                    iPhich1Included);
                (void)_map_result_field_to_name(
                    result_phich_item, "PHICH 1 Included", ValueNameYesOrNo,
                    ARRAY_SIZE(ValueNameYesOrNo, ValueName), "(MI)Unknown");
                _replace_result_int(result_phich_item, "PHICH Value",
                                    iPhichValue);
                (void)_map_result_field_to_name(
                    result_phich_item, "PHICH Value", ValueNameAckOrNack,
                    ARRAY_SIZE(ValueNameAckOrNack, ValueName), "(MI)Unknown");
                _replace_result_int(result_phich_item, "PHICH 1 Value",
                                    iPhich1Value);
                (void)_map_result_field_to_name(
                    result_phich_item, "PHICH 1 Value", ValueNameAckOrNack,
                    ARRAY_SIZE(ValueNameAckOrNack, ValueName), "(MI)Unknown");

                result_phich.push_back(result_phich_item);
            }
            result_record_item["PHICH"] = result_phich;

            // totally 20 bytes for all phich
            offset += 20 - iNumPhich * 4;

            json result_pdcch;
            for (int j = 0; j < iNumPdcch; j++) {
                json result_pdcch_item;
                offset += _decode_by_fmt(
                    LtePdcchPhichIndicationReport_Record_v33_pdcch,
                    ARRAY_SIZE(LtePdcchPhichIndicationReport_Record_v33_pdcch,
                               Fmt),
                    b, offset, length, result_pdcch_item);
                int iNonDecodePdcch_1 =
                    _search_result_int(result_pdcch_item, "Serv Cell Idx");
                int iServCellIdx = iNonDecodePdcch_1 & 7;  // last 3 bits
                int iRNTI = (iNonDecodePdcch_1 >> 3) & 15; // next 4 bits
                int iPayloadSize =
                    (iNonDecodePdcch_1 >> 7) & 127;         // next 7 bits
                int iAggLv = (iNonDecodePdcch_1 >> 14) & 3; // next 2 bits
                _replace_result_int(result_pdcch_item, "Serv Cell Idx",
                                    iServCellIdx);
                _replace_result_int(result_pdcch_item, "RNTI Type", iRNTI);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "RNTI Type", ValueNameRNTIType,
                    ARRAY_SIZE(ValueNameRNTIType, ValueName), "(MI)Unknown");
                _replace_result_int(result_pdcch_item, "Payload Size",
                                    iPayloadSize);
                _replace_result_int(result_pdcch_item, "Aggregation Level",
                                    iAggLv);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "Aggregation Level",
                    ValueNameAggregationLevel,
                    ARRAY_SIZE(ValueNameAggregationLevel, ValueName),
                    "(MI)Unknown");

                int iNonDecodePdcch_2 =
                    _search_result_int(result_pdcch_item, "Search Space");
                int iSearchSpace = iNonDecodePdcch_2 & 1; // last 1 bit
                // SPS Grant Type should take this 3 bits
                int iNewDLTx = (iNonDecodePdcch_2 >> 4) & 1;     // next 1 bit
                int iNumDLTrblks = (iNonDecodePdcch_2 >> 5) & 3; // next 2 bits
                _replace_result_int(result_pdcch_item, "Search Space",
                                    iSearchSpace);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "Search Space", ValueNameSearchSpaceType,
                    ARRAY_SIZE(ValueNameSearchSpaceType, ValueName),
                    "(MI)Unknown");
                _replace_result_int(result_pdcch_item, "New DL Tx", iNewDLTx);
                (void)_map_result_field_to_name(
                    result_pdcch_item, "New DL Tx", ValueNameTrueOrFalse,
                    ARRAY_SIZE(ValueNameTrueOrFalse, ValueName), "(MI)Unknown");
                _replace_result_int(result_pdcch_item, "Num DL Trblks",
                                    iNumDLTrblks);

                result_pdcch.push_back(result_pdcch_item);
            }
            result_record_item["PDCCH Info"] = result_pdcch;

            // at most 64 bytes for all pdcch
            offset += 64 - iNumPdcch * 8;

            result_record.push_back(result_record_item);
        }
        result["Records"] = result_record;
        return offset - start;
    }

    default:
        printf("(MI)Unknown LTE PDCCH-PHICH Indication Report version: 0x%x\n",
               pkt_ver);
        return 0;
    }
}

// ----------------------------------------------------------------------------

// Yuanjie: decode modem's internal debugging message

static size_t _decode_modem_debug_msg(const char *b, int offset, size_t length,
                                      json &result) {

    int start = offset;
    int argc = _search_result_int(result, "Number of parameters");
    char version = _search_result_int(result, "Version");

    if (version == '\x79') {
        // Yuanjie: optimization for iCellular. Don't show unnecessary logs
        return length - start;
    } else if (version == '\x92') {

        // Yuanjie: these logs are encoded in unknown approach (signature
        // based?) Currently only ad-hoc approach is available

        // Yuanjie: for this type of debugging message, a "fingerprint" exists
        // before the parameters The fingerprint seems the **unique** identifier
        // of one message. The message is pre-stored (thus not transferred) as a
        // database.
        //
        argc++;
        long *tmp_argv = new long[argc];

        // printf("%d\n",argc);

        // Get parameters
        for (int i = 0; i != argc; i++) {
            // const char *p = b + offset;

            // int ii = *((int *) p);    //a new parameter

            long ii = *((long *)(b + offset));

            tmp_argv[i] = ii;

            offset += 4; // one parameter
        }

        // if(argc>=2)
        //     printf("argc=%d argv[0]=%x argv[1]=%d\n", argc, tmp_argv[0],
        //     tmp_argv[1]);

        if (argc == 2 && tmp_argv[0] == 0x2e3bbbed) {

            // Yuanjie: for icellular only, 4G RSRP result in manual network
            // search get log "BPLMN LOG: Saved measurement results. rsrp=-121"
            // The fingerprint is tmp_argv[0]==0xedbb3b2e, tmp_argv[1]=rsrp
            // value
            //
            std::string res = "BPLMN LOG: Saved measurement results. rsrp=";
            // res+=std::to_string(tmp_argv[1]);
            res += patch::to_string(tmp_argv[1]);
            result["Msg"] =
                std::string("") + make_string(res.c_str(), res.size());

            delete[] tmp_argv;
            return length - start;
        } else if (argc == 10 && tmp_argv[0] == 0x81700a47) {

            // Yuanjie: for icellular only, 3G RSRP result in manual network
            // search The fingerprint is tmp_argv[0]==0x81700a47,
            // tmp_argv[1]=rscp value
            //
            std::string res = "Freq=%d psc=%d eng=%d filt_eng=%d rscp=%d "
                              "RxAGC=%d 2*ecio=%d 2*squal=%d srxlv=%d";
            for (int i = 1; i != argc; i++) {

                std::size_t found = res.find("%d");
                // std::string tmp = std::to_string(tmp_argv[i]);
                std::string tmp = patch::to_string(tmp_argv[i]);
                res.replace(found, 2, tmp);
            }
            result["Msg"] =
                std::string("") + make_string(res.c_str(), res.size());

            delete[] tmp_argv;
            return length - start;

        } else {

            // Ignore other unknown messages. Don't send to MobileInsight
            // Analyzers std::string res="(Unknown debug message)"; PyObject *t
            // = Py_BuildValue("(ss#s)", "Msg", res.c_str(), res.size(), "");
            // PyList_Append(result, t);
            //

            delete[] tmp_argv;
            return length - start;
        }
    }
    return length - start;
}

bool is_log_packet(const char *b, size_t length) {
    return length >= 2 && b[0] == '\x10';
}

bool is_debug_packet(const char *b, size_t length) {
    return length >= 2 && (b[0] == '\x79' || b[0] == '\x92');
    // return length >=2 && (b[0] == '\x92');  //Yuanjie: optimization for
    // iCellular, avoid unuseful debug msg return length >=2 && (b[0] ==
    // '\x79');
}

void on_demand_decode(const char *b, size_t length, LogPacketType type_id,
                      json &result) {
    int offset = 0;
    switch (type_id) {
    case CDMA_Paging_Channel_Message:
        // Not fully support.
        offset += _decode_by_fmt(CdmaPagingChannelMsg_Fmt,
                                 ARRAY_SIZE(CdmaPagingChannelMsg_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_cdma_paging_channel_msg(b, offset, length, result);
        break;

    // Yuanjie: Incomplete support. Disable it temporarily
    case _1xEV_Signaling_Control_Channel_Broadcast:
        offset += _decode_by_fmt(_1xEVSignalingFmt,
                                 ARRAY_SIZE(_1xEVSignalingFmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_1xev_signaling_control_channel_broadcast(
            b, offset, length, result);
        break;

    case WCDMA_CELL_ID:
        offset +=
            _decode_by_fmt(WcdmaCellIdFmt, ARRAY_SIZE(WcdmaCellIdFmt, Fmt), b,
                           offset, length, result);
        break;

    case WCDMA_Signaling_Messages:
        offset += _decode_by_fmt(WcdmaSignalingMessagesFmt,
                                 ARRAY_SIZE(WcdmaSignalingMessagesFmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_wcdma_signaling_messages(b, offset, length, result);
        break;

    case UMTS_NAS_GMM_State:
        offset += _decode_by_fmt(UmtsNasGmmStateFmt,
                                 ARRAY_SIZE(UmtsNasGmmStateFmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_umts_nas_gmm_state(b, offset, length, result);
        break;

    case UMTS_NAS_MM_State:
        offset += _decode_by_fmt(UmtsNasMmStateFmt,
                                 ARRAY_SIZE(UmtsNasMmStateFmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_umts_nas_mm_state(b, offset, length, result);
        break;

    case UMTS_NAS_MM_REG_State:
        offset += _decode_by_fmt(UmtsNasMmRegStateFmt,
                                 ARRAY_SIZE(UmtsNasMmRegStateFmt, Fmt), b,
                                 offset, length, result);
        break;

    case UMTS_NAS_OTA:
        offset += _decode_by_fmt(UmtsNasOtaFmt, ARRAY_SIZE(UmtsNasOtaFmt, Fmt),
                                 b, offset, length, result);
        offset += _decode_umts_nas_ota(b, offset, length, result);
        break;

    case LTE_RRC_OTA_Packet:
        offset += _decode_by_fmt(LteRrcOtaPacketFmt,
                                 ARRAY_SIZE(LteRrcOtaPacketFmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_lte_rrc_ota(b, offset, length, result);
        break;

    case LTE_RRC_MIB_Message_Log_Packet:
        offset += _decode_by_fmt(LteRrcMibMessageLogPacketFmt,
                                 ARRAY_SIZE(LteRrcMibMessageLogPacketFmt, Fmt),
                                 b, offset, length, result);
        offset += _decode_lte_rrc_mib(b, offset, length, result);
        break;

    case LTE_RRC_Serv_Cell_Info_Log_Packet:
        offset +=
            _decode_by_fmt(LteRrcServCellInfoLogPacketFmt,
                           ARRAY_SIZE(LteRrcServCellInfoLogPacketFmt, Fmt), b,
                           offset, length, result);
        offset += _decode_lte_rrc_serv_cell_info(b, offset, length, result);
        break;

    case LTE_NAS_ESM_Plain_OTA_Incoming_Message:
    case LTE_NAS_ESM_Plain_OTA_Outgoing_Message:
    case LTE_NAS_EMM_Plain_OTA_Incoming_Message:
    case LTE_NAS_EMM_Plain_OTA_Outgoing_Message:
        offset +=
            _decode_by_fmt(LteNasPlainFmt, ARRAY_SIZE(LteNasPlainFmt, Fmt), b,
                           offset, length, result);
        offset += _decode_lte_nas_plain(b, offset, length, result);
        break;

    case LTE_NAS_EMM_State:
        offset += _decode_by_fmt(LteNasEmmStateFmt,
                                 ARRAY_SIZE(LteNasEmmStateFmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_lte_nas_emm_state(b, offset, length, result);
        break;

    case LTE_NAS_ESM_State:
        offset += _decode_by_fmt(LteNasEsmStateFmt,
                                 ARRAY_SIZE(LteNasEsmStateFmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_lte_nas_esm_state(b, offset, length, result);
        break;

    case LTE_PHY_PDSCH_Demapper_Configuration:
        offset += _decode_by_fmt(LtePhyPdschDemapperConfigFmt,
                                 ARRAY_SIZE(LtePhyPdschDemapperConfigFmt, Fmt),
                                 b, offset, length, result);
        offset +=
            _decode_lte_phy_pdsch_demapper_config(b, offset, length, result);
        break;

    case LTE_PHY_Connected_Mode_LTE_Intra_Freq_Meas_Results:
        offset +=
            _decode_by_fmt(LtePhyCmlifmrFmt, ARRAY_SIZE(LtePhyCmlifmrFmt, Fmt),
                           b, offset, length, result);
        offset += _decode_lte_phy_cmlifmr(b, offset, length, result);
        break;

    case LTE_PHY_Serving_Cell_Measurement_Result:
        offset +=
            _decode_by_fmt(LtePhySubpktFmt, ARRAY_SIZE(LtePhySubpktFmt, Fmt), b,
                           offset, length, result);
        offset += _decode_lte_phy_subpkt(b, offset, length, result);
        break;

    case LTE_PHY_IRAT_MDB:
        offset += _decode_by_fmt(LtePhyIratFmt, ARRAY_SIZE(LtePhyIratFmt, Fmt),
                                 b, offset, length, result);
        offset += _decode_lte_phy_irat_subpkt(b, offset, length, result);
        break;

    case LTE_PHY_CDMA_MEAS:
        // It shares similar packet format as LTE_PHY_IRAT_MDB
        offset += _decode_by_fmt(LtePhyIratFmt, ARRAY_SIZE(LtePhyIratFmt, Fmt),
                                 b, offset, length, result);
        offset += _decode_lte_phy_irat_cdma_subpkt(b, offset, length, result);
        break;

    case LTE_PDCP_DL_SRB_Integrity_Data_PDU:
        offset +=
            _decode_by_fmt(LtePdcpDlSrbIntegrityDataPduFmt,
                           ARRAY_SIZE(LtePdcpDlSrbIntegrityDataPduFmt, Fmt), b,
                           offset, length, result);
        offset += _decode_lte_pdcp_dl_srb_integrity_data_pdu(b, offset, length,
                                                             result);
        break;

    case LTE_PDCP_UL_SRB_Integrity_Data_PDU:
        offset +=
            _decode_by_fmt(LtePdcpUlSrbIntegrityDataPduFmt,
                           ARRAY_SIZE(LtePdcpUlSrbIntegrityDataPduFmt, Fmt), b,
                           offset, length, result);
        offset += _decode_lte_pdcp_ul_srb_integrity_data_pdu(b, offset, length,
                                                             result);
        break;

    case LTE_MAC_Configuration: // Jie
        offset += _decode_by_fmt(LteMacConfigurationFmt,
                                 ARRAY_SIZE(LteMacConfigurationFmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_mac_configuration_subpkt(b, offset, length, result);
        break;

    case LTE_MAC_UL_Transport_Block: // Jie
        offset += _decode_by_fmt(LteMacULTransportBlockFmt,
                                 ARRAY_SIZE(LteMacULTransportBlockFmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_mac_ul_transportblock_subpkt(b, offset, length, result);
        break;

    case LTE_MAC_DL_Transport_Block: // Jie
        offset += _decode_by_fmt(LteMacDLTransportBlockFmt,
                                 ARRAY_SIZE(LteMacDLTransportBlockFmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_mac_dl_transportblock_subpkt(b, offset, length, result);
        break;

    case LTE_MAC_UL_Buffer_Status_Internal: // Jie
        offset +=
            _decode_by_fmt(LteMacULBufferStatusInternalFmt,
                           ARRAY_SIZE(LteMacULBufferStatusInternalFmt, Fmt), b,
                           offset, length, result);
        offset += _decode_lte_mac_ul_bufferstatusinternal_subpkt(
            b, offset, length, result);
        break;
    case LTE_MAC_UL_Tx_Statistics: // Jie
        offset += _decode_by_fmt(LteMacULTxStatisticsFmt,
                                 ARRAY_SIZE(LteMacULTxStatisticsFmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_mac_ul_txstatistics_subpkt(b, offset, length, result);
        break;

    case LTE_RLC_UL_Config_Log_Packet:
        offset += _decode_by_fmt(LteRlcUlConfigLogPacketFmt,
                                 ARRAY_SIZE(LteRlcUlConfigLogPacketFmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_rlc_ul_config_log_packet_subpkt(b, offset, length,
                                                              result);
        break;

    case LTE_RLC_DL_Config_Log_Packet:
        offset += _decode_by_fmt(LteRlcDlConfigLogPacketFmt,
                                 ARRAY_SIZE(LteRlcDlConfigLogPacketFmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_rlc_dl_config_log_packet_subpkt(b, offset, length,
                                                              result);
        break;

    case LTE_RLC_UL_AM_All_PDU:
        offset += _decode_by_fmt(LteRlcUlAmAllPduFmt,
                                 ARRAY_SIZE(LteRlcUlAmAllPduFmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_rlc_ul_am_all_pdu_subpkt(b, offset, length, result);
        break;

    case LTE_RLC_DL_AM_All_PDU:
        offset += _decode_by_fmt(LteRlcDlAmAllPduFmt,
                                 ARRAY_SIZE(LteRlcDlAmAllPduFmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_rlc_dl_am_all_pdu_subpkt(b, offset, length, result);
        break;

    case LTE_MAC_Rach_Trigger:
        offset += _decode_by_fmt(LteMacRachTriggerFmt,
                                 ARRAY_SIZE(LteMacRachTriggerFmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_mac_rach_trigger_subpkt(b, offset, length, result);
        break;
    case LTE_MAC_Rach_Attempt:
        offset += _decode_by_fmt(LteMacRachAttempt_Fmt,
                                 ARRAY_SIZE(LteMacRachAttempt_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_mac_rach_attempt_subpkt(b, offset, length, result);
        break;
    case LTE_PDCP_DL_Config:
        offset += _decode_by_fmt(LtePdcpDlConfig_Fmt,
                                 ARRAY_SIZE(LtePdcpDlConfig_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_pdcp_dl_config_subpkt(b, offset, length, result);
        break;
    case LTE_PDCP_UL_Config:
        offset += _decode_by_fmt(LtePdcpUlConfig_Fmt,
                                 ARRAY_SIZE(LtePdcpUlConfig_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_pdcp_ul_config_subpkt(b, offset, length, result);
        break;
    case LTE_PDCP_UL_Data_PDU:
        offset += _decode_by_fmt(LtePdcpUlDataPdu_Fmt,
                                 ARRAY_SIZE(LtePdcpUlDataPdu_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_pdcp_ul_data_pdu_subpkt(b, offset, length, result);
        break;
    case LTE_PDCP_DL_Stats:
        offset += _decode_by_fmt(LtePdcpDlStats_Fmt,
                                 ARRAY_SIZE(LtePdcpDlStats_Fmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_lte_pdcp_dl_stats_subpkt(b, offset, length, result);
        break;
    case LTE_PDCP_UL_Stats:
        offset += _decode_by_fmt(LtePdcpUlStats_Fmt,
                                 ARRAY_SIZE(LtePdcpUlStats_Fmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_lte_pdcp_ul_stats_subpkt(b, offset, length, result);
        break;
    case LTE_RLC_UL_Stats:
        offset += _decode_by_fmt(LteRlcUlStats_Fmt,
                                 ARRAY_SIZE(LteRlcUlStats_Fmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_lte_rlc_ul_stats_subpkt(b, offset, length, result);
        break;
    case LTE_RLC_DL_Stats:
        offset += _decode_by_fmt(LteRlcDlStats_Fmt,
                                 ARRAY_SIZE(LteRlcDlStats_Fmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_lte_rlc_dl_stats_subpkt(b, offset, length, result);
        break;
    case LTE_PDCP_DL_Ctrl_PDU:
        offset += _decode_by_fmt(LtePdcpDlCtrlPdu_Fmt,
                                 ARRAY_SIZE(LtePdcpDlCtrlPdu_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_pdcp_dl_ctrl_pdu_subpkt(b, offset, length, result);
        break;
    case LTE_PDCP_UL_Ctrl_PDU:
        offset += _decode_by_fmt(LtePdcpDlCtrlPdu_Fmt,
                                 ARRAY_SIZE(LtePdcpDlCtrlPdu_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_pdcp_ul_ctrl_pdu_subpkt(b, offset, length, result);
        break;
    case LTE_PUCCH_Power_Control:
        offset += _decode_by_fmt(LtePucchPowerControl_Fmt,
                                 ARRAY_SIZE(LtePucchPowerControl_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_pucch_power_control_payload(b, offset, length, result);
        break;
    case LTE_PUSCH_Power_Control:
        offset += _decode_by_fmt(LtePuschPowerControl_Fmt,
                                 ARRAY_SIZE(LtePuschPowerControl_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_pusch_power_control_payload(b, offset, length, result);
        break;
    case LTE_PDCCH_PHICH_Indication_Report:
        offset +=
            _decode_by_fmt(LtePdcchPhichIndicationReport_Fmt,
                           ARRAY_SIZE(LtePdcchPhichIndicationReport_Fmt, Fmt),
                           b, offset, length, result);
        offset += _decode_lte_pdcch_phich_indication_report_payload(
            b, offset, length, result);
        break;
    case _1xEV_Rx_Partial_MultiRLP_Packet:
        offset +=
            _decode_by_fmt(_1xEVRxPartialMultiRLPPacket_Fmt,
                           ARRAY_SIZE(_1xEVRxPartialMultiRLPPacket_Fmt, Fmt), b,
                           offset, length, result);
        offset += _decode_1xev_rx_partial_multirlp_packet_payload(
            b, offset, length, result);
        break;
    case _1xEV_Connected_State_Search_Info:
        offset +=
            _decode_by_fmt(_1xEVConnectedStateSearchInfo_Fmt,
                           ARRAY_SIZE(_1xEVConnectedStateSearchInfo_Fmt, Fmt),
                           b, offset, length, result);
        offset += _decode_1xev_connected_state_search_info_payload(
            b, offset, length, result);
        break;
    case _1xEV_Connection_Attempt:
        offset += _decode_by_fmt(_1xEVConnectionAttempt_Fmt,
                                 ARRAY_SIZE(_1xEVConnectionAttempt_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_1xev_connection_attempt_payload(b, offset, length, result);
        break;
    case _1xEV_Connection_Release:
        offset += _decode_by_fmt(_1xEVConnectionRelease_Fmt,
                                 ARRAY_SIZE(_1xEVConnectionRelease_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_1xev_connection_release_payload(b, offset, length, result);
        break;
    case LTE_PDSCH_Stat_Indication:
        offset += _decode_by_fmt(LtePdschStatIndication_Fmt,
                                 ARRAY_SIZE(LtePdschStatIndication_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_pdsch_stat_indication_payload(b, offset, length,
                                                            result);
        break;
    case LTE_PHY_System_Scan_Results:
        offset += _decode_by_fmt(LtePhySystemScanResults_Fmt,
                                 ARRAY_SIZE(LtePhySystemScanResults_Fmt, Fmt),
                                 b, offset, length, result);
        offset += _decode_lte_phy_system_scan_results_payload(b, offset, length,
                                                              result);
        break;
    case LTE_PHY_BPLMN_Cell_Request:
        offset += _decode_by_fmt(LtePhyBplmnCellRequest_Fmt,
                                 ARRAY_SIZE(LtePhyBplmnCellRequest_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_phy_bplmn_cell_request_payload(b, offset, length,
                                                             result);
        break;
    case LTE_PHY_BPLMN_Cell_Confirm:
        offset += _decode_by_fmt(LtePhyBplmnCellConfirm_Fmt,
                                 ARRAY_SIZE(LtePhyBplmnCellConfirm_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_phy_bplmn_cell_confirm_payload(b, offset, length,
                                                             result);
        break;
    case LTE_PHY_Serving_Cell_COM_Loop:
        offset += _decode_by_fmt(LtePhyServingCellComLoop_Fmt,
                                 ARRAY_SIZE(LtePhyServingCellComLoop_Fmt, Fmt),
                                 b, offset, length, result);
        offset += _decode_lte_phy_serving_cell_com_loop_payload(b, offset,
                                                                length, result);
        break;
    case LTE_PHY_PDCCH_Decoding_Result:
        offset += _decode_by_fmt(LtePhyPdcchDecodingResult_Fmt,
                                 ARRAY_SIZE(LtePhyPdcchDecodingResult_Fmt, Fmt),
                                 b, offset, length, result);
        offset += _decode_lte_phy_pdcch_decoding_result_payload(b, offset,
                                                                length, result);
        break;
    case LTE_PHY_PDSCH_Decoding_Result:
        offset += _decode_by_fmt(LtePhyPdschDecodingResult_Fmt,
                                 ARRAY_SIZE(LtePhyPdschDecodingResult_Fmt, Fmt),
                                 b, offset, length, result);
        offset += _decode_lte_phy_pdsch_decoding_result_payload(b, offset,
                                                                length, result);
        break;
    case LTE_PHY_PUSCH_Tx_Report:
        offset += _decode_by_fmt(LtePhyPuschTxReport_Fmt,
                                 ARRAY_SIZE(LtePhyPuschTxReport_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_phy_pusch_tx_report_payload(b, offset, length, result);
        break;
    case LTE_PHY_RLM_Report:
        offset += _decode_by_fmt(LtePhyRlmReport_Fmt,
                                 ARRAY_SIZE(LtePhyRlmReport_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_phy_rlm_report_payload(b, offset, length, result);
        break;
    case LTE_PHY_PUSCH_CSF:
        offset += _decode_by_fmt(LtePhyPuschCsf_Fmt,
                                 ARRAY_SIZE(LtePhyPuschCsf_Fmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_lte_phy_pusch_csf_payload(b, offset, length, result);
        break;
    case LTE_PHY_CDRX_Events_Info:
        offset += _decode_by_fmt(LtePhyCdrxEventsInfo_Fmt,
                                 ARRAY_SIZE(LtePhyCdrxEventsInfo_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_phy_cdrx_events_info_payload(b, offset, length, result);
        break;
    case WCDMA_RRC_States:
        offset += _decode_wcdma_rrc_states_payload(b, offset, length, result);
        break;
    case LTE_PHY_Idle_Neighbor_Cell_Meas:
        offset +=
            _decode_by_fmt(LtePhyIncm_Fmt, ARRAY_SIZE(LtePhyIncm_Fmt, Fmt), b,
                           offset, length, result);
        offset += _decode_lte_phy_idle_neighbor_cell_meas_payload(
            b, offset, length, result);
        break;
    case WCDMA_Search_Cell_Reselection_Rank:
        offset += _decode_by_fmt(WcdmaScrr_Fmt, ARRAY_SIZE(WcdmaScrr_Fmt, Fmt),
                                 b, offset, length, result);
        offset += _decode_wcdma_scrr_payload(b, offset, length, result);
        break;
    case GSM_RR_Cell_Information:
        offset += _decode_by_fmt(GsmRrCellInfo_Fmt,
                                 ARRAY_SIZE(GsmRrCellInfo_Fmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_gsm_rci_payload(b, offset, length, result);
        break;
    case GSM_RR_Cell_Reselection_Parameters:
        offset += _decode_by_fmt(GsmRrCellResParm_Fmt,
                                 ARRAY_SIZE(GsmRrCellResParm_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_gsm_rcrp_payload(b, offset, length, result);
        break;
    case GSM_Surround_Cell_BA_List:
        offset += _decode_by_fmt(GsmScbl_Fmt, ARRAY_SIZE(GsmScbl_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_gsm_scbl_payload(b, offset, length, result);
        break;
    case GSM_RR_Cell_Reselection_Meas:
        offset += _decode_by_fmt(GsmRrCellResMeas_Fmt,
                                 ARRAY_SIZE(GsmRrCellResMeas_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_gsm_rcrm_payload(b, offset, length, result);
        break;
    case GSM_RR_Signaling_Message:
        offset += _decode_by_fmt(GsmRrSignalingMsg_Fmt,
                                 ARRAY_SIZE(GsmRrSignalingMsg_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_gsm_rr_signaling_msg_payload(b, offset, length, result);
        break;
    case GSM_DSDS_RR_Cell_Information:
        offset += _decode_by_fmt(GsmDsdsRrCellInfo_Fmt,
                                 ARRAY_SIZE(GsmDsdsRrCellInfo_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_gsm_drci_payload(b, offset, length, result);
        break;
    case GSM_DSDS_RR_Cell_Reselection_Parameters:
        offset += _decode_by_fmt(GsmDsdsRrCellResParm_Fmt,
                                 ARRAY_SIZE(GsmDsdsRrCellResParm_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_gsm_drcrp_payload(b, offset, length, result);
        break;
    case GSM_DSDS_RR_Signaling_Message:
        offset += _decode_by_fmt(GsmDsdsRrSignalingMsg_Fmt,
                                 ARRAY_SIZE(GsmDsdsRrSignalingMsg_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_gsm_dsds_rr_signaling_msg_payload(b, offset, length,
                                                            result);
        break;

    case Srch_TNG_1x_Searcher_Dump:
        offset +=
            _decode_by_fmt(SrchTng1xsd_Fmt, ARRAY_SIZE(SrchTng1xsd_Fmt, Fmt), b,
                           offset, length, result);
        offset += _decode_srch_tng_1xsd_payload(b, offset, length, result);
        break;
    case _1xEVDO_Multi_Carrier_Pilot_Sets:
        offset +=
            _decode_by_fmt(_1xEvdoMcps_Fmt, ARRAY_SIZE(_1xEvdoMcps_Fmt, Fmt), b,
                           offset, length, result);
        offset += _decode_1xevdo_mcps_payload(b, offset, length, result);
        break;
    case LTE_PHY_PUCCH_Tx_Report:
        offset += _decode_by_fmt(LtePhyPucchTxReport_Fmt,
                                 ARRAY_SIZE(LtePhyPucchTxReport_Fmt, Fmt), b,
                                 offset, length, result);
        offset +=
            _decode_lte_phy_pucch_tx_report_payload(b, offset, length, result);
        break;
    case LTE_PDCP_DL_Cipher_Data_PDU:
        offset += _decode_by_fmt(LtePdcpDlCipherDataPdu_Fmt,
                                 ARRAY_SIZE(LtePdcpDlCipherDataPdu_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_pdcp_dl_cipher_data_pdu_payload(b, offset, length,
                                                              result);
        break;
    case LTE_PDCP_UL_Cipher_Data_PDU:
        offset += _decode_by_fmt(LtePdcpUlCipherDataPdu_Fmt,
                                 ARRAY_SIZE(LtePdcpUlCipherDataPdu_Fmt, Fmt), b,
                                 offset, length, result);
        offset += _decode_lte_pdcp_ul_cipher_data_pdu_payload(b, offset, length,
                                                              result);
        break;
    case LTE_PHY_PUCCH_CSF:
        offset += _decode_by_fmt(LtePhyPucchCsf_Fmt,
                                 ARRAY_SIZE(LtePhyPucchCsf_Fmt, Fmt), b, offset,
                                 length, result);
        offset += _decode_lte_phy_pucch_csf_payload(b, offset, length, result);
        break;
    case LTE_PHY_Connected_Mode_Neighbor_Meas_Req_Resp:
        offset +=
            _decode_by_fmt(LtePhyCncm_Fmt, ARRAY_SIZE(LtePhyCncm_Fmt, Fmt), b,
                           offset, length, result);
        offset += _decode_lte_phy_connected_neighbor_cell_meas_payload(
            b, offset, length, result);
        break;
    default:
        break;
    };
}
/* 
PyObject *decode_log_packet(const char *b, size_t length, bool skip_decoding) {

    if (PyDateTimeAPI == NULL) // import datetime module
        PyDateTime_IMPORT;

    int offset = 0;

    // Parse Header
    json result;
    offset = 0;
    offset +=
        _decode_by_fmt(LogPacketHeaderFmt, ARRAY_SIZE(LogPacketHeaderFmt, Fmt),
                       b, offset, length, result);
    // PyObject *old_result = result;
    // result = PyList_GetSlice(result, 1, 4); // remove the duplicate "len1"
    // field
    //
    // old_result = NULL;

    // Differentiate using type ID
    LogPacketType type_id = (LogPacketType)_map_result_field_to_name(
        result, "type_id", LogPacketTypeID_To_Name,
        ARRAY_SIZE(LogPacketTypeID_To_Name, ValueName), "Unsupported");

    if (skip_decoding) { // skip further decoding

        result["Msg"] = std::string("raw_msg/skip_decoding") +
                        make_string(b + offset, length - offset);
        return result;
    }

    on_demand_decode(b + offset, length - offset, type_id, result);

    return result;
}
*/
/*-----------------------------------------------------------------------
 * DEBUG ONLY
 */
/* 
static int _decode_by_fmt_modem(const Fmt fmt[], int n_fmt, const char *b,
                                int offset, size_t length, json &result) {
    (void)length;
    assert(PyList_Check(result));
    int n_consumed = 0;

    for (int i = 0; i < n_fmt; i++) {

        const char *p = b + offset + n_consumed;
        switch (fmt[i].type) {
        case UINT: {
            unsigned int ii = 0;
            unsigned long long iiii = 0;
            switch (fmt[i].len) {
            case 1:
                ii = *((unsigned char *)p);
                break;
            case 2:
                ii = *((unsigned short *)p);
                break;
            case 4:
                ii = *((unsigned int *)p);
                break;
            case 8:
                // iiii = *((unsigned long long *) p);
                iiii = 0; // Yuanjie: quick work-around to avoid crash. For
                          // modem debug, this is timestamp
                break;
            default:
                assert(false);
                break;
            }
            // Convert to a Python integer object or a Python long integer
            // object
            // TODO: make it little endian
            if (fmt[i].len <= 4)
                decoded = ii;
            else
                decoded = iiii;
            n_consumed += fmt[i].len;
            break;
        }

        case QCDM_TIMESTAMP: {
            const double PER_SECOND = 52428800.0;
            const double PER_USECOND = 52428800.0 / 1.0e6;
            assert(fmt[i].len == 8);
            // Convert to a Python long integer object
            // unsigned long long iiii = *((unsigned long long *) p);
            unsigned long long iiii = 0; // Yuanjie: FIX crash on Android
            int seconds = int(double(iiii) / PER_SECOND);
            int useconds =
                (double(iiii) / PER_USECOND) - double(seconds) * 1.0e6;
            PyObject *epoch =
                PyDateTime_FromDateAndTime(1980, 1, 6, 0, 0, 0, 0);
            PyObject *delta = PyDelta_FromDSU(0, seconds, useconds);
            decoded = PyNumber_Add(epoch, delta);
            n_consumed += fmt[i].len;
            break;
        }

        case SKIP:
            n_consumed += fmt[i].len;
            break;

        default:
            assert(false);
            break;
        }

        if (decoded != NULL) {
            result[fmt[i].field_name] = decoded;
        }
    }
    return n_consumed;
}

PyObject *decode_log_packet_modem(const char *b, size_t length,
                                  bool skip_decoding) {
    if (PyDateTimeAPI == NULL) // import datetime module
        PyDateTime_IMPORT;

    int offset = 0;

    // Parse Header
    json result;
    offset = 0;
    offset += _decode_by_fmt_modem(LogPacketHeaderFmt,
                                   ARRAY_SIZE(LogPacketHeaderFmt, Fmt), b,
                                   offset, length, result);

    PyObject *old_result = result;
    result = PyList_GetSlice(result, 1, 4); // remove the duplicate "len1" field
    old_result = NULL;

    // Differentiate using type ID
    LogPacketType type_id = (LogPacketType)_map_result_field_to_name(
        result, "type_id", LogPacketTypeID_To_Name,
        ARRAY_SIZE(LogPacketTypeID_To_Name, ValueName), "Unsupported");

    if (skip_decoding) { // skip further decoding
        return result;
    }

    switch (type_id) {

    case Modem_debug_message: // Yuanjie: modem debugging message
        offset += _decode_by_fmt_modem(ModemDebug_Fmt,
                                       ARRAY_SIZE(ModemDebug_Fmt, Fmt), b,
                                       offset, length, result);
        offset += _decode_modem_debug_msg(b, offset, length, result);
        break;

    default:
        break;
    };

    return result;
}
*/
