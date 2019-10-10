#pragma once

#include <map>
#include "bit_buffer.h"
#include "decoder_base.h"

namespace cmpeg
{
	class demuxer_ts
	{
	public:
		static const uint8_t PACK_HEADER = 0xBA;
		static const uint8_t SYSTEM_HEADER = 0xBB;
		static const uint8_t PROGRAM_MAP = 0xBC;
		static const uint8_t PRIVATE_1 = 0xBD;
		static const uint8_t PADDING = 0xBE;
		static const uint8_t PRIVATE_2 = 0xBF;
		static const uint8_t AUDIO_1 = 0xC0;
		static const uint8_t VIDEO_1 = 0xE0;
		static const uint8_t DIRECTORY = 0xFF;

	public:
		demuxer_ts();
		~demuxer_ts();
		void connect(uint8_t stream_id, decoder_base* decoder);
		void write(const std::vector<uint8_t>& data);
		bool parse_packet();
		bool resync();
		void packet_complete(packet_info* pi);
		void packet_start(packet_info* pi, int pts, int payloadlength);
		bool packet_add_data(packet_info* pi, int start, int end);


	public:
		bit_buffer*					m_bits;
		std::map<int, int>			m_pids_to_stream_ids;
		std::map<int, packet_info*>	m_pes_packet_info;
		bool						m_guess_video_frame_end;
		int							m_current_time;
		int							m_start_time;
	};
}