// KinectProject.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <Windows.h>
#include <NuiApi.h>

using namespace std;
int main()
{
	HRESULT hr;
	NUI_IMAGE_FRAME depthFrame;
	HANDLE hDepth;
	INuiSensor* pNuiSensor = NULL;
	int iSensorCount = 0;
	hr = NuiGetSensorCount(&iSensorCount);

	if (FAILED(hr))
		return hr;

	for (int i = 0; i < iSensorCount; i++)
	{
		INuiSensor* tempSensor;
		hr = NuiCreateSensorByIndex(i, &tempSensor);

		if (FAILED(hr))
			continue;

		hr = tempSensor->NuiStatus();
		if (S_OK == hr)
		{
			pNuiSensor = tempSensor;
			break;
		}

		tempSensor->Release();
	}

	pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH);
	pNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH,
		NUI_IMAGE_RESOLUTION_320x240,
		0,
		2,
		NULL,
		&hDepth);
		
	while (1)
	{
		hr = pNuiSensor->NuiImageStreamGetNextFrame(hDepth, 0, &depthFrame);
		if (FAILED(hr))
			continue;

		/*
		for (int y = 0; y < 240; y++)
		{
			for (int x = 0; x < 320; x++)
			{
				USHORT depth = ((x + y) & 0xfff8) >> 3;
				cout << depth << endl;
			}
		}*/

		INuiFrameTexture* pTexture;
		NUI_LOCKED_RECT LockedRect;
		
		hr = pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
			hDepth, &depthFrame, false, &pTexture);

		if (FAILED(hr))
		{
			pNuiSensor->NuiImageStreamReleaseFrame(hDepth, &depthFrame);
			continue;
		}
			

		pTexture->LockRect(0, &LockedRect, NULL, 0);

		if (LockedRect.Pitch != 0)
		{
			const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits) + 120 + 160;

			float depth = (float)(pBufferRun->depth);

			cout << "Depth Of " << depth << endl;
		}

		
		pTexture->UnlockRect(0);
		pTexture->Release();

		pNuiSensor->NuiImageStreamReleaseFrame(hDepth, &depthFrame);
	}


	NuiShutdown();
	return 0;
}