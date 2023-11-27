void PackUnix::pack(OutputFile *fo)
{
    Filter ft(ph.level);
    ft.addvalue = 0;
    b_len = 0;
    progid = 0;
 
    // set options
    blocksize = opt->o_unix.blocksize;
    if (blocksize <= 0)
        blocksize = BLOCKSIZE;
    if ((off_t)blocksize > file_size)
        blocksize = file_size;
 
    // init compression buffers
    ibuf.alloc(blocksize);
    obuf.allocForCompression(blocksize);
 
    fi->seek(0, SEEK_SET);
    pack1(fo, ft);  // generate Elf header, etc.
 
    p_info hbuf;
    set_te32(&hbuf.p_progid, progid);
    set_te32(&hbuf.p_filesize, file_size);
    set_te32(&hbuf.p_blocksize, blocksize);
    fo->write(&hbuf, sizeof(hbuf));
 
    // append the compressed body
    if (pack2(fo, ft)) {
        // write block end marker (uncompressed size 0)
        b_info hdr; memset(&hdr, 0, sizeof(hdr));
        set_le32(&hdr.sz_cpr, UPX_MAGIC_LE32);
        fo->write(&hdr, sizeof(hdr));
    }
 
    pack3(fo, ft);  // append loader
 
    pack4(fo, ft);  // append PackHeader and overlay_offset; update Elf header
 
    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}
