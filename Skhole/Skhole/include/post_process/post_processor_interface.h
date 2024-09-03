#pragma once
#include <include.h>
#include <post_process/post_processor.h>
#include <post_process/example.h>

namespace Skhole{
	inline ShrPtr<PostProcessor> GetPostProcessor(PostProcessType type)
	{
		switch (type) {
		case PostProcessType::Example:
			return std::make_shared<PPExample>();
			break;
		default:
			SKHOLE_UNIMPL();
			break;
		}
	}
}
