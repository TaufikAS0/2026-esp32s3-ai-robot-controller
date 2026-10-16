#pragma once
#include <cstddef>
#include <cstring>
// One catalog for firmware validation, rendering and the web catalog endpoint.
#define EYE_CATALOG(X) \
  X(Neutral,"neutral","Netral",Curious,Solid,56,36) \
  X(Happy,"happy","Senang",Happy,Happy,56,24) \
  X(Curious,"curious","Penasaran",Curious,Solid,50,40) \
  X(Sleepy,"sleepy","Mengantuk",Rest,Solid,56,12) \
  X(Angry,"angry","Fokus",Focus,Focus,56,26) \
  X(Surprised,"surprised","Terkejut",Surprise,Solid,46,48) \
  X(Love,"love","Sayang",Love,Heart,54,42) \
  X(Laugh,"laugh","Tertawa",Happy,Happy,62,32) \
  X(Excited,"excited","Antusias",Happy,Star,58,46) \
  X(Giggle,"giggle","Cekikikan",Happy,Happy,48,28) \
  X(LoveWink,"love_wink","Kedip sayang",Love,Heart,54,42) \
  X(Shy,"shy","Malu-malu",Love,Solid,44,30) \
  X(Sparkle,"sparkle","Berbinar",Love,Star,54,44) \
  X(Skeptical,"skeptical","Ragu",Curious,Solid,56,28) \
  X(SideEye,"side_eye","Melirik diam-diam",Curious,Googly,54,34) \
  X(Thinking,"thinking","Berpikir",Curious,Googly,52,36) \
  X(Determined,"determined","Bertekad",Focus,Focus,64,24) \
  X(Sneaky,"sneaky","Usil",Focus,Focus,58,18) \
  X(Shocked,"shocked","Kaget besar",Surprise,Ring,52,50) \
  X(Panic,"panic","Panik lucu",Surprise,Googly,48,46) \
  X(Dozing,"dozing","Terlelap",Rest,Solid,58,6) \
  X(Zen,"zen","Tenang",Rest,Happy,52,16) \
  X(Bored,"bored","Bosan",Rest,Solid,58,17) \
  X(Sad,"sad","Sedih",Sad,Sad,52,34) \
  X(Cry,"cry","Menangis",Sad,Sad,52,34) \
  X(Pleading,"pleading","Memohon",Sad,Googly,60,46) \
  X(WinkLeft,"wink_left","Kedip kiri",Wink,Solid,56,36) \
  X(WinkRight,"wink_right","Kedip kanan",Wink,Solid,56,36) \
  X(DoubleBlink,"double_blink","Kedip ganda",Wink,Solid,56,36) \
  X(Alternating,"alternating","Kedip bergantian",Wink,Solid,56,36) \
  X(Googly,"googly","Googly",Wild,Googly,58,44) \
  X(Dizzy,"dizzy","Pusing lucu",Wild,Spiral,56,44) \
  X(Derp,"derp","Konyol",Wild,Googly,56,40) \
  X(Chaos,"chaos","Kacau ceria",Wild,Googly,60,44) \
  X(Spiral,"spiral","Hipnotis",Wild,Spiral,56,46) \
  X(Party,"party","Pesta",Wild,Star,58,46) \
  X(Stars,"stars","Mata bintang",Digital,Star,56,48) \
  X(Dollars,"dollars","Mata dolar",Digital,Dollar,48,46) \
  X(Loading,"loading","Gaya loading",Digital,Ring,50,46) \
  X(Glitch,"glitch","Glitch",Digital,Solid,60,38) \
  X(RobotError,"robot_error","Gaya error",Digital,Cross,50,44) \
  X(Sharingan,"sharingan","Sharingan 3 tomoe",Sharingan,Sharingan,52,50) \
  X(Rinnegan,"rinnegan","Rinnegan",Shinobi,Rinnegan,54,50) \
  X(Cat,"cat","Kucing",Shinobi,Cat,58,40) \
  X(Fox,"fox","Rubah / Kurama",Shinobi,Fox,62,38) \
  X(Snake,"snake","Ular",Shinobi,Snake,60,32) \
  X(Toad,"toad","Katak",Shinobi,Toad,60,42) \
  X(SharinganOne,"sharingan_1","Sharingan 1 tomoe",Sharingan,Sharingan,52,50) \
  X(SharinganTwo,"sharingan_2","Sharingan 2 tomoe",Sharingan,Sharingan,52,50) \
  X(MangekyoPinwheel,"mangekyo_pinwheel","Mangekyo pinwheel",Sharingan,Sharingan,52,50) \
  X(MangekyoFlower,"mangekyo_flower","Mangekyo flower",Sharingan,Sharingan,52,50) \
  X(MangekyoStar,"mangekyo_star","Mangekyo star",Sharingan,Sharingan,52,50)
