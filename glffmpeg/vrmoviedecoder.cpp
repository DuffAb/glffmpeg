#include <iostream>
// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include "GL/glew.h"
#include "vrmoviedecoder.h"

vrmoviedecoder::vrmoviedecoder()
{
	fmt_ctx         = nullptr;
	video_stream    = nullptr;
	video_codec_ctx = nullptr;
	video_decoder   = nullptr;
	av_frame        = nullptr;
	gl_frame        = nullptr;
	packet          = nullptr;
	sws_ctx         = nullptr;
	save_file = false;
	fp_rgb = nullptr;
	
	/* 1. 注册所有的封装器，解封装器，格式 */
	av_register_all();
	avformat_network_init();
}

vrmoviedecoder::~vrmoviedecoder()
{
	avformat_close_input(&fmt_ctx);

	if (av_frame) av_free(av_frame);
	if (gl_frame) av_free(gl_frame);
	if (packet) av_free(packet);
	if (video_codec_ctx) avcodec_close(video_codec_ctx);
	if (fmt_ctx) avformat_free_context(fmt_ctx);

	fclose(fp_rgb);
}

bool vrmoviedecoder::vr_movie_decoder_open_file(const char* path, int* perror)
{
	vrMovieError errCode = vrMovieErrorNone;

	_path = (char*)path;
	errCode = vr_movie_decoder_open_input(path);
	if (errCode == vrMovieErrorNone)
	{
		/* 4. 寻找并打开解码器 */
		vrMovieError videoErr = vr_movie_decoder_open_video_stream();
		vrMovieError audioErr = vr_movie_decoder_open_audio_stream();
		if (videoErr != vrMovieErrorNone &&
			audioErr != vrMovieErrorNone) {

			errCode = videoErr; // both fails

		}
	}

	if (errCode != vrMovieErrorNone) {
		return false;
	}

	return true;
}

bool vrmoviedecoder::vr_movie_decoder_open_screen(bool save)
{
	//分配一个AVFormatContext
	fmt_ctx = avformat_alloc_context();
	if (!fmt_ctx)
		return false;

	//Register Device  
	avdevice_register_all();

	/* 2. 打开流 */
	//Use gdigrab  
	AVDictionary* options = NULL;
	//Set some options  
	//grabbing frame rate  
	av_dict_set(&options,"framerate","60",0);  
	//The distance from the left edge of the screen or desktop  
	av_dict_set(&options,"offset_x","20",0);  
	//The distance from the top edge of the screen or desktop  
	av_dict_set(&options,"offset_y","40",0);  
	//Video frame size. The default is to capture the full screen  
	av_dict_set(&options,"video_size","640x480",0);  
	AVInputFormat* ifmt = av_find_input_format("gdigrab");
	if (avformat_open_input(&fmt_ctx, "desktop", ifmt, &options) != 0) {
		printf("Couldn't open input stream.\n");
		return false;
	}

	/* 3. 获取流的信息 */
	if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
		std::cout << "failed to get stream info" << std::endl;
		avformat_close_input(&fmt_ctx);
		return vrMovieErrorStreamInfoNotFound;
	}


	/* 4. 寻找并打开解码器 */
	vrMovieError videoErr = vr_movie_decoder_open_video_stream();

	if (save) {
		fp_rgb = fopen("output.rgb", "wb+");
		save_file = save;
	}
	return true;
}

void vrmoviedecoder::vr_movie_decoder_init_avframe_rgb24()
{
	// allocate the video frames
	av_frame = av_frame_alloc();
	gl_frame = av_frame_alloc();

	// ffmpeg会计算这个格式的图片，需要多少字节来存储 AV_PIX_FMT_RGB24: width * height * 3
	int size = avpicture_get_size(AV_PIX_FMT_RGB24, video_codec_ctx->width, video_codec_ctx->height);

	// 申请空间来存放图片数据，包含源数据和目标数据
	uint8_t* internal_buffer = (uint8_t*)av_malloc(size * sizeof(uint8_t));

	//前面的av_frame_alloc函数，只是为这个AVFrame结构体分配了内存，而该类型的data指针指向的内存还没分配。  
	//这里把av_malloc得到的内存和AVFrame关联起来。当然，其还会设置AVFrame的其他成员
	avpicture_fill((AVPicture*)gl_frame, internal_buffer, AV_PIX_FMT_RGB24, video_codec_ctx->width, video_codec_ctx->height);
	packet = (AVPacket*)av_malloc(sizeof(AVPacket));
}

int vrmoviedecoder::vr_movie_decoder_width()
{
	return video_codec_ctx->width;
}

int vrmoviedecoder::vr_movie_decoder_height()
{
	return video_codec_ctx->height;
}

