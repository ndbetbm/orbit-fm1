/* SPDX-License-Identifier: GPL-3.0-only */
#include "felucca_web.c"
static void render(unsigned ms, FILE *f){unsigned blocks=(ms*441u+1279)/1280;for(unsigned b=0;b<blocks;b++){web_render(128);if(f)for(unsigned k=0;k<128;k++){float s[]={out_l[k],out_r[k]};fwrite(s,sizeof s,1,f);}}}
int main(int argc,char **argv){if(argc<2)return 1;FILE *f=fopen(argv[1],"wb");if(!f)return 1;web_nor_erase();web_boot();render(800,0);web_orbit_regenerate(42);render(100,0);web_buttons(1u<<B_PLAY);render(60,0);web_buttons(0);render(60,0);render(3000,f);fclose(f);return 0;}
