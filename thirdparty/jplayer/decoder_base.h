#pragma once

#include <vector>
#include "bit_buffer.h"
#include "render_base.h"

namespace cmpeg
{
	class decoder_base;
	struct packet_info
	{
		typedef std::vector<uint8_t> buf;

		decoder_base*	decoder;
		int				current_length;
		int				total_length;
		int				pts;
		buf				buffers;

		packet_info(decoder_base* dest)
			: decoder(dest)
			, current_length(0)
			, total_length(0)
			, pts(0)
		{}
	};

	struct time_stamp
	{
		int index;
		int time;

		time_stamp(int idx, int t) : index(idx), time(t)
		{}
	};

	class decoder_base
	{
		typedef std::vector<time_stamp> time_stamp_arr;

	public:
		decoder_base();
		virtual ~decoder_base();
		void connect(render_base* render) { m_destination = render; }
		virtual bool write(int pts, const std::vector<uint8_t>& buffer);
		void advance_decoded_time(float seconds);
		virtual bool decode()=0;

	protected:
		render_base*	m_destination;
		bool			m_can_play;
		int				m_start_time;
		float			m_decoded_time;
		bool			m_collect_time_stamps;
		time_stamp_arr	m_time_stamps;
		int				m_time_stamp_index;
		bit_buffer*		m_bits;
	};
}