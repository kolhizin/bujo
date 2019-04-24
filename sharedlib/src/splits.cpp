#include "splits.h"
#include "radon.h"

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

RegionSplit bujo::splits::findBestVSplit(const xt::xtensor<float, 2>& src, const xt::xtensor<float, 1>& angles)
{
	unsigned numRadonOffsets = static_cast<unsigned>(std::ceilf(std::sqrtf(src.shape()[0]*src.shape()[0]+ src.shape()[1]*src.shape()[1])));
	auto rtr = bujo::radon::radon(src, angles, numRadonOffsets, bujo::radon::RT_RADON);
	/*
		gradon = local_radon.calc_local_radon(grid, tmp, 0, tmp.shape[0], 0, tmp.shape[1])

		mins = local_radon.get_local_minimas_2d(gradon, window_size_1d, max_value, zero_threshold)
	
	*/
	return RegionSplit();
}
