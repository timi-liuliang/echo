#pragma once

namespace Lightmass
{
	/**
	 * Send complete lighting data to LORD
	 *
	 * @param LightingData - Object containing the computed data
	 */
	void ExportResultsToLORDLightMgr( const char* UniqueName, void* colors, int width, int height, const float* scale);

	/**
	 * Save LightmapMgr
	 */
	void LORDSaveLightMgr(int num);
}