import numpy as np

def rolling_percentile_2d(src, sz, q):
    """Calculate rolling percentile with filter size of sz

    Keyword arguments:
    sz -- pair of sizes
    q -- array-like of percentiles
    
    Returns (len(q), src.shape[0]-sz[0]+1, src.shape[1]-sz[1]+1) ndarray
    """
    res = np.zeros((sz[0]*sz[1], src.shape[0] - sz[0] + 1, src.shape[1] - sz[1] + 1))
    
    for i in range(sz[0]):
        for j in range(sz[1]):
            res[i*sz[1]+j, :, :] = src[i:(src.shape[0]-sz[0]+1+i),j:(src.shape[1]-sz[1]+1+j)]
    
    return np.percentile(res, q, axis=0)

def variance_percentile_v(src, sz, q):
    """Calculate vertical total variance with window of sz[0] and apply horizontal rolling percentile (1, sz[1])

    Keyword arguments:
    sz -- pair of sizes
    q -- array-like of percentiles
    
    Returns (len(q), src.shape[0]-sz[0], src.shape[1]-sz[1]+1) ndarray
    """
    var = np.cumsum(np.abs(src[1:,:]-src[:-1,:]), axis=0)
    mean = (var[sz[0]:,:]-var[:-sz[0],:]) / sz[0]
    return rolling_percentile_2d(mean, (1, sz[1]), q)

def variance_percentile_h(src, sz, q):
    """Calculate horizontal total variance with window of sz[1] and apply horizontal rolling percentile (sz[0], 1)

    Keyword arguments:
    sz -- pair of sizes
    q -- array-like of percentiles
    
    Returns (len(q), src.shape[0]-sz[0], src.shape[1]-sz[1]+1) ndarray
    """
    var = np.cumsum(np.abs(src[:,1:]-src[:,:-1]), axis=1)
    mean = (var[:,sz[1]:]-var[:,:-sz[1]]) / sz[1]
    return rolling_percentile_2d(mean, (sz[0], 1), q)

def local_maxima_v(src, d1, d2, cutoff):
    """Calculates local maxima filter and applies cutoff

    Keyword arguments:
    i -- row id
    d1 -- width of local-maxima
    d2 -- test-range for local-maxima
    cutoff -- minimal value for local_maxima
    
    Returns ndarray of shape src.shape
    """
    def row_local_maxima_v(i):
        ilo1 = max(0, i-d1)
        ilo2 = max(0, i-d2)
        ihi1 = min(src.shape[0], i+d1+1)
        ihi2 = min(src.shape[0], i+d2+1)
        m1 = np.max(src[ilo1:ihi1], axis=0)
        m2 = np.max(src[ilo2:ihi2], axis=0)
        return (m1 >= m2)*(m2 > cutoff)
    return np.array([row_local_maxima_v(i) for i in range(src.shape[0])])

def stddev(src, filter_size):
    """Calculate rolling standard deviation with filter size of sz

    Keyword arguments:
    src -- source image
    filter_size -- (h, w) filter size
    
    Returns (src.shape[0]-filter_size[0], src.shape[1]-filter_size[1]) ndarray
    """
    f_x1 = np.cumsum(np.cumsum(src, axis=0), axis=1)
    f_x2 = np.cumsum(np.cumsum(np.power(src, 2.0), axis=0), axis=1)
    if type(filter_size) in (int, np.int64, np.int32, np.int16):
        flt_w = filter_size
        flt_h = filter_size
    else:
        flt_w = filter_size[1]
        flt_h = filter_size[0]
    nsize = flt_w * flt_h
    d_x1 = f_x1[flt_h:,flt_w:]-f_x1[:-flt_h,flt_w:]-f_x1[flt_h:,:-flt_w]+f_x1[:-flt_h,:-flt_w]
    d_x2 = f_x2[flt_h:,flt_w:]-f_x2[:-flt_h,flt_w:]-f_x2[flt_h:,:-flt_w]+f_x2[:-flt_h,:-flt_w]
    return np.sqrt(np.maximum(0,(d_x2 - d_x1*d_x1/nsize))/nsize)