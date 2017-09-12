#include "ConversionYUV.hpp"
#include "Profile.hpp"
#include "SDR Shared\Log.hpp"

namespace
{
	namespace LocalProfiling
	{
		int PushYUV;

		SDR::PluginStartupFunctionAdder A1("YUV profiling", []()
		{
			PushYUV = SDR::Profile::RegisterProfiling("PushYUV");
		});
	}
}

void SDR::D3D11::ConversionYUV::Create(ID3D11Device* device, AVFrame* reference, bool staging)
{
	auto sizey = reference->buf[0]->size;
	auto sizeu = reference->buf[1]->size;
	auto sizev = reference->buf[2]->size;

	Y.Create(device, DXGI_FORMAT_R8_UINT, sizey, sizey, staging);
	U.Create(device, DXGI_FORMAT_R8_UINT, sizeu, sizeu, staging);
	V.Create(device, DXGI_FORMAT_R8_UINT, sizev, sizev, staging);

	/*
		Matches YUVInputData in YUVShared.hlsl.
	*/
	__declspec(align(16)) struct
	{
		int Strides[3];
		int Padding1;
		float CoeffY[3];
		int Padding2;
		float CoeffU[3];
		int Padding3;
		float CoeffV[3];
	} yuvdata;

	yuvdata.Strides[0] = reference->linesize[0];
	yuvdata.Strides[1] = reference->linesize[1];
	yuvdata.Strides[2] = reference->linesize[2];

	auto setcoeffs = [](auto& obj, float x, float y, float z)
	{
		obj[0] = x;
		obj[1] = y;
		obj[2] = z;
	};

	/*
		https://msdn.microsoft.com/en-us/library/windows/desktop/ms698715.aspx
	*/

	if (reference->colorspace == AVCOL_SPC_BT470BG)
	{
		setcoeffs(yuvdata.CoeffY, +0.299000, +0.587000, +0.114000);
		setcoeffs(yuvdata.CoeffU, -0.168736, -0.331264, +0.500000);
		setcoeffs(yuvdata.CoeffV, +0.500000, -0.418688, -0.081312);
	}

	else if (reference->colorspace == AVCOL_SPC_BT709)
	{
		setcoeffs(yuvdata.CoeffY, +0.212600, +0.715200, +0.072200);
		setcoeffs(yuvdata.CoeffU, -0.114572, -0.385428, +0.500000);
		setcoeffs(yuvdata.CoeffV, +0.500000, -0.454153, -0.045847);
	}

	else
	{
		Error::Make("No matching YUV color space for coefficients"s);
	}

	D3D11_BUFFER_DESC cbufdesc = {};
	cbufdesc.ByteWidth = sizeof(yuvdata);
	cbufdesc.Usage = D3D11_USAGE_IMMUTABLE;
	cbufdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	D3D11_SUBRESOURCE_DATA cbufsubdesc = {};
	cbufsubdesc.pSysMem = &yuvdata;

	Error::MS::ThrowIfFailed
	(
		device->CreateBuffer(&cbufdesc, &cbufsubdesc, ConstantBuffer.GetAddressOf()),
		"Could not create constant buffer for YUV GPU buffer"
	);
}

void SDR::D3D11::ConversionYUV::DynamicBind(ID3D11DeviceContext* context)
{
	auto cbufs = { ConstantBuffer.Get() };
	context->CSSetConstantBuffers(1, 1, cbufs.begin());

	auto uavs = { Y.View.Get(), U.View.Get(), V.View.Get() };
	context->CSSetUnorderedAccessViews(0, 3, uavs.begin(), nullptr);
}

bool SDR::D3D11::ConversionYUV::Download(ID3D11DeviceContext* context, Stream::FutureData& item)
{
	Profile::ScopedEntry e1(LocalProfiling::PushYUV);

	D3D11_MAPPED_SUBRESOURCE mappedy;
	D3D11_MAPPED_SUBRESOURCE mappedu;
	D3D11_MAPPED_SUBRESOURCE mappedv;

	auto hrs =
	{
		Y.Map(context, &mappedy),
		U.Map(context, &mappedu),
		V.Map(context, &mappedv)
	};

	bool pass = true;

	for (auto res : hrs)
	{
		if (FAILED(res))
		{
			pass = false;

			Log::Warning("SDR: Could not map DX11 YUV buffers\n"s);
			break;
		}
	}

	if (pass)
	{
		auto ptry = (uint8_t*)mappedy.pData;
		auto ptru = (uint8_t*)mappedu.pData;
		auto ptrv = (uint8_t*)mappedv.pData;

		item.Planes[0].assign(ptry, ptry + Y.Size);
		item.Planes[1].assign(ptru, ptru + U.Size);
		item.Planes[2].assign(ptrv, ptrv + V.Size);
	}

	Y.Unmap(context);
	U.Unmap(context);
	V.Unmap(context);

	return pass;
}
