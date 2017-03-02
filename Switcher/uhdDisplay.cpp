// Copyright(c) 2016, NVIDIA CORPORATION.All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met :
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and / or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nvapi.h"


#include "uhdDisplay.h"



static bool first = true;

// Walk all the monitors and apply the requested HDR settings
void SetHdrMonitorMode(bool enableHDR)
{
	if (first)
	{
		NvAPI_Initialize();
		first = false;
	}

	NvAPI_Status nvStatus = NVAPI_OK;
	NvDisplayHandle hNvDisplay = NULL;

	// get first display handle which should work for all NVAPI calls for all GPUs
	if ((nvStatus = NvAPI_EnumNvidiaDisplayHandle(0, &hNvDisplay)) != NVAPI_OK)
	{
		printf("NvAPI_EnumNvidiaDisplayHandle returned error code %d\r\n", nvStatus);

		return;
	}

	NvU32 gpuCount = 0;
	NvU32 maxDisplayIndex = 0;
	NvPhysicalGpuHandle ahGPU[NVAPI_MAX_PHYSICAL_GPUS] = {};

	// get the list of displays connected, populate the dynamic components
	nvStatus = NvAPI_EnumPhysicalGPUs(ahGPU, &gpuCount);

	if (NVAPI_OK != nvStatus)
	{
		printf("NvAPI_EnumPhysicalGPUs returned error code %d\r\n", nvStatus);

		return;
	}

	for (NvU32 i = 0; i < gpuCount; ++i)
	{
		NvU32 displayIdCount = 16;
		NvU32 flags = 0;
		NV_GPU_DISPLAYIDS displayIdArray[16] = {};
		displayIdArray[0].version = NV_GPU_DISPLAYIDS_VER;

		nvStatus = NvAPI_GPU_GetConnectedDisplayIds(ahGPU[i], displayIdArray, &displayIdCount, flags);

		if (NVAPI_OK == nvStatus)
		{
			printf("Display count %d\r\n", displayIdCount);

			for (maxDisplayIndex = 0; maxDisplayIndex < displayIdCount; ++maxDisplayIndex)
			{
				printf("Display tested %d\r\n", maxDisplayIndex);

				NV_HDR_CAPABILITIES hdrCapabilities = {};

				hdrCapabilities.version = NV_HDR_CAPABILITIES_VER;

				if (NVAPI_OK == NvAPI_Disp_GetHdrCapabilities(displayIdArray[maxDisplayIndex].displayId, &hdrCapabilities))
				{
					if (hdrCapabilities.isST2084EotfSupported)
					{
						printf("Display %d supports ST2084 EOTF\r\n", maxDisplayIndex);

						NV_HDR_COLOR_DATA hdrColorData = {};

						memset(&hdrColorData, 0, sizeof(hdrColorData));

						hdrColorData.version = NV_HDR_COLOR_DATA_VER;
						hdrColorData.cmd = NV_HDR_CMD_SET;
						hdrColorData.static_metadata_descriptor_id = NV_STATIC_METADATA_TYPE_1;

						hdrColorData.hdrMode = enableHDR ? NV_HDR_MODE_UHDBD : NV_HDR_MODE_OFF;


						nvStatus = NvAPI_Disp_HdrColorControl(displayIdArray[maxDisplayIndex].displayId, &hdrColorData);

						if (NVAPI_OK == nvStatus)
						{
							printf("NvAPI_Disp_SethdrColorData call has succeeded: ");
						}
						else
						{
							NvAPI_ShortString szDesc;
							NvAPI_GetErrorMessage(nvStatus, szDesc);
							printf("NvAPI_Disp_HdrColorControl returned %s (%x)\r\n", szDesc, nvStatus);
						}
					}
				}
				else
				{
					NvAPI_ShortString szDesc;
					NvAPI_GetErrorMessage(nvStatus, szDesc);
					printf("NvAPI_Disp_GetHdrCapabilities returned %s (%x)\r\n", szDesc, nvStatus);
				}
			}
		}
		else
		{
			NvAPI_ShortString szDesc;
			NvAPI_GetErrorMessage(nvStatus, szDesc);
			printf("NvAPI_GPU_GetConnectedDisplayIds returned %s (%x)\r\n", szDesc, nvStatus);
		}
	}

}