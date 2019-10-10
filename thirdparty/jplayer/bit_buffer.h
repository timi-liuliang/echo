#pragma once

#include <vector>

namespace cmpeg
{
	class bit_buffer
	{
	public:
		bit_buffer(int size = 0);
		int read(int count);
		void write(const std::vector<uint8_t>& data);
		void skip(size_t count) { m_read_index += count; }
		int peek(int count);
		void rewind(int count);
		bool has(int count);
		int find_start_code(uint8_t code);
		int find_next_start_code();
		int size() { return m_write_index - m_read_index; }
		int byte_size() { return size() >> 3; }
		uint8_t byte(size_t idx) { return m_bytes[idx]; }
		int read_index() const { return m_read_index; }
		void set_read_indx(size_t index) { m_read_index = index; }
		bool next_bytes_are_start_code();

	private:
		void tidy( size_t append_size);

	private:
		std::vector<uint8_t> m_bytes;
		size_t				 m_write_index;		// unit bit, m_write_index % 8 == 0
		size_t				 m_read_index;		// unit bit
	};
}