import unittest
import random
import cabac
import math


def create_random_symbols_geometric_distribution(num_values, p):
    import numpy as np
    return (np.random.default_rng(seed=0).geometric(p, num_values) - 1).tolist()


class MainTest(unittest.TestCase):
    
    def test_ctx_selection_bins_order1(self):
        import numpy as np  # TODO: move this import somewhere else
        def ctx_ids_bins_order1_tu(n, prev_symbol=0, n_rst=10):
            # n is the position of the to-be coded bin in the bin string (of the to be coded symbol)
            # prev_symbol is the previously coded symbol
            # bins with n < n_rst are coded with ctx model, all other bins are coded with the same context.
            # returns the context id
            #
            # First ctx_ids correspond to previous bin at position n not available case.
            # Next ctx_ids correspond to previous bin at position n = 0 case.
            # Next ctx_ids correspond to previous bin at position n = 1 case.
            # Last ctx_id is used for all bins n >= n_rst

            n = np.asarray(n)
            ctx_id = np.zeros(n.shape, dtype=np.int32)

            # binary masks
            mask_rst = n < n_rst  # mask for rest case
            mask_prev_symbol_na = mask_rst & (n > prev_symbol)  # mask for previous bin not available
            mask_prev_symbol_v0 = mask_rst & (n < prev_symbol)  # mask for previous bin 0
            mask_prev_symbol_v1 = mask_rst & (n == prev_symbol) # mask for previous bin 1

            # bin position n < n_rst
            # previously coded bin at same position not available
            ctx_id[mask_prev_symbol_na] = 0*n_rst + n[mask_prev_symbol_na]

            # previously coded bin 0 at same position n
            ctx_id[mask_prev_symbol_v0] = 1*n_rst + n[mask_prev_symbol_v0]

            # previously coded bin 1 at same position n
            ctx_id[mask_prev_symbol_v1] = 2*n_rst + n[mask_prev_symbol_v1]

            # bin position n >= n_rst
            # rst case, independent of position n
            ctx_id[~mask_rst] = 3*n_rst

            return ctx_id
        
        num_max_val = 255
        num_values = 1000
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        
        symbolPrev = 0
        num_rest = 10
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbolPrev = symbols[i-1]
            
            n = list(range(0, symbol+1))
            ctx_ids = ctx_ids_bins_order1_tu(n, symbolPrev, num_rest)
            ctx_ids2 = cabac.getContextIdsBinsOrder1TU(symbolPrev, num_rest, symbol+1)
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

    def test_ctx_selection_symbols_order1(self):
        import numpy as np  # TODO: move this import somewhere else
        def ctx_ids_symbols_order1_tu(n, prev_symbol=0, rest_pos=10, symbol_max=32):
            # ContextID dependent on bin position n of to-be-decoded symbol as well as previous integer symbol value, in fact
            # modeling the probability p(b_n | symbolPrev) with bin b_n at position n in TU-binarized bin-string.

            # In total we have num_ctx_total = (symbolMax+1)*restPos + 1 contexts
            # 
            # The ctx_id is computed as follows:
            # ctx_id:                       meaning:
            # 0*restPos ... 1*restPos-1:    previously coded symbol = 0 (and n<restPos)
            # 1*restPos ... 2*restPos-1:    previously coded symbol = 1 (and n<restPos)
            # ...
            # (symbolMax+0)*restPos ... (symbolMax+1)*restPos-1:  previously coded symbol = symbolMax (and n<restPos)
            # (symbolMax+0)*restPos ... (symbolMax+1)*restPos-1:  previously coded symbol > symbolMax (and n<restPos)
            # ...
            # (symbolMax+1)*restPos:                              position n>=restPos: rest case with single context

            n = np.asarray(n)
            ctx_id = np.zeros(n.shape, dtype=np.int32)

            # binary mask
            mask_not_rest = n < rest_pos  # mask for not-rest case

            # clip previous symbol to symbol_max
            prev_symbol0 = prev_symbol if prev_symbol <= symbol_max else symbol_max
            
            # bin position n < rest_pos
            ctx_id[mask_not_rest] = (prev_symbol0)*rest_pos + n[mask_not_rest]

            # bin position n >= rest_pos
            # rst case, independent of position n
            ctx_id[~mask_not_rest] = (symbol_max+1)*rest_pos

            return ctx_id
        
        num_max_val = 255
        num_values = 1000
        symbol_max = 15
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        
        symbolPrev = 0
        num_rest = 10
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbolPrev = symbols[i-1]
            
            n = list(range(0, symbol+1))
            ctx_ids = ctx_ids_symbols_order1_tu(n, symbolPrev, num_rest, symbol_max)
            ctx_ids2 = cabac.getContextIdsSymbolOrder1TU(symbolPrev, num_rest, symbol_max, symbol+1)
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

if __name__ == "__main__":
    unittest.main()
