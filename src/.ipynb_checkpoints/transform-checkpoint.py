import numpy
import skimage.transform

def get_text_angle(image, min_local_maxima_dst=5, interval_points=[0.75, 0.50, 0.25]):
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
    
    theta = numpy.linspace(-90., 90., max(image.shape), endpoint=False)
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