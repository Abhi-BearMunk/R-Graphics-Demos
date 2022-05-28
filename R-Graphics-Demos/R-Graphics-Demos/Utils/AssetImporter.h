#pragma once
#include "pch.h"
#include "Rendering/GraphicsUtils.h"
#include "JobSystem/JobSystem.h"
#include "Rendering/ResourceDatas.h"
namespace R
{
	namespace Utils
	{
		struct TextureAssetDesc
		{
			enum AssetType
			{
				E_Albedo,
				E_Normal,
				E_Specular,
				E_AssetTypeCount
			};
			AssetType type;
			const wchar_t* fileName;
			std::uint32_t numMips = 1;
			Rendering::TextureID* texId;
		};
		class AssetImporter
		{
		public:
			AssetImporter(Job::JobSystem& jobSystem);
			// TODO : JOBIFY THIS!
			void ImportTextures(std::uint32_t numAssets, TextureAssetDesc* pAssetDescs);
			inline const Rendering::ResourceData* GetResourceData() const { return &m_resourceData; }
		private:
			Job::JobSystem* m_pjobSystem;
			Rendering::ResourceData m_resourceData;

			void ImportTexture(TextureAssetDesc* assetDesc);
		};
	}
}