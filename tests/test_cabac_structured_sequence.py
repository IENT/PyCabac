import unittest
import random
import cabac

import tests.utils.symbolgenerator as symbolgenerator


class MainTest(unittest.TestCase):

    def _call_cabac_symbols_order_n(self, fun='BIbinsOrderN', ctx_order=1):
        import numpy as np

        ctx_rest_pos = 12
        ctx_id_offset = 0
        symbol_max = 4
        num_max_val = 255
        # num_values = 10000
        num_bi_bins = 8

        k = 1

        # symbols = symbolgenerator.random_geometric(num_values, 0.05)
        symbols = symbolgenerator.load_block()
        symbols = np.abs(symbols)
        # symbols = list(range(0,8))

        # num_values = len(symbols)
        symbols = np.array(symbols)

        (block_len, _, _, num_channels) = symbols.shape

        bin_params = [num_max_val]
        ctx_params = [ctx_order, ctx_rest_pos, ctx_id_offset, symbol_max]

        if fun == 'TUBAC':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.BAC

        elif fun == 'TUbinsOrderN':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.BINSORDERN

        elif fun == 'TUsymbolOrderN':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.SYMBOLORDERN

        elif fun == 'TUsumOrderN':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.SUMORDERN

        elif fun == 'EGkBAC':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.BAC

        elif fun == 'EGkbinsOrderN':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.BINSORDERN

            bin_params = [num_max_val, k]

        elif fun == 'EGksymbolOrderN':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.SYMBOLORDERN

            bin_params = [num_max_val, k]

        elif fun == 'EGksumOrderN':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.SUMORDERN

            bin_params = [num_max_val, k]

        else:
            raise ValueError('Unknown function: ' + fun)

        num_ctxs = cabac.getNumContexts(
            bin_id, ctx_model_id, bin_params, ctx_params
        )

        p1_init = 0.5
        shift_idx = 8

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, p1_init, shift_idx)
        enc.start()
        for ci in range(num_channels):
            symbols_c = symbols[:, :, :, ci].flatten()
            enc.encodeSymbols(
                symbols_c, bin_id, ctx_model_id, bin_params, ctx_params
            )

        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, p1_init, shift_idx)
        dec.start()
        decodedSymbols = np.zeros_like(symbols, dtype=np.int32)
        for ci in range(num_channels):
            decodedSymbols_c = dec.decodeSymbols(
                int(block_len**3), bin_id, ctx_model_id, bin_params, ctx_params
            )
            decodedSymbols[:, :, :, ci] = decodedSymbols_c.reshape(
                (block_len, block_len, block_len)
            )

        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols).all())
        return len(bs)

    def test_encode_symbols_order_n(self):
        import numpy as np
        random.seed(0)
        print('test_encode_symbols_order_n')
        funs = [
            'TUBAC', 'TUbinsOrderN', 'TUsymbolOrderN','TUsumOrderN',
            'EGkBAC', 'EGkbinsOrderN', 'EGksymbolOrderN', 'EGksumOrderN'
        ]
        # funs = ['TUsumOrderN', 'EGksymbolOrderN']
        bs_lens = np.zeros((len(funs), 2), dtype=np.int32)
        for fun in funs:
            for order in [1, 2]:
                print('Testing function: ' + fun + ' with order ' + str(order))
                bs_len = self._call_cabac_symbols_order_n(fun, order)
                bs_lens[funs.index(fun), order - 1] = bs_len

        # print minimal bs_len
        np.argmin(bs_lens)

    def _call_cabac_symbols_symbol_pos(self, fun='BIbinsOrderN'):
        import numpy as np

        ctx_order = 1
        ctx_rest_pos = 24
        ctx_id_offset = 0
        symbol_max = 16
        num_max_val = 255
        # num_values = 10000
        num_bi_bins = 8
        symbol_pos_mode = 1

        k = 1

        # symbols = symbolgenerator.random_geometric(num_values, 0.05)
        symbols = symbolgenerator.load_block()
        symbols = np.abs(symbols)
        # symbols = list(range(0,8))

        # num_values = len(symbols)
        symbols = np.array(symbols, dtype='uint8')

        (block_len, _, _, num_channels) = symbols.shape

        bin_params = [num_max_val]
        ctx_params = [ctx_order, ctx_rest_pos, ctx_id_offset, symbol_max, symbol_pos_mode, 1, 2, 3]

        if fun == 'TUBinPos':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.BINPOSITION

        elif fun == 'TUSymPos':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.SYMBOLPOSITION

        elif fun == 'TUBinSymPos':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.BINSYMBOLPOSITION

        elif fun == 'EGkBinPos':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.BINPOSITION

            bin_params = [num_max_val, k]

        elif fun == 'EGkSymPos':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.SYMBOLPOSITION

            bin_params = [num_max_val, k]

        elif fun == 'EGkBinSymPos':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.BINSYMBOLPOSITION

            bin_params = [num_max_val, k]

        else:
            raise ValueError('Unknown function: ' + fun)

        num_ctxs = cabac.getNumContexts(
            bin_id, ctx_model_id, bin_params, ctx_params
        )

        p1_init = 0.5
        shift_idx = 8

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, p1_init, shift_idx)
        enc.start()
        # for ci in range(num_channels):
        #    symbols_c = symbols[:, :, :, ci].flatten()
        #    enc.encodeSymbols(
        #        symbols_c, bin_id, ctx_model_id, bin_params, ctx_params
        #    )
        symbols_reshaped = symbols.reshape((block_len**3, num_channels))
        for xyzi in range(symbols_reshaped.shape[0]):
            symbols_c = symbols_reshaped[xyzi, :]
            enc.encodeSymbols(
                symbols_c, bin_id, ctx_model_id, bin_params, ctx_params
            )

        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, p1_init, shift_idx)
        dec.start()
        # decodedSymbols = np.zeros_like(symbols, dtype=np.int32)
        # for ci in range(num_channels):
        #     decodedSymbols_c = dec.decodeSymbols(
        #         int(block_len**3), bin_id, ctx_model_id, bin_params, ctx_params
        #     )
        #     decodedSymbols[:, :, :, ci] = decodedSymbols_c.reshape(
        #         (block_len, block_len, block_len)
        #     )

        decodedSymbols_reshaped = np.zeros_like(symbols_reshaped, dtype=np.int32)
        for xyzi in range(block_len**3):
            decodedSymbols_c = dec.decodeSymbols(
                num_channels, bin_id, ctx_model_id, bin_params, ctx_params
            )
            decodedSymbols_reshaped[xyzi, :] = decodedSymbols_c

        decodedSymbols = decodedSymbols_reshaped.reshape(
            (block_len, block_len, block_len, num_channels)
        )

        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols).all())
        return len(bs)

    def test_encode_symbols_symbol_pos(self):
        import numpy as np
        random.seed(0)
        print('test_encode_symbols_order_n')
        funs = [
            'TUBinPos', 'TUSymPos', 'TUBinSymPos',
            'EGkBinPos', 'EGkSymPos', 'EGkBinSymPos'
        ]
        # funs = ['TUsumOrderN', 'EGksymbolOrderN']
        bs_lens = np.zeros((len(funs)), dtype=np.int32)
        for fun in funs:
            print('Testing function: ' + fun)
            bs_len = self._call_cabac_symbols_symbol_pos(fun)
            bs_lens[funs.index(fun)] = bs_len

        # print minimal bs_len
        print('Summary')
        print(np.vstack((funs, bs_lens)).T)

    def _call_cabac_symbols_mask(self, fun='BIbinsOrderN'):
        import numpy as np

        ctx_order = 1
        ctx_rest_pos = 24
        ctx_id_offset = 0
        symbol_max = 16
        num_max_val = 255
        # num_values = 10000
        num_bi_bins = 8
        symbol_pos_mode = 1

        k = 1

        # symbols = symbolgenerator.random_geometric(num_values, 0.05)
        symbols = symbolgenerator.load_block()

        (block_len, _, _, num_channels) = symbols.shape

        bin_params = [num_max_val]
        ctx_params = [ctx_order, ctx_rest_pos, ctx_id_offset, symbol_max, symbol_pos_mode, 1, 2, 3]

        bin_id_mask = cabac.BinarizationId.BI
        bin_params_mask = [1, k]
        if fun == 'TUBinPos':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.BINPOSITION

        elif fun == 'TUSymPos':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.SYMBOLPOSITION

        elif fun == 'TUBinSymPos':
            bin_id = cabac.BinarizationId.TU
            ctx_model_id = cabac.ContextModelId.BINSYMBOLPOSITION

        elif fun == 'EGkBinPos':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.BINPOSITION

            bin_params = [num_max_val, k]

        elif fun == 'EGkSymPos':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.SYMBOLPOSITION

            bin_params = [num_max_val, k]

        elif fun == 'EGkBinSymPos':
            bin_id = cabac.BinarizationId.EGk
            ctx_model_id = cabac.ContextModelId.BINSYMBOLPOSITION

            bin_params = [num_max_val, k]

        else:
            raise ValueError('Unknown function: ' + fun)

        num_ctxs = cabac.getNumContexts(
            bin_id, ctx_model_id, bin_params, ctx_params
        )

        p1_init = 0.5
        shift_idx = 8

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, p1_init, shift_idx)
        enc.start()
        # for ci in range(num_channels):
        #    symbols_c = symbols[:, :, :, ci].flatten()
        #    enc.encodeSymbols(
        #        symbols_c, bin_id, ctx_model_id, bin_params, ctx_params
        #    )
        global_zero_mask = np.sum(np.abs(symbols), axis=(0, 1, 2)) == 0
        enc.encodeSymbolsBypass(
            global_zero_mask.flatten(), bin_id_mask, bin_params_mask
        )
        for ci in range(num_channels):
            if global_zero_mask[ci]:
                continue
            symbols_c = symbols[:, :, :, ci]

            mask_signif_c = np.abs(symbols_c) > 0
            symbols_sign_c = symbols_c[mask_signif_c] < 0
            symbols_abs_c = np.abs(symbols_c[mask_signif_c]) - 1

            enc.encodeSymbols(
                mask_signif_c.flatten(), bin_id_mask, ctx_model_id, bin_params_mask, ctx_params
            )
            enc.encodeSymbolsBypass(
                symbols_sign_c.flatten(), bin_id_mask, bin_params_mask
            )

            symbols_abs_c_structured = np.zeros_like(symbols_c)
            symbols_abs_c_structured[mask_signif_c] = symbols_abs_c
            enc.encodeSymbols(
                symbols_abs_c_structured.flatten(),
                bin_id, ctx_model_id, bin_params, ctx_params, [], mask_signif_c.flatten()
            )

        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, p1_init, shift_idx)
        dec.start()
        decodedSymbols = np.zeros_like(symbols, dtype=np.int32)

        global_zero_mask_dec = dec.decodeSymbolsBypass(
            num_channels, bin_id_mask, bin_params_mask
        )

        for ci in range(num_channels):
            if global_zero_mask_dec[ci]:
                continue

            mask_signif_c_dec = dec.decodeSymbols(
                int(block_len**3), bin_id_mask, ctx_model_id, bin_params_mask, ctx_params
            )
            mask_signif_c_dec = mask_signif_c_dec.astype(np.bool)
            mask_signif_c_dec = mask_signif_c_dec.reshape((block_len, block_len, block_len))

            symbols_sign_c_dec = dec.decodeSymbolsBypass(
                int(np.sum(mask_signif_c_dec)), bin_id_mask, bin_params_mask
            )

            symbols_abs_c_dec = dec.decodeSymbols(
                 int(block_len**3), bin_id, ctx_model_id, bin_params, ctx_params, [], mask_signif_c_dec.flatten()
            )

            symbols_abs_c_dec = symbols_abs_c_dec.reshape((block_len, block_len, block_len))

            symbols_abs_c_dec[mask_signif_c_dec] = symbols_abs_c_dec[mask_signif_c_dec] + 1

            symbols_c_dec = symbols_abs_c_dec.astype(np.int32)
            tmp_sign = np.zeros_like(symbols_c_dec, dtype=np.bool)
            tmp_sign[mask_signif_c_dec] = symbols_sign_c_dec
            symbols_c_dec[tmp_sign] = -symbols_c_dec[tmp_sign]
            decodedSymbols[:, :, :, ci] = symbols_c_dec

        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols).all())
        return len(bs)

    def test_encode_symbols_mask(self):
        import numpy as np
        random.seed(0)
        print('test_encode_symbols_mask')
        funs = [
            'TUBinPos', 'TUSymPos', 'TUBinSymPos',
            'EGkBinPos', 'EGkSymPos', 'EGkBinSymPos'
        ]
        # funs = ['TUsumOrderN', 'EGksymbolOrderN']
        bs_lens = np.zeros((len(funs)), dtype=np.int32)
        for fun in funs:
            print('Testing function: ' + fun)
            bs_len = self._call_cabac_symbols_mask(fun)
            bs_lens[funs.index(fun)] = bs_len

        # print minimal bs_len
        print('Summary')
        print(np.vstack((funs, bs_lens)).T)


if __name__ == "__main__":
    unittest.main()
