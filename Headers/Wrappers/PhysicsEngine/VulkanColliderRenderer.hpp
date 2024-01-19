#pragma once


#include <mutex>
#include <string_view>

#include "Maths/Maths.hpp"

#include "Renderer/VulkanRenderer.hpp"
#include "Renderer/RendererMesh.hpp"
#include "Renderer/RendererVertex.hpp"

#include "Jolt/Jolt.h"

#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#else
	#define JPH_DEBUG_RENDERER
	#include "Jolt/Renderer/DebugRenderer.h"
	#undef JPH_DEBUG_RENDERER
#endif

#define COLLIDER_DRAW_COLOR Maths::Vec3(1,0,0) //Red

namespace Wrappers::PhysicsEngine
{
	class NAT_API VulkanColliderRenderer : public JPH::DebugRenderer
	{
		class NAT_API BatchImpl : public Renderer::RendererMesh, public JPH::RefTargetVirtual, public JPH::RefTarget<BatchImpl>
		{
		public:
			JPH_OVERRIDE_NEW_DELETE

				BatchImpl(Renderer::VulkanRenderer* pRenderer) : mRenderer(pRenderer) {};
			
				void CreateVertexBuffer(const Vertex* pVertices, const u32& pVerticesCount) 
				{
					std::vector<Renderer::RendererVertex> vertices;
					vertices.reserve(pVerticesCount);
					
					mVertexCount = pVerticesCount;

					for (unsigned int i = 0 ; i < pVerticesCount ; i++)
					{
						Renderer::RendererVertex rendererVertex;

						rendererVertex.pos		= Maths::Vec3(pVertices[i].mPosition);
						rendererVertex.normal	= Maths::Vec3(pVertices[i].mNormal);
						rendererVertex.uv		= Maths::Vec2(pVertices[i].mUV);
						rendererVertex.color	= COLLIDER_DRAW_COLOR;

						vertices.push_back(rendererVertex);
					}

					this->vertexBuffer = mRenderer->CreateVertexBuffer(vertices.data(), pVerticesCount);
				};

				void CreateIndiceBuffer(const u32* pIndices, const u32& pIndicesCount)
				{
					mIndiceCount = pIndicesCount;
					this->indexBuffer = mRenderer->CreateIndiceBuffer(pIndices, pIndicesCount);
				}

				virtual void					AddRef() override { JPH::RefTarget<BatchImpl>::AddRef(); }
				
				virtual void					Release() override
				{
					if (--mRefCount == 0)
					{
						delete this;
					}
				}

				u32 GetIndiceCount() { return mIndiceCount; }
				u32 GetVertexCount() { return mVertexCount; }

		private :
				u32 mIndiceCount = 0;
				u32 mVertexCount = 0;
				Renderer::VulkanRenderer* mRenderer = nullptr;
		};

	public:
		VulkanColliderRenderer()  = default;
		~VulkanColliderRenderer() = default;

		void Init(Renderer::VulkanRenderer* pRenderer);
		void Render();
		void Release();
		
		using uint32 = uint32_t; //Hack

		virtual void	DrawLine(const JPH::Float3& inFrom, const JPH::Float3& inTo,JPH::ColorArg inColor) override;
		virtual void	DrawTriangle(JPH::Vec3Arg inV1, JPH::Vec3Arg inV2, JPH::Vec3Arg inV3, JPH::ColorArg inColor) override;
		virtual Batch	CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;
		virtual Batch	CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const uint32* inIndices, int inIndexCount) override;
		virtual void	DrawGeometry(JPH::Mat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;
		virtual void	DrawText3D(JPH::Vec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) override;

		void ClearDrawList();

	private :
		
		void DrawLines();
		void DrawTriangles();

		void CleanBuffers();

		void CleanLines();
		void CleanTriangles();

	private :
		std::vector<BatchImpl*> mBatches; //To manage the destruction of vertex/indice buffers

		struct Line
		{
			JPH::Float3 start;
			JPH::Float3 end;
			JPH::Color  color;
		};

		Resources::Mesh* mLines = nullptr;
		std::mutex		mLinesLock;

		Resources::Mesh mTriangles;
		std::mutex		mTrianglesLock;

		Batch mEmptyBatch;

		struct InstanceInfo
		{
			u32 mLOD = 0;
			JPH::Mat44 mModelMatrix;
		};

		using InstanceMap = std::unordered_map<GeometryRef, std::vector<InstanceInfo>>;
		
		std::mutex	mWireFrameLock;
		InstanceMap mWireframeInstances;

		Renderer::VulkanRenderer* mRenderer = nullptr;
	};
}