#pragma once

#include "IResource.hpp"

namespace Resources
{
	class NAT_API SkinnedModel : public IResource
	{
	public:
		SkinnedModel();
		void DeleteData() override;
		void WindowCreateResource(bool& open) override;
		~SkinnedModel() override;

	private:

	};

}