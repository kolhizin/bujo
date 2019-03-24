import numpy
import skimage.transform
import filters

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
    
    theta = numpy.linspace(-np.pi/2, np.pi/2, max(image.shape), endpoint=False)
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