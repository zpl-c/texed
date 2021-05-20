
int texed_find_op(int kind) {
    for (int i = 0; i < DEF_OPS_LEN; i += 1) {
        if (default_ops[i].kind == kind) {
            return i;
        }
    }
    return -1;
}

void texed_add_op(int kind) {
    int idx = texed_find_op(kind);
    assert(idx >= 0);
    td_op *dop = &default_ops[idx];
    
    td_op op = {
        .kind = dop->kind,
        .name = dop->name,
        .is_locked = dop->is_locked,
        .is_fixed = dop->is_fixed,
        .num_params = dop->num_params,
        .params = (td_param*)zpl_malloc(sizeof(td_param)*dop->num_params)
    };
    
    zpl_memcopy(op.params, dop->params, sizeof(td_param)*dop->num_params);
    
    //TODO(zaklaus): weird stuff down there
    //zpl_array_append_at(ctx.ops, op, ctx.selected_op+1);
    int ind = ctx.selected_op+1;
    do {                            
        if (ind >= zpl_array_count(ctx.ops)) { zpl_array_append(ctx.ops, op); break; }
        if (zpl_array_capacity(ctx.ops) < zpl_array_count(ctx.ops) + 1) zpl_array_grow(ctx.ops, 0);
        zpl_memmove(&(ctx.ops)[ind + 1], (ctx.ops + ind), zpl_size_of(td_op) * (zpl_array_count(ctx.ops) - ind));
        ctx.ops[ind] = op;
        zpl_array_count(ctx.ops)++;
    } while (0);
    ctx.selected_op++;
    
    texed_repaint_preview();
}

void texed_clone_op(int ind, td_op *dop) {
    assert(dop);
    
    td_op op = {
        .kind = dop->kind,
        .name = dop->name,
        .is_locked = dop->is_locked,
        .is_hidden = dop->is_hidden,
        .num_params = dop->num_params,
        .params = (td_param*)zpl_malloc(sizeof(td_param)*dop->num_params)
    };
    
    zpl_memcopy(op.params, dop->params, sizeof(td_param)*dop->num_params);
    
    //TODO(zaklaus): weird stuff down there
    //zpl_array_append_at(ctx.ops, op, ctx.selected_op+1);
    ind += 1;
    do {                            
        if (ind >= zpl_array_count(ctx.ops)) { zpl_array_append(ctx.ops, op); break; }
        if (zpl_array_capacity(ctx.ops) < zpl_array_count(ctx.ops) + 1) zpl_array_grow(ctx.ops, 0);
        zpl_memmove(&(ctx.ops)[ind + 1], (ctx.ops + ind), zpl_size_of(td_op) * (zpl_array_count(ctx.ops) - ind));
        ctx.ops[ind] = op;
        zpl_array_count(ctx.ops)++;
    } while (0);
    ctx.selected_op = ind;
    
    texed_repaint_preview();
}

void texed_swp_op(int idx, int idx2) {
    assert(idx >= 0 && idx < (int)zpl_array_count(ctx.ops));
    assert(idx2 >= 0 && idx2 < (int)zpl_array_count(ctx.ops));
    
    td_op tmp = ctx.ops[idx2];
    ctx.ops[idx2] = ctx.ops[idx];
    ctx.ops[idx] = tmp;
    if (idx == ctx.selected_op) ctx.selected_op = idx2;
    texed_repaint_preview();
}

void texed_rem_op(int idx) {
    assert(idx >= 0 && idx < (int)zpl_array_count(ctx.ops));
    zpl_mfree(ctx.ops[idx].params);
    zpl_array_remove_at(ctx.ops, idx);
    
    if (zpl_array_count(ctx.ops) > 0 && idx <= ctx.selected_op) ctx.selected_op -= 1;
    texed_repaint_preview();
}
