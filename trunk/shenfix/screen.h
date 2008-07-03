// $Id$
/********************************************************************
 * VMZip v1.0.0 (Aug/2005)
 * coded by El Bucanero
 *
 * Copyright (C) 2005 Damian Parrino <bucanero@fibertel.com.ar>
 * http://www.bucanero.com.ar/
 *
 ********************************************************************/

#define VMUSRC_X 260
#define VMUSRC_Y 140
#define VMUDST_X 510
#define VMUDST_Y 140
#define VMU_RGB 1, 1, 1
#define SAVES_X 255
#define SAVES_Y 140
#define SAVES_RGB 1, 1, 1
#define INFO_X 50
#define INFO_Y 360
#define INFO_LINE_LENGTH 70
#define INFO_RGB 0, 0, 0
#define ICON_X 50
#define ICON_Y 360
#define CREDITS_RGB 1, 1, 1

#define draw_pixel(x, y, color) vram_s[(y)*640+(x)]=(color)
#define PACK_RGB565(r, g, b) (((r>>3)<<11)|((g>>2)<<5)|((b>>3)<<0))
#define PACK_NYBBLE_RGB565(nybble) ((((nybble & 0x0f00)>>8)*2)<<11) + ((((nybble & 0x00f0)>>4)*4)<<5) + ((((nybble & 0x000f)>>0)*2)<<0)

//void draw_frame(void);
void back_init(char *txrfile);
void font_init(char *gzfile);
void icon_init();
void draw_texture(pvr_ptr_t texture, int tex_size, int x1, int y1, int x2, int y2, int z1);
void draw_char(float x1, float y1, float z1, float a, float r, float g, float b, int c, float xs, float ys);
void draw_string(float x, float y, float z, float a, float r, float g, float b, char *str, float xs, float ys, int max_len=0);
void unpack_color_icon(uint16 *pal, uint8 *icon);
void splash_screen(char *gzfile, int width, int height);
void credits_scroll(char *credits_bg, char *original_bg);

pvr_ptr_t font_tex;
pvr_ptr_t back_tex;
pvr_ptr_t icon_tex;

void back_init(char *txrfile) {
    back_tex = pvr_mem_malloc(512*512*2);
    png_to_texture(txrfile, back_tex, PNG_NO_ALPHA);
}

void font_init(char *gzfile) {
    uint8 *temp_tex;
	gzFile f;
    
    font_tex = pvr_mem_malloc(256*256*2);
	temp_tex=(uint8 *)malloc(512*128);
	f=gzopen(gzfile, "r");
	gzread(f, temp_tex, 512*128);
	gzclose(f);
	pvr_txr_load_ex(temp_tex, font_tex, 256, 256, PVR_TXRLOAD_16BPP);
	free(temp_tex);
}

void icon_init() {
	icon_tex = pvr_mem_malloc(32*32*2);
}

void draw_texture(pvr_ptr_t texture, int tex_size, int x1, int y1, int x2, int y2, int z1) {
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, tex_size, tex_size, texture, PVR_FILTER_BILINEAR);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;
    
    vert.x = x1;
    vert.y = y1;
    vert.z = z1;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x2;
    vert.y = y1;
    vert.z = z1;
    vert.u = 1.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x1;
    vert.y = y2;
    vert.z = z1;
    vert.u = 0.0;
    vert.v = 1.0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x2;
    vert.y = y2;
    vert.z = z1;
    vert.u = 1.0;
    vert.v = 1.0;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}

void draw_char(float x1, float y1, float z1, float a, float r, float g, float b, int c, float xs, float ys) {
    pvr_vertex_t    vert;
    int             ix, iy;
    float           u1, v1, u2, v2;
    
    ix = (c % 32) * 8;
    iy = (c / 32) * 16;
    u1 = (ix + 0.5f) * 1.0f / 256.0f;
    v1 = (iy + 0.5f) * 1.0f / 256.0f;
    u2 = (ix+7.5f) * 1.0f / 256.0f;
    v2 = (iy+15.5f) * 1.0f / 256.0f;
    
    vert.flags = PVR_CMD_VERTEX;
    vert.x = x1;
    vert.y = y1 + 16.0f * ys;
    vert.z = z1;
    vert.u = u1;
    vert.v = v2;
    vert.argb = PVR_PACK_COLOR(a,r,g,b);
    vert.oargb = 0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x1;
    vert.y = y1;
    vert.u = u1;
    vert.v = v1;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = x1 + 8.0f * xs;
    vert.y = y1 + 16.0f * ys;
    vert.u = u2;
    vert.v = v2;
    pvr_prim(&vert, sizeof(vert));
    
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x1 + 8.0f * xs;
    vert.y = y1;
    vert.u = u2;
    vert.v = v1;
    pvr_prim(&vert, sizeof(vert));
}

void draw_string(float x, float y, float z, float a, float r, float g, float b, char *str, float xs, float ys, int max_len=0) {
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	float orig_x = x;
	uint8 sr, sg, sb;
	int i=0;

	pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444, 256, 256, font_tex, PVR_FILTER_BILINEAR);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));  
	while (*str) {
	if (*str == '\r') {
		str++;
		continue;
	}
	if (*str == '¬') {
		str++;
		sr=(uint8)*str; str++;
		sg=(uint8)*str; str++;
		sb=(uint8)*str; str++;
		while (*str != '\n') {
			draw_char(x, y, z, a, sr/255.0, sg/255.0, sb/255.0, *str++, xs, ys);
			x+=8*xs;
		}
	}
	if (*str == '\n') {
		x=orig_x;
		y+=16.0f*ys + 4.0f;
		str++;
		i=0;
		continue;
	}
	if ((max_len > 0) && (i == max_len)) {
		x=orig_x;
		y+=16.0f*ys + 4.0f;
		i=0;
	}
	draw_char(x, y, z, a, r, g, b, *str++, xs, ys);
	x+=8*xs;
	i++;
  }
}

