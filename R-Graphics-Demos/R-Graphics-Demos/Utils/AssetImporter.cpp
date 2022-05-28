#include "pch.h"
#include "AssetImporter.h"

R::Utils::AssetImporter::AssetImporter(Job::JobSystem& jobSystem)
	: m_pjobSystem(&jobSystem) 
{
	R::Rendering::LogErrorIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED), "Failed to Init Coinitialize");
}

void R::Utils::AssetImporter::ImportTextures(std::uint32_t numAssets, TextureAssetDesc* pAssetDescs)
{
	TextureAssetDesc* current;
	for (std::uint32_t i = 0; i < numAssets; i++)
	{
		ImportTexture(&pAssetDescs[i]);
	}
}

void R::Utils::AssetImporter::ImportTexture(TextureAssetDesc* assetDesc)
{
	m_resourceData.textureDatas.emplace_back(assetDesc->texId);
	R::Rendering::LogErrorIfFailed(LoadFromWICFile(assetDesc->fileName, WIC_FLAGS_NONE, nullptr, m_resourceData.textureDatas.back().scratchImage), "Failed to load texture");
}
