// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: NetStreamFfmpeg.cpp,v 1.13 2007/02/05 22:22:32 tgc Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_FFMPEG

#include "log.h"
#include "NetStreamFfmpeg.h"
#include "fn_call.h"
#include "NetStream.h"
#include "render.h"	
#include "movie_root.h"
#include "NetConnection.h"

#include "tu_file.h"

#if defined(_WIN32) || defined(WIN32)
	#include <Windows.h>	// for sleep()
	#define usleep(x) Sleep(x/1000)
#else
	#include "unistd.h" // for usleep()
#endif



namespace gnash {


NetStreamFfmpeg::NetStreamFfmpeg():
	m_video_index(-1),
	m_audio_index(-1),

	m_VCodecCtx(NULL),
	m_ACodecCtx(NULL),
	m_FormatCtx(NULL),
	m_Frame(NULL),
	m_Resample(NULL),

	m_thread(NULL),
	startThread(NULL),

	m_go(false),
	m_imageframe(NULL),
	m_video_clock(0),
	m_pause(false),
	m_unqueued_data(NULL),
	inputPos(0)
{
}

NetStreamFfmpeg::~NetStreamFfmpeg()
{
	close();
}

// called from avstreamer thread
void NetStreamFfmpeg::set_status(const char* /*code*/)
{
	if (_parent)
	{
		//m_netstream_object->init_member("onStatus_Code", code);
		//push_video_event(m_netstream_object);
	}
}

void NetStreamFfmpeg::pause(int mode)
{
	if (mode == -1)
	{
		m_pause = ! m_pause;
	}
	else
	{
		m_pause = (mode == 0) ? true : false;
	}
}

void NetStreamFfmpeg::close()
{

	if (m_go)
	{
		// terminate thread
		m_go = false;

		startThread->join();
		delete startThread;

		// wait till thread is complete before main continues
		m_thread->join();
		delete m_thread;

	}


	// When closing gnash before playback is finished, the soundhandler 
	// seems to be removed before netstream is destroyed.
	/*sound_handler* s = get_sound_handler();
	if (s != NULL)
	{
		s->detach_aux_streamer((void*) NULL);
	}*/

	if (m_Frame) av_free(m_Frame);
	m_Frame = NULL;

	if (m_VCodecCtx) avcodec_close(m_VCodecCtx);
	m_VCodecCtx = NULL;

	if (m_ACodecCtx) avcodec_close(m_ACodecCtx);
	m_ACodecCtx = NULL;

	if (m_FormatCtx) {
		m_FormatCtx->iformat->flags = AVFMT_NOFILE;
		av_close_input_file(m_FormatCtx);
		m_FormatCtx = NULL;
	}

	if (m_Resample) {
		audio_resample_close (m_Resample);
	}

	if (m_imageframe) delete m_imageframe;

	while (m_qvideo.size() > 0)
	{
		delete m_qvideo.front();
		m_qvideo.pop();
	}

	while (m_qaudio.size() > 0)
	{
		delete m_qaudio.front();
		m_qaudio.pop();
	}

}

// ffmpeg callback function
int 
NetStreamFfmpeg::readPacket(void* opaque, uint8_t* buf, int buf_size)
{

	NetStreamFfmpeg* ns = static_cast<NetStreamFfmpeg*>(opaque);
	NetConnection* nc = ns->_netCon;

	size_t ret = nc->read(static_cast<void*>(buf), buf_size);
	ns->inputPos += ret;
	return ret;

}

// ffmpeg callback function
offset_t 
NetStreamFfmpeg::seekMedia(void *opaque, offset_t offset, int whence){

	NetStreamFfmpeg* ns = static_cast<NetStreamFfmpeg*>(opaque);
	NetConnection* nc = ns->_netCon;


	// Offset is absolute new position in the file
	if (whence == SEEK_SET) {
		nc->seek(offset);
		ns->inputPos = offset;

	// New position is offset + old position
	} else if (whence == SEEK_CUR) {
		nc->seek(ns->inputPos + offset);
		ns->inputPos = ns->inputPos + offset;

	// 	// New position is offset + end of file
	} else if (whence == SEEK_END) {
		// This is (most likely) a streamed file, so we can't seek to the end!
		// Instead we seek to 50.000 bytes... seems to work fine...
		nc->seek(50000);
		ns->inputPos = 50000;
		
	}

	return ns->inputPos;
}

int
NetStreamFfmpeg::play(const char* c_url)
{

	// Is it already playing ?
	if (m_go)
	{
		if (m_pause) m_pause = false;
		return 0;
	}

	// Does it have an associated NetConnectoin ?
	if ( ! _netCon )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("No NetConnection associated with this NetStream, won't play");
		);
		return 0;
	}

	url += c_url;
	// Remove any "mp3:" prefix. Maybe should use this to mark as audio-only
	if (url.compare(0, 4, std::string("mp3:")) == 0) {
		url = url.substr(4);
	}

	m_go = true;

	// To avoid blocking while connecting, we use a thread.
	startThread = new boost::thread(boost::bind(NetStreamFfmpeg::startPlayback, this));

	// This starts the decoding thread
	lock = new boost::mutex::scoped_lock(start_mutex);
	m_thread = new boost::thread(boost::bind(NetStreamFfmpeg::av_streamer, this)); 

	return 0;
}

