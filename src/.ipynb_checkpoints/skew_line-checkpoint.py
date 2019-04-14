import numpy as np
import scipy.ndimage
import local_radon


def make_curve_1way(i0, j0, step, src, grid, angles, kernel_size, reg_coef, reg_power):
    """Calculates curve from starting position (i0, j0) with directed step. Each step is in direction of regularized local angle.

    Keyword arguments:
    i0, j0 -- starting position
    step -- maximal step
    src -- ndarray of source values
    grid -- global grid
    angles -- array with angles in radians
    kernel_size -- size of region to determine local angle
    reg_coef -- mean value in subregion will be normalized on this value, i.e. np.mean(subgrid)/reg_coef
    reg_power -- np.power(np.mean(subgrid)/reg_coef, reg_power) -- full regularization term
        
    Returns list of (i, j)-coordinates of curve
    """
    isz = kernel_size[0]//2
    jsz = kernel_size[1]//2
    def calc_angle(i, j): #function to calculate local angle
        r_i0 = max(0, i-isz)
        r_i1 = min(src.shape[0], i+isz+1)
        r_j0 = max(0, j-jsz)
        r_j1 = min(src.shape[1], j+jsz+1)
        return local_radon.find_local_angle(src, grid, angles, r_i0, r_i1, r_j0, r_j1, reg_coef, reg_power)
    
    res = [(i0, j0)]
    while True:
        ci, cj = res[-1]
        # step like Runge-Kutta, i.e. we take 0.5 step in local angle, calculate new angle and take full step in that direction
        angle0 = calc_angle(ci, cj)
        di = step * np.sin(-angle0) * 0.5
        dj = step * np.cos(-angle0) * 0.5
        angle = calc_angle(int(np.round(ci+di)), int(np.round(cj+dj)))
        di = step * np.sin(-angle)
        dj = step * np.cos(-angle)
        ni = int(np.round(ci+di))
        nj = int(np.round(cj+dj))
        
        if ni < 0 or ni >= src.shape[0] or nj < 0 or nj >= src.shape[1]:
            # renormalize when we hit border
            ri = max(0, min(src.shape[0]-1, ni))
            rj = max(0, min(src.shape[1]-1, nj))
            
            rdi = 1.0
            rdj = 1.0
            if np.abs(di) > 1e-5:
                rdi = (ri - ci) / di
            if np.abs(dj) > 1e-5:
                rdj = (rj - cj) / dj
                
            rstep = min(rdi, rdj) * 0.9
            
            ni = int(np.round(ci+di*rstep))
            nj = int(np.round(cj+dj*rstep))
            res.append((ni, nj))            
            break
        
        res.append((ni, nj))
    return res

def make_curve_2way(i0, j0, step, src, grid, angles, kernel_size, reg_coef, reg_power):
    """Calculates curve from starting position (i0, j0) in both direction with specified step. Each step is in direction of regularized local angle.

    Keyword arguments:
    i0, j0 -- starting position
    step -- maximal step
    src -- ndarray of source values
    grid -- global grid
    angles -- array with angles in radians
    kernel_size -- size of region to determine local angle
    reg_coef -- mean value in subregion will be normalized on this value, i.e. np.mean(subgrid)/reg_coef
    reg_power -- np.power(np.mean(subgrid)/reg_coef, reg_power) -- full regularization term
        
    Returns list of (i, j)-coordinates of curve
    """
    crv_p = make_curve_1way(i0, j0, step, src, grid, angles, kernel_size, reg_coef, reg_power)
    crv_n = make_curve_1way(i0, j0, -step, src, grid, angles, kernel_size, reg_coef, reg_power)
    return crv_n[:0:-1]+crv_p



def interpolate_between_curves(jspace, i_min, i_max, crv1, crv2, a):
    """Calculates value i-value for each j from jspace as linear interpolation between 2 curves.

    Keyword arguments:
    jspace -- points to calculate values
    crv1, crv2 -- curves as list of (i, j)-coordinates
    a -- interpolation coef (0 = crv1, 1 = crv2)
        
    Returns j-values, i-values of result
    """
    i1_s = np.interp(jspace, crv1[:,1], crv1[:,0])
    i2_s = np.interp(jspace, crv2[:,1], crv2[:,0])
    return jspace, np.floor(0.5+i1_s*(1-a)+a*i2_s).astype(int)

