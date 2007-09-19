/********************************************************************
 * VMZip v1.0.0 (Aug/2005)
 * lists.h - coded by El Bucanero
 *
 * Copyright (C) 2005 Damian Parrino <bucanero@fibertel.com.ar>
 * http://www.bucanero.com.ar/
 *
 ********************************************************************/

typedef struct save_list {
	char name[13];
	char desc_short[17];
	char desc_long[33];
	bool vmzipped;
	uint8 icon[512];
	uint16 pal[16];
	ssize_t size;
	struct save_list *next;
} save_node_t;

typedef struct vmu_list {
	char name[3];
	uint8 free_blocks;
	struct vmu_list *next;
} vmu_node_t;

typedef struct menu_struct {
	int top;
	int pos;
	int total;
	int maxpage;
} menu_pos_t;

typedef struct interface_struct {
	vmu_node_t *src_vmu;
	menu_pos_t *src_pos;
	save_node_t *saves;
	menu_pos_t *save_pos;
	vmu_node_t *dst_vmu;
	menu_pos_t *dst_pos;
} interface_t;

int load_save_list(save_node_t *sptr, char *dir);
int load_vmu_list(vmu_node_t *vptr);
void free_save_list(save_node_t *sptr);
void free_vmu_list(vmu_node_t *vptr);
save_node_t* get_save_ptr(save_node_t *sptr, int pos);
vmu_node_t* get_vmu_ptr(vmu_node_t *vptr, int pos);
int get_free_blocks(maple_device_t *dev);
int get_free_vmzip_ext(vmu_node_t *vptr);

void draw_frame(void); // screen.h

char games_lst[128];
char saves_lst[256];
char vmus_lst[64];
char vmu_info[384];
save_node_t* vmu_icon=NULL;

save_node_t* get_save_ptr(save_node_t *sptr, int pos) {
	save_node_t *aux=sptr;
	int i=0;

	while (i < pos) {
		aux=aux->next;
		i++;
	}
	return(aux);
}

vmu_node_t* get_vmu_ptr(vmu_node_t *vptr, int pos) {
	vmu_node_t *aux=vptr;
	int i=0;

	while (i < pos) {
		aux=aux->next;
		i++;
	}
	return(aux);
}

int load_save_list(save_node_t *sptr, vmu_node_t *vptr) {
	char tmp[64];
	uint8 buf[1024];
	save_node_t *aux=sptr;
	int i=-1;
	file_t d, f;
	dirent_t *de;

	sprintf(tmp, "/vmu/%s/", vptr->name);
	d = fs_open(tmp, O_RDONLY | O_DIR);
	if (!d) {
		printf("Can't open source directory (%s)\n", tmp);
	} else {
		while ( (de = fs_readdir(d)) ) {
			sprintf(vmu_info, "Loading... %s", de->name);
			draw_frame();
			strcpy(aux->name, de->name);
			aux->size=de->size;
			if (strncmp(aux->name, "VMZIPDAT.", 9) == 0) {
				aux->vmzipped=true;
			} else {
				aux->vmzipped=false;
			}
			sprintf(tmp, "/vmu/%s/%s", vptr->name, de->name);
			f=fs_open(tmp, O_RDONLY);
			fs_read(f, buf, 1024);
			fs_close(f);
			memcpy(aux->desc_short, buf, 16);
			aux->desc_short[16]=0;
			memcpy(aux->desc_long, buf+0x10, 32);
			aux->desc_long[32]=0;
			if (strcmp(aux->name, "ICONDATA_VMS") == 0) {
				memcpy(aux->pal, buf+0xA0, 16*2);
				memcpy(aux->icon, buf+0xC0, 512);
			} else {
				memcpy(aux->pal, buf+0x60, 16*2);
				memcpy(aux->icon, buf+0x80, 512);
			}
// DEBUG
//			printf("---%d---\n%s\n%s\n%s\n%d\n", i+1, aux->name, aux->desc_short, aux->desc_long, aux->size);
// DEBUG
			aux->next=(save_node_t *)malloc(sizeof(save_node_t));
			aux=aux->next;
			i++;
		}
		aux->next=NULL;
	}
	fs_close(d);
	return(i);
}

int load_vmu_list(vmu_node_t *vptr) {
	int i(0);
	vmu_node_t *aux=vptr;
	maple_device_t *dev=NULL;

	while ((dev=maple_enum_type(i++,MAPLE_FUNC_MEMCARD))!=NULL) {
		sprintf(aux->name, "%c%d", (97+dev->port), dev->unit);		
		aux->free_blocks=get_free_blocks(dev);
// DEBUG
//		printf("---%d---\n%s\n%d\n", i-1, aux->name, aux->free_blocks);
// DEBUG
		aux->next=(vmu_node_t *)malloc(sizeof(vmu_node_t));
		aux=aux->next;
	}
	aux->next=NULL;
	return(i-2);
}

int get_free_blocks(maple_device_t *dev) {
	uint16 buf16[256];
	int free_blocks(0),i(0);

	if(vmu_block_read(dev,255,(uint8*)buf16)!=MAPLE_EOK)
		return(-1);
	if(vmu_block_read(dev,buf16[0x46/2],(uint8*)buf16)!=MAPLE_EOK)
		return(-1);
	for(i=0;i<200;++i) 
		if(buf16[i]==0xfffc)
			free_blocks++;
	return(free_blocks);
}

int get_free_vmzip_ext(vmu_node_t *vptr) {
	char tmp[64];
	int ext, i=0;
	file_t d;
	dirent_t *de;

	sprintf(tmp, "/vmu/%s/", vptr->name);
	d = fs_open(tmp, O_RDONLY | O_DIR);
	if (!d) {
		printf("Can't open source directory (%s)\n", tmp);
	} else {
		while ( (de = fs_readdir(d)) ) {
			if (strncmp(de->name, "VMZIPDAT.", 9) == 0) {
				ext=(de->name[9]-0x30)*100 + (de->name[10]-0x30)*10 + (de->name[11]-0x30);
				if (ext > i) {
					i=ext;
				}
			}
		}
	}
	fs_close(d);
	return(i+1);
}

void free_save_list(save_node_t *sptr) {
	save_node_t *aux;

	while (sptr != NULL) {
		aux=sptr->next;
		free(sptr);
		sptr=aux;
	}
}

void free_vmu_list(vmu_node_t *vptr) {
	vmu_node_t *aux;

	while (vptr != NULL) {
		aux=vptr->next;
		free(vptr);
		vptr=aux;
	}
}