void
NetStreamFfmpeg::startPlayback(NetStreamFfmpeg* ns)
{

	NetConnection* nc = ns->_netCon;
	assert(nc);

	// Pass stuff from/to the NetConnection object.
	assert(ns); // ns->_parent is ok being NULL
	if ( !nc->openConnection(ns->url.c_str(), ns->_parent) ) {
		log_warning("Gnash could not open movie url: %s", ns->url.c_str());
		return;
	}

	ns->inputPos = 0;

	// This registers all available file formats and codecs 
	// with the library so they will be used automatically when
	// a file with the corresponding format/codec is opened

	av_register_all();

	// Open video file

	// Probe the file to detect the format
	AVProbeData probe_data, *pd = &probe_data;
	pd->filename = "";
	pd->buf = new uint8_t[2048];
	pd->buf_size = 2048;
	readPacket(ns, pd->buf, pd->buf_size);

	AVInputFormat* inputFmt = av_probe_input_format(pd, 1);

	// After the format probe, reset to the beginning of the file.
	nc->seek(0);

	// Setup the filereader/seeker mechanism. 7th argument (NULL) is the writer function,
	// which isn't needed.
	init_put_byte(&ns->ByteIOCxt, new uint8_t[500000], 500000, 0, ns, NetStreamFfmpeg::readPacket, NULL, NetStreamFfmpeg::seekMedia);
	ns->ByteIOCxt.is_streamed = 1;

	ns->m_FormatCtx = av_alloc_format_context();

	// Open the stream. the 4th argument is the filename, which we ignore.
	if(av_open_input_stream(&ns->m_FormatCtx, &ns->ByteIOCxt, "", inputFmt, NULL) < 0){
		log_error("Couldn't open file '%s' for decoding", ns->url.c_str());
		ns->set_status("NetStream.Play.StreamNotFound");
		return;
	}

	// Next, we need to retrieve information about the streams contained in the file
	// This fills the streams field of the AVFormatContext with valid information
	int ret = av_find_stream_info(ns->m_FormatCtx);
	if (ret < 0)
	{
		log_error("Couldn't find stream information from '%s', error code: %d", ns->url.c_str(), ret);
		return;
	}

//	m_FormatCtx->pb.eof_reached = 0;
//	av_read_play(m_FormatCtx);

	// Find the first video & audio stream
	ns->m_video_index = -1;
	ns->m_audio_index = -1;
	assert(ns->m_FormatCtx->nb_streams >= 0);
	for (int i = 0; i < ns->m_FormatCtx->nb_streams; i++)
	{
		AVCodecContext* enc = ns->m_FormatCtx->streams[i]->codec; 

		switch (enc->codec_type)
		{
			case CODEC_TYPE_AUDIO:
				if (ns->m_audio_index < 0)
				{
					ns->m_audio_index = i;
					ns->m_audio_stream = ns->m_FormatCtx->streams[i];
				}
				break;

			case CODEC_TYPE_VIDEO:
				if (ns->m_video_index < 0)
				{
					ns->m_video_index = i;
					ns->m_video_stream = ns->m_FormatCtx->streams[i];
				}
				break;
			case CODEC_TYPE_DATA:
			case CODEC_TYPE_SUBTITLE:
			case CODEC_TYPE_UNKNOWN:
				break;
    }
	}

	if (ns->m_video_index < 0)
	{
		log_error("Didn't find a video stream from '%s'", ns->url.c_str());
		return;
	}

	// Get a pointer to the codec context for the video stream
	ns->m_VCodecCtx = ns->m_FormatCtx->streams[ns->m_video_index]->codec;

	// Find the decoder for the video stream
	AVCodec* pCodec = avcodec_find_decoder(ns->m_VCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		ns->m_VCodecCtx = NULL;
		log_error("Decoder not found");
		return;
	}

	// Open codec
	if (avcodec_open(ns->m_VCodecCtx, pCodec) < 0)
	{
		log_error("Could not open codec");
	}

	// Allocate a frame to store the decoded frame in
	ns->m_Frame = avcodec_alloc_frame();
	
	// Determine required buffer size and allocate buffer
	int videoFrameFormat = gnash::render::videoFrameFormat();

	if (videoFrameFormat == render::YUV) {
		ns->m_imageframe = new image::yuv(ns->m_VCodecCtx->width,	ns->m_VCodecCtx->height);
	} else if (videoFrameFormat == render::RGB) {
		ns->m_imageframe = new image::rgb(ns->m_VCodecCtx->width,	ns->m_VCodecCtx->height);
	}

	sound_handler* s = get_sound_handler();
	if (ns->m_audio_index >= 0 && s != NULL)
	{
		// Get a pointer to the audio codec context for the video stream
		ns->m_ACodecCtx = ns->m_FormatCtx->streams[ns->m_audio_index]->codec;

		// Find the decoder for the audio stream
		AVCodec* pACodec = avcodec_find_decoder(ns->m_ACodecCtx->codec_id);
	    if(pACodec == NULL)
		{
			log_error("No available AUDIO decoder to process MPEG file: '%s'", ns->url.c_str());
			return;
		}
        
		// Open codec
		if (avcodec_open(ns->m_ACodecCtx, pACodec) < 0)
		{
			log_error("Could not open AUDIO codec");
			return;
		}

		s->attach_aux_streamer(audio_streamer, (void*) ns);

	}

	ns->m_pause = false;
	
	// By deleting this lock we allow the av_streamer-thread to start its work
	delete ns->lock;
	return;
}

