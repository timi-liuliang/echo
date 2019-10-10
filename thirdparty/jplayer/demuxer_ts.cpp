#include "demuxer_ts.h"
#include <assert.h>

namespace cmpeg
{
	demuxer_ts::demuxer_ts()
		: m_current_time(0)
		, m_start_time(0)
		, m_guess_video_frame_end(true)
	{
		m_bits = new bit_buffer( 8*64*1024);
	}

	demuxer_ts::~demuxer_ts()
	{
		if (m_bits) delete m_bits; m_bits = NULL;
	}

	void demuxer_ts::connect(uint8_t stream_id, decoder_base* decoder)
	{
		m_pes_packet_info[stream_id] = new packet_info(decoder);
	}

	void demuxer_ts::write(const std::vector<uint8_t>& data)
	{
		m_bits->write(data);
		
		while(m_bits->has(188<<3) && parse_packet()) {}
	}

	bool demuxer_ts::parse_packet()
	{
		// check if we're in sync with packet boundaries; attempt to resync if not
		if (m_bits->read(8) != 0x47) {
			if (!resync()) {
				// Couldn't resync; maybe next time...
				return false;
			}
		}

		int end = (m_bits->read_index() >> 3) + 187;
		int transport_error = m_bits->read(1);
		int payload_start = m_bits->read(1);
		int transport_priority = m_bits->read(1);
		int pid = m_bits->read(13);
		int transport_scrambling = m_bits->read(2);
		int adaptation_field = m_bits->read(2);
		int continity_counter = m_bits->read(4);

		// if this is the start of a new payload; signal the end of the previous
		// frame, if we didn't do so already
		int stream_id = m_pids_to_stream_ids[pid];
		if (payload_start && stream_id) {
			packet_info* pi = (packet_info*)m_pes_packet_info[stream_id];
			if (pi && pi->current_length) {
				packet_complete( pi);
			}
		}

		// extract current payload
		if (adaptation_field & 0x1) {
			if (adaptation_field & 0x2) {
				int adaptation_field_length = m_bits->read(8);
				m_bits->skip(adaptation_field_length << 3);
			}

			if (payload_start && m_bits->next_bytes_are_start_code()){
				m_bits->skip(24);
				stream_id = m_bits->read(8);
				m_pids_to_stream_ids[pid] = stream_id;

				int packet_length = m_bits->read(16);
				m_bits->skip(8);
				int pts_dts_flag = m_bits->read(2);
				m_bits->skip(6);

				int header_length = m_bits->read(8);
				int payload_begin_index = m_bits->read_index() + (header_length << 3);

				packet_info* pi = m_pes_packet_info[stream_id];
				if (pi) {
					double pts = 0;
					if (pts_dts_flag & 0x2) {
						// The precision Timestamp is encoded as 33 bit integer
						// but has a 'marker bit' inserted at weird places
						// in between, making the whole thing 5 bytes in size
						// you can't make this shit up
						m_bits->skip(4);
						double p32_30 = m_bits->read(3);
						m_bits->skip(1);
						double p29_15 = m_bits->read(15);
						m_bits->skip(1);
						double p14_0 = m_bits->read(15);
						m_bits->skip(1);

						// can't use bit shifts here; we need 33 bits of precision
						pts = (p32_30 * 1073741824 + p29_15 * 32768 + p14_0) / 90000;
						m_current_time = pts;
						if (m_start_time == -1)
							m_start_time = pts;
					}

					int payload_length = packet_length ? packet_length - header_length - 3 : 0;
					packet_start( pi, pts, payload_length);
				}

				m_bits->set_read_indx( payload_begin_index);
			}

			if (stream_id) {
				// attempt to detect if the PES packet is complete. For Audio (and
				// other) packets, we received a total packet length with the PES
				// header, so we can check the current length

				// for video packets, we have to guess the end by detecting if this
				// TS packet was padded - there's no good reason to pad a TS packet
				// in between, but it might just fit exactly. if this fails, we can
				// only wait for the next PES header for that stream
				packet_info* pi = m_pes_packet_info[stream_id];
				if (pi) {
					int start = m_bits->read_index() >> 3;
					bool complete = packet_add_data( pi, start, end);

					bool has_padding = !payload_start && (adaptation_field & 0x2);
					if (complete || (m_guess_video_frame_end && has_padding)) {
						packet_complete(pi);
					}
				}
			}
		}

		m_bits->set_read_indx(end << 3);
		return true;
	}

	bool demuxer_ts::resync()
	{
		// check if we have enough data to attempt a resync. we need 5 full packets
		if (!m_bits->has(188 * 6) << 3) {
			return false;
		}

		int byte_index = m_bits->read_index() >> 3;

		// look for the first sync token in the first 187 bytes
		for (int i = 0; i < 187; i++){
			if (m_bits->byte(byte_index + i) == 0x47) {
				// look for 4 more sync tokens, each 188 bytes appart
				int found_sync = true;
				for (int j = 1; j < 5; j++)
				{
					if (m_bits->byte(byte_index + i + 188 * j) != 0x47) {
						found_sync = false;
						break;
					}
				}

				if (found_sync) {
					m_bits->set_read_indx((byte_index + i + 1) << 3);
					return true;
				}
			}
		}

		// In theroy, we shouldn't arrive here. if we do, we had enough data but
		// still didn't find sync - this can only hapen if we were fed garbage
		// data. check you stream source
		assert(false);
		m_bits->skip(187 << 3);
		return false;
	}

	void demuxer_ts::packet_complete(packet_info* pi)
	{
		pi->decoder->write( pi->pts, pi->buffers);
		pi->total_length = 0;
		pi->current_length = 0;
		pi->buffers.clear();
	}

	void demuxer_ts::packet_start(packet_info* pi, int pts, int payloadlength)
	{
		pi->total_length = payloadlength;
		pi->current_length = 0;
		pi->pts = pts;
	}

	bool demuxer_ts::packet_add_data(packet_info* pi, int start, int end)
	{
		for (int i = start; i < end; i++)
			pi->buffers.push_back(m_bits->byte(i));

		pi->current_length += end - start;

		bool complete = (pi->total_length != 0 && pi->current_length >= pi->total_length);
		return complete;
	}
}