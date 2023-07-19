import unittest
import random
import cabac

import numpy as np

import tests.utils.symbolgenerator as symbolgenerator
import tests.utils.contextselector as contextselector


class MainTest(unittest.TestCase):

    def test_ctx_selection_bins_order_n(self):
        max_val = 255
        num_values = 1000

        bin_id = cabac.BinarizationId.TU
        bin_params = [max_val]
        ctx_model_id = cabac.ContextModelId.BINSORDERN
        ctx_symbol_max = 15
        ctx_order = 2
        ctx_rest_pos = 10
        ctx_offset = 0
        ctx_params = [ctx_order, ctx_rest_pos, ctx_offset, ctx_symbol_max]

        symbols = symbolgenerator.random_uniform(
            num_values=num_values, max_val=max_val
        )
        symbols_prev = np.zeros(ctx_order)

        for i, symbol in enumerate(symbols):
            if i > 0:
                symbols_prev[0] = symbols[i-1]
            if ctx_order > 1 and i > 1:
                symbols_prev[1] = symbols[i-2]
            if ctx_order > 2 and i > 2:
                symbols_prev[2] = symbols[i-3]

            n = list(range(0, symbol+1))
            ctx_ids = contextselector.ctx_ids_bins_order_n_tu(
                ctx_order, n, symbols_prev, ctx_rest_pos
            )
            ctx_ids2 = cabac.getContextIds(
                i, symbols_prev, bin_id, ctx_model_id,
                bin_params, ctx_params, len(n)
            )
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

    def test_ctx_selection_symbols_order_n(self):

        max_val = 255
        num_values = 1000

        bin_id = cabac.BinarizationId.TU
        bin_params = [max_val]
        ctx_model_id = cabac.ContextModelId.SYMBOLORDERN

        ctx_order = 2
        ctx_rest_pos = 10
        ctx_offset = 0
        ctx_symbol_max = 15
        ctx_params = [ctx_order, ctx_rest_pos, ctx_offset, ctx_symbol_max]

        symbols = symbolgenerator.random_uniform(
            num_values=num_values, max_val=max_val
        )

        symbols_prev = np.zeros(ctx_order)
        for i, symbol in enumerate(symbols):
            if i > 0:
                symbols_prev[0] = symbols[i-1]
            if ctx_order > 1 and i > 1:
                symbols_prev[1] = symbols[i-2]
            if ctx_order > 2 and i > 2:
                symbols_prev[2] = symbols[i-3]

            n = list(range(0, symbol+1))
            ctx_ids = contextselector.ctx_ids_symbol_order_n_tu(
                ctx_order, n, symbols_prev, ctx_rest_pos, ctx_symbol_max
            )
            ctx_ids2 = cabac.getContextIds(
                i, symbols_prev, bin_id, ctx_model_id,
                bin_params, ctx_params, len(n)
            )
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

    def test_ctx_selection_bins_order_1(self):

        max_val = 255
        num_values = 1000
        symbols = symbolgenerator.random_uniform(
            num_values=num_values, max_val=max_val
        )

        symbolPrev = 0
        num_rest = 10
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbolPrev = symbols[i-1]

            n = list(range(0, symbol+1))
            ctx_ids = contextselector.ctx_ids_bins_order_1_tu(
                n, symbolPrev, num_rest
            )
            ctx_ids2 = cabac.getContextIdsBinsOrder1TU(
                symbolPrev, num_rest, symbol+1
            )
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())

    def test_ctx_selection_symbols_order_1(self):

        max_val = 255
        num_values = 1000
        symbol_max = 15
        symbols = symbolgenerator.random_uniform(
            num_values=num_values, max_val=max_val
        )

        symbolPrev = 0
        num_rest = 10
        for i, symbol in enumerate(symbols):
            if i != 0:
                symbolPrev = symbols[i-1]

            n = list(range(0, symbol+1))
            ctx_ids = contextselector.ctx_ids_symbol_order_1_tu(
                n, symbolPrev, num_rest, symbol_max
            )
            ctx_ids2 = cabac.getContextIdsSymbolOrder1TU(
                symbolPrev, num_rest, symbol_max, symbol+1
            )
            self.assertTrue((ctx_ids == np.array(ctx_ids2)).all())


if __name__ == "__main__":
    unittest.main()
