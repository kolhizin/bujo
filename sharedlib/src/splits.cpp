#include "splits.h"
#include "radon.h"
#include "extremum.h"
#include "util/quantiles.h"
#include <xtensor/xview.hpp>

using namespace bujo::splits;

/*
def search_region_vsplit(src, min_angle, num_angles,
						 window_size_1d, max_value, zero_threshold, min_split_abs, max_split_pct):
	"""Find near-vertical splits that remove irrelevant regions. Returns list of splits.

	Keyword arguments:
	min_angle -- minimal angle to consider (closer to pi/2 closer to vertical)
	num_angles -- num angles of Radon transform
	window_size_1d -- width of window to be global minima inside for split search
	max_value -- cutoff value for local minima for split search (in other words maximum number of relevant information that can be intersected by split)
	zero_threshold -- value above which to start consider relevant information of Radon transform (i.e. start and end points of value at least zero_threshold) for split search (in other words how big region should consider)
	min_split_abs -- minimal number of non-zero elements inside
	max_split_pct -- maximal region that should not be removed as pct of all

	Returns list of (angle, offset, intersect volume, sum before split, sum after split)
	"""

	theta = numpy.append(numpy.linspace(-numpy.pi/2, -min_angle, num_angles//2+1)[:-1],
						 numpy.linspace(min_angle, numpy.pi/2, num_angles//2+1))
	grid = local_radon.create_grid(src, theta)

	tmp = numpy.copy(src)
	res = []

	while True:
		gradon = local_radon.calc_local_radon(grid, tmp, 0, tmp.shape[0], 0, tmp.shape[1])

		mins = local_radon.get_local_minimas_2d(gradon, window_size_1d, max_value, zero_threshold)
		tres = select_best_local_minima(mins, theta, min_angle, max_value, min_split_abs, max_split_pct)
		if tres is None:
			return res

		off = local_radon.global_offset_from_subgrid(grid, 0, tmp.shape[0], 0, tmp.shape[1], tres[0], tres[1])
		ang = theta[tres[0]]



	def select_best_local_minima(minimas, angles, min_angle, max_value, max_split_pct, min_split_abs):
		tmp = [x for x in minimas if abs(angles[x[0]])>min_angle and x[2]<max_value
			   and min(x[3], x[4]) >= min_split_abs and min(x[3], x[4]) < max_split_pct * (x[3] + x[4])]
		if len(tmp) == 0:
			return None
		return sorted(tmp, key=lambda x: x[2])[0]
*/


/*
"""Returns indices of local maximas in arr1d using window_size and cutoff value of min_value (i.e. they are local maximas and global maximas inside window_size and greater than min_value).

	Keyword arguments:
	window_size -- width of window to be global maxima inside
	min_value -- cutoff value for local maxima

	Returns array of indices
	"""
	idf = np.array(range(1, len(arr1d)-1))[(arr1d[:-2]<=arr1d[1:-1])&(arr1d[2:]<=arr1d[1:-1])]
	if len(idf) == 0:
		return idf
	maxf = scipy.ndimage.maximum_filter1d(arr1d, window_size, mode='nearest')
	return idf[(maxf[idf]<=arr1d[idf])&(arr1d[idf]>=min_value)]
*/
std::vector<unsigned> get_radon_local_minimas_1d_(const xt::xtensor<float, 1>& src,
	unsigned window, float max_value)
{
	std::vector<unsigned> tmp = std::move(bujo::extremum::getLocalMinimas(src));
	std::vector<unsigned> res;
	res.reserve(tmp.size());
	int dneg = window / 2;
	int dpos = window - dneg;
	for (int i = 0; i < tmp.size(); i++)
	{
		if (src[tmp[i]] > max_value)
			continue;
		int j0 = std::max(0, static_cast<int>(tmp[i]) - dneg);
		int j1 = std::min(static_cast<int>(src.size()), static_cast<int>(tmp[i]) + dpos);
		float wmin = xt::amin(xt::view(src, xt::range(j0, j1)))[0];
		if (src[tmp[i]] > wmin)
			continue;
		res.push_back(tmp[i]);
	}
	return res;
}

