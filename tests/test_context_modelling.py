import unittest
import random
import cabac
import math
import numpy as np

import tests.utils.contextselector as contextselector



class MainTest(unittest.TestCase):
    
    def test_ctx_selection_bins_order_n(self):
        
        
        num_max_val = 255
        num_values = 1000
        symbol_max = 15
        order = 2
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        
        symbolsPrev = [0] * order
        num_rest = 10

        for i, symbol in enumerate(symbols):
            if i > 0:
                symbolsPrev[0] = symbols[i-1]
            if order > 1 and i > 1:
                symbolsPrev[1] = symbols[i-2]
            if order > 2 and i > 2:
                symbolsPrev[2] = symbols[i-3]
            
            n = list(range(0, symbol+1))
            ctx_ids = contextselector.ctx_ids_bins_order_n_tu(order, n, symbolsPrev, num_rest)
            ctx_ids2 = cabac.getContextIdsBinsOrderNTU(order, symbolsPrev, num_rest, symbol+1)
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

    def test_ctx_selection_symbols_order_n(self):
        
        num_max_val = 255
        num_values = 1000
        symbol_max = 15
        order = 2
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        
        symbolsPrev = [0] * order
        num_rest = 10

        for i, symbol in enumerate(symbols):
            if i > 0:
                symbolsPrev[0] = symbols[i-1]
            if order > 1 and i > 1:
                symbolsPrev[1] = symbols[i-2]
            if order > 2 and i > 2:
                symbolsPrev[2] = symbols[i-3]
            
            n = list(range(0, symbol+1))
            ctx_ids = contextselector.ctx_ids_symbols_order_n_tu(order, n, symbolsPrev, num_rest, symbol_max)
            ctx_ids2 = cabac.getContextIdsSymbolOrderNTU(order, symbolsPrev, num_rest, symbol_max, symbol+1)
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

    def test_ctx_selection_bins_order_1(self):
        
        num_max_val = 255
        num_values = 1000
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        
        symbolPrev = 0
        num_rest = 10
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbolPrev = symbols[i-1]
            
            n = list(range(0, symbol+1))
            ctx_ids = contextselector.ctx_ids_bins_order_1_tu(n, symbolPrev, num_rest)
            ctx_ids2 = cabac.getContextIdsBinsOrder1TU(symbolPrev, num_rest, symbol+1)
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

    def test_ctx_selection_symbols_order_1(self):
                
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
            ctx_ids = contextselector.ctx_ids_symbols_order_1_tu(n, symbolPrev, num_rest, symbol_max)
            ctx_ids2 = cabac.getContextIdsSymbolOrder1TU(symbolPrev, num_rest, symbol_max, symbol+1)
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

if __name__ == "__main__":
    unittest.main()
