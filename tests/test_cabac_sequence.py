import unittest
import random
import cabac
import math


def create_random_symbols_geometric_distribution(num_values, p):
    import numpy as np

    return (np.random.default_rng(seed=0).geometric(p, num_values) - 1).tolist()


class MainTest(unittest.TestCase):

    
    def _call_cabac_symbols_bac_binposition(self, fun='EG0'):
        import numpy as np
        order = 1  # not used here
        rest_pos = 24
        symbol_max = 16
        num_max_val = 255
        num_max_prefix_val = int(math.floor(math.log2(num_max_val/(2**0) + 1)) + 1)
        num_bins = 8
        num_values = 10000
        num_bi_bins = 8
        ctx_id_offset = 0
        
        k = 1
        #symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        symbols = create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = [0,1,2,3,4,5,6,7]

        num_values = len(symbols)
        symbols = np.array(symbols)
        
        binParams = [num_max_val]
        ctxParams = [order, rest_pos, ctx_id_offset]
        
        if fun == 'BIBAC':
            binId = cabac.BinarizationId.BI
            ctxModelId = cabac.ContextModelId.BAC

            binParams = [num_bi_bins]

        elif fun == 'BIbinPosition':
            binId = cabac.BinarizationId.BI
            ctxModelId = cabac.ContextModelId.BINPOSITION

            binParams = [num_bi_bins]

        elif fun == 'TUBAC':
            binId = cabac.BinarizationId.TU
            ctxModelId = cabac.ContextModelId.BAC
        
        elif fun == 'TUbinPosition':
            binId = cabac.BinarizationId.TU
            ctxModelId = cabac.ContextModelId.BINPOSITION
        
        elif fun == 'EG0BAC':
            binId = cabac.BinarizationId.EG0
            ctxModelId = cabac.ContextModelId.BAC
        
        elif fun == "EG0binPosition":
            binId = cabac.BinarizationId.EG0
            ctxModelId = cabac.ContextModelId.BINPOSITION
        
        elif fun == 'EGkBAC':
            binId = cabac.BinarizationId.EGk
            ctxModelId = cabac.ContextModelId.BAC

            binParams = [num_max_val, k]

        elif fun == "EGkbinPosition":
            binId = cabac.BinarizationId.EGk
            ctxModelId = cabac.ContextModelId.BINPOSITION

            binParams = [num_max_val, k]

        num_ctxs = cabac.getNumContexts(binId, ctxModelId, binParams, ctxParams)

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()

        enc.encodeSymbols(symbols, binId, ctxModelId, binParams, ctxParams)
                    
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        decodedSymbols = dec.decodeSymbols(len(symbols), binId, ctxModelId, binParams, ctxParams)
        
        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols).all())


    def test_encode_symbols_bac_binposition(self):
        random.seed(0)
        print('test_encode_symbols_bac_binposition')
        funs = [
            'BIBAC', 'BIbinPosition',
            'TUBAC', 'TUbinPosition',
            'EG0BAC', 'EG0binPosition',
            'EGkBAC', 'EGkbinPosition']

        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac_symbols_bac_binposition(fun)


    def _call_cabac_symbols_order_n(self, fun='BIbinsOrderN', order=1):
        import numpy as np

        rest_pos = 24
        symbol_max = 16
        num_max_val = 255
        num_max_prefix_val = int(math.floor(math.log2(num_max_val/(2**0) + 1)) + 1)
        num_bins = 8
        num_values = 10000
        num_bi_bins = 8
        ctx_id_offset = 0
        
        k = 1
        #symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        symbols = create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = [0,1,2,3,4,5,6,7]

        num_values = len(symbols)
        symbols = np.array(symbols)
        
        binParams = [num_max_val]
        ctxParams = [order, rest_pos, ctx_id_offset]
        if fun == 'BIbinsOrderN':
            binId = cabac.BinarizationId.BI
            ctxModelId = cabac.ContextModelId.BINSORDERN

            binParams = [num_bi_bins]

        elif fun == 'TUbinsOrderN':
            binId = cabac.BinarizationId.TU
            ctxModelId = cabac.ContextModelId.BINSORDERN

        elif fun == 'TUsymbolOrderN':
            binId = cabac.BinarizationId.TU
            ctxModelId = cabac.ContextModelId.SYMBOLORDERN

            ctxParams = [order, rest_pos, ctx_id_offset, symbol_max]

        elif fun == 'EG0binsOrderN':
            binId = cabac.BinarizationId.EG0
            ctxModelId = cabac.ContextModelId.BINSORDERN

        elif fun == 'EG0symbolOrderN':
            binId = cabac.BinarizationId.EG0
            ctxModelId = cabac.ContextModelId.SYMBOLORDERN

            ctxParams = [order, rest_pos, ctx_id_offset, symbol_max]

        elif fun == 'EGkbinsOrderN':
            binId = cabac.BinarizationId.EGk
            ctxModelId = cabac.ContextModelId.BINSORDERN

            binParams = [num_max_val, k]

        elif fun == 'EGksymbolOrderN':
            binId = cabac.BinarizationId.EGk
            ctxModelId = cabac.ContextModelId.SYMBOLORDERN

            binParams = [num_max_val, k]
            ctxParams = [order, rest_pos, 0, symbol_max]

        num_ctxs = cabac.getNumContexts(binId, ctxModelId, binParams, ctxParams)

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()

        enc.encodeSymbols(symbols, binId, ctxModelId, binParams, ctxParams)
                    
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        decodedSymbols = dec.decodeSymbols(len(symbols), binId, ctxModelId, binParams, ctxParams)
        
        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols).all())


    def test_encode_symbols_order_n(self):
        random.seed(0)
        print('test_encode_symbols_order_n')
        funs = [
            'BIbinsOrderN',
            'TUbinsOrderN', 'TUsymbolOrderN',
            'EG0binsOrderN', 'EG0symbolOrderN',
            'EGkbinsOrderN', 'EGksymbolOrderN'
        ]
        #funs = ['EGkbinsOrderN', 'EGksymbolOrderN']

        for fun in funs:
            for order in [1, 2, 3]:
                print('Testing function: ' + fun + ' with order ' + str(order))
                self._call_cabac_symbols_order_n(fun, order)

    
    def _call_cabac_symbols_bypass(self, fun='EG0'):
        import numpy as np

        num_max_val = 255
        num_values = 10000
        num_bi_bins = 8
        
        k = 1
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        symbols = create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = [0,1,2,3,4,5,6,7]

        num_values = len(symbols)
        symbols = np.array(symbols)
        
        binParams = [num_max_val]
        if fun == 'BI':
            binId = cabac.BinarizationId.BI
            binParams = [num_bi_bins]

        elif fun == 'TU':
            binId = cabac.BinarizationId.TU

        elif fun == 'EG0':
            binId = cabac.BinarizationId.EG0

        elif fun == 'EGk':
            binId = cabac.BinarizationId.EGk
            binParams = [num_max_val, k]

        num_ctxs = 0

        enc = cabac.cabacSimpleSequenceEncoder()
        enc.initCtx(num_ctxs, 0.5, 8)  # initialize one context with p1 = 0.5 and shift_idx = 8
        enc.start()

        enc.encodeSymbolsBypass(symbols, binId, binParams)
                    
        enc.encodeBinTrm(1)
        enc.finish()
        enc.writeByteAlignment()

        bs = enc.getBitstream()

        # Decode
        dec = cabac.cabacSimpleSequenceDecoder(bs)
        dec.initCtx(num_ctxs, 0.5, 8) 
        dec.start()

        decodedSymbols = dec.decodeSymbolsBypass(len(symbols), binId, binParams)
        
        dec.decodeBinTrm()
        dec.finish()
        print('Bitstream length: ' + str(len(bs)))

        self.assertTrue((decodedSymbols == symbols).all())


    def test_encode_symbols_bypass_joint_fun(self):
        random.seed(0)
        print('test_encode_symbols_bypass_joint_fun')
        funs = ['BI', 'TU', 'EG0', 'EGk']
        
        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac_symbols_bypass(fun)

if __name__ == "__main__":
    unittest.main()
