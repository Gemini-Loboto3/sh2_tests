#include "criware.h"

int OpenADX(const char* filename, ADXStream** obj)
{
	*obj = nullptr;

	ADXStream* adx = new ADXStream;

	if (FAILED(adx->Open(filename)))
	{
		delete adx;
		return 0;
	}

	ADX_headerV3 head;
	adx->Read(&head, sizeof(head));

	if (head.magic.w() != 0x8000)
	{
		adx->Close();
		delete adx;
		return 0;
	}

	if (head.version == 3)
	{
		adx->block_size = head.block_size;
		adx->channel_count = head.channel_count;
		adx->highpass_frequency = head.highpass_frequency.w();
		adx->loop_enabled = head.loop_enabled.w();
		adx->loop_end_index = head.loop_sample_end.dw();
		adx->loop_start_index = head.loop_sample_begin.dw();
		adx->sample_bitdepth = head.sample_bitdepth;
		adx->sample_rate = head.sample_rate.dw();
		adx->total_samples = head.total_samples.dw();
		adx->copyright_offset = head.copyright_offset.w();
	}
	else if (head.version == 4)
	{
		ADX_headerV4* head4 = (ADX_headerV4*)&head;

		adx->block_size = head4->block_size;
		adx->channel_count = head4->channel_count;
		adx->highpass_frequency = head4->highpass_frequency.w();
		adx->loop_enabled = head4->loop_enabled.dw();
		adx->loop_end_index = head4->loop_sample_end.dw();
		adx->loop_start_index = head4->loop_sample_begin.dw();
		adx->sample_bitdepth = head4->sample_bitdepth;
		adx->sample_rate = head4->sample_rate.dw();
		adx->total_samples = head4->total_samples.dw();
		adx->copyright_offset = head4->copyright_offset.w();
	}
	else
	{
		adx->Close();
		delete adx;
		return 0;
	}

	adx_set_coeff(adx);

	*obj = adx;

	return 1;
}

void CloseADX(ADX* obj)
{
	ADXStream* adx = (ADXStream*)obj;
	adx->Close();

	delete adx;
}