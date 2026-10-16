#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include "../02_Firmware/robot_controller/eye_raster.h"

bool lit(const EyeRaster& r,int x,int y) { return r.pixels[x+y/8*128] & (1<<(y%8)); }
int main(int argc,char** argv) {
  const Expression variants[]={Expression::SharinganOne,Expression::SharinganTwo,Expression::Sharingan,
    Expression::MangekyoPinwheel,Expression::MangekyoFlower,Expression::MangekyoStar};
  EyeRaster raster;
  std::vector<uint8_t> sheet(6*128*4*64,0);
  for(int phase=0;phase<4;++phase) {
    std::array<std::array<uint8_t,1024>,6> samples{};
    for(int i=0;i<6;++i) {
      EyePose p; p.x=64;p.y=32;p.width=52;p.height=50;p.phase=phase*.61f;
      raster.eye(p,variants[i],true);
      std::copy(std::begin(raster.pixels),std::end(raster.pixels),samples[i].begin());
      assert(!isBoxEye(variants[i]));
      for(int y=0;y<64;++y) for(int x=0;x<128;++x)
        sheet[(phase*64+y)*768+i*128+x]=lit(raster,x,y)?255:0;
    }
    for(int a=0;a<6;++a) for(int b=a+1;b<6;++b) {
      unsigned difference=0;
      for(int i=0;i<1024;++i) { unsigned v=samples[a][i]^samples[b][i];while(v){difference+=v&1;v>>=1;} }
      assert(difference>50); // Visible topology differences, not renamed clones.
    }
  }
  for(bool left:{false,true}) for(int phase=0;phase<100;++phase) {
    EyePose p;p.x=64+std::sin(phase*.3f)*8;p.y=32;p.width=62;p.height=38;p.phase=phase*.17f;
    raster.eye(p,Expression::Fox,left);
    for(int y=0;y<64;++y) for(int x=0;x<128;++x) if(lit(raster,x,y)) {
      const float dx=(x-p.x)/(p.width*.5f),dy=(y-p.y)/(p.height*.5f);
      assert(dx*dx+dy*dy<1.00001f);
      assert(dy>(left?.55f*dx:-.55f*dx)-.40001f);
    }
  }
  if(argc>1) {
    std::ofstream out(argv[1],std::ios::binary); out<<"P5\n768 256\n255\n";
    out.write(reinterpret_cast<const char*>(sheet.data()),sheet.size());
  }
  if(argc>2) {
    std::vector<uint8_t> fox(256*4*64,0);
    for(int phase=0;phase<4;++phase) for(int side=0;side<2;++side) {
      EyePose p;p.x=64;p.y=32;p.width=62;p.height=38;p.phase=phase*.61f;
      raster.eye(p,Expression::Fox,side==0);
      for(int y=0;y<64;++y) for(int x=0;x<128;++x)
        fox[(phase*64+y)*256+side*128+x]=lit(raster,x,y)?255:0;
    }
    std::ofstream out(argv[2],std::ios::binary);out<<"P5\n256 256\n255\n";
    out.write(reinterpret_cast<const char*>(fox.data()),fox.size());
  }
  std::cout<<"Fox silhouette and six distinct Sharingan designs: PASS\n";
}