void draw_frame(void) {
    pvr_wait_ready();
    pvr_scene_begin();
    pvr_list_begin(PVR_LIST_OP_POLY);
	draw_texture(back_tex, 512, 1, 1, 640, 480, 1);
	if (vmu_icon != NULL) {
		unpack_color_icon(vmu_icon->pal, vmu_icon->icon);
		draw_texture(icon_tex, 32, ICON_X, ICON_Y, ICON_X+32, ICON_Y+32, 2);
	}
	pvr_list_finish();
    pvr_list_begin(PVR_LIST_TR_POLY);
	draw_string(VMUSRC_X, VMUSRC_Y, 3, 1, VMU_RGB, games_lst, 3, 3);
    draw_string(SAVES_X, SAVES_Y, 3, 1, SAVES_RGB, saves_lst, 1, 1);
    draw_string(VMUDST_X, VMUDST_Y, 3, 1, VMU_RGB, vmus_lst, 2, 2);
    draw_string(INFO_X, INFO_Y, 3, 1, INFO_RGB, vmu_info, 1, 1, INFO_LINE_LENGTH);
    pvr_list_finish();
    pvr_scene_finish();
}

void splash_screen(char *gzfile, int width, int height) {
	uint8 *buf;
	int x, y, i, j, pos_x, pos_y;
	uint8 color=0;
	gzFile f;

	pos_x=(640-width)/2;
	pos_y=(480-height)/2;
	buf=(uint8 *)malloc(width*height);
	f=gzopen(gzfile, "r");
	gzread(f, buf, width*height);
	gzclose(f);
	for (j=0; j<3; j++) {
		for (i=0; i <=255; i++)	{
			switch (j) {
				case 0: color=i;
						break;
				case 1: color=0xff;
						break;
				case 2: color=0xff-i;
						break;
			}
			for (y=0; y < height; y++) {
				for (x=0; x < width; x++) {
					if (color <= *(buf + y*width+x)) {
						draw_pixel(x+pos_x, y+pos_y, PACK_RGB565(color, color, color));
					} else {
						draw_pixel(x+pos_x, y+pos_y, PACK_RGB565(*(buf + y*width+x), *(buf + y*width+x), *(buf + y*width+x)));
					}
				}
			}
		}
	}
	free(buf);
}

void credits_scroll(char *credits_bg, char *original_bg) {
	int y=500;

	pvr_mem_free(back_tex);
    back_init(credits_bg);
//						"1234567890123456789A123456789B123456789C"
	strcpy(games_lst,	"¬\xFF\xFF\x01             VMZip v1.0.0\n"
						"\n"
						"          Coded by El Bucanero\n"
						"\n"
						"¬\xFF\x01\x01                Greetz\n"
						"\n"
						"RockinB (for the original idea)\n"
						"Tommi Uimonen (for the music)\n"
						"Dan Potter (for KallistiOS)\n"
						"AndrewK (for DCload-IP & tool)\n"
						"Lawrence Sebald (for the MinGW guide)\n"
						"Majesco (for ubersexy BloodRayne)\n"
						"\n"
						"           Special thanks to\n"
						"¬\x04\x81\xC4                 SEGA\n"
						"    - for the greatest console! -\n"
						"\n"
						"  Copyright (C) 2005 - Damian Parrino\n"
						"\n"
						"     Made with 100% recycled bytes\n"
						"\n"
						"This soft is FREEWARE - ¬\xFF\x01\x01NOT FOR RESALE!\n"
						"\n"
						"¬\xFF\xFF\x01       Released on 31/Aug/MMV\n"
						"\n\n\n\n\n\n\n\n"
						"         www.bucanero.com.ar\n");
	while (y > -900) {
	    pvr_wait_ready();
		pvr_scene_begin();
	    pvr_list_begin(PVR_LIST_OP_POLY);
		draw_texture(back_tex, 512, 1, 1, 640, 480, 1);
	    pvr_list_finish();
		pvr_list_begin(PVR_LIST_TR_POLY);
		draw_string(0, y , 3, 1, CREDITS_RGB, games_lst, 2, 2);
		pvr_list_finish();
		pvr_scene_finish();
		y--;
	}
	pvr_mem_free(back_tex);
    back_init(original_bg);
}

void unpack_color_icon(uint16 *pal, uint8 *icon) {
	int x, y, nyb;
	uint16 tmp[1024];

	for (y=0; y<32; y++) {
		for (x=0; x<32; x+=2) {
			nyb=(icon[y*16 + x/2] & 0xf0) >> 4;
			tmp[x+y*32]=PACK_NYBBLE_RGB565(pal[nyb]);
			nyb=(icon[y*16 + x/2] & 0x0f) >> 0;
			tmp[x+1+y*32]=PACK_NYBBLE_RGB565(pal[nyb]);
		}
	}
	pvr_txr_load_ex(tmp, icon_tex, 32, 32, PVR_TXRLOAD_16BPP);
}