enum class Expression {
#define EYE_ENUM(symbol,id,label,category,shape,w,h) symbol,
  EYE_CATALOG(EYE_ENUM)
#undef EYE_ENUM
  Count
};
enum class EyeCategory { All, Happy, Love, Curious, Focus, Surprise, Rest, Sad, Wink, Wild, Digital, Shinobi, Sharingan, Count };
inline constexpr unsigned kEyeCategoryCount=static_cast<unsigned>(EyeCategory::Count);
enum class EyeShape { Solid, Happy, Heart, Focus, Sad, Star, Dollar, Spiral, Cross, Ring, Googly, Sharingan, Rinnegan, Cat, Fox, Snake, Toad };
enum class EyePlayMode { Reactive, Playful, Showcase, Manual };
struct EyeCatalogEntry { const char* id; const char* label; EyeCategory category; EyeShape shape; float width, height; };
inline constexpr EyeCatalogEntry kEyeCatalog[] = {
#define EYE_ENTRY(symbol,id,label,category,shape,w,h) {id,label,EyeCategory::category,EyeShape::shape,w,h},
  EYE_CATALOG(EYE_ENTRY)
#undef EYE_ENTRY
};
#undef EYE_CATALOG
inline constexpr size_t kEyeExpressionCount=sizeof(kEyeCatalog)/sizeof(kEyeCatalog[0]);
static_assert(kEyeExpressionCount==static_cast<unsigned>(Expression::Count),"Catalog count");
inline const EyeCatalogEntry& eyeEntry(Expression e) { return kEyeCatalog[static_cast<unsigned>(e)]; }
inline const char* expressionName(Expression e) { return eyeEntry(e).id; }
inline bool parseExpression(const char* s, Expression& e) {
  if(!s) return false;
  for(unsigned i=0;i<kEyeExpressionCount;++i) if(strcmp(s,kEyeCatalog[i].id)==0) { e=static_cast<Expression>(i); return true; }
  return false;
}
inline const char* eyeCategoryId(EyeCategory c) {
  static const char* names[]={"all","happy","love","curious","focus","surprise","rest","sad","wink","wild","digital","shinobi","sharingan"};
  return names[static_cast<unsigned>(c)];
}
inline const char* eyeCategoryLabel(EyeCategory c) {
  static const char* names[]={"Semua","Ceria","Sayang","Penasaran","Fokus","Terkejut","Santai","Sedih","Kedip","Liar","Digital","Hewan & shinobi","Sharingan"};
  return names[static_cast<unsigned>(c)];
}
inline const char* eyePlayName(EyePlayMode m) {
  static const char* names[]={"reactive","playful","showcase","manual"}; return names[static_cast<unsigned>(m)];
}
inline bool parseEyeCategory(const char* s, EyeCategory& c) {
  for(unsigned i=0;i<kEyeCategoryCount;++i) if(strcmp(s,eyeCategoryId(static_cast<EyeCategory>(i)))==0) { c=static_cast<EyeCategory>(i); return true; }
  return false;
}
inline bool parseEyePlay(const char* s, EyePlayMode& m) {
  for(unsigned i=0;i<4;++i) if(strcmp(s,eyePlayName(static_cast<EyePlayMode>(i)))==0) { m=static_cast<EyePlayMode>(i); return true; }
  return false;
}

inline bool isBoxEye(Expression e) {
  const auto shape=eyeEntry(e).shape;
  return shape==EyeShape::Solid || shape==EyeShape::Happy || shape==EyeShape::Focus || shape==EyeShape::Sad;
}
