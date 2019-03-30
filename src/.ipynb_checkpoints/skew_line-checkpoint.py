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