def offset_curve(jspace, crv, offset):
    """Calculates value i-value for each j from jspace as offset to provided curve

    Keyword arguments:
    jspace -- points to calculate values
    crv -- curve as list of (i, j)-coordinates
    offset -- i-offset
        
    Returns j-values, i-values of result
    """
    i_s = np.floor(0.5+np.interp(jspace, crv[:,1], crv[:,0])).astype(int)+offset
    return jspace, i_s

def integrate_between_curves(src, crv1, crv2):
    """Calculates integral over interpolated curves. Number of midpoints corresponds to largest distance between crv1 and crv2.

    Keyword arguments:
    src -- source map (2d-array)
    crv1, crv2 -- curves as list of (i, j)-coordinates
        
    Returns array of values and array of a-coefs of linear interpolation
    """
    j_s = np.arange(src.shape[1])
    i1_s = np.interp(j_s, crv1[:,1], crv1[:,0])
    i2_s = np.interp(j_s, crv2[:,1], crv2[:,0])
    num = np.max(np.abs(i1_s-i2_s))
    alphas = np.linspace(0, 1, num)
    ias = [np.minimum(src.shape[0]-1, np.maximum(0, np.floor(0.5+i1_s*(1-a)+a*i2_s))) for a in alphas]
    return np.array([np.sum(src[i_s.astype(int), j_s]) for i_s in ias]), alphas

def integrate_offsets_curve(src, crv, dvs):
    """Calculates integral over offsets of curve.

    Keyword arguments:
    src -- source map (2d-array)
    crv -- curve as list of (i, j)-coordinates
    dvs -- array of i-offsets
        
    Returns array of values and array of offsets
    """
    j_s = np.arange(src.shape[1])
    i_s = np.floor(0.5+np.interp(j_s, crv[:,1], crv[:,0])).astype(int)
    return np.array([np.sum(src[np.minimum(src.shape[0]-1, np.maximum(0, i_s+d)), j_s]) for d in dvs]), dvs

def integrate_curves(src, curves):
    """Calculates integrals over offsets of first and last curve and interpolation of all other.

    Keyword arguments:
    src -- source map (2d-array)
    curves -- list of curves as list of (i, j)-coordinates
        
    Returns array of values and array of meta information (c1,c2,a) for interpolation and (c,offset) for offset
    """
    res0 = []
    res1 = []
    df = np.min(curves[0][:,0])
    irf, dsf = integrate_offsets_curve(src, curves[0], -np.arange(df)[::-1])
    res0 += irf.tolist()
    res1 += [(curves[0], d) for d in dsf]
    for (c1,c2) in zip(curves[:-1], curves[1:]):
        ir_s, a_s = integrate_between_curves(src, c1, c2)
        res0 += ir_s.tolist()
        res1 += [(c1, c2, a) for a in a_s]
    dl = src.shape[0]-np.max(curves[-1][:,0])
    irl, dsl = integrate_offsets_curve(src, curves[-1], np.arange(dl))
    res0 += irl.tolist()
    res1 += [(curves[-1], d) for d in dsl]
    return np.array(res0), res1


