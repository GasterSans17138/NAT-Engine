#include "Core/Debugging/Log.hpp"

#include "Renderer/RendererMesh.hpp"

#include "Wrappers/PhysicsEngine/VulkanColliderRenderer.hpp"
#include "Core/App.hpp"

#include "Core/App.hpp"

namespace Wrappers::PhysicsEngine
{
	void						VulkanColliderRenderer::Init(Renderer::VulkanRenderer* pRenderer) 
	{
		mRenderer = pRenderer;

		Initialize(); //JPH::DebugRenderer::Initialize();

		Vertex vertex = { JPH::Float3(0, 0, 0), JPH::Float3(1, 0, 0), JPH::Float2(0, 0), JPH::Color::sWhite };
		u32 indices[] = { 0,1,2 };
		mEmptyBatch = CreateTriangleBatch(&vertex, 1, indices, 3);
		mLines = Core::App::GetInstance()->GetResources().CreateFixedResource<Resources::Mesh>("line_mesh", 0xf, false);
	}

	void						VulkanColliderRenderer::Render()
	{
		//Vulkan part here
		DrawTriangles(); 
		Resources::ShaderProgram* wire = Core::App::GetInstance()->GetResources().Get<Resources::ShaderProgram>(0x6c);
		Core::App::GetInstance()->GetRenderer().BindShader(wire);
		DrawLines();

		CleanBuffers();
	}

	void						VulkanColliderRenderer::Release()
	{
		for (BatchImpl* batch : mBatches)
		{
			mRenderer->FreeVertexBuffer(batch->vertexBuffer);

			if (batch->GetIndiceCount() > 0)
				mRenderer->FreeIndiceBuffer(batch->indexBuffer);
		}
	}

	///Realisticly, this will most likely never be called, it looks like jolt prefer to use triagnle batches for almost everything
	void						VulkanColliderRenderer::DrawLine(const JPH::Float3& inFrom, const JPH::Float3& inTo, JPH::ColorArg inColor)
	{
		std::lock_guard lock(mLinesLock);
		
		Renderer::RendererVertex vertex;

		vertex.color	= inColor;
		vertex.pos		= Maths::Vec3(inFrom.x,  inFrom.y,  inFrom.z);

		mLines->vertices.push_back(vertex);
		mLines->indices.push_back(mLines->indices.size());

		vertex.pos		= Maths::Vec3(inTo.x, inTo.y, inTo.z);

		mLines->vertices.push_back(vertex);
		mLines->indices.push_back(mLines->indices.size());
	}

	///Realisticly, this will most likely never be called, it looks like jolt prefer to use triagnle batches for almost everything
	void						VulkanColliderRenderer::DrawTriangle(JPH::Vec3Arg inV1, JPH::Vec3Arg inV2, JPH::Vec3Arg inV3, JPH::ColorArg inColor)
	{
		std::lock_guard lock(mTrianglesLock);
		
		Renderer::RendererVertex vertex;

		vertex.color = inColor;

		vertex.pos = Maths::Vec3(inV1.GetX(), inV1.GetY(), inV1.GetZ());

		mTriangles.vertices.push_back(vertex);

		vertex.pos = Maths::Vec3(inV2.GetX(), inV2.GetY(), inV2.GetZ());

		mTriangles.vertices.push_back(vertex);

		vertex.pos = Maths::Vec3(inV3.GetX(), inV3.GetY(), inV3.GetZ());

		mTriangles.vertices.push_back(vertex);
	}

	JPH::DebugRenderer::Batch	VulkanColliderRenderer::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
	{
		if (inTriangles == nullptr || inTriangleCount == 0)
			return mEmptyBatch;
		
		BatchImpl* mesh = new BatchImpl(this->mRenderer);

		mesh->CreateVertexBuffer(inTriangles->mV, inTriangleCount*3);

		mBatches.push_back(mesh);

		return mesh;
	}

	JPH::DebugRenderer::Batch	VulkanColliderRenderer::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const uint32* inIndices, int inIndexCount)
	{
		if (inVertices == nullptr || inVertexCount == 0 || inIndices == nullptr || inIndexCount == 0)
			return mEmptyBatch;

		BatchImpl* mesh = new BatchImpl(this->mRenderer);

		mesh->CreateVertexBuffer(inVertices, inVertexCount);
		mesh->CreateIndiceBuffer(inIndices, inIndexCount);

		mBatches.push_back(mesh);

		return mesh;
	}

	void						VulkanColliderRenderer::DrawGeometry(JPH::Mat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
	{
		if (inDrawMode == EDrawMode::Wireframe) //Should always be true tbh
		{
			std::lock_guard	lock(mWireFrameLock);

			mWireframeInstances[inGeometry].push_back({ 0, inModelMatrix });
		}
	}

	void						VulkanColliderRenderer::DrawText3D(JPH::Vec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight)
	{
		//Empty, we don't need that.
	}

	void VulkanColliderRenderer::ClearDrawList()
	{
		CleanBuffers();
	}

	void						VulkanColliderRenderer::DrawLines()
	{
		std::lock_guard lock(mLinesLock);
		mLines->UpdateVectors();
		if (mLines->GetVertexCount() > 0)
		{	
			if (mLines->isLoaded)
			{
				mRenderer->UnloadMesh(mLines);
				mLines->isLoaded = false;
			}
			mLines->isLoaded = mRenderer->LoadMesh(mLines);
			if (mLines->isLoaded) mRenderer->DrawIndexedRenderMesh(&mLines->rendererMesh, Maths::Mat4::Identity(), mLines->GetVertexCount(), mLines->GetIndexCount());

		}
	}

	void						VulkanColliderRenderer::DrawTriangles()
	{	
		std::lock_guard	lock(mWireFrameLock);

		for (InstanceMap::value_type instance : mWireframeInstances)
		{
			Geometry* geometry = instance.first.GetPtr();
			for (InstanceInfo& infos : instance.second)
			{
				BatchImpl* triangleBatch = static_cast<BatchImpl*>(geometry->mLODs[infos.mLOD].mTriangleBatch.GetPtr());
				mRenderer->DrawIndexedRenderMesh(triangleBatch, infos.mModelMatrix, triangleBatch->GetVertexCount(), triangleBatch->GetIndiceCount());
			}
		}

		std::lock_guard triangleLock(mTrianglesLock);

		if (mTriangles.GetVertexCount() > 0)
		{

			mRenderer->LoadMesh(&mTriangles);

			mRenderer->DrawIndexedRenderMesh(&mTriangles.rendererMesh, Maths::Mat4::Identity(), mTriangles.GetVertexCount(), mTriangles.GetIndexCount() );

			mRenderer->UnloadMesh(&mTriangles);
		}
	}

	void						VulkanColliderRenderer::CleanBuffers()
	{
		CleanLines();
		CleanTriangles();

		std::lock_guard lock(mWireFrameLock);
		mWireframeInstances.clear();
	}

	void						VulkanColliderRenderer::CleanLines()
	{
		std::lock_guard lock(mLinesLock);

		mLines->vertices.clear();
		mLines->indices.clear();
	}

	void						VulkanColliderRenderer::CleanTriangles()
	{
		std::lock_guard lock(mTrianglesLock);

		mTriangles.vertices.clear();
	}
}