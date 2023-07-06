import numpy as np

def ctx_ids_bin_pos(n, n_rst=10):
    # n is the position of the to-be coded bin in the bin string (of the to be coded symbol)
    # bins with n < n_rst are coded with ctx model, all other bins are coded with the same context.
    # returns the context id
    #
    # For bins at position n < n_rst, the ctx_id is computed as follows:
    # ctx_id:                       meaning:
    # 0 ... n_rst-1:                position n < n_rst
    # n_rst:                        position n >= n_rst: rest case with single context

    n = np.asarray(n)
    ctx_id = np.zeros(n.shape, dtype=np.int32)

    # binary mask
    mask_rst = n < n_rst  # mask for rest case

    # bin position n < n_rst
    ctx_id[mask_rst] = n[mask_rst]

    # bin position n >= n_rst
    # rst case, independent of position n
    ctx_id[~mask_rst] = n_rst

    return ctx_id


def ctx_id_bin_pos(n, n_rst=10):
    if n < n_rst: # bin position n < n_rst
        ctx_id = n
    else:  # rst case, independent of position n
        ctx_id =  n_rst
    
    return ctx_id


def ctx_ids_bins_order_1_tu(n, prev_symbol=0, n_rst=10):
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


def ctx_id_bins_order_1_tu(n, prev_symbol=0, n_rst=10):

    ctx_id = 0

    if n < n_rst:
        if n > prev_symbol:
            ctx_id = 0*n_rst + n  # previously coded bin at same position not available
        elif n < prev_symbol:
            ctx_id = 1*n_rst + n  # previously coded bin 0 at same position n
        else:
            ctx_id = 2*n_rst + n  # previously coded bin 1 at same position n
    else:  # rst case, independent of position n
        ctx_id = 3*n_rst

    return ctx_id


def ctx_ids_bins_order_n_tu(order, n, prev_symbols=0, rest_pos=10):
    # n is the position of the to-be coded bin in the bin string (of the to be coded symbol)
    # prev_symbols is a list of previously coded symbols
    # bins with n < rest_pos are coded with ctx model, all other bins are coded with the same context.

    n = np.asarray(n)
    prev_symbols = np.asarray(prev_symbols)
    ctx_id = np.zeros(n.shape, dtype=np.int32)

    # binary mask
    mask_not_rest = n < rest_pos  # mask for not-rest case

    offset = rest_pos
    for o in range(0, order):
        ctx_id_current = np.zeros_like(n)

        mask_prev_symbol_na = mask_not_rest & (n > prev_symbols[o])  # mask for previous bin not available
        mask_prev_symbol_v0 = mask_not_rest & (n < prev_symbols[o])  # mask for previous bin 0
        mask_prev_symbol_v1 = mask_not_rest & (n == prev_symbols[o])

        ctx_id_current[mask_prev_symbol_na] = 0
        ctx_id_current[mask_prev_symbol_v0] = 1
        ctx_id_current[mask_prev_symbol_v1] = 2

        # bin position n < rest_pos
        ctx_id[mask_not_rest] += ctx_id_current[mask_not_rest] *offset
        offset *= 3

    ctx_id[mask_not_rest] += n[mask_not_rest]
    # bin position n >= rest_pos
    # rst case, independent of position n
    ctx_id[~mask_not_rest] = (3**order)*rest_pos

    return ctx_id


def ctx_id_bins_order_n_tu(order, n, prev_symbols=0, rest_pos=10):

    if n < rest_pos:
        ctx_id = 0
        offset = rest_pos
        for o in range(0, order):

            if n > prev_symbols[o]:  # previously coded bin at same position not available
                ctx_id_current = 0
            elif n < prev_symbols[o]:  # previously coded bin 0 at same position n
                ctx_id_current = 1
            else:  # previously coded bin 1 at same position n
                ctx_id_current = 2 
            
            ctx_id += ctx_id_current*offset
            offset *= 3
        # add bin position n
        ctx_id += n

    else: # rst case, independent of position n
        ctx_id = (3**order)*rest_pos

    return ctx_id


def ctx_ids_symbol_order_1_tu(n, prev_symbol=0, rest_pos=10, symbol_max=32):
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


def ctx_id_symbol_order_1_tu(n, prev_symbol=0, rest_pos=10, symbol_max=32):

    if n < rest_pos:  # bin position n < rest_pos
        # clip previous symbol to symbol_max
        prev_symbol0 = prev_symbol if prev_symbol <= symbol_max else symbol_max
        ctx_id = prev_symbol0*rest_pos + n
    else:  # rst case, independent of position n
        ctx_id = (symbol_max + 1)*rest_pos

    return ctx_id


def ctx_ids_symbol_order_n_tu(order, n, prev_symbols=0, rest_pos=10, symbol_max=32):

    n = np.asarray(n)
    prev_symbols = np.asarray(prev_symbols)
    ctx_id = np.zeros(n.shape, dtype=np.int32)

    # binary mask
    mask_not_rest = n < rest_pos  # mask for not-rest case

    # clip previous symbols to symbol_max
    prev_symbols0 = prev_symbols
    prev_symbols0[prev_symbols > symbol_max] = symbol_max

    offset = rest_pos
    for o in range(0, order):
        # bin position n < rest_pos
        ctx_id[mask_not_rest] = ctx_id[mask_not_rest] + (prev_symbols0[o])*offset
        offset *= (symbol_max+1)

    ctx_id[mask_not_rest] = ctx_id[mask_not_rest] + n[mask_not_rest]
    # bin position n >= rest_pos
    # rst case, independent of position n
    ctx_id[~mask_not_rest] = ((symbol_max+1)**order)*rest_pos

    return ctx_id


def ctx_id_symbol_order_n_tu(order, n, prev_symbols=0, rest_pos=10, symbol_max=32):

    
    prev_symbols = np.asarray(prev_symbols)
    
    if n < rest_pos:  # bin position n < rest_pos
        # clip previous symbols to symbol_max
        prev_symbols0 = prev_symbols
        prev_symbols0[prev_symbols > symbol_max] = symbol_max

        ctx_id = 0
        offset = rest_pos
        for o in range(0, order):
            ctx_id += prev_symbols0[o]*offset
            offset *= (symbol_max+1)
        
        # add bin position n
        ctx_id += n

    else:  # rst case, independent of position n
        ctx_id = ((symbol_max+1)**order)*rest_pos

    return ctx_id