bool vrmoviedecoder::vr_movie_decoder_read_frame()
{
	int ret = 0;
	do
	{
		/* 5. 读流数据, 未解码的数据存放于packet */
		if (av_read_frame(fmt_ctx, packet) < 0)
		{
			av_free_packet(packet);
			return false;
		}
		/* 6. 解码, 解码后的数据存放于 video_frame */
		/* 视频解码 */
		if (packet->stream_index == video_streams_idx)
		{
			if (avcodec_decode_video2(video_codec_ctx, av_frame, &ret, packet) < 0)
			{
				av_free_packet(packet);
				return false;
			}

			if (0 == ret)
			{
				std::cout << "video decodec error!" << std::endl;
				continue;
			}
			
			if (!sws_ctx)
			{
				sws_ctx = sws_getContext(video_codec_ctx->width,
					video_codec_ctx->height, video_codec_ctx->pix_fmt,
					video_codec_ctx->width, video_codec_ctx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
			}

			sws_scale(sws_ctx, av_frame->data, av_frame->linesize, 0,
				video_codec_ctx->height, gl_frame->data, gl_frame->linesize);

			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_codec_ctx->width,
				video_codec_ctx->height, GL_RGB, GL_UNSIGNED_BYTE,
				gl_frame->data[0]);
			
			if (save_file)
			{
				int y_size = video_codec_ctx->width * video_codec_ctx->height*3;
				fwrite(gl_frame->data[0], 1, y_size, fp_rgb);   
			}
		}

		/* 音频解码 */
		if (packet->stream_index == audio_streams_idx)
		{
			if (avcodec_decode_audio4(audio_codec_ctx, av_frame, &ret, packet) < 0)
			{
				av_free_packet(packet);
				return false;
			}
		}

		av_free_packet(packet);
	} while (packet->stream_index != video_streams_idx);

	return true;
}

vrMovieError vrmoviedecoder::vr_movie_decoder_open_input(const char* path)
{
	//分配一个AVFormatContext
	fmt_ctx = avformat_alloc_context();
	if (!fmt_ctx)
		return vrMovieErrorOpenFile;

	/* 2. 打开流 */
	if (avformat_open_input(&fmt_ctx, path, nullptr, nullptr) < 0) {
		std::cout << "failed to open input" << std::endl;
		if (fmt_ctx)
			avformat_free_context(fmt_ctx);
		return vrMovieErrorOpenFile;
	}

	/* 3. 获取流的信息 */
	if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
		std::cout << "failed to get stream info" << std::endl;
		avformat_close_input(&fmt_ctx);
		return vrMovieErrorStreamInfoNotFound;
	}

	//打印媒体信息
	av_dump_format(fmt_ctx, 0, path, 0);

	return vrMovieErrorNone;
}

vrMovieError vrmoviedecoder::vr_movie_decoder_open_video_stream()
{
	vrMovieError errCode = vrMovieErrorStreamNotFound;

	//查找video stream idx
	video_streams_idx = vr_movie_decoder_collect_streams_idx(fmt_ctx, AVMEDIA_TYPE_VIDEO);
	if (video_streams_idx == -1)
	{
		std::cout << "failed to find video stream" << std::endl;
		return errCode;
	}

	if (0 == (fmt_ctx->streams[video_streams_idx]->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
		errCode = vr_movie_decoder_open_video_stream_by_idx();
	}

	return errCode;
}

vrMovieError vrmoviedecoder::vr_movie_decoder_open_audio_stream()
{
	vrMovieError errCode = vrMovieErrorStreamNotFound;
	audio_streams_idx = vr_movie_decoder_collect_streams_idx(fmt_ctx, AVMEDIA_TYPE_AUDIO);
	if (audio_streams_idx == -1)
	{
		std::cout << "failed to find audio stream" << std::endl;
		return vrMovieErrorStreamNotFound;
	}
	errCode = vr_movie_decoder_open_audio_stream_by_idx();
	return errCode;
}

vrMovieError vrmoviedecoder::vr_movie_decoder_open_video_stream_by_idx()
{
	// get a pointer to the codec context for the video stream
	video_stream = fmt_ctx->streams[video_streams_idx];
	video_codec_ctx = video_stream->codec;

	// find the decoder for the video stream
	video_decoder = avcodec_find_decoder(video_codec_ctx->codec_id);
	if (!video_decoder) 
	{
		std::cout << "failed to find video decoder" << std::endl;
		return vrMovieErrorCodecNotFound;
	}
	
	// open the decoder
	if (avcodec_open2(video_codec_ctx, video_decoder, nullptr) < 0)
	{
		std::cout << "failed to open video decoder" << std::endl;
		return vrMovieErrorOpenCodec;
	}

	return vrMovieErrorNone;
}

vrMovieError vrmoviedecoder::vr_movie_decoder_open_audio_stream_by_idx()
{
	audio_stream = fmt_ctx->streams[audio_streams_idx];
	audio_codec_ctx = audio_stream->codec;

	// find the decoder for the audio stream
	audio_decoder = avcodec_find_decoder(audio_codec_ctx->codec_id);
	if (!audio_decoder) 
	{
		std::cout << "failed to find audio decoder" << std::endl;
		return vrMovieErrorCodecNotFound;
	}
		
	// open the decoder
	if (avcodec_open2(audio_codec_ctx, audio_decoder, nullptr) < 0)
	{
		std::cout << "failed to open audio decoder" << std::endl;
		return vrMovieErrorOpenCodec;
	}
	return vrMovieErrorNone;
}

unsigned int vrmoviedecoder::vr_movie_decoder_collect_streams_idx(AVFormatContext* fmt_ctx, AVMediaType codec_type)
{
	// 获取流下标
	int idx = -1;
	for (unsigned int i = 0; i < fmt_ctx->nb_streams; ++i)
	{
		if (fmt_ctx->streams[i]->codec->codec_type == codec_type)
		{
			idx = i;
			break;
		}
	}
	return idx;
}
