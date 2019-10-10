#include "decoder_base.h"

namespace cmpeg
{
	decoder_base::decoder_base() 
		: m_destination(NULL)
		, m_can_play(false)
		, m_start_time(0)
		, m_decoded_time(0)
		, m_time_stamp_index(0)
	{
		m_bits = new bit_buffer(8 * 64 * 1024);
	}

	decoder_base::~decoder_base()
	{
		if (m_bits) delete m_bits; m_bits = NULL;
	}

	bool decoder_base::write(int pts, const std::vector<uint8_t>& buffer)
	{
		if (m_collect_time_stamps)
		{
			if (m_time_stamps.size() == 0)
			{
				m_start_time = pts;
				m_decoded_time = pts;
			}

			m_time_stamps.push_back( time_stamp( m_bits->byte_size()<<3,  pts));
		}

		m_bits->write(buffer);
		m_can_play = true;

		return true;
	}

	void decoder_base::advance_decoded_time(float seconds)
	{
		if (m_collect_time_stamps)
		{
			int new_time_stamp_index = -1;
			for (size_t i = m_time_stamp_index; i < m_time_stamps.size(); i++)
			{
				if( m_time_stamps[i].index > m_bits->read_index())
					break;

				new_time_stamp_index = i;
			}

			// did we find a new PTS, different from the last? If so, we don't have
			// to advance the decoded time manually and can instead sync it exactly
			// to the PTS.
			if (new_time_stamp_index != -1 && new_time_stamp_index != m_time_stamp_index)
			{
				m_time_stamp_index = new_time_stamp_index;
				m_decoded_time = m_time_stamps[m_time_stamp_index].time;
				return;
			}
		}

		m_decoded_time += seconds;
	}
}