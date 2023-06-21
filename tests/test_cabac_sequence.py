import unittest
import random
import cabac
import math


def create_random_symbols_geometric_distribution(num_values, p):
    import numpy as np

    return (np.random.default_rng(seed=0).geometric(p, num_values) - 1).tolist()


class MainTest(unittest.TestCase):
    

    def _call_cabac_symbols(self, fun='EG0'):
        import numpy as np
        rest_pos = 24
        symbol_max = 16
        num_max_val = 255
        num_max_prefix_val = int(math.floor(math.log2(num_max_val/(2**0) + 1)) + 1)
        num_bins = 8
        num_values = 10000
        
        k = 1
        symbols = [random.randint(0, num_max_val) for _ in range(0, num_values)]
        symbols = create_random_symbols_geometric_distribution(num_values, 0.05)
        #symbols = [0,1,2,3,4,5,6,7]

        num_values = len(symbols)
        symbols = np.array(symbols)
        
        if fun == 'TUbinsOrder1':
            binId = cabac.BinarizationId.TU
            ctxModelId = cabac.ContextModelId.BINSORDER1

            binParams = [num_max_val]
            ctxParams = [rest_pos]
            num_ctxs = 3*rest_pos + 1 # binsOrder1

        elif fun == 'TUsymbolOrder1':
            binId = cabac.BinarizationId.TU
            ctxModelId = cabac.ContextModelId.SYMBOLORDER1

            binParams = [num_max_val]
            ctxParams = [rest_pos, symbol_max]
            num_ctxs = (symbol_max+2)*rest_pos + 1  # symbolOrder1

        elif fun == 'EG0binsOrder1':
            binId = cabac.BinarizationId.EG0
            ctxModelId = cabac.ContextModelId.BINSORDER1

            binParams = [num_max_val]
            ctxParams = [rest_pos]
            num_ctxs = 3*rest_pos + 1 # binsOrder1

        elif fun == 'EG0symbolOrder1':
            binId = cabac.BinarizationId.EG0
            ctxModelId = cabac.ContextModelId.SYMBOLORDER1

            binParams = [num_max_val]
            ctxParams = [rest_pos, symbol_max]
            num_ctxs = (symbol_max+2)*rest_pos + 1  # symbolOrder1

        elif fun == 'EGkbinsOrder1':
            binId = cabac.BinarizationId.EGk
            ctxModelId = cabac.ContextModelId.BINSORDER1

            binParams = [num_max_val, k]
            ctxParams = [rest_pos]
            num_ctxs = 3*rest_pos + 1 # binsOrder1

        elif fun == 'EGksymbolOrder1':
            binId = cabac.BinarizationId.EGk
            ctxModelId = cabac.ContextModelId.SYMBOLORDER1

            binParams = [num_max_val, k]
            ctxParams = [rest_pos, symbol_max]
            num_ctxs = (symbol_max+2)*rest_pos + 1  # symbolOrder1

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


    def test_encode_symbols_joint_fun(self):
        random.seed(0)
        print('test_encode_symbols_symbol_order1')
        funs = ['TUbinsOrder1', 'TUsymbolOrder1', 'EG0binsOrder1', 'EG0symbolOrder1', 'EGkbinsOrder1', 'EGksymbolOrder1']

        for fun in funs:
            print('Testing function: ' + fun)
            self._call_cabac_symbols(fun)

if __name__ == "__main__":
    unittest.main()
