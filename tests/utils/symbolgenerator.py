import numpy as np


def create_random_symbols_geometric_distribution(num_values, p=0.05):
    
    return (np.random.default_rng(seed=0).geometric(p, num_values) - 1).tolist()


def create_random_symbols_uniform_distribution(num_values, max_val, min_val=0):
    
    return np.random.default_rng(seed=0).integers(min_val, max_val, num_values).tolist()
