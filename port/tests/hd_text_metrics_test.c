// Verify native glyph sizing against the em-unit metric recorded by gdi.c.
// Does not need Windows, SDL, build/gen or the proprietary game resources.
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "../third_party/stb/stb_truetype.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks, failures;
#define CHECK(p) do { checks++; if (!(p)) { failures++; fprintf(stderr,"FAIL line %d: %s\n", __LINE__, #p); } } while(0)
int main(void) {
  const char *path="../third_party/fonts/LiberationSans-Regular.ttf";
  FILE *f=fopen(path,"rb");
  if (!f) { perror(path); return 1; }
  CHECK(fseek(f,0,SEEK_END)==0);
  long len=ftell(f);
  CHECK(len>0 && len<4000000);
  rewind(f);
  unsigned char *bytes=malloc((size_t)len);
  CHECK(bytes!=NULL);
  if (!bytes) { fclose(f); return 1; }
  CHECK(fread(bytes,1,(size_t)len,f)==(size_t)len);
  fclose(f);
  stbtt_fontinfo font;
  int offset=stbtt_GetFontOffsetForIndex(bytes,0);
  CHECK(offset>=0);
  CHECK(stbtt_InitFont(&font,bytes,offset));
  float em=16.0f, sx=1.50f;
  float expected=stbtt_ScaleForMappingEmToPixels(&font,em*sx);
  float incorrect=stbtt_ScaleForPixelHeight(&font,em*sx);
  CHECK(expected>0.0f);
  CHECK(fabsf(expected-incorrect)>0.0001f);
  for(int xp=0;xp<4;xp++)for(int yp=0;yp<4;yp++) {
    int w,h,x,y;
    unsigned char *bitmap=stbtt_GetCodepointBitmapSubpixel(&font, expected, expected,
                                             xp*0.25f,yp*0.25f,'S',&w,&h,&x,&y);
    CHECK(bitmap!=NULL && w>0 && h>0 && w<=128 && h<=128);
    if(bitmap)stbtt_FreeBitmap(bitmap,NULL);
  }
  free(bytes);
  printf("hd_text_metrics_test: %d checks, %d failures\n",checks,failures);
  return failures ? 1 : 0;
}
