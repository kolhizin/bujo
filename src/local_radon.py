import numpy as np
import scipy.ndimage

def create_grid_legacy_(src, angles):
    """Create grid for Radon transform of src-image

    Keyword arguments:
    angles -- array of angles in radians
        
    Returns (src.shape[0], src.shape[1], len(angles)) ndarray
    """
    grid = np.array([[(i, j) for j in range(src.shape[1])] for i in range(src.shape[0])])
    uvecs = np.array([np.cos(angles), np.sin(angles)])
    return np.round(np.dot(grid, uvecs))

def create_grid(src, angles):
    """Create grid for Radon transform of src-image

    Keyword arguments:
    angles -- array of angles in radians
        
    Returns (src.shape[0], src.shape[1], len(angles)) ndarray
    """
    g1 = np.arange(src.shape[0]).reshape(-1, 1, 1) * np.cos(angles).reshape(1, -1)
    g2 = np.arange(src.shape[1]).reshape(1, -1, 1) * np.sin(angles).reshape(1, -1)
    return np.floor(g1+g2+0.5) #instead of np.round which is crazy slow - don't know why
    
def count_values_in_range(xres, vmin, vmax):
    """Count values in range [vmin, vmax] of xres

    Keyword arguments:
    xres -- integer array
    [vmin, vmax] -- range to be used
        
    Returns counts of each value including 0 in range
    """
    xs, cnts = np.unique(np.append(xres[(xres>=vmin)&(xres<=vmax)], range(vmin, vmax+1)), return_counts=True)
    return cnts-1

def filter_subgrid(grid, flt, i0, i1, j0, j1):
    """Creates centered subgrid of [i0, i1)x[j0, j1) with flt > 0

    Keyword arguments:
    flt -- ndarray to be used for filtering (>0)
    [i0, i1)x[j0, j1) -- range to be used
        
    Returns array of arrays of filtered elements (by number of angles)
    """
    return (grid[i0:i1, j0:j1, :]-grid[(i0+i1)//2, (j0+j1)//2, :])[flt[i0:i1, j0:j1]>0, :]

def global_offset_from_subgrid(grid, i0, i1, j0, j1, angle_idx, offset):
    """Calculates global offset from local subgrid

    Keyword arguments:
    grid -- global grid
    [i0, i1)x[j0, j1) -- local subgrid
    angle_idx -- selected angle
    offset -- local offset
        
    Returns global offset
    """
    val = max(abs(i1-i0), abs(j1-j0))
    return (-val + offset)+grid[(i0+i1)//2, (j0+j1)//2, angle_idx]

def calc_local_radon(grid, flt, i0, i1, j0, j1):
    """Calculates local Radon transform on subgrid

    Keyword arguments:
    grid -- global grid
    flt -- ndarray to be used for filtering (>0)
    [i0, i1)x[j0, j1) -- local subgrid
        
    Returns (len(angles), 2*max(abs(i1-i0), abs(j1-j0))+1) ndarray with Radon transform
    """
    val = max(abs(i1-i0), abs(j1-j0))
    tmp_grid = filter_subgrid(grid, flt, i0, i1, j0, j1)
    return np.array([count_values_in_range(tmp_grid[:, i], -val, val) for i in range(grid.shape[2])])


def get_positive_range(src):
    """Calculates start and end of positive values for each row of src. It is often useful for Radon transformed images. Input is matrix of (num_angles, num_offsets)
        
    Returns (start, end), where start/end is array of length num_angles
    """
    msk = src > 0
    i0 = np.argmax(msk, axis=1)
    i1 = np.argmax(msk[:,::-1], axis=1)
    r0 = np.array([i0[k] if msk[k, i0[k]] else msk.shape[1]-1 for k in range(len(i0))])
    r1 = np.array([src.shape[1]-i1[k]-1 if msk[k, src.shape[1]-i1[k]-1] else 0 for k in range(len(i1))])
    return (r0, r1)


def get_local_minimas_1d(arr1d, window_size, max_value):
    """Returns indices of local minimas in arr1d using window_size and cutoff value of max_value (i.e. they are local minimas and global minimas inside window_size and less than max_value).
        
    Keyword arguments:
    window_size -- width of window to be global minima inside
    max_value -- cutoff value for local minima
    
    Returns array of indices
    """
    idf = np.array(range(1, len(arr1d)-1))[(arr1d[:-2]>=arr1d[1:-1])&(arr1d[2:]>=arr1d[1:-1])]
    if len(idf) == 0:
        return idf
    minf = scipy.ndimage.minimum_filter1d(arr1d, window_size, mode='nearest')
    return idf[(minf[idf]>=arr1d[idf])&(arr1d[idf]<=max_value)]

def get_local_maximas_1d(arr1d, window_size, min_value):
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

def get_local_minimas_2d(arr2d, window_size_1d, max_value, zero_threshold):
    """Returns indices of local minimas in arr2d using window_size_1d and cutoff value of max_value (i.e. they are local minimas and global minimas inside window_size and less than max_value).
        
    Keyword arguments:
    window_size_1d -- width of window to be global minima inside
    max_value -- cutoff value for local minima
    zero_threshold -- value above which to start consider relevant information of Radon transform (i.e. start and end points of value at least zero_threshold)
    
    Returns array of tuples (angle_id, offset_id, arr2d[angle_id, offset_id], sum before offset_id, sum after offset_id)
    """
    (t0, t1) = get_positive_range(arr2d-zero_threshold)
    res = [(i, t0[i]+get_local_minimas_1d(arr2d[i, t0[i]:(t1[i]+1)], window_size_1d, max_value)) for i in range(len(t0))]
    return [(row[0], x, arr2d[row[0], x], np.sum(arr2d[row[0], :x]), np.sum(arr2d[row[0], (x+1):]))
            for row in res if len(row[1])>0 for x in row[1]]


def calc_local_radon_angle(grid, flt, i0, i1, j0, j1):
    """Calculates local Radon transform on subgrid

    Keyword arguments:
    grid -- global grid
    flt -- ndarray to be used for filtering (>0)
    [i0, i1)x[j0, j1) -- local subgrid
        
    Returns angle_id in which there is maximal variance
    """
    vals = grid[i0:i1, j0:j1, :][flt[i0:i1, j0:j1]>0, :]
    angs = [np.sum(np.power(np.unique(v, return_counts=True)[1], 2)) for v in vals.T]
    return np.argmax(angs)

def find_local_angle(src, grid, angles, i0, i1, j0, j1, reg_coef=0.2, reg_power=1.0):
    """Finds regularized angle by local Radon transform on subgrid

    Keyword arguments:
    src -- ndarray of source values
    grid -- global grid
    angles -- array with angles in radians
    [i0, i1)x[j0, j1) -- local subgrid
    reg_coef -- mean value in subregion will be normalized on this value, i.e. np.mean(subgrid)/reg_coef
    reg_power -- np.power(np.mean(subgrid)/reg_coef, reg_power) -- full regularization term
        
    Returns regularized angle
    """
    ang = angles[calc_local_radon_angle(grid, src, i0, i1, j0, j1)]
    pct = np.power(min(1, np.mean(src[i0:i1,j0:j1])/reg_coef), reg_power)
    return ang * pct

