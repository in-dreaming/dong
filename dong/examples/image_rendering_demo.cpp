#include <cstdio>
#include <cstdint>
#include <cstring>

extern "C" {
#include "dong.h"
}

void createTestPNG(){
  uint8_t png_header[]={0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0,0,0,13,73,72,68,82,0,0,0,32,0,0,0,32,8,2,0,0,0,75,109,43,220};
  uint8_t png_data[]={0,0,0,120,73,68,65,84,120,156};
  FILE*fp=fopen("test_image.png","wb");
  fwrite(png_header,1,sizeof(png_header),fp);
  fwrite(png_data,1,sizeof(png_data),fp);
  for(int i=0;i<32*32;i++){
    fputc(0,fp);fputc(255,fp);fputc(0,fp);fputc(0,fp);
  }
  fputc(0,fp);fputc(0,fp);fputc(0,fp);fputc(0,fp);
  fputc(73,fp);fputc(69,fp);fputc(78,fp);fputc(68,fp);
  fputc(174,fp);fputc(66,fp);fputc(96,fp);fputc(130,fp);
  fclose(fp);
}

int main(){
  createTestPNG();
  dong_context_t* ctx=dong_create_context();
  dong_view_t* view=dong_view_create(ctx,600,400);
  const char* html="<html><head><style>body{padding:20px;background:#f0f0f0;}h1{color:#333;}img{border:2px solid #ccc;max-width:150px;margin:20px 0;}</style></head><body>"
    "<h1>Image Rendering</h1><p>Loaded from file:</p><img src='test_image.png' alt='test'/><p>Done!</p></body></html>";
  dong_view_load_html(view,html);
  dong_view_update(view);
  void* buf=dong_view_get_pixel_buffer(view);
  if(buf){
    FILE* fp=fopen("image_rendering_demo.ppm","wb");
    fprintf(fp,"P6\n600 400\n255\n");
    uint8_t* p=(uint8_t*)buf;
    for(int i=0;i<600*400;i++){
      fprintf(fp,"%c%c%c",p[i*4],p[i*4+1],p[i*4+2]);
    }
    fclose(fp);
    printf("Saved: image_rendering_demo.ppm\n");
  }
  dong_view_destroy(view);
  dong_destroy_context(ctx);
  printf("Demo complete!\n");
  return 0;
}