/*
Returns indices of local minimas in arr2d using window_size_1d and cutoff value of max_value (i.e. they are local minimas and global minimas inside window_size and less than max_value).

Keyword arguments:
	window_size_1d -- width of window to be global minima inside
	max_value -- cutoff value for local minima
	zero_threshold -- value above which to start consider relevant information of Radon transform (i.e. start and end points of value at least zero_threshold)

Returns array of tuples (angle_id, offset_id)
*/
std::vector<std::tuple<unsigned, unsigned>> get_radon_local_minimas_2d_(const xt::xtensor<float, 2>& src,
	unsigned window, float max_value, float zero_threshold)
{
	/*
	(t0, t1) = get_positive_range(arr2d-zero_threshold)
	res = [(i, t0[i]+get_local_minimas_1d(arr2d[i, t0[i]:(t1[i]+1)], window_size_1d, max_value)) for i in range(len(t0))]
	return [(row[0], x, arr2d[row[0], x], np.sum(arr2d[row[0], :x]), np.sum(arr2d[row[0], (x+1):]))
			for row in res if len(row[1])>0 for x in row[1]]

	*/
	std::vector<std::tuple<unsigned, unsigned>> res;
	res.reserve(src.shape()[0] * src.shape()[1] / 100);
	for (unsigned i = 0; i < src.shape()[0]; i++)
	{
		std::tuple<unsigned, unsigned> jrng = bujo::extremum::getPositiveSuperRange(
			xt::view(src, i, xt::all()) - zero_threshold);
		std::vector<unsigned> ids = std::move(get_radon_local_minimas_1d_(xt::view(src, i,
			xt::range(std::get<0>(jrng), std::get<1>(jrng))), window, max_value));
		for (unsigned k = 0; k < ids.size(); k++)
			res.emplace_back(i, std::get<0>(jrng) + ids[k]);
	}
	return res;
}

SplitStat calc_split_stat_(const xt::xtensor<float, 1>& src, const xt::xtensor<float, 1>& offsets, unsigned offset_id)
{
	SplitStat res;

	res.volume_inside = src.at(offset_id);
	res.volume_before = res.volume_after = 0.0f;
	res.intensity_after = res.intensity_before = 0.0f;
	if (offset_id > 0)
	{
		res.volume_before = static_cast<float>(xt::sum(xt::view(src, xt::range(xt::placeholders::_, offset_id)))[0]);
		res.intensity_before = static_cast<float>(xt::amax(xt::view(src, xt::range(xt::placeholders::_, offset_id)))[0]);
	}
	if (offset_id < src.size() - 1)
	{
		res.volume_after = static_cast<float>(xt::sum(xt::view(src, xt::range(offset_id + 1, xt::placeholders::_)))[0]);
		res.intensity_after = static_cast<float>(xt::amax(xt::view(src, xt::range(offset_id + 1, xt::placeholders::_)))[0]);
	}

	int i_left = offset_id;
	int i_right = offset_id;
	while (i_left > 0)
	{
		if (src.at(i_left) > src.at(offset_id))
			break;
		i_left--;
	}
	while (i_right < src.size() - 1)
	{
		if (src.at(i_right) > src.at(offset_id))
			break;
		i_right++;
	}
	res.margin_after = std::fabsf(offsets.at(offset_id) - offsets.at(i_right));
	res.margin_before = std::fabsf(offsets.at(offset_id) - offsets.at(i_left));
	return res;
}

bool challenge_split_(const SplitStat& chmp, const SplitStat& chlg)
{
	if (chlg.volume_inside < chmp.volume_inside)
		return true;
	if (chlg.volume_inside > chmp.volume_inside + 1e-7f)
		return false;
	if (chlg.margin_before + chlg.margin_after > chmp.margin_before + chmp.margin_after)
		return true;
	if (chlg.margin_before + chlg.margin_after < chmp.margin_before + chmp.margin_after - 1e-7f)
		return false;
	if (chlg.margin_before * chlg.margin_after > chmp.margin_before * chmp.margin_after)
		return true;
	if (chlg.margin_before * chlg.margin_after < chmp.margin_before * chmp.margin_after - 1e-7f)
		return false;
	if (std::min(chlg.intensity_before, chlg.intensity_after) > std::min(chmp.intensity_before, chmp.intensity_after))
		return true;
	return false;
}

