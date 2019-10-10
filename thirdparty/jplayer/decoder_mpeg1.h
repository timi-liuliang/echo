#pragma once

#include "decoder_base.h"

namespace cmpeg
{
	class decoder_mpeg1 : public decoder_base
	{
	public:
		decoder_mpeg1();
		virtual ~decoder_mpeg1();
		virtual bool write(int pts, const std::vector<uint8_t>& buffer);
		
	private:
		void decode_sequence_header();
		virtual bool decode();
		void init_buffers();
		void decode_picture();
		void decode_slice(int slice);
		void decode_macro_block();
		void decode_motion_vectors();
		void decode_block(int block);
		void copy_macro_block(int motionH, int motionV, uint8_t* sY, uint8_t* sCr, uint8_t* sCb);
		void copy_value_to_destination(int value, uint8_t* dest, int index, int scan);
		void add_value_to_destination(int value, uint8_t* dest, int index, int scan);
		void copy_block_to_destination(int* block, uint8_t* dest, int index, int scan);
		void add_block_to_destination(int* block, uint8_t* dest, int index, int scan);
		void IDCT(int* block);
		void fill(int* block_data, int value);

	private:
		template<typename T> T decoder_mpeg1::read_huffman(T* code_table)
		{
			int state = 0;
			do
			{
				state = code_table[state + m_bits->read(1)];
			} while (state >= 0 && code_table[state] != 0);

			return code_table[state + 2];
		}

	private:
		bool		m_has_sequence_header;
		double		m_frame_rate;
		bool		m_decode_first_frame;
		int			m_width;
		int			m_height;
		int			m_mb_width;
		int			m_mb_height;
		int			m_mb_size;
		int			m_coded_width;
		int			m_coded_height;
		int			m_coded_size;
		int			m_half_width;
		int			m_half_height;
		int			m_picture_type;
		int			m_current_frame;

		bool		m_slice_begin;
		int			m_macro_block_address;
		int			m_macro_block_type;
		int			m_macro_block_intra;
		int			m_macro_block_motFw;
		int			m_mb_row;
		int			m_mb_col;

		int			m_motion_FwH;
		int			m_motion_FwH_prev;
		int			m_motion_FwV;
		int			m_motion_FwV_prev;

		int			m_dc_predictor_Y;
		int			m_dc_predictor_Cr;
		int			m_dc_predictor_Cb;
		int			m_quantizer_scale;

		bool		m_full_pel_foward;
		int			m_forward_f_code;
		int			m_forward_r_size;
		int			m_forward_f;

		int*		m_intra_quant_matrix;
		int*		m_non_intra_quant_matrix;
		int*		m_custom_intra_quant_matrix;
		int*		m_custom_non_intra_quant_matrix;

		int32_t*	m_block_data;

		uint8_t*	m_current_Y;
		uint8_t*	m_current_Cr;
		uint8_t*	m_current_Cb;
		uint8_t*	m_forward_Y;
		uint8_t*	m_forward_Cr;
		uint8_t*	m_forward_Cb;
	};
}