// decoder thread
void NetStreamFfmpeg::av_streamer(NetStreamFfmpeg* ns)
{

	boost::mutex::scoped_lock  lock(ns->start_mutex);

	// This should only happen if close() is called before setup is complete
	if (!ns->m_go) return;

	ns->set_status("NetStream.Play.Start");

	raw_videodata_t* video = NULL;

	ns->m_video_clock = 0;

	int delay = 0;
	ns->m_start_clock = tu_timer::ticks_to_seconds(tu_timer::get_ticks());
	ns->m_unqueued_data = NULL;

	while (ns->m_go)
	{
		if (ns->m_pause)
		{
			double t = tu_timer::ticks_to_seconds(tu_timer::get_ticks());
			usleep(100000);
			ns->m_start_clock += tu_timer::ticks_to_seconds(tu_timer::get_ticks()) - t;
			continue;
		}

		if (ns->read_frame() == false)
		{
			if (ns->m_qvideo.size() == 0)
			{
				break;
			}
		}

		if (ns->m_qvideo.size() > 0)
		{
			video = ns->m_qvideo.front();
			double clock = tu_timer::ticks_to_seconds(tu_timer::get_ticks()) - ns->m_start_clock;
			double video_clock = video->m_pts;

			if (clock >= video_clock)
			{
				int videoFrameFormat = gnash::render::videoFrameFormat();
				if (videoFrameFormat == render::YUV) {
					static_cast<image::yuv*>(ns->m_imageframe)->update(video->m_data);
				} else if (videoFrameFormat == render::RGB) {
					ns->m_imageframe->update(video->m_data);
				}
				ns->m_qvideo.pop();
				delete video;
				delay = 0;
			}
			else
			{
				delay = int((video_clock - clock)*1000000);
			}

			// Don't hog the CPU.
			// Queues have filled, video frame have shown
			// now it is possible and to have a rest

			if (ns->m_unqueued_data && delay > 0)
			{
				usleep(delay);
			}
		}
	}
	ns->set_status("NetStream.Play.Stop");

}

// audio callback is running in sound handler thread
void NetStreamFfmpeg::audio_streamer(void *owner, uint8 *stream, int len)
{
	NetStreamFfmpeg* ns = static_cast<NetStreamFfmpeg*>(owner);

	boost::mutex::scoped_lock  lock(ns->decoding_mutex);

	while (len > 0 && ns->m_qaudio.size() > 0)
	{
		raw_videodata_t* samples = ns->m_qaudio.front();

		int n = imin(samples->m_size, len);
		memcpy(stream, samples->m_ptr, n);
		stream += n;
		samples->m_ptr += n;
		samples->m_size -= n;
		len -= n;

		if (samples->m_size == 0)
		{
			ns->m_qaudio.pop();
			delete samples;
		}
	}
}

