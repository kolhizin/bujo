#pragma once
#include <algorithm>
#include <exception>

namespace bujo
{
	namespace util
	{
		template<class T, class It>
		T calculateQuantile(const It& begin, const It& end, double quantile, T* buffer, size_t buffer_size)
		{
			size_t cnt = 0;
			for (auto it = begin; it != end; ++it)
			{
				if (cnt >= buffer_size)
					throw std::runtime_error("Overflow of buffer in calculateQuantile()!");
				buffer[cnt] = *it;
				cnt++;
			}
			std::sort(buffer, buffer + cnt);
			int qid = static_cast<int>(std::floor(quantile * cnt));
			return buffer[std::min(qid, static_cast<int>(buffer_size - 1))];
		}
	}
}