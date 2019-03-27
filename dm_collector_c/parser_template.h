/*
 * LTE ******
 */

#include "consts.h"
#include "log_packet.h"
#include "log_packet_helper.h"

const Fmt Lte ******_Fmt[] = {
    {UINT, "Version", 1},
};

const Fmt Lte ***_Payload_v *[] = {};

const Fmt Lte ***_Record_v *[] = {};

const Fmt Lte ***_Stream_v *[] = {};

const Fmt Lte ***_EnergyMetric_v *[] = {};

static int _decode_lte_ ***_payload(const char *b, int offset, size_t length,
                                    json &result) {
    int start = offset;
    int pkt_ver = _search_result_int(result, "Version");

    double pyfloat;
    int temp;

    switch (pkt_ver) {
    case *: {
        offset += _decode_by_fmt(Lte * **_Payload_v *,
                                 ARRAY_SIZE(Lte * **_Payload_v *, Fmt), b,
                                 offset, length, result);

        json result_record;
        for (int i = 0; i < num_record; i++) {
            json result_record_item;
            offset += _decode_by_fmt(Lte * **_Record_v *,
                                     ARRAY_SIZE(Lte * **_Record_v *, Fmt), b,
                                     offset, length, result_record_item);

            json result_record_stream;
            for (int j = 0; j < num_stream; j++) {
                json result_record_stream_item;
                offset += _decode_by_fmt(
                    Lte * **_Stream_v *, ARRAY_SIZE(Lte * **_Stream_v *, Fmt),
                    b, offset, length, result_record_stream_item);

                json result_energy_metric;
                for (int k = 0; k < num_energy_metric; k++) {
                    json result_energy_metric_item;
                    offset += _decode_by_fmt(
                        Lte * **_EnergyMetric_v *,
                        ARRAY_SIZE(Lte * **_EnergyMetric_v *, Fmt), b, offset,
                        length, result_energy_metric_item);

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
    default:
        printf("(MI)Unknown LTE *** version: %d\n", pkt_ver);
        return 0;
    }
}