RegionSplit bujo::splits::findBestVSplit(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles,
	unsigned num_offsets, unsigned window_size,
	float minimal_abs_split_intensity, float maximal_abs_intersection, float minimal_pct_split)
{
	auto rtr = bujo::radon::radon(src, angles, num_offsets, bujo::radon::RT_RADON);
	auto mins = std::move(get_radon_local_minimas_2d_(std::get<0>(rtr), window_size, maximal_abs_intersection, minimal_abs_split_intensity));
	
	RegionSplit res;
	res.desc.direction = 0;
	res.stats.volume_inside = 1e30f;
	if (mins.empty())
		return {};

	for (unsigned i = 0; i < mins.size(); i++)
	{
		RegionSplit tmp;
		tmp.stats = calc_split_stat_(xt::view(std::get<0>(rtr), std::get<0>(mins[i]), xt::all()),
			std::get<1>(rtr), std::get<1>(mins[i]));
		if (std::min(tmp.stats.intensity_before, tmp.stats.intensity_after) <
			minimal_pct_split * (tmp.stats.intensity_before + tmp.stats.intensity_after))
			continue;
		if (challenge_split_(res.stats, tmp.stats))
		{
			res.desc.angle = angles[std::get<0>(mins[i])];
			auto idx = std::get<1>(mins[i]);
			res.desc.offset = std::get<1>(rtr).at(idx);
			res.desc.direction = (res.stats.volume_before < res.stats.volume_after ? -1 : 1);
			res.stats = tmp.stats;
			
			float offset_margin = 0.0f;
			int num_offset_margin = 0;
			if (idx < std::get<1>(rtr).size() - 1)
			{
				offset_margin += std::get<1>(rtr).at(idx + 1) - std::get<1>(rtr).at(idx);
				num_offset_margin++;
			}
			if (idx > 0)
			{
				offset_margin += std::get<1>(rtr).at(idx) - std::get<1>(rtr).at(idx - 1);
				num_offset_margin++;
			}
			res.desc.offset_margin = -0.5f*offset_margin / num_offset_margin;
		}
	}
	return res;
}

void bujo::splits::setRegionValue(xt::xtensor<float, 2>& src, const SplitDesc& split, int direction, float value)
{
	transformRegion(src, split, direction, [=](auto & v) {v = value; });
}

float bujo::splits::calcRegionQuantile(const xt::xtensor<float, 2>& src, const SplitDesc& split, int direction, float quantile)
{
	std::vector<float> data;

	transformRegion(src, split, direction, [&data](const auto & v) { data.push_back(v); });

	std::vector<float> buff(data.size());

	return bujo::util::calculateQuantile(data.begin(), data.end(), quantile, &buff[0], buff.size());
}

std::tuple<float, float> bujo::splits::calcRegionMeanStd(const xt::xtensor<float, 2>& src, const SplitDesc& split, int direction)
{
	double sum_x = 0.0f, sum_x2 = 0.0f, cnt = 0.0f;
	transformRegion(src, split, direction, [&sum_x, &sum_x2, &cnt](const auto & v) {
		sum_x += v;
		sum_x2 += v * v;
		cnt += 1.0;
		});

	return std::make_tuple(float(sum_x/cnt), float((sum_x2 - sum_x*sum_x / cnt) / cnt));
}

SplitDesc bujo::splits::rescaleSplit(const SplitDesc& desc, float dsize)
{
	SplitDesc res;
	res.angle = desc.angle;
	res.direction = desc.direction;
	res.offset = desc.offset * dsize;
	res.offset_margin = desc.offset_margin * dsize;
	return res;
}
