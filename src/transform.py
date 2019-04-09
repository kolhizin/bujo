import numpy
import skimage.transform
import filters
import local_radon
import skew_line

def get_text_angle(image, min_local_maxima_dst=5, interval_points=[0.75, 0.50, 0.25]):
    """Find most probable angle of document in degrees

    Keyword arguments:
    min_local_maxima_dst -- ?? (default=5)
    interval_points -- percent of maximum to median at which return width of confidence interval (default=[0.75, 0.50, 0.25])
    """
    #find width at certain quota
    def find_width(src, ind, i_left, i_right, quota):
        ileft = i_left
        iright = i_right
        while ileft > 0 and src[ileft] >= src[ind] * quota:
            ileft -= 1

        while iright < len(src)-1 and src[iright] >= src[ind] * quota:
            iright += 1
        return (ileft, iright)
    
    #process one local maxima
    def process_local_maxima(src, width):
        ind = numpy.argmax(src)
        if src[ind] <= 0.0:
            return (None, None)
        
        inds = [(ind, ind)]
        for q in interval_points:
            (ileft, iright) = find_width(src, ind, inds[-1][0], inds[-1][1], q)
            inds.append((ileft, iright))
            
        res = src
        res[max(0, ind - width):min(ind+width, len(src))] = 0
        return (res, inds)
    
    #make returnable argument
    def process_return(src, angles, inds):
        #magnitude is measure of probability of it being a true angle
        #denominator = numpy.sum(numpy.abs(src[1:]-src[:-1]))
        denominator = numpy.std(src[1:])
        magnitude = src[inds[0][0]] / denominator
        angle = angles[inds[0][0]]
        angle_intervals = [(angles[i1], angles[i2]) for (i1, i2) in inds[1:]]
        return (angle, magnitude, angle_intervals)
    
    theta = numpy.linspace(-90, 90, max(image.shape), endpoint=False)
    img_radon = skimage.transform.radon(image, theta=theta, circle=True)
    src_variance = numpy.sum(numpy.abs(img_radon[1:]-img_radon[:-1]), axis=0)
    src_variance = src_variance - numpy.mean(src_variance)
    src_tmp = numpy.copy(src_variance)
    
    width_ind = int(numpy.ceil(max(image.shape)/180.0*min_local_maxima_dst))
    
    res = []
    while src_tmp is not None:
        src_tmp, inds = process_local_maxima(src_tmp, width_ind)
        if src_tmp is not None:
            res.append(inds)
    return [process_return(src_variance, theta, x) for x in res]

def get_text_line_delta(src, max_delta=100):
    """Calculates most probable delta between text lines

    Keyword arguments:
    max_delta -- maximum delta to consider
    
    Returns (delta, f(delta)), where f(delta) is array of odds of delta being correct
    """
    v = numpy.sum(numpy.abs(src[:,1:]-src[:,:-1]), axis=1)
    r = numpy.array([numpy.mean(numpy.abs(v[i:]-v[:-i])) for i in range(1, max_delta)])
    i = numpy.argmax((r[:-2]>=r[1:-1])&(r[2:]>=r[1:-1]))+1
    return (i, r)


def filter_variance(src, sz, qv, qh):
    """Calculate product of variance_percentile_h and variance_percentile_v filters

    Keyword arguments:
    sz -- pair of sizes
    qv, qh -- array-like of percentiles
    
    Returns (len(q), src.shape[0]-sz[0], src.shape[1]-sz[1]+1) ndarray
    """
    var_v = filters.variance_percentile_v(src, sz, qv)
    var_h = filters.variance_percentile_h(src, sz, qh)
    return var_v[:,1:-1]*var_h[1:-1,:]


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
    def select_best_local_minima(minimas, angles, min_angle, max_value, max_split_pct, min_split_abs):
        tmp = [x for x in minimas if abs(angles[x[0]])>min_angle and x[2]<max_value
               and min(x[3], x[4]) >= min_split_abs and min(x[3], x[4]) < max_split_pct * (x[3] + x[4])]
        if len(tmp) == 0:
            return None
        return sorted(tmp, key=lambda x: x[2])[0]

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
        
        if tres[3] > tres[4]:
            tmp[grid[:,:,tres[0]]>off] = False
        else:
            tmp[grid[:,:,tres[0]]<off] = False
            
        res.append((ang, off, tres[2], tres[3], tres[4]))
        
    return res

def erase_regions(src, splits, dsz=1):
    """Remove from src splits.

    Keyword arguments:
    splits -- list of [(angle, offset, v, v_neg, v_pos)]
    dsz -- size difference coefficient
    
    Returns image size of src 
    """
    res = numpy.copy(src)
    if splits is None or len(splits)==0:
        return src
    
    grid = local_radon.create_grid(src, numpy.array([x[0] for x in splits]))
    for (i, (ang, off, v, v_neg, v_pos)) in enumerate(splits):
        direction = 1
        if v_neg < v_pos:
            direction = -1
        res[(grid[:,:,i]-off*dsz)*direction > 0] = 0
    
    return res


def extract_lines_from_image(img, src, curves, line_delta, num_points, height_margin=0, filter_offset=None):
    """Warps and extracts curved line of height defined by offset
    
    Keyword arguments:
    img -- image to extract from (required to be 2d-array)
    src -- image to search for line height (required to be 2d-array)
    curves -- array of (crv_j, crv_i), i.e. curves that determine lines
    line_delta -- approximate line delta that is used in image -- used only to determine max offset for search of line height and as to calculate filter_offset if not specified
    num_points -- number of support points to use in curve (less points less jagged line)
    height_margin -- multiplicative factor to determine final height of line, i.e. final-height = calculated-height * (1 + height_margin)
    filter_offset -- parameter that specified if any vertical filters were used on src image besides different sizes, by default is calculated from line_delta, but can be overriden
    
    Returns list of 2d-arrays of size (max(offset)-min(offset), max(crv_p)-min(crv_p)) with warped image from img
    """
    max_offset = 2 * line_delta
    additive_regularization = numpy.power(numpy.linspace(0, 2, 2 * line_delta + 1), 2.0)
    use_filter_offset = filter_offset
    if filter_offset is None:
        use_filter_offset = (line_delta//2+1)//2+1
        
    res = []
    for (crv_j, crv_i) in curves:
        offsets = skew_line.get_curve_height(src, crv_j, crv_i, max_offset, additive_regularization=additive_regularization)
        crv_r = skew_line.reparametrize_by_x(crv_j, crv_i, num_points)
        crv_l = skew_line.reparametrize_by_length(*crv_r, num_points)
        sz_factor = img.shape[0] / (src.shape[0] + (use_filter_offset-1)*2)
        crv_q = (crv_l[0]*sz_factor, crv_l[1]*sz_factor, (crv_l[2]+use_filter_offset)*sz_factor)
    
        res.append(skew_line.extract_line(img, *crv_q, (numpy.floor(offsets[0]*sz_factor*(1+height_margin)), numpy.ceil(offsets[1]*sz_factor*(1+height_margin)))))
        
    return res