#include "../src/port/hd_text_wire.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static int tests, errors;
#define CHECK(cond) do { tests++; if (!(cond)) { errors++; fprintf(stderr,"FAIL at line %d: %s\n", __LINE__,#cond); } } while(0)
int main(void) {
  int r=-1,g=-1;
  CHECK(hd_text_wire_validate(NULL,0,&r,&g) && r==0 && g==0);
  CHECK(!hd_text_wire_validate(NULL,13,&r,&g));
  CHECK(!hd_text_wire_validate(NULL,-1,&r,&g));
  float data[128]={0}; data[12]=2; data[13]=65;data[16]=66;
  CHECK(hd_text_wire_validate(data,19,&r,&g) && r==1 && g==2);
  CHECK(!hd_text_wire_validate(data,18,&r,&g));
  CHECK(!hd_text_wire_validate(data,20,&r,&g));
  data[12]=2.5f; CHECK(!hd_text_wire_validate(data,19,&r,&g)); data[12]=2;
  data[1]=NAN; CHECK(!hd_text_wire_validate(data,19,&r,&g)); data[1]=0;
  data[1]=INFINITY; CHECK(!hd_text_wire_validate(data,19,&r,&g)); data[1]=0;
  data[14]=1000001.0f; CHECK(!hd_text_wire_validate(data,19,&r,&g)); data[14]=0;
  CHECK(!hd_text_wire_validate(data,HD_TEXT_WIRE_CAP+1,&r,&g));
  data[19+12]=0;
  CHECK(hd_text_wire_validate(data,19+13,&r,&g)&&r==2&&g==2);
  data[19+12]=1;
  CHECK(!hd_text_wire_validate(data,19+13,&r,&g));
  // Deterministic hostile snapshots exercise incomplete runs and invalid floats.
  unsigned state=0x5a17c9e3u;
  int bounded=1;
  for (int i=0; i<25000; i++) {
    float noise[64];
    for (int j=0;j<64;j++) {
      state=state*1664525u+1013904223u;
      memcpy(&noise[j],&state,sizeof state);
    }
    int len=(int)(state&63u);
    r=-1; g=-1;
    int accepted=hd_text_wire_validate(noise,len,&r,&g);
    if (accepted && (r<0 || r>len/13 || g<0 || g>len/3)) bounded=0;
    if (!accepted && (r!=0 || g!=0)) bounded=0;
  }
  CHECK(bounded);
  CHECK(errors==0);
  printf("hd_text_wire_test: %d checks, %d failures\n",tests,errors);
  return errors ? 1 : 0;
}