bool NetStreamFfmpeg::read_frame()
{
	boost::mutex::scoped_lock  lock(decoding_mutex);

//	raw_videodata_t* ret = NULL;
	if (m_unqueued_data)
	{
		if (m_unqueued_data->m_stream_index == m_audio_index)
		{
			sound_handler* s = get_sound_handler();
			if (s)
			{
				m_unqueued_data = m_qaudio.push(m_unqueued_data) ? NULL : m_unqueued_data;
			}
		}
		else
		if (m_unqueued_data->m_stream_index == m_video_index)
		{
			m_unqueued_data = m_qvideo.push(m_unqueued_data) ? NULL : m_unqueued_data;
		}
		else
		{
			log_warning("read_frame: not audio & video stream\n");
		}

		return true;
	}

	AVPacket packet;
	int rc = av_read_frame(m_FormatCtx, &packet);
	if (rc >= 0)
	{
		if (packet.stream_index == m_audio_index)
		{
			sound_handler* s = get_sound_handler();
			if (s)
			{
				int frame_size;
				uint8_t* ptr = (uint8_t*) malloc((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2);
				if (avcodec_decode_audio(m_ACodecCtx, (int16_t*) ptr, &frame_size, packet.data, packet.size) >= 0)
				{

					bool stereo = m_ACodecCtx->channels > 1 ? true : false;
					int samples = stereo ? frame_size >> 2 : frame_size >> 1;

					// Resampeling using ffmpegs (libavcodecs) resampler
					if (!m_Resample) {
						// arguments: (output channels, input channels, output rate, input rate)
						m_Resample = audio_resample_init (2, m_ACodecCtx->channels, 44100, m_ACodecCtx->sample_rate);
					}
					// The size of this is a guess, we don't know yet... Lets hope it's big enough
					int16_t* output_data = new int16_t[frame_size * 2];
					samples = audio_resample (m_Resample, output_data, (int16_t*)ptr, samples);

					raw_videodata_t* raw = new raw_videodata_t;
					raw->m_data = (uint8_t*) output_data;
					raw->m_ptr = raw->m_data;
					raw->m_size = samples * 2 * 2; // 2 for stereo and 2 for samplesize = 2 bytes
					raw->m_stream_index = m_audio_index;

					m_unqueued_data = m_qaudio.push(raw) ? NULL : raw;
				}
				free(ptr);
			}
		}
		else
		if (packet.stream_index == m_video_index)
		{
			int got = 0;
			avcodec_decode_video(m_VCodecCtx, m_Frame, &got, packet.data, packet.size);
			if (got) {
				uint8_t *buffer = NULL;
				int videoFrameFormat = gnash::render::videoFrameFormat();

				if (videoFrameFormat == render::NONE) { // NullGui?
					av_free_packet(&packet);
					return false;

				} else if (videoFrameFormat == render::YUV && m_VCodecCtx->pix_fmt != PIX_FMT_YUV420P) {
					assert(0);	// TODO
					//img_convert((AVPicture*) pFrameYUV, PIX_FMT_YUV420P, (AVPicture*) pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

				} else if (videoFrameFormat == render::RGB && m_VCodecCtx->pix_fmt != PIX_FMT_RGB24) {
					AVFrame* frameRGB = avcodec_alloc_frame();
					unsigned int numBytes = avpicture_get_size(PIX_FMT_RGB24, m_VCodecCtx->width, m_VCodecCtx->height);
					buffer = new uint8_t[numBytes];
					avpicture_fill((AVPicture *)frameRGB, buffer, PIX_FMT_RGB24, m_VCodecCtx->width, m_VCodecCtx->height);
					img_convert((AVPicture*) frameRGB, PIX_FMT_RGB24, (AVPicture*) m_Frame, m_VCodecCtx->pix_fmt, m_VCodecCtx->width, m_VCodecCtx->height);
					av_free(m_Frame);
					m_Frame = frameRGB;
				}

				raw_videodata_t* video = new raw_videodata_t;
				if (videoFrameFormat == render::YUV) {
					video->m_data = new uint8_t[static_cast<image::yuv*>(m_imageframe)->size()];
				} else if (videoFrameFormat == render::RGB) {
					image::rgb* tmp = static_cast<image::rgb*>(m_imageframe);
					video->m_data = new uint8_t[tmp->m_pitch * tmp->m_height];
				}

				video->m_ptr = video->m_data;
				video->m_stream_index = m_video_index;

				// set presentation timestamp
				if (packet.dts != static_cast<signed long>(AV_NOPTS_VALUE))
				{
					video->m_pts = as_double(m_video_stream->time_base) * packet.dts;
				}

				if (video->m_pts != 0)
				{	
					// update video clock with pts, if present
					m_video_clock = video->m_pts;
				}
				else
				{
					video->m_pts = m_video_clock;
				}

				// update video clock for next frame
				double frame_delay = as_double(m_video_stream->codec->time_base);

				// for MPEG2, the frame can be repeated, so we update the clock accordingly
				frame_delay += m_Frame->repeat_pict * (frame_delay * 0.5);

				m_video_clock += frame_delay;

				if (videoFrameFormat == render::YUV) {
					image::yuv* yuvframe = static_cast<image::yuv*>(m_imageframe);
					int copied = 0;
					uint8_t* ptr = video->m_data;
					for (int i = 0; i < 3 ; i++)
					{
						int shift = (i == 0 ? 0 : 1);
						uint8_t* yuv_factor = m_Frame->data[i];
						int h = m_VCodecCtx->height >> shift;
						int w = m_VCodecCtx->width >> shift;
						for (int j = 0; j < h; j++)
						{
							copied += w;
							assert(copied <= yuvframe->size());
							memcpy(ptr, yuv_factor, w);
							yuv_factor += m_Frame->linesize[i];
							ptr += w;
						}
					}
					video->m_size = copied;
				} else if (videoFrameFormat == render::RGB) {
					for(int line = 0; line < m_VCodecCtx->height; line++)
					{
						for(int byte = 0; byte < (m_VCodecCtx->width*3); byte++)
						{
							video->m_data[byte + (line*m_VCodecCtx->width*3)] = (unsigned char) *(m_Frame->data[0]+(line*m_Frame->linesize[0])+byte);
						}
					}
				}
				delete [] buffer;

				m_unqueued_data = m_qvideo.push(video) ? NULL : video;
			}
		}
		av_free_packet(&packet);
	}
	else
	{
		log_warning("Problems decoding frame");
		return false;
	}

	return true;
}

image::image_base* NetStreamFfmpeg::get_video()
{
	return m_imageframe;
}

void
NetStreamFfmpeg::seek(double pos)
{
	boost::mutex::scoped_lock  lock(decoding_mutex);

	// Seek to new position
	double timebase = (double)m_FormatCtx->streams[m_video_index]->time_base.num / (double)m_FormatCtx->streams[m_video_index]->time_base.den;

	long newpos = (long)(pos / timebase);

	if (av_seek_frame(m_FormatCtx, m_video_index, newpos, 0) < 0) {
		log_warning("seeking failed");
		return;
	}

	// This is kindof hackish and ugly :-(
	if (newpos == 0) {
		m_video_clock = 0;
		m_start_clock = tu_timer::ticks_to_seconds(tu_timer::get_ticks());
	} else {

		AVPacket Packet;
		av_init_packet(&Packet);
		double newtime = 0;
		while (newtime == 0) {
			if ( av_read_frame(m_FormatCtx, &Packet) < 0) {
				av_seek_frame(m_FormatCtx, -1, 0,AVSEEK_FLAG_BACKWARD);
				av_free_packet(&Packet);
				return;
			}

			newtime = timebase * (double)m_FormatCtx->streams[m_video_index]->cur_dts;
		}

		av_free_packet(&Packet);
		av_seek_frame(m_FormatCtx, m_video_index, newpos, 0);

		m_start_clock +=  m_video_clock - newtime;

		m_video_clock = newtime;
	}
	// Flush the queues
	while (m_qvideo.size() > 0)
	{
		delete m_qvideo.front();
		m_qvideo.pop();
	}

	while (m_qaudio.size() > 0)
	{
		delete m_qaudio.front();
		m_qaudio.pop();
	}
}

void
NetStreamFfmpeg::setBufferTime()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

int64_t
NetStreamFfmpeg::time()
{
	if (m_FormatCtx && m_FormatCtx->nb_streams > 0) {
		double time = (double)m_FormatCtx->streams[0]->time_base.num / (double)m_FormatCtx->streams[0]->time_base.den * (double)m_FormatCtx->streams[0]->cur_dts;
	return static_cast<int64_t>(time);
	} else {
		return 0;
	}
}

} // gnash namespcae

#endif // USE_FFMPEG
