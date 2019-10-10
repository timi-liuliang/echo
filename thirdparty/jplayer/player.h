#pragma once

#include "render_base.h"
#include "easywsclient.h"
#include "demuxer_ts.h"

namespace cmpeg
{
	class player
	{
	public:
		player(render_base* render, const char* url);
		void set_url(const char* url);
		void play();
		void stop();
		void update();

	public:
		void on_accept_data(const std::vector<uint8_t>& data);

	private:
		std::string							m_url;
		easywsclient::WebSocket::pointer	m_socket;
		demuxer_ts							m_demuxer;
		decoder_base*						m_video_decoder;
	};
}