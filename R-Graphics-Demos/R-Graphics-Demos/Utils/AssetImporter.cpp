#include "pch.h"
#include "AssetImporter.h"

R::Utils::AssetImporter::AssetImporter(Job::JobSystem& jobSystem)
	: m_pjobSystem(&jobSystem) 
{
	R::Rendering::LogErrorIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED), "Failed to Init Coinitialize");
}

void R::Utils::AssetImporter::ImportTextures(std::uint32_t numAssets, TextureAssetDesc* pAssetDescs)
{
	TextureAssetDesc* current = nullptr;
	for (std::uint32_t i = 0; i < numAssets; i++)
	{
		ImportTexture(&pAssetDescs[i]);
	}
}

void R::Utils::AssetImporter::ImportTexture(TextureAssetDesc* assetDesc)
{
	m_resourceData.textureDatas.emplace_back(assetDesc->texId);
	ScratchImage scratcImageBase;
	R::Rendering::LogErrorIfFailed(LoadFromWICFile(assetDesc->fileName, WIC_FLAGS_NONE, nullptr, scratcImageBase), "Failed to load texture");
	R::Rendering::LogErrorIfFailed(GenerateMipMaps(scratcImageBase.GetImages()[0], TEX_FILTER_FANT, 4, m_resourceData.textureDatas.back().scratchImage), "Failed to generate mipmaps");
}
