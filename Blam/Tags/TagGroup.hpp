#pragma once
#include "Tag.hpp"
#include "../../../Precomp.hpp"

namespace Blam::Tags
{
	template <const Tag GroupTagValue>
	struct TagGroup
	{
		static const Tag GroupTag = GroupTagValue;
		static const Definitions::StructDefinition Definition;
	};
}
