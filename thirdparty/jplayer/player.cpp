#include "player.h"
#include "decoder_mpeg1.h"
#include "decoder_mpeg5.h"
#include <Windows.h>

namespace cmpeg
{
	static void handle_ws_message(void* user_data, const std::vector<uint8_t>& message)
	{
		player* pla = (player*)user_data;
		if (pla)
		{
			pla->on_accept_data( message);
		}
	}

	player::player(render_base* render, const char* url)
	{
		m_video_decoder = new decoder_mpeg5();
		m_video_decoder->connect(render);
		m_demuxer.connect( demuxer_ts::VIDEO_1, m_video_decoder);

		set_url(url);
	}

	void player::set_url(const char* url)
	{
		if (!m_url.empty()) {
			stop();
		}

		m_url = url;
	}

	void player::play()
	{
		if (m_url.empty())
			return;

		// connect url
		m_socket = easywsclient::WebSocket::from_url(m_url);
		m_socket->set_user_data(this);
		while (m_socket->getReadyState() != easywsclient::WebSocket::CLOSED) {
			m_socket->poll();
			m_socket->dispatchBinary(handle_ws_message);

			Sleep(10);
		}
	}

	void player::stop()
	{
		m_url.clear();
	}

	void player::on_accept_data(const std::vector<uint8_t>& data)
	{
		m_demuxer.write(data);
	}

	void player::update()
	{
		if (m_video_decoder)
			m_video_decoder->decode();
	}
}