def select_line_candidates(arr1d, window_size, min_value, minmax_ratio):
    """Selects possible lines location based on integral over curves (or Radon transform, e.g.).

    Keyword arguments:
    arr1d -- array of integral values
    window_size -- local maxima should be global maxima within window_size
    min_value -- local maxima should be at least min_value large
    minmax_ratio -- surrounding minima to local maxima should be at least minmax_ratio
        
    Returns array of indices
    """
    #select local maximas
    ids0 = local_radon.get_local_maximas_1d(arr1d, window_size, min_value)
    
    #filter the same value (e.g. flat region) by leaving only most center maxima
    ids1 = []
    msk = (ids0[1:]==(ids0[:-1]+1))
    i = 0
    while i < len(ids0):
        k = 0
        while i < len(msk) and msk[i]:
            i += 1
            k += 1
        
        ids1.append(ids0[i-k//2])
        i += 1
        
    #filter out those that do not pass surrounding minima to local maxima constraint
    ids1 = [i for i in ids1 if max(np.min(arr1d[max(0, i-window_size):(i+1)]),np.min(arr1d[i:min(len(arr1d), i+window_size+1)]))/arr1d[i] < minmax_ratio]
    
    #filter out too frequent local maxima by select the largest within window_size
    ids2 = []
    i = 0
    while i < len(ids1):
        if i < len(ids1)-1:
            if abs(ids1[i]-ids1[i+1]) < window_size and arr1d[ids1[i+1]]>arr1d[ids1[i]]:
                i += 1
                continue
        if i > 0:
            if abs(ids1[i]-ids1[i-1]) < window_size and arr1d[ids1[i-1]]>arr1d[ids1[i]]:
                i += 1
                continue
        ids2.append(ids1[i])
        i += 1
    return ids2

def get_array_range(src, q):
    """Find range where array first hits certain quantile.
    
    Keyword arguments:
    src -- 1d-array
    q -- percentile to hit
    axis -- in which axis to search
    
    Returns (min, max)
    """
    val = np.percentile(src, q)
    inds = np.arange(len(src))[src>val]
    return (min(inds), max(inds))

def get_sparse_max_points(src, i0, i1, window, stride):
    """Find sparse maximums of width window with stride.
    
    Keyword arguments:
    src -- 1d-array
    (i0, i1) -- range where to search
    window -- width of window where to search maximum
    stride -- stride between windows
    
    Returns array of indices
    """
    res = []
    for i in range(i0, i1, stride):
        res.append(i+np.argmax(src[i:(i+window)]))
    return np.array(res)

def create_support_curves(src, step, kernel_size, num_angles=91, range_quantiles=(75, 50), support_window=30, max_angle=np.pi/4, reg_coef=0.1, reg_power=0.25):
    """Creates support curves by using local angly (by Radon transform).

    Keyword arguments:
    src -- source image (required to be 0/1 2d-array)
    step -- local step while iteratively creating curves
    kernel_size -- size of region to determine local angle
    max_angle -- maximum angle for curve creation
    num_angles -- size of grid between (-max_angle, max_angle)
    range_quantiles -- pair of values specifying borders where to start looking for support qurves in terms of quantiles
    support_window -- value that specifies density of support curves
    reg_coef -- mean value in subregion will be normalized on this value, i.e. np.mean(subgrid)/reg_coef, if zero than auto as mean in range defined by range_quantiles
    reg_power -- np.power(np.mean(subgrid)/reg_coef, reg_power) -- full regularization term
        
    Returns list of curves (list of (i, j)-coordinates)
    """
    theta = np.linspace(-max_angle, max_angle, num_angles)
    rgrid = local_radon.create_grid(src, theta)

    rj0, rj1 = get_array_range(np.mean(src, axis=0), range_quantiles[1])
    src_h = np.mean(src[:,rj0:rj1], axis=1)
    ri0, ri1 = get_array_range(src_h, range_quantiles[0])
    
    support_stride = support_window * 2
    
    global_mean = np.mean(src[ri0:ri1, rj0:rj1])
    used_reg_coef = reg_coef
    if used_reg_coef <= 0.0:
        used_reg_coef = global_mean

    rows = np.concatenate(([ri0], get_sparse_max_points(src_h, ri0+support_window, ri1-support_window, support_window, support_stride), [ri1]))

    return [np.array(make_curve_2way(i, (rj0+rj1)//2, step, src, rgrid, theta, kernel_size, reg_coef=used_reg_coef, reg_power=reg_power)) for i in rows]


def calc_cumulative_integral_with_offset(src, curve_j, curve_i, offsets):
    """Calculates cumulative integral over curve of image src with specified offsets
    
    Keyword arguments:
    src -- source image (required to be 0/1 2d-array)
    curve_j, curve_i -- 1d-array of equal length specifying curve
    offsets -- list-like structure with offsets
    
    Returns 2d-array of size (len(offsets), src.shape[1]) with cumulative integrals for different offsets
    """
    return np.array([np.cumsum(src[np.minimum(src.shape[0]-1, np.maximum(0, curve_i+offset)), curve_j]) for offset in offsets])

def find_local_maximum_by_ascend(arr1d, i):
    """Finds closest local maximum by gradient ascend
    
    Keyword arguments:
    arr1d -- 1d-array
    i -- starting point
    
    Returns index of closest maximum
    """
    i = max(0, min(len(arr1d)-1, i))
    if (i<=0 or arr1d[i-1] < arr1d[i]) and (i>=len(arr1d)-1 or arr1d[i+1] < arr1d[i]):
        return i
    
    if i <= 0:
        adir = arr1d[i:]
        ddir = +1
    elif i >= len(arr1d)-1:
        adir = arr1d[:(i+1)][::-1]
        ddir = -1
    elif arr1d[i-1] >= arr1d[i] and arr1d[i+1] >= arr1d[i]:
        if arr1d[i-1]>arr1d[i+1]:
            adir = arr1d[:(i+1)][::-1]
            ddir = -1
        else:
            adir = arr1d[i:]
            ddir = +1
    elif arr1d[i-1] > arr1d[i]:
        adir = arr1d[:(i+1)][::-1]
        ddir = -1
    else:
        adir = arr1d[i:]
        ddir = +1
    off = np.argmin(adir[1:]>adir[:-1])
    return i + off*ddir

def calc_recursive_offsets_by_midpoint(arr2d, i0, i1, offset, min_window):
    """Finds best offset from specified offset and then by binary split fine-tunes results until min_window is reached.
    
    Keyword arguments:
    arr2d -- array with cumulative integrals for specified offsets (shape of image, number of offsets)
    i0, i1 -- considered segment
    offset -- current offset
    min_window -- minimal segment length
    
    Returns tuple of (i0, i1, offset, integral_value, [optional] recursive left split, [optinal] recursive right split)
    """
    integral_value = arr2d[-1,offset]-arr2d[0,offset]
    if arr2d.shape[0] < min_window or integral_value <= 0:
        return (i0, i1, offset, integral_value)
    
    midpoint = arr2d.shape[0] // 2
    arr_l = arr2d[midpoint,:] - arr2d[0,:]
    arr_r = arr2d[-1,:] - arr2d[midpoint,:]
    off_l = find_local_maximum_by_ascend(arr_l, offset)
    off_r = find_local_maximum_by_ascend(arr_r, offset)
    return (i0, i1, offset, integral_value, 
            calc_recursive_offsets_by_midpoint(arr2d[:midpoint], i0, i0+midpoint, off_l, min_window),
            calc_recursive_offsets_by_midpoint(arr2d[midpoint:], i0+midpoint, i1, off_r, min_window))

def convert_offset_tree_to_linear_vector(offset_tree):
    """Converts output of calc_recursive_offsets_by_midpoint into linear vector.
    
    Keyword arguments:
    offset_tree -- tuple of (i0, i1, offset, integral_value, [optional] recursive left split, [optinal] recursive right split)
    
    Returns list of tuples (j-coord, i-coord) and tuple of min-j and max-j
    """
    if offset_tree is None or offset_tree[3] <= 0.0:
        return [], (np.inf, -np.inf)
    if len(offset_tree) == 4:
        return [((offset_tree[0]+offset_tree[1])*0.5, offset_tree[2])], (offset_tree[0], offset_tree[1])
    
    l_part, l_mm = convert_offset_tree_to_linear_vector(offset_tree[4])
    r_part, r_mm = convert_offset_tree_to_linear_vector(offset_tree[5])
    
    f_mm = (np.min([l_mm[0], r_mm[0]]), np.max([l_mm[1], r_mm[1]]))    
    return l_part+r_part, f_mm

def optimize_curve(src, curve_j, curve_i, offset, min_window, add_trim=3):
    """Creates optimized version of specified curve by globally ascending to local maximum and fine-tuning result on binary splits. Shrinks both ends with zero integral.
    
    Keyword arguments:
    src -- source image (required to be 0/1 2d-array)
    curve_j, curve_i -- 1d-array of equal length specifying curve
    offset -- maximum offset to consider for search of local maximum
    min_window -- min segment length to split data
    add_trim -- extend ends by this amount
    
    Returns new_j, new_i -- optimized curve
    """
    loc_arr2d = calc_cumulative_integral_with_offset(src, curve_j, curve_i, np.arange(-offset, offset+1)).T
    tree_off = calc_recursive_offsets_by_midpoint(loc_arr2d, 0, loc_arr2d.shape[0], offset, min_window)
    off_ji, off_mm = convert_offset_tree_to_linear_vector(tree_off)
    off_ji_np = np.array(off_ji)
    off_min = max(0, int(off_mm[0])-add_trim)
    off_max = min(src.shape[1], int(off_mm[1])+add_trim)
    new_j = curve_j[off_min:off_max]
    new_i_full = np.minimum(src.shape[0]-1, np.maximum(0, np.round(curve_i + np.interp(curve_j, off_ji_np[:,0], off_ji_np[:,1]) - offset))).astype(int)
    return new_j, new_i_full[off_min:off_max]


def get_curve_height(src, crv_j, crv_i, max_offset, additive_regularization=0):
    """Searches height-boundaries of curve in image src with regularization by searching closes local minimum
    
    Keyword arguments:
    src -- source image (required to be 2d-array)
    crv_j, crv_i -- 1d-array of equal length specifying curve
    max_offset -- maximum offset to consider for search of local minimum
    additive_regularization -- 1d-array of size of max_offset + 1 that defines additive regularization term (i.e. f_reg(x) = f(x) + reg(x))
    
    Returns (-offset_low, offset_high) -- tuple with boundaries of curve
    """
    def get_closest_local_min(arr1d):
        off_less = np.argmax(arr1d[1:]<arr1d[:-1])    
        msk_min = arr1d[1:]>=arr1d[:-1]
        off = np.argmax(msk_min[off_less:])+off_less
        return off
    offsets = np.arange(-max_offset, max_offset+1)
    values = np.array([np.sum(src[np.maximum(0, np.minimum(src.shape[0]-1, crv_i+off)), crv_j]) for off in offsets])
    off_r = get_closest_local_min(values[max_offset:]+additive_regularization)
    off_l = get_closest_local_min(values[:(max_offset+1)][::-1]+additive_regularization)
    return (-off_l, off_r)

def reparametrize_by_x(crv_j, crv_i, num_points):
    """Creates new parametrization based on j-points with num_points support points
    
    Keyword arguments:
    crv_j, crv_i -- 1d-array of equal length specifying curve
    num -- new number of support points
    
    Returns (crv_j, crv_i) -- curve
    """
    new_j = np.linspace(np.min(crv_j), np.max(crv_j), num_points)
    new_i = np.interp(new_j, crv_j, crv_i)
    return new_j, new_i

def reparametrize_by_length(crv_j, crv_i, num_points):
    """Creates new parametrization based on curve-length with num_points support points
    
    Keyword arguments:
    crv_j, crv_i -- 1d-array of equal length specifying curve
    num -- new number of support points
    
    Returns (crv_p, crv_j, crv_i) -- curve with nonuniform parametrization
    """
    dx2 = np.power(np.abs(crv_j[1:]-crv_j[:-1]), 2.0)
    dy2 = np.power(np.abs(crv_i[1:]-crv_i[:-1]), 2.0)
    crv_len = np.insert(np.cumsum(np.sqrt(dx2+dy2)), 0, 0) #prepend 0
    new_len = np.linspace(0, crv_len[-1], num_points) #parametrization by length
    new_j = np.interp(new_len, crv_len, crv_j)
    new_i = np.interp(new_j, crv_j, crv_i)
    return new_len, new_j, new_i

def extract_line(src, crv_p, crv_j, crv_i, offset):
    """Warps and extracts curved line of height defined by offset
    
    Keyword arguments:
    src -- source image (required to be 2d-array)
    crv_p, crv_j, crv_i -- 1d-array of equal length specifying curve with nonuniform parametrization
    offset -- tuple with boundaries of curve
    
    Returns 2d-array of size (max(offset)-min(offset), max(crv_p)-min(crv_p)) with warped image from src
    """
    height = np.ceil(np.max(offset) - np.min(offset)).astype(int)
    width = np.ceil(np.max(crv_p) - np.min(crv_p)).astype(int)
    res = np.zeros(shape=(height, width))
    
    actual_p = np.arange(width) + np.min(crv_p)
    actual_j = np.floor(np.interp(actual_p, crv_p, crv_j) + 0.5).astype(int)
    actual_i = np.floor(np.interp(actual_p, crv_p, crv_i) + 0.5 + min(offset)).astype(int)
    
    for i in range(height):
        res[i,:] = src[actual_i + i, actual_j]
        
    return res



def locate_positive_regions(arr1d):
    """Locate regions where array is positive and return list of segments.
    
    Keyword arguments:
    arr1d -- numpy 1d-array
    
    Returns list of (start, end) of each positive region
    """
    #check that there are no zeros or modify array a little:
    if(np.any(arr1d==0)):
        if(np.all(arr1d==0)):
            return []
        delta = 0.5*np.min(np.abs(arr1d[arr1d!=0]))
    else:
        delta = 0
    smd = arr1d + delta
    lends = np.arange(len(smd)-1)[(smd[:-1]<0)&(smd[1:]>0)]
    rends = np.arange(len(smd)-1)[(smd[:-1]>0)&(smd[1:]<0)]+1
    
    if smd[0] > 0:
        lends = np.insert(lends, 0, 0)
    if smd[-1] > 0:
        rends = np.append(rends, len(smd))
    
    return list(zip(lends, rends))


def locate_words(src_line, min_space_size,
                 cutoff_coef_detect, cutoff_coef_confirm,
                 reg_lo, reg_hi, reg_center_pct):
    """Locate offsets of words in src_line
    
    Keyword arguments:
    src_line -- source line image (required to be 2d-array)
    min_space_size -- minimal distance between words to consider different words
    cutoff_coef_detect -- cutoff for detection of word-candidates
    cutoff_coef_confirm -- cutoff for confirmation of words
    reg_lo, reg_hi -- regularization coeficients for np.power(1 - np.power(reg2, reg_hi), reg_lo)
    reg_center_pct -- location of center of the line for regularization
    
    Returns list of (start, end) of detected words
    """
    def expand_regions_to_local_minima(src, segments):
        tmp = -src
        return [(max(0, find_local_maximum_by_ascend(tmp, a)),
                 min(src.shape[0]-1, find_local_maximum_by_ascend(tmp, b))) for (a, b) in segments]

    def filter_segments(src, segments, min_word_value):
        return [(a,b) for (a,b) in segments if b>a and np.median(src[a:b]) > min_word_value]

    def join_segments(segments, window_size):
        if len(segments) == 0:
            return []
        res = [segments[0]]
        for (a,b) in segments[1:]:
            if a - res[-1][1] < window_size:
                res[-1] = (res[-1][0], b)
            else:
                res.append((a,b))
        return res
    
    reg1 = np.abs(np.linspace(0, 1, src_line.shape[0]) - reg_center_pct)
    reg2 = np.minimum(1.0, reg1 / min(reg1[0], reg1[-1]))
    reg_vector = np.power(1 - np.power(reg2, reg_hi), reg_lo)
    rs = np.array([src_line[:,i]*reg_vector for i in range(src_line.shape[1])]).T
    rsmax = np.max(rs, axis=0)
    
    qntl = np.percentile(rsmax[rsmax>0], 90)
    cutoff0 = qntl * cutoff_coef_detect
    cutoff1 = qntl * cutoff_coef_confirm
    segs = expand_regions_to_local_minima(rsmax, locate_positive_regions(rsmax-cutoff0))
    return join_segments(filter_segments(rsmax, segs, cutoff1), min_space_size)

def calculate_word_curve(curve_p, curve_j, curve_i, word_location, num_points=2):
    """Reparametrize curve by word location
    
    Keyword arguments:
    curve_p, curve_j, curve_i -- initial curve with length-parametrization
    word_location -- tuple (start, end)
    num_points -- number of points for interpolation
    
    Returns curve-p, curve-j, curve-i with num_points corresponding to curve
    """
    interp_l = np.linspace(word_location[0], word_location[1], num_points)
    res_j = np.interp(interp_l, curve_p, curve_j)
    res_i = np.interp(interp_l, curve_p, curve_i)
    return np.linspace(0, word_location[1]-word_location[0], num_points), res_j, res_i

