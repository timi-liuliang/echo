#include "bit_buffer.h"
#include <assert.h>
#include <algorithm>

namespace cmpeg
{
	bit_buffer::bit_buffer(int size)
	{
		m_bytes.resize(size>>3);
		m_write_index = 0;
		m_read_index = 0;
	}

	int bit_buffer::read(int count)
	{
		assert(size()>count);

		int result = peek(count);
		m_read_index += count;

		return result;
	}

	void bit_buffer::write(const std::vector<uint8_t>& data)
	{
		// ensure there are enough space for write
		tidy(data.size() << 3);

		// write data
		assert(m_write_index % 8== 0);
		int write_base_byte = m_write_index >> 3;
		for (size_t i = 0; i < data.size(); i++)
		{
			m_bytes[write_base_byte] = data[i];
			write_base_byte++;
		}

		m_write_index = write_base_byte << 3;
	}

	void bit_buffer::tidy(size_t append_size)
	{
		if ((m_bytes.size() << 3) - m_write_index < append_size)
		{
			if (m_read_index > append_size && (m_read_index % 8 == 0))
			{
				int copy_len = (m_write_index - m_read_index) >> 3;
				int read_index_byte = m_read_index >> 3;
				for ( int i=0; i<copy_len; i++)
				{
					m_bytes[i] = m_bytes[read_index_byte + i];
				}

				m_write_index -= m_read_index;
				m_read_index = 0;
			}
			else
			{
				m_bytes.resize(m_bytes.size() + (append_size >> 3));
			}
		}
	}

	bool bit_buffer::has(int count)
	{
		return size() >= count;
	}

	int bit_buffer::find_start_code(uint8_t code)
	{
		int current = 0;
		while (true) {
			current = find_next_start_code();
			if (current == code || current == -1)
				return current;
		}

		return -1;
	}

	int bit_buffer::find_next_start_code()
	{
		for (int i = ((m_read_index + 7) >> 3); i < byte_size(); i++) {
			if (m_bytes[i] == 0x00 && m_bytes[i + 1] == 0x00 && m_bytes[i + 2] == 0x01) {
				m_read_index = (i + 4) << 3;
				return m_bytes[i + 3];
			}
		}

		m_read_index = byte_size() << 3;
		return -1;
	}

	int bit_buffer::peek(int count)
	{
		int offset = m_read_index;
		int value = 0;
		while (count) {
			uint8_t current_byte = m_bytes[offset >> 3];
			int remaining = 8 - (offset & 7);
			int read = remaining < count ? remaining : count;
			int shift = remaining - read;
			int mask = 0xff >> (8 - read);

			value = (value << read) | ((current_byte & (mask << shift)) >> shift);

			offset += read;
			count -= read;
		}

		return value;
	}

	void bit_buffer::rewind(int count)
	{
		m_read_index = std::max<int>(m_read_index-count, 0);
	}

	bool bit_buffer::next_bytes_are_start_code()
	{
		int i = ((m_read_index + 7) >> 3);
		return (
			i >= this->byte_size() || (
					m_bytes[i] == 0x00 &&
					m_bytes[i+1] == 0x00 &&
					m_bytes[i+2] == 0x01
				)
			);
	}
}
