
//~ NOTE(zaklaus): DATA SERIALISATION

#define ECOTEX_VERSION 3

#define UNPACK(kind) cw_unpack_next(&uc); assert(uc.item.type == kind);

void texed_load(void) {
    assert(ctx.filepath);
    zpl_printf("Loading %s ...\n", ctx.filepath);
    is_repaint_locked = true;
    texed_clear();
    uint32_t size = 0;
    uint8_t *databuf = LoadFileData(zpl_bprintf("art/%s.ecotex", ctx.filepath), &size); 
    
    cw_unpack_context uc;
    cw_unpack_context_init(&uc, databuf, (size_t)size, NULL);
    UNPACK(CWP_ITEM_POSITIVE_INTEGER);
    assert(uc.item.as.u64 == ECOTEX_VERSION);
    
    UNPACK(CWP_ITEM_POSITIVE_INTEGER);
    int selected_op = (int)uc.item.as.u64;
    
    UNPACK(CWP_ITEM_FLOAT);
    old_zoom = zoom = uc.item.as.real;
    
    UNPACK(CWP_ITEM_ARRAY);
    int arrsize = (int)uc.item.as.array.size;
    for (int i = 0; i < arrsize; i += 1) {
        UNPACK(CWP_ITEM_POSITIVE_INTEGER);
        int kind = (int)uc.item.as.u64;
        texed_add_op(kind);
        td_op *op = zpl_array_end(ctx.ops);
        UNPACK(CWP_ITEM_BOOLEAN);
        op->is_locked = uc.item.as.boolean;
        UNPACK(CWP_ITEM_BOOLEAN);
        op->is_hidden = uc.item.as.boolean;
        
        UNPACK(CWP_ITEM_ARRAY);
        int idx = texed_find_op(kind);
        op->num_params = default_ops[idx].num_params;
        op->params = zpl_malloc(sizeof(td_param)*op->num_params);
        op->num_gizmos = default_ops[idx].num_gizmos;
        op->gizmos = default_ops[idx].gizmos;
        int parmarrsize = (int)uc.item.as.array.size;
        for (int j = 0; j < parmarrsize; j += 1) {
            td_param *p = &op->params[j];
            UNPACK(CWP_ITEM_STR);
            zpl_memcopy(p->str, uc.item.as.str.start, uc.item.as.str.length);
            
            // NOTE(zaklaus): fix up other metadata
            p->name = default_ops[idx].params[j].name;
            p->kind = default_ops[idx].params[j].kind;
        }
        
        // NOTE(zaklaus): resolve missing params
        for (int j = parmarrsize; j < default_ops[idx].num_params; j += 1) {
            td_param *p = &op->params[j];
            p->name = default_ops[idx].params[j].name;
            p->kind = default_ops[idx].params[j].kind;
            zpl_strcpy(p->str, default_ops[idx].params[j].str);
        }
    }
    
    assert(uc.return_code == CWP_RC_OK);
    cw_unpack_next(&uc);
    assert(uc.return_code == CWP_RC_END_OF_INPUT);
    
    ctx.selected_op = selected_op;
    is_repaint_locked = false;
    texed_repaint_preview();
    UnloadFileData(databuf);
    ctx.is_saved = true;
}

void texed_save(void) {
    assert(ctx.filepath);
    zpl_printf("Saving %s ...\n", ctx.filepath);
    
    static uint8_t databuf[400000] = {0};
    
    cw_pack_context pc;
    cw_pack_context_init(&pc, databuf, sizeof(databuf), NULL);
    
    cw_pack_unsigned(&pc, ECOTEX_VERSION);
    cw_pack_unsigned(&pc, ctx.selected_op);
    cw_pack_float(&pc, zoom);
    
    cw_pack_array_size(&pc, zpl_array_count(ctx.ops));
    for (int i = 0; i < zpl_array_count(ctx.ops); i += 1) {
        td_op *op = &ctx.ops[i];
        cw_pack_unsigned(&pc, op->kind);
        cw_pack_boolean(&pc, (bool)op->is_locked);
        cw_pack_boolean(&pc, (bool)op->is_hidden);
        cw_pack_array_size(&pc, op->num_params);
        for (int j = 0; j < op->num_params; j += 1) {
            td_param *p = &op->params[j];
            cw_pack_str(&pc, p->str, zpl_strlen(p->str));
        }
    }
    
    SaveFileData(zpl_bprintf("art/%s.ecotex", ctx.filepath), databuf, pc.current - pc.start);
    ctx.is_saved = true;
}
