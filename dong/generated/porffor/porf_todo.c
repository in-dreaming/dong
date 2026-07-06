// Porffor module: porf_todo
#include "dong_porf_runtime.h"
#ifndef __porf_nan
#define __porf_nan (NAN)
#endif
#ifndef __porf_infinity
#define __porf_infinity (INFINITY)
#endif

char* dong_porf_porf_todo_memory; u32 dong_porf_porf_todo_memory_pages = 5;

static i32 i32_load(i32 align, i32 offset, i32 pointer) {
  return *((i32*)(dong_porf_porf_todo_memory + offset + pointer));
}

static void i32_store(i32 align, i32 offset, i32 pointer, i32 value) {
  *((i32*)(dong_porf_porf_todo_memory + offset + pointer)) = value;
}

static void i32_store8(i32 align, i32 offset, i32 pointer, u8 value) {
  *((u8*)(dong_porf_porf_todo_memory + offset + pointer)) = value;
}

static i32 i32_load8_u(i32 align, i32 offset, i32 pointer) {
  return *((u8*)(dong_porf_porf_todo_memory + offset + pointer));
}

static f64 f64_load(i32 align, i32 offset, i32 pointer) {
  return *((f64*)(dong_porf_porf_todo_memory + offset + pointer));
}

static i32 i32_load16_u(i32 align, i32 offset, i32 pointer) {
  return *((u16*)(dong_porf_porf_todo_memory + offset + pointer));
}

static void i32_store16(i32 align, i32 offset, i32 pointer, u16 value) {
  *((u16*)(dong_porf_porf_todo_memory + offset + pointer)) = value;
}

static void f64_store(i32 align, i32 offset, i32 pointer, f64 value) {
  *((f64*)(dong_porf_porf_todo_memory + offset + pointer)) = value;
}


__attribute__((import_module(""), import_name("a")))
extern f64 __porf_import_dong_dom_getElementById(f64);

__attribute__((import_module(""), import_name("d")))
extern void __porf_import_dong_stage_0(f64);

__attribute__((import_module(""), import_name("e")))
extern void __porf_import_dong_stage_1(f64);

__attribute__((import_module(""), import_name("f")))
extern void __porf_import_dong_stage_2(f64);

__attribute__((import_module(""), import_name("h")))
extern void __porf_import_dong_commit_addEventListener();

__attribute__((import_module(""), import_name("g")))
extern void __porf_import_dong_commit_set_textContent();

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_style_set(f64, f64, f64);

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_remove_attribute(f64, f64);

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_set_attribute(f64, f64, f64);

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_set_inner_html(f64, f64);

__attribute__((import_module(""), import_name("i")))
extern void __porf_import_dong_print(f64);

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_get_value(f64);

__attribute__((import_module(""), import_name("j")))
extern f64 __porf_import_dong_str_len();

__attribute__((import_module(""), import_name("l")))
extern f64 __porf_import_dong_str_byte_at(f64);

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_set_value(f64, f64);

__attribute__((import_module(""), import_name("®")))
extern void __porf_import_dong_event_key();

__attribute__((import_module(""), import_name("­")))
extern f64 __porf_import_dong_event_target();

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_get_attribute(f64, f64);

__attribute__((import_module(""), import_name("")))
extern f64 __porf_import_dong_closest(f64, f64);
void dong_porf_porf_todo__porf_init(void) {
  if (dong_porf_porf_todo_memory) return;
  dong_porf_porf_todo_memory = calloc(1, dong_porf_porf_todo_memory_pages * 65536);

  dong_porf_porf_todo_memory[16]=(u8)30;dong_porf_porf_todo_memory[20]=(u8)76;dong_porf_porf_todo_memory[21]=(u8)101;dong_porf_porf_todo_memory[22]=(u8)97;dong_porf_porf_todo_memory[23]=(u8)114;dong_porf_porf_todo_memory[24]=(u8)110;dong_porf_porf_todo_memory[25]=(u8)32;dong_porf_porf_todo_memory[26]=(u8)80;dong_porf_porf_todo_memory[27]=(u8)111;dong_porf_porf_todo_memory[28]=(u8)114;dong_porf_porf_todo_memory[29]=(u8)102;dong_porf_porf_todo_memory[30]=(u8)102;dong_porf_porf_todo_memory[31]=(u8)111;dong_porf_porf_todo_memory[32]=(u8)114;dong_porf_porf_todo_memory[33]=(u8)32;dong_porf_porf_todo_memory[34]=(u8)99;dong_porf_porf_todo_memory[35]=(u8)108;dong_porf_porf_todo_memory[36]=(u8)97;dong_porf_porf_todo_memory[37]=(u8)115;dong_porf_porf_todo_memory[38]=(u8)115;dong_porf_porf_todo_memory[39]=(u8)32;dong_porf_porf_todo_memory[40]=(u8)99;dong_porf_porf_todo_memory[41]=(u8)111;dong_porf_porf_todo_memory[42]=(u8)109;dong_porf_porf_todo_memory[43]=(u8)112;dong_porf_porf_todo_memory[44]=(u8)111;dong_porf_porf_todo_memory[45]=(u8)110;dong_porf_porf_todo_memory[46]=(u8)101;dong_porf_porf_todo_memory[47]=(u8)110;dong_porf_porf_todo_memory[48]=(u8)116;dong_porf_porf_todo_memory[49]=(u8)115;
  dong_porf_porf_todo_memory[52]=(u8)30;dong_porf_porf_todo_memory[56]=(u8)67;dong_porf_porf_todo_memory[57]=(u8)111;dong_porf_porf_todo_memory[58]=(u8)109;dong_porf_porf_todo_memory[59]=(u8)112;dong_porf_porf_todo_memory[60]=(u8)97;dong_porf_porf_todo_memory[61]=(u8)114;dong_porf_porf_todo_memory[62]=(u8)101;dong_porf_porf_todo_memory[63]=(u8)32;dong_porf_porf_todo_memory[64]=(u8)98;dong_porf_porf_todo_memory[65]=(u8)117;dong_porf_porf_todo_memory[66]=(u8)110;dong_porf_porf_todo_memory[67]=(u8)100;dong_porf_porf_todo_memory[68]=(u8)108;dong_porf_porf_todo_memory[69]=(u8)101;dong_porf_porf_todo_memory[70]=(u8)32;dong_porf_porf_todo_memory[71]=(u8)115;dong_porf_porf_todo_memory[72]=(u8)105;dong_porf_porf_todo_memory[73]=(u8)122;dong_porf_porf_todo_memory[74]=(u8)101;dong_porf_porf_todo_memory[75]=(u8)32;dong_porf_porf_todo_memory[76]=(u8)119;dong_porf_porf_todo_memory[77]=(u8)105;dong_porf_porf_todo_memory[78]=(u8)116;dong_porf_porf_todo_memory[79]=(u8)104;dong_porf_porf_todo_memory[80]=(u8)32;dong_porf_porf_todo_memory[81]=(u8)82;dong_porf_porf_todo_memory[82]=(u8)101;dong_porf_porf_todo_memory[83]=(u8)97;dong_porf_porf_todo_memory[84]=(u8)99;dong_porf_porf_todo_memory[85]=(u8)116;
  dong_porf_porf_todo_memory[88]=(u8)34;dong_porf_porf_todo_memory[92]=(u8)73;dong_porf_porf_todo_memory[93]=(u8)110;dong_porf_porf_todo_memory[94]=(u8)116;dong_porf_porf_todo_memory[95]=(u8)101;dong_porf_porf_todo_memory[96]=(u8)103;dong_porf_porf_todo_memory[97]=(u8)114;dong_porf_porf_todo_memory[98]=(u8)97;dong_porf_porf_todo_memory[99]=(u8)116;dong_porf_porf_todo_memory[100]=(u8)101;dong_porf_porf_todo_memory[101]=(u8)32;dong_porf_porf_todo_memory[102]=(u8)80;dong_porf_porf_todo_memory[103]=(u8)111;dong_porf_porf_todo_memory[104]=(u8)114;dong_porf_porf_todo_memory[105]=(u8)102;dong_porf_porf_todo_memory[106]=(u8)102;dong_porf_porf_todo_memory[107]=(u8)111;dong_porf_porf_todo_memory[108]=(u8)114;dong_porf_porf_todo_memory[109]=(u8)32;dong_porf_porf_todo_memory[110]=(u8)119;dong_porf_porf_todo_memory[111]=(u8)105;dong_porf_porf_todo_memory[112]=(u8)116;dong_porf_porf_todo_memory[113]=(u8)104;dong_porf_porf_todo_memory[114]=(u8)32;dong_porf_porf_todo_memory[115]=(u8)68;dong_porf_porf_todo_memory[116]=(u8)111;dong_porf_porf_todo_memory[117]=(u8)110;dong_porf_porf_todo_memory[118]=(u8)103;dong_porf_porf_todo_memory[119]=(u8)32;dong_porf_porf_todo_memory[120]=(u8)101;dong_porf_porf_todo_memory[121]=(u8)110;dong_porf_porf_todo_memory[122]=(u8)103;dong_porf_porf_todo_memory[123]=(u8)105;dong_porf_porf_todo_memory[124]=(u8)110;dong_porf_porf_todo_memory[125]=(u8)101;
  dong_porf_porf_todo_memory[128]=(u8)16;dong_porf_porf_todo_memory[132]=(u8)83;dong_porf_porf_todo_memory[133]=(u8)104;dong_porf_porf_todo_memory[134]=(u8)105;dong_porf_porf_todo_memory[135]=(u8)112;dong_porf_porf_todo_memory[136]=(u8)32;dong_porf_porf_todo_memory[137]=(u8)116;dong_porf_porf_todo_memory[138]=(u8)104;dong_porf_porf_todo_memory[139]=(u8)101;dong_porf_porf_todo_memory[140]=(u8)32;dong_porf_porf_todo_memory[141]=(u8)103;dong_porf_porf_todo_memory[142]=(u8)97;dong_porf_porf_todo_memory[143]=(u8)109;dong_porf_porf_todo_memory[144]=(u8)101;dong_porf_porf_todo_memory[145]=(u8)32;dong_porf_porf_todo_memory[146]=(u8)85;dong_porf_porf_todo_memory[147]=(u8)73;
  dong_porf_porf_todo_memory[150]=(u8)41;dong_porf_porf_todo_memory[154]=(u8)67;dong_porf_porf_todo_memory[155]=(u8)97;dong_porf_porf_todo_memory[156]=(u8)110;dong_porf_porf_todo_memory[157]=(u8)110;dong_porf_porf_todo_memory[158]=(u8)111;dong_porf_porf_todo_memory[159]=(u8)116;dong_porf_porf_todo_memory[160]=(u8)32;dong_porf_porf_todo_memory[161]=(u8)99;dong_porf_porf_todo_memory[162]=(u8)111;dong_porf_porf_todo_memory[163]=(u8)110;dong_porf_porf_todo_memory[164]=(u8)118;dong_porf_porf_todo_memory[165]=(u8)101;dong_porf_porf_todo_memory[166]=(u8)114;dong_porf_porf_todo_memory[167]=(u8)116;dong_porf_porf_todo_memory[168]=(u8)32;dong_porf_porf_todo_memory[169]=(u8)97;dong_porf_porf_todo_memory[170]=(u8)32;dong_porf_porf_todo_memory[171]=(u8)83;dong_porf_porf_todo_memory[172]=(u8)121;dong_porf_porf_todo_memory[173]=(u8)109;dong_porf_porf_todo_memory[174]=(u8)98;dong_porf_porf_todo_memory[175]=(u8)111;dong_porf_porf_todo_memory[176]=(u8)108;dong_porf_porf_todo_memory[177]=(u8)32;dong_porf_porf_todo_memory[178]=(u8)118;dong_porf_porf_todo_memory[179]=(u8)97;dong_porf_porf_todo_memory[180]=(u8)108;dong_porf_porf_todo_memory[181]=(u8)117;dong_porf_porf_todo_memory[182]=(u8)101;dong_porf_porf_todo_memory[183]=(u8)32;dong_porf_porf_todo_memory[184]=(u8)116;dong_porf_porf_todo_memory[185]=(u8)111;dong_porf_porf_todo_memory[186]=(u8)32;dong_porf_porf_todo_memory[187]=(u8)97;dong_porf_porf_todo_memory[188]=(u8)32;dong_porf_porf_todo_memory[189]=(u8)115;dong_porf_porf_todo_memory[190]=(u8)116;dong_porf_porf_todo_memory[191]=(u8)114;dong_porf_porf_todo_memory[192]=(u8)105;dong_porf_porf_todo_memory[193]=(u8)110;dong_porf_porf_todo_memory[194]=(u8)103;
  dong_porf_porf_todo_memory[197]=(u8)9;dong_porf_porf_todo_memory[201]=(u8)117;dong_porf_porf_todo_memory[202]=(u8)110;dong_porf_porf_todo_memory[203]=(u8)100;dong_porf_porf_todo_memory[204]=(u8)101;dong_porf_porf_todo_memory[205]=(u8)102;dong_porf_porf_todo_memory[206]=(u8)105;dong_porf_porf_todo_memory[207]=(u8)110;dong_porf_porf_todo_memory[208]=(u8)101;dong_porf_porf_todo_memory[209]=(u8)100;
  dong_porf_porf_todo_memory[212]=(u8)4;dong_porf_porf_todo_memory[216]=(u8)110;dong_porf_porf_todo_memory[217]=(u8)117;dong_porf_porf_todo_memory[218]=(u8)108;dong_porf_porf_todo_memory[219]=(u8)108;
  dong_porf_porf_todo_memory[222]=(u8)4;dong_porf_porf_todo_memory[226]=(u8)116;dong_porf_porf_todo_memory[227]=(u8)114;dong_porf_porf_todo_memory[228]=(u8)117;dong_porf_porf_todo_memory[229]=(u8)101;
  dong_porf_porf_todo_memory[232]=(u8)5;dong_porf_porf_todo_memory[236]=(u8)102;dong_porf_porf_todo_memory[237]=(u8)97;dong_porf_porf_todo_memory[238]=(u8)108;dong_porf_porf_todo_memory[239]=(u8)115;dong_porf_porf_todo_memory[240]=(u8)101;
  dong_porf_porf_todo_memory[243]=(u8)55;dong_porf_porf_todo_memory[247]=(u8)78;dong_porf_porf_todo_memory[248]=(u8)117;dong_porf_porf_todo_memory[249]=(u8)109;dong_porf_porf_todo_memory[250]=(u8)98;dong_porf_porf_todo_memory[251]=(u8)101;dong_porf_porf_todo_memory[252]=(u8)114;dong_porf_porf_todo_memory[253]=(u8)46;dong_porf_porf_todo_memory[254]=(u8)112;dong_porf_porf_todo_memory[255]=(u8)114;dong_porf_porf_todo_memory[256]=(u8)111;dong_porf_porf_todo_memory[257]=(u8)116;dong_porf_porf_todo_memory[258]=(u8)111;dong_porf_porf_todo_memory[259]=(u8)116;dong_porf_porf_todo_memory[260]=(u8)121;dong_porf_porf_todo_memory[261]=(u8)112;dong_porf_porf_todo_memory[262]=(u8)101;dong_porf_porf_todo_memory[263]=(u8)46;dong_porf_porf_todo_memory[264]=(u8)116;dong_porf_porf_todo_memory[265]=(u8)111;dong_porf_porf_todo_memory[266]=(u8)83;dong_porf_porf_todo_memory[267]=(u8)116;dong_porf_porf_todo_memory[268]=(u8)114;dong_porf_porf_todo_memory[269]=(u8)105;dong_porf_porf_todo_memory[270]=(u8)110;dong_porf_porf_todo_memory[271]=(u8)103;dong_porf_porf_todo_memory[272]=(u8)32;dong_porf_porf_todo_memory[273]=(u8)101;dong_porf_porf_todo_memory[274]=(u8)120;dong_porf_porf_todo_memory[275]=(u8)112;dong_porf_porf_todo_memory[276]=(u8)101;dong_porf_porf_todo_memory[277]=(u8)99;dong_porf_porf_todo_memory[278]=(u8)116;dong_porf_porf_todo_memory[279]=(u8)115;dong_porf_porf_todo_memory[280]=(u8)32;dong_porf_porf_todo_memory[281]=(u8)39;dong_porf_porf_todo_memory[282]=(u8)116;dong_porf_porf_todo_memory[283]=(u8)104;dong_porf_porf_todo_memory[284]=(u8)105;dong_porf_porf_todo_memory[285]=(u8)115;dong_porf_porf_todo_memory[286]=(u8)39;dong_porf_porf_todo_memory[287]=(u8)32;dong_porf_porf_todo_memory[288]=(u8)116;dong_porf_porf_todo_memory[289]=(u8)111;dong_porf_porf_todo_memory[290]=(u8)32;dong_porf_porf_todo_memory[291]=(u8)98;dong_porf_porf_todo_memory[292]=(u8)101;dong_porf_porf_todo_memory[293]=(u8)32;dong_porf_porf_todo_memory[294]=(u8)97;dong_porf_porf_todo_memory[295]=(u8)32;dong_porf_porf_todo_memory[296]=(u8)78;dong_porf_porf_todo_memory[297]=(u8)117;dong_porf_porf_todo_memory[298]=(u8)109;dong_porf_porf_todo_memory[299]=(u8)98;dong_porf_porf_todo_memory[300]=(u8)101;dong_porf_porf_todo_memory[301]=(u8)114;
  dong_porf_porf_todo_memory[304]=(u8)50;dong_porf_porf_todo_memory[308]=(u8)116;dong_porf_porf_todo_memory[309]=(u8)111;dong_porf_porf_todo_memory[310]=(u8)83;dong_porf_porf_todo_memory[311]=(u8)116;dong_porf_porf_todo_memory[312]=(u8)114;dong_porf_porf_todo_memory[313]=(u8)105;dong_porf_porf_todo_memory[314]=(u8)110;dong_porf_porf_todo_memory[315]=(u8)103;dong_porf_porf_todo_memory[316]=(u8)40;dong_porf_porf_todo_memory[317]=(u8)41;dong_porf_porf_todo_memory[318]=(u8)32;dong_porf_porf_todo_memory[319]=(u8)114;dong_porf_porf_todo_memory[320]=(u8)97;dong_porf_porf_todo_memory[321]=(u8)100;dong_porf_porf_todo_memory[322]=(u8)105;dong_porf_porf_todo_memory[323]=(u8)120;dong_porf_porf_todo_memory[324]=(u8)32;dong_porf_porf_todo_memory[325]=(u8)97;dong_porf_porf_todo_memory[326]=(u8)114;dong_porf_porf_todo_memory[327]=(u8)103;dong_porf_porf_todo_memory[328]=(u8)117;dong_porf_porf_todo_memory[329]=(u8)109;dong_porf_porf_todo_memory[330]=(u8)101;dong_porf_porf_todo_memory[331]=(u8)110;dong_porf_porf_todo_memory[332]=(u8)116;dong_porf_porf_todo_memory[333]=(u8)32;dong_porf_porf_todo_memory[334]=(u8)109;dong_porf_porf_todo_memory[335]=(u8)117;dong_porf_porf_todo_memory[336]=(u8)115;dong_porf_porf_todo_memory[337]=(u8)116;dong_porf_porf_todo_memory[338]=(u8)32;dong_porf_porf_todo_memory[339]=(u8)98;dong_porf_porf_todo_memory[340]=(u8)101;dong_porf_porf_todo_memory[341]=(u8)32;dong_porf_porf_todo_memory[342]=(u8)98;dong_porf_porf_todo_memory[343]=(u8)101;dong_porf_porf_todo_memory[344]=(u8)116;dong_porf_porf_todo_memory[345]=(u8)119;dong_porf_porf_todo_memory[346]=(u8)101;dong_porf_porf_todo_memory[347]=(u8)101;dong_porf_porf_todo_memory[348]=(u8)110;dong_porf_porf_todo_memory[349]=(u8)32;dong_porf_porf_todo_memory[350]=(u8)50;dong_porf_porf_todo_memory[351]=(u8)32;dong_porf_porf_todo_memory[352]=(u8)97;dong_porf_porf_todo_memory[353]=(u8)110;dong_porf_porf_todo_memory[354]=(u8)100;dong_porf_porf_todo_memory[355]=(u8)32;dong_porf_porf_todo_memory[356]=(u8)51;dong_porf_porf_todo_memory[357]=(u8)54;
  dong_porf_porf_todo_memory[360]=(u8)3;dong_porf_porf_todo_memory[364]=(u8)78;dong_porf_porf_todo_memory[365]=(u8)97;dong_porf_porf_todo_memory[366]=(u8)78;
  dong_porf_porf_todo_memory[369]=(u8)8;dong_porf_porf_todo_memory[373]=(u8)73;dong_porf_porf_todo_memory[374]=(u8)110;dong_porf_porf_todo_memory[375]=(u8)102;dong_porf_porf_todo_memory[376]=(u8)105;dong_porf_porf_todo_memory[377]=(u8)110;dong_porf_porf_todo_memory[378]=(u8)105;dong_porf_porf_todo_memory[379]=(u8)116;dong_porf_porf_todo_memory[380]=(u8)121;
  dong_porf_porf_todo_memory[383]=(u8)9;dong_porf_porf_todo_memory[387]=(u8)45;dong_porf_porf_todo_memory[388]=(u8)73;dong_porf_porf_todo_memory[389]=(u8)110;dong_porf_porf_todo_memory[390]=(u8)102;dong_porf_porf_todo_memory[391]=(u8)105;dong_porf_porf_todo_memory[392]=(u8)110;dong_porf_porf_todo_memory[393]=(u8)105;dong_porf_porf_todo_memory[394]=(u8)116;dong_porf_porf_todo_memory[395]=(u8)121;
  dong_porf_porf_todo_memory[398]=(u8)1;dong_porf_porf_todo_memory[402]=(u8)48;
  dong_porf_porf_todo_memory[405]=(u8)59;dong_porf_porf_todo_memory[409]=(u8)70;dong_porf_porf_todo_memory[410]=(u8)117;dong_porf_porf_todo_memory[411]=(u8)110;dong_porf_porf_todo_memory[412]=(u8)99;dong_porf_porf_todo_memory[413]=(u8)116;dong_porf_porf_todo_memory[414]=(u8)105;dong_porf_porf_todo_memory[415]=(u8)111;dong_porf_porf_todo_memory[416]=(u8)110;dong_porf_porf_todo_memory[417]=(u8)46;dong_porf_porf_todo_memory[418]=(u8)112;dong_porf_porf_todo_memory[419]=(u8)114;dong_porf_porf_todo_memory[420]=(u8)111;dong_porf_porf_todo_memory[421]=(u8)116;dong_porf_porf_todo_memory[422]=(u8)111;dong_porf_porf_todo_memory[423]=(u8)116;dong_porf_porf_todo_memory[424]=(u8)121;dong_porf_porf_todo_memory[425]=(u8)112;dong_porf_porf_todo_memory[426]=(u8)101;dong_porf_porf_todo_memory[427]=(u8)46;dong_porf_porf_todo_memory[428]=(u8)116;dong_porf_porf_todo_memory[429]=(u8)111;dong_porf_porf_todo_memory[430]=(u8)83;dong_porf_porf_todo_memory[431]=(u8)116;dong_porf_porf_todo_memory[432]=(u8)114;dong_porf_porf_todo_memory[433]=(u8)105;dong_porf_porf_todo_memory[434]=(u8)110;dong_porf_porf_todo_memory[435]=(u8)103;dong_porf_porf_todo_memory[436]=(u8)32;dong_porf_porf_todo_memory[437]=(u8)101;dong_porf_porf_todo_memory[438]=(u8)120;dong_porf_porf_todo_memory[439]=(u8)112;dong_porf_porf_todo_memory[440]=(u8)101;dong_porf_porf_todo_memory[441]=(u8)99;dong_porf_porf_todo_memory[442]=(u8)116;dong_porf_porf_todo_memory[443]=(u8)115;dong_porf_porf_todo_memory[444]=(u8)32;dong_porf_porf_todo_memory[445]=(u8)39;dong_porf_porf_todo_memory[446]=(u8)116;dong_porf_porf_todo_memory[447]=(u8)104;dong_porf_porf_todo_memory[448]=(u8)105;dong_porf_porf_todo_memory[449]=(u8)115;dong_porf_porf_todo_memory[450]=(u8)39;dong_porf_porf_todo_memory[451]=(u8)32;dong_porf_porf_todo_memory[452]=(u8)116;dong_porf_porf_todo_memory[453]=(u8)111;dong_porf_porf_todo_memory[454]=(u8)32;dong_porf_porf_todo_memory[455]=(u8)98;dong_porf_porf_todo_memory[456]=(u8)101;dong_porf_porf_todo_memory[457]=(u8)32;dong_porf_porf_todo_memory[458]=(u8)97;dong_porf_porf_todo_memory[459]=(u8)32;dong_porf_porf_todo_memory[460]=(u8)70;dong_porf_porf_todo_memory[461]=(u8)117;dong_porf_porf_todo_memory[462]=(u8)110;dong_porf_porf_todo_memory[463]=(u8)99;dong_porf_porf_todo_memory[464]=(u8)116;dong_porf_porf_todo_memory[465]=(u8)105;dong_porf_porf_todo_memory[466]=(u8)111;dong_porf_porf_todo_memory[467]=(u8)110;
  dong_porf_porf_todo_memory[470]=(u8)9;dong_porf_porf_todo_memory[474]=(u8)102;dong_porf_porf_todo_memory[475]=(u8)117;dong_porf_porf_todo_memory[476]=(u8)110;dong_porf_porf_todo_memory[477]=(u8)99;dong_porf_porf_todo_memory[478]=(u8)116;dong_porf_porf_todo_memory[479]=(u8)105;dong_porf_porf_todo_memory[480]=(u8)111;dong_porf_porf_todo_memory[481]=(u8)110;dong_porf_porf_todo_memory[482]=(u8)32;
  dong_porf_porf_todo_memory[485]=(u8)20;dong_porf_porf_todo_memory[489]=(u8)40;dong_porf_porf_todo_memory[490]=(u8)41;dong_porf_porf_todo_memory[491]=(u8)32;dong_porf_porf_todo_memory[492]=(u8)123;dong_porf_porf_todo_memory[493]=(u8)32;dong_porf_porf_todo_memory[494]=(u8)91;dong_porf_porf_todo_memory[495]=(u8)110;dong_porf_porf_todo_memory[496]=(u8)97;dong_porf_porf_todo_memory[497]=(u8)116;dong_porf_porf_todo_memory[498]=(u8)105;dong_porf_porf_todo_memory[499]=(u8)118;dong_porf_porf_todo_memory[500]=(u8)101;dong_porf_porf_todo_memory[501]=(u8)32;dong_porf_porf_todo_memory[502]=(u8)99;dong_porf_porf_todo_memory[503]=(u8)111;dong_porf_porf_todo_memory[504]=(u8)100;dong_porf_porf_todo_memory[505]=(u8)101;dong_porf_porf_todo_memory[506]=(u8)93;dong_porf_porf_todo_memory[507]=(u8)32;dong_porf_porf_todo_memory[508]=(u8)125;
  dong_porf_porf_todo_memory[511]=(u8)9;dong_porf_porf_todo_memory[515]=(u8)84;dong_porf_porf_todo_memory[516]=(u8)121;dong_porf_porf_todo_memory[517]=(u8)112;dong_porf_porf_todo_memory[518]=(u8)101;dong_porf_porf_todo_memory[519]=(u8)69;dong_porf_porf_todo_memory[520]=(u8)114;dong_porf_porf_todo_memory[521]=(u8)114;dong_porf_porf_todo_memory[522]=(u8)111;dong_porf_porf_todo_memory[523]=(u8)114;
  dong_porf_porf_todo_memory[526]=(u8)2;dong_porf_porf_todo_memory[530]=(u8)58;dong_porf_porf_todo_memory[531]=(u8)32;
  dong_porf_porf_todo_memory[534]=(u8)10;dong_porf_porf_todo_memory[538]=(u8)82;dong_porf_porf_todo_memory[539]=(u8)97;dong_porf_porf_todo_memory[540]=(u8)110;dong_porf_porf_todo_memory[541]=(u8)103;dong_porf_porf_todo_memory[542]=(u8)101;dong_porf_porf_todo_memory[543]=(u8)69;dong_porf_porf_todo_memory[544]=(u8)114;dong_porf_porf_todo_memory[545]=(u8)114;dong_porf_porf_todo_memory[546]=(u8)111;dong_porf_porf_todo_memory[547]=(u8)114;
  dong_porf_porf_todo_memory[550]=(u8)8;dong_porf_porf_todo_memory[554]=(u8)116;dong_porf_porf_todo_memory[555]=(u8)111;dong_porf_porf_todo_memory[556]=(u8)83;dong_porf_porf_todo_memory[557]=(u8)116;dong_porf_porf_todo_memory[558]=(u8)114;dong_porf_porf_todo_memory[559]=(u8)105;dong_porf_porf_todo_memory[560]=(u8)110;dong_porf_porf_todo_memory[561]=(u8)103;
  dong_porf_porf_todo_memory[564]=(u8)6;dong_porf_porf_todo_memory[568]=(u8)108;dong_porf_porf_todo_memory[569]=(u8)101;dong_porf_porf_todo_memory[570]=(u8)110;dong_porf_porf_todo_memory[571]=(u8)103;dong_porf_porf_todo_memory[572]=(u8)116;dong_porf_porf_todo_memory[573]=(u8)104;
  dong_porf_porf_todo_memory[576]=(u8)4;dong_porf_porf_todo_memory[580]=(u8)110;dong_porf_porf_todo_memory[581]=(u8)97;dong_porf_porf_todo_memory[582]=(u8)109;dong_porf_porf_todo_memory[583]=(u8)101;
  dong_porf_porf_todo_memory[586]=(u8)9;dong_porf_porf_todo_memory[590]=(u8)112;dong_porf_porf_todo_memory[591]=(u8)114;dong_porf_porf_todo_memory[592]=(u8)111;dong_porf_porf_todo_memory[593]=(u8)116;dong_porf_porf_todo_memory[594]=(u8)111;dong_porf_porf_todo_memory[595]=(u8)116;dong_porf_porf_todo_memory[596]=(u8)121;dong_porf_porf_todo_memory[597]=(u8)112;dong_porf_porf_todo_memory[598]=(u8)101;
  dong_porf_porf_todo_memory[601]=(u8)11;dong_porf_porf_todo_memory[605]=(u8)99;dong_porf_porf_todo_memory[606]=(u8)111;dong_porf_porf_todo_memory[607]=(u8)110;dong_porf_porf_todo_memory[608]=(u8)115;dong_porf_porf_todo_memory[609]=(u8)116;dong_porf_porf_todo_memory[610]=(u8)114;dong_porf_porf_todo_memory[611]=(u8)117;dong_porf_porf_todo_memory[612]=(u8)99;dong_porf_porf_todo_memory[613]=(u8)116;dong_porf_porf_todo_memory[614]=(u8)111;dong_porf_porf_todo_memory[615]=(u8)114;
  dong_porf_porf_todo_memory[618]=(u8)27;dong_porf_porf_todo_memory[622]=(u8)67;dong_porf_porf_todo_memory[623]=(u8)97;dong_porf_porf_todo_memory[624]=(u8)110;dong_porf_porf_todo_memory[625]=(u8)110;dong_porf_porf_todo_memory[626]=(u8)111;dong_porf_porf_todo_memory[627]=(u8)116;dong_porf_porf_todo_memory[628]=(u8)32;dong_porf_porf_todo_memory[629]=(u8)103;dong_porf_porf_todo_memory[630]=(u8)101;dong_porf_porf_todo_memory[631]=(u8)116;dong_porf_porf_todo_memory[632]=(u8)32;dong_porf_porf_todo_memory[633]=(u8)112;dong_porf_porf_todo_memory[634]=(u8)114;dong_porf_porf_todo_memory[635]=(u8)111;dong_porf_porf_todo_memory[636]=(u8)112;dong_porf_porf_todo_memory[637]=(u8)101;dong_porf_porf_todo_memory[638]=(u8)114;dong_porf_porf_todo_memory[639]=(u8)116;dong_porf_porf_todo_memory[640]=(u8)121;dong_porf_porf_todo_memory[641]=(u8)32;dong_porf_porf_todo_memory[642]=(u8)111;dong_porf_porf_todo_memory[643]=(u8)102;dong_porf_porf_todo_memory[644]=(u8)32;dong_porf_porf_todo_memory[645]=(u8)110;dong_porf_porf_todo_memory[646]=(u8)117;dong_porf_porf_todo_memory[647]=(u8)108;dong_porf_porf_todo_memory[648]=(u8)108;
  dong_porf_porf_todo_memory[651]=(u8)14;dong_porf_porf_todo_memory[655]=(u8)104;dong_porf_porf_todo_memory[656]=(u8)97;dong_porf_porf_todo_memory[657]=(u8)115;dong_porf_porf_todo_memory[658]=(u8)79;dong_porf_porf_todo_memory[659]=(u8)119;dong_porf_porf_todo_memory[660]=(u8)110;dong_porf_porf_todo_memory[661]=(u8)80;dong_porf_porf_todo_memory[662]=(u8)114;dong_porf_porf_todo_memory[663]=(u8)111;dong_porf_porf_todo_memory[664]=(u8)112;dong_porf_porf_todo_memory[665]=(u8)101;dong_porf_porf_todo_memory[666]=(u8)114;dong_porf_porf_todo_memory[667]=(u8)116;dong_porf_porf_todo_memory[668]=(u8)121;
  dong_porf_porf_todo_memory[671]=(u8)36;dong_porf_porf_todo_memory[675]=(u8)65;dong_porf_porf_todo_memory[676]=(u8)114;dong_porf_porf_todo_memory[677]=(u8)103;dong_porf_porf_todo_memory[678]=(u8)117;dong_porf_porf_todo_memory[679]=(u8)109;dong_porf_porf_todo_memory[680]=(u8)101;dong_porf_porf_todo_memory[681]=(u8)110;dong_porf_porf_todo_memory[682]=(u8)116;dong_porf_porf_todo_memory[683]=(u8)32;dong_porf_porf_todo_memory[684]=(u8)105;dong_porf_porf_todo_memory[685]=(u8)115;dong_porf_porf_todo_memory[686]=(u8)32;dong_porf_porf_todo_memory[687]=(u8)110;dong_porf_porf_todo_memory[688]=(u8)117;dong_porf_porf_todo_memory[689]=(u8)108;dong_porf_porf_todo_memory[690]=(u8)108;dong_porf_porf_todo_memory[691]=(u8)105;dong_porf_porf_todo_memory[692]=(u8)115;dong_porf_porf_todo_memory[693]=(u8)104;dong_porf_porf_todo_memory[694]=(u8)44;dong_porf_porf_todo_memory[695]=(u8)32;dong_porf_porf_todo_memory[696]=(u8)101;dong_porf_porf_todo_memory[697]=(u8)120;dong_porf_porf_todo_memory[698]=(u8)112;dong_porf_porf_todo_memory[699]=(u8)101;dong_porf_porf_todo_memory[700]=(u8)99;dong_porf_porf_todo_memory[701]=(u8)116;dong_porf_porf_todo_memory[702]=(u8)101;dong_porf_porf_todo_memory[703]=(u8)100;dong_porf_porf_todo_memory[704]=(u8)32;dong_porf_porf_todo_memory[705]=(u8)111;dong_porf_porf_todo_memory[706]=(u8)98;dong_porf_porf_todo_memory[707]=(u8)106;dong_porf_porf_todo_memory[708]=(u8)101;dong_porf_porf_todo_memory[709]=(u8)99;dong_porf_porf_todo_memory[710]=(u8)116;
  dong_porf_porf_todo_memory[713]=(u8)26;dong_porf_porf_todo_memory[717]=(u8)65;dong_porf_porf_todo_memory[718]=(u8)114;dong_porf_porf_todo_memory[719]=(u8)103;dong_porf_porf_todo_memory[720]=(u8)117;dong_porf_porf_todo_memory[721]=(u8)109;dong_porf_porf_todo_memory[722]=(u8)101;dong_porf_porf_todo_memory[723]=(u8)110;dong_porf_porf_todo_memory[724]=(u8)116;dong_porf_porf_todo_memory[725]=(u8)32;dong_porf_porf_todo_memory[726]=(u8)99;dong_porf_porf_todo_memory[727]=(u8)97;dong_porf_porf_todo_memory[728]=(u8)110;dong_porf_porf_todo_memory[729]=(u8)110;dong_porf_porf_todo_memory[730]=(u8)111;dong_porf_porf_todo_memory[731]=(u8)116;dong_porf_porf_todo_memory[732]=(u8)32;dong_porf_porf_todo_memory[733]=(u8)98;dong_porf_porf_todo_memory[734]=(u8)101;dong_porf_porf_todo_memory[735]=(u8)32;dong_porf_porf_todo_memory[736]=(u8)110;dong_porf_porf_todo_memory[737]=(u8)117;dong_porf_porf_todo_memory[738]=(u8)108;dong_porf_porf_todo_memory[739]=(u8)108;dong_porf_porf_todo_memory[740]=(u8)105;dong_porf_porf_todo_memory[741]=(u8)115;dong_porf_porf_todo_memory[742]=(u8)104;
  dong_porf_porf_todo_memory[745]=(u8)43;dong_porf_porf_todo_memory[749]=(u8)67;dong_porf_porf_todo_memory[750]=(u8)97;dong_porf_porf_todo_memory[751]=(u8)108;dong_porf_porf_todo_memory[752]=(u8)108;dong_porf_porf_todo_memory[753]=(u8)101;dong_porf_porf_todo_memory[754]=(u8)100;dong_porf_porf_todo_memory[755]=(u8)32;dong_porf_porf_todo_memory[756]=(u8)65;dong_porf_porf_todo_memory[757]=(u8)114;dong_porf_porf_todo_memory[758]=(u8)114;dong_porf_porf_todo_memory[759]=(u8)97;dong_porf_porf_todo_memory[760]=(u8)121;dong_porf_porf_todo_memory[761]=(u8)46;dong_porf_porf_todo_memory[762]=(u8)102;dong_porf_porf_todo_memory[763]=(u8)114;dong_porf_porf_todo_memory[764]=(u8)111;dong_porf_porf_todo_memory[765]=(u8)109;dong_porf_porf_todo_memory[766]=(u8)32;dong_porf_porf_todo_memory[767]=(u8)119;dong_porf_porf_todo_memory[768]=(u8)105;dong_porf_porf_todo_memory[769]=(u8)116;dong_porf_porf_todo_memory[770]=(u8)104;dong_porf_porf_todo_memory[771]=(u8)32;dong_porf_porf_todo_memory[772]=(u8)97;dong_porf_porf_todo_memory[773]=(u8)32;dong_porf_porf_todo_memory[774]=(u8)110;dong_porf_porf_todo_memory[775]=(u8)111;dong_porf_porf_todo_memory[776]=(u8)110;dong_porf_porf_todo_memory[777]=(u8)45;dong_porf_porf_todo_memory[778]=(u8)102;dong_porf_porf_todo_memory[779]=(u8)117;dong_porf_porf_todo_memory[780]=(u8)110;dong_porf_porf_todo_memory[781]=(u8)99;dong_porf_porf_todo_memory[782]=(u8)116;dong_porf_porf_todo_memory[783]=(u8)105;dong_porf_porf_todo_memory[784]=(u8)111;dong_porf_porf_todo_memory[785]=(u8)110;dong_porf_porf_todo_memory[786]=(u8)32;dong_porf_porf_todo_memory[787]=(u8)109;dong_porf_porf_todo_memory[788]=(u8)97;dong_porf_porf_todo_memory[789]=(u8)112;dong_porf_porf_todo_memory[790]=(u8)70;dong_porf_porf_todo_memory[791]=(u8)110;
  dong_porf_porf_todo_memory[794]=(u8)34;dong_porf_porf_todo_memory[798]=(u8)84;dong_porf_porf_todo_memory[799]=(u8)114;dong_porf_porf_todo_memory[800]=(u8)105;dong_porf_porf_todo_memory[801]=(u8)101;dong_porf_porf_todo_memory[802]=(u8)100;dong_porf_porf_todo_memory[803]=(u8)32;dong_porf_porf_todo_memory[804]=(u8)102;dong_porf_porf_todo_memory[805]=(u8)111;dong_porf_porf_todo_memory[806]=(u8)114;dong_porf_porf_todo_memory[807]=(u8)46;dong_porf_porf_todo_memory[808]=(u8)46;dong_porf_porf_todo_memory[809]=(u8)111;dong_porf_porf_todo_memory[810]=(u8)102;dong_porf_porf_todo_memory[811]=(u8)32;dong_porf_porf_todo_memory[812]=(u8)111;dong_porf_porf_todo_memory[813]=(u8)110;dong_porf_porf_todo_memory[814]=(u8)32;dong_porf_porf_todo_memory[815]=(u8)110;dong_porf_porf_todo_memory[816]=(u8)111;dong_porf_porf_todo_memory[817]=(u8)110;dong_porf_porf_todo_memory[818]=(u8)45;dong_porf_porf_todo_memory[819]=(u8)105;dong_porf_porf_todo_memory[820]=(u8)116;dong_porf_porf_todo_memory[821]=(u8)101;dong_porf_porf_todo_memory[822]=(u8)114;dong_porf_porf_todo_memory[823]=(u8)97;dong_porf_porf_todo_memory[824]=(u8)98;dong_porf_porf_todo_memory[825]=(u8)108;dong_porf_porf_todo_memory[826]=(u8)101;dong_porf_porf_todo_memory[827]=(u8)32;dong_porf_porf_todo_memory[828]=(u8)116;dong_porf_porf_todo_memory[829]=(u8)121;dong_porf_porf_todo_memory[830]=(u8)112;dong_porf_porf_todo_memory[831]=(u8)101;
  dong_porf_porf_todo_memory[834]=(u8)23;dong_porf_porf_todo_memory[838]=(u8)109;dong_porf_porf_todo_memory[839]=(u8)97;dong_porf_porf_todo_memory[840]=(u8)112;dong_porf_porf_todo_memory[841]=(u8)70;dong_porf_porf_todo_memory[842]=(u8)110;dong_porf_porf_todo_memory[843]=(u8)32;dong_porf_porf_todo_memory[844]=(u8)105;dong_porf_porf_todo_memory[845]=(u8)115;dong_porf_porf_todo_memory[846]=(u8)32;dong_porf_porf_todo_memory[847]=(u8)110;dong_porf_porf_todo_memory[848]=(u8)111;dong_porf_porf_todo_memory[849]=(u8)116;dong_porf_porf_todo_memory[850]=(u8)32;dong_porf_porf_todo_memory[851]=(u8)97;dong_porf_porf_todo_memory[852]=(u8)32;dong_porf_porf_todo_memory[853]=(u8)102;dong_porf_porf_todo_memory[854]=(u8)117;dong_porf_porf_todo_memory[855]=(u8)110;dong_porf_porf_todo_memory[856]=(u8)99;dong_porf_porf_todo_memory[857]=(u8)116;dong_porf_porf_todo_memory[858]=(u8)105;dong_porf_porf_todo_memory[859]=(u8)111;dong_porf_porf_todo_memory[860]=(u8)110;
  dong_porf_porf_todo_memory[863]=(u8)9;dong_porf_porf_todo_memory[867]=(u8)95;dong_porf_porf_todo_memory[868]=(u8)95;dong_porf_porf_todo_memory[869]=(u8)112;dong_porf_porf_todo_memory[870]=(u8)114;dong_porf_porf_todo_memory[871]=(u8)111;dong_porf_porf_todo_memory[872]=(u8)116;dong_porf_porf_todo_memory[873]=(u8)111;dong_porf_porf_todo_memory[874]=(u8)95;dong_porf_porf_todo_memory[875]=(u8)95;
  dong_porf_porf_todo_memory[878]=(u8)20;dong_porf_porf_todo_memory[882]=(u8)73;dong_porf_porf_todo_memory[883]=(u8)110;dong_porf_porf_todo_memory[884]=(u8)118;dong_porf_porf_todo_memory[885]=(u8)97;dong_porf_porf_todo_memory[886]=(u8)108;dong_porf_porf_todo_memory[887]=(u8)105;dong_porf_porf_todo_memory[888]=(u8)100;dong_porf_porf_todo_memory[889]=(u8)32;dong_porf_porf_todo_memory[890]=(u8)97;dong_porf_porf_todo_memory[891]=(u8)114;dong_porf_porf_todo_memory[892]=(u8)114;dong_porf_porf_todo_memory[893]=(u8)97;dong_porf_porf_todo_memory[894]=(u8)121;dong_porf_porf_todo_memory[895]=(u8)32;dong_porf_porf_todo_memory[896]=(u8)108;dong_porf_porf_todo_memory[897]=(u8)101;dong_porf_porf_todo_memory[898]=(u8)110;dong_porf_porf_todo_memory[899]=(u8)103;dong_porf_porf_todo_memory[900]=(u8)116;dong_porf_porf_todo_memory[901]=(u8)104;
  dong_porf_porf_todo_memory[904]=(u8)29;dong_porf_porf_todo_memory[908]=(u8)70;dong_porf_porf_todo_memory[909]=(u8)117;dong_porf_porf_todo_memory[910]=(u8)110;dong_porf_porf_todo_memory[911]=(u8)99;dong_porf_porf_todo_memory[912]=(u8)116;dong_porf_porf_todo_memory[913]=(u8)105;dong_porf_porf_todo_memory[914]=(u8)111;dong_porf_porf_todo_memory[915]=(u8)110;dong_porf_porf_todo_memory[916]=(u8)32;dong_porf_porf_todo_memory[917]=(u8)105;dong_porf_porf_todo_memory[918]=(u8)115;dong_porf_porf_todo_memory[919]=(u8)32;dong_porf_porf_todo_memory[920]=(u8)110;dong_porf_porf_todo_memory[921]=(u8)111;dong_porf_porf_todo_memory[922]=(u8)116;dong_porf_porf_todo_memory[923]=(u8)32;dong_porf_porf_todo_memory[924]=(u8)97;dong_porf_porf_todo_memory[925]=(u8)32;dong_porf_porf_todo_memory[926]=(u8)99;dong_porf_porf_todo_memory[927]=(u8)111;dong_porf_porf_todo_memory[928]=(u8)110;dong_porf_porf_todo_memory[929]=(u8)115;dong_porf_porf_todo_memory[930]=(u8)116;dong_porf_porf_todo_memory[931]=(u8)114;dong_porf_porf_todo_memory[932]=(u8)117;dong_porf_porf_todo_memory[933]=(u8)99;dong_porf_porf_todo_memory[934]=(u8)116;dong_porf_porf_todo_memory[935]=(u8)111;dong_porf_porf_todo_memory[936]=(u8)114;
  dong_porf_porf_todo_memory[939]=(u8)20;dong_porf_porf_todo_memory[943]=(u8)112;dong_porf_porf_todo_memory[944]=(u8)114;dong_porf_porf_todo_memory[945]=(u8)111;dong_porf_porf_todo_memory[946]=(u8)112;dong_porf_porf_todo_memory[947]=(u8)101;dong_porf_porf_todo_memory[948]=(u8)114;dong_porf_porf_todo_memory[949]=(u8)116;dong_porf_porf_todo_memory[950]=(u8)121;dong_porf_porf_todo_memory[951]=(u8)73;dong_porf_porf_todo_memory[952]=(u8)115;dong_porf_porf_todo_memory[953]=(u8)69;dong_porf_porf_todo_memory[954]=(u8)110;dong_porf_porf_todo_memory[955]=(u8)117;dong_porf_porf_todo_memory[956]=(u8)109;dong_porf_porf_todo_memory[957]=(u8)101;dong_porf_porf_todo_memory[958]=(u8)114;dong_porf_porf_todo_memory[959]=(u8)97;dong_porf_porf_todo_memory[960]=(u8)98;dong_porf_porf_todo_memory[961]=(u8)108;dong_porf_porf_todo_memory[962]=(u8)101;
  dong_porf_porf_todo_memory[965]=(u8)13;dong_porf_porf_todo_memory[969]=(u8)105;dong_porf_porf_todo_memory[970]=(u8)115;dong_porf_porf_todo_memory[971]=(u8)80;dong_porf_porf_todo_memory[972]=(u8)114;dong_porf_porf_todo_memory[973]=(u8)111;dong_porf_porf_todo_memory[974]=(u8)116;dong_porf_porf_todo_memory[975]=(u8)111;dong_porf_porf_todo_memory[976]=(u8)116;dong_porf_porf_todo_memory[977]=(u8)121;dong_porf_porf_todo_memory[978]=(u8)112;dong_porf_porf_todo_memory[979]=(u8)101;dong_porf_porf_todo_memory[980]=(u8)79;dong_porf_porf_todo_memory[981]=(u8)102;
  dong_porf_porf_todo_memory[984]=(u8)32;dong_porf_porf_todo_memory[988]=(u8)84;dong_porf_porf_todo_memory[989]=(u8)104;dong_porf_porf_todo_memory[990]=(u8)105;dong_porf_porf_todo_memory[991]=(u8)115;dong_porf_porf_todo_memory[992]=(u8)32;dong_porf_porf_todo_memory[993]=(u8)105;dong_porf_porf_todo_memory[994]=(u8)115;dong_porf_porf_todo_memory[995]=(u8)32;dong_porf_porf_todo_memory[996]=(u8)110;dong_porf_porf_todo_memory[997]=(u8)117;dong_porf_porf_todo_memory[998]=(u8)108;dong_porf_porf_todo_memory[999]=(u8)108;dong_porf_porf_todo_memory[1000]=(u8)105;dong_porf_porf_todo_memory[1001]=(u8)115;dong_porf_porf_todo_memory[1002]=(u8)104;dong_porf_porf_todo_memory[1003]=(u8)44;dong_porf_porf_todo_memory[1004]=(u8)32;dong_porf_porf_todo_memory[1005]=(u8)101;dong_porf_porf_todo_memory[1006]=(u8)120;dong_porf_porf_todo_memory[1007]=(u8)112;dong_porf_porf_todo_memory[1008]=(u8)101;dong_porf_porf_todo_memory[1009]=(u8)99;dong_porf_porf_todo_memory[1010]=(u8)116;dong_porf_porf_todo_memory[1011]=(u8)101;dong_porf_porf_todo_memory[1012]=(u8)100;dong_porf_porf_todo_memory[1013]=(u8)32;dong_porf_porf_todo_memory[1014]=(u8)111;dong_porf_porf_todo_memory[1015]=(u8)98;dong_porf_porf_todo_memory[1016]=(u8)106;dong_porf_porf_todo_memory[1017]=(u8)101;dong_porf_porf_todo_memory[1018]=(u8)99;dong_porf_porf_todo_memory[1019]=(u8)116;
  dong_porf_porf_todo_memory[1022]=(u8)14;dong_porf_porf_todo_memory[1026]=(u8)116;dong_porf_porf_todo_memory[1027]=(u8)111;dong_porf_porf_todo_memory[1028]=(u8)76;dong_porf_porf_todo_memory[1029]=(u8)111;dong_porf_porf_todo_memory[1030]=(u8)99;dong_porf_porf_todo_memory[1031]=(u8)97;dong_porf_porf_todo_memory[1032]=(u8)108;dong_porf_porf_todo_memory[1033]=(u8)101;dong_porf_porf_todo_memory[1034]=(u8)83;dong_porf_porf_todo_memory[1035]=(u8)116;dong_porf_porf_todo_memory[1036]=(u8)114;dong_porf_porf_todo_memory[1037]=(u8)105;dong_porf_porf_todo_memory[1038]=(u8)110;dong_porf_porf_todo_memory[1039]=(u8)103;
  dong_porf_porf_todo_memory[1042]=(u8)7;dong_porf_porf_todo_memory[1046]=(u8)118;dong_porf_porf_todo_memory[1047]=(u8)97;dong_porf_porf_todo_memory[1048]=(u8)108;dong_porf_porf_todo_memory[1049]=(u8)117;dong_porf_porf_todo_memory[1050]=(u8)101;dong_porf_porf_todo_memory[1051]=(u8)79;dong_porf_porf_todo_memory[1052]=(u8)102;
  dong_porf_porf_todo_memory[1055]=(u8)21;dong_porf_porf_todo_memory[1059]=(u8)111;dong_porf_porf_todo_memory[1060]=(u8)118;dong_porf_porf_todo_memory[1061]=(u8)114;dong_porf_porf_todo_memory[1062]=(u8)32;dong_porf_porf_todo_memory[1063]=(u8)105;dong_porf_porf_todo_memory[1064]=(u8)115;dong_porf_porf_todo_memory[1065]=(u8)32;dong_porf_porf_todo_memory[1066]=(u8)110;dong_porf_porf_todo_memory[1067]=(u8)111;dong_porf_porf_todo_memory[1068]=(u8)116;dong_porf_porf_todo_memory[1069]=(u8)32;dong_porf_porf_todo_memory[1070]=(u8)97;dong_porf_porf_todo_memory[1071]=(u8)32;dong_porf_porf_todo_memory[1072]=(u8)102;dong_porf_porf_todo_memory[1073]=(u8)117;dong_porf_porf_todo_memory[1074]=(u8)110;dong_porf_porf_todo_memory[1075]=(u8)99;dong_porf_porf_todo_memory[1076]=(u8)116;dong_porf_porf_todo_memory[1077]=(u8)105;dong_porf_porf_todo_memory[1078]=(u8)111;dong_porf_porf_todo_memory[1079]=(u8)110;
  dong_porf_porf_todo_memory[1082]=(u8)55;dong_porf_porf_todo_memory[1086]=(u8)83;dong_porf_porf_todo_memory[1087]=(u8)121;dong_porf_porf_todo_memory[1088]=(u8)109;dong_porf_porf_todo_memory[1089]=(u8)98;dong_porf_porf_todo_memory[1090]=(u8)111;dong_porf_porf_todo_memory[1091]=(u8)108;dong_porf_porf_todo_memory[1092]=(u8)46;dong_porf_porf_todo_memory[1093]=(u8)112;dong_porf_porf_todo_memory[1094]=(u8)114;dong_porf_porf_todo_memory[1095]=(u8)111;dong_porf_porf_todo_memory[1096]=(u8)116;dong_porf_porf_todo_memory[1097]=(u8)111;dong_porf_porf_todo_memory[1098]=(u8)116;dong_porf_porf_todo_memory[1099]=(u8)121;dong_porf_porf_todo_memory[1100]=(u8)112;dong_porf_porf_todo_memory[1101]=(u8)101;dong_porf_porf_todo_memory[1102]=(u8)46;dong_porf_porf_todo_memory[1103]=(u8)116;dong_porf_porf_todo_memory[1104]=(u8)111;dong_porf_porf_todo_memory[1105]=(u8)83;dong_porf_porf_todo_memory[1106]=(u8)116;dong_porf_porf_todo_memory[1107]=(u8)114;dong_porf_porf_todo_memory[1108]=(u8)105;dong_porf_porf_todo_memory[1109]=(u8)110;dong_porf_porf_todo_memory[1110]=(u8)103;dong_porf_porf_todo_memory[1111]=(u8)32;dong_porf_porf_todo_memory[1112]=(u8)101;dong_porf_porf_todo_memory[1113]=(u8)120;dong_porf_porf_todo_memory[1114]=(u8)112;dong_porf_porf_todo_memory[1115]=(u8)101;dong_porf_porf_todo_memory[1116]=(u8)99;dong_porf_porf_todo_memory[1117]=(u8)116;dong_porf_porf_todo_memory[1118]=(u8)115;dong_porf_porf_todo_memory[1119]=(u8)32;dong_porf_porf_todo_memory[1120]=(u8)39;dong_porf_porf_todo_memory[1121]=(u8)116;dong_porf_porf_todo_memory[1122]=(u8)104;dong_porf_porf_todo_memory[1123]=(u8)105;dong_porf_porf_todo_memory[1124]=(u8)115;dong_porf_porf_todo_memory[1125]=(u8)39;dong_porf_porf_todo_memory[1126]=(u8)32;dong_porf_porf_todo_memory[1127]=(u8)116;dong_porf_porf_todo_memory[1128]=(u8)111;dong_porf_porf_todo_memory[1129]=(u8)32;dong_porf_porf_todo_memory[1130]=(u8)98;dong_porf_porf_todo_memory[1131]=(u8)101;dong_porf_porf_todo_memory[1132]=(u8)32;dong_porf_porf_todo_memory[1133]=(u8)97;dong_porf_porf_todo_memory[1134]=(u8)32;dong_porf_porf_todo_memory[1135]=(u8)83;dong_porf_porf_todo_memory[1136]=(u8)121;dong_porf_porf_todo_memory[1137]=(u8)109;dong_porf_porf_todo_memory[1138]=(u8)98;dong_porf_porf_todo_memory[1139]=(u8)111;dong_porf_porf_todo_memory[1140]=(u8)108;
  dong_porf_porf_todo_memory[1143]=(u8)62;dong_porf_porf_todo_memory[1147]=(u8)83;dong_porf_porf_todo_memory[1148]=(u8)121;dong_porf_porf_todo_memory[1149]=(u8)109;dong_porf_porf_todo_memory[1150]=(u8)98;dong_porf_porf_todo_memory[1151]=(u8)111;dong_porf_porf_todo_memory[1152]=(u8)108;dong_porf_porf_todo_memory[1153]=(u8)46;dong_porf_porf_todo_memory[1154]=(u8)112;dong_porf_porf_todo_memory[1155]=(u8)114;dong_porf_porf_todo_memory[1156]=(u8)111;dong_porf_porf_todo_memory[1157]=(u8)116;dong_porf_porf_todo_memory[1158]=(u8)111;dong_porf_porf_todo_memory[1159]=(u8)116;dong_porf_porf_todo_memory[1160]=(u8)121;dong_porf_porf_todo_memory[1161]=(u8)112;dong_porf_porf_todo_memory[1162]=(u8)101;dong_porf_porf_todo_memory[1163]=(u8)46;dong_porf_porf_todo_memory[1164]=(u8)100;dong_porf_porf_todo_memory[1165]=(u8)101;dong_porf_porf_todo_memory[1166]=(u8)115;dong_porf_porf_todo_memory[1167]=(u8)99;dong_porf_porf_todo_memory[1168]=(u8)114;dong_porf_porf_todo_memory[1169]=(u8)105;dong_porf_porf_todo_memory[1170]=(u8)112;dong_porf_porf_todo_memory[1171]=(u8)116;dong_porf_porf_todo_memory[1172]=(u8)105;dong_porf_porf_todo_memory[1173]=(u8)111;dong_porf_porf_todo_memory[1174]=(u8)110;dong_porf_porf_todo_memory[1175]=(u8)36;dong_porf_porf_todo_memory[1176]=(u8)103;dong_porf_porf_todo_memory[1177]=(u8)101;dong_porf_porf_todo_memory[1178]=(u8)116;dong_porf_porf_todo_memory[1179]=(u8)32;dong_porf_porf_todo_memory[1180]=(u8)101;dong_porf_porf_todo_memory[1181]=(u8)120;dong_porf_porf_todo_memory[1182]=(u8)112;dong_porf_porf_todo_memory[1183]=(u8)101;dong_porf_porf_todo_memory[1184]=(u8)99;dong_porf_porf_todo_memory[1185]=(u8)116;dong_porf_porf_todo_memory[1186]=(u8)115;dong_porf_porf_todo_memory[1187]=(u8)32;dong_porf_porf_todo_memory[1188]=(u8)39;dong_porf_porf_todo_memory[1189]=(u8)116;dong_porf_porf_todo_memory[1190]=(u8)104;dong_porf_porf_todo_memory[1191]=(u8)105;dong_porf_porf_todo_memory[1192]=(u8)115;dong_porf_porf_todo_memory[1193]=(u8)39;dong_porf_porf_todo_memory[1194]=(u8)32;dong_porf_porf_todo_memory[1195]=(u8)116;dong_porf_porf_todo_memory[1196]=(u8)111;dong_porf_porf_todo_memory[1197]=(u8)32;dong_porf_porf_todo_memory[1198]=(u8)98;dong_porf_porf_todo_memory[1199]=(u8)101;dong_porf_porf_todo_memory[1200]=(u8)32;dong_porf_porf_todo_memory[1201]=(u8)97;dong_porf_porf_todo_memory[1202]=(u8)32;dong_porf_porf_todo_memory[1203]=(u8)83;dong_porf_porf_todo_memory[1204]=(u8)121;dong_porf_porf_todo_memory[1205]=(u8)109;dong_porf_porf_todo_memory[1206]=(u8)98;dong_porf_porf_todo_memory[1207]=(u8)111;dong_porf_porf_todo_memory[1208]=(u8)108;
  dong_porf_porf_todo_memory[1211]=(u8)54;dong_porf_porf_todo_memory[1215]=(u8)78;dong_porf_porf_todo_memory[1216]=(u8)117;dong_porf_porf_todo_memory[1217]=(u8)109;dong_porf_porf_todo_memory[1218]=(u8)98;dong_porf_porf_todo_memory[1219]=(u8)101;dong_porf_porf_todo_memory[1220]=(u8)114;dong_porf_porf_todo_memory[1221]=(u8)46;dong_porf_porf_todo_memory[1222]=(u8)112;dong_porf_porf_todo_memory[1223]=(u8)114;dong_porf_porf_todo_memory[1224]=(u8)111;dong_porf_porf_todo_memory[1225]=(u8)116;dong_porf_porf_todo_memory[1226]=(u8)111;dong_porf_porf_todo_memory[1227]=(u8)116;dong_porf_porf_todo_memory[1228]=(u8)121;dong_porf_porf_todo_memory[1229]=(u8)112;dong_porf_porf_todo_memory[1230]=(u8)101;dong_porf_porf_todo_memory[1231]=(u8)46;dong_porf_porf_todo_memory[1232]=(u8)118;dong_porf_porf_todo_memory[1233]=(u8)97;dong_porf_porf_todo_memory[1234]=(u8)108;dong_porf_porf_todo_memory[1235]=(u8)117;dong_porf_porf_todo_memory[1236]=(u8)101;dong_porf_porf_todo_memory[1237]=(u8)79;dong_porf_porf_todo_memory[1238]=(u8)102;dong_porf_porf_todo_memory[1239]=(u8)32;dong_porf_porf_todo_memory[1240]=(u8)101;dong_porf_porf_todo_memory[1241]=(u8)120;dong_porf_porf_todo_memory[1242]=(u8)112;dong_porf_porf_todo_memory[1243]=(u8)101;dong_porf_porf_todo_memory[1244]=(u8)99;dong_porf_porf_todo_memory[1245]=(u8)116;dong_porf_porf_todo_memory[1246]=(u8)115;dong_porf_porf_todo_memory[1247]=(u8)32;dong_porf_porf_todo_memory[1248]=(u8)39;dong_porf_porf_todo_memory[1249]=(u8)116;dong_porf_porf_todo_memory[1250]=(u8)104;dong_porf_porf_todo_memory[1251]=(u8)105;dong_porf_porf_todo_memory[1252]=(u8)115;dong_porf_porf_todo_memory[1253]=(u8)39;dong_porf_porf_todo_memory[1254]=(u8)32;dong_porf_porf_todo_memory[1255]=(u8)116;dong_porf_porf_todo_memory[1256]=(u8)111;dong_porf_porf_todo_memory[1257]=(u8)32;dong_porf_porf_todo_memory[1258]=(u8)98;dong_porf_porf_todo_memory[1259]=(u8)101;dong_porf_porf_todo_memory[1260]=(u8)32;dong_porf_porf_todo_memory[1261]=(u8)97;dong_porf_porf_todo_memory[1262]=(u8)32;dong_porf_porf_todo_memory[1263]=(u8)78;dong_porf_porf_todo_memory[1264]=(u8)117;dong_porf_porf_todo_memory[1265]=(u8)109;dong_porf_porf_todo_memory[1266]=(u8)98;dong_porf_porf_todo_memory[1267]=(u8)101;dong_porf_porf_todo_memory[1268]=(u8)114;
  dong_porf_porf_todo_memory[1271]=(u8)57;dong_porf_porf_todo_memory[1275]=(u8)83;dong_porf_porf_todo_memory[1276]=(u8)116;dong_porf_porf_todo_memory[1277]=(u8)114;dong_porf_porf_todo_memory[1278]=(u8)105;dong_porf_porf_todo_memory[1279]=(u8)110;dong_porf_porf_todo_memory[1280]=(u8)103;dong_porf_porf_todo_memory[1281]=(u8)46;dong_porf_porf_todo_memory[1282]=(u8)112;dong_porf_porf_todo_memory[1283]=(u8)114;dong_porf_porf_todo_memory[1284]=(u8)111;dong_porf_porf_todo_memory[1285]=(u8)116;dong_porf_porf_todo_memory[1286]=(u8)111;dong_porf_porf_todo_memory[1287]=(u8)116;dong_porf_porf_todo_memory[1288]=(u8)121;dong_porf_porf_todo_memory[1289]=(u8)112;dong_porf_porf_todo_memory[1290]=(u8)101;dong_porf_porf_todo_memory[1291]=(u8)46;dong_porf_porf_todo_memory[1292]=(u8)118;dong_porf_porf_todo_memory[1293]=(u8)97;dong_porf_porf_todo_memory[1294]=(u8)108;dong_porf_porf_todo_memory[1295]=(u8)117;dong_porf_porf_todo_memory[1296]=(u8)101;dong_porf_porf_todo_memory[1297]=(u8)79;dong_porf_porf_todo_memory[1298]=(u8)102;dong_porf_porf_todo_memory[1299]=(u8)32;dong_porf_porf_todo_memory[1300]=(u8)101;dong_porf_porf_todo_memory[1301]=(u8)120;dong_porf_porf_todo_memory[1302]=(u8)112;dong_porf_porf_todo_memory[1303]=(u8)101;dong_porf_porf_todo_memory[1304]=(u8)99;dong_porf_porf_todo_memory[1305]=(u8)116;dong_porf_porf_todo_memory[1306]=(u8)115;dong_porf_porf_todo_memory[1307]=(u8)32;dong_porf_porf_todo_memory[1308]=(u8)39;dong_porf_porf_todo_memory[1309]=(u8)116;dong_porf_porf_todo_memory[1310]=(u8)104;dong_porf_porf_todo_memory[1311]=(u8)105;dong_porf_porf_todo_memory[1312]=(u8)115;dong_porf_porf_todo_memory[1313]=(u8)39;dong_porf_porf_todo_memory[1314]=(u8)32;dong_porf_porf_todo_memory[1315]=(u8)116;dong_porf_porf_todo_memory[1316]=(u8)111;dong_porf_porf_todo_memory[1317]=(u8)32;dong_porf_porf_todo_memory[1318]=(u8)98;dong_porf_porf_todo_memory[1319]=(u8)101;dong_porf_porf_todo_memory[1320]=(u8)32;dong_porf_porf_todo_memory[1321]=(u8)110;dong_porf_porf_todo_memory[1322]=(u8)111;dong_porf_porf_todo_memory[1323]=(u8)110;dong_porf_porf_todo_memory[1324]=(u8)45;dong_porf_porf_todo_memory[1325]=(u8)110;dong_porf_porf_todo_memory[1326]=(u8)117;dong_porf_porf_todo_memory[1327]=(u8)108;dong_porf_porf_todo_memory[1328]=(u8)108;dong_porf_porf_todo_memory[1329]=(u8)105;dong_porf_porf_todo_memory[1330]=(u8)115;dong_porf_porf_todo_memory[1331]=(u8)104;
  dong_porf_porf_todo_memory[1334]=(u8)58;dong_porf_porf_todo_memory[1338]=(u8)83;dong_porf_porf_todo_memory[1339]=(u8)116;dong_porf_porf_todo_memory[1340]=(u8)114;dong_porf_porf_todo_memory[1341]=(u8)105;dong_porf_porf_todo_memory[1342]=(u8)110;dong_porf_porf_todo_memory[1343]=(u8)103;dong_porf_porf_todo_memory[1344]=(u8)46;dong_porf_porf_todo_memory[1345]=(u8)112;dong_porf_porf_todo_memory[1346]=(u8)114;dong_porf_porf_todo_memory[1347]=(u8)111;dong_porf_porf_todo_memory[1348]=(u8)116;dong_porf_porf_todo_memory[1349]=(u8)111;dong_porf_porf_todo_memory[1350]=(u8)116;dong_porf_porf_todo_memory[1351]=(u8)121;dong_porf_porf_todo_memory[1352]=(u8)112;dong_porf_porf_todo_memory[1353]=(u8)101;dong_porf_porf_todo_memory[1354]=(u8)46;dong_porf_porf_todo_memory[1355]=(u8)116;dong_porf_porf_todo_memory[1356]=(u8)111;dong_porf_porf_todo_memory[1357]=(u8)83;dong_porf_porf_todo_memory[1358]=(u8)116;dong_porf_porf_todo_memory[1359]=(u8)114;dong_porf_porf_todo_memory[1360]=(u8)105;dong_porf_porf_todo_memory[1361]=(u8)110;dong_porf_porf_todo_memory[1362]=(u8)103;dong_porf_porf_todo_memory[1363]=(u8)32;dong_porf_porf_todo_memory[1364]=(u8)101;dong_porf_porf_todo_memory[1365]=(u8)120;dong_porf_porf_todo_memory[1366]=(u8)112;dong_porf_porf_todo_memory[1367]=(u8)101;dong_porf_porf_todo_memory[1368]=(u8)99;dong_porf_porf_todo_memory[1369]=(u8)116;dong_porf_porf_todo_memory[1370]=(u8)115;dong_porf_porf_todo_memory[1371]=(u8)32;dong_porf_porf_todo_memory[1372]=(u8)39;dong_porf_porf_todo_memory[1373]=(u8)116;dong_porf_porf_todo_memory[1374]=(u8)104;dong_porf_porf_todo_memory[1375]=(u8)105;dong_porf_porf_todo_memory[1376]=(u8)115;dong_porf_porf_todo_memory[1377]=(u8)39;dong_porf_porf_todo_memory[1378]=(u8)32;dong_porf_porf_todo_memory[1379]=(u8)116;dong_porf_porf_todo_memory[1380]=(u8)111;dong_porf_porf_todo_memory[1381]=(u8)32;dong_porf_porf_todo_memory[1382]=(u8)98;dong_porf_porf_todo_memory[1383]=(u8)101;dong_porf_porf_todo_memory[1384]=(u8)32;dong_porf_porf_todo_memory[1385]=(u8)110;dong_porf_porf_todo_memory[1386]=(u8)111;dong_porf_porf_todo_memory[1387]=(u8)110;dong_porf_porf_todo_memory[1388]=(u8)45;dong_porf_porf_todo_memory[1389]=(u8)110;dong_porf_porf_todo_memory[1390]=(u8)117;dong_porf_porf_todo_memory[1391]=(u8)108;dong_porf_porf_todo_memory[1392]=(u8)108;dong_porf_porf_todo_memory[1393]=(u8)105;dong_porf_porf_todo_memory[1394]=(u8)115;dong_porf_porf_todo_memory[1395]=(u8)104;
  dong_porf_porf_todo_memory[1398]=(u8)37;dong_porf_porf_todo_memory[1402]=(u8)67;dong_porf_porf_todo_memory[1403]=(u8)97;dong_porf_porf_todo_memory[1404]=(u8)110;dong_porf_porf_todo_memory[1405]=(u8)110;dong_porf_porf_todo_memory[1406]=(u8)111;dong_porf_porf_todo_memory[1407]=(u8)116;dong_porf_porf_todo_memory[1408]=(u8)32;dong_porf_porf_todo_memory[1409]=(u8)99;dong_porf_porf_todo_memory[1410]=(u8)111;dong_porf_porf_todo_memory[1411]=(u8)110;dong_porf_porf_todo_memory[1412]=(u8)118;dong_porf_porf_todo_memory[1413]=(u8)101;dong_porf_porf_todo_memory[1414]=(u8)114;dong_porf_porf_todo_memory[1415]=(u8)116;dong_porf_porf_todo_memory[1416]=(u8)32;dong_porf_porf_todo_memory[1417]=(u8)97;dong_porf_porf_todo_memory[1418]=(u8)110;dong_porf_porf_todo_memory[1419]=(u8)32;dong_porf_porf_todo_memory[1420]=(u8)111;dong_porf_porf_todo_memory[1421]=(u8)98;dong_porf_porf_todo_memory[1422]=(u8)106;dong_porf_porf_todo_memory[1423]=(u8)101;dong_porf_porf_todo_memory[1424]=(u8)99;dong_porf_porf_todo_memory[1425]=(u8)116;dong_porf_porf_todo_memory[1426]=(u8)32;dong_porf_porf_todo_memory[1427]=(u8)116;dong_porf_porf_todo_memory[1428]=(u8)111;dong_porf_porf_todo_memory[1429]=(u8)32;dong_porf_porf_todo_memory[1430]=(u8)112;dong_porf_porf_todo_memory[1431]=(u8)114;dong_porf_porf_todo_memory[1432]=(u8)105;dong_porf_porf_todo_memory[1433]=(u8)109;dong_porf_porf_todo_memory[1434]=(u8)105;dong_porf_porf_todo_memory[1435]=(u8)116;dong_porf_porf_todo_memory[1436]=(u8)105;dong_porf_porf_todo_memory[1437]=(u8)118;dong_porf_porf_todo_memory[1438]=(u8)101;
  dong_porf_porf_todo_memory[1441]=(u8)18;dong_porf_porf_todo_memory[1445]=(u8)91;dong_porf_porf_todo_memory[1446]=(u8)111;dong_porf_porf_todo_memory[1447]=(u8)98;dong_porf_porf_todo_memory[1448]=(u8)106;dong_porf_porf_todo_memory[1449]=(u8)101;dong_porf_porf_todo_memory[1450]=(u8)99;dong_porf_porf_todo_memory[1451]=(u8)116;dong_porf_porf_todo_memory[1452]=(u8)32;dong_porf_porf_todo_memory[1453]=(u8)85;dong_porf_porf_todo_memory[1454]=(u8)110;dong_porf_porf_todo_memory[1455]=(u8)100;dong_porf_porf_todo_memory[1456]=(u8)101;dong_porf_porf_todo_memory[1457]=(u8)102;dong_porf_porf_todo_memory[1458]=(u8)105;dong_porf_porf_todo_memory[1459]=(u8)110;dong_porf_porf_todo_memory[1460]=(u8)101;dong_porf_porf_todo_memory[1461]=(u8)100;dong_porf_porf_todo_memory[1462]=(u8)93;
  dong_porf_porf_todo_memory[1465]=(u8)13;dong_porf_porf_todo_memory[1469]=(u8)91;dong_porf_porf_todo_memory[1470]=(u8)111;dong_porf_porf_todo_memory[1471]=(u8)98;dong_porf_porf_todo_memory[1472]=(u8)106;dong_porf_porf_todo_memory[1473]=(u8)101;dong_porf_porf_todo_memory[1474]=(u8)99;dong_porf_porf_todo_memory[1475]=(u8)116;dong_porf_porf_todo_memory[1476]=(u8)32;dong_porf_porf_todo_memory[1477]=(u8)78;dong_porf_porf_todo_memory[1478]=(u8)117;dong_porf_porf_todo_memory[1479]=(u8)108;dong_porf_porf_todo_memory[1480]=(u8)108;dong_porf_porf_todo_memory[1481]=(u8)93;
  dong_porf_porf_todo_memory[1484]=(u8)14;dong_porf_porf_todo_memory[1488]=(u8)91;dong_porf_porf_todo_memory[1489]=(u8)111;dong_porf_porf_todo_memory[1490]=(u8)98;dong_porf_porf_todo_memory[1491]=(u8)106;dong_porf_porf_todo_memory[1492]=(u8)101;dong_porf_porf_todo_memory[1493]=(u8)99;dong_porf_porf_todo_memory[1494]=(u8)116;dong_porf_porf_todo_memory[1495]=(u8)32;dong_porf_porf_todo_memory[1496]=(u8)65;dong_porf_porf_todo_memory[1497]=(u8)114;dong_porf_porf_todo_memory[1498]=(u8)114;dong_porf_porf_todo_memory[1499]=(u8)97;dong_porf_porf_todo_memory[1500]=(u8)121;dong_porf_porf_todo_memory[1501]=(u8)93;
  dong_porf_porf_todo_memory[1504]=(u8)17;dong_porf_porf_todo_memory[1508]=(u8)91;dong_porf_porf_todo_memory[1509]=(u8)111;dong_porf_porf_todo_memory[1510]=(u8)98;dong_porf_porf_todo_memory[1511]=(u8)106;dong_porf_porf_todo_memory[1512]=(u8)101;dong_porf_porf_todo_memory[1513]=(u8)99;dong_porf_porf_todo_memory[1514]=(u8)116;dong_porf_porf_todo_memory[1515]=(u8)32;dong_porf_porf_todo_memory[1516]=(u8)70;dong_porf_porf_todo_memory[1517]=(u8)117;dong_porf_porf_todo_memory[1518]=(u8)110;dong_porf_porf_todo_memory[1519]=(u8)99;dong_porf_porf_todo_memory[1520]=(u8)116;dong_porf_porf_todo_memory[1521]=(u8)105;dong_porf_porf_todo_memory[1522]=(u8)111;dong_porf_porf_todo_memory[1523]=(u8)110;dong_porf_porf_todo_memory[1524]=(u8)93;
  dong_porf_porf_todo_memory[1527]=(u8)16;dong_porf_porf_todo_memory[1531]=(u8)91;dong_porf_porf_todo_memory[1532]=(u8)111;dong_porf_porf_todo_memory[1533]=(u8)98;dong_porf_porf_todo_memory[1534]=(u8)106;dong_porf_porf_todo_memory[1535]=(u8)101;dong_porf_porf_todo_memory[1536]=(u8)99;dong_porf_porf_todo_memory[1537]=(u8)116;dong_porf_porf_todo_memory[1538]=(u8)32;dong_porf_porf_todo_memory[1539]=(u8)66;dong_porf_porf_todo_memory[1540]=(u8)111;dong_porf_porf_todo_memory[1541]=(u8)111;dong_porf_porf_todo_memory[1542]=(u8)108;dong_porf_porf_todo_memory[1543]=(u8)101;dong_porf_porf_todo_memory[1544]=(u8)97;dong_porf_porf_todo_memory[1545]=(u8)110;dong_porf_porf_todo_memory[1546]=(u8)93;
  dong_porf_porf_todo_memory[1549]=(u8)15;dong_porf_porf_todo_memory[1553]=(u8)91;dong_porf_porf_todo_memory[1554]=(u8)111;dong_porf_porf_todo_memory[1555]=(u8)98;dong_porf_porf_todo_memory[1556]=(u8)106;dong_porf_porf_todo_memory[1557]=(u8)101;dong_porf_porf_todo_memory[1558]=(u8)99;dong_porf_porf_todo_memory[1559]=(u8)116;dong_porf_porf_todo_memory[1560]=(u8)32;dong_porf_porf_todo_memory[1561]=(u8)78;dong_porf_porf_todo_memory[1562]=(u8)117;dong_porf_porf_todo_memory[1563]=(u8)109;dong_porf_porf_todo_memory[1564]=(u8)98;dong_porf_porf_todo_memory[1565]=(u8)101;dong_porf_porf_todo_memory[1566]=(u8)114;dong_porf_porf_todo_memory[1567]=(u8)93;
  dong_porf_porf_todo_memory[1570]=(u8)15;dong_porf_porf_todo_memory[1574]=(u8)91;dong_porf_porf_todo_memory[1575]=(u8)111;dong_porf_porf_todo_memory[1576]=(u8)98;dong_porf_porf_todo_memory[1577]=(u8)106;dong_porf_porf_todo_memory[1578]=(u8)101;dong_porf_porf_todo_memory[1579]=(u8)99;dong_porf_porf_todo_memory[1580]=(u8)116;dong_porf_porf_todo_memory[1581]=(u8)32;dong_porf_porf_todo_memory[1582]=(u8)83;dong_porf_porf_todo_memory[1583]=(u8)116;dong_porf_porf_todo_memory[1584]=(u8)114;dong_porf_porf_todo_memory[1585]=(u8)105;dong_porf_porf_todo_memory[1586]=(u8)110;dong_porf_porf_todo_memory[1587]=(u8)103;dong_porf_porf_todo_memory[1588]=(u8)93;
  dong_porf_porf_todo_memory[1591]=(u8)13;dong_porf_porf_todo_memory[1595]=(u8)91;dong_porf_porf_todo_memory[1596]=(u8)111;dong_porf_porf_todo_memory[1597]=(u8)98;dong_porf_porf_todo_memory[1598]=(u8)106;dong_porf_porf_todo_memory[1599]=(u8)101;dong_porf_porf_todo_memory[1600]=(u8)99;dong_porf_porf_todo_memory[1601]=(u8)116;dong_porf_porf_todo_memory[1602]=(u8)32;dong_porf_porf_todo_memory[1603]=(u8)68;dong_porf_porf_todo_memory[1604]=(u8)97;dong_porf_porf_todo_memory[1605]=(u8)116;dong_porf_porf_todo_memory[1606]=(u8)101;dong_porf_porf_todo_memory[1607]=(u8)93;
  dong_porf_porf_todo_memory[1610]=(u8)15;dong_porf_porf_todo_memory[1614]=(u8)91;dong_porf_porf_todo_memory[1615]=(u8)111;dong_porf_porf_todo_memory[1616]=(u8)98;dong_porf_porf_todo_memory[1617]=(u8)106;dong_porf_porf_todo_memory[1618]=(u8)101;dong_porf_porf_todo_memory[1619]=(u8)99;dong_porf_porf_todo_memory[1620]=(u8)116;dong_porf_porf_todo_memory[1621]=(u8)32;dong_porf_porf_todo_memory[1622]=(u8)82;dong_porf_porf_todo_memory[1623]=(u8)101;dong_porf_porf_todo_memory[1624]=(u8)103;dong_porf_porf_todo_memory[1625]=(u8)69;dong_porf_porf_todo_memory[1626]=(u8)120;dong_porf_porf_todo_memory[1627]=(u8)112;dong_porf_porf_todo_memory[1628]=(u8)93;
  dong_porf_porf_todo_memory[1631]=(u8)15;dong_porf_porf_todo_memory[1635]=(u8)91;dong_porf_porf_todo_memory[1636]=(u8)111;dong_porf_porf_todo_memory[1637]=(u8)98;dong_porf_porf_todo_memory[1638]=(u8)106;dong_porf_porf_todo_memory[1639]=(u8)101;dong_porf_porf_todo_memory[1640]=(u8)99;dong_porf_porf_todo_memory[1641]=(u8)116;dong_porf_porf_todo_memory[1642]=(u8)32;dong_porf_porf_todo_memory[1643]=(u8)79;dong_porf_porf_todo_memory[1644]=(u8)98;dong_porf_porf_todo_memory[1645]=(u8)106;dong_porf_porf_todo_memory[1646]=(u8)101;dong_porf_porf_todo_memory[1647]=(u8)99;dong_porf_porf_todo_memory[1648]=(u8)116;dong_porf_porf_todo_memory[1649]=(u8)93;
  dong_porf_porf_todo_memory[1652]=(u8)43;dong_porf_porf_todo_memory[1656]=(u8)67;dong_porf_porf_todo_memory[1657]=(u8)97;dong_porf_porf_todo_memory[1658]=(u8)110;dong_porf_porf_todo_memory[1659]=(u8)110;dong_porf_porf_todo_memory[1660]=(u8)111;dong_porf_porf_todo_memory[1661]=(u8)116;dong_porf_porf_todo_memory[1662]=(u8)32;dong_porf_porf_todo_memory[1663]=(u8)99;dong_porf_porf_todo_memory[1664]=(u8)111;dong_porf_porf_todo_memory[1665]=(u8)110;dong_porf_porf_todo_memory[1666]=(u8)118;dong_porf_porf_todo_memory[1667]=(u8)101;dong_porf_porf_todo_memory[1668]=(u8)114;dong_porf_porf_todo_memory[1669]=(u8)116;dong_porf_porf_todo_memory[1670]=(u8)32;dong_porf_porf_todo_memory[1671]=(u8)83;dong_porf_porf_todo_memory[1672]=(u8)121;dong_porf_porf_todo_memory[1673]=(u8)109;dong_porf_porf_todo_memory[1674]=(u8)98;dong_porf_porf_todo_memory[1675]=(u8)111;dong_porf_porf_todo_memory[1676]=(u8)108;dong_porf_porf_todo_memory[1677]=(u8)32;dong_porf_porf_todo_memory[1678]=(u8)111;dong_porf_porf_todo_memory[1679]=(u8)114;dong_porf_porf_todo_memory[1680]=(u8)32;dong_porf_porf_todo_memory[1681]=(u8)66;dong_porf_porf_todo_memory[1682]=(u8)105;dong_porf_porf_todo_memory[1683]=(u8)103;dong_porf_porf_todo_memory[1684]=(u8)73;dong_porf_porf_todo_memory[1685]=(u8)110;dong_porf_porf_todo_memory[1686]=(u8)116;dong_porf_porf_todo_memory[1687]=(u8)32;dong_porf_porf_todo_memory[1688]=(u8)116;dong_porf_porf_todo_memory[1689]=(u8)111;dong_porf_porf_todo_memory[1690]=(u8)32;dong_porf_porf_todo_memory[1691]=(u8)97;dong_porf_porf_todo_memory[1692]=(u8)32;dong_porf_porf_todo_memory[1693]=(u8)110;dong_porf_porf_todo_memory[1694]=(u8)117;dong_porf_porf_todo_memory[1695]=(u8)109;dong_porf_porf_todo_memory[1696]=(u8)98;dong_porf_porf_todo_memory[1697]=(u8)101;dong_porf_porf_todo_memory[1698]=(u8)114;
  dong_porf_porf_todo_memory[1701]=(u8)54;dong_porf_porf_todo_memory[1705]=(u8)83;dong_porf_porf_todo_memory[1706]=(u8)116;dong_porf_porf_todo_memory[1707]=(u8)114;dong_porf_porf_todo_memory[1708]=(u8)105;dong_porf_porf_todo_memory[1709]=(u8)110;dong_porf_porf_todo_memory[1710]=(u8)103;dong_porf_porf_todo_memory[1711]=(u8)46;dong_porf_porf_todo_memory[1712]=(u8)112;dong_porf_porf_todo_memory[1713]=(u8)114;dong_porf_porf_todo_memory[1714]=(u8)111;dong_porf_porf_todo_memory[1715]=(u8)116;dong_porf_porf_todo_memory[1716]=(u8)111;dong_porf_porf_todo_memory[1717]=(u8)116;dong_porf_porf_todo_memory[1718]=(u8)121;dong_porf_porf_todo_memory[1719]=(u8)112;dong_porf_porf_todo_memory[1720]=(u8)101;dong_porf_porf_todo_memory[1721]=(u8)46;dong_porf_porf_todo_memory[1722]=(u8)116;dong_porf_porf_todo_memory[1723]=(u8)114;dong_porf_porf_todo_memory[1724]=(u8)105;dong_porf_porf_todo_memory[1725]=(u8)109;dong_porf_porf_todo_memory[1726]=(u8)32;dong_porf_porf_todo_memory[1727]=(u8)101;dong_porf_porf_todo_memory[1728]=(u8)120;dong_porf_porf_todo_memory[1729]=(u8)112;dong_porf_porf_todo_memory[1730]=(u8)101;dong_porf_porf_todo_memory[1731]=(u8)99;dong_porf_porf_todo_memory[1732]=(u8)116;dong_porf_porf_todo_memory[1733]=(u8)115;dong_porf_porf_todo_memory[1734]=(u8)32;dong_porf_porf_todo_memory[1735]=(u8)39;dong_porf_porf_todo_memory[1736]=(u8)116;dong_porf_porf_todo_memory[1737]=(u8)104;dong_porf_porf_todo_memory[1738]=(u8)105;dong_porf_porf_todo_memory[1739]=(u8)115;dong_porf_porf_todo_memory[1740]=(u8)39;dong_porf_porf_todo_memory[1741]=(u8)32;dong_porf_porf_todo_memory[1742]=(u8)116;dong_porf_porf_todo_memory[1743]=(u8)111;dong_porf_porf_todo_memory[1744]=(u8)32;dong_porf_porf_todo_memory[1745]=(u8)98;dong_porf_porf_todo_memory[1746]=(u8)101;dong_porf_porf_todo_memory[1747]=(u8)32;dong_porf_porf_todo_memory[1748]=(u8)110;dong_porf_porf_todo_memory[1749]=(u8)111;dong_porf_porf_todo_memory[1750]=(u8)110;dong_porf_porf_todo_memory[1751]=(u8)45;dong_porf_porf_todo_memory[1752]=(u8)110;dong_porf_porf_todo_memory[1753]=(u8)117;dong_porf_porf_todo_memory[1754]=(u8)108;dong_porf_porf_todo_memory[1755]=(u8)108;dong_porf_porf_todo_memory[1756]=(u8)105;dong_porf_porf_todo_memory[1757]=(u8)115;dong_porf_porf_todo_memory[1758]=(u8)104;
  dong_porf_porf_todo_memory[1761]=(u8)57;dong_porf_porf_todo_memory[1765]=(u8)83;dong_porf_porf_todo_memory[1766]=(u8)116;dong_porf_porf_todo_memory[1767]=(u8)114;dong_porf_porf_todo_memory[1768]=(u8)105;dong_porf_porf_todo_memory[1769]=(u8)110;dong_porf_porf_todo_memory[1770]=(u8)103;dong_porf_porf_todo_memory[1771]=(u8)46;dong_porf_porf_todo_memory[1772]=(u8)112;dong_porf_porf_todo_memory[1773]=(u8)114;dong_porf_porf_todo_memory[1774]=(u8)111;dong_porf_porf_todo_memory[1775]=(u8)116;dong_porf_porf_todo_memory[1776]=(u8)111;dong_porf_porf_todo_memory[1777]=(u8)116;dong_porf_porf_todo_memory[1778]=(u8)121;dong_porf_porf_todo_memory[1779]=(u8)112;dong_porf_porf_todo_memory[1780]=(u8)101;dong_porf_porf_todo_memory[1781]=(u8)46;dong_porf_porf_todo_memory[1782]=(u8)116;dong_porf_porf_todo_memory[1783]=(u8)114;dong_porf_porf_todo_memory[1784]=(u8)105;dong_porf_porf_todo_memory[1785]=(u8)109;dong_porf_porf_todo_memory[1786]=(u8)69;dong_porf_porf_todo_memory[1787]=(u8)110;dong_porf_porf_todo_memory[1788]=(u8)100;dong_porf_porf_todo_memory[1789]=(u8)32;dong_porf_porf_todo_memory[1790]=(u8)101;dong_porf_porf_todo_memory[1791]=(u8)120;dong_porf_porf_todo_memory[1792]=(u8)112;dong_porf_porf_todo_memory[1793]=(u8)101;dong_porf_porf_todo_memory[1794]=(u8)99;dong_porf_porf_todo_memory[1795]=(u8)116;dong_porf_porf_todo_memory[1796]=(u8)115;dong_porf_porf_todo_memory[1797]=(u8)32;dong_porf_porf_todo_memory[1798]=(u8)39;dong_porf_porf_todo_memory[1799]=(u8)116;dong_porf_porf_todo_memory[1800]=(u8)104;dong_porf_porf_todo_memory[1801]=(u8)105;dong_porf_porf_todo_memory[1802]=(u8)115;dong_porf_porf_todo_memory[1803]=(u8)39;dong_porf_porf_todo_memory[1804]=(u8)32;dong_porf_porf_todo_memory[1805]=(u8)116;dong_porf_porf_todo_memory[1806]=(u8)111;dong_porf_porf_todo_memory[1807]=(u8)32;dong_porf_porf_todo_memory[1808]=(u8)98;dong_porf_porf_todo_memory[1809]=(u8)101;dong_porf_porf_todo_memory[1810]=(u8)32;dong_porf_porf_todo_memory[1811]=(u8)110;dong_porf_porf_todo_memory[1812]=(u8)111;dong_porf_porf_todo_memory[1813]=(u8)110;dong_porf_porf_todo_memory[1814]=(u8)45;dong_porf_porf_todo_memory[1815]=(u8)110;dong_porf_porf_todo_memory[1816]=(u8)117;dong_porf_porf_todo_memory[1817]=(u8)108;dong_porf_porf_todo_memory[1818]=(u8)108;dong_porf_porf_todo_memory[1819]=(u8)105;dong_porf_porf_todo_memory[1820]=(u8)115;dong_porf_porf_todo_memory[1821]=(u8)104;
  dong_porf_porf_todo_memory[1824]=(u8)59;dong_porf_porf_todo_memory[1828]=(u8)83;dong_porf_porf_todo_memory[1829]=(u8)116;dong_porf_porf_todo_memory[1830]=(u8)114;dong_porf_porf_todo_memory[1831]=(u8)105;dong_porf_porf_todo_memory[1832]=(u8)110;dong_porf_porf_todo_memory[1833]=(u8)103;dong_porf_porf_todo_memory[1834]=(u8)46;dong_porf_porf_todo_memory[1835]=(u8)112;dong_porf_porf_todo_memory[1836]=(u8)114;dong_porf_porf_todo_memory[1837]=(u8)111;dong_porf_porf_todo_memory[1838]=(u8)116;dong_porf_porf_todo_memory[1839]=(u8)111;dong_porf_porf_todo_memory[1840]=(u8)116;dong_porf_porf_todo_memory[1841]=(u8)121;dong_porf_porf_todo_memory[1842]=(u8)112;dong_porf_porf_todo_memory[1843]=(u8)101;dong_porf_porf_todo_memory[1844]=(u8)46;dong_porf_porf_todo_memory[1845]=(u8)116;dong_porf_porf_todo_memory[1846]=(u8)114;dong_porf_porf_todo_memory[1847]=(u8)105;dong_porf_porf_todo_memory[1848]=(u8)109;dong_porf_porf_todo_memory[1849]=(u8)83;dong_porf_porf_todo_memory[1850]=(u8)116;dong_porf_porf_todo_memory[1851]=(u8)97;dong_porf_porf_todo_memory[1852]=(u8)114;dong_porf_porf_todo_memory[1853]=(u8)116;dong_porf_porf_todo_memory[1854]=(u8)32;dong_porf_porf_todo_memory[1855]=(u8)101;dong_porf_porf_todo_memory[1856]=(u8)120;dong_porf_porf_todo_memory[1857]=(u8)112;dong_porf_porf_todo_memory[1858]=(u8)101;dong_porf_porf_todo_memory[1859]=(u8)99;dong_porf_porf_todo_memory[1860]=(u8)116;dong_porf_porf_todo_memory[1861]=(u8)115;dong_porf_porf_todo_memory[1862]=(u8)32;dong_porf_porf_todo_memory[1863]=(u8)39;dong_porf_porf_todo_memory[1864]=(u8)116;dong_porf_porf_todo_memory[1865]=(u8)104;dong_porf_porf_todo_memory[1866]=(u8)105;dong_porf_porf_todo_memory[1867]=(u8)115;dong_porf_porf_todo_memory[1868]=(u8)39;dong_porf_porf_todo_memory[1869]=(u8)32;dong_porf_porf_todo_memory[1870]=(u8)116;dong_porf_porf_todo_memory[1871]=(u8)111;dong_porf_porf_todo_memory[1872]=(u8)32;dong_porf_porf_todo_memory[1873]=(u8)98;dong_porf_porf_todo_memory[1874]=(u8)101;dong_porf_porf_todo_memory[1875]=(u8)32;dong_porf_porf_todo_memory[1876]=(u8)110;dong_porf_porf_todo_memory[1877]=(u8)111;dong_porf_porf_todo_memory[1878]=(u8)110;dong_porf_porf_todo_memory[1879]=(u8)45;dong_porf_porf_todo_memory[1880]=(u8)110;dong_porf_porf_todo_memory[1881]=(u8)117;dong_porf_porf_todo_memory[1882]=(u8)108;dong_porf_porf_todo_memory[1883]=(u8)108;dong_porf_porf_todo_memory[1884]=(u8)105;dong_porf_porf_todo_memory[1885]=(u8)115;dong_porf_porf_todo_memory[1886]=(u8)104;
  dong_porf_porf_todo_memory[1889]=(u8)62;dong_porf_porf_todo_memory[1893]=(u8)39;dong_porf_porf_todo_memory[1894]=(u8)116;dong_porf_porf_todo_memory[1895]=(u8)114;dong_porf_porf_todo_memory[1896]=(u8)105;dong_porf_porf_todo_memory[1897]=(u8)109;dong_porf_porf_todo_memory[1898]=(u8)39;dong_porf_porf_todo_memory[1899]=(u8)32;dong_porf_porf_todo_memory[1900]=(u8)112;dong_porf_porf_todo_memory[1901]=(u8)114;dong_porf_porf_todo_memory[1902]=(u8)111;dong_porf_porf_todo_memory[1903]=(u8)116;dong_porf_porf_todo_memory[1904]=(u8)111;dong_porf_porf_todo_memory[1905]=(u8)32;dong_porf_porf_todo_memory[1906]=(u8)102;dong_porf_porf_todo_memory[1907]=(u8)117;dong_porf_porf_todo_memory[1908]=(u8)110;dong_porf_porf_todo_memory[1909]=(u8)99;dong_porf_porf_todo_memory[1910]=(u8)32;dong_porf_porf_todo_memory[1911]=(u8)116;dong_porf_porf_todo_memory[1912]=(u8)114;dong_porf_porf_todo_memory[1913]=(u8)105;dong_porf_porf_todo_memory[1914]=(u8)101;dong_porf_porf_todo_memory[1915]=(u8)100;dong_porf_porf_todo_memory[1916]=(u8)32;dong_porf_porf_todo_memory[1917]=(u8)116;dong_porf_porf_todo_memory[1918]=(u8)111;dong_porf_porf_todo_memory[1919]=(u8)32;dong_porf_porf_todo_memory[1920]=(u8)98;dong_porf_porf_todo_memory[1921]=(u8)101;dong_porf_porf_todo_memory[1922]=(u8)32;dong_porf_porf_todo_memory[1923]=(u8)99;dong_porf_porf_todo_memory[1924]=(u8)97;dong_porf_porf_todo_memory[1925]=(u8)108;dong_porf_porf_todo_memory[1926]=(u8)108;dong_porf_porf_todo_memory[1927]=(u8)101;dong_porf_porf_todo_memory[1928]=(u8)100;dong_porf_porf_todo_memory[1929]=(u8)32;dong_porf_porf_todo_memory[1930]=(u8)111;dong_porf_porf_todo_memory[1931]=(u8)110;dong_porf_porf_todo_memory[1932]=(u8)32;dong_porf_porf_todo_memory[1933]=(u8)97;dong_porf_porf_todo_memory[1934]=(u8)32;dong_porf_porf_todo_memory[1935]=(u8)116;dong_porf_porf_todo_memory[1936]=(u8)121;dong_porf_porf_todo_memory[1937]=(u8)112;dong_porf_porf_todo_memory[1938]=(u8)101;dong_porf_porf_todo_memory[1939]=(u8)32;dong_porf_porf_todo_memory[1940]=(u8)119;dong_porf_porf_todo_memory[1941]=(u8)105;dong_porf_porf_todo_memory[1942]=(u8)116;dong_porf_porf_todo_memory[1943]=(u8)104;dong_porf_porf_todo_memory[1944]=(u8)111;dong_porf_porf_todo_memory[1945]=(u8)117;dong_porf_porf_todo_memory[1946]=(u8)116;dong_porf_porf_todo_memory[1947]=(u8)32;dong_porf_porf_todo_memory[1948]=(u8)97;dong_porf_porf_todo_memory[1949]=(u8)110;dong_porf_porf_todo_memory[1950]=(u8)32;dong_porf_porf_todo_memory[1951]=(u8)105;dong_porf_porf_todo_memory[1952]=(u8)109;dong_porf_porf_todo_memory[1953]=(u8)112;dong_porf_porf_todo_memory[1954]=(u8)108;
  dong_porf_porf_todo_memory[1957]=(u8)60;dong_porf_porf_todo_memory[1961]=(u8)83;dong_porf_porf_todo_memory[1962]=(u8)116;dong_porf_porf_todo_memory[1963]=(u8)114;dong_porf_porf_todo_memory[1964]=(u8)105;dong_porf_porf_todo_memory[1965]=(u8)110;dong_porf_porf_todo_memory[1966]=(u8)103;dong_porf_porf_todo_memory[1967]=(u8)46;dong_porf_porf_todo_memory[1968]=(u8)112;dong_porf_porf_todo_memory[1969]=(u8)114;dong_porf_porf_todo_memory[1970]=(u8)111;dong_porf_porf_todo_memory[1971]=(u8)116;dong_porf_porf_todo_memory[1972]=(u8)111;dong_porf_porf_todo_memory[1973]=(u8)116;dong_porf_porf_todo_memory[1974]=(u8)121;dong_porf_porf_todo_memory[1975]=(u8)112;dong_porf_porf_todo_memory[1976]=(u8)101;dong_porf_porf_todo_memory[1977]=(u8)46;dong_porf_porf_todo_memory[1978]=(u8)99;dong_porf_porf_todo_memory[1979]=(u8)104;dong_porf_porf_todo_memory[1980]=(u8)97;dong_porf_porf_todo_memory[1981]=(u8)114;dong_porf_porf_todo_memory[1982]=(u8)67;dong_porf_porf_todo_memory[1983]=(u8)111;dong_porf_porf_todo_memory[1984]=(u8)100;dong_porf_porf_todo_memory[1985]=(u8)101;dong_porf_porf_todo_memory[1986]=(u8)65;dong_porf_porf_todo_memory[1987]=(u8)116;dong_porf_porf_todo_memory[1988]=(u8)32;dong_porf_porf_todo_memory[1989]=(u8)101;dong_porf_porf_todo_memory[1990]=(u8)120;dong_porf_porf_todo_memory[1991]=(u8)112;dong_porf_porf_todo_memory[1992]=(u8)101;dong_porf_porf_todo_memory[1993]=(u8)99;dong_porf_porf_todo_memory[1994]=(u8)116;dong_porf_porf_todo_memory[1995]=(u8)115;dong_porf_porf_todo_memory[1996]=(u8)32;dong_porf_porf_todo_memory[1997]=(u8)39;dong_porf_porf_todo_memory[1998]=(u8)116;dong_porf_porf_todo_memory[1999]=(u8)104;dong_porf_porf_todo_memory[2000]=(u8)105;dong_porf_porf_todo_memory[2001]=(u8)115;dong_porf_porf_todo_memory[2002]=(u8)39;dong_porf_porf_todo_memory[2003]=(u8)32;dong_porf_porf_todo_memory[2004]=(u8)116;dong_porf_porf_todo_memory[2005]=(u8)111;dong_porf_porf_todo_memory[2006]=(u8)32;dong_porf_porf_todo_memory[2007]=(u8)98;dong_porf_porf_todo_memory[2008]=(u8)101;dong_porf_porf_todo_memory[2009]=(u8)32;dong_porf_porf_todo_memory[2010]=(u8)110;dong_porf_porf_todo_memory[2011]=(u8)111;dong_porf_porf_todo_memory[2012]=(u8)110;dong_porf_porf_todo_memory[2013]=(u8)45;dong_porf_porf_todo_memory[2014]=(u8)110;dong_porf_porf_todo_memory[2015]=(u8)117;dong_porf_porf_todo_memory[2016]=(u8)108;dong_porf_porf_todo_memory[2017]=(u8)108;dong_porf_porf_todo_memory[2018]=(u8)105;dong_porf_porf_todo_memory[2019]=(u8)115;dong_porf_porf_todo_memory[2020]=(u8)104;
  dong_porf_porf_todo_memory[2023]=(u8)68;dong_porf_porf_todo_memory[2027]=(u8)39;dong_porf_porf_todo_memory[2028]=(u8)99;dong_porf_porf_todo_memory[2029]=(u8)104;dong_porf_porf_todo_memory[2030]=(u8)97;dong_porf_porf_todo_memory[2031]=(u8)114;dong_porf_porf_todo_memory[2032]=(u8)67;dong_porf_porf_todo_memory[2033]=(u8)111;dong_porf_porf_todo_memory[2034]=(u8)100;dong_porf_porf_todo_memory[2035]=(u8)101;dong_porf_porf_todo_memory[2036]=(u8)65;dong_porf_porf_todo_memory[2037]=(u8)116;dong_porf_porf_todo_memory[2038]=(u8)39;dong_porf_porf_todo_memory[2039]=(u8)32;dong_porf_porf_todo_memory[2040]=(u8)112;dong_porf_porf_todo_memory[2041]=(u8)114;dong_porf_porf_todo_memory[2042]=(u8)111;dong_porf_porf_todo_memory[2043]=(u8)116;dong_porf_porf_todo_memory[2044]=(u8)111;dong_porf_porf_todo_memory[2045]=(u8)32;dong_porf_porf_todo_memory[2046]=(u8)102;dong_porf_porf_todo_memory[2047]=(u8)117;dong_porf_porf_todo_memory[2048]=(u8)110;dong_porf_porf_todo_memory[2049]=(u8)99;dong_porf_porf_todo_memory[2050]=(u8)32;dong_porf_porf_todo_memory[2051]=(u8)116;dong_porf_porf_todo_memory[2052]=(u8)114;dong_porf_porf_todo_memory[2053]=(u8)105;dong_porf_porf_todo_memory[2054]=(u8)101;dong_porf_porf_todo_memory[2055]=(u8)100;dong_porf_porf_todo_memory[2056]=(u8)32;dong_porf_porf_todo_memory[2057]=(u8)116;dong_porf_porf_todo_memory[2058]=(u8)111;dong_porf_porf_todo_memory[2059]=(u8)32;dong_porf_porf_todo_memory[2060]=(u8)98;dong_porf_porf_todo_memory[2061]=(u8)101;dong_porf_porf_todo_memory[2062]=(u8)32;dong_porf_porf_todo_memory[2063]=(u8)99;dong_porf_porf_todo_memory[2064]=(u8)97;dong_porf_porf_todo_memory[2065]=(u8)108;dong_porf_porf_todo_memory[2066]=(u8)108;dong_porf_porf_todo_memory[2067]=(u8)101;dong_porf_porf_todo_memory[2068]=(u8)100;dong_porf_porf_todo_memory[2069]=(u8)32;dong_porf_porf_todo_memory[2070]=(u8)111;dong_porf_porf_todo_memory[2071]=(u8)110;dong_porf_porf_todo_memory[2072]=(u8)32;dong_porf_porf_todo_memory[2073]=(u8)97;dong_porf_porf_todo_memory[2074]=(u8)32;dong_porf_porf_todo_memory[2075]=(u8)116;dong_porf_porf_todo_memory[2076]=(u8)121;dong_porf_porf_todo_memory[2077]=(u8)112;dong_porf_porf_todo_memory[2078]=(u8)101;dong_porf_porf_todo_memory[2079]=(u8)32;dong_porf_porf_todo_memory[2080]=(u8)119;dong_porf_porf_todo_memory[2081]=(u8)105;dong_porf_porf_todo_memory[2082]=(u8)116;dong_porf_porf_todo_memory[2083]=(u8)104;dong_porf_porf_todo_memory[2084]=(u8)111;dong_porf_porf_todo_memory[2085]=(u8)117;dong_porf_porf_todo_memory[2086]=(u8)116;dong_porf_porf_todo_memory[2087]=(u8)32;dong_porf_porf_todo_memory[2088]=(u8)97;dong_porf_porf_todo_memory[2089]=(u8)110;dong_porf_porf_todo_memory[2090]=(u8)32;dong_porf_porf_todo_memory[2091]=(u8)105;dong_porf_porf_todo_memory[2092]=(u8)109;dong_porf_porf_todo_memory[2093]=(u8)112;dong_porf_porf_todo_memory[2094]=(u8)108;
  dong_porf_porf_todo_memory[2097]=(u8)33;dong_porf_porf_todo_memory[2101]=(u8)67;dong_porf_porf_todo_memory[2102]=(u8)97;dong_porf_porf_todo_memory[2103]=(u8)110;dong_porf_porf_todo_memory[2104]=(u8)110;dong_porf_porf_todo_memory[2105]=(u8)111;dong_porf_porf_todo_memory[2106]=(u8)116;dong_porf_porf_todo_memory[2107]=(u8)32;dong_porf_porf_todo_memory[2108]=(u8)114;dong_porf_porf_todo_memory[2109]=(u8)101;dong_porf_porf_todo_memory[2110]=(u8)97;dong_porf_porf_todo_memory[2111]=(u8)100;dong_porf_porf_todo_memory[2112]=(u8)32;dong_porf_porf_todo_memory[2113]=(u8)112;dong_porf_porf_todo_memory[2114]=(u8)114;dong_porf_porf_todo_memory[2115]=(u8)111;dong_porf_porf_todo_memory[2116]=(u8)112;dong_porf_porf_todo_memory[2117]=(u8)101;dong_porf_porf_todo_memory[2118]=(u8)114;dong_porf_porf_todo_memory[2119]=(u8)116;dong_porf_porf_todo_memory[2120]=(u8)121;dong_porf_porf_todo_memory[2121]=(u8)32;dong_porf_porf_todo_memory[2122]=(u8)111;dong_porf_porf_todo_memory[2123]=(u8)102;dong_porf_porf_todo_memory[2124]=(u8)32;dong_porf_porf_todo_memory[2125]=(u8)117;dong_porf_porf_todo_memory[2126]=(u8)110;dong_porf_porf_todo_memory[2127]=(u8)100;dong_porf_porf_todo_memory[2128]=(u8)101;dong_porf_porf_todo_memory[2129]=(u8)102;dong_porf_porf_todo_memory[2130]=(u8)105;dong_porf_porf_todo_memory[2131]=(u8)110;dong_porf_porf_todo_memory[2132]=(u8)101;dong_porf_porf_todo_memory[2133]=(u8)100;
  dong_porf_porf_todo_memory[2136]=(u8)10;dong_porf_porf_todo_memory[2140]=(u8)99;dong_porf_porf_todo_memory[2141]=(u8)104;dong_porf_porf_todo_memory[2142]=(u8)97;dong_porf_porf_todo_memory[2143]=(u8)114;dong_porf_porf_todo_memory[2144]=(u8)67;dong_porf_porf_todo_memory[2145]=(u8)111;dong_porf_porf_todo_memory[2146]=(u8)100;dong_porf_porf_todo_memory[2147]=(u8)101;dong_porf_porf_todo_memory[2148]=(u8)65;dong_porf_porf_todo_memory[2149]=(u8)116;
  dong_porf_porf_todo_memory[2152]=(u8)27;dong_porf_porf_todo_memory[2156]=(u8)117;dong_porf_porf_todo_memory[2157]=(u8)110;dong_porf_porf_todo_memory[2158]=(u8)100;dong_porf_porf_todo_memory[2159]=(u8)101;dong_porf_porf_todo_memory[2160]=(u8)102;dong_porf_porf_todo_memory[2161]=(u8)105;dong_porf_porf_todo_memory[2162]=(u8)110;dong_porf_porf_todo_memory[2163]=(u8)101;dong_porf_porf_todo_memory[2164]=(u8)100;dong_porf_porf_todo_memory[2165]=(u8)32;dong_porf_porf_todo_memory[2166]=(u8)105;dong_porf_porf_todo_memory[2167]=(u8)115;dong_porf_porf_todo_memory[2168]=(u8)32;dong_porf_porf_todo_memory[2169]=(u8)110;dong_porf_porf_todo_memory[2170]=(u8)111;dong_porf_porf_todo_memory[2171]=(u8)116;dong_porf_porf_todo_memory[2172]=(u8)32;dong_porf_porf_todo_memory[2173]=(u8)97;dong_porf_porf_todo_memory[2174]=(u8)32;dong_porf_porf_todo_memory[2175]=(u8)102;dong_porf_porf_todo_memory[2176]=(u8)117;dong_porf_porf_todo_memory[2177]=(u8)110;dong_porf_porf_todo_memory[2178]=(u8)99;dong_porf_porf_todo_memory[2179]=(u8)116;dong_porf_porf_todo_memory[2180]=(u8)105;dong_porf_porf_todo_memory[2181]=(u8)111;dong_porf_porf_todo_memory[2182]=(u8)110;
  dong_porf_porf_todo_memory[2185]=(u8)16;dong_porf_porf_todo_memory[2189]=(u8)116;dong_porf_porf_todo_memory[2190]=(u8)111;dong_porf_porf_todo_memory[2191]=(u8)100;dong_porf_porf_todo_memory[2192]=(u8)111;dong_porf_porf_todo_memory[2193]=(u8)32;dong_porf_porf_todo_memory[2194]=(u8)109;dong_porf_porf_todo_memory[2195]=(u8)97;dong_porf_porf_todo_memory[2196]=(u8)120;dong_porf_porf_todo_memory[2197]=(u8)32;dong_porf_porf_todo_memory[2198]=(u8)114;dong_porf_porf_todo_memory[2199]=(u8)101;dong_porf_porf_todo_memory[2200]=(u8)97;dong_porf_porf_todo_memory[2201]=(u8)99;dong_porf_porf_todo_memory[2202]=(u8)104;dong_porf_porf_todo_memory[2203]=(u8)101;dong_porf_porf_todo_memory[2204]=(u8)100;
  dong_porf_porf_todo_memory[2207]=(u8)5;dong_porf_porf_todo_memory[2211]=(u8)65;dong_porf_porf_todo_memory[2212]=(u8)108;dong_porf_porf_todo_memory[2213]=(u8)108;dong_porf_porf_todo_memory[2214]=(u8)32;dong_porf_porf_todo_memory[2215]=(u8)40;
  dong_porf_porf_todo_memory[2218]=(u8)1;dong_porf_porf_todo_memory[2222]=(u8)41;
  dong_porf_porf_todo_memory[2225]=(u8)8;dong_porf_porf_todo_memory[2229]=(u8)65;dong_porf_porf_todo_memory[2230]=(u8)99;dong_porf_porf_todo_memory[2231]=(u8)116;dong_porf_porf_todo_memory[2232]=(u8)105;dong_porf_porf_todo_memory[2233]=(u8)118;dong_porf_porf_todo_memory[2234]=(u8)101;dong_porf_porf_todo_memory[2235]=(u8)32;dong_porf_porf_todo_memory[2236]=(u8)40;
  dong_porf_porf_todo_memory[2239]=(u8)6;dong_porf_porf_todo_memory[2243]=(u8)68;dong_porf_porf_todo_memory[2244]=(u8)111;dong_porf_porf_todo_memory[2245]=(u8)110;dong_porf_porf_todo_memory[2246]=(u8)101;dong_porf_porf_todo_memory[2247]=(u8)32;dong_porf_porf_todo_memory[2248]=(u8)40;
  dong_porf_porf_todo_memory[2251]=(u8)7;dong_porf_porf_todo_memory[2255]=(u8)35;dong_porf_porf_todo_memory[2256]=(u8)51;dong_porf_porf_todo_memory[2257]=(u8)52;dong_porf_porf_todo_memory[2258]=(u8)57;dong_porf_porf_todo_memory[2259]=(u8)56;dong_porf_porf_todo_memory[2260]=(u8)100;dong_porf_porf_todo_memory[2261]=(u8)98;
  dong_porf_porf_todo_memory[2264]=(u8)7;dong_porf_porf_todo_memory[2268]=(u8)35;dong_porf_porf_todo_memory[2269]=(u8)101;dong_porf_porf_todo_memory[2270]=(u8)99;dong_porf_porf_todo_memory[2271]=(u8)102;dong_porf_porf_todo_memory[2272]=(u8)48;dong_porf_porf_todo_memory[2273]=(u8)102;dong_porf_porf_todo_memory[2274]=(u8)49;
  dong_porf_porf_todo_memory[2277]=(u8)4;dong_porf_porf_todo_memory[2281]=(u8)35;dong_porf_porf_todo_memory[2282]=(u8)102;dong_porf_porf_todo_memory[2283]=(u8)102;dong_porf_porf_todo_memory[2284]=(u8)102;
  dong_porf_porf_todo_memory[2287]=(u8)7;dong_porf_porf_todo_memory[2291]=(u8)35;dong_porf_porf_todo_memory[2292]=(u8)55;dong_porf_porf_todo_memory[2293]=(u8)102;dong_porf_porf_todo_memory[2294]=(u8)56;dong_porf_porf_todo_memory[2295]=(u8)99;dong_porf_porf_todo_memory[2296]=(u8)56;dong_porf_porf_todo_memory[2297]=(u8)100;
  dong_porf_porf_todo_memory[2300]=(u8)16;dong_porf_porf_todo_memory[2304]=(u8)98;dong_porf_porf_todo_memory[2305]=(u8)97;dong_porf_porf_todo_memory[2306]=(u8)99;dong_porf_porf_todo_memory[2307]=(u8)107;dong_porf_porf_todo_memory[2308]=(u8)103;dong_porf_porf_todo_memory[2309]=(u8)114;dong_porf_porf_todo_memory[2310]=(u8)111;dong_porf_porf_todo_memory[2311]=(u8)117;dong_porf_porf_todo_memory[2312]=(u8)110;dong_porf_porf_todo_memory[2313]=(u8)100;dong_porf_porf_todo_memory[2314]=(u8)45;dong_porf_porf_todo_memory[2315]=(u8)99;dong_porf_porf_todo_memory[2316]=(u8)111;dong_porf_porf_todo_memory[2317]=(u8)108;dong_porf_porf_todo_memory[2318]=(u8)111;dong_porf_porf_todo_memory[2319]=(u8)114;
  dong_porf_porf_todo_memory[2322]=(u8)5;dong_porf_porf_todo_memory[2326]=(u8)99;dong_porf_porf_todo_memory[2327]=(u8)111;dong_porf_porf_todo_memory[2328]=(u8)108;dong_porf_porf_todo_memory[2329]=(u8)111;dong_porf_porf_todo_memory[2330]=(u8)114;
  dong_porf_porf_todo_memory[2333]=(u8)6;dong_porf_porf_todo_memory[2337]=(u8)104;dong_porf_porf_todo_memory[2338]=(u8)105;dong_porf_porf_todo_memory[2339]=(u8)100;dong_porf_porf_todo_memory[2340]=(u8)100;dong_porf_porf_todo_memory[2341]=(u8)101;dong_porf_porf_todo_memory[2342]=(u8)110;
  dong_porf_porf_todo_memory[2345]=(u8)12;dong_porf_porf_todo_memory[2349]=(u8)67;dong_porf_porf_todo_memory[2350]=(u8)108;dong_porf_porf_todo_memory[2351]=(u8)101;dong_porf_porf_todo_memory[2352]=(u8)97;dong_porf_porf_todo_memory[2353]=(u8)114;dong_porf_porf_todo_memory[2354]=(u8)32;dong_porf_porf_todo_memory[2355]=(u8)100;dong_porf_porf_todo_memory[2356]=(u8)111;dong_porf_porf_todo_memory[2357]=(u8)110;dong_porf_porf_todo_memory[2358]=(u8)101;dong_porf_porf_todo_memory[2359]=(u8)32;dong_porf_porf_todo_memory[2360]=(u8)40;
  dong_porf_porf_todo_memory[2363]=(u8)1;dong_porf_porf_todo_memory[2367]=(u8)49;
  dong_porf_porf_todo_memory[2370]=(u8)7;dong_porf_porf_todo_memory[2374]=(u8)35;dong_porf_porf_todo_memory[2375]=(u8)50;dong_porf_porf_todo_memory[2376]=(u8)99;dong_porf_porf_todo_memory[2377]=(u8)51;dong_porf_porf_todo_memory[2378]=(u8)101;dong_porf_porf_todo_memory[2379]=(u8)53;dong_porf_porf_todo_memory[2380]=(u8)48;
  dong_porf_porf_todo_memory[2383]=(u8)11;dong_porf_porf_todo_memory[2387]=(u8)116;dong_porf_porf_todo_memory[2388]=(u8)114;dong_porf_porf_todo_memory[2389]=(u8)97;dong_porf_porf_todo_memory[2390]=(u8)110;dong_porf_porf_todo_memory[2391]=(u8)115;dong_porf_porf_todo_memory[2392]=(u8)112;dong_porf_porf_todo_memory[2393]=(u8)97;dong_porf_porf_todo_memory[2394]=(u8)114;dong_porf_porf_todo_memory[2395]=(u8)101;dong_porf_porf_todo_memory[2396]=(u8)110;dong_porf_porf_todo_memory[2397]=(u8)116;
  dong_porf_porf_todo_memory[2400]=(u8)17;dong_porf_porf_todo_memory[2404]=(u8)50;dong_porf_porf_todo_memory[2405]=(u8)112;dong_porf_porf_todo_memory[2406]=(u8)120;dong_porf_porf_todo_memory[2407]=(u8)32;dong_porf_porf_todo_memory[2408]=(u8)115;dong_porf_porf_todo_memory[2409]=(u8)111;dong_porf_porf_todo_memory[2410]=(u8)108;dong_porf_porf_todo_memory[2411]=(u8)105;dong_porf_porf_todo_memory[2412]=(u8)100;dong_porf_porf_todo_memory[2413]=(u8)32;dong_porf_porf_todo_memory[2414]=(u8)35;dong_porf_porf_todo_memory[2415]=(u8)98;dong_porf_porf_todo_memory[2416]=(u8)100;dong_porf_porf_todo_memory[2417]=(u8)99;dong_porf_porf_todo_memory[2418]=(u8)51;dong_porf_porf_todo_memory[2419]=(u8)99;dong_porf_porf_todo_memory[2420]=(u8)55;
  dong_porf_porf_todo_memory[2423]=(u8)12;dong_porf_porf_todo_memory[2427]=(u8)108;dong_porf_porf_todo_memory[2428]=(u8)105;dong_porf_porf_todo_memory[2429]=(u8)110;dong_porf_porf_todo_memory[2430]=(u8)101;dong_porf_porf_todo_memory[2431]=(u8)45;dong_porf_porf_todo_memory[2432]=(u8)116;dong_porf_porf_todo_memory[2433]=(u8)104;dong_porf_porf_todo_memory[2434]=(u8)114;dong_porf_porf_todo_memory[2435]=(u8)111;dong_porf_porf_todo_memory[2436]=(u8)117;dong_porf_porf_todo_memory[2437]=(u8)103;dong_porf_porf_todo_memory[2438]=(u8)104;
  dong_porf_porf_todo_memory[2441]=(u8)7;dong_porf_porf_todo_memory[2445]=(u8)35;dong_porf_porf_todo_memory[2446]=(u8)57;dong_porf_porf_todo_memory[2447]=(u8)53;dong_porf_porf_todo_memory[2448]=(u8)97;dong_porf_porf_todo_memory[2449]=(u8)53;dong_porf_porf_todo_memory[2450]=(u8)97;dong_porf_porf_todo_memory[2451]=(u8)54;
  dong_porf_porf_todo_memory[2454]=(u8)7;dong_porf_porf_todo_memory[2458]=(u8)35;dong_porf_porf_todo_memory[2459]=(u8)50;dong_porf_porf_todo_memory[2460]=(u8)55;dong_porf_porf_todo_memory[2461]=(u8)97;dong_porf_porf_todo_memory[2462]=(u8)101;dong_porf_porf_todo_memory[2463]=(u8)54;dong_porf_porf_todo_memory[2464]=(u8)48;
  dong_porf_porf_todo_memory[2467]=(u8)4;dong_porf_porf_todo_memory[2471]=(u8)110;dong_porf_porf_todo_memory[2472]=(u8)111;dong_porf_porf_todo_memory[2473]=(u8)110;dong_porf_porf_todo_memory[2474]=(u8)101;
  dong_porf_porf_todo_memory[2477]=(u8)1;dong_porf_porf_todo_memory[2481]=(u8)86;
  dong_porf_porf_todo_memory[2484]=(u8)153;dong_porf_porf_todo_memory[2488]=(u8)60;dong_porf_porf_todo_memory[2489]=(u8)100;dong_porf_porf_todo_memory[2490]=(u8)105;dong_porf_porf_todo_memory[2491]=(u8)118;dong_porf_porf_todo_memory[2492]=(u8)32;dong_porf_porf_todo_memory[2493]=(u8)115;dong_porf_porf_todo_memory[2494]=(u8)116;dong_porf_porf_todo_memory[2495]=(u8)121;dong_porf_porf_todo_memory[2496]=(u8)108;dong_porf_porf_todo_memory[2497]=(u8)101;dong_porf_porf_todo_memory[2498]=(u8)61;dong_porf_porf_todo_memory[2499]=(u8)34;dong_porf_porf_todo_memory[2500]=(u8)100;dong_porf_porf_todo_memory[2501]=(u8)105;dong_porf_porf_todo_memory[2502]=(u8)115;dong_porf_porf_todo_memory[2503]=(u8)112;dong_porf_porf_todo_memory[2504]=(u8)108;dong_porf_porf_todo_memory[2505]=(u8)97;dong_porf_porf_todo_memory[2506]=(u8)121;dong_porf_porf_todo_memory[2507]=(u8)58;dong_porf_porf_todo_memory[2508]=(u8)102;dong_porf_porf_todo_memory[2509]=(u8)108;dong_porf_porf_todo_memory[2510]=(u8)101;dong_porf_porf_todo_memory[2511]=(u8)120;dong_porf_porf_todo_memory[2512]=(u8)59;dong_porf_porf_todo_memory[2513]=(u8)97;dong_porf_porf_todo_memory[2514]=(u8)108;dong_porf_porf_todo_memory[2515]=(u8)105;dong_porf_porf_todo_memory[2516]=(u8)103;dong_porf_porf_todo_memory[2517]=(u8)110;dong_porf_porf_todo_memory[2518]=(u8)45;dong_porf_porf_todo_memory[2519]=(u8)105;dong_porf_porf_todo_memory[2520]=(u8)116;dong_porf_porf_todo_memory[2521]=(u8)101;dong_porf_porf_todo_memory[2522]=(u8)109;dong_porf_porf_todo_memory[2523]=(u8)115;dong_porf_porf_todo_memory[2524]=(u8)58;dong_porf_porf_todo_memory[2525]=(u8)99;dong_porf_porf_todo_memory[2526]=(u8)101;dong_porf_porf_todo_memory[2527]=(u8)110;dong_porf_porf_todo_memory[2528]=(u8)116;dong_porf_porf_todo_memory[2529]=(u8)101;dong_porf_porf_todo_memory[2530]=(u8)114;dong_porf_porf_todo_memory[2531]=(u8)59;dong_porf_porf_todo_memory[2532]=(u8)112;dong_porf_porf_todo_memory[2533]=(u8)97;dong_porf_porf_todo_memory[2534]=(u8)100;dong_porf_porf_todo_memory[2535]=(u8)100;dong_porf_porf_todo_memory[2536]=(u8)105;dong_porf_porf_todo_memory[2537]=(u8)110;dong_porf_porf_todo_memory[2538]=(u8)103;dong_porf_porf_todo_memory[2539]=(u8)58;dong_porf_porf_todo_memory[2540]=(u8)49;dong_porf_porf_todo_memory[2541]=(u8)50;dong_porf_porf_todo_memory[2542]=(u8)112;dong_porf_porf_todo_memory[2543]=(u8)120;dong_porf_porf_todo_memory[2544]=(u8)32;dong_porf_porf_todo_memory[2545]=(u8)49;dong_porf_porf_todo_memory[2546]=(u8)54;dong_porf_porf_todo_memory[2547]=(u8)112;dong_porf_porf_todo_memory[2548]=(u8)120;dong_porf_porf_todo_memory[2549]=(u8)59;dong_porf_porf_todo_memory[2550]=(u8)98;dong_porf_porf_todo_memory[2551]=(u8)97;dong_porf_porf_todo_memory[2552]=(u8)99;dong_porf_porf_todo_memory[2553]=(u8)107;dong_porf_porf_todo_memory[2554]=(u8)103;dong_porf_porf_todo_memory[2555]=(u8)114;dong_porf_porf_todo_memory[2556]=(u8)111;dong_porf_porf_todo_memory[2557]=(u8)117;dong_porf_porf_todo_memory[2558]=(u8)110;dong_porf_porf_todo_memory[2559]=(u8)100;dong_porf_porf_todo_memory[2560]=(u8)58;dong_porf_porf_todo_memory[2561]=(u8)35;dong_porf_porf_todo_memory[2562]=(u8)102;dong_porf_porf_todo_memory[2563]=(u8)102;dong_porf_porf_todo_memory[2564]=(u8)102;dong_porf_porf_todo_memory[2565]=(u8)59;dong_porf_porf_todo_memory[2566]=(u8)98;dong_porf_porf_todo_memory[2567]=(u8)111;dong_porf_porf_todo_memory[2568]=(u8)114;dong_porf_porf_todo_memory[2569]=(u8)100;dong_porf_porf_todo_memory[2570]=(u8)101;dong_porf_porf_todo_memory[2571]=(u8)114;dong_porf_porf_todo_memory[2572]=(u8)45;dong_porf_porf_todo_memory[2573]=(u8)114;dong_porf_porf_todo_memory[2574]=(u8)97;dong_porf_porf_todo_memory[2575]=(u8)100;dong_porf_porf_todo_memory[2576]=(u8)105;dong_porf_porf_todo_memory[2577]=(u8)117;dong_porf_porf_todo_memory[2578]=(u8)115;dong_porf_porf_todo_memory[2579]=(u8)58;dong_porf_porf_todo_memory[2580]=(u8)56;dong_porf_porf_todo_memory[2581]=(u8)112;dong_porf_porf_todo_memory[2582]=(u8)120;dong_porf_porf_todo_memory[2583]=(u8)59;dong_porf_porf_todo_memory[2584]=(u8)109;dong_porf_porf_todo_memory[2585]=(u8)97;dong_porf_porf_todo_memory[2586]=(u8)114;dong_porf_porf_todo_memory[2587]=(u8)103;dong_porf_porf_todo_memory[2588]=(u8)105;dong_porf_porf_todo_memory[2589]=(u8)110;dong_porf_porf_todo_memory[2590]=(u8)45;dong_porf_porf_todo_memory[2591]=(u8)98;dong_porf_porf_todo_memory[2592]=(u8)111;dong_porf_porf_todo_memory[2593]=(u8)116;dong_porf_porf_todo_memory[2594]=(u8)116;dong_porf_porf_todo_memory[2595]=(u8)111;dong_porf_porf_todo_memory[2596]=(u8)109;dong_porf_porf_todo_memory[2597]=(u8)58;dong_porf_porf_todo_memory[2598]=(u8)56;dong_porf_porf_todo_memory[2599]=(u8)112;dong_porf_porf_todo_memory[2600]=(u8)120;dong_porf_porf_todo_memory[2601]=(u8)59;dong_porf_porf_todo_memory[2602]=(u8)98;dong_porf_porf_todo_memory[2603]=(u8)111;dong_porf_porf_todo_memory[2604]=(u8)120;dong_porf_porf_todo_memory[2605]=(u8)45;dong_porf_porf_todo_memory[2606]=(u8)115;dong_porf_porf_todo_memory[2607]=(u8)104;dong_porf_porf_todo_memory[2608]=(u8)97;dong_porf_porf_todo_memory[2609]=(u8)100;dong_porf_porf_todo_memory[2610]=(u8)111;dong_porf_porf_todo_memory[2611]=(u8)119;dong_porf_porf_todo_memory[2612]=(u8)58;dong_porf_porf_todo_memory[2613]=(u8)48;dong_porf_porf_todo_memory[2614]=(u8)32;dong_porf_porf_todo_memory[2615]=(u8)49;dong_porf_porf_todo_memory[2616]=(u8)112;dong_porf_porf_todo_memory[2617]=(u8)120;dong_porf_porf_todo_memory[2618]=(u8)32;dong_porf_porf_todo_memory[2619]=(u8)51;dong_porf_porf_todo_memory[2620]=(u8)112;dong_porf_porf_todo_memory[2621]=(u8)120;dong_porf_porf_todo_memory[2622]=(u8)32;dong_porf_porf_todo_memory[2623]=(u8)114;dong_porf_porf_todo_memory[2624]=(u8)103;dong_porf_porf_todo_memory[2625]=(u8)98;dong_porf_porf_todo_memory[2626]=(u8)97;dong_porf_porf_todo_memory[2627]=(u8)40;dong_porf_porf_todo_memory[2628]=(u8)48;dong_porf_porf_todo_memory[2629]=(u8)44;dong_porf_porf_todo_memory[2630]=(u8)48;dong_porf_porf_todo_memory[2631]=(u8)44;dong_porf_porf_todo_memory[2632]=(u8)48;dong_porf_porf_todo_memory[2633]=(u8)44;dong_porf_porf_todo_memory[2634]=(u8)48;dong_porf_porf_todo_memory[2635]=(u8)46;dong_porf_porf_todo_memory[2636]=(u8)49;dong_porf_porf_todo_memory[2637]=(u8)41;dong_porf_porf_todo_memory[2638]=(u8)59;dong_porf_porf_todo_memory[2639]=(u8)34;dong_porf_porf_todo_memory[2640]=(u8)62;
  dong_porf_porf_todo_memory[2643]=(u8)29;dong_porf_porf_todo_memory[2647]=(u8)60;dong_porf_porf_todo_memory[2648]=(u8)100;dong_porf_porf_todo_memory[2649]=(u8)105;dong_porf_porf_todo_memory[2650]=(u8)118;dong_porf_porf_todo_memory[2651]=(u8)32;dong_porf_porf_todo_memory[2652]=(u8)100;dong_porf_porf_todo_memory[2653]=(u8)97;dong_porf_porf_todo_memory[2654]=(u8)116;dong_porf_porf_todo_memory[2655]=(u8)97;dong_porf_porf_todo_memory[2656]=(u8)45;dong_porf_porf_todo_memory[2657]=(u8)116;dong_porf_porf_todo_memory[2658]=(u8)111;dong_porf_porf_todo_memory[2659]=(u8)100;dong_porf_porf_todo_memory[2660]=(u8)111;dong_porf_porf_todo_memory[2661]=(u8)45;dong_porf_porf_todo_memory[2662]=(u8)116;dong_porf_porf_todo_memory[2663]=(u8)111;dong_porf_porf_todo_memory[2664]=(u8)103;dong_porf_porf_todo_memory[2665]=(u8)103;dong_porf_porf_todo_memory[2666]=(u8)108;dong_porf_porf_todo_memory[2667]=(u8)101;dong_porf_porf_todo_memory[2668]=(u8)45;dong_porf_porf_todo_memory[2669]=(u8)105;dong_porf_porf_todo_memory[2670]=(u8)110;dong_porf_porf_todo_memory[2671]=(u8)100;dong_porf_porf_todo_memory[2672]=(u8)101;dong_porf_porf_todo_memory[2673]=(u8)120;dong_porf_porf_todo_memory[2674]=(u8)61;dong_porf_porf_todo_memory[2675]=(u8)34;
  dong_porf_porf_todo_memory[2678]=(u8)57;dong_porf_porf_todo_memory[2682]=(u8)34;dong_porf_porf_todo_memory[2683]=(u8)32;dong_porf_porf_todo_memory[2684]=(u8)115;dong_porf_porf_todo_memory[2685]=(u8)116;dong_porf_porf_todo_memory[2686]=(u8)121;dong_porf_porf_todo_memory[2687]=(u8)108;dong_porf_porf_todo_memory[2688]=(u8)101;dong_porf_porf_todo_memory[2689]=(u8)61;dong_porf_porf_todo_memory[2690]=(u8)34;dong_porf_porf_todo_memory[2691]=(u8)119;dong_porf_porf_todo_memory[2692]=(u8)105;dong_porf_porf_todo_memory[2693]=(u8)100;dong_porf_porf_todo_memory[2694]=(u8)116;dong_porf_porf_todo_memory[2695]=(u8)104;dong_porf_porf_todo_memory[2696]=(u8)58;dong_porf_porf_todo_memory[2697]=(u8)50;dong_porf_porf_todo_memory[2698]=(u8)52;dong_porf_porf_todo_memory[2699]=(u8)112;dong_porf_porf_todo_memory[2700]=(u8)120;dong_porf_porf_todo_memory[2701]=(u8)59;dong_porf_porf_todo_memory[2702]=(u8)104;dong_porf_porf_todo_memory[2703]=(u8)101;dong_porf_porf_todo_memory[2704]=(u8)105;dong_porf_porf_todo_memory[2705]=(u8)103;dong_porf_porf_todo_memory[2706]=(u8)104;dong_porf_porf_todo_memory[2707]=(u8)116;dong_porf_porf_todo_memory[2708]=(u8)58;dong_porf_porf_todo_memory[2709]=(u8)50;dong_porf_porf_todo_memory[2710]=(u8)52;dong_porf_porf_todo_memory[2711]=(u8)112;dong_porf_porf_todo_memory[2712]=(u8)120;dong_porf_porf_todo_memory[2713]=(u8)59;dong_porf_porf_todo_memory[2714]=(u8)98;dong_porf_porf_todo_memory[2715]=(u8)111;dong_porf_porf_todo_memory[2716]=(u8)114;dong_porf_porf_todo_memory[2717]=(u8)100;dong_porf_porf_todo_memory[2718]=(u8)101;dong_porf_porf_todo_memory[2719]=(u8)114;dong_porf_porf_todo_memory[2720]=(u8)45;dong_porf_porf_todo_memory[2721]=(u8)114;dong_porf_porf_todo_memory[2722]=(u8)97;dong_porf_porf_todo_memory[2723]=(u8)100;dong_porf_porf_todo_memory[2724]=(u8)105;dong_porf_porf_todo_memory[2725]=(u8)117;dong_porf_porf_todo_memory[2726]=(u8)115;dong_porf_porf_todo_memory[2727]=(u8)58;dong_porf_porf_todo_memory[2728]=(u8)53;dong_porf_porf_todo_memory[2729]=(u8)48;dong_porf_porf_todo_memory[2730]=(u8)37;dong_porf_porf_todo_memory[2731]=(u8)59;dong_porf_porf_todo_memory[2732]=(u8)98;dong_porf_porf_todo_memory[2733]=(u8)111;dong_porf_porf_todo_memory[2734]=(u8)114;dong_porf_porf_todo_memory[2735]=(u8)100;dong_porf_porf_todo_memory[2736]=(u8)101;dong_porf_porf_todo_memory[2737]=(u8)114;dong_porf_porf_todo_memory[2738]=(u8)58;
  dong_porf_porf_todo_memory[2741]=(u8)12;dong_porf_porf_todo_memory[2745]=(u8)59;dong_porf_porf_todo_memory[2746]=(u8)98;dong_porf_porf_todo_memory[2747]=(u8)97;dong_porf_porf_todo_memory[2748]=(u8)99;dong_porf_porf_todo_memory[2749]=(u8)107;dong_porf_porf_todo_memory[2750]=(u8)103;dong_porf_porf_todo_memory[2751]=(u8)114;dong_porf_porf_todo_memory[2752]=(u8)111;dong_porf_porf_todo_memory[2753]=(u8)117;dong_porf_porf_todo_memory[2754]=(u8)110;dong_porf_porf_todo_memory[2755]=(u8)100;dong_porf_porf_todo_memory[2756]=(u8)58;
  dong_porf_porf_todo_memory[2759]=(u8)134;dong_porf_porf_todo_memory[2763]=(u8)59;dong_porf_porf_todo_memory[2764]=(u8)109;dong_porf_porf_todo_memory[2765]=(u8)97;dong_porf_porf_todo_memory[2766]=(u8)114;dong_porf_porf_todo_memory[2767]=(u8)103;dong_porf_porf_todo_memory[2768]=(u8)105;dong_porf_porf_todo_memory[2769]=(u8)110;dong_porf_porf_todo_memory[2770]=(u8)45;dong_porf_porf_todo_memory[2771]=(u8)114;dong_porf_porf_todo_memory[2772]=(u8)105;dong_porf_porf_todo_memory[2773]=(u8)103;dong_porf_porf_todo_memory[2774]=(u8)104;dong_porf_porf_todo_memory[2775]=(u8)116;dong_porf_porf_todo_memory[2776]=(u8)58;dong_porf_porf_todo_memory[2777]=(u8)49;dong_porf_porf_todo_memory[2778]=(u8)50;dong_porf_porf_todo_memory[2779]=(u8)112;dong_porf_porf_todo_memory[2780]=(u8)120;dong_porf_porf_todo_memory[2781]=(u8)59;dong_porf_porf_todo_memory[2782]=(u8)100;dong_porf_porf_todo_memory[2783]=(u8)105;dong_porf_porf_todo_memory[2784]=(u8)115;dong_porf_porf_todo_memory[2785]=(u8)112;dong_porf_porf_todo_memory[2786]=(u8)108;dong_porf_porf_todo_memory[2787]=(u8)97;dong_porf_porf_todo_memory[2788]=(u8)121;dong_porf_porf_todo_memory[2789]=(u8)58;dong_porf_porf_todo_memory[2790]=(u8)102;dong_porf_porf_todo_memory[2791]=(u8)108;dong_porf_porf_todo_memory[2792]=(u8)101;dong_porf_porf_todo_memory[2793]=(u8)120;dong_porf_porf_todo_memory[2794]=(u8)59;dong_porf_porf_todo_memory[2795]=(u8)97;dong_porf_porf_todo_memory[2796]=(u8)108;dong_porf_porf_todo_memory[2797]=(u8)105;dong_porf_porf_todo_memory[2798]=(u8)103;dong_porf_porf_todo_memory[2799]=(u8)110;dong_porf_porf_todo_memory[2800]=(u8)45;dong_porf_porf_todo_memory[2801]=(u8)105;dong_porf_porf_todo_memory[2802]=(u8)116;dong_porf_porf_todo_memory[2803]=(u8)101;dong_porf_porf_todo_memory[2804]=(u8)109;dong_porf_porf_todo_memory[2805]=(u8)115;dong_porf_porf_todo_memory[2806]=(u8)58;dong_porf_porf_todo_memory[2807]=(u8)99;dong_porf_porf_todo_memory[2808]=(u8)101;dong_porf_porf_todo_memory[2809]=(u8)110;dong_porf_porf_todo_memory[2810]=(u8)116;dong_porf_porf_todo_memory[2811]=(u8)101;dong_porf_porf_todo_memory[2812]=(u8)114;dong_porf_porf_todo_memory[2813]=(u8)59;dong_porf_porf_todo_memory[2814]=(u8)106;dong_porf_porf_todo_memory[2815]=(u8)117;dong_porf_porf_todo_memory[2816]=(u8)115;dong_porf_porf_todo_memory[2817]=(u8)116;dong_porf_porf_todo_memory[2818]=(u8)105;dong_porf_porf_todo_memory[2819]=(u8)102;dong_porf_porf_todo_memory[2820]=(u8)121;dong_porf_porf_todo_memory[2821]=(u8)45;dong_porf_porf_todo_memory[2822]=(u8)99;dong_porf_porf_todo_memory[2823]=(u8)111;dong_porf_porf_todo_memory[2824]=(u8)110;dong_porf_porf_todo_memory[2825]=(u8)116;dong_porf_porf_todo_memory[2826]=(u8)101;dong_porf_porf_todo_memory[2827]=(u8)110;dong_porf_porf_todo_memory[2828]=(u8)116;dong_porf_porf_todo_memory[2829]=(u8)58;dong_porf_porf_todo_memory[2830]=(u8)99;dong_porf_porf_todo_memory[2831]=(u8)101;dong_porf_porf_todo_memory[2832]=(u8)110;dong_porf_porf_todo_memory[2833]=(u8)116;dong_porf_porf_todo_memory[2834]=(u8)101;dong_porf_porf_todo_memory[2835]=(u8)114;dong_porf_porf_todo_memory[2836]=(u8)59;dong_porf_porf_todo_memory[2837]=(u8)99;dong_porf_porf_todo_memory[2838]=(u8)111;dong_porf_porf_todo_memory[2839]=(u8)108;dong_porf_porf_todo_memory[2840]=(u8)111;dong_porf_porf_todo_memory[2841]=(u8)114;dong_porf_porf_todo_memory[2842]=(u8)58;dong_porf_porf_todo_memory[2843]=(u8)35;dong_porf_porf_todo_memory[2844]=(u8)102;dong_porf_porf_todo_memory[2845]=(u8)102;dong_porf_porf_todo_memory[2846]=(u8)102;dong_porf_porf_todo_memory[2847]=(u8)59;dong_porf_porf_todo_memory[2848]=(u8)102;dong_porf_porf_todo_memory[2849]=(u8)111;dong_porf_porf_todo_memory[2850]=(u8)110;dong_porf_porf_todo_memory[2851]=(u8)116;dong_porf_porf_todo_memory[2852]=(u8)45;dong_porf_porf_todo_memory[2853]=(u8)115;dong_porf_porf_todo_memory[2854]=(u8)105;dong_porf_porf_todo_memory[2855]=(u8)122;dong_porf_porf_todo_memory[2856]=(u8)101;dong_porf_porf_todo_memory[2857]=(u8)58;dong_porf_porf_todo_memory[2858]=(u8)49;dong_porf_porf_todo_memory[2859]=(u8)52;dong_porf_porf_todo_memory[2860]=(u8)112;dong_porf_porf_todo_memory[2861]=(u8)120;dong_porf_porf_todo_memory[2862]=(u8)59;dong_porf_porf_todo_memory[2863]=(u8)102;dong_porf_porf_todo_memory[2864]=(u8)111;dong_porf_porf_todo_memory[2865]=(u8)110;dong_porf_porf_todo_memory[2866]=(u8)116;dong_porf_porf_todo_memory[2867]=(u8)45;dong_porf_porf_todo_memory[2868]=(u8)119;dong_porf_porf_todo_memory[2869]=(u8)101;dong_porf_porf_todo_memory[2870]=(u8)105;dong_porf_porf_todo_memory[2871]=(u8)103;dong_porf_porf_todo_memory[2872]=(u8)104;dong_porf_porf_todo_memory[2873]=(u8)116;dong_porf_porf_todo_memory[2874]=(u8)58;dong_porf_porf_todo_memory[2875]=(u8)98;dong_porf_porf_todo_memory[2876]=(u8)111;dong_porf_porf_todo_memory[2877]=(u8)108;dong_porf_porf_todo_memory[2878]=(u8)100;dong_porf_porf_todo_memory[2879]=(u8)59;dong_porf_porf_todo_memory[2880]=(u8)99;dong_porf_porf_todo_memory[2881]=(u8)117;dong_porf_porf_todo_memory[2882]=(u8)114;dong_porf_porf_todo_memory[2883]=(u8)115;dong_porf_porf_todo_memory[2884]=(u8)111;dong_porf_porf_todo_memory[2885]=(u8)114;dong_porf_porf_todo_memory[2886]=(u8)58;dong_porf_porf_todo_memory[2887]=(u8)112;dong_porf_porf_todo_memory[2888]=(u8)111;dong_porf_porf_todo_memory[2889]=(u8)105;dong_porf_porf_todo_memory[2890]=(u8)110;dong_porf_porf_todo_memory[2891]=(u8)116;dong_porf_porf_todo_memory[2892]=(u8)101;dong_porf_porf_todo_memory[2893]=(u8)114;dong_porf_porf_todo_memory[2894]=(u8)59;dong_porf_porf_todo_memory[2895]=(u8)34;dong_porf_porf_todo_memory[2896]=(u8)62;
  dong_porf_porf_todo_memory[2899]=(u8)6;dong_porf_porf_todo_memory[2903]=(u8)60;dong_porf_porf_todo_memory[2904]=(u8)47;dong_porf_porf_todo_memory[2905]=(u8)100;dong_porf_porf_todo_memory[2906]=(u8)105;dong_porf_porf_todo_memory[2907]=(u8)118;dong_porf_porf_todo_memory[2908]=(u8)62;
  dong_porf_porf_todo_memory[2911]=(u8)41;dong_porf_porf_todo_memory[2915]=(u8)60;dong_porf_porf_todo_memory[2916]=(u8)115;dong_porf_porf_todo_memory[2917]=(u8)112;dong_porf_porf_todo_memory[2918]=(u8)97;dong_porf_porf_todo_memory[2919]=(u8)110;dong_porf_porf_todo_memory[2920]=(u8)32;dong_porf_porf_todo_memory[2921]=(u8)115;dong_porf_porf_todo_memory[2922]=(u8)116;dong_porf_porf_todo_memory[2923]=(u8)121;dong_porf_porf_todo_memory[2924]=(u8)108;dong_porf_porf_todo_memory[2925]=(u8)101;dong_porf_porf_todo_memory[2926]=(u8)61;dong_porf_porf_todo_memory[2927]=(u8)34;dong_porf_porf_todo_memory[2928]=(u8)102;dong_porf_porf_todo_memory[2929]=(u8)108;dong_porf_porf_todo_memory[2930]=(u8)101;dong_porf_porf_todo_memory[2931]=(u8)120;dong_porf_porf_todo_memory[2932]=(u8)58;dong_porf_porf_todo_memory[2933]=(u8)49;dong_porf_porf_todo_memory[2934]=(u8)59;dong_porf_porf_todo_memory[2935]=(u8)102;dong_porf_porf_todo_memory[2936]=(u8)111;dong_porf_porf_todo_memory[2937]=(u8)110;dong_porf_porf_todo_memory[2938]=(u8)116;dong_porf_porf_todo_memory[2939]=(u8)45;dong_porf_porf_todo_memory[2940]=(u8)115;dong_porf_porf_todo_memory[2941]=(u8)105;dong_porf_porf_todo_memory[2942]=(u8)122;dong_porf_porf_todo_memory[2943]=(u8)101;dong_porf_porf_todo_memory[2944]=(u8)58;dong_porf_porf_todo_memory[2945]=(u8)49;dong_porf_porf_todo_memory[2946]=(u8)54;dong_porf_porf_todo_memory[2947]=(u8)112;dong_porf_porf_todo_memory[2948]=(u8)120;dong_porf_porf_todo_memory[2949]=(u8)59;dong_porf_porf_todo_memory[2950]=(u8)99;dong_porf_porf_todo_memory[2951]=(u8)111;dong_porf_porf_todo_memory[2952]=(u8)108;dong_porf_porf_todo_memory[2953]=(u8)111;dong_porf_porf_todo_memory[2954]=(u8)114;dong_porf_porf_todo_memory[2955]=(u8)58;
  dong_porf_porf_todo_memory[2958]=(u8)17;dong_porf_porf_todo_memory[2962]=(u8)59;dong_porf_porf_todo_memory[2963]=(u8)116;dong_porf_porf_todo_memory[2964]=(u8)101;dong_porf_porf_todo_memory[2965]=(u8)120;dong_porf_porf_todo_memory[2966]=(u8)116;dong_porf_porf_todo_memory[2967]=(u8)45;dong_porf_porf_todo_memory[2968]=(u8)100;dong_porf_porf_todo_memory[2969]=(u8)101;dong_porf_porf_todo_memory[2970]=(u8)99;dong_porf_porf_todo_memory[2971]=(u8)111;dong_porf_porf_todo_memory[2972]=(u8)114;dong_porf_porf_todo_memory[2973]=(u8)97;dong_porf_porf_todo_memory[2974]=(u8)116;dong_porf_porf_todo_memory[2975]=(u8)105;dong_porf_porf_todo_memory[2976]=(u8)111;dong_porf_porf_todo_memory[2977]=(u8)110;dong_porf_porf_todo_memory[2978]=(u8)58;
  dong_porf_porf_todo_memory[2981]=(u8)3;dong_porf_porf_todo_memory[2985]=(u8)59;dong_porf_porf_todo_memory[2986]=(u8)34;dong_porf_porf_todo_memory[2987]=(u8)62;
  dong_porf_porf_todo_memory[2990]=(u8)7;dong_porf_porf_todo_memory[2994]=(u8)60;dong_porf_porf_todo_memory[2995]=(u8)47;dong_porf_porf_todo_memory[2996]=(u8)115;dong_porf_porf_todo_memory[2997]=(u8)112;dong_porf_porf_todo_memory[2998]=(u8)97;dong_porf_porf_todo_memory[2999]=(u8)110;dong_porf_porf_todo_memory[3000]=(u8)62;
  dong_porf_porf_todo_memory[3003]=(u8)32;dong_porf_porf_todo_memory[3007]=(u8)60;dong_porf_porf_todo_memory[3008]=(u8)98;dong_porf_porf_todo_memory[3009]=(u8)117;dong_porf_porf_todo_memory[3010]=(u8)116;dong_porf_porf_todo_memory[3011]=(u8)116;dong_porf_porf_todo_memory[3012]=(u8)111;dong_porf_porf_todo_memory[3013]=(u8)110;dong_porf_porf_todo_memory[3014]=(u8)32;dong_porf_porf_todo_memory[3015]=(u8)100;dong_porf_porf_todo_memory[3016]=(u8)97;dong_porf_porf_todo_memory[3017]=(u8)116;dong_porf_porf_todo_memory[3018]=(u8)97;dong_porf_porf_todo_memory[3019]=(u8)45;dong_porf_porf_todo_memory[3020]=(u8)116;dong_porf_porf_todo_memory[3021]=(u8)111;dong_porf_porf_todo_memory[3022]=(u8)100;dong_porf_porf_todo_memory[3023]=(u8)111;dong_porf_porf_todo_memory[3024]=(u8)45;dong_porf_porf_todo_memory[3025]=(u8)100;dong_porf_porf_todo_memory[3026]=(u8)101;dong_porf_porf_todo_memory[3027]=(u8)108;dong_porf_porf_todo_memory[3028]=(u8)101;dong_porf_porf_todo_memory[3029]=(u8)116;dong_porf_porf_todo_memory[3030]=(u8)101;dong_porf_porf_todo_memory[3031]=(u8)45;dong_porf_porf_todo_memory[3032]=(u8)105;dong_porf_porf_todo_memory[3033]=(u8)110;dong_porf_porf_todo_memory[3034]=(u8)100;dong_porf_porf_todo_memory[3035]=(u8)101;dong_porf_porf_todo_memory[3036]=(u8)120;dong_porf_porf_todo_memory[3037]=(u8)61;dong_porf_porf_todo_memory[3038]=(u8)34;
  dong_porf_porf_todo_memory[3041]=(u8)116;dong_porf_porf_todo_memory[3045]=(u8)34;dong_porf_porf_todo_memory[3046]=(u8)32;dong_porf_porf_todo_memory[3047]=(u8)115;dong_porf_porf_todo_memory[3048]=(u8)116;dong_porf_porf_todo_memory[3049]=(u8)121;dong_porf_porf_todo_memory[3050]=(u8)108;dong_porf_porf_todo_memory[3051]=(u8)101;dong_porf_porf_todo_memory[3052]=(u8)61;dong_porf_porf_todo_memory[3053]=(u8)34;dong_porf_porf_todo_memory[3054]=(u8)98;dong_porf_porf_todo_memory[3055]=(u8)97;dong_porf_porf_todo_memory[3056]=(u8)99;dong_porf_porf_todo_memory[3057]=(u8)107;dong_porf_porf_todo_memory[3058]=(u8)103;dong_porf_porf_todo_memory[3059]=(u8)114;dong_porf_porf_todo_memory[3060]=(u8)111;dong_porf_porf_todo_memory[3061]=(u8)117;dong_porf_porf_todo_memory[3062]=(u8)110;dong_porf_porf_todo_memory[3063]=(u8)100;dong_porf_porf_todo_memory[3064]=(u8)58;dong_porf_porf_todo_memory[3065]=(u8)116;dong_porf_porf_todo_memory[3066]=(u8)114;dong_porf_porf_todo_memory[3067]=(u8)97;dong_porf_porf_todo_memory[3068]=(u8)110;dong_porf_porf_todo_memory[3069]=(u8)115;dong_porf_porf_todo_memory[3070]=(u8)112;dong_porf_porf_todo_memory[3071]=(u8)97;dong_porf_porf_todo_memory[3072]=(u8)114;dong_porf_porf_todo_memory[3073]=(u8)101;dong_porf_porf_todo_memory[3074]=(u8)110;dong_porf_porf_todo_memory[3075]=(u8)116;dong_porf_porf_todo_memory[3076]=(u8)59;dong_porf_porf_todo_memory[3077]=(u8)98;dong_porf_porf_todo_memory[3078]=(u8)111;dong_porf_porf_todo_memory[3079]=(u8)114;dong_porf_porf_todo_memory[3080]=(u8)100;dong_porf_porf_todo_memory[3081]=(u8)101;dong_porf_porf_todo_memory[3082]=(u8)114;dong_porf_porf_todo_memory[3083]=(u8)58;dong_porf_porf_todo_memory[3084]=(u8)110;dong_porf_porf_todo_memory[3085]=(u8)111;dong_porf_porf_todo_memory[3086]=(u8)110;dong_porf_porf_todo_memory[3087]=(u8)101;dong_porf_porf_todo_memory[3088]=(u8)59;dong_porf_porf_todo_memory[3089]=(u8)99;dong_porf_porf_todo_memory[3090]=(u8)111;dong_porf_porf_todo_memory[3091]=(u8)108;dong_porf_porf_todo_memory[3092]=(u8)111;dong_porf_porf_todo_memory[3093]=(u8)114;dong_porf_porf_todo_memory[3094]=(u8)58;dong_porf_porf_todo_memory[3095]=(u8)35;dong_porf_porf_todo_memory[3096]=(u8)101;dong_porf_porf_todo_memory[3097]=(u8)55;dong_porf_porf_todo_memory[3098]=(u8)52;dong_porf_porf_todo_memory[3099]=(u8)99;dong_porf_porf_todo_memory[3100]=(u8)51;dong_porf_porf_todo_memory[3101]=(u8)99;dong_porf_porf_todo_memory[3102]=(u8)59;dong_porf_porf_todo_memory[3103]=(u8)102;dong_porf_porf_todo_memory[3104]=(u8)111;dong_porf_porf_todo_memory[3105]=(u8)110;dong_porf_porf_todo_memory[3106]=(u8)116;dong_porf_porf_todo_memory[3107]=(u8)45;dong_porf_porf_todo_memory[3108]=(u8)115;dong_porf_porf_todo_memory[3109]=(u8)105;dong_porf_porf_todo_memory[3110]=(u8)122;dong_porf_porf_todo_memory[3111]=(u8)101;dong_porf_porf_todo_memory[3112]=(u8)58;dong_porf_porf_todo_memory[3113]=(u8)49;dong_porf_porf_todo_memory[3114]=(u8)56;dong_porf_porf_todo_memory[3115]=(u8)112;dong_porf_porf_todo_memory[3116]=(u8)120;dong_porf_porf_todo_memory[3117]=(u8)59;dong_porf_porf_todo_memory[3118]=(u8)112;dong_porf_porf_todo_memory[3119]=(u8)97;dong_porf_porf_todo_memory[3120]=(u8)100;dong_porf_porf_todo_memory[3121]=(u8)100;dong_porf_porf_todo_memory[3122]=(u8)105;dong_porf_porf_todo_memory[3123]=(u8)110;dong_porf_porf_todo_memory[3124]=(u8)103;dong_porf_porf_todo_memory[3125]=(u8)58;dong_porf_porf_todo_memory[3126]=(u8)52;dong_porf_porf_todo_memory[3127]=(u8)112;dong_porf_porf_todo_memory[3128]=(u8)120;dong_porf_porf_todo_memory[3129]=(u8)32;dong_porf_porf_todo_memory[3130]=(u8)56;dong_porf_porf_todo_memory[3131]=(u8)112;dong_porf_porf_todo_memory[3132]=(u8)120;dong_porf_porf_todo_memory[3133]=(u8)59;dong_porf_porf_todo_memory[3134]=(u8)99;dong_porf_porf_todo_memory[3135]=(u8)117;dong_porf_porf_todo_memory[3136]=(u8)114;dong_porf_porf_todo_memory[3137]=(u8)115;dong_porf_porf_todo_memory[3138]=(u8)111;dong_porf_porf_todo_memory[3139]=(u8)114;dong_porf_porf_todo_memory[3140]=(u8)58;dong_porf_porf_todo_memory[3141]=(u8)112;dong_porf_porf_todo_memory[3142]=(u8)111;dong_porf_porf_todo_memory[3143]=(u8)105;dong_porf_porf_todo_memory[3144]=(u8)110;dong_porf_porf_todo_memory[3145]=(u8)116;dong_porf_porf_todo_memory[3146]=(u8)101;dong_porf_porf_todo_memory[3147]=(u8)114;dong_porf_porf_todo_memory[3148]=(u8)59;dong_porf_porf_todo_memory[3149]=(u8)34;dong_porf_porf_todo_memory[3150]=(u8)62;dong_porf_porf_todo_memory[3151]=(u8)88;dong_porf_porf_todo_memory[3152]=(u8)60;dong_porf_porf_todo_memory[3153]=(u8)47;dong_porf_porf_todo_memory[3154]=(u8)98;dong_porf_porf_todo_memory[3155]=(u8)117;dong_porf_porf_todo_memory[3156]=(u8)116;dong_porf_porf_todo_memory[3157]=(u8)116;dong_porf_porf_todo_memory[3158]=(u8)111;dong_porf_porf_todo_memory[3159]=(u8)110;dong_porf_porf_todo_memory[3160]=(u8)62;
  dong_porf_porf_todo_memory[3163]=(u8)5;dong_porf_porf_todo_memory[3167]=(u8)69;dong_porf_porf_todo_memory[3168]=(u8)110;dong_porf_porf_todo_memory[3169]=(u8)116;dong_porf_porf_todo_memory[3170]=(u8)101;dong_porf_porf_todo_memory[3171]=(u8)114;
  dong_porf_porf_todo_memory[3174]=(u8)22;dong_porf_porf_todo_memory[3178]=(u8)100;dong_porf_porf_todo_memory[3179]=(u8)97;dong_porf_porf_todo_memory[3180]=(u8)116;dong_porf_porf_todo_memory[3181]=(u8)97;dong_porf_porf_todo_memory[3182]=(u8)45;dong_porf_porf_todo_memory[3183]=(u8)116;dong_porf_porf_todo_memory[3184]=(u8)111;dong_porf_porf_todo_memory[3185]=(u8)100;dong_porf_porf_todo_memory[3186]=(u8)111;dong_porf_porf_todo_memory[3187]=(u8)45;dong_porf_porf_todo_memory[3188]=(u8)116;dong_porf_porf_todo_memory[3189]=(u8)111;dong_porf_porf_todo_memory[3190]=(u8)103;dong_porf_porf_todo_memory[3191]=(u8)103;dong_porf_porf_todo_memory[3192]=(u8)108;dong_porf_porf_todo_memory[3193]=(u8)101;dong_porf_porf_todo_memory[3194]=(u8)45;dong_porf_porf_todo_memory[3195]=(u8)105;dong_porf_porf_todo_memory[3196]=(u8)110;dong_porf_porf_todo_memory[3197]=(u8)100;dong_porf_porf_todo_memory[3198]=(u8)101;dong_porf_porf_todo_memory[3199]=(u8)120;
  dong_porf_porf_todo_memory[3202]=(u8)1;dong_porf_porf_todo_memory[3206]=(u8)50;
  dong_porf_porf_todo_memory[3209]=(u8)1;dong_porf_porf_todo_memory[3213]=(u8)51;
  dong_porf_porf_todo_memory[3216]=(u8)1;dong_porf_porf_todo_memory[3220]=(u8)52;
  dong_porf_porf_todo_memory[3223]=(u8)1;dong_porf_porf_todo_memory[3227]=(u8)53;
  dong_porf_porf_todo_memory[3230]=(u8)1;dong_porf_porf_todo_memory[3234]=(u8)54;
  dong_porf_porf_todo_memory[3237]=(u8)1;dong_porf_porf_todo_memory[3241]=(u8)55;
  dong_porf_porf_todo_memory[3244]=(u8)1;dong_porf_porf_todo_memory[3248]=(u8)56;
  dong_porf_porf_todo_memory[3251]=(u8)1;dong_porf_porf_todo_memory[3255]=(u8)57;
  dong_porf_porf_todo_memory[3258]=(u8)2;dong_porf_porf_todo_memory[3262]=(u8)49;dong_porf_porf_todo_memory[3263]=(u8)48;
  dong_porf_porf_todo_memory[3266]=(u8)2;dong_porf_porf_todo_memory[3270]=(u8)49;dong_porf_porf_todo_memory[3271]=(u8)49;
  dong_porf_porf_todo_memory[3274]=(u8)2;dong_porf_porf_todo_memory[3278]=(u8)49;dong_porf_porf_todo_memory[3279]=(u8)50;
  dong_porf_porf_todo_memory[3282]=(u8)2;dong_porf_porf_todo_memory[3286]=(u8)49;dong_porf_porf_todo_memory[3287]=(u8)51;
  dong_porf_porf_todo_memory[3290]=(u8)2;dong_porf_porf_todo_memory[3294]=(u8)49;dong_porf_porf_todo_memory[3295]=(u8)52;
  dong_porf_porf_todo_memory[3298]=(u8)2;dong_porf_porf_todo_memory[3302]=(u8)49;dong_porf_porf_todo_memory[3303]=(u8)53;
  dong_porf_porf_todo_memory[3306]=(u8)24;dong_porf_porf_todo_memory[3310]=(u8)91;dong_porf_porf_todo_memory[3311]=(u8)100;dong_porf_porf_todo_memory[3312]=(u8)97;dong_porf_porf_todo_memory[3313]=(u8)116;dong_porf_porf_todo_memory[3314]=(u8)97;dong_porf_porf_todo_memory[3315]=(u8)45;dong_porf_porf_todo_memory[3316]=(u8)116;dong_porf_porf_todo_memory[3317]=(u8)111;dong_porf_porf_todo_memory[3318]=(u8)100;dong_porf_porf_todo_memory[3319]=(u8)111;dong_porf_porf_todo_memory[3320]=(u8)45;dong_porf_porf_todo_memory[3321]=(u8)116;dong_porf_porf_todo_memory[3322]=(u8)111;dong_porf_porf_todo_memory[3323]=(u8)103;dong_porf_porf_todo_memory[3324]=(u8)103;dong_porf_porf_todo_memory[3325]=(u8)108;dong_porf_porf_todo_memory[3326]=(u8)101;dong_porf_porf_todo_memory[3327]=(u8)45;dong_porf_porf_todo_memory[3328]=(u8)105;dong_porf_porf_todo_memory[3329]=(u8)110;dong_porf_porf_todo_memory[3330]=(u8)100;dong_porf_porf_todo_memory[3331]=(u8)101;dong_porf_porf_todo_memory[3332]=(u8)120;dong_porf_porf_todo_memory[3333]=(u8)93;
  dong_porf_porf_todo_memory[3336]=(u8)22;dong_porf_porf_todo_memory[3340]=(u8)100;dong_porf_porf_todo_memory[3341]=(u8)97;dong_porf_porf_todo_memory[3342]=(u8)116;dong_porf_porf_todo_memory[3343]=(u8)97;dong_porf_porf_todo_memory[3344]=(u8)45;dong_porf_porf_todo_memory[3345]=(u8)116;dong_porf_porf_todo_memory[3346]=(u8)111;dong_porf_porf_todo_memory[3347]=(u8)100;dong_porf_porf_todo_memory[3348]=(u8)111;dong_porf_porf_todo_memory[3349]=(u8)45;dong_porf_porf_todo_memory[3350]=(u8)100;dong_porf_porf_todo_memory[3351]=(u8)101;dong_porf_porf_todo_memory[3352]=(u8)108;dong_porf_porf_todo_memory[3353]=(u8)101;dong_porf_porf_todo_memory[3354]=(u8)116;dong_porf_porf_todo_memory[3355]=(u8)101;dong_porf_porf_todo_memory[3356]=(u8)45;dong_porf_porf_todo_memory[3357]=(u8)105;dong_porf_porf_todo_memory[3358]=(u8)110;dong_porf_porf_todo_memory[3359]=(u8)100;dong_porf_porf_todo_memory[3360]=(u8)101;dong_porf_porf_todo_memory[3361]=(u8)120;
  dong_porf_porf_todo_memory[3364]=(u8)24;dong_porf_porf_todo_memory[3368]=(u8)91;dong_porf_porf_todo_memory[3369]=(u8)100;dong_porf_porf_todo_memory[3370]=(u8)97;dong_porf_porf_todo_memory[3371]=(u8)116;dong_porf_porf_todo_memory[3372]=(u8)97;dong_porf_porf_todo_memory[3373]=(u8)45;dong_porf_porf_todo_memory[3374]=(u8)116;dong_porf_porf_todo_memory[3375]=(u8)111;dong_porf_porf_todo_memory[3376]=(u8)100;dong_porf_porf_todo_memory[3377]=(u8)111;dong_porf_porf_todo_memory[3378]=(u8)45;dong_porf_porf_todo_memory[3379]=(u8)100;dong_porf_porf_todo_memory[3380]=(u8)101;dong_porf_porf_todo_memory[3381]=(u8)108;dong_porf_porf_todo_memory[3382]=(u8)101;dong_porf_porf_todo_memory[3383]=(u8)116;dong_porf_porf_todo_memory[3384]=(u8)101;dong_porf_porf_todo_memory[3385]=(u8)45;dong_porf_porf_todo_memory[3386]=(u8)105;dong_porf_porf_todo_memory[3387]=(u8)110;dong_porf_porf_todo_memory[3388]=(u8)100;dong_porf_porf_todo_memory[3389]=(u8)101;dong_porf_porf_todo_memory[3390]=(u8)120;dong_porf_porf_todo_memory[3391]=(u8)93;
  dong_porf_porf_todo_memory[3394]=(u8)10;dong_porf_porf_todo_memory[3398]=(u8)116;dong_porf_porf_todo_memory[3399]=(u8)111;dong_porf_porf_todo_memory[3400]=(u8)100;dong_porf_porf_todo_memory[3401]=(u8)111;dong_porf_porf_todo_memory[3402]=(u8)45;dong_porf_porf_todo_memory[3403]=(u8)105;dong_porf_porf_todo_memory[3404]=(u8)110;dong_porf_porf_todo_memory[3405]=(u8)112;dong_porf_porf_todo_memory[3406]=(u8)117;dong_porf_porf_todo_memory[3407]=(u8)116;
  dong_porf_porf_todo_memory[3410]=(u8)7;dong_porf_porf_todo_memory[3414]=(u8)98;dong_porf_porf_todo_memory[3415]=(u8)116;dong_porf_porf_todo_memory[3416]=(u8)110;dong_porf_porf_todo_memory[3417]=(u8)45;dong_porf_porf_todo_memory[3418]=(u8)97;dong_porf_porf_todo_memory[3419]=(u8)100;dong_porf_porf_todo_memory[3420]=(u8)100;
  dong_porf_porf_todo_memory[3423]=(u8)10;dong_porf_porf_todo_memory[3427]=(u8)102;dong_porf_porf_todo_memory[3428]=(u8)105;dong_porf_porf_todo_memory[3429]=(u8)108;dong_porf_porf_todo_memory[3430]=(u8)116;dong_porf_porf_todo_memory[3431]=(u8)101;dong_porf_porf_todo_memory[3432]=(u8)114;dong_porf_porf_todo_memory[3433]=(u8)45;dong_porf_porf_todo_memory[3434]=(u8)97;dong_porf_porf_todo_memory[3435]=(u8)108;dong_porf_porf_todo_memory[3436]=(u8)108;
  dong_porf_porf_todo_memory[3439]=(u8)13;dong_porf_porf_todo_memory[3443]=(u8)102;dong_porf_porf_todo_memory[3444]=(u8)105;dong_porf_porf_todo_memory[3445]=(u8)108;dong_porf_porf_todo_memory[3446]=(u8)116;dong_porf_porf_todo_memory[3447]=(u8)101;dong_porf_porf_todo_memory[3448]=(u8)114;dong_porf_porf_todo_memory[3449]=(u8)45;dong_porf_porf_todo_memory[3450]=(u8)97;dong_porf_porf_todo_memory[3451]=(u8)99;dong_porf_porf_todo_memory[3452]=(u8)116;dong_porf_porf_todo_memory[3453]=(u8)105;dong_porf_porf_todo_memory[3454]=(u8)118;dong_porf_porf_todo_memory[3455]=(u8)101;
  dong_porf_porf_todo_memory[3458]=(u8)11;dong_porf_porf_todo_memory[3462]=(u8)102;dong_porf_porf_todo_memory[3463]=(u8)105;dong_porf_porf_todo_memory[3464]=(u8)108;dong_porf_porf_todo_memory[3465]=(u8)116;dong_porf_porf_todo_memory[3466]=(u8)101;dong_porf_porf_todo_memory[3467]=(u8)114;dong_porf_porf_todo_memory[3468]=(u8)45;dong_porf_porf_todo_memory[3469]=(u8)100;dong_porf_porf_todo_memory[3470]=(u8)111;dong_porf_porf_todo_memory[3471]=(u8)110;dong_porf_porf_todo_memory[3472]=(u8)101;
  dong_porf_porf_todo_memory[3475]=(u8)9;dong_porf_porf_todo_memory[3479]=(u8)116;dong_porf_porf_todo_memory[3480]=(u8)111;dong_porf_porf_todo_memory[3481]=(u8)100;dong_porf_porf_todo_memory[3482]=(u8)111;dong_porf_porf_todo_memory[3483]=(u8)45;dong_porf_porf_todo_memory[3484]=(u8)108;dong_porf_porf_todo_memory[3485]=(u8)105;dong_porf_porf_todo_memory[3486]=(u8)115;dong_porf_porf_todo_memory[3487]=(u8)116;
  dong_porf_porf_todo_memory[3490]=(u8)10;dong_porf_porf_todo_memory[3494]=(u8)99;dong_porf_porf_todo_memory[3495]=(u8)108;dong_porf_porf_todo_memory[3496]=(u8)101;dong_porf_porf_todo_memory[3497]=(u8)97;dong_porf_porf_todo_memory[3498]=(u8)114;dong_porf_porf_todo_memory[3499]=(u8)45;dong_porf_porf_todo_memory[3500]=(u8)119;dong_porf_porf_todo_memory[3501]=(u8)114;dong_porf_porf_todo_memory[3502]=(u8)97;dong_porf_porf_todo_memory[3503]=(u8)112;
  dong_porf_porf_todo_memory[3506]=(u8)9;dong_porf_porf_todo_memory[3510]=(u8)98;dong_porf_porf_todo_memory[3511]=(u8)116;dong_porf_porf_todo_memory[3512]=(u8)110;dong_porf_porf_todo_memory[3513]=(u8)45;dong_porf_porf_todo_memory[3514]=(u8)99;dong_porf_porf_todo_memory[3515]=(u8)108;dong_porf_porf_todo_memory[3516]=(u8)101;dong_porf_porf_todo_memory[3517]=(u8)97;dong_porf_porf_todo_memory[3518]=(u8)114;
  dong_porf_porf_todo_memory[3521]=(u8)5;dong_porf_porf_todo_memory[3525]=(u8)99;dong_porf_porf_todo_memory[3526]=(u8)108;dong_porf_porf_todo_memory[3527]=(u8)105;dong_porf_porf_todo_memory[3528]=(u8)99;dong_porf_porf_todo_memory[3529]=(u8)107;
  dong_porf_porf_todo_memory[3532]=(u8)5;dong_porf_porf_todo_memory[3536]=(u8)111;dong_porf_porf_todo_memory[3537]=(u8)110;dong_porf_porf_todo_memory[3538]=(u8)65;dong_porf_porf_todo_memory[3539]=(u8)100;dong_porf_porf_todo_memory[3540]=(u8)100;
  dong_porf_porf_todo_memory[3543]=(u8)5;dong_porf_porf_todo_memory[3547]=(u8)105;dong_porf_porf_todo_memory[3548]=(u8)110;dong_porf_porf_todo_memory[3549]=(u8)112;dong_porf_porf_todo_memory[3550]=(u8)117;dong_porf_porf_todo_memory[3551]=(u8)116;
  dong_porf_porf_todo_memory[3554]=(u8)13;dong_porf_porf_todo_memory[3558]=(u8)111;dong_porf_porf_todo_memory[3559]=(u8)110;dong_porf_porf_todo_memory[3560]=(u8)73;dong_porf_porf_todo_memory[3561]=(u8)110;dong_porf_porf_todo_memory[3562]=(u8)112;dong_porf_porf_todo_memory[3563]=(u8)117;dong_porf_porf_todo_memory[3564]=(u8)116;dong_porf_porf_todo_memory[3565]=(u8)67;dong_porf_porf_todo_memory[3566]=(u8)104;dong_porf_porf_todo_memory[3567]=(u8)97;dong_porf_porf_todo_memory[3568]=(u8)110;dong_porf_porf_todo_memory[3569]=(u8)103;dong_porf_porf_todo_memory[3570]=(u8)101;
  dong_porf_porf_todo_memory[3573]=(u8)7;dong_porf_porf_todo_memory[3577]=(u8)107;dong_porf_porf_todo_memory[3578]=(u8)101;dong_porf_porf_todo_memory[3579]=(u8)121;dong_porf_porf_todo_memory[3580]=(u8)100;dong_porf_porf_todo_memory[3581]=(u8)111;dong_porf_porf_todo_memory[3582]=(u8)119;dong_porf_porf_todo_memory[3583]=(u8)110;
  dong_porf_porf_todo_memory[3586]=(u8)9;dong_porf_porf_todo_memory[3590]=(u8)111;dong_porf_porf_todo_memory[3591]=(u8)110;dong_porf_porf_todo_memory[3592]=(u8)75;dong_porf_porf_todo_memory[3593]=(u8)101;dong_porf_porf_todo_memory[3594]=(u8)121;dong_porf_porf_todo_memory[3595]=(u8)68;dong_porf_porf_todo_memory[3596]=(u8)111;dong_porf_porf_todo_memory[3597]=(u8)119;dong_porf_porf_todo_memory[3598]=(u8)110;
  dong_porf_porf_todo_memory[3601]=(u8)11;dong_porf_porf_todo_memory[3605]=(u8)111;dong_porf_porf_todo_memory[3606]=(u8)110;dong_porf_porf_todo_memory[3607]=(u8)70;dong_porf_porf_todo_memory[3608]=(u8)105;dong_porf_porf_todo_memory[3609]=(u8)108;dong_porf_porf_todo_memory[3610]=(u8)116;dong_porf_porf_todo_memory[3611]=(u8)101;dong_porf_porf_todo_memory[3612]=(u8)114;dong_porf_porf_todo_memory[3613]=(u8)65;dong_porf_porf_todo_memory[3614]=(u8)108;dong_porf_porf_todo_memory[3615]=(u8)108;
  dong_porf_porf_todo_memory[3618]=(u8)14;dong_porf_porf_todo_memory[3622]=(u8)111;dong_porf_porf_todo_memory[3623]=(u8)110;dong_porf_porf_todo_memory[3624]=(u8)70;dong_porf_porf_todo_memory[3625]=(u8)105;dong_porf_porf_todo_memory[3626]=(u8)108;dong_porf_porf_todo_memory[3627]=(u8)116;dong_porf_porf_todo_memory[3628]=(u8)101;dong_porf_porf_todo_memory[3629]=(u8)114;dong_porf_porf_todo_memory[3630]=(u8)65;dong_porf_porf_todo_memory[3631]=(u8)99;dong_porf_porf_todo_memory[3632]=(u8)116;dong_porf_porf_todo_memory[3633]=(u8)105;dong_porf_porf_todo_memory[3634]=(u8)118;dong_porf_porf_todo_memory[3635]=(u8)101;
  dong_porf_porf_todo_memory[3638]=(u8)12;dong_porf_porf_todo_memory[3642]=(u8)111;dong_porf_porf_todo_memory[3643]=(u8)110;dong_porf_porf_todo_memory[3644]=(u8)70;dong_porf_porf_todo_memory[3645]=(u8)105;dong_porf_porf_todo_memory[3646]=(u8)108;dong_porf_porf_todo_memory[3647]=(u8)116;dong_porf_porf_todo_memory[3648]=(u8)101;dong_porf_porf_todo_memory[3649]=(u8)114;dong_porf_porf_todo_memory[3650]=(u8)68;dong_porf_porf_todo_memory[3651]=(u8)111;dong_porf_porf_todo_memory[3652]=(u8)110;dong_porf_porf_todo_memory[3653]=(u8)101;
  dong_porf_porf_todo_memory[3656]=(u8)11;dong_porf_porf_todo_memory[3660]=(u8)111;dong_porf_porf_todo_memory[3661]=(u8)110;dong_porf_porf_todo_memory[3662]=(u8)67;dong_porf_porf_todo_memory[3663]=(u8)108;dong_porf_porf_todo_memory[3664]=(u8)101;dong_porf_porf_todo_memory[3665]=(u8)97;dong_porf_porf_todo_memory[3666]=(u8)114;dong_porf_porf_todo_memory[3667]=(u8)68;dong_porf_porf_todo_memory[3668]=(u8)111;dong_porf_porf_todo_memory[3669]=(u8)110;dong_porf_porf_todo_memory[3670]=(u8)101;
  dong_porf_porf_todo_memory[3673]=(u8)11;dong_porf_porf_todo_memory[3677]=(u8)111;dong_porf_porf_todo_memory[3678]=(u8)110;dong_porf_porf_todo_memory[3679]=(u8)76;dong_porf_porf_todo_memory[3680]=(u8)105;dong_porf_porf_todo_memory[3681]=(u8)115;dong_porf_porf_todo_memory[3682]=(u8)116;dong_porf_porf_todo_memory[3683]=(u8)67;dong_porf_porf_todo_memory[3684]=(u8)108;dong_porf_porf_todo_memory[3685]=(u8)105;dong_porf_porf_todo_memory[3686]=(u8)99;dong_porf_porf_todo_memory[3687]=(u8)107;
  dong_porf_porf_todo_memory[3690]=(u8)16;dong_porf_porf_todo_memory[3694]=(u8)112;dong_porf_porf_todo_memory[3695]=(u8)111;dong_porf_porf_todo_memory[3696]=(u8)114;dong_porf_porf_todo_memory[3697]=(u8)102;dong_porf_porf_todo_memory[3698]=(u8)95;dong_porf_porf_todo_memory[3699]=(u8)116;dong_porf_porf_todo_memory[3700]=(u8)111;dong_porf_porf_todo_memory[3701]=(u8)100;dong_porf_porf_todo_memory[3702]=(u8)111;dong_porf_porf_todo_memory[3703]=(u8)32;dong_porf_porf_todo_memory[3704]=(u8)108;dong_porf_porf_todo_memory[3705]=(u8)111;dong_porf_porf_todo_memory[3706]=(u8)97;dong_porf_porf_todo_memory[3707]=(u8)100;dong_porf_porf_todo_memory[3708]=(u8)101;dong_porf_porf_todo_memory[3709]=(u8)100;
  dong_porf_porf_todo_memory[16498]=(u8)1;dong_porf_porf_todo_memory[16501]=(u8)14;dong_porf_porf_todo_memory[16505]=(u8)104;dong_porf_porf_todo_memory[16506]=(u8)97;dong_porf_porf_todo_memory[16507]=(u8)115;dong_porf_porf_todo_memory[16508]=(u8)79;dong_porf_porf_todo_memory[16509]=(u8)119;dong_porf_porf_todo_memory[16510]=(u8)110;dong_porf_porf_todo_memory[16511]=(u8)80;dong_porf_porf_todo_memory[16512]=(u8)114;dong_porf_porf_todo_memory[16513]=(u8)111;dong_porf_porf_todo_memory[16514]=(u8)112;dong_porf_porf_todo_memory[16515]=(u8)101;dong_porf_porf_todo_memory[16516]=(u8)114;dong_porf_porf_todo_memory[16517]=(u8)116;dong_porf_porf_todo_memory[16518]=(u8)121;dong_porf_porf_todo_memory[16555]=(u8)1;dong_porf_porf_todo_memory[16558]=(u8)20;dong_porf_porf_todo_memory[16562]=(u8)112;dong_porf_porf_todo_memory[16563]=(u8)114;dong_porf_porf_todo_memory[16564]=(u8)111;dong_porf_porf_todo_memory[16565]=(u8)112;dong_porf_porf_todo_memory[16566]=(u8)101;dong_porf_porf_todo_memory[16567]=(u8)114;dong_porf_porf_todo_memory[16568]=(u8)116;dong_porf_porf_todo_memory[16569]=(u8)121;dong_porf_porf_todo_memory[16570]=(u8)73;dong_porf_porf_todo_memory[16571]=(u8)115;dong_porf_porf_todo_memory[16572]=(u8)69;dong_porf_porf_todo_memory[16573]=(u8)110;dong_porf_porf_todo_memory[16574]=(u8)117;dong_porf_porf_todo_memory[16575]=(u8)109;dong_porf_porf_todo_memory[16576]=(u8)101;dong_porf_porf_todo_memory[16577]=(u8)114;dong_porf_porf_todo_memory[16578]=(u8)97;dong_porf_porf_todo_memory[16579]=(u8)98;dong_porf_porf_todo_memory[16580]=(u8)108;dong_porf_porf_todo_memory[16581]=(u8)101;dong_porf_porf_todo_memory[16612]=(u8)1;dong_porf_porf_todo_memory[16615]=(u8)13;dong_porf_porf_todo_memory[16619]=(u8)105;dong_porf_porf_todo_memory[16620]=(u8)115;dong_porf_porf_todo_memory[16621]=(u8)80;dong_porf_porf_todo_memory[16622]=(u8)114;dong_porf_porf_todo_memory[16623]=(u8)111;dong_porf_porf_todo_memory[16624]=(u8)116;dong_porf_porf_todo_memory[16625]=(u8)111;dong_porf_porf_todo_memory[16626]=(u8)116;dong_porf_porf_todo_memory[16627]=(u8)121;dong_porf_porf_todo_memory[16628]=(u8)112;dong_porf_porf_todo_memory[16629]=(u8)101;dong_porf_porf_todo_memory[16630]=(u8)79;dong_porf_porf_todo_memory[16631]=(u8)102;dong_porf_porf_todo_memory[16672]=(u8)8;dong_porf_porf_todo_memory[16676]=(u8)116;dong_porf_porf_todo_memory[16677]=(u8)111;dong_porf_porf_todo_memory[16678]=(u8)83;dong_porf_porf_todo_memory[16679]=(u8)116;dong_porf_porf_todo_memory[16680]=(u8)114;dong_porf_porf_todo_memory[16681]=(u8)105;dong_porf_porf_todo_memory[16682]=(u8)110;dong_porf_porf_todo_memory[16683]=(u8)103;dong_porf_porf_todo_memory[16729]=(u8)14;dong_porf_porf_todo_memory[16733]=(u8)116;dong_porf_porf_todo_memory[16734]=(u8)111;dong_porf_porf_todo_memory[16735]=(u8)76;dong_porf_porf_todo_memory[16736]=(u8)111;dong_porf_porf_todo_memory[16737]=(u8)99;dong_porf_porf_todo_memory[16738]=(u8)97;dong_porf_porf_todo_memory[16739]=(u8)108;dong_porf_porf_todo_memory[16740]=(u8)101;dong_porf_porf_todo_memory[16741]=(u8)83;dong_porf_porf_todo_memory[16742]=(u8)116;dong_porf_porf_todo_memory[16743]=(u8)114;dong_porf_porf_todo_memory[16744]=(u8)105;dong_porf_porf_todo_memory[16745]=(u8)110;dong_porf_porf_todo_memory[16746]=(u8)103;dong_porf_porf_todo_memory[16786]=(u8)7;dong_porf_porf_todo_memory[16790]=(u8)118;dong_porf_porf_todo_memory[16791]=(u8)97;dong_porf_porf_todo_memory[16792]=(u8)108;dong_porf_porf_todo_memory[16793]=(u8)117;dong_porf_porf_todo_memory[16794]=(u8)101;dong_porf_porf_todo_memory[16795]=(u8)79;dong_porf_porf_todo_memory[16796]=(u8)102;dong_porf_porf_todo_memory[16840]=(u8)1;dong_porf_porf_todo_memory[16842]=(u8)2;dong_porf_porf_todo_memory[16843]=(u8)6;dong_porf_porf_todo_memory[16847]=(u8)79;dong_porf_porf_todo_memory[16848]=(u8)98;dong_porf_porf_todo_memory[16849]=(u8)106;dong_porf_porf_todo_memory[16850]=(u8)101;dong_porf_porf_todo_memory[16851]=(u8)99;dong_porf_porf_todo_memory[16852]=(u8)116;
}

static struct ReturnValue dong_porf_porf_todo_utf8AppendCodePoint(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 out, i32 outjjtype, f64 cp, i32 cpjjtype);
static struct ReturnValue dong_porf_porf_todo_toUtf8(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype);
static struct ReturnValue dong_porf_porf_todo_pullHostString(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_getElementById(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 id, i32 idjjtype);
static struct ReturnValue dong_porf_porf_todo_setTextContent(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 text, i32 textjjtype);
static struct ReturnValue dong_porf_porf_todo_addEventListener(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 type, i32 typejjtype, f64 handlerName, i32 handlerNamejjtype);
static struct ReturnValue dong_porf_porf_todo_dongLog(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 msg, i32 msgjjtype);
static struct ReturnValue dong_porf_porf_todo_getValue(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype);
static struct ReturnValue dong_porf_porf_todo_setValue(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 v, i32 vjjtype);
static struct ReturnValue dong_porf_porf_todo_getAttribute(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 name, i32 namejjtype);
static struct ReturnValue dong_porf_porf_todo_setAttribute(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 name, i32 namejjtype, f64 value, i32 valuejjtype);
static struct ReturnValue dong_porf_porf_todo_removeAttribute(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 name, i32 namejjtype);
static struct ReturnValue dong_porf_porf_todo_setInnerHTML(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 html, i32 htmljjtype);
static struct ReturnValue dong_porf_porf_todo_setStyle(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 prop, i32 propjjtype, f64 value, i32 valuejjtype);
static struct ReturnValue dong_porf_porf_todo_closestSelector(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 selector, i32 selectorjjtype);
static struct ReturnValue dong_porf_porf_todo_eventTarget(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_eventKey(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_todoIdAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype);
static struct ReturnValue dong_porf_porf_todo_todoTextAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype);
static struct ReturnValue dong_porf_porf_todo_todoDoneAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype);
static struct ReturnValue dong_porf_porf_todo_setTodoDoneAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype, f64 v, i32 vjjtype);
static struct ReturnValue dong_porf_porf_todo_setTodoSlot(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype, f64 id, i32 idjjtype, f64 text, i32 textjjtype, f64 done, i32 donejjtype);
static struct ReturnValue dong_porf_porf_todo_countActive(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_countDone(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_shouldShow(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype);
static struct ReturnValue dong_porf_porf_todo_porfRebuildList(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_parseIndexStr(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype);
static struct ReturnValue dong_porf_porf_todo_readToggleIndex(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_readDeleteIndex(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_porfPatchFilters(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_porfRefresh(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_removeAtIndex(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 idx, i32 idxjjtype);
struct ReturnValue dong_porf_porf_todo_onAdd(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static i32 dong_porf_porf_todo__Porffor_object_isObject(i32 arg, i32 argjjtype);
static struct ReturnValue dong_porf_porf_todo__String_fromCharCode(f64 codes, i32 codesjjtype);
static i32 dong_porf_porf_todo__Porffor_malloc(i32 l0);
static f64 dong_porf_porf_todo__ecma262_ToIntegerOrInfinity(f64 argument, i32 argumentjjtype);
static f64 dong_porf_porf_todo__ecma262_ToNumber(f64 argument, i32 argumentjjtype);
static f64 dong_porf_porf_todo_TypeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype);
static struct ReturnValue dong_porf_porf_todo__ecma262_ToString(f64 argument, i32 argumentjjtype);
static struct ReturnValue dong_porf_porf_todo__Number_prototype_toString(f64 _this, i32 _thisjjtype, f64 radix, i32 radixjjtype);
static f64 dong_porf_porf_todo__Math_trunc(f64 l0);
static f64 dong_porf_porf_todo_RangeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype);
static f64 dong_porf_porf_todo__Number_isFinite(f64 l0);
static f64 dong_porf_porf_todo__Number_isNaN(f64 l0);
static f64 dong_porf_porf_todo__Math_round(f64 l0);
static struct ReturnValue dong_porf_porf_todo__ecma262_ToPrimitive_String(f64 input, i32 inputjjtype);
static struct ReturnValue dong_porf_porf_todo__Boolean_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Function_prototype_toString(f64 _this, i32 _thisjjtype);
static f64 dong_porf_porf_todo__Porffor_bytestring_appendStr(f64 str, i32 strjjtype, f64 appendage, i32 appendagejjtype);
static i32 dong_porf_porf_todo__Porffor_funcLut_name(i32 l0);
static struct ReturnValue dong_porf_porf_todo__TypeError_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__TypeError_prototype_namekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__TypeError_prototype_messagekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_concatStrings(f64 a, i32 ajjtype, f64 b, i32 bjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_strcat(i32 a, i32 ajjtype, i32 b, i32 bjjtype);
static struct ReturnValue dong_porf_porf_todo__RangeError_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__RangeError_prototype_namekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__RangeError_prototype_messagekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_toString(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Object_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_get_withHash(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype, i32 hash, i32 hashjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_underlying(f64 _obj, i32 _objjjtype);
static i32 dong_porf_porf_todo__Porffor_funcLut_length(i32 l0);
static void dong_porf_porf_todo__Porffor_object_fastAdd(i32 obj, i32 objjjtype, i32 key, i32 keyjjtype, f64 value, i32 valuejjtype, i32 flags, i32 flagsjjtype);
static i32 dong_porf_porf_todo__Porffor_object_hash(i32 key, i32 keyjjtype);
static void dong_porf_porf_todo__Porffor_object_writeKey(i32 ptr, i32 ptrjjtype, i32 key, i32 keyjjtype, i32 hash, i32 hashjjtype);
static f64 dong_porf_porf_todo__ecma262_IsConstructor(f64 argument, i32 argumentjjtype);
static i32 dong_porf_porf_todo__Porffor_funcLut_flags(i32 l0);
static i32 dong_porf_porf_todo__Porffor_object_lookup(i32 obj, i32 objjjtype, i32 target, i32 targetjjtype, i32 targetHash, i32 targetHashjjtype);
static i32 dong_porf_porf_todo_jjget___Object_prototype();
static struct ReturnValue dong_porf_porf_todo__ecma262_ToPropertyKey(f64 argument, i32 argumentjjtype);
static f64 dong_porf_porf_todo__Array_from(f64 arg, i32 argjjtype, f64 mapFn, i32 mapFnjjtype, f64 thisArg, i32 thisArgjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_get(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_getHiddenPrototype(i32 trueType, i32 trueTypejjtype);
static i32 dong_porf_porf_todo__Porffor_strcmp(i32 a, i32 ajjtype, i32 b, i32 bjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_getPrototype(i32 obj, i32 objjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_accessorGet(i32 entryPtr, i32 entryPtrjjtype);
static f64 dong_porf_porf_todo__Porffor_compareStrings(f64 a, i32 ajjtype, f64 b, i32 bjjtype);
static struct ReturnValue dong_porf_porf_todo__Object_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_readValue(i32 entryPtr, i32 entryPtrjjtype);
static void dong_porf_porf_todo__Porffor_object_setPrototype(i32 obj, i32 objjjtype, i32 proto, i32 protojjtype);
static i32 dong_porf_porf_todo__Porffor_object_isObjectOrNull(i32 arg, i32 argjjtype);
static struct ReturnValue dong_porf_porf_todo_String(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 args, i32 argsjjtype);
static struct ReturnValue dong_porf_porf_todo__Symbol_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Symbol_prototype_descriptionkkget(f64 _this, i32 _thisjjtype);
static i32 dong_porf_porf_todo__Porffor_bytestringToString(i32 src);
static struct ReturnValue dong_porf_porf_todo__ecma262_ToPrimitive_Number(f64 input, i32 inputjjtype);
static struct ReturnValue dong_porf_porf_todo__Number_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Boolean_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_valueOf(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Array_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_valueOf(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_toString(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Array_prototype_toString(f64 _this, i32 _thisjjtype);
static f64 dong_porf_porf_todo__Porffor_bytestring_appendChar(f64 str, i32 strjjtype, f64 _char, i32 charjjtype);
static f64 dong_porf_porf_todo__ecma262_StringToNumber(f64 str, i32 strjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_trim(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_trimEnd(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_trimStart(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_trim(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_trimEnd(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_trimStart(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_charCodeAt(f64 _this, i32 _thisjjtype, f64 index, i32 indexjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_charCodeAt(f64 _this, i32 _thisjjtype, f64 index, i32 indexjjtype);
static f64 dong_porf_porf_todo__Porffor_stn_int(f64 str, i32 strjjtype, f64 radix, i32 radixjjtype, f64 i, i32 ijjtype);
static f64 dong_porf_porf_todo__Porffor_stn_float(f64 str, i32 strjjtype, f64 i, i32 ijjtype);
static f64 dong_porf_porf_todo__Porffor_parseExp(f64 str, i32 strjjtype, f64 i, i32 ijjtype, f64 len, i32 lenjjtype, f64 strict, i32 strictjjtype);
static f64 dong_porf_porf_todo__Math_pow(f64 base, i32 basejjtype, f64 exponent, i32 exponentjjtype);
static f64 dong_porf_porf_todo__Number_isInteger(f64 l0);
static f64 dong_porf_porf_todo__Math_abs(f64 l0);
static f64 dong_porf_porf_todo__Math_exp(f64 x, i32 xjjtype);
static f64 dong_porf_porf_todo__Math_floor(f64 l0);
static f64 dong_porf_porf_todo__Math_log(f64 y, i32 yjjtype);
static f64 dong_porf_porf_todo__Math_log2(f64 y, i32 yjjtype);
struct ReturnValue dong_porf_porf_todo_onInputChange(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onKeyDown(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onFilterAll(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onFilterActive(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onFilterDone(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onClearDone(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onListClick(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);

static f64 dong_porf_porf_todo_METRIC_OFFSET_WIDTH = 0;
static i32 dong_porf_porf_todo_METRIC_OFFSET_WIDTHjjtype = 0;
static f64 dong_porf_porf_todo_METRIC_OFFSET_HEIGHT = 0;
static i32 dong_porf_porf_todo_METRIC_OFFSET_HEIGHTjjtype = 0;
static f64 dong_porf_porf_todo_METRIC_OFFSET_TOP = 0;
static i32 dong_porf_porf_todo_METRIC_OFFSET_TOPjjtype = 0;
static f64 dong_porf_porf_todo_METRIC_OFFSET_LEFT = 0;
static i32 dong_porf_porf_todo_METRIC_OFFSET_LEFTjjtype = 0;
static f64 dong_porf_porf_todo_METRIC_CLIENT_WIDTH = 0;
static i32 dong_porf_porf_todo_METRIC_CLIENT_WIDTHjjtype = 0;
static f64 dong_porf_porf_todo_METRIC_CLIENT_HEIGHT = 0;
static i32 dong_porf_porf_todo_METRIC_CLIENT_HEIGHTjjtype = 0;
static f64 dong_porf_porf_todo_METRIC_SCROLL_WIDTH = 0;
static i32 dong_porf_porf_todo_METRIC_SCROLL_WIDTHjjtype = 0;
static f64 dong_porf_porf_todo_METRIC_SCROLL_HEIGHT = 0;
static i32 dong_porf_porf_todo_METRIC_SCROLL_HEIGHTjjtype = 0;
static f64 dong_porf_porf_todo_MAX_TODOS = 0;
static i32 dong_porf_porf_todo_MAX_TODOSjjtype = 0;
static f64 dong_porf_porf_todo_todoCount = 0;
static i32 dong_porf_porf_todo_todoCountjjtype = 0;
static f64 dong_porf_porf_todo_nextId = 0;
static i32 dong_porf_porf_todo_nextIdjjtype = 0;
static f64 dong_porf_porf_todo_filterMode = 0;
static i32 dong_porf_porf_todo_filterModejjtype = 0;
static f64 dong_porf_porf_todo_inputText = 0;
static i32 dong_porf_porf_todo_inputTextjjtype = 0;
static f64 dong_porf_porf_todo_todoId0 = 0;
static i32 dong_porf_porf_todo_todoId0jjtype = 0;
static f64 dong_porf_porf_todo_todoText0 = 0;
static i32 dong_porf_porf_todo_todoText0jjtype = 0;
static f64 dong_porf_porf_todo_todoDone0 = 0;
static i32 dong_porf_porf_todo_todoDone0jjtype = 0;
static f64 dong_porf_porf_todo_todoId1 = 0;
static i32 dong_porf_porf_todo_todoId1jjtype = 0;
static f64 dong_porf_porf_todo_todoText1 = 0;
static i32 dong_porf_porf_todo_todoText1jjtype = 0;
static f64 dong_porf_porf_todo_todoDone1 = 0;
static i32 dong_porf_porf_todo_todoDone1jjtype = 0;
static f64 dong_porf_porf_todo_todoId2 = 0;
static i32 dong_porf_porf_todo_todoId2jjtype = 0;
static f64 dong_porf_porf_todo_todoText2 = 0;
static i32 dong_porf_porf_todo_todoText2jjtype = 0;
static f64 dong_porf_porf_todo_todoDone2 = 0;
static i32 dong_porf_porf_todo_todoDone2jjtype = 0;
static f64 dong_porf_porf_todo_todoId3 = 0;
static i32 dong_porf_porf_todo_todoId3jjtype = 0;
static f64 dong_porf_porf_todo_todoText3 = 0;
static i32 dong_porf_porf_todo_todoText3jjtype = 0;
static f64 dong_porf_porf_todo_todoDone3 = 0;
static i32 dong_porf_porf_todo_todoDone3jjtype = 0;
static f64 dong_porf_porf_todo_todoInputId = 0;
static i32 dong_porf_porf_todo_todoInputIdjjtype = 0;
static f64 dong_porf_porf_todo_btnAddId = 0;
static i32 dong_porf_porf_todo_btnAddIdjjtype = 0;
static f64 dong_porf_porf_todo_filterAllId = 0;
static i32 dong_porf_porf_todo_filterAllIdjjtype = 0;
static f64 dong_porf_porf_todo_filterActiveId = 0;
static i32 dong_porf_porf_todo_filterActiveIdjjtype = 0;
static f64 dong_porf_porf_todo_filterDoneId = 0;
static i32 dong_porf_porf_todo_filterDoneIdjjtype = 0;
static f64 dong_porf_porf_todo_todoListId = 0;
static i32 dong_porf_porf_todo_todoListIdjjtype = 0;
static f64 dong_porf_porf_todo_clearWrapId = 0;
static i32 dong_porf_porf_todo_clearWrapIdjjtype = 0;
static f64 dong_porf_porf_todo_btnClearId = 0;
static i32 dong_porf_porf_todo_btnClearIdjjtype = 0;
static f64 dong_porf_porf_todo_todoId4 = 0;
static i32 dong_porf_porf_todo_todoId4jjtype = 0;
static f64 dong_porf_porf_todo_todoText4 = 0;
static i32 dong_porf_porf_todo_todoText4jjtype = 0;
static f64 dong_porf_porf_todo_todoDone4 = 0;
static i32 dong_porf_porf_todo_todoDone4jjtype = 0;
static f64 dong_porf_porf_todo_todoId5 = 0;
static i32 dong_porf_porf_todo_todoId5jjtype = 0;
static f64 dong_porf_porf_todo_todoText5 = 0;
static i32 dong_porf_porf_todo_todoText5jjtype = 0;
static f64 dong_porf_porf_todo_todoDone5 = 0;
static i32 dong_porf_porf_todo_todoDone5jjtype = 0;
static f64 dong_porf_porf_todo_todoId6 = 0;
static i32 dong_porf_porf_todo_todoId6jjtype = 0;
static f64 dong_porf_porf_todo_todoText6 = 0;
static i32 dong_porf_porf_todo_todoText6jjtype = 0;
static f64 dong_porf_porf_todo_todoDone6 = 0;
static i32 dong_porf_porf_todo_todoDone6jjtype = 0;
static f64 dong_porf_porf_todo_todoId7 = 0;
static i32 dong_porf_porf_todo_todoId7jjtype = 0;
static f64 dong_porf_porf_todo_todoText7 = 0;
static i32 dong_porf_porf_todo_todoText7jjtype = 0;
static f64 dong_porf_porf_todo_todoDone7 = 0;
static i32 dong_porf_porf_todo_todoDone7jjtype = 0;
static f64 dong_porf_porf_todo_todoId8 = 0;
static i32 dong_porf_porf_todo_todoId8jjtype = 0;
static f64 dong_porf_porf_todo_todoText8 = 0;
static i32 dong_porf_porf_todo_todoText8jjtype = 0;
static f64 dong_porf_porf_todo_todoDone8 = 0;
static i32 dong_porf_porf_todo_todoDone8jjtype = 0;
static f64 dong_porf_porf_todo_todoId9 = 0;
static i32 dong_porf_porf_todo_todoId9jjtype = 0;
static f64 dong_porf_porf_todo_todoText9 = 0;
static i32 dong_porf_porf_todo_todoText9jjtype = 0;
static f64 dong_porf_porf_todo_todoDone9 = 0;
static i32 dong_porf_porf_todo_todoDone9jjtype = 0;
static f64 dong_porf_porf_todo_todoId10 = 0;
static i32 dong_porf_porf_todo_todoId10jjtype = 0;
static f64 dong_porf_porf_todo_todoText10 = 0;
static i32 dong_porf_porf_todo_todoText10jjtype = 0;
static f64 dong_porf_porf_todo_todoDone10 = 0;
static i32 dong_porf_porf_todo_todoDone10jjtype = 0;
static f64 dong_porf_porf_todo_todoId11 = 0;
static i32 dong_porf_porf_todo_todoId11jjtype = 0;
static f64 dong_porf_porf_todo_todoText11 = 0;
static i32 dong_porf_porf_todo_todoText11jjtype = 0;
static f64 dong_porf_porf_todo_todoDone11 = 0;
static i32 dong_porf_porf_todo_todoDone11jjtype = 0;
static f64 dong_porf_porf_todo_todoId12 = 0;
static i32 dong_porf_porf_todo_todoId12jjtype = 0;
static f64 dong_porf_porf_todo_todoText12 = 0;
static i32 dong_porf_porf_todo_todoText12jjtype = 0;
static f64 dong_porf_porf_todo_todoDone12 = 0;
static i32 dong_porf_porf_todo_todoDone12jjtype = 0;
static f64 dong_porf_porf_todo_todoId13 = 0;
static i32 dong_porf_porf_todo_todoId13jjtype = 0;
static f64 dong_porf_porf_todo_todoText13 = 0;
static i32 dong_porf_porf_todo_todoText13jjtype = 0;
static f64 dong_porf_porf_todo_todoDone13 = 0;
static i32 dong_porf_porf_todo_todoDone13jjtype = 0;
static f64 dong_porf_porf_todo_todoId14 = 0;
static i32 dong_porf_porf_todo_todoId14jjtype = 0;
static f64 dong_porf_porf_todo_todoText14 = 0;
static i32 dong_porf_porf_todo_todoText14jjtype = 0;
static f64 dong_porf_porf_todo_todoDone14 = 0;
static i32 dong_porf_porf_todo_todoDone14jjtype = 0;
static f64 dong_porf_porf_todo_todoId15 = 0;
static i32 dong_porf_porf_todo_todoId15jjtype = 0;
static f64 dong_porf_porf_todo_todoText15 = 0;
static i32 dong_porf_porf_todo_todoText15jjtype = 0;
static f64 dong_porf_porf_todo_todoDone15 = 0;
static i32 dong_porf_porf_todo_todoDone15jjtype = 0;
static i32 dong_porf_porf_todo_jjporfjjcurrentPtr = 0;
static i32 dong_porf_porf_todo_jjporfjjcurrentPtrjjglbl_inited = 0;
static i32 dong_porf_porf_todo_jjporfjjendPtr = 0;
static i32 dong_porf_porf_todo_jjporfjjendPtrjjglbl_inited = 0;
static i32 dong_porf_porf_todo_jjporfjjunderlyingStore = 0;
static i32 dong_porf_porf_todo_jjporfjjunderlyingStorejjglbl_inited = 0;
static i32 dong_porf_porf_todo_jjporfjjgetptr___Object_prototype = 0;
static i32 dong_porf_porf_todo_jjporfjjgetptr___Object_prototypejjglbl_inited = 0;

static f64 dong_porf_porf_todo__Math_trunc(f64 l0) {
  f64 _get0;
  _get0 = l0;
  return trunc(_get0);
}

static i32 dong_porf_porf_todo__Porffor_malloc(i32 l0) {
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get0;
  _get0 = l0;
  // if i32
  i32 _r15;
    if ((dong_porf_porf_todo_jjporfjjcurrentPtr + _get0) >= dong_porf_porf_todo_jjporfjjendPtr) {
      const u32 _oldPages1 = dong_porf_porf_todo_memory_pages;
      dong_porf_porf_todo_memory_pages += 16;
      dong_porf_porf_todo_memory = realloc(dong_porf_porf_todo_memory, dong_porf_porf_todo_memory_pages * 65536);
      memset(dong_porf_porf_todo_memory + _oldPages1 * 65536, 0, (dong_porf_porf_todo_memory_pages - _oldPages1) * 65536);
      _get2 = l0;
      dong_porf_porf_todo_jjporfjjcurrentPtr = (_oldPages1 * 65536) + _get2;
      _get3 = l0;
      dong_porf_porf_todo_jjporfjjendPtr = (dong_porf_porf_todo_jjporfjjcurrentPtr + 1048576) - _get3;
      _get4 = l0;
      _r15 = dong_porf_porf_todo_jjporfjjcurrentPtr - _get4;
    } else {
      _get5 = l0;
      dong_porf_porf_todo_jjporfjjcurrentPtr = dong_porf_porf_todo_jjporfjjcurrentPtr + _get5;
      _r15 = dong_porf_porf_todo_jjporfjjcurrentPtr;
    }
  // end
  j15:;
  return _r15;
}

static f64 dong_porf_porf_todo_RangeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype) {
  f64 _get9;
  i32 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  i32 jjlast_type = 0;
  f64 obj = 0;

  _get0 = message;
  _get1 = messagejjtype;
  // if 
    if (((_get0 == 0) & ((_get1 | 128) == (0 | 128))) != 0) {
      message = 0;
      messagejjtype = 195;
    } else {
      _get2 = message;
      _get3 = messagejjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString(_get2, _get3);
      jjlast_type = _0.type;
      _get4 = jjlast_type;
      messagejjtype = _get4;
      message = _0.value;
    }
  // end
  j14:;
  obj = (f64)(dong_porf_porf_todo__Porffor_malloc(8));
  _get5 = obj;
  _get6 = message;
  i32_store(0, 0, (i32)(_get5), (i32)(_get6));
  _get7 = obj;
  _get8 = messagejjtype;
  i32_store8(0, 4, (i32)(_get7), _get8);
  _get9 = obj;
  return _get9;
}

static f64 dong_porf_porf_todo__Number_isFinite(f64 l0) {
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 l1 = 0;

  _get0 = l0;
  _get1 = l0;
  l1 = _get0 - _get1;
  _get2 = l1;
  _get3 = l1;
  return (f64)(_get2 == _get3);
}

static f64 dong_porf_porf_todo__Number_isNaN(f64 l0) {
  f64 _get1;
  f64 _get0;
  _get0 = l0;
  _get1 = l0;
  return (f64)(_get0 != _get1);
}

static f64 dong_porf_porf_todo__Math_round(f64 l0) {
  f64 _get0;
  _get0 = l0;
  return round(_get0);
}

static struct ReturnValue dong_porf_porf_todo__Number_prototype_toString(f64 _this, i32 _thisjjtype, f64 radix, i32 radixjjtype) {
  f64 _get239;
  f64 _get238;
  f64 _get237;
  f64 _get236;
  f64 _get235;
  f64 _get234;
  f64 _get233;
  f64 _get232;
  f64 _get231;
  f64 _get230;
  f64 _get229;
  f64 _get228;
  f64 _get227;
  f64 _get226;
  f64 _get225;
  f64 _get224;
  f64 _get223;
  f64 _get222;
  f64 _get221;
  f64 _get220;
  f64 _get219;
  f64 _get218;
  f64 _get217;
  f64 _get216;
  f64 _get215;
  f64 _get214;
  f64 _get213;
  f64 _get212;
  f64 _get211;
  f64 _get210;
  f64 _get209;
  f64 _get208;
  f64 _get207;
  f64 _get206;
  f64 _get205;
  f64 _get204;
  f64 _get203;
  f64 _get202;
  f64 _get201;
  f64 _get200;
  f64 _get199;
  f64 _get198;
  f64 _get197;
  f64 _get196;
  f64 _get195;
  f64 _get194;
  f64 _get193;
  f64 _get192;
  f64 _get191;
  f64 _get190;
  f64 _get189;
  f64 _get188;
  f64 _get187;
  f64 _get186;
  f64 _get185;
  f64 _get184;
  f64 _get183;
  f64 _get182;
  f64 _get181;
  f64 _get180;
  f64 _get179;
  f64 _get178;
  f64 _get177;
  f64 _get176;
  f64 _get175;
  f64 _get174;
  f64 _get173;
  f64 _get172;
  f64 _get171;
  f64 _get170;
  f64 _get169;
  f64 _get168;
  f64 _get167;
  f64 _get166;
  f64 _get165;
  f64 _get164;
  f64 _get163;
  f64 _get162;
  f64 _get161;
  f64 _get160;
  f64 _get159;
  f64 _get158;
  f64 _get157;
  f64 _get156;
  f64 _get155;
  f64 _get154;
  f64 _get153;
  f64 _get152;
  f64 _get151;
  f64 _get150;
  f64 _get149;
  f64 _get148;
  f64 _get147;
  f64 _get146;
  f64 _get145;
  f64 _get144;
  f64 _get143;
  f64 _get142;
  f64 _get141;
  f64 _get140;
  f64 _get139;
  f64 _get138;
  f64 _get137;
  f64 _get136;
  f64 _get135;
  f64 _get134;
  f64 _get133;
  f64 _get132;
  f64 _get131;
  f64 _get130;
  f64 _get129;
  f64 _get128;
  f64 _get127;
  f64 _get126;
  f64 _get125;
  f64 _get124;
  f64 _get123;
  f64 _get122;
  f64 _get121;
  f64 _get120;
  f64 _get119;
  f64 _get118;
  f64 _get117;
  f64 _get116;
  f64 _get115;
  f64 _get114;
  f64 _get113;
  f64 _get112;
  f64 _get111;
  f64 _get110;
  f64 _get109;
  f64 _get108;
  f64 _get107;
  f64 _get106;
  f64 _get105;
  f64 _get104;
  f64 _get103;
  f64 _get102;
  f64 _get101;
  f64 _get100;
  f64 _get99;
  f64 _get98;
  f64 _get97;
  f64 _get96;
  f64 _get95;
  f64 _get94;
  f64 _get93;
  f64 _get92;
  f64 _get91;
  f64 _get90;
  f64 _get89;
  f64 _get88;
  f64 _get87;
  f64 _get86;
  f64 _get85;
  f64 _get84;
  f64 _get83;
  f64 _get82;
  f64 _get81;
  f64 _get80;
  f64 _get79;
  f64 _get78;
  f64 _get77;
  f64 _get76;
  f64 _get75;
  f64 _get74;
  f64 _get73;
  f64 _get72;
  f64 _get71;
  f64 _get70;
  f64 _get69;
  f64 _get68;
  f64 _get67;
  f64 _get66;
  f64 _get65;
  f64 _get64;
  f64 _get63;
  f64 _get62;
  f64 _get61;
  f64 _get60;
  f64 _get59;
  f64 _get58;
  f64 _get57;
  f64 _get56;
  f64 _get55;
  f64 _get54;
  f64 _get53;
  f64 _get52;
  f64 _get51;
  f64 _get50;
  f64 _get49;
  f64 _get48;
  f64 _get47;
  f64 _get46;
  f64 _get45;
  f64 _get44;
  f64 _get43;
  f64 _get42;
  f64 _get41;
  f64 _get40;
  f64 _get39;
  f64 _get38;
  f64 _get37;
  f64 _get36;
  f64 _get35;
  f64 _get34;
  f64 _get33;
  f64 _get32;
  f64 _get31;
  f64 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  f64 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  f64 _get22;
  f64 _get21;
  f64 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 logictmpi = 0;
  i32 jjlast_type = 0;
  f64 out = 0;
  f64 outPtr = 0;
  f64 i = 0;
  f64 digits = 0;
  f64 l = 0;
  f64 trailing = 0;
  f64 e = 0;
  f64 digit = 0;
  f64 jjmath_a = 0;
  f64 jjmath_b = 0;
  f64 digitsPtr = 0;
  f64 endPtr = 0;
  f64 dotPlace = 0;
  f64 decimal = 0;
  f64 intPart = 0;
  f64 decimalDigits = 0;
  f64 j = 0;

  _get0 = _thisjjtype;
  _get1 = _thisjjtype;
  // if 
    if (((_get0 != 1) & (_get1 != 32)) != 0) {
    }
  // end
  j10:;
  _get2 = radixjjtype;
  // if 
    if ((f64)(_get2) != 1) {
      radix = 10;
      radixjjtype = 1;
    }
  // end
  j11:;
  _get3 = radix;
  radix = dong_porf_porf_todo__Math_trunc(_get3);
  radixjjtype = 1;
  _get4 = radix;
  logictmpi = _get4 < 2;
  _get5 = logictmpi;
  // if i32
  i32 _r12;
    if ((_get5) == 0) {
      _get6 = radix;
      jjlast_type = 2;
      _r12 = _get6 > 36;
    } else {
      _get7 = logictmpi;
      jjlast_type = 2;
      _r12 = _get7;
    }
  // end
  j12:;
  // if 
    if ((_r12) != 0) {
    }
  // end
  j13:;
  _get8 = _this;
  // if 
    if (dong_porf_porf_todo__Number_isFinite(_get8) == 0) {
      _get9 = _this;
      // if 
        if (((u32)(dong_porf_porf_todo__Number_isNaN(_get9))) != 0) {
          return (struct ReturnValue){ 360, 195 };
        }
      // end
      j17:;
      _get10 = _this;
      // if 
        if (_get10 == Infinity) {
          return (struct ReturnValue){ 369, 195 };
        }
      // end
      j18:;
      return (struct ReturnValue){ 383, 195 };
    }
  // end
  j16:;
  _get11 = _this;
  // if 
    if (_get11 == 0) {
      return (struct ReturnValue){ 398, 195 };
    }
  // end
  j19:;
  out = (f64)(dong_porf_porf_todo__Porffor_malloc(512));
  _get12 = out;
  outPtr = _get12;
  _get13 = _this;
  // if 
    if (_get13 < 0) {
      _get14 = _this;
      _this = -_get14;
      _get15 = outPtr;
      _get16 = outPtr;
      outPtr = _get16 + 1;
      i32_store8(0, 4, (i32)(_get15), 45);
    }
  // end
  j20:;
  _get17 = _this;
  i = dong_porf_porf_todo__Math_trunc(_get17);
  digits = 0;
  l = 0;
  _get18 = radix;
  // if 
    if (_get18 == 10) {
      _get19 = i;
      // if 
        if (_get19 >= 1e+21) {
          trailing = 1;
          e = -1;
          // loop 
          j23:;
            _get20 = i;
            // if 
              if (_get20 > 0) {
                _get21 = i;
                jjmath_a = _get21;
                _get22 = jjmath_a;
                _get23 = radix;
                jjmath_b = _get23;
                _get24 = jjmath_b;
                _get25 = jjmath_a;
                _get26 = jjmath_b;
                digit = _get22 - (_get24 * trunc((_get25 / _get26)));
                _get27 = i;
                _get28 = radix;
                i = dong_porf_porf_todo__Math_trunc(_get27 / _get28);
                _get29 = e;
                e = _get29 + 1;
                _get30 = trailing;
                // if 
                  if (((u32)(_get30)) != 0) {
                    _get31 = digit;
                    // if 
                      if (_get31 == 0) {
                        goto j23;
                      }
                    // end
                    j26:;
                    trailing = 0;
                  }
                // end
                j25:;
                _get32 = digits;
                _get33 = l;
                _get34 = digit;
                i32_store8(0, 4, (i32)((_get32 + _get33)), (i32)(_get34));
                _get35 = l;
                l = _get35 + 1;
                goto j23;
              }
            // end
            j24:;
          // end
          _get36 = digits;
          _get37 = l;
          digitsPtr = _get36 + _get37;
          _get38 = outPtr;
          _get39 = l;
          endPtr = _get38 + _get39;
          _get40 = outPtr;
          dotPlace = _get40 + 1;
          // loop 
          j27:;
            _get41 = outPtr;
            _get42 = endPtr;
            // if 
              if (_get41 < _get42) {
                _get43 = outPtr;
                _get44 = dotPlace;
                // if 
                  if (_get43 == _get44) {
                    _get45 = outPtr;
                    _get46 = outPtr;
                    outPtr = _get46 + 1;
                    i32_store8(0, 4, (i32)(_get45), 46);
                    _get47 = endPtr;
                    endPtr = _get47 + 1;
                  }
                // end
                j29:;
                _get48 = digitsPtr;
                digitsPtr = _get48 - 1;
                _get49 = digitsPtr;
                digit = (f64)(i32_load8_u(0, 4, (i32)(_get49)));
                _get50 = digit;
                // if 
                  if (_get50 < 10) {
                    _get51 = digit;
                    digit = _get51 + 48;
                  } else {
                    _get52 = digit;
                    digit = _get52 + 87;
                  }
                // end
                j30:;
                _get53 = outPtr;
                _get54 = outPtr;
                outPtr = _get54 + 1;
                _get55 = digit;
                i32_store8(0, 4, (i32)(_get53), (i32)(_get55));
                goto j27;
              }
            // end
            j28:;
          // end
          _get56 = outPtr;
          _get57 = outPtr;
          outPtr = _get57 + 1;
          i32_store8(0, 4, (i32)(_get56), 101);
          _get58 = outPtr;
          _get59 = outPtr;
          outPtr = _get59 + 1;
          i32_store8(0, 4, (i32)(_get58), 43);
          l = 0;
          // loop 
          j31:;
            _get60 = e;
            // if 
              if (_get60 > 0) {
                _get61 = digits;
                _get62 = l;
                _get63 = e;
                jjmath_a = _get63;
                _get64 = jjmath_a;
                _get65 = radix;
                jjmath_b = _get65;
                _get66 = jjmath_b;
                _get67 = jjmath_a;
                _get68 = jjmath_b;
                i32_store8(0, 4, (i32)((_get61 + _get62)), (i32)((_get64 - (_get66 * trunc((_get67 / _get68))))));
                _get69 = e;
                _get70 = radix;
                e = dong_porf_porf_todo__Math_trunc(_get69 / _get70);
                _get71 = l;
                l = _get71 + 1;
                goto j31;
              }
            // end
            j32:;
          // end
          _get72 = digits;
          _get73 = l;
          digitsPtr = _get72 + _get73;
          _get74 = outPtr;
          _get75 = l;
          endPtr = _get74 + _get75;
          // loop 
          j33:;
            _get76 = outPtr;
            _get77 = endPtr;
            // if 
              if (_get76 < _get77) {
                _get78 = digitsPtr;
                digitsPtr = _get78 - 1;
                _get79 = digitsPtr;
                digit = (f64)(i32_load8_u(0, 4, (i32)(_get79)));
                _get80 = digit;
                // if 
                  if (_get80 < 10) {
                    _get81 = digit;
                    digit = _get81 + 48;
                  } else {
                    _get82 = digit;
                    digit = _get82 + 87;
                  }
                // end
                j35:;
                _get83 = outPtr;
                _get84 = outPtr;
                outPtr = _get84 + 1;
                _get85 = digit;
                i32_store8(0, 4, (i32)(_get83), (i32)(_get85));
                goto j33;
              }
            // end
            j34:;
          // end
          _get86 = out;
          _get87 = outPtr;
          _get88 = out;
          i32_store(1, 0, (u32)(_get86), (u32)((_get87 - _get88)));
          _get89 = out;
          return (struct ReturnValue){ _get89, 195 };
        }
      // end
      j22:;
      _get90 = _this;
      // if 
        if (_get90 < 0.000001) {
          _get91 = _this;
          decimal = _get91;
          e = 1;
          // loop 
          j37:;
            // if 
              if ((1) != 0) {
                _get92 = decimal;
                _get93 = radix;
                decimal = _get92 * _get93;
                _get94 = decimal;
                intPart = dong_porf_porf_todo__Math_trunc(_get94);
                _get95 = intPart;
                // if 
                  if (_get95 > 0) {
                    _get96 = decimal;
                    _get97 = intPart;
                    // if 
                      if ((_get96 - _get97) < 1e-10) {
                        goto j38;
                      }
                    // end
                    j40:;
                  } else {
                    _get98 = e;
                    e = _get98 + 1;
                  }
                // end
                j39:;
                goto j37;
              }
            // end
            j38:;
          // end
          // loop 
          j41:;
            _get99 = decimal;
            // if 
              if (_get99 > 0) {
                _get100 = decimal;
                jjmath_a = _get100;
                _get101 = jjmath_a;
                _get102 = radix;
                jjmath_b = _get102;
                _get103 = jjmath_b;
                _get104 = jjmath_a;
                _get105 = jjmath_b;
                digit = _get101 - (_get103 * trunc((_get104 / _get105)));
                _get106 = decimal;
                _get107 = radix;
                decimal = dong_porf_porf_todo__Math_trunc(_get106 / _get107);
                _get108 = digits;
                _get109 = l;
                _get110 = digit;
                i32_store8(0, 4, (i32)((_get108 + _get109)), (i32)(_get110));
                _get111 = l;
                l = _get111 + 1;
                goto j41;
              }
            // end
            j42:;
          // end
          _get112 = digits;
          _get113 = l;
          digitsPtr = _get112 + _get113;
          _get114 = outPtr;
          _get115 = l;
          endPtr = _get114 + _get115;
          _get116 = outPtr;
          dotPlace = _get116 + 1;
          // loop 
          j43:;
            _get117 = outPtr;
            _get118 = endPtr;
            // if 
              if (_get117 < _get118) {
                _get119 = digitsPtr;
                digitsPtr = _get119 - 1;
                _get120 = digitsPtr;
                digit = (f64)(i32_load8_u(0, 4, (i32)(_get120)));
                _get121 = outPtr;
                _get122 = dotPlace;
                // if 
                  if (_get121 == _get122) {
                    _get123 = outPtr;
                    _get124 = outPtr;
                    outPtr = _get124 + 1;
                    i32_store8(0, 4, (i32)(_get123), 46);
                    _get125 = endPtr;
                    endPtr = _get125 + 1;
                  }
                // end
                j45:;
                _get126 = digit;
                // if 
                  if (_get126 < 10) {
                    _get127 = digit;
                    digit = _get127 + 48;
                  } else {
                    _get128 = digit;
                    digit = _get128 + 87;
                  }
                // end
                j46:;
                _get129 = outPtr;
                _get130 = outPtr;
                outPtr = _get130 + 1;
                _get131 = digit;
                i32_store8(0, 4, (i32)(_get129), (i32)(_get131));
                goto j43;
              }
            // end
            j44:;
          // end
          _get132 = outPtr;
          _get133 = outPtr;
          outPtr = _get133 + 1;
          i32_store8(0, 4, (i32)(_get132), 101);
          _get134 = outPtr;
          _get135 = outPtr;
          outPtr = _get135 + 1;
          i32_store8(0, 4, (i32)(_get134), 45);
          l = 0;
          // loop 
          j47:;
            _get136 = e;
            // if 
              if (_get136 > 0) {
                _get137 = digits;
                _get138 = l;
                _get139 = e;
                jjmath_a = _get139;
                _get140 = jjmath_a;
                _get141 = radix;
                jjmath_b = _get141;
                _get142 = jjmath_b;
                _get143 = jjmath_a;
                _get144 = jjmath_b;
                i32_store8(0, 4, (i32)((_get137 + _get138)), (i32)((_get140 - (_get142 * trunc((_get143 / _get144))))));
                _get145 = e;
                _get146 = radix;
                e = dong_porf_porf_todo__Math_trunc(_get145 / _get146);
                _get147 = l;
                l = _get147 + 1;
                goto j47;
              }
            // end
            j48:;
          // end
          _get148 = digits;
          _get149 = l;
          digitsPtr = _get148 + _get149;
          _get150 = outPtr;
          _get151 = l;
          endPtr = _get150 + _get151;
          // loop 
          j49:;
            _get152 = outPtr;
            _get153 = endPtr;
            // if 
              if (_get152 < _get153) {
                _get154 = digitsPtr;
                digitsPtr = _get154 - 1;
                _get155 = digitsPtr;
                digit = (f64)(i32_load8_u(0, 4, (i32)(_get155)));
                _get156 = digit;
                // if 
                  if (_get156 < 10) {
                    _get157 = digit;
                    digit = _get157 + 48;
                  } else {
                    _get158 = digit;
                    digit = _get158 + 87;
                  }
                // end
                j51:;
                _get159 = outPtr;
                _get160 = outPtr;
                outPtr = _get160 + 1;
                _get161 = digit;
                i32_store8(0, 4, (i32)(_get159), (i32)(_get161));
                goto j49;
              }
            // end
            j50:;
          // end
          _get162 = out;
          _get163 = outPtr;
          _get164 = out;
          i32_store(1, 0, (u32)(_get162), (u32)((_get163 - _get164)));
          _get165 = out;
          return (struct ReturnValue){ _get165, 195 };
        }
      // end
      j36:;
    }
  // end
  j21:;
  _get166 = i;
  // if 
    if (_get166 == 0) {
      _get167 = digits;
      i32_store8(0, 4, (i32)(_get167), 0);
      l = 1;
    } else {
      // loop 
      j53:;
        _get168 = i;
        // if 
          if (_get168 > 0) {
            _get169 = digits;
            _get170 = l;
            _get171 = i;
            jjmath_a = _get171;
            _get172 = jjmath_a;
            _get173 = radix;
            jjmath_b = _get173;
            _get174 = jjmath_b;
            _get175 = jjmath_a;
            _get176 = jjmath_b;
            i32_store8(0, 4, (i32)((_get169 + _get170)), (i32)((_get172 - (_get174 * trunc((_get175 / _get176))))));
            _get177 = i;
            _get178 = radix;
            i = dong_porf_porf_todo__Math_trunc(_get177 / _get178);
            _get179 = l;
            l = _get179 + 1;
            goto j53;
          }
        // end
        j54:;
      // end
    }
  // end
  j52:;
  _get180 = digits;
  _get181 = l;
  digitsPtr = _get180 + _get181;
  _get182 = outPtr;
  _get183 = l;
  endPtr = _get182 + _get183;
  // loop 
  j55:;
    _get184 = outPtr;
    _get185 = endPtr;
    // if 
      if (_get184 < _get185) {
        _get186 = digitsPtr;
        digitsPtr = _get186 - 1;
        _get187 = digitsPtr;
        digit = (f64)(i32_load8_u(0, 4, (i32)(_get187)));
        _get188 = digit;
        // if 
          if (_get188 < 10) {
            _get189 = digit;
            digit = _get189 + 48;
          } else {
            _get190 = digit;
            digit = _get190 + 87;
          }
        // end
        j57:;
        _get191 = outPtr;
        _get192 = outPtr;
        outPtr = _get192 + 1;
        _get193 = digit;
        i32_store8(0, 4, (i32)(_get191), (i32)(_get193));
        goto j55;
      }
    // end
    j56:;
  // end
  _get194 = _this;
  _get195 = _this;
  decimal = _get194 - dong_porf_porf_todo__Math_trunc(_get195);
  _get196 = decimal;
  // if 
    if (_get196 > 0) {
      _get197 = outPtr;
      _get198 = outPtr;
      outPtr = _get198 + 1;
      i32_store8(0, 4, (i32)(_get197), 46);
      _get199 = decimal;
      decimal = _get199 + 1;
      _get200 = l;
      decimalDigits = 16 - _get200;
      j = 0;
      // loop 
      j59:;
        _get201 = j;
        _get202 = decimalDigits;
        // if 
          if (_get201 < _get202) {
            _get203 = decimal;
            _get204 = radix;
            decimal = _get203 * _get204;
            _get205 = j;
            j = _get205 + 1;
            goto j59;
          }
        // end
        j60:;
      // end
      _get206 = decimal;
      decimal = dong_porf_porf_todo__Math_round(_get206);
      l = 0;
      trailing = 1;
      // loop 
      j61:;
        _get207 = decimal;
        // if 
          if (_get207 > 1) {
            _get208 = decimal;
            jjmath_a = _get208;
            _get209 = jjmath_a;
            _get210 = radix;
            jjmath_b = _get210;
            _get211 = jjmath_b;
            _get212 = jjmath_a;
            _get213 = jjmath_b;
            digit = _get209 - (_get211 * trunc((_get212 / _get213)));
            _get214 = decimal;
            _get215 = radix;
            decimal = dong_porf_porf_todo__Math_trunc(_get214 / _get215);
            _get216 = trailing;
            // if 
              if (((u32)(_get216)) != 0) {
                _get217 = digit;
                // if 
                  if (_get217 == 0) {
                    goto j61;
                  }
                // end
                j64:;
                trailing = 0;
              }
            // end
            j63:;
            _get218 = digits;
            _get219 = l;
            _get220 = digit;
            i32_store8(0, 4, (i32)((_get218 + _get219)), (i32)(_get220));
            _get221 = l;
            l = _get221 + 1;
            goto j61;
          }
        // end
        j62:;
      // end
      _get222 = digits;
      _get223 = l;
      digitsPtr = _get222 + _get223;
      _get224 = outPtr;
      _get225 = l;
      endPtr = _get224 + _get225;
      // loop 
      j65:;
        _get226 = outPtr;
        _get227 = endPtr;
        // if 
          if (_get226 < _get227) {
            _get228 = digitsPtr;
            digitsPtr = _get228 - 1;
            _get229 = digitsPtr;
            digit = (f64)(i32_load8_u(0, 4, (i32)(_get229)));
            _get230 = digit;
            // if 
              if (_get230 < 10) {
                _get231 = digit;
                digit = _get231 + 48;
              } else {
                _get232 = digit;
                digit = _get232 + 87;
              }
            // end
            j67:;
            _get233 = outPtr;
            _get234 = outPtr;
            outPtr = _get234 + 1;
            _get235 = digit;
            i32_store8(0, 4, (i32)(_get233), (i32)(_get235));
            goto j65;
          }
        // end
        j66:;
      // end
    }
  // end
  j58:;
  _get236 = out;
  _get237 = outPtr;
  _get238 = out;
  i32_store(1, 0, (u32)(_get236), (u32)((_get237 - _get238)));
  _get239 = out;
  return (struct ReturnValue){ _get239, 195 };
}

static struct ReturnValue dong_porf_porf_todo__Boolean_prototype_toString(f64 _this, i32 _thisjjtype) {
  f64 _get0;
  _get0 = _this;
  // if 
    if (((u32)(_get0)) != 0) {
      return (struct ReturnValue){ 222, 195 };
    }
  // end
  j73:;
  return (struct ReturnValue){ 232, 195 };
}

static f64 dong_porf_porf_todo__Porffor_bytestring_appendStr(f64 str, i32 strjjtype, f64 appendage, i32 appendagejjtype) {
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  f64 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 strLen = 0;
  f64 appendageLen = 0;
  f64 strPtr = 0;
  f64 appendagePtr = 0;
  f64 endPtr = 0;

  _get0 = str;
  strLen = (f64)(i32_load(1, 0, (u32)(_get0)));
  _get1 = appendage;
  appendageLen = (f64)(i32_load(1, 0, (u32)(_get1)));
  _get2 = str;
  _get3 = strLen;
  strPtr = _get2 + _get3;
  _get4 = appendage;
  appendagePtr = _get4;
  _get5 = appendagePtr;
  _get6 = appendageLen;
  endPtr = _get5 + _get6;
  // loop 
  j76:;
    _get7 = appendagePtr;
    _get8 = endPtr;
    // if 
      if (_get7 < _get8) {
        _get9 = strPtr;
        _get10 = strPtr;
        strPtr = _get10 + 1;
        _get11 = appendagePtr;
        _get12 = appendagePtr;
        appendagePtr = _get12 + 1;
        i32_store8(0, 4, (i32)(_get9), i32_load8_u(0, 4, (i32)(_get11)));
        goto j76;
      }
    // end
    j77:;
  // end
  _get13 = str;
  _get14 = strLen;
  _get15 = appendageLen;
  i32_store(1, 0, (u32)(_get13), (u32)((_get14 + _get15)));
  return 1;
}

static i32 dong_porf_porf_todo__Porffor_funcLut_name(i32 l0) {
  i32 _get0;
  _get0 = l0;
  return (((_get0 * 57) + 3) + 16384);
}

static struct ReturnValue dong_porf_porf_todo__Function_prototype_toString(f64 _this, i32 _thisjjtype) {
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  i32 _get0;
  f64 out = 0;

  _get0 = _thisjjtype;
  // if 
    if (_get0 != 6) {
    }
  // end
  j75:;
  out = (f64)(dong_porf_porf_todo__Porffor_malloc(256));
  _get1 = out;
  (void) dong_porf_porf_todo__Porffor_bytestring_appendStr(_get1, 195, 470, 195);
  _get2 = out;
  _get3 = _this;
  (void) dong_porf_porf_todo__Porffor_bytestring_appendStr(_get2, 195, (f64)(dong_porf_porf_todo__Porffor_funcLut_name((i32)(_get3))), 195);
  _get4 = out;
  (void) dong_porf_porf_todo__Porffor_bytestring_appendStr(_get4, 195, 485, 195);
  _get5 = out;
  return (struct ReturnValue){ _get5, 195 };
}

static i32 dong_porf_porf_todo__Porffor_funcLut_length(i32 l0) {
  i32 _get0;
  _get0 = l0;
  return i32_load16_u(0, 16384, _get0 * 57);
}

static i32 dong_porf_porf_todo__Porffor_object_hash(i32 key, i32 keyjjtype) {
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 p = 0;
  i32 len = 0;
  i32 hash = 0;
  i32 end = 0;

  _get0 = key;
  p = _get0;
  _get1 = key;
  len = i32_load(0, 0, _get1);
  _get2 = keyjjtype;
  // if 
    if (_get2 == 67) {
      _get3 = len;
      len = _get3 * 2;
    }
  // end
  j90:;
  _get4 = len;
  hash = 374761393 + _get4;
  _get5 = p;
  _get6 = len;
  end = _get5 + _get6;
  // loop 
  j91:;
    _get7 = p;
    _get8 = end;
    // if 
      if ((_get7 + 4) <= _get8) {
        _get9 = hash;
        _get10 = p;
        const i32 _x0 = (_get9 + (i32_load(0, 4, _get10) * 3266489917));
        const i32 _shift0 = 17;
        const i32 _out0 = (_x0 << _shift0) | ((u32)_x0 >> (32 - _shift0));
        hash = _out0 * 668265263;
        _get11 = p;
        p = _get11 + 4;
        goto j91;
      }
    // end
    j92:;
  // end
  _get12 = hash;
  _get13 = p;
  _get14 = end;
  _get15 = p;
  const i32 _x1 = (_get12 + ((i32_load(0, 4, _get13) & ((1 << ((_get14 - _get15) * 8)) - 1)) * 3266489917));
  const i32 _shift1 = 17;
  const i32 _out1 = (_x1 << _shift1) | ((u32)_x1 >> (32 - _shift1));
  hash = _out1 * 668265263;
  _get16 = hash;
  _get17 = hash;
  hash = (_get16 ^ ((u32)(_get17) >> 15)) * 2246822519;
  _get18 = hash;
  _get19 = hash;
  hash = (_get18 ^ ((u32)(_get19) >> 13)) * 3266489917;
  _get20 = hash;
  _get21 = hash;
  return _get20 ^ ((u32)(_get21) >> 16);
}

static void dong_porf_porf_todo__Porffor_object_writeKey(i32 ptr, i32 ptrjjtype, i32 key, i32 keyjjtype, i32 hash, i32 hashjjtype) {
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 keyEnc = 0;

  _get0 = ptr;
  _get1 = hash;
  i32_store(0, 0, _get0, _get1);
  _get2 = key;
  keyEnc = _get2;
  _get3 = keyjjtype;
  // if 
    if (_get3 == 67) {
      _get4 = keyEnc;
      keyEnc = _get4 | 2147483648;
    }
  // end
  j93:;
  _get5 = ptr;
  _get6 = keyEnc;
  i32_store(0, 4, _get5, _get6);
  return;
}

static void dong_porf_porf_todo__Porffor_object_fastAdd(i32 obj, i32 objjjtype, i32 key, i32 keyjjtype, f64 value, i32 valuejjtype, i32 flags, i32 flagsjjtype) {
  i32 _get14;
  i32 _get13;
  i32 _get12;
  f64 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 size = 0;
  i32 entryPtr = 0;

  _get0 = obj;
  size = i32_load16_u(0, 0, _get0);
  _get1 = obj;
  _get2 = size;
  i32_store16(0, 0, _get1, _get2 + 1);
  _get3 = obj;
  _get4 = size;
  entryPtr = (_get3 + 8) + (_get4 * 18);
  _get5 = entryPtr;
  _get6 = key;
  _get7 = keyjjtype;
  _get8 = key;
  _get9 = keyjjtype;
  dong_porf_porf_todo__Porffor_object_writeKey(_get5, 1, _get6, _get7, dong_porf_porf_todo__Porffor_object_hash(_get8, _get9), 1);
  _get10 = entryPtr;
  _get11 = value;
  f64_store(0, 8, _get10, _get11);
  _get12 = entryPtr;
  _get13 = flags;
  _get14 = valuejjtype;
  i32_store16(0, 16, _get12, _get13 + (_get14 << 8));
  return;
}

static i32 dong_porf_porf_todo__Porffor_funcLut_flags(i32 l0) {
  i32 _get0;
  _get0 = l0;
  return i32_load8_u(0, 16384, (_get0 * 57) + 2);
}

static f64 dong_porf_porf_todo__ecma262_IsConstructor(f64 argument, i32 argumentjjtype) {
  f64 _get1;
  i32 _get0;
  _get0 = argumentjjtype;
  // if 
    if ((f64)(_get0) != 6) {
      return 0;
    }
  // end
  j94:;
  _get1 = argument;
  return (f64)((f64)(dong_porf_porf_todo__Porffor_funcLut_flags((i32)(_get1)) & 2) == 2);
}

static struct ReturnValue dong_porf_porf_todo__Porffor_object_underlying(f64 _obj, i32 _objjjtype) {
  i32 _get84;
  f64 _get83;
  i32 _get82;
  i32 _get81;
  i32 _get80;
  f64 _get79;
  i32 _get78;
  i32 _get77;
  i32 _get76;
  i32 _get75;
  i32 _get74;
  i32 _get73;
  i32 _get72;
  i32 _get71;
  i32 _get70;
  i32 _get69;
  i32 _get68;
  i32 _get67;
  i32 _get66;
  i32 _get65;
  i32 _get64;
  i32 _get63;
  i32 _get62;
  i32 _get61;
  i32 _get60;
  i32 _get59;
  i32 _get58;
  i32 _get57;
  i32 _get56;
  i32 _get55;
  i32 _get54;
  i32 _get53;
  i32 _get52;
  i32 _get51;
  i32 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  f64 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  f64 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  i32 underlyingLength = 0;
  i32 end = 0;
  i32 i = 0;
  i32 obj = 0;
  i32 objjjtype = 0;
  i32 underlying = 0;
  i32 proto = 0;
  i32 len = 0;
  i32 ptr = 0;
  f64 x = 0;
  i32 xjjtype = 0;
  i32 jjlast_type = 0;
  i32 jjmember_obj_0 = 0;
  i32 jjmember_prop_0 = 0;
  i32 jjmember_allocd = 0;
  i32 jjmember_obj_1 = 0;
  i32 jjmember_prop_1 = 0;

  _get0 = _objjjtype;
  // if 
    if (_get0 == 7) {
      _get1 = _obj;
      return (struct ReturnValue){ (u32)(_get1), 7 };
    }
  // end
  j82:;
  _get2 = _objjjtype;
  // if 
    if (_get2 > 5) {
      // if 
        if ((dong_porf_porf_todo_jjporfjjunderlyingStorejjglbl_inited) == 0) {
          dong_porf_porf_todo_jjporfjjunderlyingStore = 0;
          dong_porf_porf_todo_jjporfjjunderlyingStorejjglbl_inited = 1;
        }
      // end
      j84:;
      // if 
        if ((dong_porf_porf_todo_jjporfjjunderlyingStore) == 0) {
          dong_porf_porf_todo_jjporfjjunderlyingStore = dong_porf_porf_todo__Porffor_malloc(16384);
        }
      // end
      j85:;
      underlyingLength = i32_load(0, 0, dong_porf_porf_todo_jjporfjjunderlyingStore);
      _get3 = underlyingLength;
      end = _get3 * 12;
      i = 0;
      // loop 
      j86:;
        _get4 = i;
        _get5 = end;
        // if 
          if (_get4 < _get5) {
            _get6 = i;
            _get7 = _obj;
            // if 
              if (f64_load(0, 4, dong_porf_porf_todo_jjporfjjunderlyingStore + _get6) == _get7) {
                _get8 = i;
                return (struct ReturnValue){ i32_load(0, 12, dong_porf_porf_todo_jjporfjjunderlyingStore + _get8), 7 };
              }
            // end
            j88:;
            _get9 = i;
            i = _get9 + 12;
            goto j86;
          }
        // end
        j87:;
      // end
      _get10 = _obj;
      obj = (u32)(_get10);
      underlying = dong_porf_porf_todo__Porffor_malloc(16384);
      _get11 = _objjjtype;
      // if 
        if (_get11 == 6) {
          _get12 = underlying;
          _get13 = obj;
          dong_porf_porf_todo__Porffor_object_fastAdd(_get12, 7, 564, 195, (f64)(dong_porf_porf_todo__Porffor_funcLut_length(_get13)), 1, 2, 1);
          _get14 = underlying;
          _get15 = obj;
          dong_porf_porf_todo__Porffor_object_fastAdd(_get14, 7, 576, 195, (f64)(dong_porf_porf_todo__Porffor_funcLut_name(_get15)), 195, 2, 1);
          _get16 = _obj;
          _get17 = _objjjtype;
          // if 
            if (((i32)(dong_porf_porf_todo__ecma262_IsConstructor(_get16, _get17))) != 0) {
              proto = dong_porf_porf_todo__Porffor_malloc(16384);
              _get18 = underlying;
              _get19 = proto;
              dong_porf_porf_todo__Porffor_object_fastAdd(_get18, 7, 586, 195, (f64)(_get19), 7, 8, 1);
              _get20 = proto;
              _get21 = _obj;
              _get22 = _objjjtype;
              dong_porf_porf_todo__Porffor_object_fastAdd(_get20, 7, 601, 195, _get21, _get22, 10, 1);
            }
          // end
          j95:;
        }
      // end
      j89:;
      _get23 = _objjjtype;
      // if 
        if (_get23 == 72) {
          _get24 = obj;
          len = i32_load(0, 0, _get24);
          _get25 = underlying;
          _get26 = len;
          dong_porf_porf_todo__Porffor_object_fastAdd(_get25, 7, 564, 195, (f64)(_get26), 1, 8, 1);
          i = 0;
          // loop 
          j97:;
            _get27 = i;
            _get28 = len;
            // if 
              if (_get27 < _get28) {
                _get29 = obj;
                _get30 = i;
                ptr = _get29 + (_get30 * 9);
                _get31 = ptr;
                x = f64_load(0, 4, _get31);
                _get32 = ptr;
                xjjtype = i32_load8_u(0, 12, _get32);
                _get33 = underlying;
                _get34 = i;
                const struct ReturnValue _0 = dong_porf_porf_todo__Number_prototype_toString((f64)(_get34), 1, 0, 0);
                jjlast_type = _0.type;
                _get35 = jjlast_type;
                _get36 = x;
                _get37 = xjjtype;
                dong_porf_porf_todo__Porffor_object_fastAdd(_get33, 7, (i32)(_0.value), _get35, _get36, _get37, 14, 1);
                _get38 = i;
                i = _get38 + 1;
                goto j97;
              }
            // end
            j98:;
          // end
        }
      // end
      j96:;
      _get39 = _objjjtype;
      _get40 = _objjjtype;
      // if 
        if (((_get39 == 67) | (_get40 == 33)) != 0) {
          _get41 = obj;
          len = i32_load(1, 0, _get41);
          _get42 = underlying;
          _get43 = len;
          dong_porf_porf_todo__Porffor_object_fastAdd(_get42, 7, 564, 195, (f64)(_get43), 1, 0, 1);
          i = 0;
          // loop 
          j100:;
            _get44 = i;
            _get45 = len;
            // if 
              if (_get44 < _get45) {
                _get46 = underlying;
                _get47 = i;
                const struct ReturnValue _1 = dong_porf_porf_todo__Number_prototype_toString((f64)(_get47), 1, 0, 0);
                jjlast_type = _1.type;
                _get48 = jjlast_type;
                _get49 = i;
                jjmember_prop_0 = _get49;
                _get50 = obj;
                jjmember_obj_0 = _get50;
                jjmember_allocd = dong_porf_porf_todo__Porffor_malloc(8);
                _get51 = jjmember_allocd;
                i32_store(0, 0, _get51, 1);
                _get52 = jjmember_allocd;
                _get53 = jjmember_prop_0;
                _get54 = jjmember_obj_0;
                i32_store16(0, 4, _get52, i32_load16_u(0, 4, (_get53 * 2) + _get54));
                _get55 = jjmember_allocd;
                jjlast_type = 67;
                dong_porf_porf_todo__Porffor_object_fastAdd(_get46, 7, (i32)(_1.value), _get48, (f64)(_get55), 67, 4, 1);
                _get56 = i;
                i = _get56 + 1;
                goto j100;
              }
            // end
            j101:;
          // end
          _get57 = _objjjtype;
          // if 
            if (_get57 == 67) {
              _get58 = obj;
              i32_store8(0, 2, _get58, 1);
            }
          // end
          j102:;
        }
      // end
      j99:;
      _get59 = _objjjtype;
      // if 
        if (_get59 == 195) {
          _get60 = obj;
          len = i32_load(1, 0, _get60);
          _get61 = underlying;
          _get62 = len;
          dong_porf_porf_todo__Porffor_object_fastAdd(_get61, 7, 564, 195, (f64)(_get62), 1, 0, 1);
          i = 0;
          // loop 
          j104:;
            _get63 = i;
            _get64 = len;
            // if 
              if (_get63 < _get64) {
                _get65 = underlying;
                _get66 = i;
                const struct ReturnValue _2 = dong_porf_porf_todo__Number_prototype_toString((f64)(_get66), 1, 0, 0);
                jjlast_type = _2.type;
                _get67 = jjlast_type;
                _get68 = i;
                jjmember_prop_1 = _get68;
                _get69 = obj;
                jjmember_obj_1 = _get69;
                jjmember_allocd = dong_porf_porf_todo__Porffor_malloc(8);
                _get70 = jjmember_allocd;
                i32_store(0, 0, _get70, 1);
                _get71 = jjmember_allocd;
                _get72 = jjmember_prop_1;
                _get73 = jjmember_obj_1;
                i32_store8(0, 4, _get71, i32_load8_u(0, 4, _get72 + _get73));
                _get74 = jjmember_allocd;
                jjlast_type = 195;
                dong_porf_porf_todo__Porffor_object_fastAdd(_get65, 7, (i32)(_2.value), _get67, (f64)(_get74), 195, 4, 1);
                _get75 = i;
                i = _get75 + 1;
                goto j104;
              }
            // end
            j105:;
          // end
          _get76 = obj;
          i32_store8(0, 2, _get76, 1);
        }
      // end
      j103:;
      _get77 = underlyingLength;
      i32_store(0, 0, dong_porf_porf_todo_jjporfjjunderlyingStore, _get77 + 1);
      _get78 = underlyingLength;
      _get79 = _obj;
      f64_store(0, 4, dong_porf_porf_todo_jjporfjjunderlyingStore + (_get78 * 12), _get79);
      _get80 = underlyingLength;
      _get81 = underlying;
      i32_store(0, 12, dong_porf_porf_todo_jjporfjjunderlyingStore + (_get80 * 12), _get81);
      _get82 = underlying;
      return (struct ReturnValue){ _get82, 7 };
    }
  // end
  j83:;
  _get83 = _obj;
  _get84 = _objjjtype;
  return (struct ReturnValue){ (u32)(_get83), _get84 };
}

static i32 dong_porf_porf_todo__Porffor_object_lookup(i32 obj, i32 objjjtype, i32 target, i32 targetjjtype, i32 targetHash, i32 targetHashjjtype) {
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 ptr = 0;
  i32 endPtr = 0;

  _get0 = obj;
  // if 
    if ((_get0) == 0) {
      return -1;
    }
  // end
  j107:;
  _get1 = obj;
  ptr = _get1 + 8;
  _get2 = ptr;
  _get3 = obj;
  endPtr = _get2 + (i32_load16_u(0, 0, _get3) * 18);
  // loop 
  j108:;
    _get4 = ptr;
    _get5 = endPtr;
    // if 
      if (_get4 < _get5) {
        _get6 = ptr;
        _get7 = targetHash;
        // if 
          if (i32_load(0, 0, _get6) == _get7) {
            _get8 = ptr;
            return _get8;
          }
        // end
        j110:;
        _get9 = ptr;
        ptr = _get9 + 18;
        goto j108;
      }
    // end
    j109:;
  // end
  return -1;
}

static i32 dong_porf_porf_todo__Porffor_object_isObjectOrNull(i32 arg, i32 argjjtype) {
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 t = 0;

  _get0 = argjjtype;
  t = _get0;
  _get1 = t;
  _get2 = t;
  _get3 = t;
  return ((_get1 > 5) & (_get2 != 67)) & (_get3 != 195);
}

static void dong_porf_porf_todo__Porffor_object_setPrototype(i32 obj, i32 objjjtype, i32 proto, i32 protojjtype) {
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 jjlast_type = 0;

  _get0 = objjjtype;
  // if 
    if (_get0 != 7) {
      _get1 = obj;
      _get2 = objjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__Porffor_object_underlying((f64)(_get1), _get2);
      jjlast_type = _0.type;
      _get3 = jjlast_type;
      objjjtype = _get3;
      obj = _0.value;
      _get4 = objjjtype;
      // if 
        if (_get4 != 7) {
          return;
        }
      // end
      j116:;
    }
  // end
  j115:;
  _get5 = proto;
  _get6 = protojjtype;
  // if 
    if ((dong_porf_porf_todo__Porffor_object_isObjectOrNull(_get5, _get6)) != 0) {
      _get7 = obj;
      _get8 = proto;
      i32_store(0, 4, _get7, _get8);
      _get9 = obj;
      _get10 = protojjtype;
      i32_store8(0, 3, _get9, _get10);
    }
  // end
  j117:;
  return;
}

static i32 dong_porf_porf_todo_jjget___Object_prototype() {
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 l0 = 0;

  // if 
    if ((dong_porf_porf_todo_jjporfjjgetptr___Object_prototype) != 0) {
      return dong_porf_porf_todo_jjporfjjgetptr___Object_prototype;
    }
  // end
  j114:;
  l0 = 49152;
  _get0 = l0;
  dong_porf_porf_todo_jjporfjjgetptr___Object_prototype = _get0;
  _get1 = l0;
  const struct ReturnValue _0 = dong_porf_porf_todo__Porffor_object_underlying((f64)(_get1), 7);
  (void) _0.type;
  l0 = _0.value;
  _get2 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get2, 7, 651, 195, 2, 6, 10, 1);
  _get3 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get3, 7, 939, 195, 3, 6, 10, 1);
  _get4 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get4, 7, 965, 195, 4, 6, 10, 1);
  _get5 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get5, 7, 550, 195, 5, 6, 10, 1);
  _get6 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get6, 7, 1022, 195, 6, 6, 10, 1);
  _get7 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get7, 7, 1042, 195, 7, 6, 10, 1);
  _get8 = l0;
  dong_porf_porf_todo__Porffor_object_setPrototype(_get8, 7, 0, 7);
  _get9 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get9, 7, 601, 195, 8, 6, 10, 1);
  return dong_porf_porf_todo_jjporfjjgetptr___Object_prototype;
}

static struct ReturnValue dong_porf_porf_todo__Porffor_object_getHiddenPrototype(i32 trueType, i32 trueTypejjtype) {
  return (struct ReturnValue){ dong_porf_porf_todo_jjget___Object_prototype(), 7 };
}

static struct ReturnValue dong_porf_porf_todo__Porffor_object_getPrototype(i32 obj, i32 objjjtype) {
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 jjlast_type = 0;

  _get0 = objjjtype;
  // if 
    if (_get0 != 7) {
      _get1 = obj;
      _get2 = objjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__Porffor_object_underlying((f64)(_get1), _get2);
      jjlast_type = _0.type;
      _get3 = jjlast_type;
      objjjtype = _get3;
      obj = _0.value;
      _get4 = objjjtype;
      // if 
        if (_get4 != 7) {
          return (struct ReturnValue){ 0, 0 };
        }
      // end
      j125:;
    }
  // end
  j124:;
  _get5 = obj;
  _get6 = obj;
  return (struct ReturnValue){ i32_load(0, 4, _get5), i32_load8_u(0, 3, _get6) };
}

static struct ReturnValue dong_porf_porf_todo__Porffor_object_accessorGet(i32 entryPtr, i32 entryPtrjjtype) {
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 out = 0;

  _get0 = entryPtr;
  out = i32_load(0, 8, _get0);
  _get1 = out;
  // if 
    if ((_get1) == 0) {
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j133:;
  _get2 = out;
  return (struct ReturnValue){ _get2, 6 };
}

static struct ReturnValue dong_porf_porf_todo__Porffor_object_get_withHash(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype, i32 hash, i32 hashjjtype) {
  i32 _get67;
  i32 _get66;
  i32 _get65;
  i32 _get64;
  i32 _get63;
  i32 _get62;
  i32 _get61;
  i32 _get60;
  i32 _get59;
  i32 _get58;
  i32 _get57;
  i32 _get56;
  i32 _get55;
  i32 _get54;
  i32 _get53;
  i32 _get52;
  i32 _get51;
  i32 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 obj = 0;
  i32 objjjtype = 0;
  i32 trueType = 0;
  i32 jjlast_type = 0;
  i32 entryPtr = 0;
  i32 lastProto = 0;
  i32 lastProtojjtype = 0;
  i32 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 tail = 0;
  i32 get = 0;
  i32 jjcall_val = 0;
  i32 jjcall_type = 0;
  i32 jjindirect_3_callee = 0;
  i32 jjswap = 0;

  _get0 = _obj;
  obj = _get0;
  _get1 = _objjjtype;
  objjjtype = _get1;
  _get2 = objjjtype;
  trueType = _get2;
  _get3 = trueType;
  // if 
    if (_get3 != 7) {
      _get4 = obj;
      _get5 = objjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__Porffor_object_underlying((f64)(_get4), _get5);
      jjlast_type = _0.type;
      _get6 = jjlast_type;
      objjjtype = _get6;
      obj = _0.value;
    }
  // end
  j81:;
  _get7 = obj;
  // if 
    if ((_get7) == 0) {
    }
  // end
  j106:;
  _get8 = obj;
  _get9 = objjjtype;
  _get10 = key;
  _get11 = keyjjtype;
  _get12 = hash;
  entryPtr = dong_porf_porf_todo__Porffor_object_lookup(_get8, _get9, _get10, _get11, _get12, 1);
  _get13 = entryPtr;
  // if 
    if (_get13 == -1) {
      _get14 = trueType;
      // if 
        if (_get14 == 7) {
          _get15 = obj;
          _get16 = obj;
          obj = i32_load(0, 4, _get16);
          objjjtype = i32_load8_u(0, 3, _get15);
          _get17 = objjjtype;
          // if 
            if ((_get17) == 0) {
              obj = dong_porf_porf_todo_jjget___Object_prototype();
              objjjtype = 7;
            }
          // end
          j113:;
        } else {
          _get18 = trueType;
          const struct ReturnValue _1 = dong_porf_porf_todo__Porffor_object_getHiddenPrototype(_get18, 1);
          jjlast_type = _1.type;
          _get19 = jjlast_type;
          objjjtype = _get19;
          obj = _1.value;
        }
      // end
      j112:;
      _get20 = objjjtype;
      // if 
        if (_get20 != 7) {
          _get21 = obj;
          _get22 = objjjtype;
          const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_object_underlying((f64)(_get21), _get22);
          jjlast_type = _2.type;
          _get23 = jjlast_type;
          objjjtype = _get23;
          obj = _2.value;
        }
      // end
      j118:;
      _get24 = obj;
      lastProto = _get24;
      _get25 = objjjtype;
      lastProtojjtype = _get25;
      // loop 
      j119:;
        // if 
          if ((1) != 0) {
            _get26 = obj;
            _get27 = objjjtype;
            _get28 = key;
            _get29 = keyjjtype;
            _get30 = hash;
            entryPtr = dong_porf_porf_todo__Porffor_object_lookup(_get26, _get27, _get28, _get29, _get30, 1);
            _get31 = entryPtr;
            // if 
              if (_get31 != -1) {
                goto j120;
              }
            // end
            j121:;
            _get32 = objjjtype;
            // if 
              if (_get32 == 7) {
                _get33 = obj;
                _get34 = obj;
                obj = i32_load(0, 4, _get34);
                objjjtype = i32_load8_u(0, 3, _get33);
                _get35 = objjjtype;
                // if 
                  if ((_get35) == 0) {
                    obj = dong_porf_porf_todo_jjget___Object_prototype();
                    objjjtype = 7;
                  }
                // end
                j123:;
              } else {
                _get36 = obj;
                _get37 = objjjtype;
                const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_object_getPrototype(_get36, _get37);
                jjlast_type = _3.type;
                _get38 = jjlast_type;
                objjjtype = _get38;
                obj = _3.value;
              }
            // end
            j122:;
            _get39 = objjjtype;
            // if 
              if (_get39 != 7) {
                _get40 = obj;
                _get41 = objjjtype;
                const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_object_underlying((f64)(_get40), _get41);
                jjlast_type = _4.type;
                _get42 = jjlast_type;
                objjjtype = _get42;
                obj = _4.value;
              }
            // end
            j126:;
            _get43 = obj;
            jjlogicinner_tmp = _get43;
            _get44 = objjjtype;
            jjtypeswitch_tmp1 = _get44;
            // block i32
            i32 _r127;
              _get45 = jjtypeswitch_tmp1;
              // if 
                if ((_get45) == 0) {
                  _r127 = 1;
                  goto j127;
                }
              // end
              j128:;
              _get46 = jjtypeswitch_tmp1;
              // if 
                if (_get46 == 7) {
                  _get47 = jjlogicinner_tmp;
                  _r127 = (_get47) == 0;
                  goto j127;
                }
              // end
              j129:;
              _r127 = 0;
            // end
            j127:;
            _get48 = obj;
            _get49 = lastProto;
            // if 
              if ((_r127 | (_get48 == _get49)) != 0) {
                goto j120;
              }
            // end
            j130:;
            _get50 = obj;
            lastProto = _get50;
            _get51 = objjjtype;
            lastProtojjtype = _get51;
            goto j119;
          }
        // end
        j120:;
      // end
      _get52 = entryPtr;
      // if 
        if (_get52 == -1) {
          return (struct ReturnValue){ 0, 0 };
        }
      // end
      j131:;
    }
  // end
  j111:;
  _get53 = entryPtr;
  tail = i32_load16_u(0, 16, _get53);
  _get54 = tail;
  // if 
    if ((_get54 & 1) != 0) {
      _get55 = entryPtr;
      const struct ReturnValue _5 = dong_porf_porf_todo__Porffor_object_accessorGet(_get55, 1);
      jjlast_type = _5.type;
      get = _5.value;
      _get56 = get;
      // if 
        if ((_get56) == 0) {
          return (struct ReturnValue){ 0, 0 };
        }
      // end
      j134:;
      _get57 = get;
      jjindirect_3_callee = _get57;
      jjswap = 0;
      _get58 = jjswap;
      _get59 = _obj;
      jjcall_val = _get59;
      _get60 = jjcall_val;
      _get61 = _objjjtype;
      jjcall_type = _get61;
      _get62 = jjcall_type;
      jjswap = _get62;
      _get63 = jjswap;
      _get64 = jjindirect_3_callee;
      jjlast_type = 0;
      _get65 = jjlast_type;
      return (struct ReturnValue){ 0, _get65 };
      (void) 0;
    }
  // end
  j132:;
  _get66 = entryPtr;
  _get67 = tail;
  return (struct ReturnValue){ f64_load(0, 8, _get66), (u32)(_get67) >> 8 };
}

static struct ReturnValue dong_porf_porf_todo__Porffor_object_readValue(i32 entryPtr, i32 entryPtrjjtype) {
  i32 _get1;
  i32 _get0;
  _get0 = entryPtr;
  _get1 = entryPtr;
  return (struct ReturnValue){ f64_load(0, 8, _get0), i32_load8_u(0, 17, _get1) };
}

static struct ReturnValue dong_porf_porf_todo__Object_prototype_toString(f64 _this, i32 _thisjjtype) {
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  f64 _get32;
  i32 _get31;
  f64 _get30;
  i32 _get29;
  i32 _get28;
  f64 _get27;
  f64 _get26;
  i32 _get25;
  f64 _get24;
  i32 _get23;
  i32 _get22;
  f64 _get21;
  f64 _get20;
  f64 _get19;
  i32 _get18;
  f64 _get17;
  i32 _get16;
  i32 _get15;
  f64 _get14;
  f64 _get13;
  i32 _get12;
  f64 _get11;
  i32 _get10;
  f64 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  i32 _get0;
  f64 obj = 0;
  f64 ovr = 0;
  i32 ovrjjtype = 0;
  f64 jjmember_obj_254 = 0;
  f64 jjmember_prop_254 = 0;
  i32 jjlast_type = 0;
  i32 logictmpi = 0;
  f64 jjcall_val = 0;
  i32 jjcall_type = 0;
  f64 jjindirect_255_callee = 0;
  f64 entryPtr = 0;
  f64 jjindirect_256_callee = 0;

  _get0 = _thisjjtype;
  // if 
    if ((f64)(_get0) == 7) {
      _get1 = _this;
      obj = _get1;
      _get2 = obj;
      // if 
        if (!(_get2 == 0)) {
          jjmember_prop_254 = 550;
          _get3 = obj;
          jjmember_obj_254 = _get3;
          _get4 = jjmember_obj_254;
          _get5 = jjmember_prop_254;
          const struct ReturnValue _0 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get4), 7, (u32)(_get5), 195, -2133638001, 1);
          jjlast_type = _0.type;
          _get6 = jjlast_type;
          ovrjjtype = _get6;
          ovr = _0.value;
          _get7 = ovrjjtype;
          logictmpi = (f64)(_get7) == 6;
          _get8 = logictmpi;
          // if i32
          i32 _r135;
            if ((_get8) != 0) {
              _get9 = ovr;
              jjlast_type = 2;
              _r135 = _get9 != 5;
            } else {
              _get10 = logictmpi;
              jjlast_type = 2;
              _r135 = _get10;
            }
          // end
          j135:;
          // if 
            if ((_r135) != 0) {
              _get11 = ovr;
              jjindirect_255_callee = _get11;
              _get12 = ovrjjtype;
              // if f64
              f64 _r137;
                if (_get12 == 6) {
                  _get13 = _this;
                  jjcall_val = _get13;
                  _get14 = jjcall_val;
                  _get15 = _thisjjtype;
                  jjcall_type = _get15;
                  _get16 = jjcall_type;
                  _get17 = jjindirect_255_callee;
                  jjlast_type = 0;
                  _r137 = 0;
                } else {
                  _r137 = 0;
                }
              // end
              j137:;
              _get18 = jjlast_type;
              return (struct ReturnValue){ _r137, _get18 };
              (void) 0;
            }
          // end
          j136:;
          _get19 = obj;
          entryPtr = (f64)(dong_porf_porf_todo__Porffor_object_lookup((i32)(_get19), 7, 550, 195, dong_porf_porf_todo__Porffor_object_hash(550, 195), 1));
          _get20 = entryPtr;
          // if 
            if (_get20 != -1) {
              _get21 = entryPtr;
              const struct ReturnValue _1 = dong_porf_porf_todo__Porffor_object_readValue((i32)(_get21), 1);
              jjlast_type = _1.type;
              _get22 = jjlast_type;
              ovrjjtype = _get22;
              ovr = _1.value;
              _get23 = ovrjjtype;
              // if 
                if ((f64)(_get23) == 6) {
                  _get24 = ovr;
                  jjindirect_256_callee = _get24;
                  _get25 = ovrjjtype;
                  // if f64
                  f64 _r140;
                    if (_get25 == 6) {
                      _get26 = _this;
                      jjcall_val = _get26;
                      _get27 = jjcall_val;
                      _get28 = _thisjjtype;
                      jjcall_type = _get28;
                      _get29 = jjcall_type;
                      _get30 = jjindirect_256_callee;
                      jjlast_type = 0;
                      _r140 = 0;
                    } else {
                      _r140 = 0;
                    }
                  // end
                  j140:;
                  _get31 = jjlast_type;
                  return (struct ReturnValue){ _r140, _get31 };
                  (void) 0;
                } else {
                  return (struct ReturnValue){ 0, 0 };
                  (void) 0;
                }
              // end
              j139:;
            }
          // end
          j138:;
        }
      // end
      j80:;
    }
  // end
  j79:;
  _get32 = _this;
  _get33 = _thisjjtype;
  // if 
    if (((_get32 == 0) & ((_get33 | 128) == (0 | 128))) != 0) {
      return (struct ReturnValue){ 1441, 195 };
      (void) 0;
    }
  // end
  j141:;
  _get34 = _this;
  _get35 = _thisjjtype;
  // if 
    if (((_get34 == 0) & ((_get35 | 128) == (7 | 128))) != 0) {
      return (struct ReturnValue){ 1465, 195 };
      (void) 0;
    }
  // end
  j142:;
  _get36 = _thisjjtype;
  // if 
    if ((f64)(_get36) == 72) {
      return (struct ReturnValue){ 1484, 195 };
      (void) 0;
    }
  // end
  j143:;
  _get37 = _thisjjtype;
  // if 
    if ((f64)(_get37) == 6) {
      return (struct ReturnValue){ 1504, 195 };
      (void) 0;
    }
  // end
  j144:;
  _get38 = _thisjjtype;
  _get39 = _thisjjtype;
  // if 
    if ((((f64)(_get38) == 2) | ((f64)(_get39) == 31)) != 0) {
      return (struct ReturnValue){ 1527, 195 };
      (void) 0;
    }
  // end
  j145:;
  _get40 = _thisjjtype;
  _get41 = _thisjjtype;
  // if 
    if ((((f64)(_get40) == 1) | ((f64)(_get41) == 32)) != 0) {
      return (struct ReturnValue){ 1549, 195 };
      (void) 0;
    }
  // end
  j146:;
  _get42 = _thisjjtype;
  _get43 = _thisjjtype;
  // if 
    if ((((f64)(_get42 | 128) == 195) | ((f64)(_get43) == 33)) != 0) {
      return (struct ReturnValue){ 1570, 195 };
      (void) 0;
    }
  // end
  j147:;
  _get44 = _thisjjtype;
  // if 
    if ((f64)(_get44) == 10) {
      return (struct ReturnValue){ 1591, 195 };
      (void) 0;
    }
  // end
  j148:;
  _get45 = _thisjjtype;
  // if 
    if ((f64)(_get45) == 9) {
      return (struct ReturnValue){ 1610, 195 };
      (void) 0;
    }
  // end
  j149:;
  return (struct ReturnValue){ 1631, 195 };
}

static i32 dong_porf_porf_todo__Porffor_bytestringToString(i32 src) {
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 len = 0;
  i32 counter = 0;
  i32 dst = 0;

  _get0 = src;
  len = i32_load(0, 0, _get0);
  _get1 = len;
  dst = dong_porf_porf_todo__Porffor_malloc((_get1 * 2) + 6);
  _get2 = dst;
  _get3 = len;
  i32_store(0, 0, _get2, _get3);
  // loop 
  j156:;
    _get4 = counter;
    _get5 = dst;
    _get6 = src;
    _get7 = counter;
    i32_store16(0, 4, (_get4 * 2) + _get5, i32_load8_u(0, 4, _get6 + _get7));
    _get8 = counter;
    counter = _get8 + 1;
    _get9 = counter;
    _get10 = len;
    if (_get9 < _get10) {
      goto j156;
    }
  // end
  _get11 = dst;
  return _get11;
}

static struct ReturnValue dong_porf_porf_todo__String_prototype_toString(i32 _this, i32 _thisjjtype) {
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  _get0 = _thisjjtype;
  // if 
    if (_get0 != 67) {
      _get1 = _thisjjtype;
      _get2 = _thisjjtype;
      _get3 = _this;
      // if 
        if ((((_get1) == 0) | ((_get2 == 7) & ((_get3) == 0))) != 0) {
        }
      // end
      j154:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_porf_todo__Porffor_bytestringToString(_get7);
        }
      // end
      j155:;
    }
  // end
  j153:;
  _get8 = _this;
  return (struct ReturnValue){ _get8, 67 };
}

static struct ReturnValue dong_porf_porf_todo__TypeError_prototype_namekkget(f64 _this, i32 _thisjjtype) {
  return (struct ReturnValue){ 511, 195 };
}

static struct ReturnValue dong_porf_porf_todo__TypeError_prototype_messagekkget(f64 _this, i32 _thisjjtype) {
  f64 _get1;
  f64 _get0;
  _get0 = _this;
  _get1 = _this;
  return (struct ReturnValue){ (f64)(i32_load(0, 0, (u32)(_get0))), i32_load8_u(0, 4, (u32)(_get1)) };
}

static struct ReturnValue dong_porf_porf_todo__Porffor_strcat(i32 a, i32 ajjtype, i32 b, i32 bjjtype) {
  i32 _get65;
  i32 _get64;
  i32 _get63;
  i32 _get62;
  i32 _get61;
  i32 _get60;
  i32 _get59;
  i32 _get58;
  i32 _get57;
  i32 _get56;
  i32 _get55;
  i32 _get54;
  i32 _get53;
  i32 _get52;
  i32 _get51;
  i32 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 al = 0;
  i32 bl = 0;
  i32 out = 0;
  i32 i = 0;
  i32 ptr = 0;

  _get0 = a;
  al = i32_load(0, 0, _get0);
  _get1 = b;
  bl = i32_load(0, 0, _get1);
  _get2 = ajjtype;
  // if 
    if (_get2 == 195) {
      _get3 = bjjtype;
      // if 
        if (_get3 == 195) {
          _get4 = al;
          _get5 = bl;
          out = dong_porf_porf_todo__Porffor_malloc((6 + _get4) + _get5);
          _get6 = out;
          _get7 = al;
          _get8 = bl;
          i32_store(0, 0, _get6, _get7 + _get8);
          _get9 = out;
          _get10 = a;
          _get11 = al;
          memcpy(dong_porf_porf_todo_memory + (_get9 + 4), dong_porf_porf_todo_memory + (_get10 + 4), _get11);
          _get12 = out;
          _get13 = al;
          _get14 = b;
          _get15 = bl;
          memcpy(dong_porf_porf_todo_memory + ((_get12 + 4) + _get13), dong_porf_porf_todo_memory + (_get14 + 4), _get15);
          _get16 = out;
          return (struct ReturnValue){ _get16, 195 };
        } else {
          _get17 = al;
          _get18 = bl;
          out = dong_porf_porf_todo__Porffor_malloc(6 + ((_get17 + _get18) * 2));
          _get19 = out;
          _get20 = al;
          _get21 = bl;
          i32_store(0, 0, _get19, _get20 + _get21);
          i = 0;
          // loop 
          j163:;
            _get22 = i;
            _get23 = al;
            // if 
              if (_get22 < _get23) {
                _get24 = out;
                _get25 = i;
                _get26 = a;
                _get27 = i;
                i32_store16(0, 4, _get24 + (_get25 * 2), i32_load8_u(0, 4, _get26 + _get27));
                _get28 = i;
                i = _get28 + 1;
                goto j163;
              }
            // end
            j164:;
          // end
          _get29 = out;
          _get30 = al;
          _get31 = b;
          _get32 = bl;
          memcpy(dong_porf_porf_todo_memory + ((_get29 + 4) + (_get30 * 2)), dong_porf_porf_todo_memory + (_get31 + 4), (_get32 * 2));
          _get33 = out;
          return (struct ReturnValue){ _get33, 67 };
        }
      // end
      j162:;
    } else {
      _get34 = bjjtype;
      // if 
        if (_get34 == 195) {
          _get35 = al;
          _get36 = bl;
          out = dong_porf_porf_todo__Porffor_malloc(6 + ((_get35 + _get36) * 2));
          _get37 = out;
          _get38 = al;
          _get39 = bl;
          i32_store(0, 0, _get37, _get38 + _get39);
          _get40 = out;
          _get41 = a;
          _get42 = al;
          memcpy(dong_porf_porf_todo_memory + (_get40 + 4), dong_porf_porf_todo_memory + (_get41 + 4), (_get42 * 2));
          _get43 = out;
          _get44 = al;
          ptr = _get43 + (_get44 * 2);
          i = 0;
          // loop 
          j166:;
            _get45 = i;
            _get46 = bl;
            // if 
              if (_get45 < _get46) {
                _get47 = ptr;
                _get48 = i;
                _get49 = b;
                _get50 = i;
                i32_store16(0, 4, _get47 + (_get48 * 2), i32_load8_u(0, 4, _get49 + _get50));
                _get51 = i;
                i = _get51 + 1;
                goto j166;
              }
            // end
            j167:;
          // end
          _get52 = out;
          return (struct ReturnValue){ _get52, 67 };
        } else {
          _get53 = al;
          _get54 = bl;
          out = dong_porf_porf_todo__Porffor_malloc(6 + ((_get53 + _get54) * 2));
          _get55 = out;
          _get56 = al;
          _get57 = bl;
          i32_store(0, 0, _get55, _get56 + _get57);
          _get58 = out;
          _get59 = a;
          _get60 = al;
          memcpy(dong_porf_porf_todo_memory + (_get58 + 4), dong_porf_porf_todo_memory + (_get59 + 4), (_get60 * 2));
          _get61 = out;
          _get62 = al;
          _get63 = b;
          _get64 = bl;
          memcpy(dong_porf_porf_todo_memory + ((_get61 + 4) + (_get62 * 2)), dong_porf_porf_todo_memory + (_get63 + 4), (_get64 * 2));
          _get65 = out;
          return (struct ReturnValue){ _get65, 67 };
        }
      // end
      j165:;
    }
  // end
  j161:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo__Porffor_concatStrings(f64 a, i32 ajjtype, f64 b, i32 bjjtype) {
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  i32 jjlast_type = 0;

  _get0 = ajjtype;
  // if 
    if ((f64)(_get0 | 128) != 195) {
      _get1 = a;
      _get2 = ajjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString(_get1, _get2);
      jjlast_type = _0.type;
      _get3 = jjlast_type;
      ajjtype = _get3;
      a = _0.value;
    }
  // end
  j159:;
  _get4 = bjjtype;
  // if 
    if ((f64)(_get4 | 128) != 195) {
      _get5 = b;
      _get6 = bjjtype;
      const struct ReturnValue _1 = dong_porf_porf_todo__ecma262_ToString(_get5, _get6);
      jjlast_type = _1.type;
      _get7 = jjlast_type;
      bjjtype = _get7;
      b = _1.value;
    }
  // end
  j160:;
  _get8 = a;
  _get9 = ajjtype;
  _get10 = b;
  _get11 = bjjtype;
  const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_strcat((i32)(_get8), _get9, (i32)(_get10), _get11);
  jjlast_type = _2.type;
  _get12 = jjlast_type;
  return (struct ReturnValue){ (f64)(_2.value), _get12 };
}

static struct ReturnValue dong_porf_porf_todo__TypeError_prototype_toString(f64 _this, i32 _thisjjtype) {
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  f64 _get4;
  i32 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  f64 name = 0;
  i32 namejjtype = 0;
  f64 jjmember_obj_144 = 0;
  f64 jjmember_prop_144 = 0;
  i32 jjlast_type = 0;
  f64 message = 0;
  i32 messagejjtype = 0;
  f64 jjmember_obj_145 = 0;
  f64 jjmember_prop_145 = 0;

  _get0 = _this;
  const struct ReturnValue _0 = dong_porf_porf_todo__TypeError_prototype_namekkget(_get0, 38);
  jjlast_type = _0.type;
  _get1 = jjlast_type;
  namejjtype = _get1;
  name = _0.value;
  _get2 = _this;
  const struct ReturnValue _1 = dong_porf_porf_todo__TypeError_prototype_messagekkget(_get2, 38);
  jjlast_type = _1.type;
  _get3 = jjlast_type;
  messagejjtype = _get3;
  message = _1.value;
  _get4 = message;
  // if 
    if ((f64)(i32_load(1, 0, (u32)(_get4))) == 0) {
      _get5 = name;
      _get6 = namejjtype;
      return (struct ReturnValue){ _get5, _get6 };
    }
  // end
  j158:;
  _get7 = name;
  _get8 = namejjtype;
  const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_concatStrings(_get7, _get8, 526, 195);
  jjlast_type = _2.type;
  _get9 = jjlast_type;
  _get10 = message;
  _get11 = messagejjtype;
  const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_concatStrings(_2.value, _get9, _get10, _get11);
  jjlast_type = _3.type;
  _get12 = jjlast_type;
  return (struct ReturnValue){ _3.value, _get12 };
}

static struct ReturnValue dong_porf_porf_todo__RangeError_prototype_namekkget(f64 _this, i32 _thisjjtype) {
  return (struct ReturnValue){ 534, 195 };
}

static struct ReturnValue dong_porf_porf_todo__RangeError_prototype_messagekkget(f64 _this, i32 _thisjjtype) {
  f64 _get1;
  f64 _get0;
  _get0 = _this;
  _get1 = _this;
  return (struct ReturnValue){ (f64)(i32_load(0, 0, (u32)(_get0))), i32_load8_u(0, 4, (u32)(_get1)) };
}

static struct ReturnValue dong_porf_porf_todo__RangeError_prototype_toString(f64 _this, i32 _thisjjtype) {
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  f64 _get4;
  i32 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  f64 name = 0;
  i32 namejjtype = 0;
  f64 jjmember_obj_150 = 0;
  f64 jjmember_prop_150 = 0;
  i32 jjlast_type = 0;
  f64 message = 0;
  i32 messagejjtype = 0;
  f64 jjmember_obj_151 = 0;
  f64 jjmember_prop_151 = 0;

  _get0 = _this;
  const struct ReturnValue _0 = dong_porf_porf_todo__RangeError_prototype_namekkget(_get0, 41);
  jjlast_type = _0.type;
  _get1 = jjlast_type;
  namejjtype = _get1;
  name = _0.value;
  _get2 = _this;
  const struct ReturnValue _1 = dong_porf_porf_todo__RangeError_prototype_messagekkget(_get2, 41);
  jjlast_type = _1.type;
  _get3 = jjlast_type;
  messagejjtype = _get3;
  message = _1.value;
  _get4 = message;
  // if 
    if ((f64)(i32_load(1, 0, (u32)(_get4))) == 0) {
      _get5 = name;
      _get6 = namejjtype;
      return (struct ReturnValue){ _get5, _get6 };
    }
  // end
  j169:;
  _get7 = name;
  _get8 = namejjtype;
  const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_concatStrings(_get7, _get8, 526, 195);
  jjlast_type = _2.type;
  _get9 = jjlast_type;
  _get10 = message;
  _get11 = messagejjtype;
  const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_concatStrings(_2.value, _get9, _get10, _get11);
  jjlast_type = _3.type;
  _get12 = jjlast_type;
  return (struct ReturnValue){ _3.value, _get12 };
}

static i32 dong_porf_porf_todo__Porffor_object_isObject(i32 arg, i32 argjjtype) {
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 t = 0;

  _get0 = argjjtype;
  t = _get0;
  _get1 = arg;
  _get2 = t;
  _get3 = t;
  _get4 = t;
  return (((_get1 != 0) & (_get2 > 5)) & (_get3 != 67)) & (_get4 != 195);
}

static struct ReturnValue dong_porf_porf_todo__ecma262_ToPropertyKey(f64 argument, i32 argumentjjtype) {
  i32 _get14;
  i32 _get13;
  f64 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  f64 _get0;
  f64 key = 0;
  i32 keyjjtype = 0;
  i32 logictmpi = 0;
  i32 jjlast_type = 0;

  _get0 = argument;
  key = _get0;
  _get1 = argumentjjtype;
  keyjjtype = _get1;
  _get2 = argumentjjtype;
  logictmpi = (f64)(_get2) == 7;
  _get3 = logictmpi;
  // if i32
  i32 _r198;
    if ((_get3) != 0) {
      _get4 = argument;
      jjlast_type = 2;
      _r198 = _get4 != 0;
    } else {
      _get5 = logictmpi;
      jjlast_type = 2;
      _r198 = _get5;
    }
  // end
  j198:;
  // if 
    if ((_r198) != 0) {
      _get6 = argument;
      _get7 = argumentjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToPrimitive_String(_get6, _get7);
      jjlast_type = _0.type;
      _get8 = jjlast_type;
      keyjjtype = _get8;
      key = _0.value;
    }
  // end
  j199:;
  _get9 = keyjjtype;
  // if 
    if ((f64)(_get9) == 5) {
      _get10 = key;
      _get11 = keyjjtype;
      return (struct ReturnValue){ _get10, _get11 };
    }
  // end
  j200:;
  _get12 = key;
  _get13 = keyjjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo__ecma262_ToString(_get12, _get13);
  jjlast_type = _1.type;
  _get14 = jjlast_type;
  return (struct ReturnValue){ _1.value, _get14 };
}

static i32 dong_porf_porf_todo__Porffor_strcmp(i32 a, i32 ajjtype, i32 b, i32 bjjtype) {
  i32 _get85;
  i32 _get84;
  i32 _get83;
  i32 _get82;
  i32 _get81;
  i32 _get80;
  i32 _get79;
  i32 _get78;
  i32 _get77;
  i32 _get76;
  i32 _get75;
  i32 _get74;
  i32 _get73;
  i32 _get72;
  i32 _get71;
  i32 _get70;
  i32 _get69;
  i32 _get68;
  i32 _get67;
  i32 _get66;
  i32 _get65;
  i32 _get64;
  i32 _get63;
  i32 _get62;
  i32 _get61;
  i32 _get60;
  i32 _get59;
  i32 _get58;
  i32 _get57;
  i32 _get56;
  i32 _get55;
  i32 _get54;
  i32 _get53;
  i32 _get52;
  i32 _get51;
  i32 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 al = 0;
  i32 bl = 0;
  i32 ap32 = 0;
  i32 bp32 = 0;
  i32 ap8 = 0;
  i32 bp8 = 0;
  i32 i = 0;

  _get0 = a;
  _get1 = b;
  // if 
    if (_get0 == _get1) {
      return 1;
    }
  // end
  j207:;
  _get2 = a;
  al = i32_load(0, 0, _get2);
  _get3 = b;
  bl = i32_load(0, 0, _get3);
  _get4 = al;
  _get5 = bl;
  // if 
    if (_get4 != _get5) {
      return 0;
    }
  // end
  j208:;
  _get6 = ajjtype;
  // if 
    if (_get6 == 195) {
      _get7 = bjjtype;
      // if 
        if (_get7 == 195) {
          _get8 = a;
          ap32 = _get8 - 28;
          _get9 = b;
          bp32 = _get9 - 28;
          _get10 = a;
          ap8 = _get10 - 4;
          _get11 = b;
          bp8 = _get11 - 4;
          _get12 = al;
          // if 
            if (_get12 >= 32) {
              // loop 
              j212:;
                _get13 = ap32;
                _get14 = al;
                _get15 = bp32;
                _get16 = al;
                _get17 = ap32;
                _get18 = al;
                _get19 = bp32;
                _get20 = al;
                // if 
                  if ((_get19 + _get20) != 0) {
                    return 0;
                  }
                // end
                j213:;
                _get21 = al;
                al = _get21 - 32;
                _get22 = al;
                if (_get22 >= 32) {
                  goto j212;
                }
              // end
            }
          // end
          j211:;
          _get23 = al;
          // if 
            if (_get23 >= 8) {
              // loop 
              j215:;
                _get24 = ap8;
                _get25 = al;
                _get26 = bp8;
                _get27 = al;
                // if 
                  if ((_get26 + _get27) != 0) {
                    return 0;
                  }
                // end
                j216:;
                _get28 = al;
                al = _get28 - 8;
                _get29 = al;
                if (_get29 >= 8) {
                  goto j215;
                }
              // end
            }
          // end
          j214:;
          _get30 = al;
          // if 
            if (_get30 >= 2) {
              // loop 
              j218:;
                _get31 = a;
                _get32 = al;
                _get33 = b;
                _get34 = al;
                // if 
                  if (i32_load16_u(0, 2, _get31 + _get32) != i32_load16_u(0, 2, _get33 + _get34)) {
                    return 0;
                  }
                // end
                j219:;
                _get35 = al;
                al = _get35 - 2;
                _get36 = al;
                if (_get36 >= 2) {
                  goto j218;
                }
              // end
            }
          // end
          j217:;
          _get37 = al;
          // if 
            if (_get37 == 1) {
              _get38 = a;
              _get39 = b;
              // if 
                if (i32_load8_u(0, 4, _get38) != i32_load8_u(0, 4, _get39)) {
                  return 0;
                  (void) (_get24 + _get25);
                }
              // end
              j221:;
            }
          // end
          j220:;
          return 1;
          (void) (_get17 + _get18);
        } else {
          i = 0;
          // loop 
          j222:;
            _get40 = i;
            _get41 = al;
            // if 
              if (_get40 < _get41) {
                _get42 = a;
                _get43 = i;
                _get44 = b;
                _get45 = i;
                // if 
                  if (i32_load8_u(0, 4, _get42 + _get43) != i32_load16_u(0, 4, _get44 + (_get45 * 2))) {
                    return 0;
                    (void) (_get15 + _get16);
                  }
                // end
                j224:;
                _get46 = i;
                i = _get46 + 1;
                goto j222;
              }
            // end
            j223:;
          // end
          return 1;
          (void) (_get13 + _get14);
        }
      // end
      j210:;
    } else {
      _get47 = bjjtype;
      // if 
        if (_get47 == 195) {
          i = 0;
          // loop 
          j226:;
            _get48 = i;
            _get49 = al;
            // if 
              if (_get48 < _get49) {
                _get50 = a;
                _get51 = i;
                _get52 = b;
                _get53 = i;
                // if 
                  if (i32_load16_u(0, 4, _get50 + (_get51 * 2)) != i32_load8_u(0, 4, _get52 + _get53)) {
                    return 0;
                  }
                // end
                j228:;
                _get54 = i;
                i = _get54 + 1;
                goto j226;
              }
            // end
            j227:;
          // end
          return 1;
        } else {
          _get55 = al;
          al = _get55 * 2;
          _get56 = bl;
          bl = _get56 * 2;
          _get57 = a;
          ap32 = _get57 - 28;
          _get58 = b;
          bp32 = _get58 - 28;
          _get59 = a;
          ap8 = _get59 - 4;
          _get60 = b;
          bp8 = _get60 - 4;
          _get61 = al;
          // if 
            if (_get61 >= 32) {
              // loop 
              j230:;
                _get62 = ap32;
                _get63 = al;
                _get64 = bp32;
                _get65 = al;
                _get66 = ap32;
                _get67 = al;
                _get68 = bp32;
                _get69 = al;
                // if 
                  if ((_get68 + _get69) != 0) {
                    return 0;
                  }
                // end
                j231:;
                _get70 = al;
                al = _get70 - 32;
                _get71 = al;
                if (_get71 >= 32) {
                  goto j230;
                }
              // end
            }
          // end
          j229:;
          _get72 = al;
          // if 
            if (_get72 >= 8) {
              // loop 
              j233:;
                _get73 = ap8;
                _get74 = al;
                _get75 = bp8;
                _get76 = al;
                // if 
                  if ((_get75 + _get76) != 0) {
                    return 0;
                  }
                // end
                j234:;
                _get77 = al;
                al = _get77 - 8;
                _get78 = al;
                if (_get78 >= 8) {
                  goto j233;
                }
              // end
            }
          // end
          j232:;
          _get79 = al;
          // if 
            if (_get79 >= 2) {
              // loop 
              j236:;
                _get80 = a;
                _get81 = al;
                _get82 = b;
                _get83 = al;
                // if 
                  if (i32_load16_u(0, 2, _get80 + _get81) != i32_load16_u(0, 2, _get82 + _get83)) {
                    return 0;
                  }
                // end
                j237:;
                _get84 = al;
                al = _get84 - 2;
                _get85 = al;
                if (_get85 >= 2) {
                  goto j236;
                }
              // end
            }
          // end
          j235:;
          return 1;
          (void) (_get73 + _get74);
        }
      // end
      j225:;
    }
  // end
  j209:;
  return 0;
}

static struct ReturnValue dong_porf_porf_todo__Porffor_object_get(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype) {
  i32 _get74;
  i32 _get73;
  i32 _get72;
  i32 _get71;
  i32 _get70;
  i32 _get69;
  i32 _get68;
  i32 _get67;
  i32 _get66;
  i32 _get65;
  i32 _get64;
  i32 _get63;
  i32 _get62;
  i32 _get61;
  i32 _get60;
  i32 _get59;
  i32 _get58;
  i32 _get57;
  i32 _get56;
  i32 _get55;
  i32 _get54;
  i32 _get53;
  i32 _get52;
  i32 _get51;
  i32 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 obj = 0;
  i32 objjjtype = 0;
  i32 trueType = 0;
  i32 jjlast_type = 0;
  i32 hash = 0;
  i32 entryPtr = 0;
  i32 lastProto = 0;
  i32 lastProtojjtype = 0;
  i32 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 tail = 0;
  i32 get = 0;
  i32 jjcall_val = 0;
  i32 jjcall_type = 0;
  i32 jjindirect_2_callee = 0;
  i32 jjswap = 0;

  _get0 = _obj;
  obj = _get0;
  _get1 = _objjjtype;
  objjjtype = _get1;
  _get2 = objjjtype;
  trueType = _get2;
  _get3 = trueType;
  // if 
    if (_get3 != 7) {
      _get4 = obj;
      _get5 = objjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__Porffor_object_underlying((f64)(_get4), _get5);
      jjlast_type = _0.type;
      _get6 = jjlast_type;
      objjjtype = _get6;
      obj = _0.value;
    }
  // end
  j201:;
  _get7 = obj;
  // if 
    if ((_get7) == 0) {
    }
  // end
  j202:;
  _get8 = key;
  _get9 = keyjjtype;
  hash = dong_porf_porf_todo__Porffor_object_hash(_get8, _get9);
  _get10 = obj;
  _get11 = objjjtype;
  _get12 = key;
  _get13 = keyjjtype;
  _get14 = hash;
  entryPtr = dong_porf_porf_todo__Porffor_object_lookup(_get10, _get11, _get12, _get13, _get14, 1);
  _get15 = entryPtr;
  // if 
    if (_get15 == -1) {
      _get16 = trueType;
      // if 
        if (_get16 == 7) {
          _get17 = obj;
          _get18 = obj;
          obj = i32_load(0, 4, _get18);
          objjjtype = i32_load8_u(0, 3, _get17);
          _get19 = objjjtype;
          // if 
            if ((_get19) == 0) {
              obj = dong_porf_porf_todo_jjget___Object_prototype();
              objjjtype = 7;
            }
          // end
          j205:;
        } else {
          _get20 = trueType;
          const struct ReturnValue _1 = dong_porf_porf_todo__Porffor_object_getHiddenPrototype(_get20, 1);
          jjlast_type = _1.type;
          _get21 = jjlast_type;
          objjjtype = _get21;
          obj = _1.value;
        }
      // end
      j204:;
      _get22 = hash;
      // if 
        if (_get22 == 593337848) {
          _get23 = key;
          _get24 = keyjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_strcmp(_get23, _get24, 863, 195)) != 0) {
              _get25 = obj;
              _get26 = objjjtype;
              return (struct ReturnValue){ (f64)(_get25), _get26 };
            }
          // end
          j238:;
        }
      // end
      j206:;
      _get27 = objjjtype;
      // if 
        if (_get27 != 7) {
          _get28 = obj;
          _get29 = objjjtype;
          const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_object_underlying((f64)(_get28), _get29);
          jjlast_type = _2.type;
          _get30 = jjlast_type;
          objjjtype = _get30;
          obj = _2.value;
        }
      // end
      j239:;
      _get31 = obj;
      lastProto = _get31;
      _get32 = objjjtype;
      lastProtojjtype = _get32;
      // loop 
      j240:;
        // if 
          if ((1) != 0) {
            _get33 = obj;
            _get34 = objjjtype;
            _get35 = key;
            _get36 = keyjjtype;
            _get37 = hash;
            entryPtr = dong_porf_porf_todo__Porffor_object_lookup(_get33, _get34, _get35, _get36, _get37, 1);
            _get38 = entryPtr;
            // if 
              if (_get38 != -1) {
                goto j241;
              }
            // end
            j242:;
            _get39 = objjjtype;
            // if 
              if (_get39 == 7) {
                _get40 = obj;
                _get41 = obj;
                obj = i32_load(0, 4, _get41);
                objjjtype = i32_load8_u(0, 3, _get40);
                _get42 = objjjtype;
                // if 
                  if ((_get42) == 0) {
                    obj = dong_porf_porf_todo_jjget___Object_prototype();
                    objjjtype = 7;
                  }
                // end
                j244:;
              } else {
                _get43 = obj;
                _get44 = objjjtype;
                const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_object_getPrototype(_get43, _get44);
                jjlast_type = _3.type;
                _get45 = jjlast_type;
                objjjtype = _get45;
                obj = _3.value;
              }
            // end
            j243:;
            _get46 = objjjtype;
            // if 
              if (_get46 != 7) {
                _get47 = obj;
                _get48 = objjjtype;
                const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_object_underlying((f64)(_get47), _get48);
                jjlast_type = _4.type;
                _get49 = jjlast_type;
                objjjtype = _get49;
                obj = _4.value;
              }
            // end
            j245:;
            _get50 = obj;
            jjlogicinner_tmp = _get50;
            _get51 = objjjtype;
            jjtypeswitch_tmp1 = _get51;
            // block i32
            i32 _r246;
              _get52 = jjtypeswitch_tmp1;
              // if 
                if ((_get52) == 0) {
                  _r246 = 1;
                  goto j246;
                }
              // end
              j247:;
              _get53 = jjtypeswitch_tmp1;
              // if 
                if (_get53 == 7) {
                  _get54 = jjlogicinner_tmp;
                  _r246 = (_get54) == 0;
                  goto j246;
                }
              // end
              j248:;
              _r246 = 0;
            // end
            j246:;
            _get55 = obj;
            _get56 = lastProto;
            // if 
              if ((_r246 | (_get55 == _get56)) != 0) {
                goto j241;
              }
            // end
            j249:;
            _get57 = obj;
            lastProto = _get57;
            _get58 = objjjtype;
            lastProtojjtype = _get58;
            goto j240;
          }
        // end
        j241:;
      // end
      _get59 = entryPtr;
      // if 
        if (_get59 == -1) {
          return (struct ReturnValue){ 0, 0 };
        }
      // end
      j250:;
    }
  // end
  j203:;
  _get60 = entryPtr;
  tail = i32_load16_u(0, 16, _get60);
  _get61 = tail;
  // if 
    if ((_get61 & 1) != 0) {
      _get62 = entryPtr;
      const struct ReturnValue _5 = dong_porf_porf_todo__Porffor_object_accessorGet(_get62, 1);
      jjlast_type = _5.type;
      get = _5.value;
      _get63 = get;
      // if 
        if ((_get63) == 0) {
          return (struct ReturnValue){ 0, 0 };
        }
      // end
      j252:;
      _get64 = get;
      jjindirect_2_callee = _get64;
      jjswap = 0;
      _get65 = jjswap;
      _get66 = _obj;
      jjcall_val = _get66;
      _get67 = jjcall_val;
      _get68 = _objjjtype;
      jjcall_type = _get68;
      _get69 = jjcall_type;
      jjswap = _get69;
      _get70 = jjswap;
      _get71 = jjindirect_2_callee;
      jjlast_type = 0;
      _get72 = jjlast_type;
      return (struct ReturnValue){ 0, _get72 };
      (void) 0;
    }
  // end
  j251:;
  _get73 = entryPtr;
  _get74 = tail;
  return (struct ReturnValue){ f64_load(0, 8, _get73), (u32)(_get74) >> 8 };
}

static struct ReturnValue dong_porf_porf_todo__String_prototype_trimEnd(i32 _this, i32 _thisjjtype) {
  i32 _get54;
  i32 _get53;
  i32 _get52;
  i32 _get51;
  i32 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 len = 0;
  i32 out = 0;
  i32 outPtr = 0;
  i32 thisPtr = 0;
  i32 thisPtrStart = 0;
  i32 n = 0;
  i32 start = 0;
  i32 chr = 0;

  _get0 = _thisjjtype;
  // if 
    if (_get0 != 67) {
      _get1 = _thisjjtype;
      _get2 = _thisjjtype;
      _get3 = _this;
      // if 
        if ((((_get1) == 0) | ((_get2 == 7) & ((_get3) == 0))) != 0) {
        }
      // end
      j265:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_porf_todo__Porffor_bytestringToString(_get7);
        }
      // end
      j266:;
    }
  // end
  j264:;
  _get8 = _this;
  len = i32_load(1, 0, _get8);
  _get9 = len;
  out = dong_porf_porf_todo__Porffor_malloc(6 + (_get9 * 2));
  _get10 = out;
  outPtr = _get10;
  _get11 = _this;
  thisPtr = _get11;
  _get12 = thisPtr;
  thisPtrStart = _get12;
  _get13 = thisPtr;
  _get14 = len;
  thisPtr = _get13 + (_get14 * 2);
  _get15 = outPtr;
  _get16 = len;
  outPtr = _get15 + (_get16 * 2);
  n = 0;
  start = 1;
  // loop 
  j267:;
    _get17 = thisPtr;
    _get18 = thisPtrStart;
    // if 
      if (_get17 > _get18) {
        _get19 = thisPtr;
        thisPtr = _get19 - 2;
        _get20 = thisPtr;
        chr = i32_load16_u(0, 4, _get20);
        _get21 = outPtr;
        outPtr = _get21 - 2;
        _get22 = start;
        // if 
          if ((_get22) != 0) {
            _get23 = chr;
            _get24 = chr;
            _get25 = chr;
            _get26 = chr;
            _get27 = chr;
            _get28 = chr;
            _get29 = chr;
            _get30 = chr;
            _get31 = chr;
            _get32 = chr;
            _get33 = chr;
            _get34 = chr;
            _get35 = chr;
            _get36 = chr;
            _get37 = chr;
            _get38 = chr;
            _get39 = chr;
            _get40 = chr;
            _get41 = chr;
            _get42 = chr;
            _get43 = chr;
            _get44 = chr;
            _get45 = chr;
            _get46 = chr;
            _get47 = chr;
            // if 
              if ((((((((((((((((((((((((((_get23 == 9) | (_get24 == 11)) | (_get25 == 12)) | (_get26 == 65279)) | (_get27 == 32)) | (_get28 == 160)) | (_get29 == 5760)) | (_get30 == 8192)) | (_get31 == 8193)) | (_get32 == 8194)) | (_get33 == 8195)) | (_get34 == 8196)) | (_get35 == 8197)) | (_get36 == 8198)) | (_get37 == 8199)) | (_get38 == 8200)) | (_get39 == 8201)) | (_get40 == 8202)) | (_get41 == 8239)) | (_get42 == 8287)) | (_get43 == 12288)) | (_get44 == 10)) | (_get45 == 13)) | (_get46 == 8232)) | (_get47 == 8233)) != 0) {
                _get48 = n;
                n = _get48 + 1;
                goto j267;
              }
            // end
            j270:;
            start = 0;
          }
        // end
        j269:;
        _get49 = outPtr;
        _get50 = chr;
        i32_store16(0, 4, _get49, _get50);
        goto j267;
      }
    // end
    j268:;
  // end
  _get51 = out;
  _get52 = len;
  _get53 = n;
  i32_store(1, 0, _get51, _get52 - _get53);
  _get54 = out;
  return (struct ReturnValue){ _get54, 67 };
}

static struct ReturnValue dong_porf_porf_todo__String_prototype_trimStart(i32 _this, i32 _thisjjtype) {
  i32 _get51;
  i32 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 len = 0;
  i32 out = 0;
  i32 outPtr = 0;
  i32 thisPtr = 0;
  i32 thisPtrEnd = 0;
  i32 n = 0;
  i32 start = 0;
  i32 chr = 0;

  _get0 = _thisjjtype;
  // if 
    if (_get0 != 67) {
      _get1 = _thisjjtype;
      _get2 = _thisjjtype;
      _get3 = _this;
      // if 
        if ((((_get1) == 0) | ((_get2 == 7) & ((_get3) == 0))) != 0) {
        }
      // end
      j272:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_porf_todo__Porffor_bytestringToString(_get7);
        }
      // end
      j273:;
    }
  // end
  j271:;
  _get8 = _this;
  len = i32_load(1, 0, _get8);
  _get9 = len;
  out = dong_porf_porf_todo__Porffor_malloc(6 + (_get9 * 2));
  _get10 = out;
  outPtr = _get10;
  _get11 = _this;
  thisPtr = _get11;
  _get12 = thisPtr;
  _get13 = len;
  thisPtrEnd = _get12 + (_get13 * 2);
  n = 0;
  start = 1;
  // loop 
  j274:;
    _get14 = thisPtr;
    _get15 = thisPtrEnd;
    // if 
      if (_get14 < _get15) {
        _get16 = thisPtr;
        chr = i32_load16_u(0, 4, _get16);
        _get17 = thisPtr;
        thisPtr = _get17 + 2;
        _get18 = start;
        // if 
          if ((_get18) != 0) {
            _get19 = chr;
            _get20 = chr;
            _get21 = chr;
            _get22 = chr;
            _get23 = chr;
            _get24 = chr;
            _get25 = chr;
            _get26 = chr;
            _get27 = chr;
            _get28 = chr;
            _get29 = chr;
            _get30 = chr;
            _get31 = chr;
            _get32 = chr;
            _get33 = chr;
            _get34 = chr;
            _get35 = chr;
            _get36 = chr;
            _get37 = chr;
            _get38 = chr;
            _get39 = chr;
            _get40 = chr;
            _get41 = chr;
            _get42 = chr;
            _get43 = chr;
            // if 
              if ((((((((((((((((((((((((((_get19 == 9) | (_get20 == 11)) | (_get21 == 12)) | (_get22 == 65279)) | (_get23 == 32)) | (_get24 == 160)) | (_get25 == 5760)) | (_get26 == 8192)) | (_get27 == 8193)) | (_get28 == 8194)) | (_get29 == 8195)) | (_get30 == 8196)) | (_get31 == 8197)) | (_get32 == 8198)) | (_get33 == 8199)) | (_get34 == 8200)) | (_get35 == 8201)) | (_get36 == 8202)) | (_get37 == 8239)) | (_get38 == 8287)) | (_get39 == 12288)) | (_get40 == 10)) | (_get41 == 13)) | (_get42 == 8232)) | (_get43 == 8233)) != 0) {
                _get44 = n;
                n = _get44 + 1;
                goto j274;
              }
            // end
            j277:;
            start = 0;
          }
        // end
        j276:;
        _get45 = outPtr;
        _get46 = chr;
        i32_store16(0, 4, _get45, _get46);
        _get47 = outPtr;
        outPtr = _get47 + 2;
        goto j274;
      }
    // end
    j275:;
  // end
  _get48 = out;
  _get49 = len;
  _get50 = n;
  i32_store(1, 0, _get48, _get49 - _get50);
  _get51 = out;
  return (struct ReturnValue){ _get51, 67 };
}

static struct ReturnValue dong_porf_porf_todo__String_prototype_trim(i32 _this, i32 _thisjjtype) {
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 jjlast_type = 0;

  _get0 = _thisjjtype;
  // if 
    if (_get0 != 67) {
      _get1 = _thisjjtype;
      _get2 = _thisjjtype;
      _get3 = _this;
      // if 
        if ((((_get1) == 0) | ((_get2 == 7) & ((_get3) == 0))) != 0) {
        }
      // end
      j262:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_porf_todo__Porffor_bytestringToString(_get7);
        }
      // end
      j263:;
    }
  // end
  j261:;
  _get8 = _this;
  const struct ReturnValue _1 = dong_porf_porf_todo__String_prototype_trimEnd(_get8, 67);
  jjlast_type = _1.type;
  _get9 = jjlast_type;
  const struct ReturnValue _2 = dong_porf_porf_todo__String_prototype_trimStart(_1.value, _get9);
  jjlast_type = _2.type;
  _get10 = jjlast_type;
  return (struct ReturnValue){ _2.value, _get10 };
}

static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_trimEnd(i32 _this, i32 _thisjjtype) {
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 len = 0;
  i32 out = 0;
  i32 outPtr = 0;
  i32 thisPtr = 0;
  i32 thisPtrStart = 0;
  i32 n = 0;
  i32 start = 0;
  i32 chr = 0;

  _get0 = _this;
  len = i32_load(1, 0, _get0);
  _get1 = len;
  out = dong_porf_porf_todo__Porffor_malloc(6 + _get1);
  _get2 = out;
  outPtr = _get2;
  _get3 = _this;
  thisPtr = _get3;
  _get4 = thisPtr;
  thisPtrStart = _get4;
  _get5 = thisPtr;
  _get6 = len;
  thisPtr = _get5 + _get6;
  _get7 = outPtr;
  _get8 = len;
  outPtr = _get7 + _get8;
  n = 0;
  start = 1;
  // loop 
  j280:;
    _get9 = thisPtr;
    _get10 = thisPtrStart;
    // if 
      if (_get9 > _get10) {
        _get11 = thisPtr;
        thisPtr = _get11 - 1;
        _get12 = thisPtr;
        chr = i32_load8_u(0, 4, _get12);
        _get13 = outPtr;
        outPtr = _get13 - 1;
        _get14 = start;
        // if 
          if ((_get14) != 0) {
            _get15 = chr;
            _get16 = chr;
            _get17 = chr;
            _get18 = chr;
            _get19 = chr;
            _get20 = chr;
            _get21 = chr;
            _get22 = chr;
            _get23 = chr;
            _get24 = chr;
            _get25 = chr;
            _get26 = chr;
            _get27 = chr;
            _get28 = chr;
            _get29 = chr;
            _get30 = chr;
            _get31 = chr;
            _get32 = chr;
            _get33 = chr;
            _get34 = chr;
            _get35 = chr;
            _get36 = chr;
            _get37 = chr;
            _get38 = chr;
            _get39 = chr;
            // if 
              if ((((((((((((((((((((((((((_get15 == 9) | (_get16 == 11)) | (_get17 == 12)) | (_get18 == 65279)) | (_get19 == 32)) | (_get20 == 160)) | (_get21 == 5760)) | (_get22 == 8192)) | (_get23 == 8193)) | (_get24 == 8194)) | (_get25 == 8195)) | (_get26 == 8196)) | (_get27 == 8197)) | (_get28 == 8198)) | (_get29 == 8199)) | (_get30 == 8200)) | (_get31 == 8201)) | (_get32 == 8202)) | (_get33 == 8239)) | (_get34 == 8287)) | (_get35 == 12288)) | (_get36 == 10)) | (_get37 == 13)) | (_get38 == 8232)) | (_get39 == 8233)) != 0) {
                _get40 = n;
                n = _get40 + 1;
                goto j280;
              }
            // end
            j283:;
            start = 0;
          }
        // end
        j282:;
        _get41 = outPtr;
        _get42 = chr;
        i32_store8(0, 4, _get41, _get42);
        goto j280;
      }
    // end
    j281:;
  // end
  _get43 = out;
  _get44 = len;
  _get45 = n;
  i32_store(1, 0, _get43, _get44 - _get45);
  _get46 = out;
  return (struct ReturnValue){ _get46, 195 };
}

static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_trimStart(i32 _this, i32 _thisjjtype) {
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 len = 0;
  i32 out = 0;
  i32 outPtr = 0;
  i32 thisPtr = 0;
  i32 thisPtrEnd = 0;
  i32 n = 0;
  i32 start = 0;
  i32 chr = 0;

  _get0 = _this;
  len = i32_load(1, 0, _get0);
  _get1 = len;
  out = dong_porf_porf_todo__Porffor_malloc(6 + _get1);
  _get2 = out;
  outPtr = _get2;
  _get3 = _this;
  thisPtr = _get3;
  _get4 = thisPtr;
  _get5 = len;
  thisPtrEnd = _get4 + _get5;
  n = 0;
  start = 1;
  // loop 
  j284:;
    _get6 = thisPtr;
    _get7 = thisPtrEnd;
    // if 
      if (_get6 < _get7) {
        _get8 = thisPtr;
        _get9 = thisPtr;
        thisPtr = _get9 + 1;
        chr = i32_load8_u(0, 4, _get8);
        _get10 = start;
        // if 
          if ((_get10) != 0) {
            _get11 = chr;
            _get12 = chr;
            _get13 = chr;
            _get14 = chr;
            _get15 = chr;
            _get16 = chr;
            _get17 = chr;
            _get18 = chr;
            _get19 = chr;
            _get20 = chr;
            _get21 = chr;
            _get22 = chr;
            _get23 = chr;
            _get24 = chr;
            _get25 = chr;
            _get26 = chr;
            _get27 = chr;
            _get28 = chr;
            _get29 = chr;
            _get30 = chr;
            _get31 = chr;
            _get32 = chr;
            _get33 = chr;
            _get34 = chr;
            _get35 = chr;
            // if 
              if ((((((((((((((((((((((((((_get11 == 9) | (_get12 == 11)) | (_get13 == 12)) | (_get14 == 65279)) | (_get15 == 32)) | (_get16 == 160)) | (_get17 == 5760)) | (_get18 == 8192)) | (_get19 == 8193)) | (_get20 == 8194)) | (_get21 == 8195)) | (_get22 == 8196)) | (_get23 == 8197)) | (_get24 == 8198)) | (_get25 == 8199)) | (_get26 == 8200)) | (_get27 == 8201)) | (_get28 == 8202)) | (_get29 == 8239)) | (_get30 == 8287)) | (_get31 == 12288)) | (_get32 == 10)) | (_get33 == 13)) | (_get34 == 8232)) | (_get35 == 8233)) != 0) {
                _get36 = n;
                n = _get36 + 1;
                goto j284;
              }
            // end
            j287:;
            start = 0;
          }
        // end
        j286:;
        _get37 = outPtr;
        _get38 = outPtr;
        outPtr = _get38 + 1;
        _get39 = chr;
        i32_store8(0, 4, _get37, _get39);
        goto j284;
      }
    // end
    j285:;
  // end
  _get40 = out;
  _get41 = len;
  _get42 = n;
  i32_store(1, 0, _get40, _get41 - _get42);
  _get43 = out;
  return (struct ReturnValue){ _get43, 195 };
}

static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_trim(i32 _this, i32 _thisjjtype) {
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 jjlast_type = 0;

  _get0 = _this;
  const struct ReturnValue _0 = dong_porf_porf_todo__ByteString_prototype_trimEnd(_get0, 195);
  jjlast_type = _0.type;
  _get1 = jjlast_type;
  const struct ReturnValue _1 = dong_porf_porf_todo__ByteString_prototype_trimStart(_0.value, _get1);
  jjlast_type = _1.type;
  _get2 = jjlast_type;
  return (struct ReturnValue){ _1.value, _get2 };
}

static struct ReturnValue dong_porf_porf_todo__String_prototype_charCodeAt(f64 _this, i32 _thisjjtype, f64 index, i32 indexjjtype) {
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  f64 _get8;
  f64 _get7;
  i32 _get6;
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  f64 len = 0;

  _get0 = _thisjjtype;
  // if 
    if (_get0 != 67) {
      _get1 = _thisjjtype;
      _get2 = _thisjjtype;
      _get3 = _this;
      // if 
        if (((_get1 == 0) | ((_get2 == 7) & (_get3 == 0))) != 0) {
        }
      // end
      j292:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString(_get4, _get5);
      _thisjjtype = _0.type;
      _this = _0.value;
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = (f64)(dong_porf_porf_todo__Porffor_bytestringToString((u32)(_get7)));
        }
      // end
      j293:;
    }
  // end
  j291:;
  _get8 = _this;
  len = (f64)(i32_load(1, 0, (u32)(_get8)));
  _get9 = index;
  index = dong_porf_porf_todo__Math_trunc(_get9);
  _get10 = index;
  _get11 = index;
  _get12 = len;
  // if 
    if (((_get10 < 0) | (_get11 >= _get12)) != 0) {
      return (struct ReturnValue){ NaN, 1 };
    }
  // end
  j294:;
  _get13 = _this;
  _get14 = index;
  return (struct ReturnValue){ (f64)(i32_load16_u(0, 4, (i32)((_get13 + (_get14 * 2))))), 1 };
}

static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_charCodeAt(f64 _this, i32 _thisjjtype, f64 index, i32 indexjjtype) {
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 len = 0;

  _get0 = _this;
  len = (f64)(i32_load(1, 0, (u32)(_get0)));
  _get1 = index;
  index = dong_porf_porf_todo__Math_trunc(_get1);
  _get2 = index;
  _get3 = index;
  _get4 = len;
  // if 
    if (((_get2 < 0) | (_get3 >= _get4)) != 0) {
      return (struct ReturnValue){ NaN, 1 };
    }
  // end
  j297:;
  _get5 = _this;
  _get6 = index;
  return (struct ReturnValue){ (f64)(i32_load8_u(0, 4, (i32)((_get5 + _get6)))), 1 };
}

static f64 dong_porf_porf_todo__Porffor_stn_int(f64 str, i32 strjjtype, f64 radix, i32 radixjjtype, f64 i, i32 ijjtype) {
  f64 _get50;
  f64 _get49;
  f64 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  f64 _get44;
  i32 _get43;
  f64 _get42;
  f64 _get41;
  f64 _get40;
  f64 _get39;
  i32 _get38;
  f64 _get37;
  f64 _get36;
  i32 _get35;
  f64 _get34;
  f64 _get33;
  f64 _get32;
  f64 _get31;
  f64 _get30;
  i32 _get29;
  f64 _get28;
  f64 _get27;
  i32 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  i32 _get22;
  f64 _get21;
  i32 _get20;
  f64 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  f64 _get13;
  i32 _get12;
  f64 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 nMax = 0;
  f64 n = 0;
  f64 len = 0;
  f64 chr = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 jjlast_type = 0;
  i32 logictmpi = 0;

  nMax = 58;
  _get0 = radix;
  // if 
    if (_get0 < 10) {
      _get1 = radix;
      nMax = 48 + _get1;
    }
  // end
  j305:;
  n = 0;
  _get2 = str;
  len = (f64)(i32_load(1, 0, (u32)(_get2)));
  _get3 = len;
  _get4 = i;
  // if 
    if ((_get3 - _get4) == 0) {
      return NaN;
    }
  // end
  j306:;
  // loop 
  j307:;
    _get5 = i;
    _get6 = len;
    // if 
      if (_get5 < _get6) {
        _get7 = str;
        jjproto_target = _get7;
        _get8 = strjjtype;
        jjproto_targetjjtype = _get8;
        _get9 = strjjtype;
        jjtypeswitch_tmp1 = _get9;
        // block f64
        f64 _r309;
          _get10 = jjtypeswitch_tmp1;
          // if 
            if (_get10 == 33) {
              _get11 = jjproto_target;
              _get12 = jjproto_targetjjtype;
              _get13 = i;
              _get14 = i;
              i = _get14 + 1;
              const struct ReturnValue _0 = dong_porf_porf_todo__String_prototype_charCodeAt(_get11, _get12, _get13, 1);
              jjlast_type = _0.type;
              _r309 = _0.value;
              goto j309;
            }
          // end
          j310:;
          _get15 = jjtypeswitch_tmp1;
          // if 
            if (_get15 == 67) {
              _get16 = jjproto_target;
              _get17 = jjproto_targetjjtype;
              _get18 = i;
              _get19 = i;
              i = _get19 + 1;
              const struct ReturnValue _1 = dong_porf_porf_todo__String_prototype_charCodeAt(_get16, _get17, _get18, 1);
              jjlast_type = _1.type;
              _r309 = _1.value;
              goto j309;
            }
          // end
          j311:;
          _get20 = jjtypeswitch_tmp1;
          // if 
            if (_get20 == 195) {
              _get21 = jjproto_target;
              _get22 = jjproto_targetjjtype;
              _get23 = i;
              _get24 = i;
              i = _get24 + 1;
              const struct ReturnValue _2 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get21, _get22, _get23, 1);
              jjlast_type = _2.type;
              _r309 = _2.value;
              goto j309;
            }
          // end
          j312:;
          _r309 = 0;
        // end
        j309:;
        chr = _r309;
        _get25 = chr;
        logictmpi = _get25 >= 48;
        _get26 = logictmpi;
        // if i32
        i32 _r313;
          if ((_get26) != 0) {
            _get27 = chr;
            _get28 = nMax;
            jjlast_type = 2;
            _r313 = _get27 < _get28;
          } else {
            _get29 = logictmpi;
            jjlast_type = 2;
            _r313 = _get29;
          }
        // end
        j313:;
        // if 
          if ((_r313) != 0) {
            _get30 = n;
            _get31 = radix;
            _get32 = chr;
            n = ((_get30 * _get31) + _get32) - 48;
          } else {
            _get33 = radix;
            // if 
              if (_get33 > 10) {
                _get34 = chr;
                logictmpi = _get34 >= 97;
                _get35 = logictmpi;
                // if i32
                i32 _r316;
                  if ((_get35) != 0) {
                    _get36 = chr;
                    _get37 = radix;
                    jjlast_type = 2;
                    _r316 = _get36 < (87 + _get37);
                  } else {
                    _get38 = logictmpi;
                    jjlast_type = 2;
                    _r316 = _get38;
                  }
                // end
                j316:;
                // if 
                  if ((_r316) != 0) {
                    _get39 = n;
                    _get40 = radix;
                    _get41 = chr;
                    n = ((_get39 * _get40) + _get41) - 87;
                  } else {
                    _get42 = chr;
                    logictmpi = _get42 >= 65;
                    _get43 = logictmpi;
                    // if i32
                    i32 _r318;
                      if ((_get43) != 0) {
                        _get44 = chr;
                        _get45 = radix;
                        jjlast_type = 2;
                        _r318 = _get44 < (55 + _get45);
                      } else {
                        _get46 = logictmpi;
                        jjlast_type = 2;
                        _r318 = _get46;
                      }
                    // end
                    j318:;
                    // if 
                      if ((_r318) != 0) {
                        _get47 = n;
                        _get48 = radix;
                        _get49 = chr;
                        n = ((_get47 * _get48) + _get49) - 55;
                      } else {
                        return NaN;
                      }
                    // end
                    j319:;
                  }
                // end
                j317:;
              } else {
                return NaN;
              }
            // end
            j315:;
          }
        // end
        j314:;
        goto j307;
      }
    // end
    j308:;
  // end
  _get50 = n;
  return _get50;
}

static f64 dong_porf_porf_todo__Porffor_parseExp(f64 str, i32 strjjtype, f64 i, i32 ijjtype, f64 len, i32 lenjjtype, f64 strict, i32 strictjjtype) {
  f64 _get59;
  f64 _get58;
  f64 _get57;
  f64 _get56;
  f64 _get55;
  f64 _get54;
  f64 _get53;
  f64 _get52;
  i32 _get51;
  f64 _get50;
  i32 _get49;
  f64 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  f64 _get41;
  i32 _get40;
  f64 _get39;
  i32 _get38;
  f64 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  f64 _get33;
  f64 _get32;
  f64 _get31;
  f64 _get30;
  f64 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  f64 _get22;
  f64 _get21;
  f64 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  f64 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 expNeg = 0;
  f64 exp = 0;
  f64 hasDigit = 0;
  f64 sign = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 jjlast_type = 0;
  f64 logictmp = 0;
  f64 jjlogicinner_tmp = 0;
  f64 chr = 0;
  i32 logictmpi = 0;

  expNeg = 0;
  exp = 0;
  hasDigit = 0;
  _get0 = i;
  _get1 = len;
  // if 
    if (_get0 < _get1) {
      _get2 = str;
      jjproto_target = _get2;
      _get3 = strjjtype;
      jjproto_targetjjtype = _get3;
      _get4 = strjjtype;
      jjtypeswitch_tmp1 = _get4;
      // block f64
      f64 _r393;
        _get5 = jjtypeswitch_tmp1;
        // if 
          if (_get5 == 33) {
            _get6 = jjproto_target;
            _get7 = jjproto_targetjjtype;
            _get8 = i;
            const struct ReturnValue _0 = dong_porf_porf_todo__String_prototype_charCodeAt(_get6, _get7, _get8, 1);
            jjlast_type = _0.type;
            _r393 = _0.value;
            goto j393;
          }
        // end
        j394:;
        _get9 = jjtypeswitch_tmp1;
        // if 
          if (_get9 == 67) {
            _get10 = jjproto_target;
            _get11 = jjproto_targetjjtype;
            _get12 = i;
            const struct ReturnValue _1 = dong_porf_porf_todo__String_prototype_charCodeAt(_get10, _get11, _get12, 1);
            jjlast_type = _1.type;
            _r393 = _1.value;
            goto j393;
          }
        // end
        j395:;
        _get13 = jjtypeswitch_tmp1;
        // if 
          if (_get13 == 195) {
            _get14 = jjproto_target;
            _get15 = jjproto_targetjjtype;
            _get16 = i;
            const struct ReturnValue _2 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get14, _get15, _get16, 1);
            jjlast_type = _2.type;
            _r393 = _2.value;
            goto j393;
          }
        // end
        j396:;
        _r393 = 0;
      // end
      j393:;
      sign = _r393;
      _get17 = sign;
      // if 
        if (_get17 == 43) {
          _get18 = i;
          i = _get18 + 1;
        } else {
          _get19 = sign;
          // if 
            if (_get19 == 45) {
              expNeg = 1;
              _get20 = i;
              i = _get20 + 1;
            }
          // end
          j398:;
        }
      // end
      j397:;
    }
  // end
  j392:;
  _get21 = strict;
  logictmp = _get21;
  _get22 = logictmp;
  // if f64
  f64 _r399;
    if (((u32)(_get22)) != 0) {
      _get23 = i;
      _get24 = len;
      jjlast_type = 2;
      _r399 = (f64)(_get23 >= _get24);
    } else {
      _get25 = logictmp;
      jjlast_type = 2;
      _r399 = _get25;
    }
  // end
  j399:;
  jjlogicinner_tmp = _r399;
  _get26 = jjlast_type;
  jjtypeswitch_tmp1 = _get26;
  // block i32
  i32 _r400;
    _get27 = jjtypeswitch_tmp1;
    _get28 = jjtypeswitch_tmp1;
    // if 
      if (((_get27 == 67) | (_get28 == 195)) != 0) {
        _get29 = jjlogicinner_tmp;
        _r400 = i32_load(1, 0, (u32)(_get29));
        goto j400;
      }
    // end
    j401:;
    _get30 = jjlogicinner_tmp;
    _r400 = (u32)(_get30);
  // end
  j400:;
  // if 
    if ((_r400) != 0) {
      return NaN;
    }
  // end
  j402:;
  // loop 
  j403:;
    _get31 = i;
    _get32 = len;
    // if 
      if (_get31 < _get32) {
        _get33 = str;
        jjproto_target = _get33;
        _get34 = strjjtype;
        jjproto_targetjjtype = _get34;
        _get35 = strjjtype;
        jjtypeswitch_tmp1 = _get35;
        // block f64
        f64 _r405;
          _get36 = jjtypeswitch_tmp1;
          // if 
            if (_get36 == 33) {
              _get37 = jjproto_target;
              _get38 = jjproto_targetjjtype;
              _get39 = i;
              const struct ReturnValue _3 = dong_porf_porf_todo__String_prototype_charCodeAt(_get37, _get38, _get39, 1);
              jjlast_type = _3.type;
              _r405 = _3.value;
              goto j405;
            }
          // end
          j406:;
          _get40 = jjtypeswitch_tmp1;
          // if 
            if (_get40 == 67) {
              _get41 = jjproto_target;
              _get42 = jjproto_targetjjtype;
              _get43 = i;
              const struct ReturnValue _4 = dong_porf_porf_todo__String_prototype_charCodeAt(_get41, _get42, _get43, 1);
              jjlast_type = _4.type;
              _r405 = _4.value;
              goto j405;
            }
          // end
          j407:;
          _get44 = jjtypeswitch_tmp1;
          // if 
            if (_get44 == 195) {
              _get45 = jjproto_target;
              _get46 = jjproto_targetjjtype;
              _get47 = i;
              const struct ReturnValue _5 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get45, _get46, _get47, 1);
              jjlast_type = _5.type;
              _r405 = _5.value;
              goto j405;
            }
          // end
          j408:;
          _r405 = 0;
        // end
        j405:;
        chr = _r405;
        _get48 = chr;
        logictmpi = _get48 >= 48;
        _get49 = logictmpi;
        // if i32
        i32 _r409;
          if ((_get49) != 0) {
            _get50 = chr;
            jjlast_type = 2;
            _r409 = _get50 <= 57;
          } else {
            _get51 = logictmpi;
            jjlast_type = 2;
            _r409 = _get51;
          }
        // end
        j409:;
        // if 
          if ((_r409) != 0) {
            _get52 = exp;
            _get53 = chr;
            exp = ((_get52 * 10) + _get53) - 48;
            hasDigit = 1;
            _get54 = i;
            i = _get54 + 1;
          } else {
            _get55 = strict;
            // if 
              if (((u32)(_get55)) != 0) {
                return NaN;
              }
            // end
            j411:;
            goto j404;
          }
        // end
        j410:;
        goto j403;
      }
    // end
    j404:;
  // end
  _get56 = hasDigit;
  // if 
    if (_get56 == 0) {
      return NaN;
    }
  // end
  j412:;
  _get57 = expNeg;
  // if 
    if (((u32)(_get57)) != 0) {
      _get58 = exp;
      return -_get58;
    }
  // end
  j413:;
  _get59 = exp;
  return _get59;
}

static f64 dong_porf_porf_todo__Number_isInteger(f64 l0) {
  f64 _get1;
  f64 _get0;
  _get0 = l0;
  _get1 = l0;
  return (f64)(_get0 == trunc(_get1));
}

static f64 dong_porf_porf_todo__Math_abs(f64 l0) {
  f64 _get0;
  _get0 = l0;
  const f64 _tmp0 = _get0;
  return (_tmp0 < 0 ? -_tmp0 : _tmp0);
}

static f64 dong_porf_porf_todo__Math_floor(f64 l0) {
  f64 _get0;
  _get0 = l0;
  return floor(_get0);
}

static f64 dong_porf_porf_todo__Math_exp(f64 x, i32 xjjtype) {
  f64 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  f64 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 k = 0;
  f64 r = 0;
  f64 term = 0;
  f64 sum = 0;
  f64 i = 0;

  _get0 = x;
  // if 
    if (dong_porf_porf_todo__Number_isFinite(_get0) == 0) {
      _get1 = x;
      // if 
        if (_get1 == (-Infinity)) {
          return 0;
        }
      // end
      j443:;
      _get2 = x;
      return _get2;
    }
  // end
  j442:;
  _get3 = x;
  // if 
    if (_get3 < 0) {
      _get4 = x;
      return 1 / dong_porf_porf_todo__Math_exp(-_get4, 1);
    }
  // end
  j444:;
  _get5 = x;
  k = dong_porf_porf_todo__Math_floor(_get5 / 0.6931471805599453);
  _get6 = x;
  _get7 = k;
  r = _get6 - (_get7 * 0.6931471805599453);
  _get8 = r;
  term = _get8;
  _get9 = r;
  sum = 1 + _get9;
  i = 2;
  // loop 
  j445:;
    _get10 = term;
    // if 
      if (dong_porf_porf_todo__Math_abs(_get10) > 1e-15) {
        _get11 = term;
        _get12 = r;
        _get13 = i;
        term = _get11 * (_get12 / _get13);
        _get14 = sum;
        _get15 = term;
        sum = _get14 + _get15;
        _get16 = i;
        i = _get16 + 1;
        goto j445;
      }
    // end
    j446:;
  // end
  // loop 
  j447:;
    _get17 = k;
    _get18 = k;
    k = _get18 - 1;
    // if 
      if (_get17 > 0) {
        _get19 = sum;
        sum = _get19 * 2;
        goto j447;
      }
    // end
    j448:;
  // end
  _get20 = sum;
  return _get20;
}

static f64 dong_porf_porf_todo__Math_log2(f64 y, i32 yjjtype) {
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  f64 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 x = 0;
  f64 exponent = 0;
  f64 delta = 0;
  f64 e_x = 0;

  _get0 = y;
  // if 
    if (_get0 <= 0) {
      return NaN;
    }
  // end
  j457:;
  _get1 = y;
  // if 
    if (dong_porf_porf_todo__Number_isFinite(_get1) == 0) {
      _get2 = y;
      return _get2;
    }
  // end
  j458:;
  _get3 = y;
  x = _get3;
  exponent = 0;
  // loop 
  j459:;
    _get4 = x;
    // if 
      if (_get4 >= 2) {
        _get5 = x;
        x = _get5 / 2;
        _get6 = exponent;
        exponent = _get6 + 1;
        goto j459;
      }
    // end
    j460:;
  // end
  // loop 
  j461:;
    _get7 = x;
    // if 
      if (_get7 < 1) {
        _get8 = x;
        x = _get8 * 2;
        _get9 = exponent;
        exponent = _get9 - 1;
        goto j461;
      }
    // end
    j462:;
  // end
  _get10 = x;
  x = _get10 - 1;
  // loop 
  j463:;
    // block 
      _get11 = x;
      e_x = dong_porf_porf_todo__Math_exp(_get11 * 0.6931471805599453, 1);
      _get12 = e_x;
      _get13 = y;
      _get14 = e_x;
      delta = (_get12 - _get13) / (_get14 * 0.6931471805599453);
      _get15 = x;
      _get16 = delta;
      x = _get15 - _get16;
      _get17 = delta;
      if (dong_porf_porf_todo__Math_abs(_get17) > 1e-15) {
        goto j463;
      }
    // end
    j464:;
  // end
  _get18 = x;
  _get19 = exponent;
  return _get18 + _get19;
}

static f64 dong_porf_porf_todo__Math_log(f64 y, i32 yjjtype) {
  f64 _get31;
  f64 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  f64 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  f64 _get22;
  f64 _get21;
  f64 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  f64 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 n = 0;
  i32 njjtype = 0;
  f64 m = 0;
  i32 mjjtype = 0;
  f64 x = 0;
  i32 xjjtype = 0;
  f64 x2 = 0;
  i32 x2jjtype = 0;
  f64 sum = 0;
  i32 sumjjtype = 0;
  f64 term = 0;
  i32 termjjtype = 0;
  f64 i = 0;
  i32 ijjtype = 0;

  _get0 = y;
  // if 
    if (_get0 <= 0) {
      _get1 = y;
      // if 
        if (_get1 == 0) {
          return -Infinity;
        }
      // end
      j454:;
      return NaN;
    }
  // end
  j453:;
  _get2 = y;
  // if 
    if (dong_porf_porf_todo__Number_isFinite(_get2) == 0) {
      _get3 = y;
      return _get3;
    }
  // end
  j455:;
  _get4 = y;
  // if 
    if (_get4 > 1e+308) {
      _get5 = y;
      n = dong_porf_porf_todo__Math_floor(dong_porf_porf_todo__Math_log2(_get5, 1));
      njjtype = 1;
      _get6 = n;
      _get7 = y;
      _get8 = n;
      return (0.6931471805599453 * _get6) + dong_porf_porf_todo__Math_log((_get7 / (f64)(1 << 30)) / (f64)(1 << (i32)((_get8 - 30))), 1);
    }
  // end
  j456:;
  m = 0;
  mjjtype = 1;
  // loop 
  j465:;
    _get9 = y;
    // if 
      if (_get9 >= 2) {
        _get10 = y;
        y = _get10 / 2;
        _get11 = m;
        m = _get11 + 1;
        goto j465;
      }
    // end
    j466:;
  // end
  // loop 
  j467:;
    _get12 = y;
    // if 
      if (_get12 < 1) {
        _get13 = y;
        y = _get13 * 2;
        _get14 = m;
        m = _get14 - 1;
        goto j467;
      }
    // end
    j468:;
  // end
  _get15 = y;
  y = _get15 - 1;
  _get16 = y;
  _get17 = y;
  x = _get16 / (2 + _get17);
  xjjtype = 1;
  _get18 = x;
  _get19 = x;
  x2 = _get18 * _get19;
  x2jjtype = 1;
  _get20 = x;
  sum = _get20;
  sumjjtype = 1;
  _get21 = x;
  term = _get21;
  termjjtype = 1;
  i = 1;
  ijjtype = 1;
  // loop 
  j469:;
    _get22 = term;
    // if 
      if (dong_porf_porf_todo__Math_abs(_get22) > 1e-15) {
        _get23 = term;
        _get24 = x2;
        _get25 = i;
        _get26 = i;
        term = _get23 * ((_get24 * ((2 * _get25) - 1)) / ((2 * _get26) + 1));
        termjjtype = 1;
        _get27 = sum;
        _get28 = term;
        sum = _get27 + _get28;
        sumjjtype = 1;
        _get29 = i;
        i = _get29 + 1;
        goto j469;
      }
    // end
    j470:;
  // end
  _get30 = sum;
  _get31 = m;
  return (2 * _get30) + (_get31 * 0.6931471805599453);
}

static f64 dong_porf_porf_todo__Math_pow(f64 base, i32 basejjtype, f64 exponent, i32 exponentjjtype) {
  f64 _get59;
  f64 _get58;
  f64 _get57;
  f64 _get56;
  f64 _get55;
  f64 _get54;
  f64 _get53;
  f64 _get52;
  f64 _get51;
  f64 _get50;
  f64 _get49;
  f64 _get48;
  f64 _get47;
  f64 _get46;
  f64 _get45;
  f64 _get44;
  f64 _get43;
  f64 _get42;
  f64 _get41;
  f64 _get40;
  f64 _get39;
  f64 _get38;
  f64 _get37;
  f64 _get36;
  f64 _get35;
  f64 _get34;
  f64 _get33;
  f64 _get32;
  f64 _get31;
  f64 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  f64 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  f64 _get22;
  f64 _get21;
  f64 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  f64 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 isOdd = 0;
  i32 isOddjjtype = 0;
  f64 jjmath_a = 0;
  f64 jjmath_b = 0;
  f64 abs = 0;
  i32 absjjtype = 0;
  f64 currentBase = 0;
  f64 currentExponent = 0;
  f64 result = 0;
  i32 jjlast_type = 0;

  _get0 = exponent;
  // if 
    if (((u32)(dong_porf_porf_todo__Number_isNaN(_get0))) != 0) {
      return NaN;
    }
  // end
  j416:;
  _get1 = exponent;
  // if 
    if (_get1 == 0) {
      return 1;
    }
  // end
  j417:;
  _get2 = base;
  // if 
    if (_get2 == 2) {
      _get3 = exponent;
      _get4 = exponent;
      _get5 = exponent;
      // if 
        if ((((u32)(dong_porf_porf_todo__Number_isInteger(_get3)) & (_get4 > 0)) & (_get5 < 31)) != 0) {
          _get6 = exponent;
          return (f64)(2 << (i32)((_get6 - 1)));
        }
      // end
      j419:;
    }
  // end
  j418:;
  _get7 = base;
  // if 
    if (dong_porf_porf_todo__Number_isFinite(_get7) == 0) {
      _get8 = base;
      // if 
        if (((u32)(dong_porf_porf_todo__Number_isNaN(_get8))) != 0) {
          _get9 = base;
          return _get9;
        }
      // end
      j421:;
      _get10 = base;
      // if 
        if (_get10 == Infinity) {
          _get11 = exponent;
          // if 
            if (_get11 > 0) {
              _get12 = base;
              return _get12;
            }
          // end
          j423:;
          return 0;
        }
      // end
      j422:;
      _get13 = exponent;
      jjmath_a = dong_porf_porf_todo__Math_abs(_get13);
      _get14 = jjmath_a;
      jjmath_b = 2;
      _get15 = jjmath_b;
      _get16 = jjmath_a;
      _get17 = jjmath_b;
      isOdd = (f64)((_get14 - (_get15 * trunc((_get16 / _get17)))) == 1);
      isOddjjtype = 2;
      _get18 = exponent;
      // if 
        if (_get18 > 0) {
          _get19 = isOdd;
          // if 
            if (((u32)(_get19)) != 0) {
              return -Infinity;
            }
          // end
          j425:;
          return Infinity;
        }
      // end
      j424:;
      _get20 = isOdd;
      // if 
        if (((u32)(_get20)) != 0) {
          return 0;
        }
      // end
      j426:;
      return 0;
    }
  // end
  j420:;
  _get21 = base;
  // if 
    if (_get21 == 0) {
      _get22 = base;
      // if 
        if ((1 / _get22) == Infinity) {
          _get23 = exponent;
          // if 
            if (_get23 > 0) {
              return 0;
            }
          // end
          j429:;
          return Infinity;
        }
      // end
      j428:;
      _get24 = exponent;
      jjmath_a = dong_porf_porf_todo__Math_abs(_get24);
      _get25 = jjmath_a;
      jjmath_b = 2;
      _get26 = jjmath_b;
      _get27 = jjmath_a;
      _get28 = jjmath_b;
      isOdd = (f64)((_get25 - (_get26 * trunc((_get27 / _get28)))) == 1);
      isOddjjtype = 2;
      _get29 = exponent;
      // if 
        if (_get29 > 0) {
          _get30 = isOdd;
          // if 
            if (((u32)(_get30)) != 0) {
              return 0;
            }
          // end
          j431:;
          return 0;
        }
      // end
      j430:;
      _get31 = isOdd;
      // if 
        if (((u32)(_get31)) != 0) {
          return -Infinity;
        }
      // end
      j432:;
      return Infinity;
    }
  // end
  j427:;
  _get32 = exponent;
  // if 
    if (_get32 == Infinity) {
      _get33 = base;
      abs = dong_porf_porf_todo__Math_abs(_get33);
      absjjtype = 1;
      _get34 = abs;
      // if 
        if (_get34 > 1) {
          return Infinity;
        }
      // end
      j434:;
      _get35 = abs;
      // if 
        if (_get35 == 1) {
          return NaN;
        }
      // end
      j435:;
      return 0;
    }
  // end
  j433:;
  _get36 = exponent;
  // if 
    if (_get36 == (-Infinity)) {
      _get37 = base;
      abs = dong_porf_porf_todo__Math_abs(_get37);
      absjjtype = 1;
      _get38 = abs;
      // if 
        if (_get38 > 1) {
          return 0;
        }
      // end
      j437:;
      _get39 = abs;
      // if 
        if (_get39 == 1) {
          return NaN;
        }
      // end
      j438:;
      return Infinity;
    }
  // end
  j436:;
  _get40 = base;
  // if 
    if (_get40 < 0) {
      _get41 = exponent;
      // if 
        if (dong_porf_porf_todo__Number_isInteger(_get41) == 0) {
          return NaN;
        }
      // end
      j440:;
    }
  // end
  j439:;
  _get42 = base;
  // if 
    if (_get42 == 2.718281828459045) {
      _get43 = exponent;
      return dong_porf_porf_todo__Math_exp(_get43, 1);
    }
  // end
  j441:;
  _get44 = base;
  currentBase = _get44;
  _get45 = exponent;
  currentExponent = dong_porf_porf_todo__Math_abs(_get45);
  result = 1;
  // loop 
  j449:;
    _get46 = currentExponent;
    // if 
      if (_get46 > 0) {
        _get47 = currentExponent;
        // if 
          if (_get47 >= 1) {
            _get48 = currentExponent;
            // if 
              if (((i32)(_get48) & 1) != 0) {
                _get49 = result;
                _get50 = currentBase;
                result = _get49 * _get50;
              }
            // end
            j452:;
            _get51 = currentBase;
            _get52 = currentBase;
            currentBase = _get51 * _get52;
            _get53 = currentExponent;
            currentExponent = dong_porf_porf_todo__Math_trunc(_get53 / 2);
          } else {
            _get54 = result;
            _get55 = currentExponent;
            _get56 = currentBase;
            result = _get54 * dong_porf_porf_todo__Math_exp(_get55 * dong_porf_porf_todo__Math_log(dong_porf_porf_todo__Math_abs(_get56), 1), 1);
            goto j450;
          }
        // end
        j451:;
        goto j449;
      }
    // end
    j450:;
  // end
  _get57 = exponent;
  // if f64
  f64 _r471;
    if (_get57 < 0) {
      _get58 = result;
      _r471 = 1 / _get58;
    } else {
      _get59 = result;
      _r471 = _get59;
    }
  // end
  j471:;
  return _r471;
}

static f64 dong_porf_porf_todo__Porffor_stn_float(f64 str, i32 strjjtype, f64 i, i32 ijjtype) {
  f64 _get50;
  f64 _get49;
  f64 _get48;
  f64 _get47;
  f64 _get46;
  f64 _get45;
  f64 _get44;
  f64 _get43;
  f64 _get42;
  i32 _get41;
  f64 _get40;
  i32 _get39;
  f64 _get38;
  i32 _get37;
  f64 _get36;
  f64 _get35;
  f64 _get34;
  f64 _get33;
  f64 _get32;
  f64 _get31;
  f64 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  i32 _get26;
  f64 _get25;
  i32 _get24;
  f64 _get23;
  f64 _get22;
  f64 _get21;
  i32 _get20;
  f64 _get19;
  i32 _get18;
  f64 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  f64 _get12;
  f64 _get11;
  i32 _get10;
  f64 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 n = 0;
  f64 dec = 0;
  f64 len = 0;
  f64 chr = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 jjlast_type = 0;
  i32 logictmpi = 0;
  f64 exp = 0;

  n = 0;
  dec = 0;
  _get0 = str;
  len = (f64)(i32_load(1, 0, (u32)(_get0)));
  _get1 = len;
  _get2 = i;
  // if 
    if ((_get1 - _get2) == 0) {
      return NaN;
    }
  // end
  j378:;
  // loop 
  j379:;
    _get3 = i;
    _get4 = len;
    // if 
      if (_get3 < _get4) {
        _get5 = str;
        jjproto_target = _get5;
        _get6 = strjjtype;
        jjproto_targetjjtype = _get6;
        _get7 = strjjtype;
        jjtypeswitch_tmp1 = _get7;
        // block f64
        f64 _r381;
          _get8 = jjtypeswitch_tmp1;
          // if 
            if (_get8 == 33) {
              _get9 = jjproto_target;
              _get10 = jjproto_targetjjtype;
              _get11 = i;
              _get12 = i;
              i = _get12 + 1;
              const struct ReturnValue _0 = dong_porf_porf_todo__String_prototype_charCodeAt(_get9, _get10, _get11, 1);
              jjlast_type = _0.type;
              _r381 = _0.value;
              goto j381;
            }
          // end
          j382:;
          _get13 = jjtypeswitch_tmp1;
          // if 
            if (_get13 == 67) {
              _get14 = jjproto_target;
              _get15 = jjproto_targetjjtype;
              _get16 = i;
              _get17 = i;
              i = _get17 + 1;
              const struct ReturnValue _1 = dong_porf_porf_todo__String_prototype_charCodeAt(_get14, _get15, _get16, 1);
              jjlast_type = _1.type;
              _r381 = _1.value;
              goto j381;
            }
          // end
          j383:;
          _get18 = jjtypeswitch_tmp1;
          // if 
            if (_get18 == 195) {
              _get19 = jjproto_target;
              _get20 = jjproto_targetjjtype;
              _get21 = i;
              _get22 = i;
              i = _get22 + 1;
              const struct ReturnValue _2 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get19, _get20, _get21, 1);
              jjlast_type = _2.type;
              _r381 = _2.value;
              goto j381;
            }
          // end
          j384:;
          _r381 = 0;
        // end
        j381:;
        chr = _r381;
        _get23 = chr;
        logictmpi = _get23 >= 48;
        _get24 = logictmpi;
        // if i32
        i32 _r385;
          if ((_get24) != 0) {
            _get25 = chr;
            jjlast_type = 2;
            _r385 = _get25 <= 57;
          } else {
            _get26 = logictmpi;
            jjlast_type = 2;
            _r385 = _get26;
          }
        // end
        j385:;
        // if 
          if ((_r385) != 0) {
            _get27 = dec;
            // if 
              if (((u32)(_get27)) != 0) {
                _get28 = dec;
                dec = _get28 * 10;
                _get29 = n;
                _get30 = chr;
                _get31 = dec;
                n = _get29 + ((_get30 - 48) / _get31);
              } else {
                _get32 = n;
                _get33 = chr;
                n = ((_get32 * 10) + _get33) - 48;
              }
            // end
            j387:;
          } else {
            _get34 = chr;
            // if 
              if (_get34 == 46) {
                _get35 = dec;
                // if 
                  if (((u32)(_get35)) != 0) {
                    return NaN;
                  }
                // end
                j389:;
                dec = 1;
              } else {
                _get36 = chr;
                logictmpi = _get36 == 69;
                _get37 = logictmpi;
                // if i32
                i32 _r390;
                  if ((_get37) == 0) {
                    _get38 = chr;
                    jjlast_type = 2;
                    _r390 = _get38 == 101;
                  } else {
                    _get39 = logictmpi;
                    jjlast_type = 2;
                    _r390 = _get39;
                  }
                // end
                j390:;
                // if 
                  if ((_r390) != 0) {
                    _get40 = str;
                    _get41 = strjjtype;
                    _get42 = i;
                    _get43 = len;
                    exp = dong_porf_porf_todo__Porffor_parseExp(_get40, _get41, _get42, 1, _get43, 1, 1, 2);
                    _get44 = exp;
                    // if 
                      if (((u32)(dong_porf_porf_todo__Number_isNaN(_get44))) != 0) {
                        return NaN;
                      }
                    // end
                    j414:;
                    _get45 = exp;
                    // if 
                      if (_get45 < 0) {
                        _get46 = n;
                        _get47 = exp;
                        return _get46 / dong_porf_porf_todo__Math_pow(10, 1, -_get47, 1);
                      }
                    // end
                    j415:;
                    _get48 = n;
                    _get49 = exp;
                    return _get48 * dong_porf_porf_todo__Math_pow(10, 1, _get49, 1);
                  } else {
                    return NaN;
                  }
                // end
                j391:;
              }
            // end
            j388:;
          }
        // end
        j386:;
        goto j379;
      }
    // end
    j380:;
  // end
  _get50 = n;
  return _get50;
}

static f64 dong_porf_porf_todo__ecma262_StringToNumber(f64 str, i32 strjjtype) {
  f64 _get233;
  f64 _get232;
  f64 _get231;
  f64 _get230;
  i32 _get229;
  f64 _get228;
  f64 _get227;
  f64 _get226;
  f64 _get225;
  i32 _get224;
  i32 _get223;
  f64 _get222;
  i32 _get221;
  f64 _get220;
  i32 _get219;
  f64 _get218;
  i32 _get217;
  f64 _get216;
  i32 _get215;
  f64 _get214;
  i32 _get213;
  f64 _get212;
  i32 _get211;
  i32 _get210;
  i32 _get209;
  f64 _get208;
  i32 _get207;
  i32 _get206;
  i32 _get205;
  i32 _get204;
  i32 _get203;
  i32 _get202;
  i32 _get201;
  i32 _get200;
  f64 _get199;
  i32 _get198;
  f64 _get197;
  i32 _get196;
  f64 _get195;
  i32 _get194;
  f64 _get193;
  i32 _get192;
  f64 _get191;
  i32 _get190;
  f64 _get189;
  i32 _get188;
  i32 _get187;
  i32 _get186;
  f64 _get185;
  i32 _get184;
  i32 _get183;
  i32 _get182;
  i32 _get181;
  i32 _get180;
  i32 _get179;
  i32 _get178;
  i32 _get177;
  f64 _get176;
  i32 _get175;
  f64 _get174;
  i32 _get173;
  f64 _get172;
  i32 _get171;
  f64 _get170;
  i32 _get169;
  f64 _get168;
  i32 _get167;
  f64 _get166;
  i32 _get165;
  i32 _get164;
  i32 _get163;
  f64 _get162;
  i32 _get161;
  i32 _get160;
  i32 _get159;
  i32 _get158;
  i32 _get157;
  i32 _get156;
  i32 _get155;
  i32 _get154;
  f64 _get153;
  i32 _get152;
  f64 _get151;
  i32 _get150;
  f64 _get149;
  i32 _get148;
  f64 _get147;
  i32 _get146;
  f64 _get145;
  i32 _get144;
  f64 _get143;
  i32 _get142;
  i32 _get141;
  i32 _get140;
  f64 _get139;
  i32 _get138;
  i32 _get137;
  i32 _get136;
  i32 _get135;
  i32 _get134;
  i32 _get133;
  i32 _get132;
  i32 _get131;
  f64 _get130;
  i32 _get129;
  f64 _get128;
  i32 _get127;
  f64 _get126;
  i32 _get125;
  f64 _get124;
  i32 _get123;
  f64 _get122;
  i32 _get121;
  f64 _get120;
  i32 _get119;
  i32 _get118;
  i32 _get117;
  f64 _get116;
  i32 _get115;
  i32 _get114;
  i32 _get113;
  i32 _get112;
  i32 _get111;
  i32 _get110;
  i32 _get109;
  f64 _get108;
  i32 _get107;
  f64 _get106;
  i32 _get105;
  f64 _get104;
  i32 _get103;
  f64 _get102;
  i32 _get101;
  f64 _get100;
  i32 _get99;
  f64 _get98;
  i32 _get97;
  i32 _get96;
  i32 _get95;
  f64 _get94;
  i32 _get93;
  f64 _get92;
  i32 _get91;
  f64 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  f64 _get86;
  i32 _get85;
  f64 _get84;
  i32 _get83;
  f64 _get82;
  i32 _get81;
  i32 _get80;
  i32 _get79;
  f64 _get78;
  i32 _get77;
  f64 _get76;
  i32 _get75;
  f64 _get74;
  i32 _get73;
  f64 _get72;
  i32 _get71;
  f64 _get70;
  i32 _get69;
  f64 _get68;
  i32 _get67;
  f64 _get66;
  i32 _get65;
  i32 _get64;
  i32 _get63;
  f64 _get62;
  i32 _get61;
  f64 _get60;
  f64 _get59;
  f64 _get58;
  f64 _get57;
  i32 _get56;
  f64 _get55;
  i32 _get54;
  f64 _get53;
  i32 _get52;
  f64 _get51;
  i32 _get50;
  f64 _get49;
  i32 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  f64 _get41;
  i32 _get40;
  f64 _get39;
  f64 _get38;
  i32 _get37;
  f64 _get36;
  i32 _get35;
  i32 _get34;
  f64 _get33;
  i32 _get32;
  i32 _get31;
  f64 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  f64 _get26;
  i32 _get25;
  f64 _get24;
  i32 _get23;
  i32 _get22;
  f64 _get21;
  i32 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  f64 _get14;
  f64 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  i32 _get5;
  f64 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  f64 _get0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 jjlast_type = 0;
  f64 first = 0;
  f64 second = 0;
  i32 logictmpi = 0;
  f64 i = 0;
  f64 negative = 0;
  i32 jjlogicinner_tmp_int = 0;
  f64 n = 0;

  _get0 = str;
  jjproto_target = _get0;
  _get1 = strjjtype;
  jjproto_targetjjtype = _get1;
  _get2 = strjjtype;
  jjtypeswitch_tmp1 = _get2;
  // block f64
  f64 _r259;
    _get3 = jjtypeswitch_tmp1;
    // if 
      if (_get3 == 33) {
        _get4 = jjproto_target;
        _get5 = jjproto_targetjjtype;
        const struct ReturnValue _0 = dong_porf_porf_todo__String_prototype_trim((i32)(_get4), _get5);
        jjlast_type = _0.type;
        _r259 = (f64)(_0.value);
        goto j259;
      }
    // end
    j260:;
    _get6 = jjtypeswitch_tmp1;
    // if 
      if (_get6 == 67) {
        _get7 = jjproto_target;
        _get8 = jjproto_targetjjtype;
        const struct ReturnValue _1 = dong_porf_porf_todo__String_prototype_trim((i32)(_get7), _get8);
        jjlast_type = _1.type;
        _r259 = (f64)(_1.value);
        goto j259;
      }
    // end
    j278:;
    _get9 = jjtypeswitch_tmp1;
    // if 
      if (_get9 == 195) {
        _get10 = jjproto_target;
        _get11 = jjproto_targetjjtype;
        const struct ReturnValue _2 = dong_porf_porf_todo__ByteString_prototype_trim((i32)(_get10), _get11);
        jjlast_type = _2.type;
        _r259 = (f64)(_2.value);
        goto j259;
      }
    // end
    j279:;
    _r259 = 0;
  // end
  j259:;
  str = _r259;
  _get12 = jjlast_type;
  strjjtype = _get12;
  _get13 = str;
  // if 
    if ((f64)(i32_load(1, 0, (u32)(_get13))) == 0) {
      return 0;
    }
  // end
  j288:;
  _get14 = str;
  jjproto_target = _get14;
  _get15 = strjjtype;
  jjproto_targetjjtype = _get15;
  _get16 = strjjtype;
  jjtypeswitch_tmp1 = _get16;
  // block f64
  f64 _r289;
    _get17 = jjtypeswitch_tmp1;
    // if 
      if (_get17 == 33) {
        _get18 = jjproto_target;
        _get19 = jjproto_targetjjtype;
        const struct ReturnValue _3 = dong_porf_porf_todo__String_prototype_charCodeAt(_get18, _get19, 0, 1);
        jjlast_type = _3.type;
        _r289 = _3.value;
        goto j289;
      }
    // end
    j290:;
    _get20 = jjtypeswitch_tmp1;
    // if 
      if (_get20 == 67) {
        _get21 = jjproto_target;
        _get22 = jjproto_targetjjtype;
        const struct ReturnValue _4 = dong_porf_porf_todo__String_prototype_charCodeAt(_get21, _get22, 0, 1);
        jjlast_type = _4.type;
        _r289 = _4.value;
        goto j289;
      }
    // end
    j295:;
    _get23 = jjtypeswitch_tmp1;
    // if 
      if (_get23 == 195) {
        _get24 = jjproto_target;
        _get25 = jjproto_targetjjtype;
        const struct ReturnValue _5 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get24, _get25, 0, 1);
        jjlast_type = _5.type;
        _r289 = _5.value;
        goto j289;
      }
    // end
    j296:;
    _r289 = 0;
  // end
  j289:;
  first = _r289;
  _get26 = str;
  jjproto_target = _get26;
  _get27 = strjjtype;
  jjproto_targetjjtype = _get27;
  _get28 = strjjtype;
  jjtypeswitch_tmp1 = _get28;
  // block f64
  f64 _r298;
    _get29 = jjtypeswitch_tmp1;
    // if 
      if (_get29 == 33) {
        _get30 = jjproto_target;
        _get31 = jjproto_targetjjtype;
        const struct ReturnValue _6 = dong_porf_porf_todo__String_prototype_charCodeAt(_get30, _get31, 1, 1);
        jjlast_type = _6.type;
        _r298 = _6.value;
        goto j298;
      }
    // end
    j299:;
    _get32 = jjtypeswitch_tmp1;
    // if 
      if (_get32 == 67) {
        _get33 = jjproto_target;
        _get34 = jjproto_targetjjtype;
        const struct ReturnValue _7 = dong_porf_porf_todo__String_prototype_charCodeAt(_get33, _get34, 1, 1);
        jjlast_type = _7.type;
        _r298 = _7.value;
        goto j298;
      }
    // end
    j300:;
    _get35 = jjtypeswitch_tmp1;
    // if 
      if (_get35 == 195) {
        _get36 = jjproto_target;
        _get37 = jjproto_targetjjtype;
        const struct ReturnValue _8 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get36, _get37, 1, 1);
        jjlast_type = _8.type;
        _r298 = _8.value;
        goto j298;
      }
    // end
    j301:;
    _r298 = 0;
  // end
  j298:;
  second = _r298;
  _get38 = first;
  // if 
    if (_get38 == 48) {
      _get39 = second;
      logictmpi = _get39 == 120;
      _get40 = logictmpi;
      // if i32
      i32 _r303;
        if ((_get40) == 0) {
          _get41 = second;
          jjlast_type = 2;
          _r303 = _get41 == 88;
        } else {
          _get42 = logictmpi;
          jjlast_type = 2;
          _r303 = _get42;
        }
      // end
      j303:;
      // if 
        if ((_r303) != 0) {
          _get43 = str;
          _get44 = strjjtype;
          return dong_porf_porf_todo__Porffor_stn_int(_get43, _get44, 16, 1, 2, 1);
        }
      // end
      j304:;
      _get45 = second;
      logictmpi = _get45 == 111;
      _get46 = logictmpi;
      // if i32
      i32 _r320;
        if ((_get46) == 0) {
          _get47 = second;
          jjlast_type = 2;
          _r320 = _get47 == 79;
        } else {
          _get48 = logictmpi;
          jjlast_type = 2;
          _r320 = _get48;
        }
      // end
      j320:;
      // if 
        if ((_r320) != 0) {
          _get49 = str;
          _get50 = strjjtype;
          return dong_porf_porf_todo__Porffor_stn_int(_get49, _get50, 8, 1, 2, 1);
        }
      // end
      j321:;
      _get51 = second;
      logictmpi = _get51 == 98;
      _get52 = logictmpi;
      // if i32
      i32 _r322;
        if ((_get52) == 0) {
          _get53 = second;
          jjlast_type = 2;
          _r322 = _get53 == 66;
        } else {
          _get54 = logictmpi;
          jjlast_type = 2;
          _r322 = _get54;
        }
      // end
      j322:;
      // if 
        if ((_r322) != 0) {
          _get55 = str;
          _get56 = strjjtype;
          return dong_porf_porf_todo__Porffor_stn_int(_get55, _get56, 2, 1, 2, 1);
        }
      // end
      j323:;
    }
  // end
  j302:;
  i = 0;
  negative = 0;
  _get57 = first;
  // if 
    if (_get57 == 43) {
      i = 1;
    }
  // end
  j324:;
  _get58 = first;
  // if 
    if (_get58 == 45) {
      negative = 1;
      i = 1;
    }
  // end
  j325:;
  _get59 = i;
  _get60 = str;
  logictmpi = (_get59 + 8) == (f64)(i32_load(1, 0, (u32)(_get60)));
  _get61 = logictmpi;
  // if i32
  i32 _r326;
    if ((_get61) != 0) {
      _get62 = str;
      jjproto_target = _get62;
      _get63 = strjjtype;
      jjproto_targetjjtype = _get63;
      _get64 = strjjtype;
      jjtypeswitch_tmp1 = _get64;
      // block f64
      f64 _r327;
        _get65 = jjtypeswitch_tmp1;
        // if 
          if (_get65 == 33) {
            _get66 = jjproto_target;
            _get67 = jjproto_targetjjtype;
            _get68 = i;
            const struct ReturnValue _9 = dong_porf_porf_todo__String_prototype_charCodeAt(_get66, _get67, _get68, 1);
            jjlast_type = _9.type;
            _r327 = _9.value;
            goto j327;
          }
        // end
        j328:;
        _get69 = jjtypeswitch_tmp1;
        // if 
          if (_get69 == 67) {
            _get70 = jjproto_target;
            _get71 = jjproto_targetjjtype;
            _get72 = i;
            const struct ReturnValue _10 = dong_porf_porf_todo__String_prototype_charCodeAt(_get70, _get71, _get72, 1);
            jjlast_type = _10.type;
            _r327 = _10.value;
            goto j327;
          }
        // end
        j329:;
        _get73 = jjtypeswitch_tmp1;
        // if 
          if (_get73 == 195) {
            _get74 = jjproto_target;
            _get75 = jjproto_targetjjtype;
            _get76 = i;
            const struct ReturnValue _11 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get74, _get75, _get76, 1);
            jjlast_type = _11.type;
            _r327 = _11.value;
            goto j327;
          }
        // end
        j330:;
        _r327 = 0;
      // end
      j327:;
      jjlast_type = 2;
      _r326 = _r327 == 73;
    } else {
      _get77 = logictmpi;
      jjlast_type = 2;
      _r326 = _get77;
    }
  // end
  j326:;
  // if 
    if ((_r326) != 0) {
      _get78 = str;
      jjproto_target = _get78;
      _get79 = strjjtype;
      jjproto_targetjjtype = _get79;
      _get80 = strjjtype;
      jjtypeswitch_tmp1 = _get80;
      // block f64
      f64 _r332;
        _get81 = jjtypeswitch_tmp1;
        // if 
          if (_get81 == 33) {
            _get82 = jjproto_target;
            _get83 = jjproto_targetjjtype;
            _get84 = i;
            const struct ReturnValue _12 = dong_porf_porf_todo__String_prototype_charCodeAt(_get82, _get83, _get84 + 1, 1);
            jjlast_type = _12.type;
            _r332 = _12.value;
            goto j332;
          }
        // end
        j333:;
        _get85 = jjtypeswitch_tmp1;
        // if 
          if (_get85 == 67) {
            _get86 = jjproto_target;
            _get87 = jjproto_targetjjtype;
            _get88 = i;
            const struct ReturnValue _13 = dong_porf_porf_todo__String_prototype_charCodeAt(_get86, _get87, _get88 + 1, 1);
            jjlast_type = _13.type;
            _r332 = _13.value;
            goto j332;
          }
        // end
        j334:;
        _get89 = jjtypeswitch_tmp1;
        // if 
          if (_get89 == 195) {
            _get90 = jjproto_target;
            _get91 = jjproto_targetjjtype;
            _get92 = i;
            const struct ReturnValue _14 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get90, _get91, _get92 + 1, 1);
            jjlast_type = _14.type;
            _r332 = _14.value;
            goto j332;
          }
        // end
        j335:;
        _r332 = 0;
      // end
      j332:;
      logictmpi = _r332 == 110;
      _get93 = logictmpi;
      // if i32
      i32 _r336;
        if ((_get93) != 0) {
          _get94 = str;
          jjproto_target = _get94;
          _get95 = strjjtype;
          jjproto_targetjjtype = _get95;
          _get96 = strjjtype;
          jjtypeswitch_tmp1 = _get96;
          // block f64
          f64 _r337;
            _get97 = jjtypeswitch_tmp1;
            // if 
              if (_get97 == 33) {
                _get98 = jjproto_target;
                _get99 = jjproto_targetjjtype;
                _get100 = i;
                const struct ReturnValue _15 = dong_porf_porf_todo__String_prototype_charCodeAt(_get98, _get99, _get100 + 2, 1);
                jjlast_type = _15.type;
                _r337 = _15.value;
                goto j337;
              }
            // end
            j338:;
            _get101 = jjtypeswitch_tmp1;
            // if 
              if (_get101 == 67) {
                _get102 = jjproto_target;
                _get103 = jjproto_targetjjtype;
                _get104 = i;
                const struct ReturnValue _16 = dong_porf_porf_todo__String_prototype_charCodeAt(_get102, _get103, _get104 + 2, 1);
                jjlast_type = _16.type;
                _r337 = _16.value;
                goto j337;
              }
            // end
            j339:;
            _get105 = jjtypeswitch_tmp1;
            // if 
              if (_get105 == 195) {
                _get106 = jjproto_target;
                _get107 = jjproto_targetjjtype;
                _get108 = i;
                const struct ReturnValue _17 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get106, _get107, _get108 + 2, 1);
                jjlast_type = _17.type;
                _r337 = _17.value;
                goto j337;
              }
            // end
            j340:;
            _r337 = 0;
          // end
          j337:;
          jjlast_type = 2;
          _r336 = _r337 == 102;
        } else {
          _get109 = logictmpi;
          jjlast_type = 2;
          _r336 = _get109;
        }
      // end
      j336:;
      logictmpi = _r336;
      _get110 = logictmpi;
      jjlogicinner_tmp_int = _get110;
      _get111 = jjlast_type;
      jjtypeswitch_tmp1 = _get111;
      // block i32
      i32 _r341;
        _get112 = jjtypeswitch_tmp1;
        _get113 = jjtypeswitch_tmp1;
        // if 
          if (((_get112 == 67) | (_get113 == 195)) != 0) {
            _get114 = jjlogicinner_tmp_int;
            _r341 = i32_load(1, 0, _get114);
            goto j341;
          }
        // end
        j342:;
        _get115 = jjlogicinner_tmp_int;
        _r341 = _get115;
      // end
      j341:;
      // if i32
      i32 _r343;
        if ((_r341) != 0) {
          _get116 = str;
          jjproto_target = _get116;
          _get117 = strjjtype;
          jjproto_targetjjtype = _get117;
          _get118 = strjjtype;
          jjtypeswitch_tmp1 = _get118;
          // block f64
          f64 _r344;
            _get119 = jjtypeswitch_tmp1;
            // if 
              if (_get119 == 33) {
                _get120 = jjproto_target;
                _get121 = jjproto_targetjjtype;
                _get122 = i;
                const struct ReturnValue _18 = dong_porf_porf_todo__String_prototype_charCodeAt(_get120, _get121, _get122 + 3, 1);
                jjlast_type = _18.type;
                _r344 = _18.value;
                goto j344;
              }
            // end
            j345:;
            _get123 = jjtypeswitch_tmp1;
            // if 
              if (_get123 == 67) {
                _get124 = jjproto_target;
                _get125 = jjproto_targetjjtype;
                _get126 = i;
                const struct ReturnValue _19 = dong_porf_porf_todo__String_prototype_charCodeAt(_get124, _get125, _get126 + 3, 1);
                jjlast_type = _19.type;
                _r344 = _19.value;
                goto j344;
              }
            // end
            j346:;
            _get127 = jjtypeswitch_tmp1;
            // if 
              if (_get127 == 195) {
                _get128 = jjproto_target;
                _get129 = jjproto_targetjjtype;
                _get130 = i;
                const struct ReturnValue _20 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get128, _get129, _get130 + 3, 1);
                jjlast_type = _20.type;
                _r344 = _20.value;
                goto j344;
              }
            // end
            j347:;
            _r344 = 0;
          // end
          j344:;
          jjlast_type = 2;
          _r343 = _r344 == 105;
        } else {
          _get131 = logictmpi;
          _get132 = jjlast_type;
          jjlast_type = _get132;
          _r343 = _get131;
        }
      // end
      j343:;
      logictmpi = _r343;
      _get133 = logictmpi;
      jjlogicinner_tmp_int = _get133;
      _get134 = jjlast_type;
      jjtypeswitch_tmp1 = _get134;
      // block i32
      i32 _r348;
        _get135 = jjtypeswitch_tmp1;
        _get136 = jjtypeswitch_tmp1;
        // if 
          if (((_get135 == 67) | (_get136 == 195)) != 0) {
            _get137 = jjlogicinner_tmp_int;
            _r348 = i32_load(1, 0, _get137);
            goto j348;
          }
        // end
        j349:;
        _get138 = jjlogicinner_tmp_int;
        _r348 = _get138;
      // end
      j348:;
      // if i32
      i32 _r350;
        if ((_r348) != 0) {
          _get139 = str;
          jjproto_target = _get139;
          _get140 = strjjtype;
          jjproto_targetjjtype = _get140;
          _get141 = strjjtype;
          jjtypeswitch_tmp1 = _get141;
          // block f64
          f64 _r351;
            _get142 = jjtypeswitch_tmp1;
            // if 
              if (_get142 == 33) {
                _get143 = jjproto_target;
                _get144 = jjproto_targetjjtype;
                _get145 = i;
                const struct ReturnValue _21 = dong_porf_porf_todo__String_prototype_charCodeAt(_get143, _get144, _get145 + 4, 1);
                jjlast_type = _21.type;
                _r351 = _21.value;
                goto j351;
              }
            // end
            j352:;
            _get146 = jjtypeswitch_tmp1;
            // if 
              if (_get146 == 67) {
                _get147 = jjproto_target;
                _get148 = jjproto_targetjjtype;
                _get149 = i;
                const struct ReturnValue _22 = dong_porf_porf_todo__String_prototype_charCodeAt(_get147, _get148, _get149 + 4, 1);
                jjlast_type = _22.type;
                _r351 = _22.value;
                goto j351;
              }
            // end
            j353:;
            _get150 = jjtypeswitch_tmp1;
            // if 
              if (_get150 == 195) {
                _get151 = jjproto_target;
                _get152 = jjproto_targetjjtype;
                _get153 = i;
                const struct ReturnValue _23 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get151, _get152, _get153 + 4, 1);
                jjlast_type = _23.type;
                _r351 = _23.value;
                goto j351;
              }
            // end
            j354:;
            _r351 = 0;
          // end
          j351:;
          jjlast_type = 2;
          _r350 = _r351 == 110;
        } else {
          _get154 = logictmpi;
          _get155 = jjlast_type;
          jjlast_type = _get155;
          _r350 = _get154;
        }
      // end
      j350:;
      logictmpi = _r350;
      _get156 = logictmpi;
      jjlogicinner_tmp_int = _get156;
      _get157 = jjlast_type;
      jjtypeswitch_tmp1 = _get157;
      // block i32
      i32 _r355;
        _get158 = jjtypeswitch_tmp1;
        _get159 = jjtypeswitch_tmp1;
        // if 
          if (((_get158 == 67) | (_get159 == 195)) != 0) {
            _get160 = jjlogicinner_tmp_int;
            _r355 = i32_load(1, 0, _get160);
            goto j355;
          }
        // end
        j356:;
        _get161 = jjlogicinner_tmp_int;
        _r355 = _get161;
      // end
      j355:;
      // if i32
      i32 _r357;
        if ((_r355) != 0) {
          _get162 = str;
          jjproto_target = _get162;
          _get163 = strjjtype;
          jjproto_targetjjtype = _get163;
          _get164 = strjjtype;
          jjtypeswitch_tmp1 = _get164;
          // block f64
          f64 _r358;
            _get165 = jjtypeswitch_tmp1;
            // if 
              if (_get165 == 33) {
                _get166 = jjproto_target;
                _get167 = jjproto_targetjjtype;
                _get168 = i;
                const struct ReturnValue _24 = dong_porf_porf_todo__String_prototype_charCodeAt(_get166, _get167, _get168 + 5, 1);
                jjlast_type = _24.type;
                _r358 = _24.value;
                goto j358;
              }
            // end
            j359:;
            _get169 = jjtypeswitch_tmp1;
            // if 
              if (_get169 == 67) {
                _get170 = jjproto_target;
                _get171 = jjproto_targetjjtype;
                _get172 = i;
                const struct ReturnValue _25 = dong_porf_porf_todo__String_prototype_charCodeAt(_get170, _get171, _get172 + 5, 1);
                jjlast_type = _25.type;
                _r358 = _25.value;
                goto j358;
              }
            // end
            j360:;
            _get173 = jjtypeswitch_tmp1;
            // if 
              if (_get173 == 195) {
                _get174 = jjproto_target;
                _get175 = jjproto_targetjjtype;
                _get176 = i;
                const struct ReturnValue _26 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get174, _get175, _get176 + 5, 1);
                jjlast_type = _26.type;
                _r358 = _26.value;
                goto j358;
              }
            // end
            j361:;
            _r358 = 0;
          // end
          j358:;
          jjlast_type = 2;
          _r357 = _r358 == 105;
        } else {
          _get177 = logictmpi;
          _get178 = jjlast_type;
          jjlast_type = _get178;
          _r357 = _get177;
        }
      // end
      j357:;
      logictmpi = _r357;
      _get179 = logictmpi;
      jjlogicinner_tmp_int = _get179;
      _get180 = jjlast_type;
      jjtypeswitch_tmp1 = _get180;
      // block i32
      i32 _r362;
        _get181 = jjtypeswitch_tmp1;
        _get182 = jjtypeswitch_tmp1;
        // if 
          if (((_get181 == 67) | (_get182 == 195)) != 0) {
            _get183 = jjlogicinner_tmp_int;
            _r362 = i32_load(1, 0, _get183);
            goto j362;
          }
        // end
        j363:;
        _get184 = jjlogicinner_tmp_int;
        _r362 = _get184;
      // end
      j362:;
      // if i32
      i32 _r364;
        if ((_r362) != 0) {
          _get185 = str;
          jjproto_target = _get185;
          _get186 = strjjtype;
          jjproto_targetjjtype = _get186;
          _get187 = strjjtype;
          jjtypeswitch_tmp1 = _get187;
          // block f64
          f64 _r365;
            _get188 = jjtypeswitch_tmp1;
            // if 
              if (_get188 == 33) {
                _get189 = jjproto_target;
                _get190 = jjproto_targetjjtype;
                _get191 = i;
                const struct ReturnValue _27 = dong_porf_porf_todo__String_prototype_charCodeAt(_get189, _get190, _get191 + 6, 1);
                jjlast_type = _27.type;
                _r365 = _27.value;
                goto j365;
              }
            // end
            j366:;
            _get192 = jjtypeswitch_tmp1;
            // if 
              if (_get192 == 67) {
                _get193 = jjproto_target;
                _get194 = jjproto_targetjjtype;
                _get195 = i;
                const struct ReturnValue _28 = dong_porf_porf_todo__String_prototype_charCodeAt(_get193, _get194, _get195 + 6, 1);
                jjlast_type = _28.type;
                _r365 = _28.value;
                goto j365;
              }
            // end
            j367:;
            _get196 = jjtypeswitch_tmp1;
            // if 
              if (_get196 == 195) {
                _get197 = jjproto_target;
                _get198 = jjproto_targetjjtype;
                _get199 = i;
                const struct ReturnValue _29 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get197, _get198, _get199 + 6, 1);
                jjlast_type = _29.type;
                _r365 = _29.value;
                goto j365;
              }
            // end
            j368:;
            _r365 = 0;
          // end
          j365:;
          jjlast_type = 2;
          _r364 = _r365 == 116;
        } else {
          _get200 = logictmpi;
          _get201 = jjlast_type;
          jjlast_type = _get201;
          _r364 = _get200;
        }
      // end
      j364:;
      logictmpi = _r364;
      _get202 = logictmpi;
      jjlogicinner_tmp_int = _get202;
      _get203 = jjlast_type;
      jjtypeswitch_tmp1 = _get203;
      // block i32
      i32 _r369;
        _get204 = jjtypeswitch_tmp1;
        _get205 = jjtypeswitch_tmp1;
        // if 
          if (((_get204 == 67) | (_get205 == 195)) != 0) {
            _get206 = jjlogicinner_tmp_int;
            _r369 = i32_load(1, 0, _get206);
            goto j369;
          }
        // end
        j370:;
        _get207 = jjlogicinner_tmp_int;
        _r369 = _get207;
      // end
      j369:;
      // if i32
      i32 _r371;
        if ((_r369) != 0) {
          _get208 = str;
          jjproto_target = _get208;
          _get209 = strjjtype;
          jjproto_targetjjtype = _get209;
          _get210 = strjjtype;
          jjtypeswitch_tmp1 = _get210;
          // block f64
          f64 _r372;
            _get211 = jjtypeswitch_tmp1;
            // if 
              if (_get211 == 33) {
                _get212 = jjproto_target;
                _get213 = jjproto_targetjjtype;
                _get214 = i;
                const struct ReturnValue _30 = dong_porf_porf_todo__String_prototype_charCodeAt(_get212, _get213, _get214 + 7, 1);
                jjlast_type = _30.type;
                _r372 = _30.value;
                goto j372;
              }
            // end
            j373:;
            _get215 = jjtypeswitch_tmp1;
            // if 
              if (_get215 == 67) {
                _get216 = jjproto_target;
                _get217 = jjproto_targetjjtype;
                _get218 = i;
                const struct ReturnValue _31 = dong_porf_porf_todo__String_prototype_charCodeAt(_get216, _get217, _get218 + 7, 1);
                jjlast_type = _31.type;
                _r372 = _31.value;
                goto j372;
              }
            // end
            j374:;
            _get219 = jjtypeswitch_tmp1;
            // if 
              if (_get219 == 195) {
                _get220 = jjproto_target;
                _get221 = jjproto_targetjjtype;
                _get222 = i;
                const struct ReturnValue _32 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get220, _get221, _get222 + 7, 1);
                jjlast_type = _32.type;
                _r372 = _32.value;
                goto j372;
              }
            // end
            j375:;
            _r372 = 0;
          // end
          j372:;
          jjlast_type = 2;
          _r371 = _r372 == 121;
        } else {
          _get223 = logictmpi;
          _get224 = jjlast_type;
          jjlast_type = _get224;
          _r371 = _get223;
        }
      // end
      j371:;
      // if 
        if ((_r371) != 0) {
          n = Infinity;
          _get225 = negative;
          // if f64
          f64 _r377;
            if (((u32)(_get225)) != 0) {
              _get226 = n;
              jjlast_type = 1;
              _r377 = -_get226;
            } else {
              _get227 = n;
              jjlast_type = 1;
              _r377 = _get227;
            }
          // end
          j377:;
          return _r377;
        }
      // end
      j376:;
      return NaN;
    }
  // end
  j331:;
  _get228 = str;
  _get229 = strjjtype;
  _get230 = i;
  n = dong_porf_porf_todo__Porffor_stn_float(_get228, _get229, _get230, 1);
  _get231 = negative;
  // if 
    if (((u32)(_get231)) != 0) {
      _get232 = n;
      return -_get232;
    }
  // end
  j472:;
  _get233 = n;
  return _get233;
}

static struct ReturnValue dong_porf_porf_todo__Number_prototype_valueOf(f64 _this, i32 _thisjjtype) {
  f64 _get2;
  i32 _get1;
  i32 _get0;
  _get0 = _thisjjtype;
  _get1 = _thisjjtype;
  // if 
    if (((_get0 != 1) & (_get1 != 32)) != 0) {
    }
  // end
  j476:;
  _get2 = _this;
  return (struct ReturnValue){ _get2, 1 };
}

static struct ReturnValue dong_porf_porf_todo__Boolean_prototype_valueOf(f64 _this, i32 _thisjjtype) {
  f64 _get0;
  _get0 = _this;
  return (struct ReturnValue){ _get0, 2 };
}

static struct ReturnValue dong_porf_porf_todo__Object_prototype_valueOf(f64 _this, i32 _thisjjtype) {
  i32 _get33;
  f64 _get32;
  i32 _get31;
  f64 _get30;
  i32 _get29;
  i32 _get28;
  f64 _get27;
  f64 _get26;
  i32 _get25;
  f64 _get24;
  i32 _get23;
  i32 _get22;
  f64 _get21;
  f64 _get20;
  f64 _get19;
  i32 _get18;
  f64 _get17;
  i32 _get16;
  i32 _get15;
  f64 _get14;
  f64 _get13;
  i32 _get12;
  f64 _get11;
  i32 _get10;
  f64 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  i32 _get0;
  f64 obj = 0;
  f64 ovr = 0;
  i32 ovrjjtype = 0;
  f64 jjmember_obj_257 = 0;
  f64 jjmember_prop_257 = 0;
  i32 jjlast_type = 0;
  i32 logictmpi = 0;
  f64 jjcall_val = 0;
  i32 jjcall_type = 0;
  f64 jjindirect_258_callee = 0;
  f64 entryPtr = 0;
  f64 jjindirect_259_callee = 0;

  _get0 = _thisjjtype;
  // if 
    if ((f64)(_get0) == 7) {
      _get1 = _this;
      obj = _get1;
      _get2 = obj;
      // if 
        if (!(_get2 == 0)) {
          jjmember_prop_257 = 1042;
          _get3 = obj;
          jjmember_obj_257 = _get3;
          _get4 = jjmember_obj_257;
          _get5 = jjmember_prop_257;
          const struct ReturnValue _0 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get4), 7, (u32)(_get5), 195, -238740424, 1);
          jjlast_type = _0.type;
          _get6 = jjlast_type;
          ovrjjtype = _get6;
          ovr = _0.value;
          _get7 = ovrjjtype;
          logictmpi = (f64)(_get7) == 6;
          _get8 = logictmpi;
          // if i32
          i32 _r481;
            if ((_get8) != 0) {
              _get9 = ovr;
              jjlast_type = 2;
              _r481 = _get9 != 7;
            } else {
              _get10 = logictmpi;
              jjlast_type = 2;
              _r481 = _get10;
            }
          // end
          j481:;
          // if 
            if ((_r481) != 0) {
              _get11 = ovr;
              jjindirect_258_callee = _get11;
              _get12 = ovrjjtype;
              // if f64
              f64 _r483;
                if (_get12 == 6) {
                  _get13 = _this;
                  jjcall_val = _get13;
                  _get14 = jjcall_val;
                  _get15 = _thisjjtype;
                  jjcall_type = _get15;
                  _get16 = jjcall_type;
                  _get17 = jjindirect_258_callee;
                  jjlast_type = 0;
                  _r483 = 0;
                } else {
                  _r483 = 0;
                }
              // end
              j483:;
              _get18 = jjlast_type;
              return (struct ReturnValue){ _r483, _get18 };
              (void) 0;
            }
          // end
          j482:;
          _get19 = obj;
          entryPtr = (f64)(dong_porf_porf_todo__Porffor_object_lookup((i32)(_get19), 7, 1042, 195, dong_porf_porf_todo__Porffor_object_hash(1042, 195), 1));
          _get20 = entryPtr;
          // if 
            if (_get20 != -1) {
              _get21 = entryPtr;
              const struct ReturnValue _1 = dong_porf_porf_todo__Porffor_object_readValue((i32)(_get21), 1);
              jjlast_type = _1.type;
              _get22 = jjlast_type;
              ovrjjtype = _get22;
              ovr = _1.value;
              _get23 = ovrjjtype;
              // if 
                if ((f64)(_get23) == 6) {
                  _get24 = ovr;
                  jjindirect_259_callee = _get24;
                  _get25 = ovrjjtype;
                  // if f64
                  f64 _r486;
                    if (_get25 == 6) {
                      _get26 = _this;
                      jjcall_val = _get26;
                      _get27 = jjcall_val;
                      _get28 = _thisjjtype;
                      jjcall_type = _get28;
                      _get29 = jjcall_type;
                      _get30 = jjindirect_259_callee;
                      jjlast_type = 0;
                      _r486 = 0;
                    } else {
                      _r486 = 0;
                    }
                  // end
                  j486:;
                  _get31 = jjlast_type;
                  return (struct ReturnValue){ _r486, _get31 };
                  (void) 0;
                } else {
                  return (struct ReturnValue){ 0, 0 };
                  (void) 0;
                }
              // end
              j485:;
            }
          // end
          j484:;
        }
      // end
      j480:;
    }
  // end
  j479:;
  _get32 = _this;
  _get33 = _thisjjtype;
  return (struct ReturnValue){ _get32, _get33 };
}

static struct ReturnValue dong_porf_porf_todo__String_prototype_valueOf(i32 _this, i32 _thisjjtype) {
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  _get0 = _thisjjtype;
  // if 
    if (_get0 != 67) {
      _get1 = _thisjjtype;
      _get2 = _thisjjtype;
      _get3 = _this;
      // if 
        if ((((_get1) == 0) | ((_get2 == 7) & ((_get3) == 0))) != 0) {
        }
      // end
      j491:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_porf_todo__Porffor_bytestringToString(_get7);
        }
      // end
      j492:;
    }
  // end
  j490:;
  _get8 = _this;
  return (struct ReturnValue){ _get8, 67 };
}

static struct ReturnValue dong_porf_porf_todo__Array_prototype_valueOf(f64 _this, i32 _thisjjtype) {
  f64 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  _get0 = _thisjjtype;
  // if 
    if (_get0 != 72) {
      _get1 = _this;
      _get2 = _thisjjtype;
      _this = dong_porf_porf_todo__Array_from(_get1, _get2, 0, 0, 0, 0);
      _thisjjtype = 72;
    }
  // end
  j495:;
  _get3 = _this;
  return (struct ReturnValue){ _get3, 72 };
}

static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_valueOf(i32 _this, i32 _thisjjtype) {
  i32 _get0;
  _get0 = _this;
  return (struct ReturnValue){ _get0, 195 };
}

static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_toString(i32 _this, i32 _thisjjtype) {
  i32 _get0;
  _get0 = _this;
  return (struct ReturnValue){ _get0, 195 };
}

static struct ReturnValue dong_porf_porf_todo__ecma262_ToPrimitive_Number(f64 input, i32 inputjjtype) {
  i32 _get98;
  f64 _get97;
  i32 _get96;
  i32 _get95;
  f64 _get94;
  i32 _get93;
  f64 _get92;
  i32 _get91;
  i32 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  i32 _get86;
  f64 _get85;
  i32 _get84;
  f64 _get83;
  i32 _get82;
  i32 _get81;
  f64 _get80;
  i32 _get79;
  i32 _get78;
  f64 _get77;
  i32 _get76;
  i32 _get75;
  f64 _get74;
  i32 _get73;
  i32 _get72;
  f64 _get71;
  i32 _get70;
  i32 _get69;
  f64 _get68;
  i32 _get67;
  i32 _get66;
  f64 _get65;
  i32 _get64;
  i32 _get63;
  f64 _get62;
  i32 _get61;
  i32 _get60;
  f64 _get59;
  i32 _get58;
  i32 _get57;
  f64 _get56;
  i32 _get55;
  i32 _get54;
  f64 _get53;
  i32 _get52;
  i32 _get51;
  f64 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  i32 _get41;
  f64 _get40;
  i32 _get39;
  f64 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  i32 _get32;
  f64 _get31;
  i32 _get30;
  f64 _get29;
  i32 _get28;
  i32 _get27;
  f64 _get26;
  i32 _get25;
  i32 _get24;
  f64 _get23;
  i32 _get22;
  i32 _get21;
  f64 _get20;
  i32 _get19;
  i32 _get18;
  f64 _get17;
  i32 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  i32 _get12;
  f64 _get11;
  i32 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  f64 _get0;
  f64 value = 0;
  i32 valuejjtype = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  i32 jjlast_type = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 jjlogicinner_tmp = 0;
  i32 logictmpi = 0;

  _get0 = input;
  jjproto_target = _get0;
  _get1 = inputjjtype;
  jjproto_targetjjtype = _get1;
  _get2 = inputjjtype;
  jjtypeswitch_tmp1 = _get2;
  // block f64
  f64 _r473;
    _get3 = jjtypeswitch_tmp1;
    // if 
      if (_get3 == 0) {
        jjlast_type = 0;
        _r473 = 0;
        goto j473;
      }
    // end
    j474:;
    _get4 = jjtypeswitch_tmp1;
    // if 
      if (_get4 == 1) {
        _get5 = jjproto_target;
        _get6 = jjproto_targetjjtype;
        const struct ReturnValue _0 = dong_porf_porf_todo__Number_prototype_valueOf(_get5, _get6);
        jjlast_type = _0.type;
        _r473 = _0.value;
        goto j473;
      }
    // end
    j475:;
    _get7 = jjtypeswitch_tmp1;
    // if 
      if (_get7 == 2) {
        _get8 = jjproto_target;
        _get9 = jjproto_targetjjtype;
        const struct ReturnValue _1 = dong_porf_porf_todo__Boolean_prototype_valueOf(_get8, _get9);
        jjlast_type = _1.type;
        _r473 = _1.value;
        goto j473;
      }
    // end
    j477:;
    _get10 = jjtypeswitch_tmp1;
    // if 
      if (_get10 == 7) {
        _get11 = jjproto_target;
        _get12 = jjproto_targetjjtype;
        const struct ReturnValue _2 = dong_porf_porf_todo__Object_prototype_valueOf(_get11, _get12);
        jjlast_type = _2.type;
        _r473 = _2.value;
        goto j473;
      }
    // end
    j478:;
    _get13 = jjtypeswitch_tmp1;
    // if 
      if (_get13 == 31) {
        _get14 = jjproto_target;
        _get15 = jjproto_targetjjtype;
        const struct ReturnValue _3 = dong_porf_porf_todo__Boolean_prototype_valueOf(_get14, _get15);
        jjlast_type = _3.type;
        _r473 = _3.value;
        goto j473;
      }
    // end
    j487:;
    _get16 = jjtypeswitch_tmp1;
    // if 
      if (_get16 == 32) {
        _get17 = jjproto_target;
        _get18 = jjproto_targetjjtype;
        const struct ReturnValue _4 = dong_porf_porf_todo__Number_prototype_valueOf(_get17, _get18);
        jjlast_type = _4.type;
        _r473 = _4.value;
        goto j473;
      }
    // end
    j488:;
    _get19 = jjtypeswitch_tmp1;
    // if 
      if (_get19 == 33) {
        _get20 = jjproto_target;
        _get21 = jjproto_targetjjtype;
        const struct ReturnValue _5 = dong_porf_porf_todo__String_prototype_valueOf((i32)(_get20), _get21);
        jjlast_type = _5.type;
        _r473 = (f64)(_5.value);
        goto j473;
      }
    // end
    j489:;
    _get22 = jjtypeswitch_tmp1;
    // if 
      if (_get22 == 67) {
        _get23 = jjproto_target;
        _get24 = jjproto_targetjjtype;
        const struct ReturnValue _6 = dong_porf_porf_todo__String_prototype_valueOf((i32)(_get23), _get24);
        jjlast_type = _6.type;
        _r473 = (f64)(_6.value);
        goto j473;
      }
    // end
    j493:;
    _get25 = jjtypeswitch_tmp1;
    // if 
      if (_get25 == 72) {
        _get26 = jjproto_target;
        _get27 = jjproto_targetjjtype;
        const struct ReturnValue _7 = dong_porf_porf_todo__Array_prototype_valueOf(_get26, _get27);
        jjlast_type = _7.type;
        _r473 = _7.value;
        goto j473;
      }
    // end
    j494:;
    _get28 = jjtypeswitch_tmp1;
    // if 
      if (_get28 == 195) {
        _get29 = jjproto_target;
        _get30 = jjproto_targetjjtype;
        const struct ReturnValue _8 = dong_porf_porf_todo__ByteString_prototype_valueOf((i32)(_get29), _get30);
        jjlast_type = _8.type;
        _r473 = (f64)(_8.value);
        goto j473;
      }
    // end
    j496:;
    _get31 = jjproto_target;
    _get32 = jjproto_targetjjtype;
    const struct ReturnValue _9 = dong_porf_porf_todo__Object_prototype_valueOf(_get31, _get32);
    jjlast_type = _9.type;
    _r473 = _9.value;
  // end
  j473:;
  value = _r473;
  _get33 = jjlast_type;
  valuejjtype = _get33;
  _get34 = value;
  jjlogicinner_tmp = _get34;
  _get35 = valuejjtype;
  jjtypeswitch_tmp1 = _get35;
  // block i32
  i32 _r497;
    _get36 = jjtypeswitch_tmp1;
    // if 
      if (_get36 == 0) {
        _r497 = 1;
        goto j497;
      }
    // end
    j498:;
    _get37 = jjtypeswitch_tmp1;
    // if 
      if (_get37 == 7) {
        _get38 = jjlogicinner_tmp;
        _r497 = _get38 == 0;
        goto j497;
      }
    // end
    j499:;
    _r497 = 0;
  // end
  j497:;
  logictmpi = (_r497) == 0;
  _get39 = logictmpi;
  // if i32
  i32 _r500;
    if ((_get39) != 0) {
      _get40 = value;
      _get41 = valuejjtype;
      jjlast_type = 2;
      _r500 = (f64)(dong_porf_porf_todo__Porffor_object_isObjectOrNull((i32)(_get40), _get41)) == 0;
    } else {
      _get42 = logictmpi;
      jjlast_type = 2;
      _r500 = _get42;
    }
  // end
  j500:;
  // if 
    if ((_r500) != 0) {
      _get43 = value;
      _get44 = valuejjtype;
      return (struct ReturnValue){ _get43, _get44 };
    }
  // end
  j501:;
  _get45 = input;
  jjproto_target = _get45;
  _get46 = inputjjtype;
  jjproto_targetjjtype = _get46;
  _get47 = inputjjtype;
  jjtypeswitch_tmp1 = _get47;
  // block f64
  f64 _r502;
    _get48 = jjtypeswitch_tmp1;
    // if 
      if (_get48 == 0) {
        jjlast_type = 0;
        _r502 = 0;
        goto j502;
      }
    // end
    j503:;
    _get49 = jjtypeswitch_tmp1;
    // if 
      if (_get49 == 1) {
        _get50 = jjproto_target;
        _get51 = jjproto_targetjjtype;
        const struct ReturnValue _10 = dong_porf_porf_todo__Number_prototype_toString(_get50, _get51, 0, 0);
        jjlast_type = _10.type;
        _r502 = _10.value;
        goto j502;
      }
    // end
    j504:;
    _get52 = jjtypeswitch_tmp1;
    // if 
      if (_get52 == 2) {
        _get53 = jjproto_target;
        _get54 = jjproto_targetjjtype;
        const struct ReturnValue _11 = dong_porf_porf_todo__Boolean_prototype_toString(_get53, _get54);
        jjlast_type = _11.type;
        _r502 = _11.value;
        goto j502;
      }
    // end
    j505:;
    _get55 = jjtypeswitch_tmp1;
    // if 
      if (_get55 == 6) {
        _get56 = jjproto_target;
        _get57 = jjproto_targetjjtype;
        const struct ReturnValue _12 = dong_porf_porf_todo__Function_prototype_toString(_get56, _get57);
        jjlast_type = _12.type;
        _r502 = _12.value;
        goto j502;
      }
    // end
    j506:;
    _get58 = jjtypeswitch_tmp1;
    // if 
      if (_get58 == 7) {
        _get59 = jjproto_target;
        _get60 = jjproto_targetjjtype;
        const struct ReturnValue _13 = dong_porf_porf_todo__Object_prototype_toString(_get59, _get60);
        jjlast_type = _13.type;
        _r502 = _13.value;
        goto j502;
      }
    // end
    j507:;
    _get61 = jjtypeswitch_tmp1;
    // if 
      if (_get61 == 31) {
        _get62 = jjproto_target;
        _get63 = jjproto_targetjjtype;
        const struct ReturnValue _14 = dong_porf_porf_todo__Boolean_prototype_toString(_get62, _get63);
        jjlast_type = _14.type;
        _r502 = _14.value;
        goto j502;
      }
    // end
    j508:;
    _get64 = jjtypeswitch_tmp1;
    // if 
      if (_get64 == 32) {
        _get65 = jjproto_target;
        _get66 = jjproto_targetjjtype;
        const struct ReturnValue _15 = dong_porf_porf_todo__Number_prototype_toString(_get65, _get66, 0, 0);
        jjlast_type = _15.type;
        _r502 = _15.value;
        goto j502;
      }
    // end
    j509:;
    _get67 = jjtypeswitch_tmp1;
    // if 
      if (_get67 == 33) {
        _get68 = jjproto_target;
        _get69 = jjproto_targetjjtype;
        const struct ReturnValue _16 = dong_porf_porf_todo__String_prototype_toString((i32)(_get68), _get69);
        jjlast_type = _16.type;
        _r502 = (f64)(_16.value);
        goto j502;
      }
    // end
    j510:;
    _get70 = jjtypeswitch_tmp1;
    // if 
      if (_get70 == 38) {
        _get71 = jjproto_target;
        _get72 = jjproto_targetjjtype;
        const struct ReturnValue _17 = dong_porf_porf_todo__TypeError_prototype_toString(_get71, _get72);
        jjlast_type = _17.type;
        _r502 = _17.value;
        goto j502;
      }
    // end
    j511:;
    _get73 = jjtypeswitch_tmp1;
    // if 
      if (_get73 == 41) {
        _get74 = jjproto_target;
        _get75 = jjproto_targetjjtype;
        const struct ReturnValue _18 = dong_porf_porf_todo__RangeError_prototype_toString(_get74, _get75);
        jjlast_type = _18.type;
        _r502 = _18.value;
        goto j502;
      }
    // end
    j512:;
    _get76 = jjtypeswitch_tmp1;
    // if 
      if (_get76 == 67) {
        _get77 = jjproto_target;
        _get78 = jjproto_targetjjtype;
        const struct ReturnValue _19 = dong_porf_porf_todo__String_prototype_toString((i32)(_get77), _get78);
        jjlast_type = _19.type;
        _r502 = (f64)(_19.value);
        goto j502;
      }
    // end
    j513:;
    _get79 = jjtypeswitch_tmp1;
    // if 
      if (_get79 == 72) {
        _get80 = jjproto_target;
        _get81 = jjproto_targetjjtype;
        const struct ReturnValue _20 = dong_porf_porf_todo__Array_prototype_toString(_get80, _get81);
        jjlast_type = _20.type;
        _r502 = _20.value;
        goto j502;
      }
    // end
    j514:;
    _get82 = jjtypeswitch_tmp1;
    // if 
      if (_get82 == 195) {
        _get83 = jjproto_target;
        _get84 = jjproto_targetjjtype;
        const struct ReturnValue _21 = dong_porf_porf_todo__ByteString_prototype_toString((i32)(_get83), _get84);
        jjlast_type = _21.type;
        _r502 = (f64)(_21.value);
        goto j502;
      }
    // end
    j515:;
    _get85 = jjproto_target;
    _get86 = jjproto_targetjjtype;
    const struct ReturnValue _22 = dong_porf_porf_todo__Object_prototype_toString(_get85, _get86);
    jjlast_type = _22.type;
    _r502 = _22.value;
  // end
  j502:;
  value = _r502;
  _get87 = jjlast_type;
  valuejjtype = _get87;
  _get88 = value;
  jjlogicinner_tmp = _get88;
  _get89 = valuejjtype;
  jjtypeswitch_tmp1 = _get89;
  // block i32
  i32 _r516;
    _get90 = jjtypeswitch_tmp1;
    // if 
      if (_get90 == 0) {
        _r516 = 1;
        goto j516;
      }
    // end
    j517:;
    _get91 = jjtypeswitch_tmp1;
    // if 
      if (_get91 == 7) {
        _get92 = jjlogicinner_tmp;
        _r516 = _get92 == 0;
        goto j516;
      }
    // end
    j518:;
    _r516 = 0;
  // end
  j516:;
  logictmpi = (_r516) == 0;
  _get93 = logictmpi;
  // if i32
  i32 _r519;
    if ((_get93) != 0) {
      _get94 = value;
      _get95 = valuejjtype;
      jjlast_type = 2;
      _r519 = (f64)(dong_porf_porf_todo__Porffor_object_isObjectOrNull((i32)(_get94), _get95)) == 0;
    } else {
      _get96 = logictmpi;
      jjlast_type = 2;
      _r519 = _get96;
    }
  // end
  j519:;
  // if 
    if ((_r519) != 0) {
      _get97 = value;
      _get98 = valuejjtype;
      return (struct ReturnValue){ _get97, _get98 };
    }
  // end
  j520:;
  return (struct ReturnValue){ 0, 0 };
}

static f64 dong_porf_porf_todo__ecma262_ToNumber(f64 argument, i32 argumentjjtype) {
  i32 _get18;
  f64 _get17;
  i32 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  f64 _get12;
  i32 _get11;
  i32 _get10;
  f64 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 primValue = 0;
  i32 primValuejjtype = 0;
  i32 jjlast_type = 0;

  _get0 = argumentjjtype;
  // if 
    if ((f64)(_get0) == 1) {
      _get1 = argument;
      return _get1;
    }
  // end
  j253:;
  _get2 = argumentjjtype;
  _get3 = argumentjjtype;
  // if 
    if ((((f64)(_get2) == 5) | ((f64)(_get3) == 4)) != 0) {
    }
  // end
  j254:;
  _get4 = argumentjjtype;
  // if 
    if ((f64)(_get4) == 0) {
      return NaN;
    }
  // end
  j255:;
  _get5 = argument;
  _get6 = argumentjjtype;
  _get7 = argument;
  _get8 = argumentjjtype;
  // if 
    if ((((_get5 == 0) & ((_get6 | 128) == (7 | 128))) | ((_get7 == 0) & ((_get8 | 128) == (2 | 128)))) != 0) {
      return 0;
    }
  // end
  j256:;
  _get9 = argument;
  _get10 = argumentjjtype;
  // if 
    if (((_get9 == 1) & ((_get10 | 128) == (2 | 128))) != 0) {
      return 1;
    }
  // end
  j257:;
  _get11 = argumentjjtype;
  // if 
    if ((f64)(_get11 | 128) == 195) {
      _get12 = argument;
      _get13 = argumentjjtype;
      return dong_porf_porf_todo__ecma262_StringToNumber(_get12, _get13);
    }
  // end
  j258:;
  _get14 = argument;
  _get15 = argumentjjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToPrimitive_Number(_get14, _get15);
  jjlast_type = _0.type;
  _get16 = jjlast_type;
  primValuejjtype = _get16;
  primValue = _0.value;
  _get17 = primValue;
  _get18 = primValuejjtype;
  return dong_porf_porf_todo__ecma262_ToNumber(_get17, _get18);
}

static f64 dong_porf_porf_todo__ecma262_ToIntegerOrInfinity(f64 argument, i32 argumentjjtype) {
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  f64 number = 0;

  _get0 = argument;
  _get1 = argumentjjtype;
  number = dong_porf_porf_todo__ecma262_ToNumber(_get0, _get1);
  _get2 = number;
  // if 
    if (((u32)(dong_porf_porf_todo__Number_isNaN(_get2))) != 0) {
      return 0;
    }
  // end
  j521:;
  _get3 = number;
  // if 
    if (dong_porf_porf_todo__Number_isFinite(_get3) == 0) {
      _get4 = number;
      return _get4;
    }
  // end
  j522:;
  _get5 = number;
  number = dong_porf_porf_todo__Math_trunc(_get5);
  _get6 = number;
  // if 
    if (_get6 == 0) {
      return 0;
    }
  // end
  j523:;
  _get7 = number;
  return _get7;
}

static f64 dong_porf_porf_todo__Array_from(f64 arg, i32 argjjtype, f64 mapFn, i32 mapFnjjtype, f64 thisArg, i32 thisArgjjtype) {
  f64 _get174;
  f64 _get173;
  f64 _get172;
  f64 _get171;
  f64 _get170;
  i32 _get169;
  i32 _get168;
  i32 _get167;
  f64 _get166;
  f64 _get165;
  f64 _get164;
  f64 _get163;
  i32 _get162;
  f64 _get161;
  f64 _get160;
  f64 _get159;
  f64 _get158;
  f64 _get157;
  f64 _get156;
  f64 _get155;
  i32 _get154;
  i32 _get153;
  f64 _get152;
  f64 _get151;
  i32 _get150;
  i32 _get149;
  f64 _get148;
  f64 _get147;
  f64 _get146;
  f64 _get145;
  i32 _get144;
  i32 _get143;
  f64 _get142;
  f64 _get141;
  i32 _get140;
  f64 _get139;
  i32 _get138;
  f64 _get137;
  f64 _get136;
  f64 _get135;
  f64 _get134;
  f64 _get133;
  f64 _get132;
  i32 _get131;
  i32 _get130;
  f64 _get129;
  f64 _get128;
  i32 _get127;
  i32 _get126;
  f64 _get125;
  f64 _get124;
  f64 _get123;
  i32 _get122;
  i32 _get121;
  f64 _get120;
  i32 _get119;
  f64 _get118;
  i32 _get117;
  i32 _get116;
  f64 _get115;
  f64 _get114;
  f64 _get113;
  f64 _get112;
  i32 _get111;
  i32 _get110;
  f64 _get109;
  i32 _get108;
  f64 _get107;
  f64 _get106;
  f64 _get105;
  f64 _get104;
  f64 _get103;
  i32 _get102;
  i32 _get101;
  i32 _get100;
  i32 _get99;
  i32 _get98;
  i32 _get97;
  i32 _get96;
  i32 _get95;
  i32 _get94;
  i32 _get93;
  i32 _get92;
  i32 _get91;
  i32 _get90;
  i32 _get89;
  i32 _get88;
  i32 _get87;
  i32 _get86;
  i32 _get85;
  i32 _get84;
  i32 _get83;
  i32 _get82;
  i32 _get81;
  i32 _get80;
  i32 _get79;
  i32 _get78;
  i32 _get77;
  i32 _get76;
  i32 _get75;
  i32 _get74;
  i32 _get73;
  i32 _get72;
  i32 _get71;
  i32 _get70;
  i32 _get69;
  i32 _get68;
  f64 _get67;
  f64 _get66;
  i32 _get65;
  i32 _get64;
  f64 _get63;
  f64 _get62;
  i32 _get61;
  f64 _get60;
  i32 _get59;
  i32 _get58;
  f64 _get57;
  f64 _get56;
  i32 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  f64 _get51;
  f64 _get50;
  f64 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  f64 _get13;
  i32 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 out = 0;
  f64 i = 0;
  i32 jjforof_base_pointer0 = 0;
  i32 jjforof_length0 = 0;
  i32 jjforof_counter0 = 0;
  i32 jjforof_itertype0 = 0;
  f64 x = 0;
  i32 xjjtype = 0;
  i32 jjlast_type = 0;
  i32 jjforof_allocd = 0;
  i32 jjforof_mapptr = 0;
  i32 jjmember_setter_ptr_tmp = 0;
  f64 jjmember_obj_14 = 0;
  f64 jjmember_prop_14 = 0;
  f64 jjcall_val = 0;
  i32 jjcall_type = 0;
  f64 jjindirect_15_callee = 0;
  f64 jjmember_obj_16 = 0;
  f64 jjmember_prop_16 = 0;
  f64 obj = 0;
  f64 len = 0;
  f64 jjmember_obj_17 = 0;
  f64 jjmember_prop_17 = 0;
  i32 jjswap = 0;
  f64 jjmember_obj_18 = 0;
  f64 jjmember_prop_18 = 0;
  f64 jjmember_obj_19 = 0;
  f64 jjmember_prop_19 = 0;
  f64 jjindirect_20_callee = 0;
  f64 jjmember_obj_21 = 0;
  f64 jjmember_prop_21 = 0;
  f64 jjmember_obj_22 = 0;
  f64 jjmember_prop_22 = 0;

  _get0 = thisArgjjtype;
  // if 
    if (_get0 == 0) {
      thisArg = 0;
      thisArgjjtype = 0;
    }
  // end
  j173:;
  _get1 = arg;
  jjlogicinner_tmp = _get1;
  _get2 = argjjtype;
  jjtypeswitch_tmp1 = _get2;
  // block i32
  i32 _r174;
    _get3 = jjtypeswitch_tmp1;
    // if 
      if (_get3 == 0) {
        _r174 = 1;
        goto j174;
      }
    // end
    j175:;
    _get4 = jjtypeswitch_tmp1;
    // if 
      if (_get4 == 7) {
        _get5 = jjlogicinner_tmp;
        _r174 = _get5 == 0;
        goto j174;
      }
    // end
    j176:;
    _r174 = 0;
  // end
  j174:;
  // if 
    if ((_r174) != 0) {
    }
  // end
  j177:;
  out = (f64)(dong_porf_porf_todo__Porffor_malloc(16384));
  _get6 = argjjtype;
  _get7 = argjjtype;
  _get8 = argjjtype;
  _get9 = argjjtype;
  _get10 = argjjtype;
  // if 
    if ((((((f64)(_get6) == 72) | ((f64)(_get7 | 128) == 195)) | ((f64)(_get8) == 11)) | (((f64)(_get9) >= 80) & ((f64)(_get10) <= 90))) != 0) {
      i = 0;
      _get11 = mapFnjjtype;
      // if 
        if ((f64)(_get11) != 0) {
          _get12 = mapFnjjtype;
          // if 
            if ((f64)(_get12) != 6) {
            }
          // end
          j180:;
          _get13 = arg;
          jjforof_base_pointer0 = (u32)(_get13);
          _get14 = argjjtype;
          jjforof_itertype0 = _get14;
          jjforof_counter0 = 0;
          _get15 = jjforof_itertype0;
          _get16 = jjforof_itertype0;
          _get17 = jjforof_itertype0;
          _get18 = jjforof_itertype0;
          _get19 = jjforof_itertype0;
          _get20 = jjforof_itertype0;
          _get21 = jjforof_itertype0;
          _get22 = jjforof_itertype0;
          // if 
            if ((((((((_get15 == 72) | (_get16 == 11)) | (_get17 == 12)) | (_get18 == 67)) | (_get19 == 195)) | (_get20 == 34)) | ((_get21 >= 80) & (_get22 <= 90))) == 0) {
            }
          // end
          j181:;
          _get23 = jjforof_base_pointer0;
          jjforof_length0 = i32_load(1, 0, _get23);
          // loop 
          j182:;
            // block 
              _get24 = jjforof_itertype0;
              jjtypeswitch_tmp1 = _get24;
              // block f64
              f64 _r184;
                _get25 = jjtypeswitch_tmp1;
                _get26 = jjtypeswitch_tmp1;
                // if 
                  if (((_get25 == 72) | (_get26 == 11)) != 0) {
                    _get27 = jjforof_length0;
                    if ((_get27) == 0) {
                      goto j183;
                    }
                    _get28 = jjforof_base_pointer0;
                    _get29 = jjforof_base_pointer0;
                    _get30 = jjforof_base_pointer0;
                    jjforof_base_pointer0 = _get30 + 9;
                    _get31 = jjforof_length0;
                    jjforof_length0 = _get31 - 1;
                    jjlast_type = i32_load8_u(0, 12, _get29);
                    _r184 = f64_load(0, 4, _get28);
                    goto j184;
                  }
                // end
                j185:;
                _get32 = jjtypeswitch_tmp1;
                // if 
                  if (_get32 == 67) {
                    _get33 = jjforof_length0;
                    if ((_get33) == 0) {
                      goto j183;
                    }
                    jjforof_allocd = dong_porf_porf_todo__Porffor_malloc(8);
                    _get34 = jjforof_allocd;
                    i32_store(0, 0, _get34, 1);
                    _get35 = jjforof_allocd;
                    _get36 = jjforof_base_pointer0;
                    i32_store16(1, 4, _get35, i32_load16_u(1, 4, _get36));
                    _get37 = jjforof_base_pointer0;
                    jjforof_base_pointer0 = _get37 + 2;
                    _get38 = jjforof_length0;
                    jjforof_length0 = _get38 - 1;
                    _get39 = jjforof_allocd;
                    jjlast_type = 67;
                    _r184 = (f64)(_get39);
                    goto j184;
                  }
                // end
                j186:;
                _get40 = jjtypeswitch_tmp1;
                // if 
                  if (_get40 == 195) {
                    _get41 = jjforof_length0;
                    if ((_get41) == 0) {
                      goto j183;
                    }
                    jjforof_allocd = dong_porf_porf_todo__Porffor_malloc(8);
                    _get42 = jjforof_allocd;
                    i32_store(0, 0, _get42, 1);
                    _get43 = jjforof_allocd;
                    _get44 = jjforof_base_pointer0;
                    i32_store8(0, 4, _get43, i32_load8_u(0, 4, _get44));
                    _get45 = jjforof_base_pointer0;
                    jjforof_base_pointer0 = _get45 + 1;
                    _get46 = jjforof_length0;
                    jjforof_length0 = _get46 - 1;
                    _get47 = jjforof_allocd;
                    jjlast_type = 195;
                    _r184 = (f64)(_get47);
                    goto j184;
                  }
                // end
                j187:;
              // end
              j184:;
              x = _r184;
              _get48 = jjlast_type;
              xjjtype = _get48;
              _get49 = out;
              jjmember_obj_14 = _get49;
              _get50 = i;
              jjmember_prop_14 = _get50;
              _get51 = jjmember_obj_14;
              _get52 = jjmember_prop_14;
              jjmember_setter_ptr_tmp = (u32)(_get51) + ((u32)(_get52) * 9);
              _get53 = jjmember_setter_ptr_tmp;
              _get54 = mapFn;
              jjindirect_15_callee = _get54;
              _get55 = mapFnjjtype;
              // if f64
              f64 _r188;
                if (_get55 == 6) {
                  _get56 = thisArg;
                  jjcall_val = _get56;
                  _get57 = jjcall_val;
                  _get58 = thisArgjjtype;
                  jjcall_type = _get58;
                  _get59 = jjcall_type;
                  _get60 = x;
                  _get61 = xjjtype;
                  _get62 = i;
                  _get63 = jjindirect_15_callee;
                  jjlast_type = 0;
                  _r188 = 0;
                } else {
                  _r188 = 0;
                }
              // end
              j188:;
              f64_store(0, 4, 0, _r188);
              _get64 = jjmember_setter_ptr_tmp;
              _get65 = jjlast_type;
              i32_store8(0, 12, _get64, _get65);
              _get66 = i;
              i = _get66 + 1;
              goto j182;
            // end
            j183:;
          // end
        } else {
          _get67 = arg;
          jjforof_base_pointer0 = (u32)(_get67);
          _get68 = argjjtype;
          jjforof_itertype0 = _get68;
          jjforof_counter0 = 0;
          _get69 = jjforof_itertype0;
          _get70 = jjforof_itertype0;
          _get71 = jjforof_itertype0;
          _get72 = jjforof_itertype0;
          _get73 = jjforof_itertype0;
          _get74 = jjforof_itertype0;
          _get75 = jjforof_itertype0;
          _get76 = jjforof_itertype0;
          // if 
            if ((((((((_get69 == 72) | (_get70 == 11)) | (_get71 == 12)) | (_get72 == 67)) | (_get73 == 195)) | (_get74 == 34)) | ((_get75 >= 80) & (_get76 <= 90))) == 0) {
            }
          // end
          j189:;
          _get77 = jjforof_base_pointer0;
          jjforof_length0 = i32_load(1, 0, _get77);
          // loop 
          j190:;
            // block 
              _get78 = jjforof_itertype0;
              jjtypeswitch_tmp1 = _get78;
              // block f64
              f64 _r192;
                _get79 = jjtypeswitch_tmp1;
                _get80 = jjtypeswitch_tmp1;
                // if 
                  if (((_get79 == 72) | (_get80 == 11)) != 0) {
                    _get81 = jjforof_length0;
                    if ((_get81) == 0) {
                      goto j191;
                    }
                    _get82 = jjforof_base_pointer0;
                    _get83 = jjforof_base_pointer0;
                    _get84 = jjforof_base_pointer0;
                    jjforof_base_pointer0 = _get84 + 9;
                    _get85 = jjforof_length0;
                    jjforof_length0 = _get85 - 1;
                    jjlast_type = i32_load8_u(0, 12, _get83);
                    _r192 = f64_load(0, 4, _get82);
                    goto j192;
                  }
                // end
                j193:;
                _get86 = jjtypeswitch_tmp1;
                // if 
                  if (_get86 == 67) {
                    _get87 = jjforof_length0;
                    if ((_get87) == 0) {
                      goto j191;
                    }
                    jjforof_allocd = dong_porf_porf_todo__Porffor_malloc(8);
                    _get88 = jjforof_allocd;
                    i32_store(0, 0, _get88, 1);
                    _get89 = jjforof_allocd;
                    _get90 = jjforof_base_pointer0;
                    i32_store16(1, 4, _get89, i32_load16_u(1, 4, _get90));
                    _get91 = jjforof_base_pointer0;
                    jjforof_base_pointer0 = _get91 + 2;
                    _get92 = jjforof_length0;
                    jjforof_length0 = _get92 - 1;
                    _get93 = jjforof_allocd;
                    jjlast_type = 67;
                    _r192 = (f64)(_get93);
                    goto j192;
                  }
                // end
                j194:;
                _get94 = jjtypeswitch_tmp1;
                // if 
                  if (_get94 == 195) {
                    _get95 = jjforof_length0;
                    if ((_get95) == 0) {
                      goto j191;
                    }
                    jjforof_allocd = dong_porf_porf_todo__Porffor_malloc(8);
                    _get96 = jjforof_allocd;
                    i32_store(0, 0, _get96, 1);
                    _get97 = jjforof_allocd;
                    _get98 = jjforof_base_pointer0;
                    i32_store8(0, 4, _get97, i32_load8_u(0, 4, _get98));
                    _get99 = jjforof_base_pointer0;
                    jjforof_base_pointer0 = _get99 + 1;
                    _get100 = jjforof_length0;
                    jjforof_length0 = _get100 - 1;
                    _get101 = jjforof_allocd;
                    jjlast_type = 195;
                    _r192 = (f64)(_get101);
                    goto j192;
                  }
                // end
                j195:;
                _r192 = 0;
              // end
              j192:;
              x = _r192;
              _get102 = jjlast_type;
              xjjtype = _get102;
              _get103 = out;
              jjmember_obj_16 = _get103;
              _get104 = i;
              _get105 = i;
              i = _get105 + 1;
              jjmember_prop_16 = _get104;
              _get106 = jjmember_obj_16;
              _get107 = jjmember_prop_16;
              jjmember_setter_ptr_tmp = (u32)(_get106) + ((u32)(_get107) * 9);
              _get108 = jjmember_setter_ptr_tmp;
              _get109 = x;
              f64_store(0, 4, _get108, _get109);
              _get110 = jjmember_setter_ptr_tmp;
              _get111 = xjjtype;
              i32_store8(0, 12, _get110, _get111);
              goto j190;
            // end
            j191:;
          // end
        }
      // end
      j179:;
      _get112 = out;
      _get113 = i;
      i32_store(1, 0, (u32)(_get112), (u32)(_get113));
      _get114 = out;
      return _get114;
      (void) 0;
    }
  // end
  j178:;
  _get115 = arg;
  _get116 = argjjtype;
  // if 
    if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get115), _get116)) != 0) {
      _get117 = argjjtype;
      // if f64
      f64 _r197;
        if ((f64)(_get117) == 7) {
          _get118 = arg;
          _get119 = argjjtype;
          jjlast_type = _get119;
          _r197 = _get118;
        } else {
          _get120 = arg;
          _get121 = argjjtype;
          const struct ReturnValue _0 = dong_porf_porf_todo__Porffor_object_underlying(_get120, _get121);
          jjlast_type = _0.type;
          _get122 = jjlast_type;
          jjlast_type = _get122;
          _r197 = (f64)(_0.value);
        }
      // end
      j197:;
      obj = _r197;
      jjmember_prop_17 = 564;
      _get123 = obj;
      jjmember_obj_17 = _get123;
      _get124 = jjmember_obj_17;
      _get125 = jjmember_prop_17;
      const struct ReturnValue _1 = dong_porf_porf_todo__ecma262_ToPropertyKey(_get125, 195);
      jjswap = _1.type;
      _get126 = jjswap;
      const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_object_get((i32)(_get124), 7, (i32)(_1.value), _get126);
      jjlast_type = _2.type;
      _get127 = jjlast_type;
      len = dong_porf_porf_todo__ecma262_ToIntegerOrInfinity(_2.value, _get127);
      _get128 = len;
      // if 
        if (_get128 > 4294967295) {
          (void) 0;
        }
      // end
      j524:;
      _get129 = len;
      // if 
        if (_get129 < 0) {
          len = 0;
        }
      // end
      j525:;
      _get130 = mapFnjjtype;
      // if 
        if ((f64)(_get130) != 0) {
          _get131 = mapFnjjtype;
          // if 
            if ((f64)(_get131) != 6) {
              (void) 0;
            }
          // end
          j527:;
          i = 0;
          // loop 
          j528:;
            _get132 = i;
            _get133 = len;
            // if 
              if (_get132 < _get133) {
                _get134 = out;
                jjmember_obj_18 = _get134;
                _get135 = i;
                jjmember_prop_18 = _get135;
                _get136 = jjmember_obj_18;
                _get137 = jjmember_prop_18;
                jjmember_setter_ptr_tmp = (u32)(_get136) + ((u32)(_get137) * 9);
                _get138 = jjmember_setter_ptr_tmp;
                _get139 = mapFn;
                jjindirect_20_callee = _get139;
                _get140 = mapFnjjtype;
                // if f64
                f64 _r530;
                  if (_get140 == 6) {
                    _get141 = thisArg;
                    jjcall_val = _get141;
                    _get142 = jjcall_val;
                    _get143 = thisArgjjtype;
                    jjcall_type = _get143;
                    _get144 = jjcall_type;
                    _get145 = i;
                    jjmember_prop_19 = _get145;
                    _get146 = obj;
                    jjmember_obj_19 = _get146;
                    _get147 = jjmember_obj_19;
                    _get148 = jjmember_prop_19;
                    const struct ReturnValue _3 = dong_porf_porf_todo__ecma262_ToPropertyKey(_get148, 1);
                    jjswap = _3.type;
                    _get149 = jjswap;
                    const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_object_get((i32)(_get147), 7, (i32)(_3.value), _get149);
                    jjlast_type = _4.type;
                    _get150 = jjlast_type;
                    _get151 = i;
                    _get152 = jjindirect_20_callee;
                    jjlast_type = 0;
                    _r530 = 0;
                  } else {
                    _r530 = 0;
                  }
                // end
                j530:;
                f64_store(0, 4, 0, _r530);
                _get153 = jjmember_setter_ptr_tmp;
                _get154 = jjlast_type;
                i32_store8(0, 12, _get153, _get154);
                _get155 = i;
                i = _get155 + 1;
                goto j528;
              }
            // end
            j529:;
          // end
        } else {
          i = 0;
          // loop 
          j531:;
            _get156 = i;
            _get157 = len;
            // if 
              if (_get156 < _get157) {
                _get158 = out;
                jjmember_obj_21 = _get158;
                _get159 = i;
                jjmember_prop_21 = _get159;
                _get160 = jjmember_obj_21;
                _get161 = jjmember_prop_21;
                jjmember_setter_ptr_tmp = (u32)(_get160) + ((u32)(_get161) * 9);
                _get162 = jjmember_setter_ptr_tmp;
                _get163 = i;
                jjmember_prop_22 = _get163;
                _get164 = obj;
                jjmember_obj_22 = _get164;
                _get165 = jjmember_obj_22;
                _get166 = jjmember_prop_22;
                const struct ReturnValue _5 = dong_porf_porf_todo__ecma262_ToPropertyKey(_get166, 1);
                jjswap = _5.type;
                _get167 = jjswap;
                const struct ReturnValue _6 = dong_porf_porf_todo__Porffor_object_get((i32)(_get165), 7, (i32)(_5.value), _get167);
                jjlast_type = _6.type;
                f64_store(0, 4, _get162, _6.value);
                _get168 = jjmember_setter_ptr_tmp;
                _get169 = jjlast_type;
                i32_store8(0, 12, _get168, _get169);
                _get170 = i;
                i = _get170 + 1;
                goto j531;
              }
            // end
            j532:;
          // end
        }
      // end
      j526:;
      _get171 = out;
      _get172 = len;
      i32_store(1, 0, (u32)(_get171), (u32)(_get172));
      _get173 = out;
      return _get173;
      (void) 0;
    }
  // end
  j196:;
  _get174 = out;
  return _get174;
}

static f64 dong_porf_porf_todo__Porffor_bytestring_appendChar(f64 str, i32 strjjtype, f64 _char, i32 charjjtype) {
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 len = 0;

  _get0 = str;
  len = (f64)(i32_load(1, 0, (u32)(_get0)));
  _get1 = str;
  _get2 = len;
  _get3 = _char;
  i32_store8(0, 4, (i32)((_get1 + _get2)), (i32)(_get3));
  _get4 = str;
  _get5 = len;
  i32_store(1, 0, (u32)(_get4), (u32)((_get5 + 1)));
  return 1;
}

static struct ReturnValue dong_porf_porf_todo__Array_prototype_toString(f64 _this, i32 _thisjjtype) {
  f64 _get25;
  i32 _get24;
  i32 _get23;
  f64 _get22;
  f64 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  f64 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 out = 0;
  f64 len = 0;
  f64 i = 0;
  f64 element = 0;
  i32 elementjjtype = 0;
  f64 jjmember_obj_80 = 0;
  f64 jjmember_prop_80 = 0;
  i32 jjlast_type = 0;
  i32 jjloadArray_offset = 0;
  i32 logictmpi = 0;

  _get0 = _thisjjtype;
  // if 
    if (_get0 != 72) {
      _get1 = _this;
      _get2 = _thisjjtype;
      _this = dong_porf_porf_todo__Array_from(_get1, _get2, 0, 0, 0, 0);
      _thisjjtype = 72;
    }
  // end
  j172:;
  out = (f64)(dong_porf_porf_todo__Porffor_malloc(16384));
  _get3 = _this;
  len = (f64)(i32_load(1, 0, (u32)(_get3)));
  i = 0;
  // loop 
  j533:;
    _get4 = i;
    _get5 = len;
    // if 
      if (_get4 < _get5) {
        _get6 = i;
        // if 
          if (_get6 > 0) {
            _get7 = out;
            (void) dong_porf_porf_todo__Porffor_bytestring_appendChar(_get7, 195, 44, 1);
          }
        // end
        j535:;
        _get8 = i;
        _get9 = i;
        i = _get9 + 1;
        jjmember_prop_80 = _get8;
        _get10 = _this;
        jjmember_obj_80 = _get10;
        _get11 = jjmember_prop_80;
        _get12 = jjmember_obj_80;
        jjloadArray_offset = ((u32)(_get11) * 9) + (u32)(_get12);
        _get13 = jjloadArray_offset;
        _get14 = jjloadArray_offset;
        jjlast_type = i32_load8_u(0, 12, _get14);
        _get15 = jjlast_type;
        elementjjtype = _get15;
        element = f64_load(0, 4, _get13);
        _get16 = element;
        logictmpi = _get16 != 0;
        _get17 = logictmpi;
        // if i32
        i32 _r536;
          if ((_get17) == 0) {
            _get18 = elementjjtype;
            _get19 = elementjjtype;
            jjlast_type = 2;
            _r536 = ((f64)(_get18) != 0) & ((f64)(_get19) != 7);
          } else {
            _get20 = logictmpi;
            jjlast_type = 2;
            _r536 = _get20;
          }
        // end
        j536:;
        // if 
          if ((_r536) != 0) {
            _get21 = out;
            _get22 = element;
            _get23 = elementjjtype;
            const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString(_get22, _get23);
            jjlast_type = _0.type;
            _get24 = jjlast_type;
            (void) dong_porf_porf_todo__Porffor_bytestring_appendStr(_get21, 195, _0.value, _get24);
          }
        // end
        j537:;
        goto j533;
      }
    // end
    j534:;
  // end
  _get25 = out;
  return (struct ReturnValue){ _get25, 195 };
}

static struct ReturnValue dong_porf_porf_todo__ecma262_ToPrimitive_String(f64 input, i32 inputjjtype) {
  i32 _get98;
  f64 _get97;
  i32 _get96;
  i32 _get95;
  f64 _get94;
  i32 _get93;
  f64 _get92;
  i32 _get91;
  i32 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  i32 _get86;
  f64 _get85;
  i32 _get84;
  f64 _get83;
  i32 _get82;
  i32 _get81;
  f64 _get80;
  i32 _get79;
  i32 _get78;
  f64 _get77;
  i32 _get76;
  i32 _get75;
  f64 _get74;
  i32 _get73;
  i32 _get72;
  f64 _get71;
  i32 _get70;
  i32 _get69;
  f64 _get68;
  i32 _get67;
  i32 _get66;
  f64 _get65;
  i32 _get64;
  i32 _get63;
  f64 _get62;
  i32 _get61;
  i32 _get60;
  f64 _get59;
  i32 _get58;
  i32 _get57;
  i32 _get56;
  i32 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  i32 _get51;
  i32 _get50;
  f64 _get49;
  i32 _get48;
  f64 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  i32 _get41;
  f64 _get40;
  i32 _get39;
  f64 _get38;
  i32 _get37;
  i32 _get36;
  f64 _get35;
  i32 _get34;
  i32 _get33;
  f64 _get32;
  i32 _get31;
  i32 _get30;
  f64 _get29;
  i32 _get28;
  i32 _get27;
  f64 _get26;
  i32 _get25;
  i32 _get24;
  f64 _get23;
  i32 _get22;
  i32 _get21;
  f64 _get20;
  i32 _get19;
  i32 _get18;
  f64 _get17;
  i32 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  i32 _get12;
  f64 _get11;
  i32 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  f64 _get0;
  f64 value = 0;
  i32 valuejjtype = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  i32 jjlast_type = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 jjlogicinner_tmp = 0;
  i32 logictmpi = 0;

  _get0 = input;
  jjproto_target = _get0;
  _get1 = inputjjtype;
  jjproto_targetjjtype = _get1;
  _get2 = inputjjtype;
  jjtypeswitch_tmp1 = _get2;
  // block f64
  f64 _r69;
    _get3 = jjtypeswitch_tmp1;
    // if 
      if (_get3 == 0) {
        jjlast_type = 0;
        _r69 = 0;
        goto j69;
      }
    // end
    j70:;
    _get4 = jjtypeswitch_tmp1;
    // if 
      if (_get4 == 1) {
        _get5 = jjproto_target;
        _get6 = jjproto_targetjjtype;
        const struct ReturnValue _0 = dong_porf_porf_todo__Number_prototype_toString(_get5, _get6, 0, 0);
        jjlast_type = _0.type;
        _r69 = _0.value;
        goto j69;
      }
    // end
    j71:;
    _get7 = jjtypeswitch_tmp1;
    // if 
      if (_get7 == 2) {
        _get8 = jjproto_target;
        _get9 = jjproto_targetjjtype;
        const struct ReturnValue _1 = dong_porf_porf_todo__Boolean_prototype_toString(_get8, _get9);
        jjlast_type = _1.type;
        _r69 = _1.value;
        goto j69;
      }
    // end
    j72:;
    _get10 = jjtypeswitch_tmp1;
    // if 
      if (_get10 == 6) {
        _get11 = jjproto_target;
        _get12 = jjproto_targetjjtype;
        const struct ReturnValue _2 = dong_porf_porf_todo__Function_prototype_toString(_get11, _get12);
        jjlast_type = _2.type;
        _r69 = _2.value;
        goto j69;
      }
    // end
    j74:;
    _get13 = jjtypeswitch_tmp1;
    // if 
      if (_get13 == 7) {
        _get14 = jjproto_target;
        _get15 = jjproto_targetjjtype;
        const struct ReturnValue _3 = dong_porf_porf_todo__Object_prototype_toString(_get14, _get15);
        jjlast_type = _3.type;
        _r69 = _3.value;
        goto j69;
      }
    // end
    j78:;
    _get16 = jjtypeswitch_tmp1;
    // if 
      if (_get16 == 31) {
        _get17 = jjproto_target;
        _get18 = jjproto_targetjjtype;
        const struct ReturnValue _4 = dong_porf_porf_todo__Boolean_prototype_toString(_get17, _get18);
        jjlast_type = _4.type;
        _r69 = _4.value;
        goto j69;
      }
    // end
    j150:;
    _get19 = jjtypeswitch_tmp1;
    // if 
      if (_get19 == 32) {
        _get20 = jjproto_target;
        _get21 = jjproto_targetjjtype;
        const struct ReturnValue _5 = dong_porf_porf_todo__Number_prototype_toString(_get20, _get21, 0, 0);
        jjlast_type = _5.type;
        _r69 = _5.value;
        goto j69;
      }
    // end
    j151:;
    _get22 = jjtypeswitch_tmp1;
    // if 
      if (_get22 == 33) {
        _get23 = jjproto_target;
        _get24 = jjproto_targetjjtype;
        const struct ReturnValue _6 = dong_porf_porf_todo__String_prototype_toString((i32)(_get23), _get24);
        jjlast_type = _6.type;
        _r69 = (f64)(_6.value);
        goto j69;
      }
    // end
    j152:;
    _get25 = jjtypeswitch_tmp1;
    // if 
      if (_get25 == 38) {
        _get26 = jjproto_target;
        _get27 = jjproto_targetjjtype;
        const struct ReturnValue _7 = dong_porf_porf_todo__TypeError_prototype_toString(_get26, _get27);
        jjlast_type = _7.type;
        _r69 = _7.value;
        goto j69;
      }
    // end
    j157:;
    _get28 = jjtypeswitch_tmp1;
    // if 
      if (_get28 == 41) {
        _get29 = jjproto_target;
        _get30 = jjproto_targetjjtype;
        const struct ReturnValue _8 = dong_porf_porf_todo__RangeError_prototype_toString(_get29, _get30);
        jjlast_type = _8.type;
        _r69 = _8.value;
        goto j69;
      }
    // end
    j168:;
    _get31 = jjtypeswitch_tmp1;
    // if 
      if (_get31 == 67) {
        _get32 = jjproto_target;
        _get33 = jjproto_targetjjtype;
        const struct ReturnValue _9 = dong_porf_porf_todo__String_prototype_toString((i32)(_get32), _get33);
        jjlast_type = _9.type;
        _r69 = (f64)(_9.value);
        goto j69;
      }
    // end
    j170:;
    _get34 = jjtypeswitch_tmp1;
    // if 
      if (_get34 == 72) {
        _get35 = jjproto_target;
        _get36 = jjproto_targetjjtype;
        const struct ReturnValue _10 = dong_porf_porf_todo__Array_prototype_toString(_get35, _get36);
        jjlast_type = _10.type;
        _r69 = _10.value;
        goto j69;
      }
    // end
    j171:;
    _get37 = jjtypeswitch_tmp1;
    // if 
      if (_get37 == 195) {
        _get38 = jjproto_target;
        _get39 = jjproto_targetjjtype;
        const struct ReturnValue _11 = dong_porf_porf_todo__ByteString_prototype_toString((i32)(_get38), _get39);
        jjlast_type = _11.type;
        _r69 = (f64)(_11.value);
        goto j69;
      }
    // end
    j538:;
    _get40 = jjproto_target;
    _get41 = jjproto_targetjjtype;
    const struct ReturnValue _12 = dong_porf_porf_todo__Object_prototype_toString(_get40, _get41);
    jjlast_type = _12.type;
    _r69 = _12.value;
  // end
  j69:;
  value = _r69;
  _get42 = jjlast_type;
  valuejjtype = _get42;
  _get43 = value;
  jjlogicinner_tmp = _get43;
  _get44 = valuejjtype;
  jjtypeswitch_tmp1 = _get44;
  // block i32
  i32 _r539;
    _get45 = jjtypeswitch_tmp1;
    // if 
      if (_get45 == 0) {
        _r539 = 1;
        goto j539;
      }
    // end
    j540:;
    _get46 = jjtypeswitch_tmp1;
    // if 
      if (_get46 == 7) {
        _get47 = jjlogicinner_tmp;
        _r539 = _get47 == 0;
        goto j539;
      }
    // end
    j541:;
    _r539 = 0;
  // end
  j539:;
  logictmpi = (_r539) == 0;
  _get48 = logictmpi;
  // if i32
  i32 _r542;
    if ((_get48) != 0) {
      _get49 = value;
      _get50 = valuejjtype;
      jjlast_type = 2;
      _r542 = (f64)(dong_porf_porf_todo__Porffor_object_isObjectOrNull((i32)(_get49), _get50)) == 0;
    } else {
      _get51 = logictmpi;
      jjlast_type = 2;
      _r542 = _get51;
    }
  // end
  j542:;
  // if 
    if ((_r542) != 0) {
      _get52 = value;
      _get53 = valuejjtype;
      return (struct ReturnValue){ _get52, _get53 };
    }
  // end
  j543:;
  _get54 = input;
  jjproto_target = _get54;
  _get55 = inputjjtype;
  jjproto_targetjjtype = _get55;
  _get56 = inputjjtype;
  jjtypeswitch_tmp1 = _get56;
  // block f64
  f64 _r544;
    _get57 = jjtypeswitch_tmp1;
    // if 
      if (_get57 == 0) {
        jjlast_type = 0;
        _r544 = 0;
        goto j544;
      }
    // end
    j545:;
    _get58 = jjtypeswitch_tmp1;
    // if 
      if (_get58 == 1) {
        _get59 = jjproto_target;
        _get60 = jjproto_targetjjtype;
        const struct ReturnValue _13 = dong_porf_porf_todo__Number_prototype_valueOf(_get59, _get60);
        jjlast_type = _13.type;
        _r544 = _13.value;
        goto j544;
      }
    // end
    j546:;
    _get61 = jjtypeswitch_tmp1;
    // if 
      if (_get61 == 2) {
        _get62 = jjproto_target;
        _get63 = jjproto_targetjjtype;
        const struct ReturnValue _14 = dong_porf_porf_todo__Boolean_prototype_valueOf(_get62, _get63);
        jjlast_type = _14.type;
        _r544 = _14.value;
        goto j544;
      }
    // end
    j547:;
    _get64 = jjtypeswitch_tmp1;
    // if 
      if (_get64 == 7) {
        _get65 = jjproto_target;
        _get66 = jjproto_targetjjtype;
        const struct ReturnValue _15 = dong_porf_porf_todo__Object_prototype_valueOf(_get65, _get66);
        jjlast_type = _15.type;
        _r544 = _15.value;
        goto j544;
      }
    // end
    j548:;
    _get67 = jjtypeswitch_tmp1;
    // if 
      if (_get67 == 31) {
        _get68 = jjproto_target;
        _get69 = jjproto_targetjjtype;
        const struct ReturnValue _16 = dong_porf_porf_todo__Boolean_prototype_valueOf(_get68, _get69);
        jjlast_type = _16.type;
        _r544 = _16.value;
        goto j544;
      }
    // end
    j549:;
    _get70 = jjtypeswitch_tmp1;
    // if 
      if (_get70 == 32) {
        _get71 = jjproto_target;
        _get72 = jjproto_targetjjtype;
        const struct ReturnValue _17 = dong_porf_porf_todo__Number_prototype_valueOf(_get71, _get72);
        jjlast_type = _17.type;
        _r544 = _17.value;
        goto j544;
      }
    // end
    j550:;
    _get73 = jjtypeswitch_tmp1;
    // if 
      if (_get73 == 33) {
        _get74 = jjproto_target;
        _get75 = jjproto_targetjjtype;
        const struct ReturnValue _18 = dong_porf_porf_todo__String_prototype_valueOf((i32)(_get74), _get75);
        jjlast_type = _18.type;
        _r544 = (f64)(_18.value);
        goto j544;
      }
    // end
    j551:;
    _get76 = jjtypeswitch_tmp1;
    // if 
      if (_get76 == 67) {
        _get77 = jjproto_target;
        _get78 = jjproto_targetjjtype;
        const struct ReturnValue _19 = dong_porf_porf_todo__String_prototype_valueOf((i32)(_get77), _get78);
        jjlast_type = _19.type;
        _r544 = (f64)(_19.value);
        goto j544;
      }
    // end
    j552:;
    _get79 = jjtypeswitch_tmp1;
    // if 
      if (_get79 == 72) {
        _get80 = jjproto_target;
        _get81 = jjproto_targetjjtype;
        const struct ReturnValue _20 = dong_porf_porf_todo__Array_prototype_valueOf(_get80, _get81);
        jjlast_type = _20.type;
        _r544 = _20.value;
        goto j544;
      }
    // end
    j553:;
    _get82 = jjtypeswitch_tmp1;
    // if 
      if (_get82 == 195) {
        _get83 = jjproto_target;
        _get84 = jjproto_targetjjtype;
        const struct ReturnValue _21 = dong_porf_porf_todo__ByteString_prototype_valueOf((i32)(_get83), _get84);
        jjlast_type = _21.type;
        _r544 = (f64)(_21.value);
        goto j544;
      }
    // end
    j554:;
    _get85 = jjproto_target;
    _get86 = jjproto_targetjjtype;
    const struct ReturnValue _22 = dong_porf_porf_todo__Object_prototype_valueOf(_get85, _get86);
    jjlast_type = _22.type;
    _r544 = _22.value;
  // end
  j544:;
  value = _r544;
  _get87 = jjlast_type;
  valuejjtype = _get87;
  _get88 = value;
  jjlogicinner_tmp = _get88;
  _get89 = valuejjtype;
  jjtypeswitch_tmp1 = _get89;
  // block i32
  i32 _r555;
    _get90 = jjtypeswitch_tmp1;
    // if 
      if (_get90 == 0) {
        _r555 = 1;
        goto j555;
      }
    // end
    j556:;
    _get91 = jjtypeswitch_tmp1;
    // if 
      if (_get91 == 7) {
        _get92 = jjlogicinner_tmp;
        _r555 = _get92 == 0;
        goto j555;
      }
    // end
    j557:;
    _r555 = 0;
  // end
  j555:;
  logictmpi = (_r555) == 0;
  _get93 = logictmpi;
  // if i32
  i32 _r558;
    if ((_get93) != 0) {
      _get94 = value;
      _get95 = valuejjtype;
      jjlast_type = 2;
      _r558 = (f64)(dong_porf_porf_todo__Porffor_object_isObjectOrNull((i32)(_get94), _get95)) == 0;
    } else {
      _get96 = logictmpi;
      jjlast_type = 2;
      _r558 = _get96;
    }
  // end
  j558:;
  // if 
    if ((_r558) != 0) {
      _get97 = value;
      _get98 = valuejjtype;
      return (struct ReturnValue){ _get97, _get98 };
    }
  // end
  j559:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo__ecma262_ToString(f64 argument, i32 argumentjjtype) {
  i32 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  i32 _get16;
  f64 _get15;
  f64 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  i32 jjlast_type = 0;
  f64 primValue = 0;
  i32 primValuejjtype = 0;

  _get0 = argumentjjtype;
  // if 
    if ((f64)(_get0 | 128) == 195) {
      _get1 = argument;
      _get2 = argumentjjtype;
      return (struct ReturnValue){ _get1, _get2 };
    }
  // end
  j3:;
  _get3 = argumentjjtype;
  // if 
    if ((f64)(_get3) == 5) {
    }
  // end
  j4:;
  _get4 = argumentjjtype;
  // if 
    if ((f64)(_get4) == 0) {
      return (struct ReturnValue){ 197, 195 };
    }
  // end
  j5:;
  _get5 = argumentjjtype;
  _get6 = argument;
  // if 
    if ((((f64)(_get5) == 7) & (_get6 == 0)) != 0) {
      return (struct ReturnValue){ 212, 195 };
    }
  // end
  j6:;
  _get7 = argumentjjtype;
  // if 
    if ((f64)(_get7) == 2) {
      _get8 = argument;
      // if 
        if (_get8 == 1) {
          return (struct ReturnValue){ 222, 195 };
        }
      // end
      j8:;
      return (struct ReturnValue){ 232, 195 };
    }
  // end
  j7:;
  _get9 = argumentjjtype;
  // if 
    if ((f64)(_get9) == 1) {
      _get10 = argument;
      _get11 = argumentjjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__Number_prototype_toString(_get10, _get11, 10, 1);
      jjlast_type = _0.type;
      _get12 = jjlast_type;
      return (struct ReturnValue){ _0.value, _get12 };
    }
  // end
  j9:;
  _get13 = argumentjjtype;
  // if 
    if ((f64)(_get13) == 33) {
      _get14 = argument;
      return (struct ReturnValue){ _get14, 67 };
    }
  // end
  j68:;
  _get15 = argument;
  _get16 = argumentjjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo__ecma262_ToPrimitive_String(_get15, _get16);
  jjlast_type = _1.type;
  _get17 = jjlast_type;
  primValuejjtype = _get17;
  primValue = _1.value;
  _get18 = primValue;
  _get19 = primValuejjtype;
  const struct ReturnValue _2 = dong_porf_porf_todo__ecma262_ToString(_get18, _get19);
  jjlast_type = _2.type;
  _get20 = jjlast_type;
  return (struct ReturnValue){ _2.value, _get20 };
}

static f64 dong_porf_porf_todo_TypeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype) {
  f64 _get9;
  i32 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  i32 jjlast_type = 0;
  f64 obj = 0;

  _get0 = message;
  _get1 = messagejjtype;
  // if 
    if (((_get0 == 0) & ((_get1 | 128) == (0 | 128))) != 0) {
      message = 0;
      messagejjtype = 195;
    } else {
      _get2 = message;
      _get3 = messagejjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString(_get2, _get3);
      jjlast_type = _0.type;
      _get4 = jjlast_type;
      messagejjtype = _get4;
      message = _0.value;
    }
  // end
  j2:;
  obj = (f64)(dong_porf_porf_todo__Porffor_malloc(8));
  _get5 = obj;
  _get6 = message;
  i32_store(0, 0, (i32)(_get5), (i32)(_get6));
  _get7 = obj;
  _get8 = messagejjtype;
  i32_store8(0, 4, (i32)(_get7), _get8);
  _get9 = obj;
  return _get9;
}

static struct ReturnValue dong_porf_porf_todo__String_fromCharCode(f64 codes, i32 codesjjtype) {
  f64 _get27;
  f64 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  f64 _get22;
  f64 _get21;
  f64 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  f64 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 out = 0;
  f64 len = 0;
  f64 bytestringable = 0;
  f64 i = 0;
  f64 v = 0;
  f64 jjmember_obj_426 = 0;
  f64 jjmember_prop_426 = 0;
  i32 jjlast_type = 0;
  i32 jjloadArray_offset = 0;
  f64 out2 = 0;

  out = (f64)(dong_porf_porf_todo__Porffor_malloc(16384));
  _get0 = codes;
  len = (f64)(i32_load(1, 0, (u32)(_get0)));
  _get1 = out;
  _get2 = len;
  i32_store(1, 0, (u32)(_get1), (u32)(_get2));
  bytestringable = 1;
  i = 0;
  // loop 
  j586:;
    _get3 = i;
    _get4 = len;
    // if 
      if (_get3 < _get4) {
        _get5 = i;
        jjmember_prop_426 = _get5;
        _get6 = codes;
        jjmember_obj_426 = _get6;
        _get7 = jjmember_prop_426;
        _get8 = jjmember_obj_426;
        jjloadArray_offset = ((u32)(_get7) * 9) + (u32)(_get8);
        _get9 = jjloadArray_offset;
        _get10 = jjloadArray_offset;
        jjlast_type = i32_load8_u(0, 12, _get10);
        _get11 = jjlast_type;
        v = dong_porf_porf_todo__ecma262_ToIntegerOrInfinity(f64_load(0, 4, _get9), _get11);
        _get12 = v;
        // if 
          if (_get12 > 255) {
            bytestringable = 0;
          }
        // end
        j588:;
        _get13 = out;
        _get14 = i;
        _get15 = v;
        i32_store16(0, 4, (i32)((_get13 + (_get14 * 2))), (i32)(_get15));
        _get16 = i;
        i = _get16 + 1;
        goto j586;
      }
    // end
    j587:;
  // end
  _get17 = bytestringable;
  // if 
    if (((u32)(_get17)) != 0) {
      _get18 = out;
      out2 = _get18;
      i = 0;
      // loop 
      j590:;
        _get19 = i;
        _get20 = len;
        // if 
          if (_get19 < _get20) {
            _get21 = out;
            _get22 = i;
            _get23 = out;
            _get24 = i;
            i32_store8(0, 4, (i32)((_get21 + _get22)), i32_load8_u(0, 4, (i32)((_get23 + (_get24 * 2)))));
            _get25 = i;
            i = _get25 + 1;
            goto j590;
          }
        // end
        j591:;
      // end
      _get26 = out2;
      return (struct ReturnValue){ _get26, 195 };
    }
  // end
  j589:;
  _get27 = out;
  return (struct ReturnValue){ _get27, 67 };
}

static struct ReturnValue dong_porf_porf_todo_utf8AppendCodePoint(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 out, i32 outjjtype, f64 cp, i32 cpjjtype) {
  i32 _get207;
  f64 _get206;
  i32 _get205;
  f64 _get204;
  i32 _get203;
  f64 _get202;
  f64 _get201;
  i32 _get200;
  i32 _get199;
  f64 _get198;
  i32 _get197;
  f64 _get196;
  i32 _get195;
  i32 _get194;
  f64 _get193;
  f64 _get192;
  f64 _get191;
  f64 _get190;
  f64 _get189;
  f64 _get188;
  f64 _get187;
  f64 _get186;
  f64 _get185;
  f64 _get184;
  f64 _get183;
  f64 _get182;
  f64 _get181;
  f64 _get180;
  f64 _get179;
  f64 _get178;
  f64 _get177;
  f64 _get176;
  f64 _get175;
  f64 _get174;
  f64 _get173;
  f64 _get172;
  f64 _get171;
  f64 _get170;
  f64 _get169;
  f64 _get168;
  f64 _get167;
  f64 _get166;
  f64 _get165;
  f64 _get164;
  f64 _get163;
  f64 _get162;
  f64 _get161;
  f64 _get160;
  f64 _get159;
  f64 _get158;
  f64 _get157;
  f64 _get156;
  f64 _get155;
  f64 _get154;
  f64 _get153;
  f64 _get152;
  f64 _get151;
  f64 _get150;
  f64 _get149;
  f64 _get148;
  f64 _get147;
  f64 _get146;
  f64 _get145;
  f64 _get144;
  f64 _get143;
  f64 _get142;
  f64 _get141;
  f64 _get140;
  f64 _get139;
  f64 _get138;
  f64 _get137;
  f64 _get136;
  f64 _get135;
  f64 _get134;
  f64 _get133;
  f64 _get132;
  f64 _get131;
  f64 _get130;
  f64 _get129;
  f64 _get128;
  f64 _get127;
  i32 _get126;
  f64 _get125;
  i32 _get124;
  f64 _get123;
  i32 _get122;
  f64 _get121;
  f64 _get120;
  i32 _get119;
  i32 _get118;
  f64 _get117;
  i32 _get116;
  f64 _get115;
  i32 _get114;
  i32 _get113;
  f64 _get112;
  f64 _get111;
  f64 _get110;
  f64 _get109;
  f64 _get108;
  f64 _get107;
  f64 _get106;
  f64 _get105;
  f64 _get104;
  f64 _get103;
  f64 _get102;
  f64 _get101;
  f64 _get100;
  f64 _get99;
  f64 _get98;
  f64 _get97;
  f64 _get96;
  f64 _get95;
  f64 _get94;
  f64 _get93;
  f64 _get92;
  f64 _get91;
  f64 _get90;
  f64 _get89;
  f64 _get88;
  f64 _get87;
  f64 _get86;
  f64 _get85;
  f64 _get84;
  f64 _get83;
  f64 _get82;
  f64 _get81;
  f64 _get80;
  f64 _get79;
  f64 _get78;
  f64 _get77;
  f64 _get76;
  f64 _get75;
  f64 _get74;
  f64 _get73;
  f64 _get72;
  f64 _get71;
  f64 _get70;
  f64 _get69;
  f64 _get68;
  f64 _get67;
  f64 _get66;
  f64 _get65;
  f64 _get64;
  i32 _get63;
  f64 _get62;
  i32 _get61;
  f64 _get60;
  i32 _get59;
  f64 _get58;
  f64 _get57;
  i32 _get56;
  i32 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  i32 _get51;
  i32 _get50;
  f64 _get49;
  f64 _get48;
  f64 _get47;
  f64 _get46;
  f64 _get45;
  f64 _get44;
  f64 _get43;
  f64 _get42;
  f64 _get41;
  f64 _get40;
  f64 _get39;
  f64 _get38;
  f64 _get37;
  f64 _get36;
  f64 _get35;
  f64 _get34;
  f64 _get33;
  f64 _get32;
  f64 _get31;
  f64 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  f64 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  f64 _get22;
  f64 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  f64 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;
  f64 __tmpop_left = 0;
  f64 __tmpop_right = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;
  f64 jjbitwise_left = 0;
  f64 jjbitwise_right = 0;

  _get0 = cp;
  // if 
    if ((f64)(_get0 < 128) != 0) {
      // block f64
      f64 _r585;
        _get1 = out;
        __tmpop_left = _get1;
        _get2 = __tmpop_left;
        _get3 = cp;
        f64_store(0, 4, 147456, _get3);
        _get4 = cpjjtype;
        i32_store8(0, 12, 147456, _get4);
        i32_store(1, 0, 147456, 1);
        const struct ReturnValue _0 = dong_porf_porf_todo__String_fromCharCode(147456, 72);
        jjlast_type = _0.type;
        __tmpop_right = _0.value;
        _get5 = __tmpop_right;
        _get6 = outjjtype;
        _get7 = jjlast_type;
        // if 
          if ((((_get6 | 128) == 195) | ((_get7 | 128) == 195)) != 0) {
            _get8 = __tmpop_left;
            _get9 = outjjtype;
            _get10 = __tmpop_right;
            _get11 = jjlast_type;
            const struct ReturnValue _1 = dong_porf_porf_todo__Porffor_concatStrings(_get8, _get9, _get10, _get11);
            jjlast_type = _1.type;
            _r585 = _1.value;
            goto j585;
          }
        // end
        j592:;
        jjlast_type = 1;
        _r585 = _get2 + _get5;
      // end
      j585:;
      jjreturn = _r585;
      _get12 = jjlast_type;
      jjreturnjjtype = _get12;
      _get13 = jjnewtarget;
      // if 
        if (((u32)(_get13)) != 0) {
          _get14 = jjreturn;
          _get15 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get14), _get15)) == 0) {
              _get16 = jjthis;
              _get17 = jjthisjjtype;
              return (struct ReturnValue){ _get16, _get17 };
            }
          // end
          j594:;
        }
      // end
      j593:;
      _get18 = jjreturn;
      _get19 = jjreturnjjtype;
      return (struct ReturnValue){ _get18, _get19 };
    }
  // end
  j584:;
  _get20 = cp;
  // if 
    if ((f64)(_get20 < 2048) != 0) {
      // block f64
      f64 _r596;
        _get21 = out;
        __tmpop_left = _get21;
        _get22 = __tmpop_left;
        jjbitwise_left = 192;
        _get23 = jjbitwise_left;
        _get24 = jjbitwise_left;
        // if i32
        i32 _r597;
          if ((_get23 - _get24) == 0) {
            _get25 = jjbitwise_left;
            _r597 = (i32)((i64)(_get25));
          } else {
            _r597 = 0;
          }
        // end
        j597:;
        _get26 = cp;
        jjbitwise_left = _get26;
        _get27 = jjbitwise_left;
        _get28 = jjbitwise_left;
        // if i32
        i32 _r598;
          if ((_get27 - _get28) == 0) {
            _get29 = jjbitwise_left;
            _r598 = (i32)((i64)(_get29));
          } else {
            _r598 = 0;
          }
        // end
        j598:;
        jjbitwise_right = 6;
        _get30 = jjbitwise_right;
        _get31 = jjbitwise_right;
        // if i32
        i32 _r599;
          if ((_get30 - _get31) == 0) {
            _get32 = jjbitwise_right;
            _r599 = (i32)((i64)(_get32));
          } else {
            _r599 = 0;
          }
        // end
        j599:;
        jjbitwise_right = (f64)(_r598 >> _r599);
        _get33 = jjbitwise_right;
        _get34 = jjbitwise_right;
        // if i32
        i32 _r600;
          if ((_get33 - _get34) == 0) {
            _get35 = jjbitwise_right;
            _r600 = (i32)((i64)(_get35));
          } else {
            _r600 = 0;
          }
        // end
        j600:;
        f64_store(0, 4, 163840, (f64)(_r597 | _r600));
        i32_store8(0, 12, 163840, 1);
        jjbitwise_left = 128;
        _get36 = jjbitwise_left;
        _get37 = jjbitwise_left;
        // if i32
        i32 _r601;
          if ((_get36 - _get37) == 0) {
            _get38 = jjbitwise_left;
            _r601 = (i32)((i64)(_get38));
          } else {
            _r601 = 0;
          }
        // end
        j601:;
        _get39 = cp;
        jjbitwise_left = _get39;
        _get40 = jjbitwise_left;
        _get41 = jjbitwise_left;
        // if i32
        i32 _r602;
          if ((_get40 - _get41) == 0) {
            _get42 = jjbitwise_left;
            _r602 = (i32)((i64)(_get42));
          } else {
            _r602 = 0;
          }
        // end
        j602:;
        jjbitwise_right = 63;
        _get43 = jjbitwise_right;
        _get44 = jjbitwise_right;
        // if i32
        i32 _r603;
          if ((_get43 - _get44) == 0) {
            _get45 = jjbitwise_right;
            _r603 = (i32)((i64)(_get45));
          } else {
            _r603 = 0;
          }
        // end
        j603:;
        jjbitwise_right = (f64)(_r602 & _r603);
        _get46 = jjbitwise_right;
        _get47 = jjbitwise_right;
        // if i32
        i32 _r604;
          if ((_get46 - _get47) == 0) {
            _get48 = jjbitwise_right;
            _r604 = (i32)((i64)(_get48));
          } else {
            _r604 = 0;
          }
        // end
        j604:;
        f64_store(0, 13, 163840, (f64)(_r601 | _r604));
        i32_store8(0, 21, 163840, 1);
        i32_store(1, 0, 163840, 2);
        const struct ReturnValue _2 = dong_porf_porf_todo__String_fromCharCode(163840, 72);
        jjlast_type = _2.type;
        __tmpop_right = _2.value;
        _get49 = __tmpop_right;
        _get50 = outjjtype;
        _get51 = jjlast_type;
        // if 
          if ((((_get50 | 128) == 195) | ((_get51 | 128) == 195)) != 0) {
            _get52 = __tmpop_left;
            _get53 = outjjtype;
            _get54 = __tmpop_right;
            _get55 = jjlast_type;
            const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_concatStrings(_get52, _get53, _get54, _get55);
            jjlast_type = _3.type;
            _r596 = _3.value;
            goto j596;
          }
        // end
        j605:;
        jjlast_type = 1;
        _r596 = _get22 + _get49;
      // end
      j596:;
      jjreturn = _r596;
      _get56 = jjlast_type;
      jjreturnjjtype = _get56;
      _get57 = jjnewtarget;
      // if 
        if (((u32)(_get57)) != 0) {
          _get58 = jjreturn;
          _get59 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get58), _get59)) == 0) {
              _get60 = jjthis;
              _get61 = jjthisjjtype;
              return (struct ReturnValue){ _get60, _get61 };
            }
          // end
          j607:;
        }
      // end
      j606:;
      _get62 = jjreturn;
      _get63 = jjreturnjjtype;
      return (struct ReturnValue){ _get62, _get63 };
    }
  // end
  j595:;
  _get64 = cp;
  // if 
    if ((f64)(_get64 < 65536) != 0) {
      // block f64
      f64 _r609;
        _get65 = out;
        __tmpop_left = _get65;
        _get66 = __tmpop_left;
        jjbitwise_left = 224;
        _get67 = jjbitwise_left;
        _get68 = jjbitwise_left;
        // if i32
        i32 _r610;
          if ((_get67 - _get68) == 0) {
            _get69 = jjbitwise_left;
            _r610 = (i32)((i64)(_get69));
          } else {
            _r610 = 0;
          }
        // end
        j610:;
        _get70 = cp;
        jjbitwise_left = _get70;
        _get71 = jjbitwise_left;
        _get72 = jjbitwise_left;
        // if i32
        i32 _r611;
          if ((_get71 - _get72) == 0) {
            _get73 = jjbitwise_left;
            _r611 = (i32)((i64)(_get73));
          } else {
            _r611 = 0;
          }
        // end
        j611:;
        jjbitwise_right = 12;
        _get74 = jjbitwise_right;
        _get75 = jjbitwise_right;
        // if i32
        i32 _r612;
          if ((_get74 - _get75) == 0) {
            _get76 = jjbitwise_right;
            _r612 = (i32)((i64)(_get76));
          } else {
            _r612 = 0;
          }
        // end
        j612:;
        jjbitwise_right = (f64)(_r611 >> _r612);
        _get77 = jjbitwise_right;
        _get78 = jjbitwise_right;
        // if i32
        i32 _r613;
          if ((_get77 - _get78) == 0) {
            _get79 = jjbitwise_right;
            _r613 = (i32)((i64)(_get79));
          } else {
            _r613 = 0;
          }
        // end
        j613:;
        f64_store(0, 4, 180224, (f64)(_r610 | _r613));
        i32_store8(0, 12, 180224, 1);
        jjbitwise_left = 128;
        _get80 = jjbitwise_left;
        _get81 = jjbitwise_left;
        // if i32
        i32 _r614;
          if ((_get80 - _get81) == 0) {
            _get82 = jjbitwise_left;
            _r614 = (i32)((i64)(_get82));
          } else {
            _r614 = 0;
          }
        // end
        j614:;
        _get83 = cp;
        jjbitwise_left = _get83;
        _get84 = jjbitwise_left;
        _get85 = jjbitwise_left;
        // if i32
        i32 _r615;
          if ((_get84 - _get85) == 0) {
            _get86 = jjbitwise_left;
            _r615 = (i32)((i64)(_get86));
          } else {
            _r615 = 0;
          }
        // end
        j615:;
        jjbitwise_right = 6;
        _get87 = jjbitwise_right;
        _get88 = jjbitwise_right;
        // if i32
        i32 _r616;
          if ((_get87 - _get88) == 0) {
            _get89 = jjbitwise_right;
            _r616 = (i32)((i64)(_get89));
          } else {
            _r616 = 0;
          }
        // end
        j616:;
        jjbitwise_left = (f64)(_r615 >> _r616);
        _get90 = jjbitwise_left;
        _get91 = jjbitwise_left;
        // if i32
        i32 _r617;
          if ((_get90 - _get91) == 0) {
            _get92 = jjbitwise_left;
            _r617 = (i32)((i64)(_get92));
          } else {
            _r617 = 0;
          }
        // end
        j617:;
        jjbitwise_right = 63;
        _get93 = jjbitwise_right;
        _get94 = jjbitwise_right;
        // if i32
        i32 _r618;
          if ((_get93 - _get94) == 0) {
            _get95 = jjbitwise_right;
            _r618 = (i32)((i64)(_get95));
          } else {
            _r618 = 0;
          }
        // end
        j618:;
        jjbitwise_right = (f64)(_r617 & _r618);
        _get96 = jjbitwise_right;
        _get97 = jjbitwise_right;
        // if i32
        i32 _r619;
          if ((_get96 - _get97) == 0) {
            _get98 = jjbitwise_right;
            _r619 = (i32)((i64)(_get98));
          } else {
            _r619 = 0;
          }
        // end
        j619:;
        f64_store(0, 13, 180224, (f64)(_r614 | _r619));
        i32_store8(0, 21, 180224, 1);
        jjbitwise_left = 128;
        _get99 = jjbitwise_left;
        _get100 = jjbitwise_left;
        // if i32
        i32 _r620;
          if ((_get99 - _get100) == 0) {
            _get101 = jjbitwise_left;
            _r620 = (i32)((i64)(_get101));
          } else {
            _r620 = 0;
          }
        // end
        j620:;
        _get102 = cp;
        jjbitwise_left = _get102;
        _get103 = jjbitwise_left;
        _get104 = jjbitwise_left;
        // if i32
        i32 _r621;
          if ((_get103 - _get104) == 0) {
            _get105 = jjbitwise_left;
            _r621 = (i32)((i64)(_get105));
          } else {
            _r621 = 0;
          }
        // end
        j621:;
        jjbitwise_right = 63;
        _get106 = jjbitwise_right;
        _get107 = jjbitwise_right;
        // if i32
        i32 _r622;
          if ((_get106 - _get107) == 0) {
            _get108 = jjbitwise_right;
            _r622 = (i32)((i64)(_get108));
          } else {
            _r622 = 0;
          }
        // end
        j622:;
        jjbitwise_right = (f64)(_r621 & _r622);
        _get109 = jjbitwise_right;
        _get110 = jjbitwise_right;
        // if i32
        i32 _r623;
          if ((_get109 - _get110) == 0) {
            _get111 = jjbitwise_right;
            _r623 = (i32)((i64)(_get111));
          } else {
            _r623 = 0;
          }
        // end
        j623:;
        f64_store(0, 22, 180224, (f64)(_r620 | _r623));
        i32_store8(0, 30, 180224, 1);
        i32_store(1, 0, 180224, 3);
        const struct ReturnValue _4 = dong_porf_porf_todo__String_fromCharCode(180224, 72);
        jjlast_type = _4.type;
        __tmpop_right = _4.value;
        _get112 = __tmpop_right;
        _get113 = outjjtype;
        _get114 = jjlast_type;
        // if 
          if ((((_get113 | 128) == 195) | ((_get114 | 128) == 195)) != 0) {
            _get115 = __tmpop_left;
            _get116 = outjjtype;
            _get117 = __tmpop_right;
            _get118 = jjlast_type;
            const struct ReturnValue _5 = dong_porf_porf_todo__Porffor_concatStrings(_get115, _get116, _get117, _get118);
            jjlast_type = _5.type;
            _r609 = _5.value;
            goto j609;
          }
        // end
        j624:;
        jjlast_type = 1;
        _r609 = _get66 + _get112;
      // end
      j609:;
      jjreturn = _r609;
      _get119 = jjlast_type;
      jjreturnjjtype = _get119;
      _get120 = jjnewtarget;
      // if 
        if (((u32)(_get120)) != 0) {
          _get121 = jjreturn;
          _get122 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get121), _get122)) == 0) {
              _get123 = jjthis;
              _get124 = jjthisjjtype;
              return (struct ReturnValue){ _get123, _get124 };
            }
          // end
          j626:;
        }
      // end
      j625:;
      _get125 = jjreturn;
      _get126 = jjreturnjjtype;
      return (struct ReturnValue){ _get125, _get126 };
    }
  // end
  j608:;
  // block f64
  f64 _r627;
    _get127 = out;
    __tmpop_left = _get127;
    _get128 = __tmpop_left;
    jjbitwise_left = 240;
    _get129 = jjbitwise_left;
    _get130 = jjbitwise_left;
    // if i32
    i32 _r628;
      if ((_get129 - _get130) == 0) {
        _get131 = jjbitwise_left;
        _r628 = (i32)((i64)(_get131));
      } else {
        _r628 = 0;
      }
    // end
    j628:;
    _get132 = cp;
    jjbitwise_left = _get132;
    _get133 = jjbitwise_left;
    _get134 = jjbitwise_left;
    // if i32
    i32 _r629;
      if ((_get133 - _get134) == 0) {
        _get135 = jjbitwise_left;
        _r629 = (i32)((i64)(_get135));
      } else {
        _r629 = 0;
      }
    // end
    j629:;
    jjbitwise_right = 18;
    _get136 = jjbitwise_right;
    _get137 = jjbitwise_right;
    // if i32
    i32 _r630;
      if ((_get136 - _get137) == 0) {
        _get138 = jjbitwise_right;
        _r630 = (i32)((i64)(_get138));
      } else {
        _r630 = 0;
      }
    // end
    j630:;
    jjbitwise_right = (f64)(_r629 >> _r630);
    _get139 = jjbitwise_right;
    _get140 = jjbitwise_right;
    // if i32
    i32 _r631;
      if ((_get139 - _get140) == 0) {
        _get141 = jjbitwise_right;
        _r631 = (i32)((i64)(_get141));
      } else {
        _r631 = 0;
      }
    // end
    j631:;
    f64_store(0, 4, 196608, (f64)(_r628 | _r631));
    i32_store8(0, 12, 196608, 1);
    jjbitwise_left = 128;
    _get142 = jjbitwise_left;
    _get143 = jjbitwise_left;
    // if i32
    i32 _r632;
      if ((_get142 - _get143) == 0) {
        _get144 = jjbitwise_left;
        _r632 = (i32)((i64)(_get144));
      } else {
        _r632 = 0;
      }
    // end
    j632:;
    _get145 = cp;
    jjbitwise_left = _get145;
    _get146 = jjbitwise_left;
    _get147 = jjbitwise_left;
    // if i32
    i32 _r633;
      if ((_get146 - _get147) == 0) {
        _get148 = jjbitwise_left;
        _r633 = (i32)((i64)(_get148));
      } else {
        _r633 = 0;
      }
    // end
    j633:;
    jjbitwise_right = 12;
    _get149 = jjbitwise_right;
    _get150 = jjbitwise_right;
    // if i32
    i32 _r634;
      if ((_get149 - _get150) == 0) {
        _get151 = jjbitwise_right;
        _r634 = (i32)((i64)(_get151));
      } else {
        _r634 = 0;
      }
    // end
    j634:;
    jjbitwise_left = (f64)(_r633 >> _r634);
    _get152 = jjbitwise_left;
    _get153 = jjbitwise_left;
    // if i32
    i32 _r635;
      if ((_get152 - _get153) == 0) {
        _get154 = jjbitwise_left;
        _r635 = (i32)((i64)(_get154));
      } else {
        _r635 = 0;
      }
    // end
    j635:;
    jjbitwise_right = 63;
    _get155 = jjbitwise_right;
    _get156 = jjbitwise_right;
    // if i32
    i32 _r636;
      if ((_get155 - _get156) == 0) {
        _get157 = jjbitwise_right;
        _r636 = (i32)((i64)(_get157));
      } else {
        _r636 = 0;
      }
    // end
    j636:;
    jjbitwise_right = (f64)(_r635 & _r636);
    _get158 = jjbitwise_right;
    _get159 = jjbitwise_right;
    // if i32
    i32 _r637;
      if ((_get158 - _get159) == 0) {
        _get160 = jjbitwise_right;
        _r637 = (i32)((i64)(_get160));
      } else {
        _r637 = 0;
      }
    // end
    j637:;
    f64_store(0, 13, 196608, (f64)(_r632 | _r637));
    i32_store8(0, 21, 196608, 1);
    jjbitwise_left = 128;
    _get161 = jjbitwise_left;
    _get162 = jjbitwise_left;
    // if i32
    i32 _r638;
      if ((_get161 - _get162) == 0) {
        _get163 = jjbitwise_left;
        _r638 = (i32)((i64)(_get163));
      } else {
        _r638 = 0;
      }
    // end
    j638:;
    _get164 = cp;
    jjbitwise_left = _get164;
    _get165 = jjbitwise_left;
    _get166 = jjbitwise_left;
    // if i32
    i32 _r639;
      if ((_get165 - _get166) == 0) {
        _get167 = jjbitwise_left;
        _r639 = (i32)((i64)(_get167));
      } else {
        _r639 = 0;
      }
    // end
    j639:;
    jjbitwise_right = 6;
    _get168 = jjbitwise_right;
    _get169 = jjbitwise_right;
    // if i32
    i32 _r640;
      if ((_get168 - _get169) == 0) {
        _get170 = jjbitwise_right;
        _r640 = (i32)((i64)(_get170));
      } else {
        _r640 = 0;
      }
    // end
    j640:;
    jjbitwise_left = (f64)(_r639 >> _r640);
    _get171 = jjbitwise_left;
    _get172 = jjbitwise_left;
    // if i32
    i32 _r641;
      if ((_get171 - _get172) == 0) {
        _get173 = jjbitwise_left;
        _r641 = (i32)((i64)(_get173));
      } else {
        _r641 = 0;
      }
    // end
    j641:;
    jjbitwise_right = 63;
    _get174 = jjbitwise_right;
    _get175 = jjbitwise_right;
    // if i32
    i32 _r642;
      if ((_get174 - _get175) == 0) {
        _get176 = jjbitwise_right;
        _r642 = (i32)((i64)(_get176));
      } else {
        _r642 = 0;
      }
    // end
    j642:;
    jjbitwise_right = (f64)(_r641 & _r642);
    _get177 = jjbitwise_right;
    _get178 = jjbitwise_right;
    // if i32
    i32 _r643;
      if ((_get177 - _get178) == 0) {
        _get179 = jjbitwise_right;
        _r643 = (i32)((i64)(_get179));
      } else {
        _r643 = 0;
      }
    // end
    j643:;
    f64_store(0, 22, 196608, (f64)(_r638 | _r643));
    i32_store8(0, 30, 196608, 1);
    jjbitwise_left = 128;
    _get180 = jjbitwise_left;
    _get181 = jjbitwise_left;
    // if i32
    i32 _r644;
      if ((_get180 - _get181) == 0) {
        _get182 = jjbitwise_left;
        _r644 = (i32)((i64)(_get182));
      } else {
        _r644 = 0;
      }
    // end
    j644:;
    _get183 = cp;
    jjbitwise_left = _get183;
    _get184 = jjbitwise_left;
    _get185 = jjbitwise_left;
    // if i32
    i32 _r645;
      if ((_get184 - _get185) == 0) {
        _get186 = jjbitwise_left;
        _r645 = (i32)((i64)(_get186));
      } else {
        _r645 = 0;
      }
    // end
    j645:;
    jjbitwise_right = 63;
    _get187 = jjbitwise_right;
    _get188 = jjbitwise_right;
    // if i32
    i32 _r646;
      if ((_get187 - _get188) == 0) {
        _get189 = jjbitwise_right;
        _r646 = (i32)((i64)(_get189));
      } else {
        _r646 = 0;
      }
    // end
    j646:;
    jjbitwise_right = (f64)(_r645 & _r646);
    _get190 = jjbitwise_right;
    _get191 = jjbitwise_right;
    // if i32
    i32 _r647;
      if ((_get190 - _get191) == 0) {
        _get192 = jjbitwise_right;
        _r647 = (i32)((i64)(_get192));
      } else {
        _r647 = 0;
      }
    // end
    j647:;
    f64_store(0, 31, 196608, (f64)(_r644 | _r647));
    i32_store8(0, 39, 196608, 1);
    i32_store(1, 0, 196608, 4);
    const struct ReturnValue _6 = dong_porf_porf_todo__String_fromCharCode(196608, 72);
    jjlast_type = _6.type;
    __tmpop_right = _6.value;
    _get193 = __tmpop_right;
    _get194 = outjjtype;
    _get195 = jjlast_type;
    // if 
      if ((((_get194 | 128) == 195) | ((_get195 | 128) == 195)) != 0) {
        _get196 = __tmpop_left;
        _get197 = outjjtype;
        _get198 = __tmpop_right;
        _get199 = jjlast_type;
        const struct ReturnValue _7 = dong_porf_porf_todo__Porffor_concatStrings(_get196, _get197, _get198, _get199);
        jjlast_type = _7.type;
        _r627 = _7.value;
        goto j627;
      }
    // end
    j648:;
    jjlast_type = 1;
    _r627 = _get128 + _get193;
  // end
  j627:;
  jjreturn = _r627;
  _get200 = jjlast_type;
  jjreturnjjtype = _get200;
  _get201 = jjnewtarget;
  // if 
    if (((u32)(_get201)) != 0) {
      _get202 = jjreturn;
      _get203 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get202), _get203)) == 0) {
          _get204 = jjthis;
          _get205 = jjthisjjtype;
          return (struct ReturnValue){ _get204, _get205 };
        }
      // end
      j650:;
    }
  // end
  j649:;
  _get206 = jjreturn;
  _get207 = jjreturnjjtype;
  return (struct ReturnValue){ _get206, _get207 };
}

static struct ReturnValue dong_porf_porf_todo_toUtf8(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype) {
  i32 _get116;
  f64 _get115;
  i32 _get114;
  f64 _get113;
  i32 _get112;
  f64 _get111;
  f64 _get110;
  f64 _get109;
  f64 _get108;
  f64 _get107;
  i32 _get106;
  i32 _get105;
  f64 _get104;
  i32 _get103;
  f64 _get102;
  f64 _get101;
  f64 _get100;
  f64 _get99;
  f64 _get98;
  f64 _get97;
  f64 _get96;
  f64 _get95;
  f64 _get94;
  f64 _get93;
  f64 _get92;
  f64 _get91;
  i32 _get90;
  f64 _get89;
  i32 _get88;
  f64 _get87;
  i32 _get86;
  f64 _get85;
  f64 _get84;
  i32 _get83;
  f64 _get82;
  i32 _get81;
  f64 _get80;
  i32 _get79;
  f64 _get78;
  i32 _get77;
  i32 _get76;
  f64 _get75;
  f64 _get74;
  f64 _get73;
  i32 _get72;
  f64 _get71;
  i32 _get70;
  f64 _get69;
  i32 _get68;
  f64 _get67;
  i32 _get66;
  f64 _get65;
  i32 _get64;
  f64 _get63;
  i32 _get62;
  i32 _get61;
  i32 _get60;
  f64 _get59;
  i32 _get58;
  i32 _get57;
  f64 _get56;
  f64 _get55;
  i32 _get54;
  i32 _get53;
  i32 _get52;
  i32 _get51;
  i32 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  f64 _get41;
  i32 _get40;
  f64 _get39;
  i32 _get38;
  f64 _get37;
  i32 _get36;
  f64 _get35;
  i32 _get34;
  f64 _get33;
  i32 _get32;
  i32 _get31;
  f64 _get30;
  f64 _get29;
  i32 _get28;
  f64 _get27;
  i32 _get26;
  f64 _get25;
  i32 _get24;
  i32 _get23;
  f64 _get22;
  i32 _get21;
  f64 _get20;
  i32 _get19;
  i32 _get18;
  f64 _get17;
  i32 _get16;
  f64 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  i32 _get1;
  f64 _get0;
  f64 out = 0;
  i32 outjjtype = 0;
  f64 i = 0;
  i32 ijjtype = 0;
  f64 len = 0;
  i32 lenjjtype = 0;
  i32 jjlength_tmp = 0;
  i32 jjlast_type = 0;
  f64 jjmember_obj_153 = 0;
  f64 jjmember_prop_153 = 0;
  f64 cp = 0;
  i32 cpjjtype = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  f64 jjindirect_154_callee = 0;
  f64 jjindirect_154_caller = 0;
  i32 jjindirect_154_callerjjtype = 0;
  f64 jjmember_obj_155 = 0;
  f64 jjmember_prop_155 = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 logictmpi = 0;
  i32 jjlogicinner_tmp_int = 0;
  f64 next = 0;
  i32 nextjjtype = 0;
  f64 jjindirect_156_callee = 0;
  f64 jjindirect_156_caller = 0;
  i32 jjindirect_156_callerjjtype = 0;
  f64 jjmember_obj_157 = 0;
  f64 jjmember_prop_157 = 0;
  f64 jjbitwise_left = 0;
  f64 jjbitwise_right = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  out = 0;
  outjjtype = 195;
  i = 0;
  ijjtype = 1;
  _get0 = s;
  jjlength_tmp = (u32)(_get0);
  _get1 = sjjtype;
  // if f64
  f64 _r0;
    if ((_get1 & 64) != 0) {
      _get2 = jjlength_tmp;
      jjlast_type = 1;
      _r0 = (f64)(i32_load(1, 0, _get2));
    } else {
      jjmember_prop_153 = 564;
      _get3 = s;
      jjmember_obj_153 = _get3;
      _get4 = sjjtype;
      // if f64
      f64 _r1;
        if (_get4 == 0) {
          _r1 = 0;
        } else {
          _get5 = jjmember_obj_153;
          _get6 = sjjtype;
          _get7 = jjmember_prop_153;
          const struct ReturnValue _0 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get5), _get6, (u32)(_get7), 195, -2086110260, 1);
          jjlast_type = _0.type;
          _r1 = _0.value;
        }
      // end
      j1:;
      _r0 = _r1;
    }
  // end
  j0:;
  len = _r0;
  _get8 = jjlast_type;
  lenjjtype = _get8;
  // loop 
  j560:;
    _get9 = i;
    _get10 = len;
    // if 
      if (_get9 < _get10) {
        _get11 = s;
        jjproto_target = _get11;
        _get12 = sjjtype;
        jjproto_targetjjtype = _get12;
        _get13 = sjjtype;
        jjtypeswitch_tmp1 = _get13;
        // block f64
        f64 _r562;
          _get14 = jjtypeswitch_tmp1;
          // if 
            if (_get14 == 33) {
              _get15 = jjproto_target;
              _get16 = jjproto_targetjjtype;
              _get17 = i;
              _get18 = ijjtype;
              const struct ReturnValue _1 = dong_porf_porf_todo__String_prototype_charCodeAt(_get15, _get16, _get17, _get18);
              jjlast_type = _1.type;
              _r562 = _1.value;
              goto j562;
            }
          // end
          j563:;
          _get19 = jjtypeswitch_tmp1;
          // if 
            if (_get19 == 67) {
              _get20 = jjproto_target;
              _get21 = jjproto_targetjjtype;
              _get22 = i;
              _get23 = ijjtype;
              const struct ReturnValue _2 = dong_porf_porf_todo__String_prototype_charCodeAt(_get20, _get21, _get22, _get23);
              jjlast_type = _2.type;
              _r562 = _2.value;
              goto j562;
            }
          // end
          j564:;
          _get24 = jjtypeswitch_tmp1;
          // if 
            if (_get24 == 195) {
              _get25 = jjproto_target;
              _get26 = jjproto_targetjjtype;
              _get27 = i;
              _get28 = ijjtype;
              const struct ReturnValue _3 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get25, _get26, _get27, _get28);
              jjlast_type = _3.type;
              _r562 = _3.value;
              goto j562;
            }
          // end
          j565:;
          jjmember_prop_155 = 2136;
          _get29 = s;
          jjindirect_154_caller = _get29;
          _get30 = jjindirect_154_caller;
          _get31 = sjjtype;
          jjindirect_154_callerjjtype = _get31;
          jjmember_obj_155 = _get30;
          _get32 = jjindirect_154_callerjjtype;
          // if f64
          f64 _r566;
            if (_get32 == 0) {
              _r566 = 0;
            } else {
              _get33 = jjmember_obj_155;
              _get34 = jjindirect_154_callerjjtype;
              _get35 = jjmember_prop_155;
              const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get33), _get34, (u32)(_get35), 195, -1592872053, 1);
              jjlast_type = _4.type;
              _r566 = _4.value;
            }
          // end
          j566:;
          jjindirect_154_callee = _r566;
          _get36 = jjlast_type;
          // if f64
          f64 _r567;
            if (_get36 == 6) {
              _get37 = jjindirect_154_caller;
              _get38 = jjindirect_154_callerjjtype;
              _get39 = i;
              _get40 = ijjtype;
              _get41 = jjindirect_154_callee;
              jjlast_type = 0;
              _r567 = 0;
            } else {
              _r567 = 0;
            }
          // end
          j567:;
          _r562 = _r567;
        // end
        j562:;
        cp = _r562;
        _get42 = jjlast_type;
        cpjjtype = _get42;
        _get43 = cp;
        logictmpi = _get43 >= 55296;
        _get44 = logictmpi;
        // if i32
        i32 _r568;
          if (_get44 != 0) {
            _get45 = cp;
            jjlast_type = 2;
            _r568 = _get45 <= 56319;
          } else {
            _get46 = logictmpi;
            jjlast_type = 2;
            _r568 = _get46;
          }
        // end
        j568:;
        logictmpi = _r568;
        _get47 = logictmpi;
        jjlogicinner_tmp_int = _get47;
        _get48 = jjlast_type;
        jjtypeswitch_tmp1 = _get48;
        // block i32
        i32 _r569;
          _get49 = jjtypeswitch_tmp1;
          _get50 = jjtypeswitch_tmp1;
          // if 
            if (((_get49 == 67) | (_get50 == 195)) != 0) {
              _get51 = jjlogicinner_tmp_int;
              _r569 = i32_load(1, 0, _get51);
              goto j569;
            }
          // end
          j570:;
          _get52 = jjtypeswitch_tmp1;
          _get53 = jjtypeswitch_tmp1;
          // if 
            if (((_get52 == 31) | (_get53 == 32)) != 0) {
              _r569 = 1;
              goto j569;
            }
          // end
          j571:;
          _get54 = jjlogicinner_tmp_int;
          _r569 = _get54 != 0;
        // end
        j569:;
        // if i32
        i32 _r572;
          if ((_r569) != 0) {
            _get55 = i;
            _get56 = len;
            jjlast_type = 2;
            _r572 = (_get55 + 1) < _get56;
          } else {
            _get57 = logictmpi;
            _get58 = jjlast_type;
            jjlast_type = _get58;
            _r572 = _get57;
          }
        // end
        j572:;
        // if 
          if ((f64)(_r572) != 0) {
            _get59 = s;
            jjproto_target = _get59;
            _get60 = sjjtype;
            jjproto_targetjjtype = _get60;
            _get61 = sjjtype;
            jjtypeswitch_tmp1 = _get61;
            // block f64
            f64 _r574;
              _get62 = jjtypeswitch_tmp1;
              // if 
                if (_get62 == 33) {
                  _get63 = jjproto_target;
                  _get64 = jjproto_targetjjtype;
                  _get65 = i;
                  const struct ReturnValue _5 = dong_porf_porf_todo__String_prototype_charCodeAt(_get63, _get64, _get65 + 1, 1);
                  jjlast_type = _5.type;
                  _r574 = _5.value;
                  goto j574;
                }
              // end
              j575:;
              _get66 = jjtypeswitch_tmp1;
              // if 
                if (_get66 == 67) {
                  _get67 = jjproto_target;
                  _get68 = jjproto_targetjjtype;
                  _get69 = i;
                  const struct ReturnValue _6 = dong_porf_porf_todo__String_prototype_charCodeAt(_get67, _get68, _get69 + 1, 1);
                  jjlast_type = _6.type;
                  _r574 = _6.value;
                  goto j574;
                }
              // end
              j576:;
              _get70 = jjtypeswitch_tmp1;
              // if 
                if (_get70 == 195) {
                  _get71 = jjproto_target;
                  _get72 = jjproto_targetjjtype;
                  _get73 = i;
                  const struct ReturnValue _7 = dong_porf_porf_todo__ByteString_prototype_charCodeAt(_get71, _get72, _get73 + 1, 1);
                  jjlast_type = _7.type;
                  _r574 = _7.value;
                  goto j574;
                }
              // end
              j577:;
              jjmember_prop_157 = 2136;
              _get74 = s;
              jjindirect_156_caller = _get74;
              _get75 = jjindirect_156_caller;
              _get76 = sjjtype;
              jjindirect_156_callerjjtype = _get76;
              jjmember_obj_157 = _get75;
              _get77 = jjindirect_156_callerjjtype;
              // if f64
              f64 _r578;
                if (_get77 == 0) {
                  _r578 = 0;
                } else {
                  _get78 = jjmember_obj_157;
                  _get79 = jjindirect_156_callerjjtype;
                  _get80 = jjmember_prop_157;
                  const struct ReturnValue _8 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get78), _get79, (u32)(_get80), 195, -1592872053, 1);
                  jjlast_type = _8.type;
                  _r578 = _8.value;
                }
              // end
              j578:;
              jjindirect_156_callee = _r578;
              _get81 = jjlast_type;
              // if f64
              f64 _r579;
                if (_get81 == 6) {
                  _get82 = jjindirect_156_caller;
                  _get83 = jjindirect_156_callerjjtype;
                  _get84 = i;
                  _get85 = jjindirect_156_callee;
                  jjlast_type = 0;
                  _r579 = 0;
                } else {
                  _r579 = 0;
                }
              // end
              j579:;
              _r574 = _r579;
            // end
            j574:;
            next = _r574;
            _get86 = jjlast_type;
            nextjjtype = _get86;
            _get87 = next;
            logictmpi = _get87 >= 56320;
            _get88 = logictmpi;
            // if i32
            i32 _r580;
              if (_get88 != 0) {
                _get89 = next;
                jjlast_type = 2;
                _r580 = _get89 <= 57343;
              } else {
                _get90 = logictmpi;
                jjlast_type = 2;
                _r580 = _get90;
              }
            // end
            j580:;
            // if 
              if ((f64)(_r580) != 0) {
                _get91 = cp;
                jjbitwise_left = _get91 - 55296;
                _get92 = jjbitwise_left;
                _get93 = jjbitwise_left;
                // if i32
                i32 _r582;
                  if ((_get92 - _get93) == 0) {
                    _get94 = jjbitwise_left;
                    _r582 = (i32)((i64)(_get94));
                  } else {
                    _r582 = 0;
                  }
                // end
                j582:;
                jjbitwise_right = 10;
                _get95 = jjbitwise_right;
                _get96 = jjbitwise_right;
                // if i32
                i32 _r583;
                  if ((_get95 - _get96) == 0) {
                    _get97 = jjbitwise_right;
                    _r583 = (i32)((i64)(_get97));
                  } else {
                    _r583 = 0;
                  }
                // end
                j583:;
                _get98 = next;
                cp = (65536 + (f64)(_r582 << _r583)) + (_get98 - 56320);
                _get99 = cp;
                cpjjtype = 1;
                (void) _get99;
                _get100 = i;
                i = _get100 + 1;
                _get101 = i;
                ijjtype = 1;
                (void) _get101;
              }
            // end
            j581:;
          }
        // end
        j573:;
        _get102 = out;
        _get103 = outjjtype;
        _get104 = cp;
        _get105 = cpjjtype;
        const struct ReturnValue _9 = dong_porf_porf_todo_utf8AppendCodePoint(0, 0, 0, 0, _get102, _get103, _get104, _get105);
        jjlast_type = _9.type;
        _get106 = jjlast_type;
        outjjtype = _get106;
        out = _9.value;
        _get107 = i;
        i = _get107 + 1;
        _get108 = i;
        ijjtype = 1;
        (void) _get108;
        goto j560;
      }
    // end
    j561:;
  // end
  _get109 = out;
  jjreturn = _get109;
  jjreturnjjtype = 195;
  _get110 = jjnewtarget;
  // if 
    if (((u32)(_get110)) != 0) {
      _get111 = jjreturn;
      _get112 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get111), _get112)) == 0) {
          _get113 = jjthis;
          _get114 = jjthisjjtype;
          return (struct ReturnValue){ _get113, _get114 };
        }
      // end
      j652:;
    }
  // end
  j651:;
  _get115 = jjreturn;
  _get116 = jjreturnjjtype;
  return (struct ReturnValue){ _get115, _get116 };
}

static struct ReturnValue dong_porf_porf_todo_getElementById(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 id, i32 idjjtype) {
  i32 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  i32 _get1;
  f64 _get0;
  i32 jjlast_type = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = id;
  _get1 = idjjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get0, _get1);
  jjlast_type = _0.type;
  jjreturn = __porf_import_dong_dom_getElementById(_0.value);
  _get2 = jjlast_type;
  jjreturnjjtype = _get2;
  _get3 = jjnewtarget;
  // if 
    if (((u32)(_get3)) != 0) {
      _get4 = jjreturn;
      _get5 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get4), _get5)) == 0) {
          _get6 = jjthis;
          _get7 = jjthisjjtype;
          return (struct ReturnValue){ _get6, _get7 };
        }
      // end
      j654:;
    }
  // end
  j653:;
  _get8 = jjreturn;
  _get9 = jjreturnjjtype;
  return (struct ReturnValue){ _get8, _get9 };
}

static struct ReturnValue dong_porf_porf_todo_addEventListener(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 type, i32 typejjtype, f64 handlerName, i32 handlerNamejjtype) {
  i32 _get7;
  f64 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = nodeId;
  __porf_import_dong_stage_0(_get0);
  _get1 = type;
  _get2 = typejjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get1, _get2);
  (void) _0.type;
  __porf_import_dong_stage_1(_0.value);
  _get3 = handlerName;
  _get4 = handlerNamejjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get3, _get4);
  (void) _1.type;
  __porf_import_dong_stage_2(_1.value);
  __porf_import_dong_commit_addEventListener();
  _get5 = jjnewtarget;
  // if 
    if (((u32)(_get5)) != 0) {
      _get6 = jjthis;
      _get7 = jjthisjjtype;
      return (struct ReturnValue){ _get6, _get7 };
    }
  // end
  j655:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_todoDoneAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype) {
  i32 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  i32 _get137;
  f64 _get136;
  f64 _get135;
  i32 _get134;
  f64 _get133;
  i32 _get132;
  f64 _get131;
  i32 _get130;
  f64 _get129;
  f64 _get128;
  i32 _get127;
  f64 _get126;
  i32 _get125;
  f64 _get124;
  i32 _get123;
  f64 _get122;
  i32 _get121;
  f64 _get120;
  f64 _get119;
  i32 _get118;
  f64 _get117;
  i32 _get116;
  f64 _get115;
  i32 _get114;
  f64 _get113;
  i32 _get112;
  f64 _get111;
  f64 _get110;
  i32 _get109;
  f64 _get108;
  i32 _get107;
  f64 _get106;
  i32 _get105;
  f64 _get104;
  i32 _get103;
  f64 _get102;
  f64 _get101;
  i32 _get100;
  f64 _get99;
  i32 _get98;
  f64 _get97;
  i32 _get96;
  f64 _get95;
  i32 _get94;
  f64 _get93;
  f64 _get92;
  i32 _get91;
  f64 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  f64 _get86;
  i32 _get85;
  f64 _get84;
  f64 _get83;
  i32 _get82;
  f64 _get81;
  i32 _get80;
  f64 _get79;
  i32 _get78;
  f64 _get77;
  i32 _get76;
  f64 _get75;
  f64 _get74;
  i32 _get73;
  f64 _get72;
  i32 _get71;
  f64 _get70;
  i32 _get69;
  f64 _get68;
  i32 _get67;
  f64 _get66;
  f64 _get65;
  i32 _get64;
  f64 _get63;
  i32 _get62;
  f64 _get61;
  i32 _get60;
  f64 _get59;
  i32 _get58;
  f64 _get57;
  f64 _get56;
  i32 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  i32 _get51;
  f64 _get50;
  i32 _get49;
  f64 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  f64 _get41;
  i32 _get40;
  f64 _get39;
  f64 _get38;
  i32 _get37;
  f64 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  f64 _get32;
  i32 _get31;
  f64 _get30;
  f64 _get29;
  i32 _get28;
  f64 _get27;
  i32 _get26;
  f64 _get25;
  i32 _get24;
  f64 _get23;
  i32 _get22;
  f64 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  f64 _get12;
  f64 _get11;
  i32 _get10;
  f64 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = i;
  _get1 = ijjtype;
  // if 
    if ((f64)((_get0 == 0) & ((_get1 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone0;
      jjreturnjjtype = dong_porf_porf_todo_todoDone0jjtype;
      _get2 = jjnewtarget;
      // if 
        if (((u32)(_get2)) != 0) {
          _get3 = jjreturn;
          _get4 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get3), _get4)) == 0) {
              _get5 = jjthis;
              _get6 = jjthisjjtype;
              return (struct ReturnValue){ _get5, _get6 };
            }
          // end
          j660:;
        }
      // end
      j659:;
      _get7 = jjreturn;
      _get8 = jjreturnjjtype;
      return (struct ReturnValue){ _get7, _get8 };
    }
  // end
  j658:;
  _get9 = i;
  _get10 = ijjtype;
  // if 
    if ((f64)((_get9 == 1) & ((_get10 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone1;
      jjreturnjjtype = dong_porf_porf_todo_todoDone1jjtype;
      _get11 = jjnewtarget;
      // if 
        if (((u32)(_get11)) != 0) {
          _get12 = jjreturn;
          _get13 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get12), _get13)) == 0) {
              _get14 = jjthis;
              _get15 = jjthisjjtype;
              return (struct ReturnValue){ _get14, _get15 };
            }
          // end
          j663:;
        }
      // end
      j662:;
      _get16 = jjreturn;
      _get17 = jjreturnjjtype;
      return (struct ReturnValue){ _get16, _get17 };
    }
  // end
  j661:;
  _get18 = i;
  _get19 = ijjtype;
  // if 
    if ((f64)((_get18 == 2) & ((_get19 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone2;
      jjreturnjjtype = dong_porf_porf_todo_todoDone2jjtype;
      _get20 = jjnewtarget;
      // if 
        if (((u32)(_get20)) != 0) {
          _get21 = jjreturn;
          _get22 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get21), _get22)) == 0) {
              _get23 = jjthis;
              _get24 = jjthisjjtype;
              return (struct ReturnValue){ _get23, _get24 };
            }
          // end
          j666:;
        }
      // end
      j665:;
      _get25 = jjreturn;
      _get26 = jjreturnjjtype;
      return (struct ReturnValue){ _get25, _get26 };
    }
  // end
  j664:;
  _get27 = i;
  _get28 = ijjtype;
  // if 
    if ((f64)((_get27 == 3) & ((_get28 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone3;
      jjreturnjjtype = dong_porf_porf_todo_todoDone3jjtype;
      _get29 = jjnewtarget;
      // if 
        if (((u32)(_get29)) != 0) {
          _get30 = jjreturn;
          _get31 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get30), _get31)) == 0) {
              _get32 = jjthis;
              _get33 = jjthisjjtype;
              return (struct ReturnValue){ _get32, _get33 };
            }
          // end
          j669:;
        }
      // end
      j668:;
      _get34 = jjreturn;
      _get35 = jjreturnjjtype;
      return (struct ReturnValue){ _get34, _get35 };
    }
  // end
  j667:;
  _get36 = i;
  _get37 = ijjtype;
  // if 
    if ((f64)((_get36 == 4) & ((_get37 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone4;
      jjreturnjjtype = dong_porf_porf_todo_todoDone4jjtype;
      _get38 = jjnewtarget;
      // if 
        if (((u32)(_get38)) != 0) {
          _get39 = jjreturn;
          _get40 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get39), _get40)) == 0) {
              _get41 = jjthis;
              _get42 = jjthisjjtype;
              return (struct ReturnValue){ _get41, _get42 };
            }
          // end
          j672:;
        }
      // end
      j671:;
      _get43 = jjreturn;
      _get44 = jjreturnjjtype;
      return (struct ReturnValue){ _get43, _get44 };
    }
  // end
  j670:;
  _get45 = i;
  _get46 = ijjtype;
  // if 
    if ((f64)((_get45 == 5) & ((_get46 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone5;
      jjreturnjjtype = dong_porf_porf_todo_todoDone5jjtype;
      _get47 = jjnewtarget;
      // if 
        if (((u32)(_get47)) != 0) {
          _get48 = jjreturn;
          _get49 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get48), _get49)) == 0) {
              _get50 = jjthis;
              _get51 = jjthisjjtype;
              return (struct ReturnValue){ _get50, _get51 };
            }
          // end
          j675:;
        }
      // end
      j674:;
      _get52 = jjreturn;
      _get53 = jjreturnjjtype;
      return (struct ReturnValue){ _get52, _get53 };
    }
  // end
  j673:;
  _get54 = i;
  _get55 = ijjtype;
  // if 
    if ((f64)((_get54 == 6) & ((_get55 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone6;
      jjreturnjjtype = dong_porf_porf_todo_todoDone6jjtype;
      _get56 = jjnewtarget;
      // if 
        if (((u32)(_get56)) != 0) {
          _get57 = jjreturn;
          _get58 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get57), _get58)) == 0) {
              _get59 = jjthis;
              _get60 = jjthisjjtype;
              return (struct ReturnValue){ _get59, _get60 };
            }
          // end
          j678:;
        }
      // end
      j677:;
      _get61 = jjreturn;
      _get62 = jjreturnjjtype;
      return (struct ReturnValue){ _get61, _get62 };
    }
  // end
  j676:;
  _get63 = i;
  _get64 = ijjtype;
  // if 
    if ((f64)((_get63 == 7) & ((_get64 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone7;
      jjreturnjjtype = dong_porf_porf_todo_todoDone7jjtype;
      _get65 = jjnewtarget;
      // if 
        if (((u32)(_get65)) != 0) {
          _get66 = jjreturn;
          _get67 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get66), _get67)) == 0) {
              _get68 = jjthis;
              _get69 = jjthisjjtype;
              return (struct ReturnValue){ _get68, _get69 };
            }
          // end
          j681:;
        }
      // end
      j680:;
      _get70 = jjreturn;
      _get71 = jjreturnjjtype;
      return (struct ReturnValue){ _get70, _get71 };
    }
  // end
  j679:;
  _get72 = i;
  _get73 = ijjtype;
  // if 
    if ((f64)((_get72 == 8) & ((_get73 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone8;
      jjreturnjjtype = dong_porf_porf_todo_todoDone8jjtype;
      _get74 = jjnewtarget;
      // if 
        if (((u32)(_get74)) != 0) {
          _get75 = jjreturn;
          _get76 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get75), _get76)) == 0) {
              _get77 = jjthis;
              _get78 = jjthisjjtype;
              return (struct ReturnValue){ _get77, _get78 };
            }
          // end
          j684:;
        }
      // end
      j683:;
      _get79 = jjreturn;
      _get80 = jjreturnjjtype;
      return (struct ReturnValue){ _get79, _get80 };
    }
  // end
  j682:;
  _get81 = i;
  _get82 = ijjtype;
  // if 
    if ((f64)((_get81 == 9) & ((_get82 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone9;
      jjreturnjjtype = dong_porf_porf_todo_todoDone9jjtype;
      _get83 = jjnewtarget;
      // if 
        if (((u32)(_get83)) != 0) {
          _get84 = jjreturn;
          _get85 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get84), _get85)) == 0) {
              _get86 = jjthis;
              _get87 = jjthisjjtype;
              return (struct ReturnValue){ _get86, _get87 };
            }
          // end
          j687:;
        }
      // end
      j686:;
      _get88 = jjreturn;
      _get89 = jjreturnjjtype;
      return (struct ReturnValue){ _get88, _get89 };
    }
  // end
  j685:;
  _get90 = i;
  _get91 = ijjtype;
  // if 
    if ((f64)((_get90 == 10) & ((_get91 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone10;
      jjreturnjjtype = dong_porf_porf_todo_todoDone10jjtype;
      _get92 = jjnewtarget;
      // if 
        if (((u32)(_get92)) != 0) {
          _get93 = jjreturn;
          _get94 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get93), _get94)) == 0) {
              _get95 = jjthis;
              _get96 = jjthisjjtype;
              return (struct ReturnValue){ _get95, _get96 };
            }
          // end
          j690:;
        }
      // end
      j689:;
      _get97 = jjreturn;
      _get98 = jjreturnjjtype;
      return (struct ReturnValue){ _get97, _get98 };
    }
  // end
  j688:;
  _get99 = i;
  _get100 = ijjtype;
  // if 
    if ((f64)((_get99 == 11) & ((_get100 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone11;
      jjreturnjjtype = dong_porf_porf_todo_todoDone11jjtype;
      _get101 = jjnewtarget;
      // if 
        if (((u32)(_get101)) != 0) {
          _get102 = jjreturn;
          _get103 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get102), _get103)) == 0) {
              _get104 = jjthis;
              _get105 = jjthisjjtype;
              return (struct ReturnValue){ _get104, _get105 };
            }
          // end
          j693:;
        }
      // end
      j692:;
      _get106 = jjreturn;
      _get107 = jjreturnjjtype;
      return (struct ReturnValue){ _get106, _get107 };
    }
  // end
  j691:;
  _get108 = i;
  _get109 = ijjtype;
  // if 
    if ((f64)((_get108 == 12) & ((_get109 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone12;
      jjreturnjjtype = dong_porf_porf_todo_todoDone12jjtype;
      _get110 = jjnewtarget;
      // if 
        if (((u32)(_get110)) != 0) {
          _get111 = jjreturn;
          _get112 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get111), _get112)) == 0) {
              _get113 = jjthis;
              _get114 = jjthisjjtype;
              return (struct ReturnValue){ _get113, _get114 };
            }
          // end
          j696:;
        }
      // end
      j695:;
      _get115 = jjreturn;
      _get116 = jjreturnjjtype;
      return (struct ReturnValue){ _get115, _get116 };
    }
  // end
  j694:;
  _get117 = i;
  _get118 = ijjtype;
  // if 
    if ((f64)((_get117 == 13) & ((_get118 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone13;
      jjreturnjjtype = dong_porf_porf_todo_todoDone13jjtype;
      _get119 = jjnewtarget;
      // if 
        if (((u32)(_get119)) != 0) {
          _get120 = jjreturn;
          _get121 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get120), _get121)) == 0) {
              _get122 = jjthis;
              _get123 = jjthisjjtype;
              return (struct ReturnValue){ _get122, _get123 };
            }
          // end
          j699:;
        }
      // end
      j698:;
      _get124 = jjreturn;
      _get125 = jjreturnjjtype;
      return (struct ReturnValue){ _get124, _get125 };
    }
  // end
  j697:;
  _get126 = i;
  _get127 = ijjtype;
  // if 
    if ((f64)((_get126 == 14) & ((_get127 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone14;
      jjreturnjjtype = dong_porf_porf_todo_todoDone14jjtype;
      _get128 = jjnewtarget;
      // if 
        if (((u32)(_get128)) != 0) {
          _get129 = jjreturn;
          _get130 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get129), _get130)) == 0) {
              _get131 = jjthis;
              _get132 = jjthisjjtype;
              return (struct ReturnValue){ _get131, _get132 };
            }
          // end
          j702:;
        }
      // end
      j701:;
      _get133 = jjreturn;
      _get134 = jjreturnjjtype;
      return (struct ReturnValue){ _get133, _get134 };
    }
  // end
  j700:;
  jjreturn = dong_porf_porf_todo_todoDone15;
  jjreturnjjtype = dong_porf_porf_todo_todoDone15jjtype;
  _get135 = jjnewtarget;
  // if 
    if (((u32)(_get135)) != 0) {
      _get136 = jjreturn;
      _get137 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get136), _get137)) == 0) {
          _get138 = jjthis;
          _get139 = jjthisjjtype;
          return (struct ReturnValue){ _get138, _get139 };
        }
      // end
      j704:;
    }
  // end
  j703:;
  _get140 = jjreturn;
  _get141 = jjreturnjjtype;
  return (struct ReturnValue){ _get140, _get141 };
}

static struct ReturnValue dong_porf_porf_todo_countActive(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get71;
  f64 _get70;
  i32 _get69;
  f64 _get68;
  i32 _get67;
  f64 _get66;
  f64 _get65;
  f64 _get64;
  f64 _get63;
  f64 _get62;
  f64 _get61;
  f64 _get60;
  f64 _get59;
  f64 _get58;
  f64 _get57;
  i32 _get56;
  i32 _get55;
  f64 _get54;
  i32 _get53;
  i32 _get52;
  i32 _get51;
  i32 _get50;
  f64 _get49;
  f64 _get48;
  f64 _get47;
  f64 _get46;
  f64 _get45;
  f64 _get44;
  f64 _get43;
  f64 _get42;
  f64 _get41;
  i32 _get40;
  i32 _get39;
  f64 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  f64 _get33;
  f64 _get32;
  f64 _get31;
  f64 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  f64 _get26;
  f64 _get25;
  i32 _get24;
  i32 _get23;
  f64 _get22;
  i32 _get21;
  i32 _get20;
  i32 _get19;
  i32 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  i32 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  f64 n = 0;
  i32 njjtype = 0;
  f64 d = 0;
  i32 djjtype = 0;
  f64 i = 0;
  i32 ijjtype = 0;
  i32 jjlast_type = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  n = 0;
  njjtype = 1;
  d = 0;
  djjtype = 1;
  i = 0;
  ijjtype = 1;
  // loop 
  j656:;
    _get0 = i;
    // if 
      if (_get0 < dong_porf_porf_todo_todoCount) {
        _get1 = i;
        _get2 = ijjtype;
        const struct ReturnValue _0 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get1, _get2);
        jjlast_type = _0.type;
        jjlogicinner_tmp = _0.value;
        _get3 = jjlast_type;
        jjtypeswitch_tmp1 = _get3;
        // block i32
        i32 _r705;
          _get4 = jjtypeswitch_tmp1;
          _get5 = jjtypeswitch_tmp1;
          // if 
            if (((_get4 == 67) | (_get5 == 195)) != 0) {
              _get6 = jjlogicinner_tmp;
              _r705 = i32_load(1, 0, (u32)(_get6));
              goto j705;
            }
          // end
          j706:;
          _get7 = jjtypeswitch_tmp1;
          _get8 = jjtypeswitch_tmp1;
          // if 
            if (((_get7 == 31) | (_get8 == 32)) != 0) {
              _r705 = 1;
              goto j705;
            }
          // end
          j707:;
          _get9 = jjlogicinner_tmp;
          const f64 _tmp0 = _get9;
          _r705 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
        // end
        j705:;
        // if 
          if ((_r705) != 0) {
            _get10 = d;
            d = _get10 + 1;
            _get11 = d;
            djjtype = 1;
            (void) _get11;
          } else {
            _get12 = n;
            n = _get12 + 1;
            _get13 = n;
            njjtype = 1;
            (void) _get13;
          }
        // end
        j708:;
        _get14 = i;
        i = _get14 + 1;
        _get15 = i;
        ijjtype = 1;
        (void) _get15;
        _get16 = i;
        if (!(_get16 < dong_porf_porf_todo_todoCount)) {
          goto j657;
        }
        _get17 = i;
        _get18 = ijjtype;
        const struct ReturnValue _1 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get17, _get18);
        jjlast_type = _1.type;
        jjlogicinner_tmp = _1.value;
        _get19 = jjlast_type;
        jjtypeswitch_tmp1 = _get19;
        // block i32
        i32 _r709;
          _get20 = jjtypeswitch_tmp1;
          _get21 = jjtypeswitch_tmp1;
          // if 
            if (((_get20 == 67) | (_get21 == 195)) != 0) {
              _get22 = jjlogicinner_tmp;
              _r709 = i32_load(1, 0, (u32)(_get22));
              goto j709;
            }
          // end
          j710:;
          _get23 = jjtypeswitch_tmp1;
          _get24 = jjtypeswitch_tmp1;
          // if 
            if (((_get23 == 31) | (_get24 == 32)) != 0) {
              _r709 = 1;
              goto j709;
            }
          // end
          j711:;
          _get25 = jjlogicinner_tmp;
          const f64 _tmp1 = _get25;
          _r709 = (_tmp1 < 0 ? -_tmp1 : _tmp1) > 0;
        // end
        j709:;
        // if 
          if ((_r709) != 0) {
            _get26 = d;
            d = _get26 + 1;
            _get27 = d;
            djjtype = 1;
            (void) _get27;
          } else {
            _get28 = n;
            n = _get28 + 1;
            _get29 = n;
            njjtype = 1;
            (void) _get29;
          }
        // end
        j712:;
        _get30 = i;
        i = _get30 + 1;
        _get31 = i;
        ijjtype = 1;
        (void) _get31;
        _get32 = i;
        if (!(_get32 < dong_porf_porf_todo_todoCount)) {
          goto j657;
        }
        _get33 = i;
        _get34 = ijjtype;
        const struct ReturnValue _2 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get33, _get34);
        jjlast_type = _2.type;
        jjlogicinner_tmp = _2.value;
        _get35 = jjlast_type;
        jjtypeswitch_tmp1 = _get35;
        // block i32
        i32 _r713;
          _get36 = jjtypeswitch_tmp1;
          _get37 = jjtypeswitch_tmp1;
          // if 
            if (((_get36 == 67) | (_get37 == 195)) != 0) {
              _get38 = jjlogicinner_tmp;
              _r713 = i32_load(1, 0, (u32)(_get38));
              goto j713;
            }
          // end
          j714:;
          _get39 = jjtypeswitch_tmp1;
          _get40 = jjtypeswitch_tmp1;
          // if 
            if (((_get39 == 31) | (_get40 == 32)) != 0) {
              _r713 = 1;
              goto j713;
            }
          // end
          j715:;
          _get41 = jjlogicinner_tmp;
          const f64 _tmp2 = _get41;
          _r713 = (_tmp2 < 0 ? -_tmp2 : _tmp2) > 0;
        // end
        j713:;
        // if 
          if ((_r713) != 0) {
            _get42 = d;
            d = _get42 + 1;
            _get43 = d;
            djjtype = 1;
            (void) _get43;
          } else {
            _get44 = n;
            n = _get44 + 1;
            _get45 = n;
            njjtype = 1;
            (void) _get45;
          }
        // end
        j716:;
        _get46 = i;
        i = _get46 + 1;
        _get47 = i;
        ijjtype = 1;
        (void) _get47;
        _get48 = i;
        if (!(_get48 < dong_porf_porf_todo_todoCount)) {
          goto j657;
        }
        _get49 = i;
        _get50 = ijjtype;
        const struct ReturnValue _3 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get49, _get50);
        jjlast_type = _3.type;
        jjlogicinner_tmp = _3.value;
        _get51 = jjlast_type;
        jjtypeswitch_tmp1 = _get51;
        // block i32
        i32 _r717;
          _get52 = jjtypeswitch_tmp1;
          _get53 = jjtypeswitch_tmp1;
          // if 
            if (((_get52 == 67) | (_get53 == 195)) != 0) {
              _get54 = jjlogicinner_tmp;
              _r717 = i32_load(1, 0, (u32)(_get54));
              goto j717;
            }
          // end
          j718:;
          _get55 = jjtypeswitch_tmp1;
          _get56 = jjtypeswitch_tmp1;
          // if 
            if (((_get55 == 31) | (_get56 == 32)) != 0) {
              _r717 = 1;
              goto j717;
            }
          // end
          j719:;
          _get57 = jjlogicinner_tmp;
          const f64 _tmp3 = _get57;
          _r717 = (_tmp3 < 0 ? -_tmp3 : _tmp3) > 0;
        // end
        j717:;
        // if 
          if ((_r717) != 0) {
            _get58 = d;
            d = _get58 + 1;
            _get59 = d;
            djjtype = 1;
            (void) _get59;
          } else {
            _get60 = n;
            n = _get60 + 1;
            _get61 = n;
            njjtype = 1;
            (void) _get61;
          }
        // end
        j720:;
        _get62 = i;
        i = _get62 + 1;
        _get63 = i;
        ijjtype = 1;
        (void) _get63;
        goto j656;
      }
    // end
    j657:;
  // end
  _get64 = n;
  jjreturn = _get64;
  jjreturnjjtype = 1;
  _get65 = jjnewtarget;
  // if 
    if (((u32)(_get65)) != 0) {
      _get66 = jjreturn;
      _get67 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get66), _get67)) == 0) {
          _get68 = jjthis;
          _get69 = jjthisjjtype;
          return (struct ReturnValue){ _get68, _get69 };
        }
      // end
      j722:;
    }
  // end
  j721:;
  _get70 = jjreturn;
  _get71 = jjreturnjjtype;
  return (struct ReturnValue){ _get70, _get71 };
}

static struct ReturnValue dong_porf_porf_todo_countDone(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get63;
  f64 _get62;
  i32 _get61;
  f64 _get60;
  i32 _get59;
  f64 _get58;
  f64 _get57;
  f64 _get56;
  f64 _get55;
  f64 _get54;
  f64 _get53;
  f64 _get52;
  f64 _get51;
  i32 _get50;
  i32 _get49;
  f64 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  f64 _get43;
  f64 _get42;
  f64 _get41;
  f64 _get40;
  f64 _get39;
  f64 _get38;
  f64 _get37;
  i32 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  f64 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  i32 _get22;
  i32 _get21;
  f64 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  i32 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  f64 d = 0;
  i32 djjtype = 0;
  f64 i = 0;
  i32 ijjtype = 0;
  i32 jjlast_type = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  d = 0;
  djjtype = 1;
  i = 0;
  ijjtype = 1;
  // loop 
  j723:;
    _get0 = i;
    // if 
      if (_get0 < dong_porf_porf_todo_todoCount) {
        _get1 = i;
        _get2 = ijjtype;
        const struct ReturnValue _0 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get1, _get2);
        jjlast_type = _0.type;
        jjlogicinner_tmp = _0.value;
        _get3 = jjlast_type;
        jjtypeswitch_tmp1 = _get3;
        // block i32
        i32 _r725;
          _get4 = jjtypeswitch_tmp1;
          _get5 = jjtypeswitch_tmp1;
          // if 
            if (((_get4 == 67) | (_get5 == 195)) != 0) {
              _get6 = jjlogicinner_tmp;
              _r725 = i32_load(1, 0, (u32)(_get6));
              goto j725;
            }
          // end
          j726:;
          _get7 = jjtypeswitch_tmp1;
          _get8 = jjtypeswitch_tmp1;
          // if 
            if (((_get7 == 31) | (_get8 == 32)) != 0) {
              _r725 = 1;
              goto j725;
            }
          // end
          j727:;
          _get9 = jjlogicinner_tmp;
          const f64 _tmp0 = _get9;
          _r725 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
        // end
        j725:;
        // if 
          if ((_r725) != 0) {
            _get10 = d;
            d = _get10 + 1;
            _get11 = d;
            djjtype = 1;
            (void) _get11;
          }
        // end
        j728:;
        _get12 = i;
        i = _get12 + 1;
        _get13 = i;
        ijjtype = 1;
        (void) _get13;
        _get14 = i;
        if (!(_get14 < dong_porf_porf_todo_todoCount)) {
          goto j724;
        }
        _get15 = i;
        _get16 = ijjtype;
        const struct ReturnValue _1 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get15, _get16);
        jjlast_type = _1.type;
        jjlogicinner_tmp = _1.value;
        _get17 = jjlast_type;
        jjtypeswitch_tmp1 = _get17;
        // block i32
        i32 _r729;
          _get18 = jjtypeswitch_tmp1;
          _get19 = jjtypeswitch_tmp1;
          // if 
            if (((_get18 == 67) | (_get19 == 195)) != 0) {
              _get20 = jjlogicinner_tmp;
              _r729 = i32_load(1, 0, (u32)(_get20));
              goto j729;
            }
          // end
          j730:;
          _get21 = jjtypeswitch_tmp1;
          _get22 = jjtypeswitch_tmp1;
          // if 
            if (((_get21 == 31) | (_get22 == 32)) != 0) {
              _r729 = 1;
              goto j729;
            }
          // end
          j731:;
          _get23 = jjlogicinner_tmp;
          const f64 _tmp1 = _get23;
          _r729 = (_tmp1 < 0 ? -_tmp1 : _tmp1) > 0;
        // end
        j729:;
        // if 
          if ((_r729) != 0) {
            _get24 = d;
            d = _get24 + 1;
            _get25 = d;
            djjtype = 1;
            (void) _get25;
          }
        // end
        j732:;
        _get26 = i;
        i = _get26 + 1;
        _get27 = i;
        ijjtype = 1;
        (void) _get27;
        _get28 = i;
        if (!(_get28 < dong_porf_porf_todo_todoCount)) {
          goto j724;
        }
        _get29 = i;
        _get30 = ijjtype;
        const struct ReturnValue _2 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get29, _get30);
        jjlast_type = _2.type;
        jjlogicinner_tmp = _2.value;
        _get31 = jjlast_type;
        jjtypeswitch_tmp1 = _get31;
        // block i32
        i32 _r733;
          _get32 = jjtypeswitch_tmp1;
          _get33 = jjtypeswitch_tmp1;
          // if 
            if (((_get32 == 67) | (_get33 == 195)) != 0) {
              _get34 = jjlogicinner_tmp;
              _r733 = i32_load(1, 0, (u32)(_get34));
              goto j733;
            }
          // end
          j734:;
          _get35 = jjtypeswitch_tmp1;
          _get36 = jjtypeswitch_tmp1;
          // if 
            if (((_get35 == 31) | (_get36 == 32)) != 0) {
              _r733 = 1;
              goto j733;
            }
          // end
          j735:;
          _get37 = jjlogicinner_tmp;
          const f64 _tmp2 = _get37;
          _r733 = (_tmp2 < 0 ? -_tmp2 : _tmp2) > 0;
        // end
        j733:;
        // if 
          if ((_r733) != 0) {
            _get38 = d;
            d = _get38 + 1;
            _get39 = d;
            djjtype = 1;
            (void) _get39;
          }
        // end
        j736:;
        _get40 = i;
        i = _get40 + 1;
        _get41 = i;
        ijjtype = 1;
        (void) _get41;
        _get42 = i;
        if (!(_get42 < dong_porf_porf_todo_todoCount)) {
          goto j724;
        }
        _get43 = i;
        _get44 = ijjtype;
        const struct ReturnValue _3 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get43, _get44);
        jjlast_type = _3.type;
        jjlogicinner_tmp = _3.value;
        _get45 = jjlast_type;
        jjtypeswitch_tmp1 = _get45;
        // block i32
        i32 _r737;
          _get46 = jjtypeswitch_tmp1;
          _get47 = jjtypeswitch_tmp1;
          // if 
            if (((_get46 == 67) | (_get47 == 195)) != 0) {
              _get48 = jjlogicinner_tmp;
              _r737 = i32_load(1, 0, (u32)(_get48));
              goto j737;
            }
          // end
          j738:;
          _get49 = jjtypeswitch_tmp1;
          _get50 = jjtypeswitch_tmp1;
          // if 
            if (((_get49 == 31) | (_get50 == 32)) != 0) {
              _r737 = 1;
              goto j737;
            }
          // end
          j739:;
          _get51 = jjlogicinner_tmp;
          const f64 _tmp3 = _get51;
          _r737 = (_tmp3 < 0 ? -_tmp3 : _tmp3) > 0;
        // end
        j737:;
        // if 
          if ((_r737) != 0) {
            _get52 = d;
            d = _get52 + 1;
            _get53 = d;
            djjtype = 1;
            (void) _get53;
          }
        // end
        j740:;
        _get54 = i;
        i = _get54 + 1;
        _get55 = i;
        ijjtype = 1;
        (void) _get55;
        goto j723;
      }
    // end
    j724:;
  // end
  _get56 = d;
  jjreturn = _get56;
  jjreturnjjtype = 1;
  _get57 = jjnewtarget;
  // if 
    if (((u32)(_get57)) != 0) {
      _get58 = jjreturn;
      _get59 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get58), _get59)) == 0) {
          _get60 = jjthis;
          _get61 = jjthisjjtype;
          return (struct ReturnValue){ _get60, _get61 };
        }
      // end
      j742:;
    }
  // end
  j741:;
  _get62 = jjreturn;
  _get63 = jjreturnjjtype;
  return (struct ReturnValue){ _get62, _get63 };
}

static struct ReturnValue dong_porf_porf_todo__Symbol_prototype_descriptionkkget(f64 _this, i32 _thisjjtype) {
  f64 _get2;
  f64 _get1;
  i32 _get0;
  _get0 = _thisjjtype;
  // if 
    if (_get0 != 5) {
    }
  // end
  j751:;
  _get1 = _this;
  _get2 = _this;
  return (struct ReturnValue){ f64_load(0, 0, (u32)(_get1)), i32_load8_u(0, 8, (u32)(_get2)) };
}

static struct ReturnValue dong_porf_porf_todo__Symbol_prototype_toString(f64 _this, i32 _thisjjtype) {
  f64 _get27;
  f64 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  f64 _get22;
  f64 _get21;
  f64 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  i32 _get0;
  f64 out = 0;
  f64 description = 0;
  i32 descriptionjjtype = 0;
  f64 jjmember_obj_433 = 0;
  f64 jjmember_prop_433 = 0;
  i32 jjlast_type = 0;
  f64 descLen = 0;
  f64 outPtr = 0;
  f64 descPtr = 0;
  f64 descPtrEnd = 0;

  _get0 = _thisjjtype;
  // if 
    if (_get0 != 5) {
    }
  // end
  j750:;
  out = (f64)(dong_porf_porf_todo__Porffor_malloc(16384));
  _get1 = out;
  i32_store8(0, 4, (i32)(_get1), 83);
  _get2 = out;
  i32_store8(0, 5, (i32)(_get2), 121);
  _get3 = out;
  i32_store8(0, 6, (i32)(_get3), 109);
  _get4 = out;
  i32_store8(0, 7, (i32)(_get4), 98);
  _get5 = out;
  i32_store8(0, 8, (i32)(_get5), 111);
  _get6 = out;
  i32_store8(0, 9, (i32)(_get6), 108);
  _get7 = out;
  i32_store8(0, 10, (i32)(_get7), 40);
  _get8 = _this;
  const struct ReturnValue _0 = dong_porf_porf_todo__Symbol_prototype_descriptionkkget(_get8, 5);
  jjlast_type = _0.type;
  _get9 = jjlast_type;
  descriptionjjtype = _get9;
  description = _0.value;
  descLen = 0;
  _get10 = description;
  _get11 = descriptionjjtype;
  // if 
    if (((_get10 != 0) | ((_get11 | 128) != (0 | 128))) != 0) {
      _get12 = description;
      descLen = (f64)(i32_load(1, 0, (u32)(_get12)));
      _get13 = out;
      outPtr = _get13 + 7;
      _get14 = description;
      descPtr = _get14;
      _get15 = descPtr;
      _get16 = descLen;
      descPtrEnd = _get15 + _get16;
      // loop 
      j753:;
        _get17 = descPtr;
        _get18 = descPtrEnd;
        // if 
          if (_get17 < _get18) {
            _get19 = outPtr;
            _get20 = outPtr;
            outPtr = _get20 + 1;
            _get21 = descPtr;
            _get22 = descPtr;
            descPtr = _get22 + 1;
            i32_store8(0, 4, (i32)(_get19), i32_load8_u(0, 4, (i32)(_get21)));
            goto j753;
          }
        // end
        j754:;
      // end
    }
  // end
  j752:;
  _get23 = out;
  _get24 = descLen;
  i32_store8(0, 11, (i32)((_get23 + _get24)), 41);
  _get25 = out;
  _get26 = descLen;
  i32_store(1, 0, (u32)(_get25), (u32)((8 + _get26)));
  _get27 = out;
  return (struct ReturnValue){ _get27, 195 };
}

static struct ReturnValue dong_porf_porf_todo_String(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 args, i32 argsjjtype) {
  f64 _get37;
  f64 _get36;
  i32 _get35;
  i32 _get34;
  f64 _get33;
  f64 _get32;
  f64 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  f64 _get27;
  i32 _get26;
  i32 _get25;
  f64 _get24;
  i32 _get23;
  i32 _get22;
  f64 _get21;
  f64 _get20;
  f64 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  f64 _get15;
  i32 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  i32 _get10;
  i32 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 s = 0;
  i32 sjjtype = 0;
  f64 value = 0;
  i32 valuejjtype = 0;
  f64 jjmember_obj_425 = 0;
  f64 jjmember_prop_425 = 0;
  i32 jjlast_type = 0;
  i32 jjloadArray_offset = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 logictmp = 0;

  s = 0;
  sjjtype = 195;
  _get0 = args;
  // if 
    if ((f64)(i32_load(1, 0, (u32)(_get0))) > 0) {
      jjmember_prop_425 = 0;
      _get1 = args;
      jjmember_obj_425 = _get1;
      _get2 = jjmember_prop_425;
      _get3 = jjmember_obj_425;
      jjloadArray_offset = ((u32)(_get2) * 9) + (u32)(_get3);
      _get4 = jjloadArray_offset;
      _get5 = jjloadArray_offset;
      jjlast_type = i32_load8_u(0, 12, _get5);
      _get6 = jjlast_type;
      valuejjtype = _get6;
      value = f64_load(0, 4, _get4);
      _get7 = jjnewtarget;
      jjlogicinner_tmp = _get7;
      _get8 = jjnewtargetjjtype;
      jjtypeswitch_tmp1 = _get8;
      // block f64
      f64 _r744;
        _get9 = jjtypeswitch_tmp1;
        _get10 = jjtypeswitch_tmp1;
        // if 
          if (((_get9 == 67) | (_get10 == 195)) != 0) {
            _get11 = jjlogicinner_tmp;
            _r744 = (f64)((i32_load(1, 0, (u32)(_get11))) == 0);
            goto j744;
          }
        // end
        j745:;
        _get12 = jjlogicinner_tmp;
        _r744 = (f64)(_get12 == 0);
      // end
      j744:;
      logictmp = _r744;
      _get13 = logictmp;
      // if f64
      f64 _r746;
        if (((u32)(_get13)) != 0) {
          _get14 = valuejjtype;
          jjlast_type = 2;
          _r746 = (f64)((f64)(_get14) == 5);
        } else {
          _get15 = logictmp;
          jjlast_type = 2;
          _r746 = _get15;
        }
      // end
      j746:;
      jjlogicinner_tmp = _r746;
      _get16 = jjlast_type;
      jjtypeswitch_tmp1 = _get16;
      // block i32
      i32 _r747;
        _get17 = jjtypeswitch_tmp1;
        _get18 = jjtypeswitch_tmp1;
        // if 
          if (((_get17 == 67) | (_get18 == 195)) != 0) {
            _get19 = jjlogicinner_tmp;
            _r747 = i32_load(1, 0, (u32)(_get19));
            goto j747;
          }
        // end
        j748:;
        _get20 = jjlogicinner_tmp;
        _r747 = (u32)(_get20);
      // end
      j747:;
      // if 
        if ((_r747) != 0) {
          _get21 = value;
          _get22 = valuejjtype;
          const struct ReturnValue _0 = dong_porf_porf_todo__Symbol_prototype_toString(_get21, _get22);
          jjlast_type = _0.type;
          _get23 = jjlast_type;
          return (struct ReturnValue){ _0.value, _get23 };
        }
      // end
      j749:;
      _get24 = value;
      _get25 = valuejjtype;
      const struct ReturnValue _1 = dong_porf_porf_todo__ecma262_ToString(_get24, _get25);
      jjlast_type = _1.type;
      _get26 = jjlast_type;
      sjjtype = _get26;
      s = _1.value;
    }
  // end
  j743:;
  _get27 = jjnewtarget;
  jjlogicinner_tmp = _get27;
  _get28 = jjnewtargetjjtype;
  jjtypeswitch_tmp1 = _get28;
  // block f64
  f64 _r755;
    _get29 = jjtypeswitch_tmp1;
    _get30 = jjtypeswitch_tmp1;
    // if 
      if (((_get29 == 67) | (_get30 == 195)) != 0) {
        _get31 = jjlogicinner_tmp;
        _r755 = (f64)((i32_load(1, 0, (u32)(_get31))) == 0);
        goto j755;
      }
    // end
    j756:;
    _get32 = jjlogicinner_tmp;
    _r755 = (f64)(_get32 == 0);
  // end
  j755:;
  // if 
    if (((u32)(_r755)) != 0) {
      _get33 = s;
      _get34 = sjjtype;
      return (struct ReturnValue){ _get33, _get34 };
    }
  // end
  j757:;
  _get35 = sjjtype;
  // if 
    if ((f64)(_get35) == 195) {
      _get36 = s;
      s = (f64)(dong_porf_porf_todo__Porffor_bytestringToString((i32)(_get36)));
      sjjtype = 67;
    }
  // end
  j758:;
  _get37 = s;
  return (struct ReturnValue){ _get37, 33 };
}

static struct ReturnValue dong_porf_porf_todo_setTextContent(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 text, i32 textjjtype) {
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = nodeId;
  __porf_import_dong_stage_0(_get0);
  _get1 = text;
  _get2 = textjjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get1, _get2);
  (void) _0.type;
  __porf_import_dong_stage_1(_0.value);
  __porf_import_dong_commit_set_textContent();
  _get3 = jjnewtarget;
  // if 
    if (((u32)(_get3)) != 0) {
      _get4 = jjthis;
      _get5 = jjthisjjtype;
      return (struct ReturnValue){ _get4, _get5 };
    }
  // end
  j759:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_setStyle(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 prop, i32 propjjtype, f64 value, i32 valuejjtype) {
  i32 _get7;
  f64 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = nodeId;
  _get1 = prop;
  _get2 = propjjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get1, _get2);
  (void) _0.type;
  _get3 = value;
  _get4 = valuejjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get3, _get4);
  (void) _1.type;
  __porf_import_dong_style_set(_get0, _0.value, _1.value);
  _get5 = jjnewtarget;
  // if 
    if (((u32)(_get5)) != 0) {
      _get6 = jjthis;
      _get7 = jjthisjjtype;
      return (struct ReturnValue){ _get6, _get7 };
    }
  // end
  j766:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_removeAttribute(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 name, i32 namejjtype) {
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = nodeId;
  _get1 = name;
  _get2 = namejjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get1, _get2);
  (void) _0.type;
  __porf_import_dong_remove_attribute(_get0, _0.value);
  _get3 = jjnewtarget;
  // if 
    if (((u32)(_get3)) != 0) {
      _get4 = jjthis;
      _get5 = jjthisjjtype;
      return (struct ReturnValue){ _get4, _get5 };
    }
  // end
  j768:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_setAttribute(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 name, i32 namejjtype, f64 value, i32 valuejjtype) {
  i32 _get7;
  f64 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = nodeId;
  _get1 = name;
  _get2 = namejjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get1, _get2);
  (void) _0.type;
  _get3 = value;
  _get4 = valuejjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get3, _get4);
  (void) _1.type;
  __porf_import_dong_set_attribute(_get0, _0.value, _1.value);
  _get5 = jjnewtarget;
  // if 
    if (((u32)(_get5)) != 0) {
      _get6 = jjthis;
      _get7 = jjthisjjtype;
      return (struct ReturnValue){ _get6, _get7 };
    }
  // end
  j769:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_porfPatchFilters(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get39;
  f64 _get38;
  f64 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  f64 _get33;
  f64 _get32;
  i32 _get31;
  f64 _get30;
  i32 _get29;
  f64 _get28;
  i32 _get27;
  f64 _get26;
  i32 _get25;
  f64 _get24;
  i32 _get23;
  f64 _get22;
  i32 _get21;
  f64 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  f64 _get2;
  i32 _get1;
  i32 _get0;
  f64 total = 0;
  i32 totaljjtype = 0;
  f64 active = 0;
  i32 activejjtype = 0;
  i32 jjlast_type = 0;
  f64 done = 0;
  i32 donejjtype = 0;
  f64 allBg = 0;
  i32 allBgjjtype = 0;
  f64 allColor = 0;
  i32 allColorjjtype = 0;
  f64 actBg = 0;
  i32 actBgjjtype = 0;
  f64 actColor = 0;
  i32 actColorjjtype = 0;
  f64 doneBg = 0;
  i32 doneBgjjtype = 0;
  f64 doneColor = 0;
  i32 doneColorjjtype = 0;

  total = dong_porf_porf_todo_todoCount;
  totaljjtype = dong_porf_porf_todo_todoCountjjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_countActive(0, 0, 0, 0);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  activejjtype = _get0;
  active = _0.value;
  const struct ReturnValue _1 = dong_porf_porf_todo_countDone(0, 0, 0, 0);
  jjlast_type = _1.type;
  _get1 = jjlast_type;
  donejjtype = _get1;
  done = _1.value;
  _get2 = total;
  f64_store(0, 4, 212992, _get2);
  _get3 = totaljjtype;
  i32_store8(0, 12, 212992, _get3);
  i32_store(1, 0, 212992, 1);
  const struct ReturnValue _2 = dong_porf_porf_todo_String(0, 0, 0, 0, 212992, 72);
  jjlast_type = _2.type;
  _get4 = jjlast_type;
  const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_concatStrings(2207, 195, _2.value, _get4);
  jjlast_type = _3.type;
  _get5 = jjlast_type;
  const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_concatStrings(_3.value, _get5, 2218, 195);
  jjlast_type = _4.type;
  const struct ReturnValue _5 = dong_porf_porf_todo_setTextContent(0, 0, 0, 0, dong_porf_porf_todo_filterAllId, 1, _4.value, 195);
  jjlast_type = _5.type;
  (void) _5.value;
  _get6 = active;
  f64_store(0, 4, 229376, _get6);
  _get7 = activejjtype;
  i32_store8(0, 12, 229376, _get7);
  i32_store(1, 0, 229376, 1);
  const struct ReturnValue _6 = dong_porf_porf_todo_String(0, 0, 0, 0, 229376, 72);
  jjlast_type = _6.type;
  _get8 = jjlast_type;
  const struct ReturnValue _7 = dong_porf_porf_todo__Porffor_concatStrings(2225, 195, _6.value, _get8);
  jjlast_type = _7.type;
  _get9 = jjlast_type;
  const struct ReturnValue _8 = dong_porf_porf_todo__Porffor_concatStrings(_7.value, _get9, 2218, 195);
  jjlast_type = _8.type;
  const struct ReturnValue _9 = dong_porf_porf_todo_setTextContent(0, 0, 0, 0, dong_porf_porf_todo_filterActiveId, 1, _8.value, 195);
  jjlast_type = _9.type;
  (void) _9.value;
  _get10 = done;
  f64_store(0, 4, 245760, _get10);
  _get11 = donejjtype;
  i32_store8(0, 12, 245760, _get11);
  i32_store(1, 0, 245760, 1);
  const struct ReturnValue _10 = dong_porf_porf_todo_String(0, 0, 0, 0, 245760, 72);
  jjlast_type = _10.type;
  _get12 = jjlast_type;
  const struct ReturnValue _11 = dong_porf_porf_todo__Porffor_concatStrings(2239, 195, _10.value, _get12);
  jjlast_type = _11.type;
  _get13 = jjlast_type;
  const struct ReturnValue _12 = dong_porf_porf_todo__Porffor_concatStrings(_11.value, _get13, 2218, 195);
  jjlast_type = _12.type;
  const struct ReturnValue _13 = dong_porf_porf_todo_setTextContent(0, 0, 0, 0, dong_porf_porf_todo_filterDoneId, 1, _12.value, 195);
  jjlast_type = _13.type;
  (void) _13.value;
  // if f64
  f64 _r760;
    if ((f64)(dong_porf_porf_todo_filterMode == 0) != 0) {
      jjlast_type = 195;
      _r760 = 2251;
    } else {
      jjlast_type = 195;
      _r760 = 2264;
    }
  // end
  j760:;
  allBg = _r760;
  _get14 = jjlast_type;
  allBgjjtype = _get14;
  // if f64
  f64 _r761;
    if ((f64)(dong_porf_porf_todo_filterMode == 0) != 0) {
      jjlast_type = 195;
      _r761 = 2277;
    } else {
      jjlast_type = 195;
      _r761 = 2287;
    }
  // end
  j761:;
  allColor = _r761;
  _get15 = jjlast_type;
  allColorjjtype = _get15;
  // if f64
  f64 _r762;
    if ((f64)(dong_porf_porf_todo_filterMode == 1) != 0) {
      jjlast_type = 195;
      _r762 = 2251;
    } else {
      jjlast_type = 195;
      _r762 = 2264;
    }
  // end
  j762:;
  actBg = _r762;
  _get16 = jjlast_type;
  actBgjjtype = _get16;
  // if f64
  f64 _r763;
    if ((f64)(dong_porf_porf_todo_filterMode == 1) != 0) {
      jjlast_type = 195;
      _r763 = 2277;
    } else {
      jjlast_type = 195;
      _r763 = 2287;
    }
  // end
  j763:;
  actColor = _r763;
  _get17 = jjlast_type;
  actColorjjtype = _get17;
  // if f64
  f64 _r764;
    if ((f64)(dong_porf_porf_todo_filterMode == 2) != 0) {
      jjlast_type = 195;
      _r764 = 2251;
    } else {
      jjlast_type = 195;
      _r764 = 2264;
    }
  // end
  j764:;
  doneBg = _r764;
  _get18 = jjlast_type;
  doneBgjjtype = _get18;
  // if f64
  f64 _r765;
    if ((f64)(dong_porf_porf_todo_filterMode == 2) != 0) {
      jjlast_type = 195;
      _r765 = 2277;
    } else {
      jjlast_type = 195;
      _r765 = 2287;
    }
  // end
  j765:;
  doneColor = _r765;
  _get19 = jjlast_type;
  doneColorjjtype = _get19;
  _get20 = allBg;
  _get21 = allBgjjtype;
  const struct ReturnValue _14 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filterAllId, 1, 2300, 195, _get20, _get21);
  jjlast_type = _14.type;
  (void) _14.value;
  _get22 = allColor;
  _get23 = allColorjjtype;
  const struct ReturnValue _15 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filterAllId, 1, 2322, 195, _get22, _get23);
  jjlast_type = _15.type;
  (void) _15.value;
  _get24 = actBg;
  _get25 = actBgjjtype;
  const struct ReturnValue _16 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filterActiveId, 1, 2300, 195, _get24, _get25);
  jjlast_type = _16.type;
  (void) _16.value;
  _get26 = actColor;
  _get27 = actColorjjtype;
  const struct ReturnValue _17 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filterActiveId, 1, 2322, 195, _get26, _get27);
  jjlast_type = _17.type;
  (void) _17.value;
  _get28 = doneBg;
  _get29 = doneBgjjtype;
  const struct ReturnValue _18 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filterDoneId, 1, 2300, 195, _get28, _get29);
  jjlast_type = _18.type;
  (void) _18.value;
  _get30 = doneColor;
  _get31 = doneColorjjtype;
  const struct ReturnValue _19 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filterDoneId, 1, 2322, 195, _get30, _get31);
  jjlast_type = _19.type;
  (void) _19.value;
  _get32 = done;
  // if 
    if ((f64)(_get32 > 0) != 0) {
      const struct ReturnValue _20 = dong_porf_porf_todo_removeAttribute(0, 0, 0, 0, dong_porf_porf_todo_clearWrapId, 1, 2333, 195);
      jjlast_type = _20.type;
      (void) _20.value;
      _get33 = done;
      f64_store(0, 4, 262144, _get33);
      _get34 = donejjtype;
      i32_store8(0, 12, 262144, _get34);
      i32_store(1, 0, 262144, 1);
      const struct ReturnValue _21 = dong_porf_porf_todo_String(0, 0, 0, 0, 262144, 72);
      jjlast_type = _21.type;
      _get35 = jjlast_type;
      const struct ReturnValue _22 = dong_porf_porf_todo__Porffor_concatStrings(2345, 195, _21.value, _get35);
      jjlast_type = _22.type;
      _get36 = jjlast_type;
      const struct ReturnValue _23 = dong_porf_porf_todo__Porffor_concatStrings(_22.value, _get36, 2218, 195);
      jjlast_type = _23.type;
      const struct ReturnValue _24 = dong_porf_porf_todo_setTextContent(0, 0, 0, 0, dong_porf_porf_todo_btnClearId, 1, _23.value, 195);
      jjlast_type = _24.type;
      (void) _24.value;
    } else {
      const struct ReturnValue _25 = dong_porf_porf_todo_setAttribute(0, 0, 0, 0, dong_porf_porf_todo_clearWrapId, 1, 2333, 195, 2363, 195);
      jjlast_type = _25.type;
      (void) _25.value;
    }
  // end
  j767:;
  _get37 = jjnewtarget;
  // if 
    if (((u32)(_get37)) != 0) {
      _get38 = jjthis;
      _get39 = jjthisjjtype;
      return (struct ReturnValue){ _get38, _get39 };
    }
  // end
  j770:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_shouldShow(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype) {
  i32 _get53;
  f64 _get52;
  i32 _get51;
  f64 _get50;
  i32 _get49;
  f64 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  f64 _get41;
  f64 _get40;
  i32 _get39;
  f64 _get38;
  i32 _get37;
  f64 _get36;
  i32 _get35;
  f64 _get34;
  f64 _get33;
  f64 _get32;
  i32 _get31;
  i32 _get30;
  f64 _get29;
  i32 _get28;
  i32 _get27;
  i32 _get26;
  f64 _get25;
  i32 _get24;
  f64 _get23;
  i32 _get22;
  f64 _get21;
  i32 _get20;
  f64 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  i32 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  i32 _get1;
  f64 _get0;
  f64 done = 0;
  i32 donejjtype = 0;
  i32 jjlast_type = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = i;
  _get1 = ijjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get0, _get1);
  jjlast_type = _0.type;
  _get2 = jjlast_type;
  donejjtype = _get2;
  done = _0.value;
  // if 
    if ((f64)((dong_porf_porf_todo_filterMode == 1) & ((dong_porf_porf_todo_filterModejjtype | 128) == (1 | 128))) != 0) {
      _get3 = done;
      jjlogicinner_tmp = _get3;
      _get4 = donejjtype;
      jjtypeswitch_tmp1 = _get4;
      // block i32
      i32 _r774;
        _get5 = jjtypeswitch_tmp1;
        _get6 = jjtypeswitch_tmp1;
        // if 
          if (((_get5 == 67) | (_get6 == 195)) != 0) {
            _get7 = jjlogicinner_tmp;
            _r774 = i32_load(1, 0, (u32)(_get7));
            goto j774;
          }
        // end
        j775:;
        _get8 = jjtypeswitch_tmp1;
        _get9 = jjtypeswitch_tmp1;
        // if 
          if (((_get8 == 31) | (_get9 == 32)) != 0) {
            _r774 = 1;
            goto j774;
          }
        // end
        j776:;
        _get10 = jjlogicinner_tmp;
        const f64 _tmp0 = _get10;
        _r774 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
      // end
      j774:;
      // if 
        if ((_r774) != 0) {
          jjreturn = 0;
          jjreturnjjtype = 1;
          _get11 = jjnewtarget;
          // if 
            if (((u32)(_get11)) != 0) {
              _get12 = jjreturn;
              _get13 = jjreturnjjtype;
              // if 
                if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get12), _get13)) == 0) {
                  _get14 = jjthis;
                  _get15 = jjthisjjtype;
                  return (struct ReturnValue){ _get14, _get15 };
                }
              // end
              j779:;
            }
          // end
          j778:;
          _get16 = jjreturn;
          _get17 = jjreturnjjtype;
          return (struct ReturnValue){ _get16, _get17 };
        }
      // end
      j777:;
      jjreturn = 1;
      jjreturnjjtype = 1;
      _get18 = jjnewtarget;
      // if 
        if (((u32)(_get18)) != 0) {
          _get19 = jjreturn;
          _get20 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get19), _get20)) == 0) {
              _get21 = jjthis;
              _get22 = jjthisjjtype;
              return (struct ReturnValue){ _get21, _get22 };
            }
          // end
          j781:;
        }
      // end
      j780:;
      _get23 = jjreturn;
      _get24 = jjreturnjjtype;
      return (struct ReturnValue){ _get23, _get24 };
    }
  // end
  j773:;
  // if 
    if ((f64)((dong_porf_porf_todo_filterMode == 2) & ((dong_porf_porf_todo_filterModejjtype | 128) == (1 | 128))) != 0) {
      _get25 = done;
      jjlogicinner_tmp = _get25;
      _get26 = donejjtype;
      jjtypeswitch_tmp1 = _get26;
      // block i32
      i32 _r783;
        _get27 = jjtypeswitch_tmp1;
        _get28 = jjtypeswitch_tmp1;
        // if 
          if (((_get27 == 67) | (_get28 == 195)) != 0) {
            _get29 = jjlogicinner_tmp;
            _r783 = i32_load(1, 0, (u32)(_get29));
            goto j783;
          }
        // end
        j784:;
        _get30 = jjtypeswitch_tmp1;
        _get31 = jjtypeswitch_tmp1;
        // if 
          if (((_get30 == 31) | (_get31 == 32)) != 0) {
            _r783 = 1;
            goto j783;
          }
        // end
        j785:;
        _get32 = jjlogicinner_tmp;
        const f64 _tmp1 = _get32;
        _r783 = (_tmp1 < 0 ? -_tmp1 : _tmp1) > 0;
      // end
      j783:;
      // if 
        if ((_r783) != 0) {
          jjreturn = 1;
          jjreturnjjtype = 1;
          _get33 = jjnewtarget;
          // if 
            if (((u32)(_get33)) != 0) {
              _get34 = jjreturn;
              _get35 = jjreturnjjtype;
              // if 
                if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get34), _get35)) == 0) {
                  _get36 = jjthis;
                  _get37 = jjthisjjtype;
                  return (struct ReturnValue){ _get36, _get37 };
                }
              // end
              j788:;
            }
          // end
          j787:;
          _get38 = jjreturn;
          _get39 = jjreturnjjtype;
          return (struct ReturnValue){ _get38, _get39 };
        }
      // end
      j786:;
      jjreturn = 0;
      jjreturnjjtype = 1;
      _get40 = jjnewtarget;
      // if 
        if (((u32)(_get40)) != 0) {
          _get41 = jjreturn;
          _get42 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get41), _get42)) == 0) {
              _get43 = jjthis;
              _get44 = jjthisjjtype;
              return (struct ReturnValue){ _get43, _get44 };
            }
          // end
          j790:;
        }
      // end
      j789:;
      _get45 = jjreturn;
      _get46 = jjreturnjjtype;
      return (struct ReturnValue){ _get45, _get46 };
    }
  // end
  j782:;
  jjreturn = 1;
  jjreturnjjtype = 1;
  _get47 = jjnewtarget;
  // if 
    if (((u32)(_get47)) != 0) {
      _get48 = jjreturn;
      _get49 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get48), _get49)) == 0) {
          _get50 = jjthis;
          _get51 = jjthisjjtype;
          return (struct ReturnValue){ _get50, _get51 };
        }
      // end
      j792:;
    }
  // end
  j791:;
  _get52 = jjreturn;
  _get53 = jjreturnjjtype;
  return (struct ReturnValue){ _get52, _get53 };
}

static struct ReturnValue dong_porf_porf_todo_todoIdAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype) {
  i32 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  i32 _get137;
  f64 _get136;
  f64 _get135;
  i32 _get134;
  f64 _get133;
  i32 _get132;
  f64 _get131;
  i32 _get130;
  f64 _get129;
  f64 _get128;
  i32 _get127;
  f64 _get126;
  i32 _get125;
  f64 _get124;
  i32 _get123;
  f64 _get122;
  i32 _get121;
  f64 _get120;
  f64 _get119;
  i32 _get118;
  f64 _get117;
  i32 _get116;
  f64 _get115;
  i32 _get114;
  f64 _get113;
  i32 _get112;
  f64 _get111;
  f64 _get110;
  i32 _get109;
  f64 _get108;
  i32 _get107;
  f64 _get106;
  i32 _get105;
  f64 _get104;
  i32 _get103;
  f64 _get102;
  f64 _get101;
  i32 _get100;
  f64 _get99;
  i32 _get98;
  f64 _get97;
  i32 _get96;
  f64 _get95;
  i32 _get94;
  f64 _get93;
  f64 _get92;
  i32 _get91;
  f64 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  f64 _get86;
  i32 _get85;
  f64 _get84;
  f64 _get83;
  i32 _get82;
  f64 _get81;
  i32 _get80;
  f64 _get79;
  i32 _get78;
  f64 _get77;
  i32 _get76;
  f64 _get75;
  f64 _get74;
  i32 _get73;
  f64 _get72;
  i32 _get71;
  f64 _get70;
  i32 _get69;
  f64 _get68;
  i32 _get67;
  f64 _get66;
  f64 _get65;
  i32 _get64;
  f64 _get63;
  i32 _get62;
  f64 _get61;
  i32 _get60;
  f64 _get59;
  i32 _get58;
  f64 _get57;
  f64 _get56;
  i32 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  i32 _get51;
  f64 _get50;
  i32 _get49;
  f64 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  f64 _get41;
  i32 _get40;
  f64 _get39;
  f64 _get38;
  i32 _get37;
  f64 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  f64 _get32;
  i32 _get31;
  f64 _get30;
  f64 _get29;
  i32 _get28;
  f64 _get27;
  i32 _get26;
  f64 _get25;
  i32 _get24;
  f64 _get23;
  i32 _get22;
  f64 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  f64 _get12;
  f64 _get11;
  i32 _get10;
  f64 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = i;
  _get1 = ijjtype;
  // if 
    if ((f64)((_get0 == 0) & ((_get1 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId0;
      jjreturnjjtype = dong_porf_porf_todo_todoId0jjtype;
      _get2 = jjnewtarget;
      // if 
        if (((u32)(_get2)) != 0) {
          _get3 = jjreturn;
          _get4 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get3), _get4)) == 0) {
              _get5 = jjthis;
              _get6 = jjthisjjtype;
              return (struct ReturnValue){ _get5, _get6 };
            }
          // end
          j799:;
        }
      // end
      j798:;
      _get7 = jjreturn;
      _get8 = jjreturnjjtype;
      return (struct ReturnValue){ _get7, _get8 };
    }
  // end
  j797:;
  _get9 = i;
  _get10 = ijjtype;
  // if 
    if ((f64)((_get9 == 1) & ((_get10 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId1;
      jjreturnjjtype = dong_porf_porf_todo_todoId1jjtype;
      _get11 = jjnewtarget;
      // if 
        if (((u32)(_get11)) != 0) {
          _get12 = jjreturn;
          _get13 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get12), _get13)) == 0) {
              _get14 = jjthis;
              _get15 = jjthisjjtype;
              return (struct ReturnValue){ _get14, _get15 };
            }
          // end
          j802:;
        }
      // end
      j801:;
      _get16 = jjreturn;
      _get17 = jjreturnjjtype;
      return (struct ReturnValue){ _get16, _get17 };
    }
  // end
  j800:;
  _get18 = i;
  _get19 = ijjtype;
  // if 
    if ((f64)((_get18 == 2) & ((_get19 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId2;
      jjreturnjjtype = dong_porf_porf_todo_todoId2jjtype;
      _get20 = jjnewtarget;
      // if 
        if (((u32)(_get20)) != 0) {
          _get21 = jjreturn;
          _get22 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get21), _get22)) == 0) {
              _get23 = jjthis;
              _get24 = jjthisjjtype;
              return (struct ReturnValue){ _get23, _get24 };
            }
          // end
          j805:;
        }
      // end
      j804:;
      _get25 = jjreturn;
      _get26 = jjreturnjjtype;
      return (struct ReturnValue){ _get25, _get26 };
    }
  // end
  j803:;
  _get27 = i;
  _get28 = ijjtype;
  // if 
    if ((f64)((_get27 == 3) & ((_get28 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId3;
      jjreturnjjtype = dong_porf_porf_todo_todoId3jjtype;
      _get29 = jjnewtarget;
      // if 
        if (((u32)(_get29)) != 0) {
          _get30 = jjreturn;
          _get31 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get30), _get31)) == 0) {
              _get32 = jjthis;
              _get33 = jjthisjjtype;
              return (struct ReturnValue){ _get32, _get33 };
            }
          // end
          j808:;
        }
      // end
      j807:;
      _get34 = jjreturn;
      _get35 = jjreturnjjtype;
      return (struct ReturnValue){ _get34, _get35 };
    }
  // end
  j806:;
  _get36 = i;
  _get37 = ijjtype;
  // if 
    if ((f64)((_get36 == 4) & ((_get37 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId4;
      jjreturnjjtype = dong_porf_porf_todo_todoId4jjtype;
      _get38 = jjnewtarget;
      // if 
        if (((u32)(_get38)) != 0) {
          _get39 = jjreturn;
          _get40 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get39), _get40)) == 0) {
              _get41 = jjthis;
              _get42 = jjthisjjtype;
              return (struct ReturnValue){ _get41, _get42 };
            }
          // end
          j811:;
        }
      // end
      j810:;
      _get43 = jjreturn;
      _get44 = jjreturnjjtype;
      return (struct ReturnValue){ _get43, _get44 };
    }
  // end
  j809:;
  _get45 = i;
  _get46 = ijjtype;
  // if 
    if ((f64)((_get45 == 5) & ((_get46 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId5;
      jjreturnjjtype = dong_porf_porf_todo_todoId5jjtype;
      _get47 = jjnewtarget;
      // if 
        if (((u32)(_get47)) != 0) {
          _get48 = jjreturn;
          _get49 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get48), _get49)) == 0) {
              _get50 = jjthis;
              _get51 = jjthisjjtype;
              return (struct ReturnValue){ _get50, _get51 };
            }
          // end
          j814:;
        }
      // end
      j813:;
      _get52 = jjreturn;
      _get53 = jjreturnjjtype;
      return (struct ReturnValue){ _get52, _get53 };
    }
  // end
  j812:;
  _get54 = i;
  _get55 = ijjtype;
  // if 
    if ((f64)((_get54 == 6) & ((_get55 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId6;
      jjreturnjjtype = dong_porf_porf_todo_todoId6jjtype;
      _get56 = jjnewtarget;
      // if 
        if (((u32)(_get56)) != 0) {
          _get57 = jjreturn;
          _get58 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get57), _get58)) == 0) {
              _get59 = jjthis;
              _get60 = jjthisjjtype;
              return (struct ReturnValue){ _get59, _get60 };
            }
          // end
          j817:;
        }
      // end
      j816:;
      _get61 = jjreturn;
      _get62 = jjreturnjjtype;
      return (struct ReturnValue){ _get61, _get62 };
    }
  // end
  j815:;
  _get63 = i;
  _get64 = ijjtype;
  // if 
    if ((f64)((_get63 == 7) & ((_get64 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId7;
      jjreturnjjtype = dong_porf_porf_todo_todoId7jjtype;
      _get65 = jjnewtarget;
      // if 
        if (((u32)(_get65)) != 0) {
          _get66 = jjreturn;
          _get67 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get66), _get67)) == 0) {
              _get68 = jjthis;
              _get69 = jjthisjjtype;
              return (struct ReturnValue){ _get68, _get69 };
            }
          // end
          j820:;
        }
      // end
      j819:;
      _get70 = jjreturn;
      _get71 = jjreturnjjtype;
      return (struct ReturnValue){ _get70, _get71 };
    }
  // end
  j818:;
  _get72 = i;
  _get73 = ijjtype;
  // if 
    if ((f64)((_get72 == 8) & ((_get73 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId8;
      jjreturnjjtype = dong_porf_porf_todo_todoId8jjtype;
      _get74 = jjnewtarget;
      // if 
        if (((u32)(_get74)) != 0) {
          _get75 = jjreturn;
          _get76 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get75), _get76)) == 0) {
              _get77 = jjthis;
              _get78 = jjthisjjtype;
              return (struct ReturnValue){ _get77, _get78 };
            }
          // end
          j823:;
        }
      // end
      j822:;
      _get79 = jjreturn;
      _get80 = jjreturnjjtype;
      return (struct ReturnValue){ _get79, _get80 };
    }
  // end
  j821:;
  _get81 = i;
  _get82 = ijjtype;
  // if 
    if ((f64)((_get81 == 9) & ((_get82 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId9;
      jjreturnjjtype = dong_porf_porf_todo_todoId9jjtype;
      _get83 = jjnewtarget;
      // if 
        if (((u32)(_get83)) != 0) {
          _get84 = jjreturn;
          _get85 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get84), _get85)) == 0) {
              _get86 = jjthis;
              _get87 = jjthisjjtype;
              return (struct ReturnValue){ _get86, _get87 };
            }
          // end
          j826:;
        }
      // end
      j825:;
      _get88 = jjreturn;
      _get89 = jjreturnjjtype;
      return (struct ReturnValue){ _get88, _get89 };
    }
  // end
  j824:;
  _get90 = i;
  _get91 = ijjtype;
  // if 
    if ((f64)((_get90 == 10) & ((_get91 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId10;
      jjreturnjjtype = dong_porf_porf_todo_todoId10jjtype;
      _get92 = jjnewtarget;
      // if 
        if (((u32)(_get92)) != 0) {
          _get93 = jjreturn;
          _get94 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get93), _get94)) == 0) {
              _get95 = jjthis;
              _get96 = jjthisjjtype;
              return (struct ReturnValue){ _get95, _get96 };
            }
          // end
          j829:;
        }
      // end
      j828:;
      _get97 = jjreturn;
      _get98 = jjreturnjjtype;
      return (struct ReturnValue){ _get97, _get98 };
    }
  // end
  j827:;
  _get99 = i;
  _get100 = ijjtype;
  // if 
    if ((f64)((_get99 == 11) & ((_get100 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId11;
      jjreturnjjtype = dong_porf_porf_todo_todoId11jjtype;
      _get101 = jjnewtarget;
      // if 
        if (((u32)(_get101)) != 0) {
          _get102 = jjreturn;
          _get103 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get102), _get103)) == 0) {
              _get104 = jjthis;
              _get105 = jjthisjjtype;
              return (struct ReturnValue){ _get104, _get105 };
            }
          // end
          j832:;
        }
      // end
      j831:;
      _get106 = jjreturn;
      _get107 = jjreturnjjtype;
      return (struct ReturnValue){ _get106, _get107 };
    }
  // end
  j830:;
  _get108 = i;
  _get109 = ijjtype;
  // if 
    if ((f64)((_get108 == 12) & ((_get109 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId12;
      jjreturnjjtype = dong_porf_porf_todo_todoId12jjtype;
      _get110 = jjnewtarget;
      // if 
        if (((u32)(_get110)) != 0) {
          _get111 = jjreturn;
          _get112 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get111), _get112)) == 0) {
              _get113 = jjthis;
              _get114 = jjthisjjtype;
              return (struct ReturnValue){ _get113, _get114 };
            }
          // end
          j835:;
        }
      // end
      j834:;
      _get115 = jjreturn;
      _get116 = jjreturnjjtype;
      return (struct ReturnValue){ _get115, _get116 };
    }
  // end
  j833:;
  _get117 = i;
  _get118 = ijjtype;
  // if 
    if ((f64)((_get117 == 13) & ((_get118 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId13;
      jjreturnjjtype = dong_porf_porf_todo_todoId13jjtype;
      _get119 = jjnewtarget;
      // if 
        if (((u32)(_get119)) != 0) {
          _get120 = jjreturn;
          _get121 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get120), _get121)) == 0) {
              _get122 = jjthis;
              _get123 = jjthisjjtype;
              return (struct ReturnValue){ _get122, _get123 };
            }
          // end
          j838:;
        }
      // end
      j837:;
      _get124 = jjreturn;
      _get125 = jjreturnjjtype;
      return (struct ReturnValue){ _get124, _get125 };
    }
  // end
  j836:;
  _get126 = i;
  _get127 = ijjtype;
  // if 
    if ((f64)((_get126 == 14) & ((_get127 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId14;
      jjreturnjjtype = dong_porf_porf_todo_todoId14jjtype;
      _get128 = jjnewtarget;
      // if 
        if (((u32)(_get128)) != 0) {
          _get129 = jjreturn;
          _get130 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get129), _get130)) == 0) {
              _get131 = jjthis;
              _get132 = jjthisjjtype;
              return (struct ReturnValue){ _get131, _get132 };
            }
          // end
          j841:;
        }
      // end
      j840:;
      _get133 = jjreturn;
      _get134 = jjreturnjjtype;
      return (struct ReturnValue){ _get133, _get134 };
    }
  // end
  j839:;
  jjreturn = dong_porf_porf_todo_todoId15;
  jjreturnjjtype = dong_porf_porf_todo_todoId15jjtype;
  _get135 = jjnewtarget;
  // if 
    if (((u32)(_get135)) != 0) {
      _get136 = jjreturn;
      _get137 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get136), _get137)) == 0) {
          _get138 = jjthis;
          _get139 = jjthisjjtype;
          return (struct ReturnValue){ _get138, _get139 };
        }
      // end
      j843:;
    }
  // end
  j842:;
  _get140 = jjreturn;
  _get141 = jjreturnjjtype;
  return (struct ReturnValue){ _get140, _get141 };
}

static struct ReturnValue dong_porf_porf_todo_todoTextAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype) {
  i32 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  i32 _get137;
  f64 _get136;
  f64 _get135;
  i32 _get134;
  f64 _get133;
  i32 _get132;
  f64 _get131;
  i32 _get130;
  f64 _get129;
  f64 _get128;
  i32 _get127;
  f64 _get126;
  i32 _get125;
  f64 _get124;
  i32 _get123;
  f64 _get122;
  i32 _get121;
  f64 _get120;
  f64 _get119;
  i32 _get118;
  f64 _get117;
  i32 _get116;
  f64 _get115;
  i32 _get114;
  f64 _get113;
  i32 _get112;
  f64 _get111;
  f64 _get110;
  i32 _get109;
  f64 _get108;
  i32 _get107;
  f64 _get106;
  i32 _get105;
  f64 _get104;
  i32 _get103;
  f64 _get102;
  f64 _get101;
  i32 _get100;
  f64 _get99;
  i32 _get98;
  f64 _get97;
  i32 _get96;
  f64 _get95;
  i32 _get94;
  f64 _get93;
  f64 _get92;
  i32 _get91;
  f64 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  f64 _get86;
  i32 _get85;
  f64 _get84;
  f64 _get83;
  i32 _get82;
  f64 _get81;
  i32 _get80;
  f64 _get79;
  i32 _get78;
  f64 _get77;
  i32 _get76;
  f64 _get75;
  f64 _get74;
  i32 _get73;
  f64 _get72;
  i32 _get71;
  f64 _get70;
  i32 _get69;
  f64 _get68;
  i32 _get67;
  f64 _get66;
  f64 _get65;
  i32 _get64;
  f64 _get63;
  i32 _get62;
  f64 _get61;
  i32 _get60;
  f64 _get59;
  i32 _get58;
  f64 _get57;
  f64 _get56;
  i32 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  i32 _get51;
  f64 _get50;
  i32 _get49;
  f64 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  f64 _get41;
  i32 _get40;
  f64 _get39;
  f64 _get38;
  i32 _get37;
  f64 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  f64 _get32;
  i32 _get31;
  f64 _get30;
  f64 _get29;
  i32 _get28;
  f64 _get27;
  i32 _get26;
  f64 _get25;
  i32 _get24;
  f64 _get23;
  i32 _get22;
  f64 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  f64 _get12;
  f64 _get11;
  i32 _get10;
  f64 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = i;
  _get1 = ijjtype;
  // if 
    if ((f64)((_get0 == 0) & ((_get1 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText0;
      jjreturnjjtype = dong_porf_porf_todo_todoText0jjtype;
      _get2 = jjnewtarget;
      // if 
        if (((u32)(_get2)) != 0) {
          _get3 = jjreturn;
          _get4 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get3), _get4)) == 0) {
              _get5 = jjthis;
              _get6 = jjthisjjtype;
              return (struct ReturnValue){ _get5, _get6 };
            }
          // end
          j846:;
        }
      // end
      j845:;
      _get7 = jjreturn;
      _get8 = jjreturnjjtype;
      return (struct ReturnValue){ _get7, _get8 };
    }
  // end
  j844:;
  _get9 = i;
  _get10 = ijjtype;
  // if 
    if ((f64)((_get9 == 1) & ((_get10 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText1;
      jjreturnjjtype = dong_porf_porf_todo_todoText1jjtype;
      _get11 = jjnewtarget;
      // if 
        if (((u32)(_get11)) != 0) {
          _get12 = jjreturn;
          _get13 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get12), _get13)) == 0) {
              _get14 = jjthis;
              _get15 = jjthisjjtype;
              return (struct ReturnValue){ _get14, _get15 };
            }
          // end
          j849:;
        }
      // end
      j848:;
      _get16 = jjreturn;
      _get17 = jjreturnjjtype;
      return (struct ReturnValue){ _get16, _get17 };
    }
  // end
  j847:;
  _get18 = i;
  _get19 = ijjtype;
  // if 
    if ((f64)((_get18 == 2) & ((_get19 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText2;
      jjreturnjjtype = dong_porf_porf_todo_todoText2jjtype;
      _get20 = jjnewtarget;
      // if 
        if (((u32)(_get20)) != 0) {
          _get21 = jjreturn;
          _get22 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get21), _get22)) == 0) {
              _get23 = jjthis;
              _get24 = jjthisjjtype;
              return (struct ReturnValue){ _get23, _get24 };
            }
          // end
          j852:;
        }
      // end
      j851:;
      _get25 = jjreturn;
      _get26 = jjreturnjjtype;
      return (struct ReturnValue){ _get25, _get26 };
    }
  // end
  j850:;
  _get27 = i;
  _get28 = ijjtype;
  // if 
    if ((f64)((_get27 == 3) & ((_get28 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText3;
      jjreturnjjtype = dong_porf_porf_todo_todoText3jjtype;
      _get29 = jjnewtarget;
      // if 
        if (((u32)(_get29)) != 0) {
          _get30 = jjreturn;
          _get31 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get30), _get31)) == 0) {
              _get32 = jjthis;
              _get33 = jjthisjjtype;
              return (struct ReturnValue){ _get32, _get33 };
            }
          // end
          j855:;
        }
      // end
      j854:;
      _get34 = jjreturn;
      _get35 = jjreturnjjtype;
      return (struct ReturnValue){ _get34, _get35 };
    }
  // end
  j853:;
  _get36 = i;
  _get37 = ijjtype;
  // if 
    if ((f64)((_get36 == 4) & ((_get37 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText4;
      jjreturnjjtype = dong_porf_porf_todo_todoText4jjtype;
      _get38 = jjnewtarget;
      // if 
        if (((u32)(_get38)) != 0) {
          _get39 = jjreturn;
          _get40 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get39), _get40)) == 0) {
              _get41 = jjthis;
              _get42 = jjthisjjtype;
              return (struct ReturnValue){ _get41, _get42 };
            }
          // end
          j858:;
        }
      // end
      j857:;
      _get43 = jjreturn;
      _get44 = jjreturnjjtype;
      return (struct ReturnValue){ _get43, _get44 };
    }
  // end
  j856:;
  _get45 = i;
  _get46 = ijjtype;
  // if 
    if ((f64)((_get45 == 5) & ((_get46 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText5;
      jjreturnjjtype = dong_porf_porf_todo_todoText5jjtype;
      _get47 = jjnewtarget;
      // if 
        if (((u32)(_get47)) != 0) {
          _get48 = jjreturn;
          _get49 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get48), _get49)) == 0) {
              _get50 = jjthis;
              _get51 = jjthisjjtype;
              return (struct ReturnValue){ _get50, _get51 };
            }
          // end
          j861:;
        }
      // end
      j860:;
      _get52 = jjreturn;
      _get53 = jjreturnjjtype;
      return (struct ReturnValue){ _get52, _get53 };
    }
  // end
  j859:;
  _get54 = i;
  _get55 = ijjtype;
  // if 
    if ((f64)((_get54 == 6) & ((_get55 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText6;
      jjreturnjjtype = dong_porf_porf_todo_todoText6jjtype;
      _get56 = jjnewtarget;
      // if 
        if (((u32)(_get56)) != 0) {
          _get57 = jjreturn;
          _get58 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get57), _get58)) == 0) {
              _get59 = jjthis;
              _get60 = jjthisjjtype;
              return (struct ReturnValue){ _get59, _get60 };
            }
          // end
          j864:;
        }
      // end
      j863:;
      _get61 = jjreturn;
      _get62 = jjreturnjjtype;
      return (struct ReturnValue){ _get61, _get62 };
    }
  // end
  j862:;
  _get63 = i;
  _get64 = ijjtype;
  // if 
    if ((f64)((_get63 == 7) & ((_get64 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText7;
      jjreturnjjtype = dong_porf_porf_todo_todoText7jjtype;
      _get65 = jjnewtarget;
      // if 
        if (((u32)(_get65)) != 0) {
          _get66 = jjreturn;
          _get67 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get66), _get67)) == 0) {
              _get68 = jjthis;
              _get69 = jjthisjjtype;
              return (struct ReturnValue){ _get68, _get69 };
            }
          // end
          j867:;
        }
      // end
      j866:;
      _get70 = jjreturn;
      _get71 = jjreturnjjtype;
      return (struct ReturnValue){ _get70, _get71 };
    }
  // end
  j865:;
  _get72 = i;
  _get73 = ijjtype;
  // if 
    if ((f64)((_get72 == 8) & ((_get73 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText8;
      jjreturnjjtype = dong_porf_porf_todo_todoText8jjtype;
      _get74 = jjnewtarget;
      // if 
        if (((u32)(_get74)) != 0) {
          _get75 = jjreturn;
          _get76 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get75), _get76)) == 0) {
              _get77 = jjthis;
              _get78 = jjthisjjtype;
              return (struct ReturnValue){ _get77, _get78 };
            }
          // end
          j870:;
        }
      // end
      j869:;
      _get79 = jjreturn;
      _get80 = jjreturnjjtype;
      return (struct ReturnValue){ _get79, _get80 };
    }
  // end
  j868:;
  _get81 = i;
  _get82 = ijjtype;
  // if 
    if ((f64)((_get81 == 9) & ((_get82 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText9;
      jjreturnjjtype = dong_porf_porf_todo_todoText9jjtype;
      _get83 = jjnewtarget;
      // if 
        if (((u32)(_get83)) != 0) {
          _get84 = jjreturn;
          _get85 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get84), _get85)) == 0) {
              _get86 = jjthis;
              _get87 = jjthisjjtype;
              return (struct ReturnValue){ _get86, _get87 };
            }
          // end
          j873:;
        }
      // end
      j872:;
      _get88 = jjreturn;
      _get89 = jjreturnjjtype;
      return (struct ReturnValue){ _get88, _get89 };
    }
  // end
  j871:;
  _get90 = i;
  _get91 = ijjtype;
  // if 
    if ((f64)((_get90 == 10) & ((_get91 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText10;
      jjreturnjjtype = dong_porf_porf_todo_todoText10jjtype;
      _get92 = jjnewtarget;
      // if 
        if (((u32)(_get92)) != 0) {
          _get93 = jjreturn;
          _get94 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get93), _get94)) == 0) {
              _get95 = jjthis;
              _get96 = jjthisjjtype;
              return (struct ReturnValue){ _get95, _get96 };
            }
          // end
          j876:;
        }
      // end
      j875:;
      _get97 = jjreturn;
      _get98 = jjreturnjjtype;
      return (struct ReturnValue){ _get97, _get98 };
    }
  // end
  j874:;
  _get99 = i;
  _get100 = ijjtype;
  // if 
    if ((f64)((_get99 == 11) & ((_get100 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText11;
      jjreturnjjtype = dong_porf_porf_todo_todoText11jjtype;
      _get101 = jjnewtarget;
      // if 
        if (((u32)(_get101)) != 0) {
          _get102 = jjreturn;
          _get103 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get102), _get103)) == 0) {
              _get104 = jjthis;
              _get105 = jjthisjjtype;
              return (struct ReturnValue){ _get104, _get105 };
            }
          // end
          j879:;
        }
      // end
      j878:;
      _get106 = jjreturn;
      _get107 = jjreturnjjtype;
      return (struct ReturnValue){ _get106, _get107 };
    }
  // end
  j877:;
  _get108 = i;
  _get109 = ijjtype;
  // if 
    if ((f64)((_get108 == 12) & ((_get109 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText12;
      jjreturnjjtype = dong_porf_porf_todo_todoText12jjtype;
      _get110 = jjnewtarget;
      // if 
        if (((u32)(_get110)) != 0) {
          _get111 = jjreturn;
          _get112 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get111), _get112)) == 0) {
              _get113 = jjthis;
              _get114 = jjthisjjtype;
              return (struct ReturnValue){ _get113, _get114 };
            }
          // end
          j882:;
        }
      // end
      j881:;
      _get115 = jjreturn;
      _get116 = jjreturnjjtype;
      return (struct ReturnValue){ _get115, _get116 };
    }
  // end
  j880:;
  _get117 = i;
  _get118 = ijjtype;
  // if 
    if ((f64)((_get117 == 13) & ((_get118 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText13;
      jjreturnjjtype = dong_porf_porf_todo_todoText13jjtype;
      _get119 = jjnewtarget;
      // if 
        if (((u32)(_get119)) != 0) {
          _get120 = jjreturn;
          _get121 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get120), _get121)) == 0) {
              _get122 = jjthis;
              _get123 = jjthisjjtype;
              return (struct ReturnValue){ _get122, _get123 };
            }
          // end
          j885:;
        }
      // end
      j884:;
      _get124 = jjreturn;
      _get125 = jjreturnjjtype;
      return (struct ReturnValue){ _get124, _get125 };
    }
  // end
  j883:;
  _get126 = i;
  _get127 = ijjtype;
  // if 
    if ((f64)((_get126 == 14) & ((_get127 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText14;
      jjreturnjjtype = dong_porf_porf_todo_todoText14jjtype;
      _get128 = jjnewtarget;
      // if 
        if (((u32)(_get128)) != 0) {
          _get129 = jjreturn;
          _get130 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get129), _get130)) == 0) {
              _get131 = jjthis;
              _get132 = jjthisjjtype;
              return (struct ReturnValue){ _get131, _get132 };
            }
          // end
          j888:;
        }
      // end
      j887:;
      _get133 = jjreturn;
      _get134 = jjreturnjjtype;
      return (struct ReturnValue){ _get133, _get134 };
    }
  // end
  j886:;
  jjreturn = dong_porf_porf_todo_todoText15;
  jjreturnjjtype = dong_porf_porf_todo_todoText15jjtype;
  _get135 = jjnewtarget;
  // if 
    if (((u32)(_get135)) != 0) {
      _get136 = jjreturn;
      _get137 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get136), _get137)) == 0) {
          _get138 = jjthis;
          _get139 = jjthisjjtype;
          return (struct ReturnValue){ _get138, _get139 };
        }
      // end
      j890:;
    }
  // end
  j889:;
  _get140 = jjreturn;
  _get141 = jjreturnjjtype;
  return (struct ReturnValue){ _get140, _get141 };
}

static struct ReturnValue dong_porf_porf_todo_setInnerHTML(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 html, i32 htmljjtype) {
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = nodeId;
  _get1 = html;
  _get2 = htmljjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get1, _get2);
  (void) _0.type;
  __porf_import_dong_set_inner_html(_get0, _0.value);
  _get3 = jjnewtarget;
  // if 
    if (((u32)(_get3)) != 0) {
      _get4 = jjthis;
      _get5 = jjthisjjtype;
      return (struct ReturnValue){ _get4, _get5 };
    }
  // end
  j895:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_porfRebuildList(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get69;
  f64 _get68;
  f64 _get67;
  f64 _get66;
  f64 _get65;
  f64 _get64;
  i32 _get63;
  i32 _get62;
  f64 _get61;
  i32 _get60;
  i32 _get59;
  i32 _get58;
  i32 _get57;
  f64 _get56;
  i32 _get55;
  i32 _get54;
  f64 _get53;
  i32 _get52;
  i32 _get51;
  i32 _get50;
  f64 _get49;
  f64 _get48;
  f64 _get47;
  f64 _get46;
  f64 _get45;
  f64 _get44;
  f64 _get43;
  f64 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  f64 _get38;
  i32 _get37;
  i32 _get36;
  f64 _get35;
  i32 _get34;
  i32 _get33;
  f64 _get32;
  f64 _get31;
  f64 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  f64 _get26;
  i32 _get25;
  i32 _get24;
  f64 _get23;
  i32 _get22;
  i32 _get21;
  i32 _get20;
  f64 _get19;
  i32 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  i32 _get14;
  f64 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  f64 _get9;
  i32 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  f64 html = 0;
  i32 htmljjtype = 0;
  f64 i = 0;
  i32 ijjtype = 0;
  i32 jjlast_type = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 tid = 0;
  i32 tidjjtype = 0;
  f64 text = 0;
  i32 textjjtype = 0;
  f64 done = 0;
  i32 donejjtype = 0;
  f64 strike = 0;
  i32 strikejjtype = 0;
  f64 color = 0;
  i32 colorjjtype = 0;
  f64 checkBg = 0;
  i32 checkBgjjtype = 0;
  f64 checkBorder = 0;
  i32 checkBorderjjtype = 0;
  f64 checkMark = 0;
  i32 checkMarkjjtype = 0;

  html = 0;
  htmljjtype = 195;
  i = 0;
  ijjtype = 1;
  // loop 
  j771:;
    _get0 = i;
    // if 
      if (_get0 < dong_porf_porf_todo_todoCount) {
        _get1 = i;
        _get2 = ijjtype;
        const struct ReturnValue _0 = dong_porf_porf_todo_shouldShow(0, 0, 0, 0, _get1, _get2);
        jjlast_type = _0.type;
        jjlogicinner_tmp = _0.value;
        _get3 = jjlast_type;
        jjtypeswitch_tmp1 = _get3;
        // block i32
        i32 _r793;
          _get4 = jjtypeswitch_tmp1;
          _get5 = jjtypeswitch_tmp1;
          // if 
            if (((_get4 == 67) | (_get5 == 195)) != 0) {
              _get6 = jjlogicinner_tmp;
              _r793 = i32_load(1, 0, (u32)(_get6));
              goto j793;
            }
          // end
          j794:;
          _get7 = jjtypeswitch_tmp1;
          _get8 = jjtypeswitch_tmp1;
          // if 
            if (((_get7 == 31) | (_get8 == 32)) != 0) {
              _r793 = 1;
              goto j793;
            }
          // end
          j795:;
          _get9 = jjlogicinner_tmp;
          const f64 _tmp0 = _get9;
          _r793 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
        // end
        j793:;
        // if 
          if ((_r793) != 0) {
            _get10 = i;
            _get11 = ijjtype;
            const struct ReturnValue _1 = dong_porf_porf_todo_todoIdAt(0, 0, 0, 0, _get10, _get11);
            jjlast_type = _1.type;
            _get12 = jjlast_type;
            tidjjtype = _get12;
            tid = _1.value;
            _get13 = i;
            _get14 = ijjtype;
            const struct ReturnValue _2 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get13, _get14);
            jjlast_type = _2.type;
            _get15 = jjlast_type;
            textjjtype = _get15;
            text = _2.value;
            _get16 = i;
            _get17 = ijjtype;
            const struct ReturnValue _3 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get16, _get17);
            jjlast_type = _3.type;
            _get18 = jjlast_type;
            donejjtype = _get18;
            done = _3.value;
            strike = 0;
            strikejjtype = 195;
            color = 2370;
            colorjjtype = 195;
            checkBg = 2383;
            checkBgjjtype = 195;
            checkBorder = 2400;
            checkBorderjjtype = 195;
            checkMark = 0;
            checkMarkjjtype = 195;
            _get19 = done;
            jjlogicinner_tmp = _get19;
            _get20 = donejjtype;
            jjtypeswitch_tmp1 = _get20;
            // block i32
            i32 _r891;
              _get21 = jjtypeswitch_tmp1;
              _get22 = jjtypeswitch_tmp1;
              // if 
                if (((_get21 == 67) | (_get22 == 195)) != 0) {
                  _get23 = jjlogicinner_tmp;
                  _r891 = i32_load(1, 0, (u32)(_get23));
                  goto j891;
                }
              // end
              j892:;
              _get24 = jjtypeswitch_tmp1;
              _get25 = jjtypeswitch_tmp1;
              // if 
                if (((_get24 == 31) | (_get25 == 32)) != 0) {
                  _r891 = 1;
                  goto j891;
                }
              // end
              j893:;
              _get26 = jjlogicinner_tmp;
              const f64 _tmp1 = _get26;
              _r891 = (_tmp1 < 0 ? -_tmp1 : _tmp1) > 0;
            // end
            j891:;
            // if 
              if ((_r891) != 0) {
                strike = 2423;
                _get27 = strike;
                strikejjtype = 195;
                (void) _get27;
                color = 2441;
                _get28 = color;
                colorjjtype = 195;
                (void) _get28;
                checkBg = 2454;
                _get29 = checkBg;
                checkBgjjtype = 195;
                (void) _get29;
                checkBorder = 2467;
                _get30 = checkBorder;
                checkBorderjjtype = 195;
                (void) _get30;
                checkMark = 2477;
                _get31 = checkMark;
                checkMarkjjtype = 195;
                (void) _get31;
              }
            // end
            j894:;
            _get32 = html;
            _get33 = htmljjtype;
            const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_concatStrings(_get32, _get33, 2484, 195);
            jjlast_type = _4.type;
            _get34 = jjlast_type;
            htmljjtype = _get34;
            html = _4.value;
            _get35 = html;
            _get36 = htmljjtype;
            const struct ReturnValue _5 = dong_porf_porf_todo__Porffor_concatStrings(_get35, _get36, 2643, 195);
            jjlast_type = _5.type;
            _get37 = jjlast_type;
            _get38 = i;
            f64_store(0, 4, 278528, _get38);
            _get39 = ijjtype;
            i32_store8(0, 12, 278528, _get39);
            i32_store(1, 0, 278528, 1);
            const struct ReturnValue _6 = dong_porf_porf_todo_String(0, 0, 0, 0, 278528, 72);
            jjlast_type = _6.type;
            _get40 = jjlast_type;
            const struct ReturnValue _7 = dong_porf_porf_todo__Porffor_concatStrings(_5.value, _get37, _6.value, _get40);
            jjlast_type = _7.type;
            _get41 = jjlast_type;
            const struct ReturnValue _8 = dong_porf_porf_todo__Porffor_concatStrings(_7.value, _get41, 2678, 195);
            jjlast_type = _8.type;
            _get42 = checkBorder;
            const struct ReturnValue _9 = dong_porf_porf_todo__Porffor_strcat((u32)(_8.value), 195, (u32)(_get42), 195);
            jjlast_type = _9.type;
            const struct ReturnValue _10 = dong_porf_porf_todo__Porffor_strcat(_9.value, 195, 2741, 195);
            jjlast_type = _10.type;
            _get43 = checkBg;
            const struct ReturnValue _11 = dong_porf_porf_todo__Porffor_strcat(_10.value, 195, (u32)(_get43), 195);
            jjlast_type = _11.type;
            const struct ReturnValue _12 = dong_porf_porf_todo__Porffor_strcat(_11.value, 195, 2759, 195);
            jjlast_type = _12.type;
            _get44 = checkMark;
            const struct ReturnValue _13 = dong_porf_porf_todo__Porffor_strcat(_12.value, 195, (u32)(_get44), 195);
            jjlast_type = _13.type;
            const struct ReturnValue _14 = dong_porf_porf_todo__Porffor_strcat(_13.value, 195, 2899, 195);
            jjlast_type = _14.type;
            html = (f64)(_14.value);
            _get45 = html;
            htmljjtype = 195;
            (void) _get45;
            _get46 = html;
            const struct ReturnValue _15 = dong_porf_porf_todo__Porffor_strcat((u32)(_get46), 195, 2911, 195);
            jjlast_type = _15.type;
            _get47 = color;
            const struct ReturnValue _16 = dong_porf_porf_todo__Porffor_strcat(_15.value, 195, (u32)(_get47), 195);
            jjlast_type = _16.type;
            const struct ReturnValue _17 = dong_porf_porf_todo__Porffor_strcat(_16.value, 195, 2958, 195);
            jjlast_type = _17.type;
            _get48 = strike;
            const struct ReturnValue _18 = dong_porf_porf_todo__Porffor_strcat(_17.value, 195, (u32)(_get48), 195);
            jjlast_type = _18.type;
            const struct ReturnValue _19 = dong_porf_porf_todo__Porffor_strcat(_18.value, 195, 2981, 195);
            jjlast_type = _19.type;
            _get49 = text;
            _get50 = textjjtype;
            const struct ReturnValue _20 = dong_porf_porf_todo__Porffor_concatStrings((f64)(_19.value), 195, _get49, _get50);
            jjlast_type = _20.type;
            _get51 = jjlast_type;
            const struct ReturnValue _21 = dong_porf_porf_todo__Porffor_concatStrings(_20.value, _get51, 2990, 195);
            jjlast_type = _21.type;
            _get52 = jjlast_type;
            htmljjtype = _get52;
            html = _21.value;
            _get53 = html;
            _get54 = htmljjtype;
            const struct ReturnValue _22 = dong_porf_porf_todo__Porffor_concatStrings(_get53, _get54, 3003, 195);
            jjlast_type = _22.type;
            _get55 = jjlast_type;
            _get56 = i;
            f64_store(0, 4, 294912, _get56);
            _get57 = ijjtype;
            i32_store8(0, 12, 294912, _get57);
            i32_store(1, 0, 294912, 1);
            const struct ReturnValue _23 = dong_porf_porf_todo_String(0, 0, 0, 0, 294912, 72);
            jjlast_type = _23.type;
            _get58 = jjlast_type;
            const struct ReturnValue _24 = dong_porf_porf_todo__Porffor_concatStrings(_22.value, _get55, _23.value, _get58);
            jjlast_type = _24.type;
            _get59 = jjlast_type;
            const struct ReturnValue _25 = dong_porf_porf_todo__Porffor_concatStrings(_24.value, _get59, 3041, 195);
            jjlast_type = _25.type;
            _get60 = jjlast_type;
            htmljjtype = _get60;
            html = _25.value;
            _get61 = html;
            _get62 = htmljjtype;
            const struct ReturnValue _26 = dong_porf_porf_todo__Porffor_concatStrings(_get61, _get62, 2899, 195);
            jjlast_type = _26.type;
            _get63 = jjlast_type;
            htmljjtype = _get63;
            html = _26.value;
          }
        // end
        j796:;
        _get64 = i;
        i = _get64 + 1;
        _get65 = i;
        ijjtype = 1;
        (void) _get65;
        goto j771;
      }
    // end
    j772:;
  // end
  _get66 = html;
  const struct ReturnValue _27 = dong_porf_porf_todo_setInnerHTML(0, 0, 0, 0, dong_porf_porf_todo_todoListId, 1, _get66, 195);
  jjlast_type = _27.type;
  (void) _27.value;
  _get67 = jjnewtarget;
  // if 
    if (((u32)(_get67)) != 0) {
      _get68 = jjthis;
      _get69 = jjthisjjtype;
      return (struct ReturnValue){ _get68, _get69 };
    }
  // end
  j896:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_porfRefresh(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_porfPatchFilters(0, 0, 0, 0);
  (void) _0.type;
  (void) _0.value;
  const struct ReturnValue _1 = dong_porf_porf_todo_porfRebuildList(0, 0, 0, 0);
  (void) _1.type;
  (void) _1.value;
  _get0 = jjnewtarget;
  // if 
    if (((u32)(_get0)) != 0) {
      _get1 = jjthis;
      _get2 = jjthisjjtype;
      return (struct ReturnValue){ _get1, _get2 };
    }
  // end
  j897:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_dongLog(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 msg, i32 msgjjtype) {
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = msg;
  _get1 = msgjjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get0, _get1);
  (void) _0.type;
  __porf_import_dong_print(_0.value);
  _get2 = jjnewtarget;
  // if 
    if (((u32)(_get2)) != 0) {
      _get3 = jjthis;
      _get4 = jjthisjjtype;
      return (struct ReturnValue){ _get3, _get4 };
    }
  // end
  j898:;
  return (struct ReturnValue){ 0, 0 };
}

int dong_porf_porf_todo_main() {
  i32 _get8;
  i32 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  dong_porf_porf_todo__porf_init();

  i32 jjlast_type = 0;

  dong_porf_porf_todo_METRIC_OFFSET_WIDTH = 0;
  dong_porf_porf_todo_METRIC_OFFSET_WIDTHjjtype = 1;
  dong_porf_porf_todo_METRIC_OFFSET_HEIGHT = 1;
  dong_porf_porf_todo_METRIC_OFFSET_HEIGHTjjtype = 1;
  dong_porf_porf_todo_METRIC_OFFSET_TOP = 2;
  dong_porf_porf_todo_METRIC_OFFSET_TOPjjtype = 1;
  dong_porf_porf_todo_METRIC_OFFSET_LEFT = 3;
  dong_porf_porf_todo_METRIC_OFFSET_LEFTjjtype = 1;
  dong_porf_porf_todo_METRIC_CLIENT_WIDTH = 4;
  dong_porf_porf_todo_METRIC_CLIENT_WIDTHjjtype = 1;
  dong_porf_porf_todo_METRIC_CLIENT_HEIGHT = 5;
  dong_porf_porf_todo_METRIC_CLIENT_HEIGHTjjtype = 1;
  dong_porf_porf_todo_METRIC_SCROLL_WIDTH = 6;
  dong_porf_porf_todo_METRIC_SCROLL_WIDTHjjtype = 1;
  dong_porf_porf_todo_METRIC_SCROLL_HEIGHT = 7;
  dong_porf_porf_todo_METRIC_SCROLL_HEIGHTjjtype = 1;
  dong_porf_porf_todo_MAX_TODOS = 16;
  dong_porf_porf_todo_MAX_TODOSjjtype = 1;
  dong_porf_porf_todo_todoCount = 4;
  dong_porf_porf_todo_todoCountjjtype = 1;
  dong_porf_porf_todo_nextId = 5;
  dong_porf_porf_todo_nextIdjjtype = 1;
  dong_porf_porf_todo_filterMode = 0;
  dong_porf_porf_todo_filterModejjtype = 1;
  dong_porf_porf_todo_inputText = 0;
  dong_porf_porf_todo_inputTextjjtype = 195;
  dong_porf_porf_todo_todoId0 = 1;
  dong_porf_porf_todo_todoId0jjtype = 1;
  dong_porf_porf_todo_todoText0 = 16;
  dong_porf_porf_todo_todoText0jjtype = 195;
  dong_porf_porf_todo_todoDone0 = 1;
  dong_porf_porf_todo_todoDone0jjtype = 1;
  dong_porf_porf_todo_todoId1 = 2;
  dong_porf_porf_todo_todoId1jjtype = 1;
  dong_porf_porf_todo_todoText1 = 52;
  dong_porf_porf_todo_todoText1jjtype = 195;
  dong_porf_porf_todo_todoDone1 = 1;
  dong_porf_porf_todo_todoDone1jjtype = 1;
  dong_porf_porf_todo_todoId2 = 3;
  dong_porf_porf_todo_todoId2jjtype = 1;
  dong_porf_porf_todo_todoText2 = 88;
  dong_porf_porf_todo_todoText2jjtype = 195;
  dong_porf_porf_todo_todoDone2 = 0;
  dong_porf_porf_todo_todoDone2jjtype = 1;
  dong_porf_porf_todo_todoId3 = 4;
  dong_porf_porf_todo_todoId3jjtype = 1;
  dong_porf_porf_todo_todoText3 = 128;
  dong_porf_porf_todo_todoText3jjtype = 195;
  dong_porf_porf_todo_todoDone3 = 0;
  dong_porf_porf_todo_todoDone3jjtype = 1;
  dong_porf_porf_todo_todoInputId = 0;
  dong_porf_porf_todo_todoInputIdjjtype = 1;
  dong_porf_porf_todo_btnAddId = 0;
  dong_porf_porf_todo_btnAddIdjjtype = 1;
  dong_porf_porf_todo_filterAllId = 0;
  dong_porf_porf_todo_filterAllIdjjtype = 1;
  dong_porf_porf_todo_filterActiveId = 0;
  dong_porf_porf_todo_filterActiveIdjjtype = 1;
  dong_porf_porf_todo_filterDoneId = 0;
  dong_porf_porf_todo_filterDoneIdjjtype = 1;
  dong_porf_porf_todo_todoListId = 0;
  dong_porf_porf_todo_todoListIdjjtype = 1;
  dong_porf_porf_todo_clearWrapId = 0;
  dong_porf_porf_todo_clearWrapIdjjtype = 1;
  dong_porf_porf_todo_btnClearId = 0;
  dong_porf_porf_todo_btnClearIdjjtype = 1;
  dong_porf_porf_todo_todoId4 = 0;
  dong_porf_porf_todo_todoId4jjtype = 1;
  dong_porf_porf_todo_todoText4 = 0;
  dong_porf_porf_todo_todoText4jjtype = 195;
  dong_porf_porf_todo_todoDone4 = 0;
  dong_porf_porf_todo_todoDone4jjtype = 1;
  dong_porf_porf_todo_todoId5 = 0;
  dong_porf_porf_todo_todoId5jjtype = 1;
  dong_porf_porf_todo_todoText5 = 0;
  dong_porf_porf_todo_todoText5jjtype = 195;
  dong_porf_porf_todo_todoDone5 = 0;
  dong_porf_porf_todo_todoDone5jjtype = 1;
  dong_porf_porf_todo_todoId6 = 0;
  dong_porf_porf_todo_todoId6jjtype = 1;
  dong_porf_porf_todo_todoText6 = 0;
  dong_porf_porf_todo_todoText6jjtype = 195;
  dong_porf_porf_todo_todoDone6 = 0;
  dong_porf_porf_todo_todoDone6jjtype = 1;
  dong_porf_porf_todo_todoId7 = 0;
  dong_porf_porf_todo_todoId7jjtype = 1;
  dong_porf_porf_todo_todoText7 = 0;
  dong_porf_porf_todo_todoText7jjtype = 195;
  dong_porf_porf_todo_todoDone7 = 0;
  dong_porf_porf_todo_todoDone7jjtype = 1;
  dong_porf_porf_todo_todoId8 = 0;
  dong_porf_porf_todo_todoId8jjtype = 1;
  dong_porf_porf_todo_todoText8 = 0;
  dong_porf_porf_todo_todoText8jjtype = 195;
  dong_porf_porf_todo_todoDone8 = 0;
  dong_porf_porf_todo_todoDone8jjtype = 1;
  dong_porf_porf_todo_todoId9 = 0;
  dong_porf_porf_todo_todoId9jjtype = 1;
  dong_porf_porf_todo_todoText9 = 0;
  dong_porf_porf_todo_todoText9jjtype = 195;
  dong_porf_porf_todo_todoDone9 = 0;
  dong_porf_porf_todo_todoDone9jjtype = 1;
  dong_porf_porf_todo_todoId10 = 0;
  dong_porf_porf_todo_todoId10jjtype = 1;
  dong_porf_porf_todo_todoText10 = 0;
  dong_porf_porf_todo_todoText10jjtype = 195;
  dong_porf_porf_todo_todoDone10 = 0;
  dong_porf_porf_todo_todoDone10jjtype = 1;
  dong_porf_porf_todo_todoId11 = 0;
  dong_porf_porf_todo_todoId11jjtype = 1;
  dong_porf_porf_todo_todoText11 = 0;
  dong_porf_porf_todo_todoText11jjtype = 195;
  dong_porf_porf_todo_todoDone11 = 0;
  dong_porf_porf_todo_todoDone11jjtype = 1;
  dong_porf_porf_todo_todoId12 = 0;
  dong_porf_porf_todo_todoId12jjtype = 1;
  dong_porf_porf_todo_todoText12 = 0;
  dong_porf_porf_todo_todoText12jjtype = 195;
  dong_porf_porf_todo_todoDone12 = 0;
  dong_porf_porf_todo_todoDone12jjtype = 1;
  dong_porf_porf_todo_todoId13 = 0;
  dong_porf_porf_todo_todoId13jjtype = 1;
  dong_porf_porf_todo_todoText13 = 0;
  dong_porf_porf_todo_todoText13jjtype = 195;
  dong_porf_porf_todo_todoDone13 = 0;
  dong_porf_porf_todo_todoDone13jjtype = 1;
  dong_porf_porf_todo_todoId14 = 0;
  dong_porf_porf_todo_todoId14jjtype = 1;
  dong_porf_porf_todo_todoText14 = 0;
  dong_porf_porf_todo_todoText14jjtype = 195;
  dong_porf_porf_todo_todoDone14 = 0;
  dong_porf_porf_todo_todoDone14jjtype = 1;
  dong_porf_porf_todo_todoId15 = 0;
  dong_porf_porf_todo_todoId15jjtype = 1;
  dong_porf_porf_todo_todoText15 = 0;
  dong_porf_porf_todo_todoText15jjtype = 195;
  dong_porf_porf_todo_todoDone15 = 0;
  dong_porf_porf_todo_todoDone15jjtype = 1;
  const struct ReturnValue _0 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 3394, 195);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  dong_porf_porf_todo_todoInputIdjjtype = _get0;
  dong_porf_porf_todo_todoInputId = _0.value;
  const struct ReturnValue _1 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 3410, 195);
  jjlast_type = _1.type;
  _get1 = jjlast_type;
  dong_porf_porf_todo_btnAddIdjjtype = _get1;
  dong_porf_porf_todo_btnAddId = _1.value;
  const struct ReturnValue _2 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 3423, 195);
  jjlast_type = _2.type;
  _get2 = jjlast_type;
  dong_porf_porf_todo_filterAllIdjjtype = _get2;
  dong_porf_porf_todo_filterAllId = _2.value;
  const struct ReturnValue _3 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 3439, 195);
  jjlast_type = _3.type;
  _get3 = jjlast_type;
  dong_porf_porf_todo_filterActiveIdjjtype = _get3;
  dong_porf_porf_todo_filterActiveId = _3.value;
  const struct ReturnValue _4 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 3458, 195);
  jjlast_type = _4.type;
  _get4 = jjlast_type;
  dong_porf_porf_todo_filterDoneIdjjtype = _get4;
  dong_porf_porf_todo_filterDoneId = _4.value;
  const struct ReturnValue _5 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 3475, 195);
  jjlast_type = _5.type;
  _get5 = jjlast_type;
  dong_porf_porf_todo_todoListIdjjtype = _get5;
  dong_porf_porf_todo_todoListId = _5.value;
  const struct ReturnValue _6 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 3490, 195);
  jjlast_type = _6.type;
  _get6 = jjlast_type;
  dong_porf_porf_todo_clearWrapIdjjtype = _get6;
  dong_porf_porf_todo_clearWrapId = _6.value;
  const struct ReturnValue _7 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 3506, 195);
  jjlast_type = _7.type;
  _get7 = jjlast_type;
  dong_porf_porf_todo_btnClearIdjjtype = _get7;
  dong_porf_porf_todo_btnClearId = _7.value;
  const struct ReturnValue _8 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_btnAddId, dong_porf_porf_todo_btnAddIdjjtype, 3521, 195, 3532, 195);
  jjlast_type = _8.type;
  (void) _8.value;
  const struct ReturnValue _9 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_todoInputId, dong_porf_porf_todo_todoInputIdjjtype, 3543, 195, 3554, 195);
  jjlast_type = _9.type;
  (void) _9.value;
  const struct ReturnValue _10 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_todoInputId, dong_porf_porf_todo_todoInputIdjjtype, 3573, 195, 3586, 195);
  jjlast_type = _10.type;
  (void) _10.value;
  const struct ReturnValue _11 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_filterAllId, dong_porf_porf_todo_filterAllIdjjtype, 3521, 195, 3601, 195);
  jjlast_type = _11.type;
  (void) _11.value;
  const struct ReturnValue _12 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_filterActiveId, dong_porf_porf_todo_filterActiveIdjjtype, 3521, 195, 3618, 195);
  jjlast_type = _12.type;
  (void) _12.value;
  const struct ReturnValue _13 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_filterDoneId, dong_porf_porf_todo_filterDoneIdjjtype, 3521, 195, 3638, 195);
  jjlast_type = _13.type;
  (void) _13.value;
  const struct ReturnValue _14 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_btnClearId, dong_porf_porf_todo_btnClearIdjjtype, 3521, 195, 3656, 195);
  jjlast_type = _14.type;
  (void) _14.value;
  const struct ReturnValue _15 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_todoListId, dong_porf_porf_todo_todoListIdjjtype, 3521, 195, 3673, 195);
  jjlast_type = _15.type;
  (void) _15.value;
  const struct ReturnValue _16 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
  jjlast_type = _16.type;
  (void) _16.value;
  const struct ReturnValue _17 = dong_porf_porf_todo_dongLog(0, 0, 0, 0, 3690, 195);
  jjlast_type = _17.type;
  _get8 = jjlast_type;

  return 0;
}

static struct ReturnValue dong_porf_porf_todo_pullHostString(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get294;
  f64 _get293;
  i32 _get292;
  f64 _get291;
  i32 _get290;
  f64 _get289;
  f64 _get288;
  f64 _get287;
  f64 _get286;
  f64 _get285;
  f64 _get284;
  f64 _get283;
  i32 _get282;
  f64 _get281;
  i32 _get280;
  f64 _get279;
  i32 _get278;
  f64 _get277;
  i32 _get276;
  i32 _get275;
  f64 _get274;
  f64 _get273;
  f64 _get272;
  f64 _get271;
  i32 _get270;
  f64 _get269;
  i32 _get268;
  f64 _get267;
  i32 _get266;
  f64 _get265;
  i32 _get264;
  i32 _get263;
  f64 _get262;
  f64 _get261;
  f64 _get260;
  f64 _get259;
  f64 _get258;
  f64 _get257;
  f64 _get256;
  f64 _get255;
  f64 _get254;
  f64 _get253;
  f64 _get252;
  f64 _get251;
  f64 _get250;
  f64 _get249;
  f64 _get248;
  f64 _get247;
  f64 _get246;
  f64 _get245;
  f64 _get244;
  f64 _get243;
  f64 _get242;
  f64 _get241;
  f64 _get240;
  f64 _get239;
  f64 _get238;
  f64 _get237;
  f64 _get236;
  f64 _get235;
  f64 _get234;
  f64 _get233;
  f64 _get232;
  f64 _get231;
  f64 _get230;
  f64 _get229;
  f64 _get228;
  f64 _get227;
  f64 _get226;
  f64 _get225;
  f64 _get224;
  f64 _get223;
  f64 _get222;
  f64 _get221;
  f64 _get220;
  f64 _get219;
  f64 _get218;
  f64 _get217;
  f64 _get216;
  f64 _get215;
  f64 _get214;
  f64 _get213;
  f64 _get212;
  f64 _get211;
  f64 _get210;
  f64 _get209;
  f64 _get208;
  f64 _get207;
  f64 _get206;
  f64 _get205;
  f64 _get204;
  f64 _get203;
  f64 _get202;
  f64 _get201;
  f64 _get200;
  f64 _get199;
  f64 _get198;
  f64 _get197;
  f64 _get196;
  f64 _get195;
  f64 _get194;
  f64 _get193;
  f64 _get192;
  f64 _get191;
  f64 _get190;
  f64 _get189;
  f64 _get188;
  f64 _get187;
  f64 _get186;
  f64 _get185;
  f64 _get184;
  f64 _get183;
  f64 _get182;
  f64 _get181;
  f64 _get180;
  f64 _get179;
  f64 _get178;
  i32 _get177;
  f64 _get176;
  f64 _get175;
  i32 _get174;
  f64 _get173;
  f64 _get172;
  i32 _get171;
  f64 _get170;
  f64 _get169;
  f64 _get168;
  f64 _get167;
  f64 _get166;
  f64 _get165;
  f64 _get164;
  f64 _get163;
  f64 _get162;
  f64 _get161;
  f64 _get160;
  f64 _get159;
  f64 _get158;
  i32 _get157;
  f64 _get156;
  i32 _get155;
  f64 _get154;
  i32 _get153;
  f64 _get152;
  i32 _get151;
  i32 _get150;
  f64 _get149;
  f64 _get148;
  f64 _get147;
  f64 _get146;
  f64 _get145;
  f64 _get144;
  f64 _get143;
  f64 _get142;
  f64 _get141;
  f64 _get140;
  f64 _get139;
  f64 _get138;
  f64 _get137;
  f64 _get136;
  f64 _get135;
  f64 _get134;
  f64 _get133;
  f64 _get132;
  f64 _get131;
  f64 _get130;
  f64 _get129;
  f64 _get128;
  f64 _get127;
  f64 _get126;
  f64 _get125;
  f64 _get124;
  f64 _get123;
  f64 _get122;
  f64 _get121;
  f64 _get120;
  f64 _get119;
  f64 _get118;
  f64 _get117;
  f64 _get116;
  f64 _get115;
  f64 _get114;
  f64 _get113;
  f64 _get112;
  f64 _get111;
  f64 _get110;
  f64 _get109;
  f64 _get108;
  f64 _get107;
  f64 _get106;
  f64 _get105;
  f64 _get104;
  f64 _get103;
  f64 _get102;
  f64 _get101;
  f64 _get100;
  i32 _get99;
  f64 _get98;
  f64 _get97;
  i32 _get96;
  f64 _get95;
  f64 _get94;
  f64 _get93;
  f64 _get92;
  f64 _get91;
  f64 _get90;
  f64 _get89;
  f64 _get88;
  f64 _get87;
  f64 _get86;
  f64 _get85;
  f64 _get84;
  f64 _get83;
  i32 _get82;
  f64 _get81;
  i32 _get80;
  f64 _get79;
  i32 _get78;
  f64 _get77;
  i32 _get76;
  i32 _get75;
  f64 _get74;
  f64 _get73;
  f64 _get72;
  f64 _get71;
  f64 _get70;
  f64 _get69;
  f64 _get68;
  f64 _get67;
  f64 _get66;
  f64 _get65;
  f64 _get64;
  f64 _get63;
  f64 _get62;
  f64 _get61;
  f64 _get60;
  f64 _get59;
  f64 _get58;
  f64 _get57;
  f64 _get56;
  f64 _get55;
  f64 _get54;
  f64 _get53;
  f64 _get52;
  f64 _get51;
  f64 _get50;
  f64 _get49;
  f64 _get48;
  f64 _get47;
  f64 _get46;
  f64 _get45;
  f64 _get44;
  i32 _get43;
  f64 _get42;
  f64 _get41;
  f64 _get40;
  f64 _get39;
  f64 _get38;
  f64 _get37;
  f64 _get36;
  f64 _get35;
  f64 _get34;
  f64 _get33;
  f64 _get32;
  f64 _get31;
  f64 _get30;
  i32 _get29;
  f64 _get28;
  i32 _get27;
  f64 _get26;
  i32 _get25;
  f64 _get24;
  i32 _get23;
  i32 _get22;
  f64 _get21;
  i32 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  i32 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 len = 0;
  i32 lenjjtype = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;
  f64 out = 0;
  i32 outjjtype = 0;
  f64 i = 0;
  i32 ijjtype = 0;
  f64 b0 = 0;
  i32 b0jjtype = 0;
  f64 b1 = 0;
  i32 b1jjtype = 0;
  f64 b2 = 0;
  i32 b2jjtype = 0;
  f64 b3 = 0;
  i32 b3jjtype = 0;
  f64 cp = 0;
  i32 cpjjtype = 0;
  i32 jjlast_type = 0;
  f64 __tmpop_left = 0;
  f64 __tmpop_right = 0;
  f64 jjbitwise_left = 0;
  f64 jjbitwise_right = 0;

  len = __porf_import_dong_str_len();
  _get0 = jjlast_type;
  lenjjtype = _get0;
  _get1 = len;
  _get2 = lenjjtype;
  // if 
    if ((f64)((_get1 == 0) & ((_get2 | 128) == (1 | 128))) != 0) {
      jjreturn = 0;
      jjreturnjjtype = 195;
      _get3 = jjnewtarget;
      // if 
        if (((u32)(_get3)) != 0) {
          _get4 = jjreturn;
          _get5 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get4), _get5)) == 0) {
              _get6 = jjthis;
              _get7 = jjthisjjtype;
              return (struct ReturnValue){ _get6, _get7 };
            }
          // end
          j901:;
        }
      // end
      j900:;
      _get8 = jjreturn;
      _get9 = jjreturnjjtype;
      return (struct ReturnValue){ _get8, _get9 };
    }
  // end
  j899:;
  out = 0;
  outjjtype = 195;
  i = 0;
  ijjtype = 1;
  b0 = 0;
  b0jjtype = 1;
  b1 = 0;
  b1jjtype = 1;
  b2 = 0;
  b2jjtype = 1;
  b3 = 0;
  b3jjtype = 1;
  cp = 0;
  cpjjtype = 1;
  // loop 
  j902:;
    _get10 = i;
    _get11 = len;
    // if 
      if (_get10 < _get11) {
        _get12 = i;
        b0 = __porf_import_dong_str_byte_at(_get12);
        _get13 = b0;
        _get14 = jjlast_type;
        b0jjtype = _get14;
        (void) _get13;
        _get15 = b0;
        // if 
          if ((f64)(_get15 < 0) != 0) {
            goto j903;
          }
        // end
        j904:;
        _get16 = b0;
        // if 
          if ((f64)(_get16 < 128) != 0) {
            // block f64
            f64 _r906;
              _get17 = out;
              __tmpop_left = _get17;
              _get18 = __tmpop_left;
              _get19 = b0;
              f64_store(0, 4, 65536, _get19);
              _get20 = b0jjtype;
              i32_store8(0, 12, 65536, _get20);
              i32_store(1, 0, 65536, 1);
              const struct ReturnValue _0 = dong_porf_porf_todo__String_fromCharCode(65536, 72);
              jjlast_type = _0.type;
              __tmpop_right = _0.value;
              _get21 = __tmpop_right;
              _get22 = outjjtype;
              _get23 = jjlast_type;
              // if 
                if ((((_get22 | 128) == 195) | ((_get23 | 128) == 195)) != 0) {
                  _get24 = __tmpop_left;
                  _get25 = outjjtype;
                  _get26 = __tmpop_right;
                  _get27 = jjlast_type;
                  const struct ReturnValue _1 = dong_porf_porf_todo__Porffor_concatStrings(_get24, _get25, _get26, _get27);
                  jjlast_type = _1.type;
                  _r906 = _1.value;
                  goto j906;
                }
              // end
              j907:;
              jjlast_type = 1;
              _r906 = _get18 + _get21;
            // end
            j906:;
            out = _r906;
            _get28 = out;
            _get29 = jjlast_type;
            outjjtype = _get29;
            (void) _get28;
            _get30 = i;
            i = _get30 + 1;
            _get31 = i;
            ijjtype = 1;
            (void) _get31;
          } else {
            _get32 = b0;
            jjbitwise_left = _get32;
            _get33 = jjbitwise_left;
            _get34 = jjbitwise_left;
            // if i32
            i32 _r908;
              if ((_get33 - _get34) == 0) {
                _get35 = jjbitwise_left;
                _r908 = (i32)((i64)(_get35));
              } else {
                _r908 = 0;
              }
            // end
            j908:;
            jjbitwise_right = 224;
            _get36 = jjbitwise_right;
            _get37 = jjbitwise_right;
            // if i32
            i32 _r909;
              if ((_get36 - _get37) == 0) {
                _get38 = jjbitwise_right;
                _r909 = (i32)((i64)(_get38));
              } else {
                _r909 = 0;
              }
            // end
            j909:;
            // if 
              if ((f64)((f64)(_r908 & _r909) == 192) != 0) {
                _get39 = i;
                _get40 = len;
                // if 
                  if ((f64)((_get39 + 1) >= _get40) != 0) {
                    goto j903;
                  }
                // end
                j911:;
                _get41 = i;
                b1 = __porf_import_dong_str_byte_at(_get41 + 1);
                _get42 = b1;
                _get43 = jjlast_type;
                b1jjtype = _get43;
                (void) _get42;
                _get44 = b0;
                jjbitwise_left = _get44;
                _get45 = jjbitwise_left;
                _get46 = jjbitwise_left;
                // if i32
                i32 _r912;
                  if ((_get45 - _get46) == 0) {
                    _get47 = jjbitwise_left;
                    _r912 = (i32)((i64)(_get47));
                  } else {
                    _r912 = 0;
                  }
                // end
                j912:;
                jjbitwise_right = 31;
                _get48 = jjbitwise_right;
                _get49 = jjbitwise_right;
                // if i32
                i32 _r913;
                  if ((_get48 - _get49) == 0) {
                    _get50 = jjbitwise_right;
                    _r913 = (i32)((i64)(_get50));
                  } else {
                    _r913 = 0;
                  }
                // end
                j913:;
                jjbitwise_left = (f64)(_r912 & _r913);
                _get51 = jjbitwise_left;
                _get52 = jjbitwise_left;
                // if i32
                i32 _r914;
                  if ((_get51 - _get52) == 0) {
                    _get53 = jjbitwise_left;
                    _r914 = (i32)((i64)(_get53));
                  } else {
                    _r914 = 0;
                  }
                // end
                j914:;
                jjbitwise_right = 6;
                _get54 = jjbitwise_right;
                _get55 = jjbitwise_right;
                // if i32
                i32 _r915;
                  if ((_get54 - _get55) == 0) {
                    _get56 = jjbitwise_right;
                    _r915 = (i32)((i64)(_get56));
                  } else {
                    _r915 = 0;
                  }
                // end
                j915:;
                jjbitwise_left = (f64)(_r914 << _r915);
                _get57 = jjbitwise_left;
                _get58 = jjbitwise_left;
                // if i32
                i32 _r916;
                  if ((_get57 - _get58) == 0) {
                    _get59 = jjbitwise_left;
                    _r916 = (i32)((i64)(_get59));
                  } else {
                    _r916 = 0;
                  }
                // end
                j916:;
                _get60 = b1;
                jjbitwise_left = _get60;
                _get61 = jjbitwise_left;
                _get62 = jjbitwise_left;
                // if i32
                i32 _r917;
                  if ((_get61 - _get62) == 0) {
                    _get63 = jjbitwise_left;
                    _r917 = (i32)((i64)(_get63));
                  } else {
                    _r917 = 0;
                  }
                // end
                j917:;
                jjbitwise_right = 63;
                _get64 = jjbitwise_right;
                _get65 = jjbitwise_right;
                // if i32
                i32 _r918;
                  if ((_get64 - _get65) == 0) {
                    _get66 = jjbitwise_right;
                    _r918 = (i32)((i64)(_get66));
                  } else {
                    _r918 = 0;
                  }
                // end
                j918:;
                jjbitwise_right = (f64)(_r917 & _r918);
                _get67 = jjbitwise_right;
                _get68 = jjbitwise_right;
                // if i32
                i32 _r919;
                  if ((_get67 - _get68) == 0) {
                    _get69 = jjbitwise_right;
                    _r919 = (i32)((i64)(_get69));
                  } else {
                    _r919 = 0;
                  }
                // end
                j919:;
                cp = (f64)(_r916 | _r919);
                _get70 = cp;
                cpjjtype = 1;
                (void) _get70;
                // block f64
                f64 _r920;
                  _get71 = out;
                  __tmpop_left = _get71;
                  _get72 = __tmpop_left;
                  _get73 = cp;
                  f64_store(0, 4, 81920, _get73);
                  i32_store8(0, 12, 81920, 1);
                  i32_store(1, 0, 81920, 1);
                  const struct ReturnValue _2 = dong_porf_porf_todo__String_fromCharCode(81920, 72);
                  jjlast_type = _2.type;
                  __tmpop_right = _2.value;
                  _get74 = __tmpop_right;
                  _get75 = outjjtype;
                  _get76 = jjlast_type;
                  // if 
                    if ((((_get75 | 128) == 195) | ((_get76 | 128) == 195)) != 0) {
                      _get77 = __tmpop_left;
                      _get78 = outjjtype;
                      _get79 = __tmpop_right;
                      _get80 = jjlast_type;
                      const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_concatStrings(_get77, _get78, _get79, _get80);
                      jjlast_type = _3.type;
                      _r920 = _3.value;
                      goto j920;
                    }
                  // end
                  j921:;
                  jjlast_type = 1;
                  _r920 = _get72 + _get74;
                // end
                j920:;
                out = _r920;
                _get81 = out;
                _get82 = jjlast_type;
                outjjtype = _get82;
                (void) _get81;
                _get83 = i;
                i = _get83 + 2;
                _get84 = i;
                ijjtype = 1;
                (void) _get84;
              } else {
                _get85 = b0;
                jjbitwise_left = _get85;
                _get86 = jjbitwise_left;
                _get87 = jjbitwise_left;
                // if i32
                i32 _r922;
                  if ((_get86 - _get87) == 0) {
                    _get88 = jjbitwise_left;
                    _r922 = (i32)((i64)(_get88));
                  } else {
                    _r922 = 0;
                  }
                // end
                j922:;
                jjbitwise_right = 240;
                _get89 = jjbitwise_right;
                _get90 = jjbitwise_right;
                // if i32
                i32 _r923;
                  if ((_get89 - _get90) == 0) {
                    _get91 = jjbitwise_right;
                    _r923 = (i32)((i64)(_get91));
                  } else {
                    _r923 = 0;
                  }
                // end
                j923:;
                // if 
                  if ((f64)((f64)(_r922 & _r923) == 224) != 0) {
                    _get92 = i;
                    _get93 = len;
                    // if 
                      if ((f64)((_get92 + 2) >= _get93) != 0) {
                        goto j903;
                      }
                    // end
                    j925:;
                    _get94 = i;
                    b1 = __porf_import_dong_str_byte_at(_get94 + 1);
                    _get95 = b1;
                    _get96 = jjlast_type;
                    b1jjtype = _get96;
                    (void) _get95;
                    _get97 = i;
                    b2 = __porf_import_dong_str_byte_at(_get97 + 2);
                    _get98 = b2;
                    _get99 = jjlast_type;
                    b2jjtype = _get99;
                    (void) _get98;
                    _get100 = b0;
                    jjbitwise_left = _get100;
                    _get101 = jjbitwise_left;
                    _get102 = jjbitwise_left;
                    // if i32
                    i32 _r926;
                      if ((_get101 - _get102) == 0) {
                        _get103 = jjbitwise_left;
                        _r926 = (i32)((i64)(_get103));
                      } else {
                        _r926 = 0;
                      }
                    // end
                    j926:;
                    jjbitwise_right = 15;
                    _get104 = jjbitwise_right;
                    _get105 = jjbitwise_right;
                    // if i32
                    i32 _r927;
                      if ((_get104 - _get105) == 0) {
                        _get106 = jjbitwise_right;
                        _r927 = (i32)((i64)(_get106));
                      } else {
                        _r927 = 0;
                      }
                    // end
                    j927:;
                    jjbitwise_left = (f64)(_r926 & _r927);
                    _get107 = jjbitwise_left;
                    _get108 = jjbitwise_left;
                    // if i32
                    i32 _r928;
                      if ((_get107 - _get108) == 0) {
                        _get109 = jjbitwise_left;
                        _r928 = (i32)((i64)(_get109));
                      } else {
                        _r928 = 0;
                      }
                    // end
                    j928:;
                    jjbitwise_right = 12;
                    _get110 = jjbitwise_right;
                    _get111 = jjbitwise_right;
                    // if i32
                    i32 _r929;
                      if ((_get110 - _get111) == 0) {
                        _get112 = jjbitwise_right;
                        _r929 = (i32)((i64)(_get112));
                      } else {
                        _r929 = 0;
                      }
                    // end
                    j929:;
                    jjbitwise_left = (f64)(_r928 << _r929);
                    _get113 = jjbitwise_left;
                    _get114 = jjbitwise_left;
                    // if i32
                    i32 _r930;
                      if ((_get113 - _get114) == 0) {
                        _get115 = jjbitwise_left;
                        _r930 = (i32)((i64)(_get115));
                      } else {
                        _r930 = 0;
                      }
                    // end
                    j930:;
                    _get116 = b1;
                    jjbitwise_left = _get116;
                    _get117 = jjbitwise_left;
                    _get118 = jjbitwise_left;
                    // if i32
                    i32 _r931;
                      if ((_get117 - _get118) == 0) {
                        _get119 = jjbitwise_left;
                        _r931 = (i32)((i64)(_get119));
                      } else {
                        _r931 = 0;
                      }
                    // end
                    j931:;
                    jjbitwise_right = 63;
                    _get120 = jjbitwise_right;
                    _get121 = jjbitwise_right;
                    // if i32
                    i32 _r932;
                      if ((_get120 - _get121) == 0) {
                        _get122 = jjbitwise_right;
                        _r932 = (i32)((i64)(_get122));
                      } else {
                        _r932 = 0;
                      }
                    // end
                    j932:;
                    jjbitwise_left = (f64)(_r931 & _r932);
                    _get123 = jjbitwise_left;
                    _get124 = jjbitwise_left;
                    // if i32
                    i32 _r933;
                      if ((_get123 - _get124) == 0) {
                        _get125 = jjbitwise_left;
                        _r933 = (i32)((i64)(_get125));
                      } else {
                        _r933 = 0;
                      }
                    // end
                    j933:;
                    jjbitwise_right = 6;
                    _get126 = jjbitwise_right;
                    _get127 = jjbitwise_right;
                    // if i32
                    i32 _r934;
                      if ((_get126 - _get127) == 0) {
                        _get128 = jjbitwise_right;
                        _r934 = (i32)((i64)(_get128));
                      } else {
                        _r934 = 0;
                      }
                    // end
                    j934:;
                    jjbitwise_right = (f64)(_r933 << _r934);
                    _get129 = jjbitwise_right;
                    _get130 = jjbitwise_right;
                    // if i32
                    i32 _r935;
                      if ((_get129 - _get130) == 0) {
                        _get131 = jjbitwise_right;
                        _r935 = (i32)((i64)(_get131));
                      } else {
                        _r935 = 0;
                      }
                    // end
                    j935:;
                    jjbitwise_left = (f64)(_r930 | _r935);
                    _get132 = jjbitwise_left;
                    _get133 = jjbitwise_left;
                    // if i32
                    i32 _r936;
                      if ((_get132 - _get133) == 0) {
                        _get134 = jjbitwise_left;
                        _r936 = (i32)((i64)(_get134));
                      } else {
                        _r936 = 0;
                      }
                    // end
                    j936:;
                    _get135 = b2;
                    jjbitwise_left = _get135;
                    _get136 = jjbitwise_left;
                    _get137 = jjbitwise_left;
                    // if i32
                    i32 _r937;
                      if ((_get136 - _get137) == 0) {
                        _get138 = jjbitwise_left;
                        _r937 = (i32)((i64)(_get138));
                      } else {
                        _r937 = 0;
                      }
                    // end
                    j937:;
                    jjbitwise_right = 63;
                    _get139 = jjbitwise_right;
                    _get140 = jjbitwise_right;
                    // if i32
                    i32 _r938;
                      if ((_get139 - _get140) == 0) {
                        _get141 = jjbitwise_right;
                        _r938 = (i32)((i64)(_get141));
                      } else {
                        _r938 = 0;
                      }
                    // end
                    j938:;
                    jjbitwise_right = (f64)(_r937 & _r938);
                    _get142 = jjbitwise_right;
                    _get143 = jjbitwise_right;
                    // if i32
                    i32 _r939;
                      if ((_get142 - _get143) == 0) {
                        _get144 = jjbitwise_right;
                        _r939 = (i32)((i64)(_get144));
                      } else {
                        _r939 = 0;
                      }
                    // end
                    j939:;
                    cp = (f64)(_r936 | _r939);
                    _get145 = cp;
                    cpjjtype = 1;
                    (void) _get145;
                    // block f64
                    f64 _r940;
                      _get146 = out;
                      __tmpop_left = _get146;
                      _get147 = __tmpop_left;
                      _get148 = cp;
                      f64_store(0, 4, 98304, _get148);
                      i32_store8(0, 12, 98304, 1);
                      i32_store(1, 0, 98304, 1);
                      const struct ReturnValue _4 = dong_porf_porf_todo__String_fromCharCode(98304, 72);
                      jjlast_type = _4.type;
                      __tmpop_right = _4.value;
                      _get149 = __tmpop_right;
                      _get150 = outjjtype;
                      _get151 = jjlast_type;
                      // if 
                        if ((((_get150 | 128) == 195) | ((_get151 | 128) == 195)) != 0) {
                          _get152 = __tmpop_left;
                          _get153 = outjjtype;
                          _get154 = __tmpop_right;
                          _get155 = jjlast_type;
                          const struct ReturnValue _5 = dong_porf_porf_todo__Porffor_concatStrings(_get152, _get153, _get154, _get155);
                          jjlast_type = _5.type;
                          _r940 = _5.value;
                          goto j940;
                        }
                      // end
                      j941:;
                      jjlast_type = 1;
                      _r940 = _get147 + _get149;
                    // end
                    j940:;
                    out = _r940;
                    _get156 = out;
                    _get157 = jjlast_type;
                    outjjtype = _get157;
                    (void) _get156;
                    _get158 = i;
                    i = _get158 + 3;
                    _get159 = i;
                    ijjtype = 1;
                    (void) _get159;
                  } else {
                    _get160 = b0;
                    jjbitwise_left = _get160;
                    _get161 = jjbitwise_left;
                    _get162 = jjbitwise_left;
                    // if i32
                    i32 _r942;
                      if ((_get161 - _get162) == 0) {
                        _get163 = jjbitwise_left;
                        _r942 = (i32)((i64)(_get163));
                      } else {
                        _r942 = 0;
                      }
                    // end
                    j942:;
                    jjbitwise_right = 248;
                    _get164 = jjbitwise_right;
                    _get165 = jjbitwise_right;
                    // if i32
                    i32 _r943;
                      if ((_get164 - _get165) == 0) {
                        _get166 = jjbitwise_right;
                        _r943 = (i32)((i64)(_get166));
                      } else {
                        _r943 = 0;
                      }
                    // end
                    j943:;
                    // if 
                      if ((f64)((f64)(_r942 & _r943) == 240) != 0) {
                        _get167 = i;
                        _get168 = len;
                        // if 
                          if ((f64)((_get167 + 3) >= _get168) != 0) {
                            goto j903;
                          }
                        // end
                        j945:;
                        _get169 = i;
                        b1 = __porf_import_dong_str_byte_at(_get169 + 1);
                        _get170 = b1;
                        _get171 = jjlast_type;
                        b1jjtype = _get171;
                        (void) _get170;
                        _get172 = i;
                        b2 = __porf_import_dong_str_byte_at(_get172 + 2);
                        _get173 = b2;
                        _get174 = jjlast_type;
                        b2jjtype = _get174;
                        (void) _get173;
                        _get175 = i;
                        b3 = __porf_import_dong_str_byte_at(_get175 + 3);
                        _get176 = b3;
                        _get177 = jjlast_type;
                        b3jjtype = _get177;
                        (void) _get176;
                        _get178 = b0;
                        jjbitwise_left = _get178;
                        _get179 = jjbitwise_left;
                        _get180 = jjbitwise_left;
                        // if i32
                        i32 _r946;
                          if ((_get179 - _get180) == 0) {
                            _get181 = jjbitwise_left;
                            _r946 = (i32)((i64)(_get181));
                          } else {
                            _r946 = 0;
                          }
                        // end
                        j946:;
                        jjbitwise_right = 7;
                        _get182 = jjbitwise_right;
                        _get183 = jjbitwise_right;
                        // if i32
                        i32 _r947;
                          if ((_get182 - _get183) == 0) {
                            _get184 = jjbitwise_right;
                            _r947 = (i32)((i64)(_get184));
                          } else {
                            _r947 = 0;
                          }
                        // end
                        j947:;
                        jjbitwise_left = (f64)(_r946 & _r947);
                        _get185 = jjbitwise_left;
                        _get186 = jjbitwise_left;
                        // if i32
                        i32 _r948;
                          if ((_get185 - _get186) == 0) {
                            _get187 = jjbitwise_left;
                            _r948 = (i32)((i64)(_get187));
                          } else {
                            _r948 = 0;
                          }
                        // end
                        j948:;
                        jjbitwise_right = 18;
                        _get188 = jjbitwise_right;
                        _get189 = jjbitwise_right;
                        // if i32
                        i32 _r949;
                          if ((_get188 - _get189) == 0) {
                            _get190 = jjbitwise_right;
                            _r949 = (i32)((i64)(_get190));
                          } else {
                            _r949 = 0;
                          }
                        // end
                        j949:;
                        jjbitwise_left = (f64)(_r948 << _r949);
                        _get191 = jjbitwise_left;
                        _get192 = jjbitwise_left;
                        // if i32
                        i32 _r950;
                          if ((_get191 - _get192) == 0) {
                            _get193 = jjbitwise_left;
                            _r950 = (i32)((i64)(_get193));
                          } else {
                            _r950 = 0;
                          }
                        // end
                        j950:;
                        _get194 = b1;
                        jjbitwise_left = _get194;
                        _get195 = jjbitwise_left;
                        _get196 = jjbitwise_left;
                        // if i32
                        i32 _r951;
                          if ((_get195 - _get196) == 0) {
                            _get197 = jjbitwise_left;
                            _r951 = (i32)((i64)(_get197));
                          } else {
                            _r951 = 0;
                          }
                        // end
                        j951:;
                        jjbitwise_right = 63;
                        _get198 = jjbitwise_right;
                        _get199 = jjbitwise_right;
                        // if i32
                        i32 _r952;
                          if ((_get198 - _get199) == 0) {
                            _get200 = jjbitwise_right;
                            _r952 = (i32)((i64)(_get200));
                          } else {
                            _r952 = 0;
                          }
                        // end
                        j952:;
                        jjbitwise_left = (f64)(_r951 & _r952);
                        _get201 = jjbitwise_left;
                        _get202 = jjbitwise_left;
                        // if i32
                        i32 _r953;
                          if ((_get201 - _get202) == 0) {
                            _get203 = jjbitwise_left;
                            _r953 = (i32)((i64)(_get203));
                          } else {
                            _r953 = 0;
                          }
                        // end
                        j953:;
                        jjbitwise_right = 12;
                        _get204 = jjbitwise_right;
                        _get205 = jjbitwise_right;
                        // if i32
                        i32 _r954;
                          if ((_get204 - _get205) == 0) {
                            _get206 = jjbitwise_right;
                            _r954 = (i32)((i64)(_get206));
                          } else {
                            _r954 = 0;
                          }
                        // end
                        j954:;
                        jjbitwise_right = (f64)(_r953 << _r954);
                        _get207 = jjbitwise_right;
                        _get208 = jjbitwise_right;
                        // if i32
                        i32 _r955;
                          if ((_get207 - _get208) == 0) {
                            _get209 = jjbitwise_right;
                            _r955 = (i32)((i64)(_get209));
                          } else {
                            _r955 = 0;
                          }
                        // end
                        j955:;
                        jjbitwise_left = (f64)(_r950 | _r955);
                        _get210 = jjbitwise_left;
                        _get211 = jjbitwise_left;
                        // if i32
                        i32 _r956;
                          if ((_get210 - _get211) == 0) {
                            _get212 = jjbitwise_left;
                            _r956 = (i32)((i64)(_get212));
                          } else {
                            _r956 = 0;
                          }
                        // end
                        j956:;
                        _get213 = b2;
                        jjbitwise_left = _get213;
                        _get214 = jjbitwise_left;
                        _get215 = jjbitwise_left;
                        // if i32
                        i32 _r957;
                          if ((_get214 - _get215) == 0) {
                            _get216 = jjbitwise_left;
                            _r957 = (i32)((i64)(_get216));
                          } else {
                            _r957 = 0;
                          }
                        // end
                        j957:;
                        jjbitwise_right = 63;
                        _get217 = jjbitwise_right;
                        _get218 = jjbitwise_right;
                        // if i32
                        i32 _r958;
                          if ((_get217 - _get218) == 0) {
                            _get219 = jjbitwise_right;
                            _r958 = (i32)((i64)(_get219));
                          } else {
                            _r958 = 0;
                          }
                        // end
                        j958:;
                        jjbitwise_left = (f64)(_r957 & _r958);
                        _get220 = jjbitwise_left;
                        _get221 = jjbitwise_left;
                        // if i32
                        i32 _r959;
                          if ((_get220 - _get221) == 0) {
                            _get222 = jjbitwise_left;
                            _r959 = (i32)((i64)(_get222));
                          } else {
                            _r959 = 0;
                          }
                        // end
                        j959:;
                        jjbitwise_right = 6;
                        _get223 = jjbitwise_right;
                        _get224 = jjbitwise_right;
                        // if i32
                        i32 _r960;
                          if ((_get223 - _get224) == 0) {
                            _get225 = jjbitwise_right;
                            _r960 = (i32)((i64)(_get225));
                          } else {
                            _r960 = 0;
                          }
                        // end
                        j960:;
                        jjbitwise_right = (f64)(_r959 << _r960);
                        _get226 = jjbitwise_right;
                        _get227 = jjbitwise_right;
                        // if i32
                        i32 _r961;
                          if ((_get226 - _get227) == 0) {
                            _get228 = jjbitwise_right;
                            _r961 = (i32)((i64)(_get228));
                          } else {
                            _r961 = 0;
                          }
                        // end
                        j961:;
                        jjbitwise_left = (f64)(_r956 | _r961);
                        _get229 = jjbitwise_left;
                        _get230 = jjbitwise_left;
                        // if i32
                        i32 _r962;
                          if ((_get229 - _get230) == 0) {
                            _get231 = jjbitwise_left;
                            _r962 = (i32)((i64)(_get231));
                          } else {
                            _r962 = 0;
                          }
                        // end
                        j962:;
                        _get232 = b3;
                        jjbitwise_left = _get232;
                        _get233 = jjbitwise_left;
                        _get234 = jjbitwise_left;
                        // if i32
                        i32 _r963;
                          if ((_get233 - _get234) == 0) {
                            _get235 = jjbitwise_left;
                            _r963 = (i32)((i64)(_get235));
                          } else {
                            _r963 = 0;
                          }
                        // end
                        j963:;
                        jjbitwise_right = 63;
                        _get236 = jjbitwise_right;
                        _get237 = jjbitwise_right;
                        // if i32
                        i32 _r964;
                          if ((_get236 - _get237) == 0) {
                            _get238 = jjbitwise_right;
                            _r964 = (i32)((i64)(_get238));
                          } else {
                            _r964 = 0;
                          }
                        // end
                        j964:;
                        jjbitwise_right = (f64)(_r963 & _r964);
                        _get239 = jjbitwise_right;
                        _get240 = jjbitwise_right;
                        // if i32
                        i32 _r965;
                          if ((_get239 - _get240) == 0) {
                            _get241 = jjbitwise_right;
                            _r965 = (i32)((i64)(_get241));
                          } else {
                            _r965 = 0;
                          }
                        // end
                        j965:;
                        cp = (f64)(_r962 | _r965);
                        _get242 = cp;
                        cpjjtype = 1;
                        (void) _get242;
                        _get243 = cp;
                        // if 
                          if ((f64)(_get243 > 65535) != 0) {
                            _get244 = cp;
                            cp = _get244 - 65536;
                            _get245 = cp;
                            cpjjtype = 1;
                            (void) _get245;
                            // block f64
                            f64 _r967;
                              _get246 = out;
                              __tmpop_left = _get246;
                              _get247 = __tmpop_left;
                              _get248 = cp;
                              jjbitwise_left = _get248;
                              _get249 = jjbitwise_left;
                              _get250 = jjbitwise_left;
                              // if i32
                              i32 _r968;
                                if ((_get249 - _get250) == 0) {
                                  _get251 = jjbitwise_left;
                                  _r968 = (i32)((i64)(_get251));
                                } else {
                                  _r968 = 0;
                                }
                              // end
                              j968:;
                              jjbitwise_right = 10;
                              _get252 = jjbitwise_right;
                              _get253 = jjbitwise_right;
                              // if i32
                              i32 _r969;
                                if ((_get252 - _get253) == 0) {
                                  _get254 = jjbitwise_right;
                                  _r969 = (i32)((i64)(_get254));
                                } else {
                                  _r969 = 0;
                                }
                              // end
                              j969:;
                              f64_store(0, 4, 114688, 55296 + (f64)(_r968 >> _r969));
                              i32_store8(0, 12, 114688, 1);
                              _get255 = cp;
                              jjbitwise_left = _get255;
                              _get256 = jjbitwise_left;
                              _get257 = jjbitwise_left;
                              // if i32
                              i32 _r970;
                                if ((_get256 - _get257) == 0) {
                                  _get258 = jjbitwise_left;
                                  _r970 = (i32)((i64)(_get258));
                                } else {
                                  _r970 = 0;
                                }
                              // end
                              j970:;
                              jjbitwise_right = 1023;
                              _get259 = jjbitwise_right;
                              _get260 = jjbitwise_right;
                              // if i32
                              i32 _r971;
                                if ((_get259 - _get260) == 0) {
                                  _get261 = jjbitwise_right;
                                  _r971 = (i32)((i64)(_get261));
                                } else {
                                  _r971 = 0;
                                }
                              // end
                              j971:;
                              f64_store(0, 13, 114688, 56320 + (f64)(_r970 & _r971));
                              i32_store8(0, 21, 114688, 1);
                              i32_store(1, 0, 114688, 2);
                              const struct ReturnValue _6 = dong_porf_porf_todo__String_fromCharCode(114688, 72);
                              jjlast_type = _6.type;
                              __tmpop_right = _6.value;
                              _get262 = __tmpop_right;
                              _get263 = outjjtype;
                              _get264 = jjlast_type;
                              // if 
                                if ((((_get263 | 128) == 195) | ((_get264 | 128) == 195)) != 0) {
                                  _get265 = __tmpop_left;
                                  _get266 = outjjtype;
                                  _get267 = __tmpop_right;
                                  _get268 = jjlast_type;
                                  const struct ReturnValue _7 = dong_porf_porf_todo__Porffor_concatStrings(_get265, _get266, _get267, _get268);
                                  jjlast_type = _7.type;
                                  _r967 = _7.value;
                                  goto j967;
                                }
                              // end
                              j972:;
                              jjlast_type = 1;
                              _r967 = _get247 + _get262;
                            // end
                            j967:;
                            out = _r967;
                            _get269 = out;
                            _get270 = jjlast_type;
                            outjjtype = _get270;
                            (void) _get269;
                          } else {
                            // block f64
                            f64 _r973;
                              _get271 = out;
                              __tmpop_left = _get271;
                              _get272 = __tmpop_left;
                              _get273 = cp;
                              f64_store(0, 4, 131072, _get273);
                              i32_store8(0, 12, 131072, 1);
                              i32_store(1, 0, 131072, 1);
                              const struct ReturnValue _8 = dong_porf_porf_todo__String_fromCharCode(131072, 72);
                              jjlast_type = _8.type;
                              __tmpop_right = _8.value;
                              _get274 = __tmpop_right;
                              _get275 = outjjtype;
                              _get276 = jjlast_type;
                              // if 
                                if ((((_get275 | 128) == 195) | ((_get276 | 128) == 195)) != 0) {
                                  _get277 = __tmpop_left;
                                  _get278 = outjjtype;
                                  _get279 = __tmpop_right;
                                  _get280 = jjlast_type;
                                  const struct ReturnValue _9 = dong_porf_porf_todo__Porffor_concatStrings(_get277, _get278, _get279, _get280);
                                  jjlast_type = _9.type;
                                  _r973 = _9.value;
                                  goto j973;
                                }
                              // end
                              j974:;
                              jjlast_type = 1;
                              _r973 = _get272 + _get274;
                            // end
                            j973:;
                            out = _r973;
                            _get281 = out;
                            _get282 = jjlast_type;
                            outjjtype = _get282;
                            (void) _get281;
                          }
                        // end
                        j966:;
                        _get283 = i;
                        i = _get283 + 4;
                        _get284 = i;
                        ijjtype = 1;
                        (void) _get284;
                      } else {
                        _get285 = i;
                        i = _get285 + 1;
                        _get286 = i;
                        ijjtype = 1;
                        (void) _get286;
                      }
                    // end
                    j944:;
                  }
                // end
                j924:;
              }
            // end
            j910:;
          }
        // end
        j905:;
        goto j902;
      }
    // end
    j903:;
  // end
  _get287 = out;
  jjreturn = _get287;
  jjreturnjjtype = 195;
  _get288 = jjnewtarget;
  // if 
    if (((u32)(_get288)) != 0) {
      _get289 = jjreturn;
      _get290 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get289), _get290)) == 0) {
          _get291 = jjthis;
          _get292 = jjthisjjtype;
          return (struct ReturnValue){ _get291, _get292 };
        }
      // end
      j976:;
    }
  // end
  j975:;
  _get293 = jjreturn;
  _get294 = jjreturnjjtype;
  return (struct ReturnValue){ _get293, _get294 };
}

static struct ReturnValue dong_porf_porf_todo_getValue(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype) {
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  i32 jjlast_type = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = nodeId;
  __porf_import_dong_get_value(_get0);
  const struct ReturnValue _0 = dong_porf_porf_todo_pullHostString(0, 0, 0, 0);
  jjlast_type = _0.type;
  jjreturn = _0.value;
  _get1 = jjlast_type;
  jjreturnjjtype = _get1;
  _get2 = jjnewtarget;
  // if 
    if (((u32)(_get2)) != 0) {
      _get3 = jjreturn;
      _get4 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get3), _get4)) == 0) {
          _get5 = jjthis;
          _get6 = jjthisjjtype;
          return (struct ReturnValue){ _get5, _get6 };
        }
      // end
      j978:;
    }
  // end
  j977:;
  _get7 = jjreturn;
  _get8 = jjreturnjjtype;
  return (struct ReturnValue){ _get7, _get8 };
}

static struct ReturnValue dong_porf_porf_todo_setTodoSlot(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype, f64 id, i32 idjjtype, f64 text, i32 textjjtype, f64 done, i32 donejjtype) {
  i32 _get173;
  f64 _get172;
  f64 _get171;
  i32 _get170;
  f64 _get169;
  i32 _get168;
  f64 _get167;
  i32 _get166;
  f64 _get165;
  i32 _get164;
  f64 _get163;
  f64 _get162;
  i32 _get161;
  f64 _get160;
  i32 _get159;
  f64 _get158;
  i32 _get157;
  f64 _get156;
  i32 _get155;
  f64 _get154;
  i32 _get153;
  f64 _get152;
  f64 _get151;
  i32 _get150;
  f64 _get149;
  i32 _get148;
  f64 _get147;
  i32 _get146;
  f64 _get145;
  i32 _get144;
  f64 _get143;
  i32 _get142;
  f64 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  i32 _get137;
  f64 _get136;
  i32 _get135;
  f64 _get134;
  i32 _get133;
  f64 _get132;
  i32 _get131;
  f64 _get130;
  f64 _get129;
  i32 _get128;
  f64 _get127;
  i32 _get126;
  f64 _get125;
  i32 _get124;
  f64 _get123;
  i32 _get122;
  f64 _get121;
  i32 _get120;
  f64 _get119;
  f64 _get118;
  i32 _get117;
  f64 _get116;
  i32 _get115;
  f64 _get114;
  i32 _get113;
  f64 _get112;
  i32 _get111;
  f64 _get110;
  i32 _get109;
  f64 _get108;
  f64 _get107;
  i32 _get106;
  f64 _get105;
  i32 _get104;
  f64 _get103;
  i32 _get102;
  f64 _get101;
  i32 _get100;
  f64 _get99;
  i32 _get98;
  f64 _get97;
  f64 _get96;
  i32 _get95;
  f64 _get94;
  i32 _get93;
  f64 _get92;
  i32 _get91;
  f64 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  f64 _get86;
  f64 _get85;
  i32 _get84;
  f64 _get83;
  i32 _get82;
  f64 _get81;
  i32 _get80;
  f64 _get79;
  i32 _get78;
  f64 _get77;
  i32 _get76;
  f64 _get75;
  f64 _get74;
  i32 _get73;
  f64 _get72;
  i32 _get71;
  f64 _get70;
  i32 _get69;
  f64 _get68;
  i32 _get67;
  f64 _get66;
  i32 _get65;
  f64 _get64;
  f64 _get63;
  i32 _get62;
  f64 _get61;
  i32 _get60;
  f64 _get59;
  i32 _get58;
  f64 _get57;
  i32 _get56;
  f64 _get55;
  i32 _get54;
  f64 _get53;
  f64 _get52;
  i32 _get51;
  f64 _get50;
  i32 _get49;
  f64 _get48;
  i32 _get47;
  f64 _get46;
  i32 _get45;
  f64 _get44;
  i32 _get43;
  f64 _get42;
  f64 _get41;
  i32 _get40;
  f64 _get39;
  i32 _get38;
  f64 _get37;
  i32 _get36;
  f64 _get35;
  i32 _get34;
  f64 _get33;
  i32 _get32;
  f64 _get31;
  f64 _get30;
  i32 _get29;
  f64 _get28;
  i32 _get27;
  f64 _get26;
  i32 _get25;
  f64 _get24;
  i32 _get23;
  f64 _get22;
  i32 _get21;
  f64 _get20;
  f64 _get19;
  i32 _get18;
  f64 _get17;
  i32 _get16;
  f64 _get15;
  i32 _get14;
  f64 _get13;
  i32 _get12;
  f64 _get11;
  i32 _get10;
  f64 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  i32 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  _get0 = i;
  _get1 = ijjtype;
  // if 
    if ((f64)((_get0 == 0) & ((_get1 | 128) == (1 | 128))) != 0) {
      _get2 = id;
      dong_porf_porf_todo_todoId0 = _get2;
      _get3 = idjjtype;
      dong_porf_porf_todo_todoId0jjtype = _get3;
      (void) dong_porf_porf_todo_todoId0;
      _get4 = text;
      dong_porf_porf_todo_todoText0 = _get4;
      _get5 = textjjtype;
      dong_porf_porf_todo_todoText0jjtype = _get5;
      (void) dong_porf_porf_todo_todoText0;
      _get6 = done;
      dong_porf_porf_todo_todoDone0 = _get6;
      _get7 = donejjtype;
      dong_porf_porf_todo_todoDone0jjtype = _get7;
      (void) dong_porf_porf_todo_todoDone0;
      _get8 = jjnewtarget;
      // if 
        if (((u32)(_get8)) != 0) {
          _get9 = jjthis;
          _get10 = jjthisjjtype;
          return (struct ReturnValue){ _get9, _get10 };
        }
      // end
      j986:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j985:;
  _get11 = i;
  _get12 = ijjtype;
  // if 
    if ((f64)((_get11 == 1) & ((_get12 | 128) == (1 | 128))) != 0) {
      _get13 = id;
      dong_porf_porf_todo_todoId1 = _get13;
      _get14 = idjjtype;
      dong_porf_porf_todo_todoId1jjtype = _get14;
      (void) dong_porf_porf_todo_todoId1;
      _get15 = text;
      dong_porf_porf_todo_todoText1 = _get15;
      _get16 = textjjtype;
      dong_porf_porf_todo_todoText1jjtype = _get16;
      (void) dong_porf_porf_todo_todoText1;
      _get17 = done;
      dong_porf_porf_todo_todoDone1 = _get17;
      _get18 = donejjtype;
      dong_porf_porf_todo_todoDone1jjtype = _get18;
      (void) dong_porf_porf_todo_todoDone1;
      _get19 = jjnewtarget;
      // if 
        if (((u32)(_get19)) != 0) {
          _get20 = jjthis;
          _get21 = jjthisjjtype;
          return (struct ReturnValue){ _get20, _get21 };
        }
      // end
      j988:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j987:;
  _get22 = i;
  _get23 = ijjtype;
  // if 
    if ((f64)((_get22 == 2) & ((_get23 | 128) == (1 | 128))) != 0) {
      _get24 = id;
      dong_porf_porf_todo_todoId2 = _get24;
      _get25 = idjjtype;
      dong_porf_porf_todo_todoId2jjtype = _get25;
      (void) dong_porf_porf_todo_todoId2;
      _get26 = text;
      dong_porf_porf_todo_todoText2 = _get26;
      _get27 = textjjtype;
      dong_porf_porf_todo_todoText2jjtype = _get27;
      (void) dong_porf_porf_todo_todoText2;
      _get28 = done;
      dong_porf_porf_todo_todoDone2 = _get28;
      _get29 = donejjtype;
      dong_porf_porf_todo_todoDone2jjtype = _get29;
      (void) dong_porf_porf_todo_todoDone2;
      _get30 = jjnewtarget;
      // if 
        if (((u32)(_get30)) != 0) {
          _get31 = jjthis;
          _get32 = jjthisjjtype;
          return (struct ReturnValue){ _get31, _get32 };
        }
      // end
      j990:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j989:;
  _get33 = i;
  _get34 = ijjtype;
  // if 
    if ((f64)((_get33 == 3) & ((_get34 | 128) == (1 | 128))) != 0) {
      _get35 = id;
      dong_porf_porf_todo_todoId3 = _get35;
      _get36 = idjjtype;
      dong_porf_porf_todo_todoId3jjtype = _get36;
      (void) dong_porf_porf_todo_todoId3;
      _get37 = text;
      dong_porf_porf_todo_todoText3 = _get37;
      _get38 = textjjtype;
      dong_porf_porf_todo_todoText3jjtype = _get38;
      (void) dong_porf_porf_todo_todoText3;
      _get39 = done;
      dong_porf_porf_todo_todoDone3 = _get39;
      _get40 = donejjtype;
      dong_porf_porf_todo_todoDone3jjtype = _get40;
      (void) dong_porf_porf_todo_todoDone3;
      _get41 = jjnewtarget;
      // if 
        if (((u32)(_get41)) != 0) {
          _get42 = jjthis;
          _get43 = jjthisjjtype;
          return (struct ReturnValue){ _get42, _get43 };
        }
      // end
      j992:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j991:;
  _get44 = i;
  _get45 = ijjtype;
  // if 
    if ((f64)((_get44 == 4) & ((_get45 | 128) == (1 | 128))) != 0) {
      _get46 = id;
      dong_porf_porf_todo_todoId4 = _get46;
      _get47 = idjjtype;
      dong_porf_porf_todo_todoId4jjtype = _get47;
      (void) dong_porf_porf_todo_todoId4;
      _get48 = text;
      dong_porf_porf_todo_todoText4 = _get48;
      _get49 = textjjtype;
      dong_porf_porf_todo_todoText4jjtype = _get49;
      (void) dong_porf_porf_todo_todoText4;
      _get50 = done;
      dong_porf_porf_todo_todoDone4 = _get50;
      _get51 = donejjtype;
      dong_porf_porf_todo_todoDone4jjtype = _get51;
      (void) dong_porf_porf_todo_todoDone4;
      _get52 = jjnewtarget;
      // if 
        if (((u32)(_get52)) != 0) {
          _get53 = jjthis;
          _get54 = jjthisjjtype;
          return (struct ReturnValue){ _get53, _get54 };
        }
      // end
      j994:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j993:;
  _get55 = i;
  _get56 = ijjtype;
  // if 
    if ((f64)((_get55 == 5) & ((_get56 | 128) == (1 | 128))) != 0) {
      _get57 = id;
      dong_porf_porf_todo_todoId5 = _get57;
      _get58 = idjjtype;
      dong_porf_porf_todo_todoId5jjtype = _get58;
      (void) dong_porf_porf_todo_todoId5;
      _get59 = text;
      dong_porf_porf_todo_todoText5 = _get59;
      _get60 = textjjtype;
      dong_porf_porf_todo_todoText5jjtype = _get60;
      (void) dong_porf_porf_todo_todoText5;
      _get61 = done;
      dong_porf_porf_todo_todoDone5 = _get61;
      _get62 = donejjtype;
      dong_porf_porf_todo_todoDone5jjtype = _get62;
      (void) dong_porf_porf_todo_todoDone5;
      _get63 = jjnewtarget;
      // if 
        if (((u32)(_get63)) != 0) {
          _get64 = jjthis;
          _get65 = jjthisjjtype;
          return (struct ReturnValue){ _get64, _get65 };
        }
      // end
      j996:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j995:;
  _get66 = i;
  _get67 = ijjtype;
  // if 
    if ((f64)((_get66 == 6) & ((_get67 | 128) == (1 | 128))) != 0) {
      _get68 = id;
      dong_porf_porf_todo_todoId6 = _get68;
      _get69 = idjjtype;
      dong_porf_porf_todo_todoId6jjtype = _get69;
      (void) dong_porf_porf_todo_todoId6;
      _get70 = text;
      dong_porf_porf_todo_todoText6 = _get70;
      _get71 = textjjtype;
      dong_porf_porf_todo_todoText6jjtype = _get71;
      (void) dong_porf_porf_todo_todoText6;
      _get72 = done;
      dong_porf_porf_todo_todoDone6 = _get72;
      _get73 = donejjtype;
      dong_porf_porf_todo_todoDone6jjtype = _get73;
      (void) dong_porf_porf_todo_todoDone6;
      _get74 = jjnewtarget;
      // if 
        if (((u32)(_get74)) != 0) {
          _get75 = jjthis;
          _get76 = jjthisjjtype;
          return (struct ReturnValue){ _get75, _get76 };
        }
      // end
      j998:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j997:;
  _get77 = i;
  _get78 = ijjtype;
  // if 
    if ((f64)((_get77 == 7) & ((_get78 | 128) == (1 | 128))) != 0) {
      _get79 = id;
      dong_porf_porf_todo_todoId7 = _get79;
      _get80 = idjjtype;
      dong_porf_porf_todo_todoId7jjtype = _get80;
      (void) dong_porf_porf_todo_todoId7;
      _get81 = text;
      dong_porf_porf_todo_todoText7 = _get81;
      _get82 = textjjtype;
      dong_porf_porf_todo_todoText7jjtype = _get82;
      (void) dong_porf_porf_todo_todoText7;
      _get83 = done;
      dong_porf_porf_todo_todoDone7 = _get83;
      _get84 = donejjtype;
      dong_porf_porf_todo_todoDone7jjtype = _get84;
      (void) dong_porf_porf_todo_todoDone7;
      _get85 = jjnewtarget;
      // if 
        if (((u32)(_get85)) != 0) {
          _get86 = jjthis;
          _get87 = jjthisjjtype;
          return (struct ReturnValue){ _get86, _get87 };
        }
      // end
      j1000:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j999:;
  _get88 = i;
  _get89 = ijjtype;
  // if 
    if ((f64)((_get88 == 8) & ((_get89 | 128) == (1 | 128))) != 0) {
      _get90 = id;
      dong_porf_porf_todo_todoId8 = _get90;
      _get91 = idjjtype;
      dong_porf_porf_todo_todoId8jjtype = _get91;
      (void) dong_porf_porf_todo_todoId8;
      _get92 = text;
      dong_porf_porf_todo_todoText8 = _get92;
      _get93 = textjjtype;
      dong_porf_porf_todo_todoText8jjtype = _get93;
      (void) dong_porf_porf_todo_todoText8;
      _get94 = done;
      dong_porf_porf_todo_todoDone8 = _get94;
      _get95 = donejjtype;
      dong_porf_porf_todo_todoDone8jjtype = _get95;
      (void) dong_porf_porf_todo_todoDone8;
      _get96 = jjnewtarget;
      // if 
        if (((u32)(_get96)) != 0) {
          _get97 = jjthis;
          _get98 = jjthisjjtype;
          return (struct ReturnValue){ _get97, _get98 };
        }
      // end
      j1002:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1001:;
  _get99 = i;
  _get100 = ijjtype;
  // if 
    if ((f64)((_get99 == 9) & ((_get100 | 128) == (1 | 128))) != 0) {
      _get101 = id;
      dong_porf_porf_todo_todoId9 = _get101;
      _get102 = idjjtype;
      dong_porf_porf_todo_todoId9jjtype = _get102;
      (void) dong_porf_porf_todo_todoId9;
      _get103 = text;
      dong_porf_porf_todo_todoText9 = _get103;
      _get104 = textjjtype;
      dong_porf_porf_todo_todoText9jjtype = _get104;
      (void) dong_porf_porf_todo_todoText9;
      _get105 = done;
      dong_porf_porf_todo_todoDone9 = _get105;
      _get106 = donejjtype;
      dong_porf_porf_todo_todoDone9jjtype = _get106;
      (void) dong_porf_porf_todo_todoDone9;
      _get107 = jjnewtarget;
      // if 
        if (((u32)(_get107)) != 0) {
          _get108 = jjthis;
          _get109 = jjthisjjtype;
          return (struct ReturnValue){ _get108, _get109 };
        }
      // end
      j1004:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1003:;
  _get110 = i;
  _get111 = ijjtype;
  // if 
    if ((f64)((_get110 == 10) & ((_get111 | 128) == (1 | 128))) != 0) {
      _get112 = id;
      dong_porf_porf_todo_todoId10 = _get112;
      _get113 = idjjtype;
      dong_porf_porf_todo_todoId10jjtype = _get113;
      (void) dong_porf_porf_todo_todoId10;
      _get114 = text;
      dong_porf_porf_todo_todoText10 = _get114;
      _get115 = textjjtype;
      dong_porf_porf_todo_todoText10jjtype = _get115;
      (void) dong_porf_porf_todo_todoText10;
      _get116 = done;
      dong_porf_porf_todo_todoDone10 = _get116;
      _get117 = donejjtype;
      dong_porf_porf_todo_todoDone10jjtype = _get117;
      (void) dong_porf_porf_todo_todoDone10;
      _get118 = jjnewtarget;
      // if 
        if (((u32)(_get118)) != 0) {
          _get119 = jjthis;
          _get120 = jjthisjjtype;
          return (struct ReturnValue){ _get119, _get120 };
        }
      // end
      j1006:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1005:;
  _get121 = i;
  _get122 = ijjtype;
  // if 
    if ((f64)((_get121 == 11) & ((_get122 | 128) == (1 | 128))) != 0) {
      _get123 = id;
      dong_porf_porf_todo_todoId11 = _get123;
      _get124 = idjjtype;
      dong_porf_porf_todo_todoId11jjtype = _get124;
      (void) dong_porf_porf_todo_todoId11;
      _get125 = text;
      dong_porf_porf_todo_todoText11 = _get125;
      _get126 = textjjtype;
      dong_porf_porf_todo_todoText11jjtype = _get126;
      (void) dong_porf_porf_todo_todoText11;
      _get127 = done;
      dong_porf_porf_todo_todoDone11 = _get127;
      _get128 = donejjtype;
      dong_porf_porf_todo_todoDone11jjtype = _get128;
      (void) dong_porf_porf_todo_todoDone11;
      _get129 = jjnewtarget;
      // if 
        if (((u32)(_get129)) != 0) {
          _get130 = jjthis;
          _get131 = jjthisjjtype;
          return (struct ReturnValue){ _get130, _get131 };
        }
      // end
      j1008:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1007:;
  _get132 = i;
  _get133 = ijjtype;
  // if 
    if ((f64)((_get132 == 12) & ((_get133 | 128) == (1 | 128))) != 0) {
      _get134 = id;
      dong_porf_porf_todo_todoId12 = _get134;
      _get135 = idjjtype;
      dong_porf_porf_todo_todoId12jjtype = _get135;
      (void) dong_porf_porf_todo_todoId12;
      _get136 = text;
      dong_porf_porf_todo_todoText12 = _get136;
      _get137 = textjjtype;
      dong_porf_porf_todo_todoText12jjtype = _get137;
      (void) dong_porf_porf_todo_todoText12;
      _get138 = done;
      dong_porf_porf_todo_todoDone12 = _get138;
      _get139 = donejjtype;
      dong_porf_porf_todo_todoDone12jjtype = _get139;
      (void) dong_porf_porf_todo_todoDone12;
      _get140 = jjnewtarget;
      // if 
        if (((u32)(_get140)) != 0) {
          _get141 = jjthis;
          _get142 = jjthisjjtype;
          return (struct ReturnValue){ _get141, _get142 };
        }
      // end
      j1010:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1009:;
  _get143 = i;
  _get144 = ijjtype;
  // if 
    if ((f64)((_get143 == 13) & ((_get144 | 128) == (1 | 128))) != 0) {
      _get145 = id;
      dong_porf_porf_todo_todoId13 = _get145;
      _get146 = idjjtype;
      dong_porf_porf_todo_todoId13jjtype = _get146;
      (void) dong_porf_porf_todo_todoId13;
      _get147 = text;
      dong_porf_porf_todo_todoText13 = _get147;
      _get148 = textjjtype;
      dong_porf_porf_todo_todoText13jjtype = _get148;
      (void) dong_porf_porf_todo_todoText13;
      _get149 = done;
      dong_porf_porf_todo_todoDone13 = _get149;
      _get150 = donejjtype;
      dong_porf_porf_todo_todoDone13jjtype = _get150;
      (void) dong_porf_porf_todo_todoDone13;
      _get151 = jjnewtarget;
      // if 
        if (((u32)(_get151)) != 0) {
          _get152 = jjthis;
          _get153 = jjthisjjtype;
          return (struct ReturnValue){ _get152, _get153 };
        }
      // end
      j1012:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1011:;
  _get154 = i;
  _get155 = ijjtype;
  // if 
    if ((f64)((_get154 == 14) & ((_get155 | 128) == (1 | 128))) != 0) {
      _get156 = id;
      dong_porf_porf_todo_todoId14 = _get156;
      _get157 = idjjtype;
      dong_porf_porf_todo_todoId14jjtype = _get157;
      (void) dong_porf_porf_todo_todoId14;
      _get158 = text;
      dong_porf_porf_todo_todoText14 = _get158;
      _get159 = textjjtype;
      dong_porf_porf_todo_todoText14jjtype = _get159;
      (void) dong_porf_porf_todo_todoText14;
      _get160 = done;
      dong_porf_porf_todo_todoDone14 = _get160;
      _get161 = donejjtype;
      dong_porf_porf_todo_todoDone14jjtype = _get161;
      (void) dong_porf_porf_todo_todoDone14;
      _get162 = jjnewtarget;
      // if 
        if (((u32)(_get162)) != 0) {
          _get163 = jjthis;
          _get164 = jjthisjjtype;
          return (struct ReturnValue){ _get163, _get164 };
        }
      // end
      j1014:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1013:;
  _get165 = id;
  dong_porf_porf_todo_todoId15 = _get165;
  _get166 = idjjtype;
  dong_porf_porf_todo_todoId15jjtype = _get166;
  (void) dong_porf_porf_todo_todoId15;
  _get167 = text;
  dong_porf_porf_todo_todoText15 = _get167;
  _get168 = textjjtype;
  dong_porf_porf_todo_todoText15jjtype = _get168;
  (void) dong_porf_porf_todo_todoText15;
  _get169 = done;
  dong_porf_porf_todo_todoDone15 = _get169;
  _get170 = donejjtype;
  dong_porf_porf_todo_todoDone15jjtype = _get170;
  (void) dong_porf_porf_todo_todoDone15;
  _get171 = jjnewtarget;
  // if 
    if (((u32)(_get171)) != 0) {
      _get172 = jjthis;
      _get173 = jjthisjjtype;
      return (struct ReturnValue){ _get172, _get173 };
    }
  // end
  j1015:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_setValue(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 v, i32 vjjtype) {
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = nodeId;
  _get1 = v;
  _get2 = vjjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get1, _get2);
  (void) _0.type;
  __porf_import_dong_set_value(_get0, _0.value);
  _get3 = jjnewtarget;
  // if 
    if (((u32)(_get3)) != 0) {
      _get4 = jjthis;
      _get5 = jjthisjjtype;
      return (struct ReturnValue){ _get4, _get5 };
    }
  // end
  j1016:;
  return (struct ReturnValue){ 0, 0 };
}

struct ReturnValue dong_porf_porf_todo_onAdd(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get22;
  f64 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  f64 _get15;
  i32 _get14;
  f64 _get13;
  f64 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 text = 0;
  i32 textjjtype = 0;
  i32 jjlast_type = 0;
  f64 trimmed = 0;
  i32 trimmedjjtype = 0;
  i32 jjlength_tmp = 0;
  f64 jjmember_obj_152 = 0;
  f64 jjmember_prop_152 = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_getValue(0, 0, 0, 0, dong_porf_porf_todo_todoInputId, 1);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  textjjtype = _get0;
  text = _0.value;
  _get1 = text;
  trimmed = _get1;
  _get2 = textjjtype;
  trimmedjjtype = _get2;
  _get3 = trimmed;
  jjlength_tmp = (u32)(_get3);
  _get4 = trimmedjjtype;
  // if f64
  f64 _r979;
    if ((_get4 & 64) != 0) {
      _get5 = jjlength_tmp;
      jjlast_type = 1;
      _r979 = (f64)(i32_load(1, 0, _get5));
    } else {
      jjmember_prop_152 = 564;
      _get6 = trimmed;
      jjmember_obj_152 = _get6;
      _get7 = trimmedjjtype;
      // if f64
      f64 _r980;
        if (_get7 == 0) {
          _r980 = 0;
        } else {
          _get8 = jjmember_obj_152;
          _get9 = trimmedjjtype;
          _get10 = jjmember_prop_152;
          const struct ReturnValue _1 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get8), _get9, (u32)(_get10), 195, -2086110260, 1);
          jjlast_type = _1.type;
          _r980 = _1.value;
        }
      // end
      j980:;
      _r979 = _r980;
    }
  // end
  j979:;
  _get11 = jjlast_type;
  // if 
    if ((f64)((_r979 == 0) & ((_get11 | 128) == (1 | 128))) != 0) {
      _get12 = jjnewtarget;
      // if 
        if (((u32)(_get12)) != 0) {
          _get13 = jjthis;
          _get14 = jjthisjjtype;
          return (struct ReturnValue){ _get13, _get14 };
        }
      // end
      j982:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j981:;
  // if 
    if ((f64)(dong_porf_porf_todo_todoCount >= dong_porf_porf_todo_MAX_TODOS) != 0) {
      const struct ReturnValue _2 = dong_porf_porf_todo_dongLog(0, 0, 0, 0, 2185, 195);
      jjlast_type = _2.type;
      (void) _2.value;
      _get15 = jjnewtarget;
      // if 
        if (((u32)(_get15)) != 0) {
          _get16 = jjthis;
          _get17 = jjthisjjtype;
          return (struct ReturnValue){ _get16, _get17 };
        }
      // end
      j984:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j983:;
  _get18 = trimmed;
  _get19 = trimmedjjtype;
  const struct ReturnValue _3 = dong_porf_porf_todo_setTodoSlot(0, 0, 0, 0, dong_porf_porf_todo_todoCount, 1, dong_porf_porf_todo_nextId, 1, _get18, _get19, 0, 1);
  jjlast_type = _3.type;
  (void) _3.value;
  dong_porf_porf_todo_todoCount = dong_porf_porf_todo_todoCount + 1;
  dong_porf_porf_todo_todoCountjjtype = 1;
  (void) dong_porf_porf_todo_todoCount;
  dong_porf_porf_todo_nextId = dong_porf_porf_todo_nextId + 1;
  dong_porf_porf_todo_nextIdjjtype = 1;
  (void) dong_porf_porf_todo_nextId;
  const struct ReturnValue _4 = dong_porf_porf_todo_setValue(0, 0, 0, 0, dong_porf_porf_todo_todoInputId, 1, 0, 195);
  jjlast_type = _4.type;
  (void) _4.value;
  const struct ReturnValue _5 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
  jjlast_type = _5.type;
  (void) _5.value;
  _get20 = jjnewtarget;
  // if 
    if (((u32)(_get20)) != 0) {
      _get21 = jjthis;
      _get22 = jjthisjjtype;
      return (struct ReturnValue){ _get21, _get22 };
    }
  // end
  j1017:;
  return (struct ReturnValue){ 0, 0 };
}

struct ReturnValue dong_porf_porf_todo_onInputChange(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get3;
  f64 _get2;
  f64 _get1;
  i32 _get0;
  i32 jjlast_type = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_getValue(0, 0, 0, 0, dong_porf_porf_todo_todoInputId, 1);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  dong_porf_porf_todo_inputTextjjtype = _get0;
  dong_porf_porf_todo_inputText = _0.value;
  _get1 = jjnewtarget;
  // if 
    if (((u32)(_get1)) != 0) {
      _get2 = jjthis;
      _get3 = jjthisjjtype;
      return (struct ReturnValue){ _get2, _get3 };
    }
  // end
  j1018:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_eventKey(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  i32 _get3;
  f64 _get2;
  f64 _get1;
  i32 _get0;
  i32 jjlast_type = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  __porf_import_dong_event_key();
  const struct ReturnValue _0 = dong_porf_porf_todo_pullHostString(0, 0, 0, 0);
  jjlast_type = _0.type;
  jjreturn = _0.value;
  _get0 = jjlast_type;
  jjreturnjjtype = _get0;
  _get1 = jjnewtarget;
  // if 
    if (((u32)(_get1)) != 0) {
      _get2 = jjreturn;
      _get3 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get2), _get3)) == 0) {
          _get4 = jjthis;
          _get5 = jjthisjjtype;
          return (struct ReturnValue){ _get4, _get5 };
        }
      // end
      j1020:;
    }
  // end
  j1019:;
  _get6 = jjreturn;
  _get7 = jjreturnjjtype;
  return (struct ReturnValue){ _get6, _get7 };
}

static f64 dong_porf_porf_todo__Porffor_compareStrings(f64 a, i32 ajjtype, f64 b, i32 bjjtype) {
  i32 _get25;
  f64 _get24;
  i32 _get23;
  f64 _get22;
  i32 _get21;
  i32 _get20;
  f64 _get19;
  i32 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  i32 _get14;
  i32 _get13;
  f64 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 jjlast_type = 0;

  _get0 = ajjtype;
  // if 
    if ((f64)(_get0 | 128) != 195) {
      _get1 = a;
      jjlogicinner_tmp = _get1;
      _get2 = ajjtype;
      jjtypeswitch_tmp1 = _get2;
      // block i32
      i32 _r1022;
        _get3 = jjtypeswitch_tmp1;
        // if 
          if (_get3 == 0) {
            _r1022 = 1;
            goto j1022;
          }
        // end
        j1023:;
        _get4 = jjtypeswitch_tmp1;
        // if 
          if (_get4 == 7) {
            _get5 = jjlogicinner_tmp;
            _r1022 = _get5 == 0;
            goto j1022;
          }
        // end
        j1024:;
        _r1022 = 0;
      // end
      j1022:;
      _get6 = ajjtype;
      _get7 = ajjtype;
      // if 
        if (((_r1022 | ((f64)(_get6) == 5)) | ((f64)(_get7) == 2)) != 0) {
          return 0;
        }
      // end
      j1025:;
      _get8 = a;
      _get9 = ajjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString(_get8, _get9);
      jjlast_type = _0.type;
      _get10 = jjlast_type;
      ajjtype = _get10;
      a = _0.value;
    }
  // end
  j1021:;
  _get11 = bjjtype;
  // if 
    if ((f64)(_get11 | 128) != 195) {
      _get12 = b;
      jjlogicinner_tmp = _get12;
      _get13 = bjjtype;
      jjtypeswitch_tmp1 = _get13;
      // block i32
      i32 _r1027;
        _get14 = jjtypeswitch_tmp1;
        // if 
          if (_get14 == 0) {
            _r1027 = 1;
            goto j1027;
          }
        // end
        j1028:;
        _get15 = jjtypeswitch_tmp1;
        // if 
          if (_get15 == 7) {
            _get16 = jjlogicinner_tmp;
            _r1027 = _get16 == 0;
            goto j1027;
          }
        // end
        j1029:;
        _r1027 = 0;
      // end
      j1027:;
      _get17 = bjjtype;
      _get18 = bjjtype;
      // if 
        if (((_r1027 | ((f64)(_get17) == 5)) | ((f64)(_get18) == 2)) != 0) {
          return 0;
        }
      // end
      j1030:;
      _get19 = b;
      _get20 = bjjtype;
      const struct ReturnValue _1 = dong_porf_porf_todo__ecma262_ToString(_get19, _get20);
      jjlast_type = _1.type;
      _get21 = jjlast_type;
      bjjtype = _get21;
      b = _1.value;
    }
  // end
  j1026:;
  _get22 = a;
  _get23 = ajjtype;
  _get24 = b;
  _get25 = bjjtype;
  return (f64)(dong_porf_porf_todo__Porffor_strcmp((i32)(_get22), _get23, (i32)(_get24), _get25));
}

struct ReturnValue dong_porf_porf_todo_onKeyDown(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get6;
  f64 _get5;
  f64 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 key = 0;
  i32 keyjjtype = 0;
  i32 jjlast_type = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_eventKey(0, 0, 0, 0);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  keyjjtype = _get0;
  key = _0.value;
  _get1 = key;
  _get2 = keyjjtype;
  _get3 = keyjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get1, _get2, 3163, 195)) & ((_get3 | 128) == (195 | 128))) != 0) {
      const struct ReturnValue _1 = dong_porf_porf_todo_onAdd(0, 0, 0, 0);
      jjlast_type = _1.type;
      (void) _1.value;
    }
  // end
  j1031:;
  _get4 = jjnewtarget;
  // if 
    if (((u32)(_get4)) != 0) {
      _get5 = jjthis;
      _get6 = jjthisjjtype;
      return (struct ReturnValue){ _get5, _get6 };
    }
  // end
  j1032:;
  return (struct ReturnValue){ 0, 0 };
}

struct ReturnValue dong_porf_porf_todo_onFilterAll(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  dong_porf_porf_todo_filterMode = 0;
  dong_porf_porf_todo_filterModejjtype = 1;
  (void) dong_porf_porf_todo_filterMode;
  const struct ReturnValue _0 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
  (void) _0.type;
  (void) _0.value;
  _get0 = jjnewtarget;
  // if 
    if (((u32)(_get0)) != 0) {
      _get1 = jjthis;
      _get2 = jjthisjjtype;
      return (struct ReturnValue){ _get1, _get2 };
    }
  // end
  j1033:;
  return (struct ReturnValue){ 0, 0 };
}

struct ReturnValue dong_porf_porf_todo_onFilterActive(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  dong_porf_porf_todo_filterMode = 1;
  dong_porf_porf_todo_filterModejjtype = 1;
  (void) dong_porf_porf_todo_filterMode;
  const struct ReturnValue _0 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
  (void) _0.type;
  (void) _0.value;
  _get0 = jjnewtarget;
  // if 
    if (((u32)(_get0)) != 0) {
      _get1 = jjthis;
      _get2 = jjthisjjtype;
      return (struct ReturnValue){ _get1, _get2 };
    }
  // end
  j1034:;
  return (struct ReturnValue){ 0, 0 };
}

struct ReturnValue dong_porf_porf_todo_onFilterDone(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  dong_porf_porf_todo_filterMode = 2;
  dong_porf_porf_todo_filterModejjtype = 1;
  (void) dong_porf_porf_todo_filterMode;
  const struct ReturnValue _0 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
  (void) _0.type;
  (void) _0.value;
  _get0 = jjnewtarget;
  // if 
    if (((u32)(_get0)) != 0) {
      _get1 = jjthis;
      _get2 = jjthisjjtype;
      return (struct ReturnValue){ _get1, _get2 };
    }
  // end
  j1035:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_removeAtIndex(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 idx, i32 idxjjtype) {
  i32 _get48;
  f64 _get47;
  f64 _get46;
  f64 _get45;
  f64 _get44;
  i32 _get43;
  f64 _get42;
  i32 _get41;
  f64 _get40;
  i32 _get39;
  f64 _get38;
  i32 _get37;
  f64 _get36;
  f64 _get35;
  f64 _get34;
  f64 _get33;
  i32 _get32;
  f64 _get31;
  i32 _get30;
  f64 _get29;
  i32 _get28;
  f64 _get27;
  i32 _get26;
  f64 _get25;
  f64 _get24;
  f64 _get23;
  f64 _get22;
  i32 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  i32 _get10;
  f64 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  f64 j = 0;
  i32 jjjtype = 0;
  i32 jjlast_type = 0;

  _get0 = idx;
  j = _get0;
  _get1 = idxjjtype;
  jjjtype = _get1;
  // loop 
  j1042:;
    _get2 = j;
    // if 
      if ((_get2 + 1) < dong_porf_porf_todo_todoCount) {
        _get3 = j;
        _get4 = jjjtype;
        _get5 = j;
        const struct ReturnValue _0 = dong_porf_porf_todo_todoIdAt(0, 0, 0, 0, _get5 + 1, 1);
        jjlast_type = _0.type;
        _get6 = jjlast_type;
        _get7 = j;
        const struct ReturnValue _1 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get7 + 1, 1);
        jjlast_type = _1.type;
        _get8 = jjlast_type;
        _get9 = j;
        const struct ReturnValue _2 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get9 + 1, 1);
        jjlast_type = _2.type;
        _get10 = jjlast_type;
        const struct ReturnValue _3 = dong_porf_porf_todo_setTodoSlot(0, 0, 0, 0, _get3, _get4, _0.value, _get6, _1.value, _get8, _2.value, _get10);
        jjlast_type = _3.type;
        (void) _3.value;
        _get11 = j;
        j = _get11 + 1;
        _get12 = j;
        jjjtype = 1;
        (void) _get12;
        _get13 = j;
        if (!((_get13 + 1) < dong_porf_porf_todo_todoCount)) {
          goto j1043;
        }
        _get14 = j;
        _get15 = jjjtype;
        _get16 = j;
        const struct ReturnValue _4 = dong_porf_porf_todo_todoIdAt(0, 0, 0, 0, _get16 + 1, 1);
        jjlast_type = _4.type;
        _get17 = jjlast_type;
        _get18 = j;
        const struct ReturnValue _5 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get18 + 1, 1);
        jjlast_type = _5.type;
        _get19 = jjlast_type;
        _get20 = j;
        const struct ReturnValue _6 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get20 + 1, 1);
        jjlast_type = _6.type;
        _get21 = jjlast_type;
        const struct ReturnValue _7 = dong_porf_porf_todo_setTodoSlot(0, 0, 0, 0, _get14, _get15, _4.value, _get17, _5.value, _get19, _6.value, _get21);
        jjlast_type = _7.type;
        (void) _7.value;
        _get22 = j;
        j = _get22 + 1;
        _get23 = j;
        jjjtype = 1;
        (void) _get23;
        _get24 = j;
        if (!((_get24 + 1) < dong_porf_porf_todo_todoCount)) {
          goto j1043;
        }
        _get25 = j;
        _get26 = jjjtype;
        _get27 = j;
        const struct ReturnValue _8 = dong_porf_porf_todo_todoIdAt(0, 0, 0, 0, _get27 + 1, 1);
        jjlast_type = _8.type;
        _get28 = jjlast_type;
        _get29 = j;
        const struct ReturnValue _9 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get29 + 1, 1);
        jjlast_type = _9.type;
        _get30 = jjlast_type;
        _get31 = j;
        const struct ReturnValue _10 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get31 + 1, 1);
        jjlast_type = _10.type;
        _get32 = jjlast_type;
        const struct ReturnValue _11 = dong_porf_porf_todo_setTodoSlot(0, 0, 0, 0, _get25, _get26, _8.value, _get28, _9.value, _get30, _10.value, _get32);
        jjlast_type = _11.type;
        (void) _11.value;
        _get33 = j;
        j = _get33 + 1;
        _get34 = j;
        jjjtype = 1;
        (void) _get34;
        _get35 = j;
        if (!((_get35 + 1) < dong_porf_porf_todo_todoCount)) {
          goto j1043;
        }
        _get36 = j;
        _get37 = jjjtype;
        _get38 = j;
        const struct ReturnValue _12 = dong_porf_porf_todo_todoIdAt(0, 0, 0, 0, _get38 + 1, 1);
        jjlast_type = _12.type;
        _get39 = jjlast_type;
        _get40 = j;
        const struct ReturnValue _13 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get40 + 1, 1);
        jjlast_type = _13.type;
        _get41 = jjlast_type;
        _get42 = j;
        const struct ReturnValue _14 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get42 + 1, 1);
        jjlast_type = _14.type;
        _get43 = jjlast_type;
        const struct ReturnValue _15 = dong_porf_porf_todo_setTodoSlot(0, 0, 0, 0, _get36, _get37, _12.value, _get39, _13.value, _get41, _14.value, _get43);
        jjlast_type = _15.type;
        (void) _15.value;
        _get44 = j;
        j = _get44 + 1;
        _get45 = j;
        jjjtype = 1;
        (void) _get45;
        goto j1042;
      }
    // end
    j1043:;
  // end
  dong_porf_porf_todo_todoCount = dong_porf_porf_todo_todoCount - 1;
  dong_porf_porf_todo_todoCountjjtype = 1;
  (void) dong_porf_porf_todo_todoCount;
  _get46 = jjnewtarget;
  // if 
    if (((u32)(_get46)) != 0) {
      _get47 = jjthis;
      _get48 = jjthisjjtype;
      return (struct ReturnValue){ _get47, _get48 };
    }
  // end
  j1044:;
  return (struct ReturnValue){ 0, 0 };
}

struct ReturnValue dong_porf_porf_todo_onClearDone(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get58;
  f64 _get57;
  f64 _get56;
  f64 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  f64 _get51;
  i32 _get50;
  i32 _get49;
  f64 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  f64 _get43;
  f64 _get42;
  f64 _get41;
  f64 _get40;
  i32 _get39;
  f64 _get38;
  f64 _get37;
  i32 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  f64 _get26;
  i32 _get25;
  f64 _get24;
  f64 _get23;
  i32 _get22;
  i32 _get21;
  f64 _get20;
  i32 _get19;
  i32 _get18;
  i32 _get17;
  i32 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  f64 _get12;
  i32 _get11;
  f64 _get10;
  f64 _get9;
  i32 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  f64 i = 0;
  i32 ijjtype = 0;
  i32 jjlast_type = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;

  i = dong_porf_porf_todo_todoCount - 1;
  ijjtype = 1;
  // loop 
  j1036:;
    _get0 = i;
    // if 
      if (_get0 >= 0) {
        _get1 = i;
        _get2 = ijjtype;
        const struct ReturnValue _0 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get1, _get2);
        jjlast_type = _0.type;
        jjlogicinner_tmp = _0.value;
        _get3 = jjlast_type;
        jjtypeswitch_tmp1 = _get3;
        // block i32
        i32 _r1038;
          _get4 = jjtypeswitch_tmp1;
          _get5 = jjtypeswitch_tmp1;
          // if 
            if (((_get4 == 67) | (_get5 == 195)) != 0) {
              _get6 = jjlogicinner_tmp;
              _r1038 = i32_load(1, 0, (u32)(_get6));
              goto j1038;
            }
          // end
          j1039:;
          _get7 = jjtypeswitch_tmp1;
          _get8 = jjtypeswitch_tmp1;
          // if 
            if (((_get7 == 31) | (_get8 == 32)) != 0) {
              _r1038 = 1;
              goto j1038;
            }
          // end
          j1040:;
          _get9 = jjlogicinner_tmp;
          const f64 _tmp0 = _get9;
          _r1038 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
        // end
        j1038:;
        // if 
          if ((_r1038) != 0) {
            _get10 = i;
            _get11 = ijjtype;
            const struct ReturnValue _1 = dong_porf_porf_todo_removeAtIndex(0, 0, 0, 0, _get10, _get11);
            jjlast_type = _1.type;
            (void) _1.value;
          }
        // end
        j1041:;
        _get12 = i;
        i = _get12 - 1;
        _get13 = i;
        ijjtype = 1;
        (void) _get13;
        _get14 = i;
        if (!(_get14 >= 0)) {
          goto j1037;
        }
        _get15 = i;
        _get16 = ijjtype;
        const struct ReturnValue _2 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get15, _get16);
        jjlast_type = _2.type;
        jjlogicinner_tmp = _2.value;
        _get17 = jjlast_type;
        jjtypeswitch_tmp1 = _get17;
        // block i32
        i32 _r1045;
          _get18 = jjtypeswitch_tmp1;
          _get19 = jjtypeswitch_tmp1;
          // if 
            if (((_get18 == 67) | (_get19 == 195)) != 0) {
              _get20 = jjlogicinner_tmp;
              _r1045 = i32_load(1, 0, (u32)(_get20));
              goto j1045;
            }
          // end
          j1046:;
          _get21 = jjtypeswitch_tmp1;
          _get22 = jjtypeswitch_tmp1;
          // if 
            if (((_get21 == 31) | (_get22 == 32)) != 0) {
              _r1045 = 1;
              goto j1045;
            }
          // end
          j1047:;
          _get23 = jjlogicinner_tmp;
          const f64 _tmp1 = _get23;
          _r1045 = (_tmp1 < 0 ? -_tmp1 : _tmp1) > 0;
        // end
        j1045:;
        // if 
          if ((_r1045) != 0) {
            _get24 = i;
            _get25 = ijjtype;
            const struct ReturnValue _3 = dong_porf_porf_todo_removeAtIndex(0, 0, 0, 0, _get24, _get25);
            jjlast_type = _3.type;
            (void) _3.value;
          }
        // end
        j1048:;
        _get26 = i;
        i = _get26 - 1;
        _get27 = i;
        ijjtype = 1;
        (void) _get27;
        _get28 = i;
        if (!(_get28 >= 0)) {
          goto j1037;
        }
        _get29 = i;
        _get30 = ijjtype;
        const struct ReturnValue _4 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get29, _get30);
        jjlast_type = _4.type;
        jjlogicinner_tmp = _4.value;
        _get31 = jjlast_type;
        jjtypeswitch_tmp1 = _get31;
        // block i32
        i32 _r1049;
          _get32 = jjtypeswitch_tmp1;
          _get33 = jjtypeswitch_tmp1;
          // if 
            if (((_get32 == 67) | (_get33 == 195)) != 0) {
              _get34 = jjlogicinner_tmp;
              _r1049 = i32_load(1, 0, (u32)(_get34));
              goto j1049;
            }
          // end
          j1050:;
          _get35 = jjtypeswitch_tmp1;
          _get36 = jjtypeswitch_tmp1;
          // if 
            if (((_get35 == 31) | (_get36 == 32)) != 0) {
              _r1049 = 1;
              goto j1049;
            }
          // end
          j1051:;
          _get37 = jjlogicinner_tmp;
          const f64 _tmp2 = _get37;
          _r1049 = (_tmp2 < 0 ? -_tmp2 : _tmp2) > 0;
        // end
        j1049:;
        // if 
          if ((_r1049) != 0) {
            _get38 = i;
            _get39 = ijjtype;
            const struct ReturnValue _5 = dong_porf_porf_todo_removeAtIndex(0, 0, 0, 0, _get38, _get39);
            jjlast_type = _5.type;
            (void) _5.value;
          }
        // end
        j1052:;
        _get40 = i;
        i = _get40 - 1;
        _get41 = i;
        ijjtype = 1;
        (void) _get41;
        _get42 = i;
        if (!(_get42 >= 0)) {
          goto j1037;
        }
        _get43 = i;
        _get44 = ijjtype;
        const struct ReturnValue _6 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get43, _get44);
        jjlast_type = _6.type;
        jjlogicinner_tmp = _6.value;
        _get45 = jjlast_type;
        jjtypeswitch_tmp1 = _get45;
        // block i32
        i32 _r1053;
          _get46 = jjtypeswitch_tmp1;
          _get47 = jjtypeswitch_tmp1;
          // if 
            if (((_get46 == 67) | (_get47 == 195)) != 0) {
              _get48 = jjlogicinner_tmp;
              _r1053 = i32_load(1, 0, (u32)(_get48));
              goto j1053;
            }
          // end
          j1054:;
          _get49 = jjtypeswitch_tmp1;
          _get50 = jjtypeswitch_tmp1;
          // if 
            if (((_get49 == 31) | (_get50 == 32)) != 0) {
              _r1053 = 1;
              goto j1053;
            }
          // end
          j1055:;
          _get51 = jjlogicinner_tmp;
          const f64 _tmp3 = _get51;
          _r1053 = (_tmp3 < 0 ? -_tmp3 : _tmp3) > 0;
        // end
        j1053:;
        // if 
          if ((_r1053) != 0) {
            _get52 = i;
            _get53 = ijjtype;
            const struct ReturnValue _7 = dong_porf_porf_todo_removeAtIndex(0, 0, 0, 0, _get52, _get53);
            jjlast_type = _7.type;
            (void) _7.value;
          }
        // end
        j1056:;
        _get54 = i;
        i = _get54 - 1;
        _get55 = i;
        ijjtype = 1;
        (void) _get55;
        goto j1036;
      }
    // end
    j1037:;
  // end
  const struct ReturnValue _8 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
  jjlast_type = _8.type;
  (void) _8.value;
  _get56 = jjnewtarget;
  // if 
    if (((u32)(_get56)) != 0) {
      _get57 = jjthis;
      _get58 = jjthisjjtype;
      return (struct ReturnValue){ _get57, _get58 };
    }
  // end
  j1057:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_eventTarget(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  jjreturn = __porf_import_dong_event_target();
  jjreturnjjtype = 1;
  _get0 = jjnewtarget;
  // if 
    if (((u32)(_get0)) != 0) {
      _get1 = jjreturn;
      _get2 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get1), _get2)) == 0) {
          _get3 = jjthis;
          _get4 = jjthisjjtype;
          return (struct ReturnValue){ _get3, _get4 };
        }
      // end
      j1059:;
    }
  // end
  j1058:;
  _get5 = jjreturn;
  _get6 = jjreturnjjtype;
  return (struct ReturnValue){ _get5, _get6 };
}

static struct ReturnValue dong_porf_porf_todo_getAttribute(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 name, i32 namejjtype) {
  i32 _get10;
  f64 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  f64 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = nodeId;
  _get1 = name;
  _get2 = namejjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get1, _get2);
  jjlast_type = _0.type;
  __porf_import_dong_get_attribute(_get0, _0.value);
  const struct ReturnValue _1 = dong_porf_porf_todo_pullHostString(0, 0, 0, 0);
  jjlast_type = _1.type;
  jjreturn = _1.value;
  _get3 = jjlast_type;
  jjreturnjjtype = _get3;
  _get4 = jjnewtarget;
  // if 
    if (((u32)(_get4)) != 0) {
      _get5 = jjreturn;
      _get6 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get5), _get6)) == 0) {
          _get7 = jjthis;
          _get8 = jjthisjjtype;
          return (struct ReturnValue){ _get7, _get8 };
        }
      // end
      j1064:;
    }
  // end
  j1063:;
  _get9 = jjreturn;
  _get10 = jjreturnjjtype;
  return (struct ReturnValue){ _get9, _get10 };
}

static struct ReturnValue dong_porf_porf_todo_parseIndexStr(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype) {
  i32 _get166;
  f64 _get165;
  i32 _get164;
  f64 _get163;
  i32 _get162;
  f64 _get161;
  f64 _get160;
  i32 _get159;
  f64 _get158;
  i32 _get157;
  f64 _get156;
  i32 _get155;
  f64 _get154;
  f64 _get153;
  i32 _get152;
  i32 _get151;
  f64 _get150;
  i32 _get149;
  f64 _get148;
  i32 _get147;
  f64 _get146;
  i32 _get145;
  f64 _get144;
  f64 _get143;
  i32 _get142;
  i32 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  i32 _get137;
  f64 _get136;
  i32 _get135;
  f64 _get134;
  f64 _get133;
  i32 _get132;
  i32 _get131;
  f64 _get130;
  i32 _get129;
  f64 _get128;
  i32 _get127;
  f64 _get126;
  i32 _get125;
  f64 _get124;
  f64 _get123;
  i32 _get122;
  i32 _get121;
  f64 _get120;
  i32 _get119;
  f64 _get118;
  i32 _get117;
  f64 _get116;
  i32 _get115;
  f64 _get114;
  f64 _get113;
  i32 _get112;
  i32 _get111;
  f64 _get110;
  i32 _get109;
  f64 _get108;
  i32 _get107;
  f64 _get106;
  i32 _get105;
  f64 _get104;
  f64 _get103;
  i32 _get102;
  i32 _get101;
  f64 _get100;
  i32 _get99;
  f64 _get98;
  i32 _get97;
  f64 _get96;
  i32 _get95;
  f64 _get94;
  f64 _get93;
  i32 _get92;
  i32 _get91;
  f64 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  f64 _get86;
  i32 _get85;
  f64 _get84;
  f64 _get83;
  i32 _get82;
  i32 _get81;
  f64 _get80;
  i32 _get79;
  f64 _get78;
  i32 _get77;
  f64 _get76;
  i32 _get75;
  f64 _get74;
  f64 _get73;
  i32 _get72;
  i32 _get71;
  f64 _get70;
  i32 _get69;
  f64 _get68;
  i32 _get67;
  f64 _get66;
  i32 _get65;
  f64 _get64;
  f64 _get63;
  i32 _get62;
  i32 _get61;
  f64 _get60;
  i32 _get59;
  f64 _get58;
  i32 _get57;
  f64 _get56;
  i32 _get55;
  f64 _get54;
  f64 _get53;
  i32 _get52;
  i32 _get51;
  f64 _get50;
  i32 _get49;
  f64 _get48;
  i32 _get47;
  f64 _get46;
  i32 _get45;
  f64 _get44;
  f64 _get43;
  i32 _get42;
  i32 _get41;
  f64 _get40;
  i32 _get39;
  f64 _get38;
  i32 _get37;
  f64 _get36;
  i32 _get35;
  f64 _get34;
  f64 _get33;
  i32 _get32;
  i32 _get31;
  f64 _get30;
  i32 _get29;
  f64 _get28;
  i32 _get27;
  f64 _get26;
  i32 _get25;
  f64 _get24;
  f64 _get23;
  i32 _get22;
  i32 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  f64 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  i32 _get1;
  f64 _get0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = s;
  _get1 = sjjtype;
  _get2 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get0, _get1, 398, 195)) & ((_get2 | 128) == (195 | 128))) != 0) {
      jjreturn = 0;
      jjreturnjjtype = 1;
      _get3 = jjnewtarget;
      // if 
        if (((u32)(_get3)) != 0) {
          _get4 = jjreturn;
          _get5 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get4), _get5)) == 0) {
              _get6 = jjthis;
              _get7 = jjthisjjtype;
              return (struct ReturnValue){ _get6, _get7 };
            }
          // end
          j1070:;
        }
      // end
      j1069:;
      _get8 = jjreturn;
      _get9 = jjreturnjjtype;
      return (struct ReturnValue){ _get8, _get9 };
    }
  // end
  j1068:;
  _get10 = s;
  _get11 = sjjtype;
  _get12 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get10, _get11, 2363, 195)) & ((_get12 | 128) == (195 | 128))) != 0) {
      jjreturn = 1;
      jjreturnjjtype = 1;
      _get13 = jjnewtarget;
      // if 
        if (((u32)(_get13)) != 0) {
          _get14 = jjreturn;
          _get15 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get14), _get15)) == 0) {
              _get16 = jjthis;
              _get17 = jjthisjjtype;
              return (struct ReturnValue){ _get16, _get17 };
            }
          // end
          j1073:;
        }
      // end
      j1072:;
      _get18 = jjreturn;
      _get19 = jjreturnjjtype;
      return (struct ReturnValue){ _get18, _get19 };
    }
  // end
  j1071:;
  _get20 = s;
  _get21 = sjjtype;
  _get22 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get20, _get21, 3202, 195)) & ((_get22 | 128) == (195 | 128))) != 0) {
      jjreturn = 2;
      jjreturnjjtype = 1;
      _get23 = jjnewtarget;
      // if 
        if (((u32)(_get23)) != 0) {
          _get24 = jjreturn;
          _get25 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get24), _get25)) == 0) {
              _get26 = jjthis;
              _get27 = jjthisjjtype;
              return (struct ReturnValue){ _get26, _get27 };
            }
          // end
          j1076:;
        }
      // end
      j1075:;
      _get28 = jjreturn;
      _get29 = jjreturnjjtype;
      return (struct ReturnValue){ _get28, _get29 };
    }
  // end
  j1074:;
  _get30 = s;
  _get31 = sjjtype;
  _get32 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get30, _get31, 3209, 195)) & ((_get32 | 128) == (195 | 128))) != 0) {
      jjreturn = 3;
      jjreturnjjtype = 1;
      _get33 = jjnewtarget;
      // if 
        if (((u32)(_get33)) != 0) {
          _get34 = jjreturn;
          _get35 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get34), _get35)) == 0) {
              _get36 = jjthis;
              _get37 = jjthisjjtype;
              return (struct ReturnValue){ _get36, _get37 };
            }
          // end
          j1079:;
        }
      // end
      j1078:;
      _get38 = jjreturn;
      _get39 = jjreturnjjtype;
      return (struct ReturnValue){ _get38, _get39 };
    }
  // end
  j1077:;
  _get40 = s;
  _get41 = sjjtype;
  _get42 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get40, _get41, 3216, 195)) & ((_get42 | 128) == (195 | 128))) != 0) {
      jjreturn = 4;
      jjreturnjjtype = 1;
      _get43 = jjnewtarget;
      // if 
        if (((u32)(_get43)) != 0) {
          _get44 = jjreturn;
          _get45 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get44), _get45)) == 0) {
              _get46 = jjthis;
              _get47 = jjthisjjtype;
              return (struct ReturnValue){ _get46, _get47 };
            }
          // end
          j1082:;
        }
      // end
      j1081:;
      _get48 = jjreturn;
      _get49 = jjreturnjjtype;
      return (struct ReturnValue){ _get48, _get49 };
    }
  // end
  j1080:;
  _get50 = s;
  _get51 = sjjtype;
  _get52 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get50, _get51, 3223, 195)) & ((_get52 | 128) == (195 | 128))) != 0) {
      jjreturn = 5;
      jjreturnjjtype = 1;
      _get53 = jjnewtarget;
      // if 
        if (((u32)(_get53)) != 0) {
          _get54 = jjreturn;
          _get55 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get54), _get55)) == 0) {
              _get56 = jjthis;
              _get57 = jjthisjjtype;
              return (struct ReturnValue){ _get56, _get57 };
            }
          // end
          j1085:;
        }
      // end
      j1084:;
      _get58 = jjreturn;
      _get59 = jjreturnjjtype;
      return (struct ReturnValue){ _get58, _get59 };
    }
  // end
  j1083:;
  _get60 = s;
  _get61 = sjjtype;
  _get62 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get60, _get61, 3230, 195)) & ((_get62 | 128) == (195 | 128))) != 0) {
      jjreturn = 6;
      jjreturnjjtype = 1;
      _get63 = jjnewtarget;
      // if 
        if (((u32)(_get63)) != 0) {
          _get64 = jjreturn;
          _get65 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get64), _get65)) == 0) {
              _get66 = jjthis;
              _get67 = jjthisjjtype;
              return (struct ReturnValue){ _get66, _get67 };
            }
          // end
          j1088:;
        }
      // end
      j1087:;
      _get68 = jjreturn;
      _get69 = jjreturnjjtype;
      return (struct ReturnValue){ _get68, _get69 };
    }
  // end
  j1086:;
  _get70 = s;
  _get71 = sjjtype;
  _get72 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get70, _get71, 3237, 195)) & ((_get72 | 128) == (195 | 128))) != 0) {
      jjreturn = 7;
      jjreturnjjtype = 1;
      _get73 = jjnewtarget;
      // if 
        if (((u32)(_get73)) != 0) {
          _get74 = jjreturn;
          _get75 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get74), _get75)) == 0) {
              _get76 = jjthis;
              _get77 = jjthisjjtype;
              return (struct ReturnValue){ _get76, _get77 };
            }
          // end
          j1091:;
        }
      // end
      j1090:;
      _get78 = jjreturn;
      _get79 = jjreturnjjtype;
      return (struct ReturnValue){ _get78, _get79 };
    }
  // end
  j1089:;
  _get80 = s;
  _get81 = sjjtype;
  _get82 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get80, _get81, 3244, 195)) & ((_get82 | 128) == (195 | 128))) != 0) {
      jjreturn = 8;
      jjreturnjjtype = 1;
      _get83 = jjnewtarget;
      // if 
        if (((u32)(_get83)) != 0) {
          _get84 = jjreturn;
          _get85 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get84), _get85)) == 0) {
              _get86 = jjthis;
              _get87 = jjthisjjtype;
              return (struct ReturnValue){ _get86, _get87 };
            }
          // end
          j1094:;
        }
      // end
      j1093:;
      _get88 = jjreturn;
      _get89 = jjreturnjjtype;
      return (struct ReturnValue){ _get88, _get89 };
    }
  // end
  j1092:;
  _get90 = s;
  _get91 = sjjtype;
  _get92 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get90, _get91, 3251, 195)) & ((_get92 | 128) == (195 | 128))) != 0) {
      jjreturn = 9;
      jjreturnjjtype = 1;
      _get93 = jjnewtarget;
      // if 
        if (((u32)(_get93)) != 0) {
          _get94 = jjreturn;
          _get95 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get94), _get95)) == 0) {
              _get96 = jjthis;
              _get97 = jjthisjjtype;
              return (struct ReturnValue){ _get96, _get97 };
            }
          // end
          j1097:;
        }
      // end
      j1096:;
      _get98 = jjreturn;
      _get99 = jjreturnjjtype;
      return (struct ReturnValue){ _get98, _get99 };
    }
  // end
  j1095:;
  _get100 = s;
  _get101 = sjjtype;
  _get102 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get100, _get101, 3258, 195)) & ((_get102 | 128) == (195 | 128))) != 0) {
      jjreturn = 10;
      jjreturnjjtype = 1;
      _get103 = jjnewtarget;
      // if 
        if (((u32)(_get103)) != 0) {
          _get104 = jjreturn;
          _get105 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get104), _get105)) == 0) {
              _get106 = jjthis;
              _get107 = jjthisjjtype;
              return (struct ReturnValue){ _get106, _get107 };
            }
          // end
          j1100:;
        }
      // end
      j1099:;
      _get108 = jjreturn;
      _get109 = jjreturnjjtype;
      return (struct ReturnValue){ _get108, _get109 };
    }
  // end
  j1098:;
  _get110 = s;
  _get111 = sjjtype;
  _get112 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get110, _get111, 3266, 195)) & ((_get112 | 128) == (195 | 128))) != 0) {
      jjreturn = 11;
      jjreturnjjtype = 1;
      _get113 = jjnewtarget;
      // if 
        if (((u32)(_get113)) != 0) {
          _get114 = jjreturn;
          _get115 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get114), _get115)) == 0) {
              _get116 = jjthis;
              _get117 = jjthisjjtype;
              return (struct ReturnValue){ _get116, _get117 };
            }
          // end
          j1103:;
        }
      // end
      j1102:;
      _get118 = jjreturn;
      _get119 = jjreturnjjtype;
      return (struct ReturnValue){ _get118, _get119 };
    }
  // end
  j1101:;
  _get120 = s;
  _get121 = sjjtype;
  _get122 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get120, _get121, 3274, 195)) & ((_get122 | 128) == (195 | 128))) != 0) {
      jjreturn = 12;
      jjreturnjjtype = 1;
      _get123 = jjnewtarget;
      // if 
        if (((u32)(_get123)) != 0) {
          _get124 = jjreturn;
          _get125 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get124), _get125)) == 0) {
              _get126 = jjthis;
              _get127 = jjthisjjtype;
              return (struct ReturnValue){ _get126, _get127 };
            }
          // end
          j1106:;
        }
      // end
      j1105:;
      _get128 = jjreturn;
      _get129 = jjreturnjjtype;
      return (struct ReturnValue){ _get128, _get129 };
    }
  // end
  j1104:;
  _get130 = s;
  _get131 = sjjtype;
  _get132 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get130, _get131, 3282, 195)) & ((_get132 | 128) == (195 | 128))) != 0) {
      jjreturn = 13;
      jjreturnjjtype = 1;
      _get133 = jjnewtarget;
      // if 
        if (((u32)(_get133)) != 0) {
          _get134 = jjreturn;
          _get135 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get134), _get135)) == 0) {
              _get136 = jjthis;
              _get137 = jjthisjjtype;
              return (struct ReturnValue){ _get136, _get137 };
            }
          // end
          j1109:;
        }
      // end
      j1108:;
      _get138 = jjreturn;
      _get139 = jjreturnjjtype;
      return (struct ReturnValue){ _get138, _get139 };
    }
  // end
  j1107:;
  _get140 = s;
  _get141 = sjjtype;
  _get142 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get140, _get141, 3290, 195)) & ((_get142 | 128) == (195 | 128))) != 0) {
      jjreturn = 14;
      jjreturnjjtype = 1;
      _get143 = jjnewtarget;
      // if 
        if (((u32)(_get143)) != 0) {
          _get144 = jjreturn;
          _get145 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get144), _get145)) == 0) {
              _get146 = jjthis;
              _get147 = jjthisjjtype;
              return (struct ReturnValue){ _get146, _get147 };
            }
          // end
          j1112:;
        }
      // end
      j1111:;
      _get148 = jjreturn;
      _get149 = jjreturnjjtype;
      return (struct ReturnValue){ _get148, _get149 };
    }
  // end
  j1110:;
  _get150 = s;
  _get151 = sjjtype;
  _get152 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get150, _get151, 3298, 195)) & ((_get152 | 128) == (195 | 128))) != 0) {
      jjreturn = 15;
      jjreturnjjtype = 1;
      _get153 = jjnewtarget;
      // if 
        if (((u32)(_get153)) != 0) {
          _get154 = jjreturn;
          _get155 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get154), _get155)) == 0) {
              _get156 = jjthis;
              _get157 = jjthisjjtype;
              return (struct ReturnValue){ _get156, _get157 };
            }
          // end
          j1115:;
        }
      // end
      j1114:;
      _get158 = jjreturn;
      _get159 = jjreturnjjtype;
      return (struct ReturnValue){ _get158, _get159 };
    }
  // end
  j1113:;
  jjreturn = -1;
  jjreturnjjtype = 1;
  _get160 = jjnewtarget;
  // if 
    if (((u32)(_get160)) != 0) {
      _get161 = jjreturn;
      _get162 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get161), _get162)) == 0) {
          _get163 = jjthis;
          _get164 = jjthisjjtype;
          return (struct ReturnValue){ _get163, _get164 };
        }
      // end
      j1117:;
    }
  // end
  j1116:;
  _get165 = jjreturn;
  _get166 = jjreturnjjtype;
  return (struct ReturnValue){ _get165, _get166 };
}

static struct ReturnValue dong_porf_porf_todo_closestSelector(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 selector, i32 selectorjjtype) {
  i32 _get10;
  f64 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  f64 _get4;
  i32 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = nodeId;
  _get1 = selector;
  _get2 = selectorjjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get1, _get2);
  jjlast_type = _0.type;
  jjreturn = __porf_import_dong_closest(_get0, _0.value);
  _get3 = jjlast_type;
  jjreturnjjtype = _get3;
  _get4 = jjnewtarget;
  // if 
    if (((u32)(_get4)) != 0) {
      _get5 = jjreturn;
      _get6 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get5), _get6)) == 0) {
          _get7 = jjthis;
          _get8 = jjthisjjtype;
          return (struct ReturnValue){ _get7, _get8 };
        }
      // end
      j1121:;
    }
  // end
  j1120:;
  _get9 = jjreturn;
  _get10 = jjreturnjjtype;
  return (struct ReturnValue){ _get9, _get10 };
}

static struct ReturnValue dong_porf_porf_todo_readToggleIndex(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  i32 _get51;
  f64 _get50;
  f64 _get49;
  i32 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  f64 _get42;
  i32 _get41;
  i32 _get40;
  f64 _get39;
  i32 _get38;
  i32 _get37;
  f64 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  i32 _get32;
  f64 _get31;
  i32 _get30;
  f64 _get29;
  i32 _get28;
  f64 _get27;
  i32 _get26;
  f64 _get25;
  f64 _get24;
  i32 _get23;
  i32 _get22;
  f64 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  i32 _get14;
  f64 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 target = 0;
  i32 targetjjtype = 0;
  i32 jjlast_type = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;
  f64 v = 0;
  i32 vjjtype = 0;
  i32 jjlength_tmp = 0;
  f64 jjmember_obj_168 = 0;
  f64 jjmember_prop_168 = 0;
  f64 hit = 0;
  i32 hitjjtype = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_eventTarget(0, 0, 0, 0);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  targetjjtype = _get0;
  target = _0.value;
  _get1 = target;
  _get2 = targetjjtype;
  // if 
    if ((f64)((_get1 == 0) & ((_get2 | 128) == (1 | 128))) != 0) {
      jjreturn = -1;
      jjreturnjjtype = 1;
      _get3 = jjnewtarget;
      // if 
        if (((u32)(_get3)) != 0) {
          _get4 = jjreturn;
          _get5 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get4), _get5)) == 0) {
              _get6 = jjthis;
              _get7 = jjthisjjtype;
              return (struct ReturnValue){ _get6, _get7 };
            }
          // end
          j1062:;
        }
      // end
      j1061:;
      _get8 = jjreturn;
      _get9 = jjreturnjjtype;
      return (struct ReturnValue){ _get8, _get9 };
    }
  // end
  j1060:;
  _get10 = target;
  _get11 = targetjjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo_getAttribute(0, 0, 0, 0, _get10, _get11, 3174, 195);
  jjlast_type = _1.type;
  _get12 = jjlast_type;
  vjjtype = _get12;
  v = _1.value;
  _get13 = v;
  jjlength_tmp = (u32)(_get13);
  _get14 = vjjtype;
  // if f64
  f64 _r1065;
    if ((_get14 & 64) != 0) {
      _get15 = jjlength_tmp;
      jjlast_type = 1;
      _r1065 = (f64)(i32_load(1, 0, _get15));
    } else {
      jjmember_prop_168 = 564;
      _get16 = v;
      jjmember_obj_168 = _get16;
      _get17 = vjjtype;
      // if f64
      f64 _r1066;
        if (_get17 == 0) {
          _r1066 = 0;
        } else {
          _get18 = jjmember_obj_168;
          _get19 = vjjtype;
          _get20 = jjmember_prop_168;
          const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get18), _get19, (u32)(_get20), 195, -2086110260, 1);
          jjlast_type = _2.type;
          _r1066 = _2.value;
        }
      // end
      j1066:;
      _r1065 = _r1066;
    }
  // end
  j1065:;
  // if 
    if ((f64)(_r1065 > 0) != 0) {
      _get21 = v;
      _get22 = vjjtype;
      const struct ReturnValue _3 = dong_porf_porf_todo_parseIndexStr(0, 0, 0, 0, _get21, _get22);
      jjlast_type = _3.type;
      jjreturn = _3.value;
      _get23 = jjlast_type;
      jjreturnjjtype = _get23;
      _get24 = jjnewtarget;
      // if 
        if (((u32)(_get24)) != 0) {
          _get25 = jjreturn;
          _get26 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get25), _get26)) == 0) {
              _get27 = jjthis;
              _get28 = jjthisjjtype;
              return (struct ReturnValue){ _get27, _get28 };
            }
          // end
          j1119:;
        }
      // end
      j1118:;
      _get29 = jjreturn;
      _get30 = jjreturnjjtype;
      return (struct ReturnValue){ _get29, _get30 };
    }
  // end
  j1067:;
  _get31 = target;
  _get32 = targetjjtype;
  const struct ReturnValue _4 = dong_porf_porf_todo_closestSelector(0, 0, 0, 0, _get31, _get32, 3306, 195);
  jjlast_type = _4.type;
  _get33 = jjlast_type;
  hitjjtype = _get33;
  hit = _4.value;
  _get34 = hit;
  _get35 = hitjjtype;
  // if 
    if ((f64)((_get34 != 0) | ((_get35 | 128) != (1 | 128))) != 0) {
      _get36 = hit;
      _get37 = hitjjtype;
      const struct ReturnValue _5 = dong_porf_porf_todo_getAttribute(0, 0, 0, 0, _get36, _get37, 3174, 195);
      jjlast_type = _5.type;
      _get38 = jjlast_type;
      vjjtype = _get38;
      v = _5.value;
      _get39 = v;
      _get40 = vjjtype;
      const struct ReturnValue _6 = dong_porf_porf_todo_parseIndexStr(0, 0, 0, 0, _get39, _get40);
      jjlast_type = _6.type;
      jjreturn = _6.value;
      _get41 = jjlast_type;
      jjreturnjjtype = _get41;
      _get42 = jjnewtarget;
      // if 
        if (((u32)(_get42)) != 0) {
          _get43 = jjreturn;
          _get44 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get43), _get44)) == 0) {
              _get45 = jjthis;
              _get46 = jjthisjjtype;
              return (struct ReturnValue){ _get45, _get46 };
            }
          // end
          j1124:;
        }
      // end
      j1123:;
      _get47 = jjreturn;
      _get48 = jjreturnjjtype;
      return (struct ReturnValue){ _get47, _get48 };
    }
  // end
  j1122:;
  jjreturn = -1;
  jjreturnjjtype = 1;
  _get49 = jjnewtarget;
  // if 
    if (((u32)(_get49)) != 0) {
      _get50 = jjreturn;
      _get51 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get50), _get51)) == 0) {
          _get52 = jjthis;
          _get53 = jjthisjjtype;
          return (struct ReturnValue){ _get52, _get53 };
        }
      // end
      j1126:;
    }
  // end
  j1125:;
  _get54 = jjreturn;
  _get55 = jjreturnjjtype;
  return (struct ReturnValue){ _get54, _get55 };
}

static struct ReturnValue dong_porf_porf_todo_setTodoDoneAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype, f64 v, i32 vjjtype) {
  i32 _get109;
  f64 _get108;
  f64 _get107;
  i32 _get106;
  f64 _get105;
  i32 _get104;
  f64 _get103;
  f64 _get102;
  i32 _get101;
  f64 _get100;
  i32 _get99;
  f64 _get98;
  i32 _get97;
  f64 _get96;
  f64 _get95;
  i32 _get94;
  f64 _get93;
  i32 _get92;
  f64 _get91;
  i32 _get90;
  f64 _get89;
  f64 _get88;
  i32 _get87;
  f64 _get86;
  i32 _get85;
  f64 _get84;
  i32 _get83;
  f64 _get82;
  f64 _get81;
  i32 _get80;
  f64 _get79;
  i32 _get78;
  f64 _get77;
  i32 _get76;
  f64 _get75;
  f64 _get74;
  i32 _get73;
  f64 _get72;
  i32 _get71;
  f64 _get70;
  i32 _get69;
  f64 _get68;
  f64 _get67;
  i32 _get66;
  f64 _get65;
  i32 _get64;
  f64 _get63;
  i32 _get62;
  f64 _get61;
  f64 _get60;
  i32 _get59;
  f64 _get58;
  i32 _get57;
  f64 _get56;
  i32 _get55;
  f64 _get54;
  f64 _get53;
  i32 _get52;
  f64 _get51;
  i32 _get50;
  f64 _get49;
  i32 _get48;
  f64 _get47;
  f64 _get46;
  i32 _get45;
  f64 _get44;
  i32 _get43;
  f64 _get42;
  i32 _get41;
  f64 _get40;
  f64 _get39;
  i32 _get38;
  f64 _get37;
  i32 _get36;
  f64 _get35;
  i32 _get34;
  f64 _get33;
  f64 _get32;
  i32 _get31;
  f64 _get30;
  i32 _get29;
  f64 _get28;
  i32 _get27;
  f64 _get26;
  f64 _get25;
  i32 _get24;
  f64 _get23;
  i32 _get22;
  f64 _get21;
  i32 _get20;
  f64 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  f64 _get14;
  i32 _get13;
  f64 _get12;
  f64 _get11;
  i32 _get10;
  f64 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  f64 _get5;
  f64 _get4;
  i32 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  _get0 = i;
  _get1 = ijjtype;
  // if 
    if ((f64)((_get0 == 0) & ((_get1 | 128) == (1 | 128))) != 0) {
      _get2 = v;
      dong_porf_porf_todo_todoDone0 = _get2;
      _get3 = vjjtype;
      dong_porf_porf_todo_todoDone0jjtype = _get3;
      (void) dong_porf_porf_todo_todoDone0;
      _get4 = jjnewtarget;
      // if 
        if (((u32)(_get4)) != 0) {
          _get5 = jjthis;
          _get6 = jjthisjjtype;
          return (struct ReturnValue){ _get5, _get6 };
        }
      // end
      j1134:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1133:;
  _get7 = i;
  _get8 = ijjtype;
  // if 
    if ((f64)((_get7 == 1) & ((_get8 | 128) == (1 | 128))) != 0) {
      _get9 = v;
      dong_porf_porf_todo_todoDone1 = _get9;
      _get10 = vjjtype;
      dong_porf_porf_todo_todoDone1jjtype = _get10;
      (void) dong_porf_porf_todo_todoDone1;
      _get11 = jjnewtarget;
      // if 
        if (((u32)(_get11)) != 0) {
          _get12 = jjthis;
          _get13 = jjthisjjtype;
          return (struct ReturnValue){ _get12, _get13 };
        }
      // end
      j1136:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1135:;
  _get14 = i;
  _get15 = ijjtype;
  // if 
    if ((f64)((_get14 == 2) & ((_get15 | 128) == (1 | 128))) != 0) {
      _get16 = v;
      dong_porf_porf_todo_todoDone2 = _get16;
      _get17 = vjjtype;
      dong_porf_porf_todo_todoDone2jjtype = _get17;
      (void) dong_porf_porf_todo_todoDone2;
      _get18 = jjnewtarget;
      // if 
        if (((u32)(_get18)) != 0) {
          _get19 = jjthis;
          _get20 = jjthisjjtype;
          return (struct ReturnValue){ _get19, _get20 };
        }
      // end
      j1138:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1137:;
  _get21 = i;
  _get22 = ijjtype;
  // if 
    if ((f64)((_get21 == 3) & ((_get22 | 128) == (1 | 128))) != 0) {
      _get23 = v;
      dong_porf_porf_todo_todoDone3 = _get23;
      _get24 = vjjtype;
      dong_porf_porf_todo_todoDone3jjtype = _get24;
      (void) dong_porf_porf_todo_todoDone3;
      _get25 = jjnewtarget;
      // if 
        if (((u32)(_get25)) != 0) {
          _get26 = jjthis;
          _get27 = jjthisjjtype;
          return (struct ReturnValue){ _get26, _get27 };
        }
      // end
      j1140:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1139:;
  _get28 = i;
  _get29 = ijjtype;
  // if 
    if ((f64)((_get28 == 4) & ((_get29 | 128) == (1 | 128))) != 0) {
      _get30 = v;
      dong_porf_porf_todo_todoDone4 = _get30;
      _get31 = vjjtype;
      dong_porf_porf_todo_todoDone4jjtype = _get31;
      (void) dong_porf_porf_todo_todoDone4;
      _get32 = jjnewtarget;
      // if 
        if (((u32)(_get32)) != 0) {
          _get33 = jjthis;
          _get34 = jjthisjjtype;
          return (struct ReturnValue){ _get33, _get34 };
        }
      // end
      j1142:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1141:;
  _get35 = i;
  _get36 = ijjtype;
  // if 
    if ((f64)((_get35 == 5) & ((_get36 | 128) == (1 | 128))) != 0) {
      _get37 = v;
      dong_porf_porf_todo_todoDone5 = _get37;
      _get38 = vjjtype;
      dong_porf_porf_todo_todoDone5jjtype = _get38;
      (void) dong_porf_porf_todo_todoDone5;
      _get39 = jjnewtarget;
      // if 
        if (((u32)(_get39)) != 0) {
          _get40 = jjthis;
          _get41 = jjthisjjtype;
          return (struct ReturnValue){ _get40, _get41 };
        }
      // end
      j1144:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1143:;
  _get42 = i;
  _get43 = ijjtype;
  // if 
    if ((f64)((_get42 == 6) & ((_get43 | 128) == (1 | 128))) != 0) {
      _get44 = v;
      dong_porf_porf_todo_todoDone6 = _get44;
      _get45 = vjjtype;
      dong_porf_porf_todo_todoDone6jjtype = _get45;
      (void) dong_porf_porf_todo_todoDone6;
      _get46 = jjnewtarget;
      // if 
        if (((u32)(_get46)) != 0) {
          _get47 = jjthis;
          _get48 = jjthisjjtype;
          return (struct ReturnValue){ _get47, _get48 };
        }
      // end
      j1146:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1145:;
  _get49 = i;
  _get50 = ijjtype;
  // if 
    if ((f64)((_get49 == 7) & ((_get50 | 128) == (1 | 128))) != 0) {
      _get51 = v;
      dong_porf_porf_todo_todoDone7 = _get51;
      _get52 = vjjtype;
      dong_porf_porf_todo_todoDone7jjtype = _get52;
      (void) dong_porf_porf_todo_todoDone7;
      _get53 = jjnewtarget;
      // if 
        if (((u32)(_get53)) != 0) {
          _get54 = jjthis;
          _get55 = jjthisjjtype;
          return (struct ReturnValue){ _get54, _get55 };
        }
      // end
      j1148:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1147:;
  _get56 = i;
  _get57 = ijjtype;
  // if 
    if ((f64)((_get56 == 8) & ((_get57 | 128) == (1 | 128))) != 0) {
      _get58 = v;
      dong_porf_porf_todo_todoDone8 = _get58;
      _get59 = vjjtype;
      dong_porf_porf_todo_todoDone8jjtype = _get59;
      (void) dong_porf_porf_todo_todoDone8;
      _get60 = jjnewtarget;
      // if 
        if (((u32)(_get60)) != 0) {
          _get61 = jjthis;
          _get62 = jjthisjjtype;
          return (struct ReturnValue){ _get61, _get62 };
        }
      // end
      j1150:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1149:;
  _get63 = i;
  _get64 = ijjtype;
  // if 
    if ((f64)((_get63 == 9) & ((_get64 | 128) == (1 | 128))) != 0) {
      _get65 = v;
      dong_porf_porf_todo_todoDone9 = _get65;
      _get66 = vjjtype;
      dong_porf_porf_todo_todoDone9jjtype = _get66;
      (void) dong_porf_porf_todo_todoDone9;
      _get67 = jjnewtarget;
      // if 
        if (((u32)(_get67)) != 0) {
          _get68 = jjthis;
          _get69 = jjthisjjtype;
          return (struct ReturnValue){ _get68, _get69 };
        }
      // end
      j1152:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1151:;
  _get70 = i;
  _get71 = ijjtype;
  // if 
    if ((f64)((_get70 == 10) & ((_get71 | 128) == (1 | 128))) != 0) {
      _get72 = v;
      dong_porf_porf_todo_todoDone10 = _get72;
      _get73 = vjjtype;
      dong_porf_porf_todo_todoDone10jjtype = _get73;
      (void) dong_porf_porf_todo_todoDone10;
      _get74 = jjnewtarget;
      // if 
        if (((u32)(_get74)) != 0) {
          _get75 = jjthis;
          _get76 = jjthisjjtype;
          return (struct ReturnValue){ _get75, _get76 };
        }
      // end
      j1154:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1153:;
  _get77 = i;
  _get78 = ijjtype;
  // if 
    if ((f64)((_get77 == 11) & ((_get78 | 128) == (1 | 128))) != 0) {
      _get79 = v;
      dong_porf_porf_todo_todoDone11 = _get79;
      _get80 = vjjtype;
      dong_porf_porf_todo_todoDone11jjtype = _get80;
      (void) dong_porf_porf_todo_todoDone11;
      _get81 = jjnewtarget;
      // if 
        if (((u32)(_get81)) != 0) {
          _get82 = jjthis;
          _get83 = jjthisjjtype;
          return (struct ReturnValue){ _get82, _get83 };
        }
      // end
      j1156:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1155:;
  _get84 = i;
  _get85 = ijjtype;
  // if 
    if ((f64)((_get84 == 12) & ((_get85 | 128) == (1 | 128))) != 0) {
      _get86 = v;
      dong_porf_porf_todo_todoDone12 = _get86;
      _get87 = vjjtype;
      dong_porf_porf_todo_todoDone12jjtype = _get87;
      (void) dong_porf_porf_todo_todoDone12;
      _get88 = jjnewtarget;
      // if 
        if (((u32)(_get88)) != 0) {
          _get89 = jjthis;
          _get90 = jjthisjjtype;
          return (struct ReturnValue){ _get89, _get90 };
        }
      // end
      j1158:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1157:;
  _get91 = i;
  _get92 = ijjtype;
  // if 
    if ((f64)((_get91 == 13) & ((_get92 | 128) == (1 | 128))) != 0) {
      _get93 = v;
      dong_porf_porf_todo_todoDone13 = _get93;
      _get94 = vjjtype;
      dong_porf_porf_todo_todoDone13jjtype = _get94;
      (void) dong_porf_porf_todo_todoDone13;
      _get95 = jjnewtarget;
      // if 
        if (((u32)(_get95)) != 0) {
          _get96 = jjthis;
          _get97 = jjthisjjtype;
          return (struct ReturnValue){ _get96, _get97 };
        }
      // end
      j1160:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1159:;
  _get98 = i;
  _get99 = ijjtype;
  // if 
    if ((f64)((_get98 == 14) & ((_get99 | 128) == (1 | 128))) != 0) {
      _get100 = v;
      dong_porf_porf_todo_todoDone14 = _get100;
      _get101 = vjjtype;
      dong_porf_porf_todo_todoDone14jjtype = _get101;
      (void) dong_porf_porf_todo_todoDone14;
      _get102 = jjnewtarget;
      // if 
        if (((u32)(_get102)) != 0) {
          _get103 = jjthis;
          _get104 = jjthisjjtype;
          return (struct ReturnValue){ _get103, _get104 };
        }
      // end
      j1162:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1161:;
  _get105 = v;
  dong_porf_porf_todo_todoDone15 = _get105;
  _get106 = vjjtype;
  dong_porf_porf_todo_todoDone15jjtype = _get106;
  (void) dong_porf_porf_todo_todoDone15;
  _get107 = jjnewtarget;
  // if 
    if (((u32)(_get107)) != 0) {
      _get108 = jjthis;
      _get109 = jjthisjjtype;
      return (struct ReturnValue){ _get108, _get109 };
    }
  // end
  j1163:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_readDeleteIndex(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  i32 _get51;
  f64 _get50;
  f64 _get49;
  i32 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  i32 _get44;
  f64 _get43;
  f64 _get42;
  i32 _get41;
  i32 _get40;
  f64 _get39;
  i32 _get38;
  i32 _get37;
  f64 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  i32 _get32;
  f64 _get31;
  i32 _get30;
  f64 _get29;
  i32 _get28;
  f64 _get27;
  i32 _get26;
  f64 _get25;
  f64 _get24;
  i32 _get23;
  i32 _get22;
  f64 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  i32 _get15;
  i32 _get14;
  f64 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 target = 0;
  i32 targetjjtype = 0;
  i32 jjlast_type = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;
  f64 v = 0;
  i32 vjjtype = 0;
  i32 jjlength_tmp = 0;
  f64 jjmember_obj_169 = 0;
  f64 jjmember_prop_169 = 0;
  f64 hit = 0;
  i32 hitjjtype = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_eventTarget(0, 0, 0, 0);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  targetjjtype = _get0;
  target = _0.value;
  _get1 = target;
  _get2 = targetjjtype;
  // if 
    if ((f64)((_get1 == 0) & ((_get2 | 128) == (1 | 128))) != 0) {
      jjreturn = -1;
      jjreturnjjtype = 1;
      _get3 = jjnewtarget;
      // if 
        if (((u32)(_get3)) != 0) {
          _get4 = jjreturn;
          _get5 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get4), _get5)) == 0) {
              _get6 = jjthis;
              _get7 = jjthisjjtype;
              return (struct ReturnValue){ _get6, _get7 };
            }
          // end
          j1167:;
        }
      // end
      j1166:;
      _get8 = jjreturn;
      _get9 = jjreturnjjtype;
      return (struct ReturnValue){ _get8, _get9 };
    }
  // end
  j1165:;
  _get10 = target;
  _get11 = targetjjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo_getAttribute(0, 0, 0, 0, _get10, _get11, 3336, 195);
  jjlast_type = _1.type;
  _get12 = jjlast_type;
  vjjtype = _get12;
  v = _1.value;
  _get13 = v;
  jjlength_tmp = (u32)(_get13);
  _get14 = vjjtype;
  // if f64
  f64 _r1168;
    if ((_get14 & 64) != 0) {
      _get15 = jjlength_tmp;
      jjlast_type = 1;
      _r1168 = (f64)(i32_load(1, 0, _get15));
    } else {
      jjmember_prop_169 = 564;
      _get16 = v;
      jjmember_obj_169 = _get16;
      _get17 = vjjtype;
      // if f64
      f64 _r1169;
        if (_get17 == 0) {
          _r1169 = 0;
        } else {
          _get18 = jjmember_obj_169;
          _get19 = vjjtype;
          _get20 = jjmember_prop_169;
          const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get18), _get19, (u32)(_get20), 195, -2086110260, 1);
          jjlast_type = _2.type;
          _r1169 = _2.value;
        }
      // end
      j1169:;
      _r1168 = _r1169;
    }
  // end
  j1168:;
  // if 
    if ((f64)(_r1168 > 0) != 0) {
      _get21 = v;
      _get22 = vjjtype;
      const struct ReturnValue _3 = dong_porf_porf_todo_parseIndexStr(0, 0, 0, 0, _get21, _get22);
      jjlast_type = _3.type;
      jjreturn = _3.value;
      _get23 = jjlast_type;
      jjreturnjjtype = _get23;
      _get24 = jjnewtarget;
      // if 
        if (((u32)(_get24)) != 0) {
          _get25 = jjreturn;
          _get26 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get25), _get26)) == 0) {
              _get27 = jjthis;
              _get28 = jjthisjjtype;
              return (struct ReturnValue){ _get27, _get28 };
            }
          // end
          j1172:;
        }
      // end
      j1171:;
      _get29 = jjreturn;
      _get30 = jjreturnjjtype;
      return (struct ReturnValue){ _get29, _get30 };
    }
  // end
  j1170:;
  _get31 = target;
  _get32 = targetjjtype;
  const struct ReturnValue _4 = dong_porf_porf_todo_closestSelector(0, 0, 0, 0, _get31, _get32, 3364, 195);
  jjlast_type = _4.type;
  _get33 = jjlast_type;
  hitjjtype = _get33;
  hit = _4.value;
  _get34 = hit;
  _get35 = hitjjtype;
  // if 
    if ((f64)((_get34 != 0) | ((_get35 | 128) != (1 | 128))) != 0) {
      _get36 = hit;
      _get37 = hitjjtype;
      const struct ReturnValue _5 = dong_porf_porf_todo_getAttribute(0, 0, 0, 0, _get36, _get37, 3336, 195);
      jjlast_type = _5.type;
      _get38 = jjlast_type;
      vjjtype = _get38;
      v = _5.value;
      _get39 = v;
      _get40 = vjjtype;
      const struct ReturnValue _6 = dong_porf_porf_todo_parseIndexStr(0, 0, 0, 0, _get39, _get40);
      jjlast_type = _6.type;
      jjreturn = _6.value;
      _get41 = jjlast_type;
      jjreturnjjtype = _get41;
      _get42 = jjnewtarget;
      // if 
        if (((u32)(_get42)) != 0) {
          _get43 = jjreturn;
          _get44 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get43), _get44)) == 0) {
              _get45 = jjthis;
              _get46 = jjthisjjtype;
              return (struct ReturnValue){ _get45, _get46 };
            }
          // end
          j1175:;
        }
      // end
      j1174:;
      _get47 = jjreturn;
      _get48 = jjreturnjjtype;
      return (struct ReturnValue){ _get47, _get48 };
    }
  // end
  j1173:;
  jjreturn = -1;
  jjreturnjjtype = 1;
  _get49 = jjnewtarget;
  // if 
    if (((u32)(_get49)) != 0) {
      _get50 = jjreturn;
      _get51 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get50), _get51)) == 0) {
          _get52 = jjthis;
          _get53 = jjthisjjtype;
          return (struct ReturnValue){ _get52, _get53 };
        }
      // end
      j1177:;
    }
  // end
  j1176:;
  _get54 = jjreturn;
  _get55 = jjreturnjjtype;
  return (struct ReturnValue){ _get54, _get55 };
}

struct ReturnValue dong_porf_porf_todo_onListClick(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get32;
  f64 _get31;
  f64 _get30;
  i32 _get29;
  f64 _get28;
  i32 _get27;
  f64 _get26;
  i32 _get25;
  f64 _get24;
  i32 _get23;
  i32 _get22;
  f64 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  f64 _get16;
  f64 _get15;
  i32 _get14;
  i32 _get13;
  f64 _get12;
  i32 _get11;
  i32 _get10;
  i32 _get9;
  f64 _get8;
  i32 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  f64 idx = 0;
  i32 idxjjtype = 0;
  i32 jjlast_type = 0;
  i32 logictmpi = 0;
  f64 d = 0;
  i32 djjtype = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 idx2 = 0;
  i32 idx2jjtype = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_readToggleIndex(0, 0, 0, 0);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  idxjjtype = _get0;
  idx = _0.value;
  _get1 = idx;
  logictmpi = _get1 >= 0;
  _get2 = logictmpi;
  // if i32
  i32 _r1127;
    if (_get2 != 0) {
      _get3 = idx;
      jjlast_type = 2;
      _r1127 = _get3 < dong_porf_porf_todo_todoCount;
    } else {
      _get4 = logictmpi;
      jjlast_type = 2;
      _r1127 = _get4;
    }
  // end
  j1127:;
  // if 
    if ((f64)(_r1127) != 0) {
      _get5 = idx;
      _get6 = idxjjtype;
      const struct ReturnValue _1 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get5, _get6);
      jjlast_type = _1.type;
      _get7 = jjlast_type;
      djjtype = _get7;
      d = _1.value;
      _get8 = d;
      jjlogicinner_tmp = _get8;
      _get9 = djjtype;
      jjtypeswitch_tmp1 = _get9;
      // block i32
      i32 _r1129;
        _get10 = jjtypeswitch_tmp1;
        _get11 = jjtypeswitch_tmp1;
        // if 
          if (((_get10 == 67) | (_get11 == 195)) != 0) {
            _get12 = jjlogicinner_tmp;
            _r1129 = i32_load(1, 0, (u32)(_get12));
            goto j1129;
          }
        // end
        j1130:;
        _get13 = jjtypeswitch_tmp1;
        _get14 = jjtypeswitch_tmp1;
        // if 
          if (((_get13 == 31) | (_get14 == 32)) != 0) {
            _r1129 = 1;
            goto j1129;
          }
        // end
        j1131:;
        _get15 = jjlogicinner_tmp;
        const f64 _tmp0 = _get15;
        _r1129 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
      // end
      j1129:;
      // if 
        if ((_r1129) != 0) {
          _get16 = idx;
          _get17 = idxjjtype;
          const struct ReturnValue _2 = dong_porf_porf_todo_setTodoDoneAt(0, 0, 0, 0, _get16, _get17, 0, 1);
          jjlast_type = _2.type;
          (void) _2.value;
        } else {
          _get18 = idx;
          _get19 = idxjjtype;
          const struct ReturnValue _3 = dong_porf_porf_todo_setTodoDoneAt(0, 0, 0, 0, _get18, _get19, 1, 1);
          jjlast_type = _3.type;
          (void) _3.value;
        }
      // end
      j1132:;
      const struct ReturnValue _4 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
      jjlast_type = _4.type;
      (void) _4.value;
      _get20 = jjnewtarget;
      // if 
        if (((u32)(_get20)) != 0) {
          _get21 = jjthis;
          _get22 = jjthisjjtype;
          return (struct ReturnValue){ _get21, _get22 };
        }
      // end
      j1164:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1128:;
  const struct ReturnValue _5 = dong_porf_porf_todo_readDeleteIndex(0, 0, 0, 0);
  jjlast_type = _5.type;
  _get23 = jjlast_type;
  idx2jjtype = _get23;
  idx2 = _5.value;
  _get24 = idx2;
  logictmpi = _get24 >= 0;
  _get25 = logictmpi;
  // if i32
  i32 _r1178;
    if (_get25 != 0) {
      _get26 = idx2;
      jjlast_type = 2;
      _r1178 = _get26 < dong_porf_porf_todo_todoCount;
    } else {
      _get27 = logictmpi;
      jjlast_type = 2;
      _r1178 = _get27;
    }
  // end
  j1178:;
  // if 
    if ((f64)(_r1178) != 0) {
      _get28 = idx2;
      _get29 = idx2jjtype;
      const struct ReturnValue _6 = dong_porf_porf_todo_removeAtIndex(0, 0, 0, 0, _get28, _get29);
      jjlast_type = _6.type;
      (void) _6.value;
      const struct ReturnValue _7 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
      jjlast_type = _7.type;
      (void) _7.value;
    }
  // end
  j1179:;
  _get30 = jjnewtarget;
  // if 
    if (((u32)(_get30)) != 0) {
      _get31 = jjthis;
      _get32 = jjthisjjtype;
      return (struct ReturnValue){ _get31, _get32 };
    }
  // end
  j1180:;
  return (struct ReturnValue){ 0, 0 };
}

int dong_porf_porf_todo_export_onAdd(void) {
  dong_porf_porf_todo__porf_init();
  (void)dong_porf_porf_todo_onAdd(0, 0, 0, 0);
  return 0;
}

int dong_porf_porf_todo_export_onInputChange(void) {
  dong_porf_porf_todo__porf_init();
  (void)dong_porf_porf_todo_onInputChange(0, 0, 0, 0);
  return 0;
}

int dong_porf_porf_todo_export_onKeyDown(void) {
  dong_porf_porf_todo__porf_init();
  (void)dong_porf_porf_todo_onKeyDown(0, 0, 0, 0);
  return 0;
}

int dong_porf_porf_todo_export_onFilterAll(void) {
  dong_porf_porf_todo__porf_init();
  (void)dong_porf_porf_todo_onFilterAll(0, 0, 0, 0);
  return 0;
}

int dong_porf_porf_todo_export_onFilterActive(void) {
  dong_porf_porf_todo__porf_init();
  (void)dong_porf_porf_todo_onFilterActive(0, 0, 0, 0);
  return 0;
}

int dong_porf_porf_todo_export_onFilterDone(void) {
  dong_porf_porf_todo__porf_init();
  (void)dong_porf_porf_todo_onFilterDone(0, 0, 0, 0);
  return 0;
}

int dong_porf_porf_todo_export_onClearDone(void) {
  dong_porf_porf_todo__porf_init();
  (void)dong_porf_porf_todo_onClearDone(0, 0, 0, 0);
  return 0;
}

int dong_porf_porf_todo_export_onListClick(void) {
  dong_porf_porf_todo__porf_init();
  (void)dong_porf_porf_todo_onListClick(0, 0, 0, 0);
  return 0;
}

typedef struct {
  f64 dong_porf_porf_todo_METRIC_OFFSET_WIDTH;
  i32 dong_porf_porf_todo_METRIC_OFFSET_WIDTHjjtype;
  f64 dong_porf_porf_todo_METRIC_OFFSET_HEIGHT;
  i32 dong_porf_porf_todo_METRIC_OFFSET_HEIGHTjjtype;
  f64 dong_porf_porf_todo_METRIC_OFFSET_TOP;
  i32 dong_porf_porf_todo_METRIC_OFFSET_TOPjjtype;
  f64 dong_porf_porf_todo_METRIC_OFFSET_LEFT;
  i32 dong_porf_porf_todo_METRIC_OFFSET_LEFTjjtype;
  f64 dong_porf_porf_todo_METRIC_CLIENT_WIDTH;
  i32 dong_porf_porf_todo_METRIC_CLIENT_WIDTHjjtype;
  f64 dong_porf_porf_todo_METRIC_CLIENT_HEIGHT;
  i32 dong_porf_porf_todo_METRIC_CLIENT_HEIGHTjjtype;
  f64 dong_porf_porf_todo_METRIC_SCROLL_WIDTH;
  i32 dong_porf_porf_todo_METRIC_SCROLL_WIDTHjjtype;
  f64 dong_porf_porf_todo_METRIC_SCROLL_HEIGHT;
  i32 dong_porf_porf_todo_METRIC_SCROLL_HEIGHTjjtype;
  f64 dong_porf_porf_todo_MAX_TODOS;
  i32 dong_porf_porf_todo_MAX_TODOSjjtype;
  f64 dong_porf_porf_todo_todoCount;
  i32 dong_porf_porf_todo_todoCountjjtype;
  f64 dong_porf_porf_todo_nextId;
  i32 dong_porf_porf_todo_nextIdjjtype;
  f64 dong_porf_porf_todo_filterMode;
  i32 dong_porf_porf_todo_filterModejjtype;
  f64 dong_porf_porf_todo_inputText;
  i32 dong_porf_porf_todo_inputTextjjtype;
  f64 dong_porf_porf_todo_todoId0;
  i32 dong_porf_porf_todo_todoId0jjtype;
  f64 dong_porf_porf_todo_todoText0;
  i32 dong_porf_porf_todo_todoText0jjtype;
  f64 dong_porf_porf_todo_todoDone0;
  i32 dong_porf_porf_todo_todoDone0jjtype;
  f64 dong_porf_porf_todo_todoId1;
  i32 dong_porf_porf_todo_todoId1jjtype;
  f64 dong_porf_porf_todo_todoText1;
  i32 dong_porf_porf_todo_todoText1jjtype;
  f64 dong_porf_porf_todo_todoDone1;
  i32 dong_porf_porf_todo_todoDone1jjtype;
  f64 dong_porf_porf_todo_todoId2;
  i32 dong_porf_porf_todo_todoId2jjtype;
  f64 dong_porf_porf_todo_todoText2;
  i32 dong_porf_porf_todo_todoText2jjtype;
  f64 dong_porf_porf_todo_todoDone2;
  i32 dong_porf_porf_todo_todoDone2jjtype;
  f64 dong_porf_porf_todo_todoId3;
  i32 dong_porf_porf_todo_todoId3jjtype;
  f64 dong_porf_porf_todo_todoText3;
  i32 dong_porf_porf_todo_todoText3jjtype;
  f64 dong_porf_porf_todo_todoDone3;
  i32 dong_porf_porf_todo_todoDone3jjtype;
  f64 dong_porf_porf_todo_todoInputId;
  i32 dong_porf_porf_todo_todoInputIdjjtype;
  f64 dong_porf_porf_todo_btnAddId;
  i32 dong_porf_porf_todo_btnAddIdjjtype;
  f64 dong_porf_porf_todo_filterAllId;
  i32 dong_porf_porf_todo_filterAllIdjjtype;
  f64 dong_porf_porf_todo_filterActiveId;
  i32 dong_porf_porf_todo_filterActiveIdjjtype;
  f64 dong_porf_porf_todo_filterDoneId;
  i32 dong_porf_porf_todo_filterDoneIdjjtype;
  f64 dong_porf_porf_todo_todoListId;
  i32 dong_porf_porf_todo_todoListIdjjtype;
  f64 dong_porf_porf_todo_clearWrapId;
  i32 dong_porf_porf_todo_clearWrapIdjjtype;
  f64 dong_porf_porf_todo_btnClearId;
  i32 dong_porf_porf_todo_btnClearIdjjtype;
  f64 dong_porf_porf_todo_todoId4;
  i32 dong_porf_porf_todo_todoId4jjtype;
  f64 dong_porf_porf_todo_todoText4;
  i32 dong_porf_porf_todo_todoText4jjtype;
  f64 dong_porf_porf_todo_todoDone4;
  i32 dong_porf_porf_todo_todoDone4jjtype;
  f64 dong_porf_porf_todo_todoId5;
  i32 dong_porf_porf_todo_todoId5jjtype;
  f64 dong_porf_porf_todo_todoText5;
  i32 dong_porf_porf_todo_todoText5jjtype;
  f64 dong_porf_porf_todo_todoDone5;
  i32 dong_porf_porf_todo_todoDone5jjtype;
  f64 dong_porf_porf_todo_todoId6;
  i32 dong_porf_porf_todo_todoId6jjtype;
  f64 dong_porf_porf_todo_todoText6;
  i32 dong_porf_porf_todo_todoText6jjtype;
  f64 dong_porf_porf_todo_todoDone6;
  i32 dong_porf_porf_todo_todoDone6jjtype;
  f64 dong_porf_porf_todo_todoId7;
  i32 dong_porf_porf_todo_todoId7jjtype;
  f64 dong_porf_porf_todo_todoText7;
  i32 dong_porf_porf_todo_todoText7jjtype;
  f64 dong_porf_porf_todo_todoDone7;
  i32 dong_porf_porf_todo_todoDone7jjtype;
  f64 dong_porf_porf_todo_todoId8;
  i32 dong_porf_porf_todo_todoId8jjtype;
  f64 dong_porf_porf_todo_todoText8;
  i32 dong_porf_porf_todo_todoText8jjtype;
  f64 dong_porf_porf_todo_todoDone8;
  i32 dong_porf_porf_todo_todoDone8jjtype;
  f64 dong_porf_porf_todo_todoId9;
  i32 dong_porf_porf_todo_todoId9jjtype;
  f64 dong_porf_porf_todo_todoText9;
  i32 dong_porf_porf_todo_todoText9jjtype;
  f64 dong_porf_porf_todo_todoDone9;
  i32 dong_porf_porf_todo_todoDone9jjtype;
  f64 dong_porf_porf_todo_todoId10;
  i32 dong_porf_porf_todo_todoId10jjtype;
  f64 dong_porf_porf_todo_todoText10;
  i32 dong_porf_porf_todo_todoText10jjtype;
  f64 dong_porf_porf_todo_todoDone10;
  i32 dong_porf_porf_todo_todoDone10jjtype;
  f64 dong_porf_porf_todo_todoId11;
  i32 dong_porf_porf_todo_todoId11jjtype;
  f64 dong_porf_porf_todo_todoText11;
  i32 dong_porf_porf_todo_todoText11jjtype;
  f64 dong_porf_porf_todo_todoDone11;
  i32 dong_porf_porf_todo_todoDone11jjtype;
  f64 dong_porf_porf_todo_todoId12;
  i32 dong_porf_porf_todo_todoId12jjtype;
  f64 dong_porf_porf_todo_todoText12;
  i32 dong_porf_porf_todo_todoText12jjtype;
  f64 dong_porf_porf_todo_todoDone12;
  i32 dong_porf_porf_todo_todoDone12jjtype;
  f64 dong_porf_porf_todo_todoId13;
  i32 dong_porf_porf_todo_todoId13jjtype;
  f64 dong_porf_porf_todo_todoText13;
  i32 dong_porf_porf_todo_todoText13jjtype;
  f64 dong_porf_porf_todo_todoDone13;
  i32 dong_porf_porf_todo_todoDone13jjtype;
  f64 dong_porf_porf_todo_todoId14;
  i32 dong_porf_porf_todo_todoId14jjtype;
  f64 dong_porf_porf_todo_todoText14;
  i32 dong_porf_porf_todo_todoText14jjtype;
  f64 dong_porf_porf_todo_todoDone14;
  i32 dong_porf_porf_todo_todoDone14jjtype;
  f64 dong_porf_porf_todo_todoId15;
  i32 dong_porf_porf_todo_todoId15jjtype;
  f64 dong_porf_porf_todo_todoText15;
  i32 dong_porf_porf_todo_todoText15jjtype;
  f64 dong_porf_porf_todo_todoDone15;
  i32 dong_porf_porf_todo_todoDone15jjtype;
  i32 dong_porf_porf_todo_jjporfjjcurrentPtr;
  i32 dong_porf_porf_todo_jjporfjjcurrentPtrjjglbl_inited;
  i32 dong_porf_porf_todo_jjporfjjendPtr;
  i32 dong_porf_porf_todo_jjporfjjendPtrjjglbl_inited;
  i32 dong_porf_porf_todo_jjporfjjunderlyingStore;
  i32 dong_porf_porf_todo_jjporfjjunderlyingStorejjglbl_inited;
  i32 dong_porf_porf_todo_jjporfjjgetptr___Object_prototype;
  i32 dong_porf_porf_todo_jjporfjjgetptr___Object_prototypejjglbl_inited;
} dong_porf_porf_todo_state_t;

void dong_porf_porf_todo_state_capture(dong_porf_porf_todo_state_t* out) {
  out->dong_porf_porf_todo_METRIC_OFFSET_WIDTH = dong_porf_porf_todo_METRIC_OFFSET_WIDTH;
  out->dong_porf_porf_todo_METRIC_OFFSET_WIDTHjjtype = dong_porf_porf_todo_METRIC_OFFSET_WIDTHjjtype;
  out->dong_porf_porf_todo_METRIC_OFFSET_HEIGHT = dong_porf_porf_todo_METRIC_OFFSET_HEIGHT;
  out->dong_porf_porf_todo_METRIC_OFFSET_HEIGHTjjtype = dong_porf_porf_todo_METRIC_OFFSET_HEIGHTjjtype;
  out->dong_porf_porf_todo_METRIC_OFFSET_TOP = dong_porf_porf_todo_METRIC_OFFSET_TOP;
  out->dong_porf_porf_todo_METRIC_OFFSET_TOPjjtype = dong_porf_porf_todo_METRIC_OFFSET_TOPjjtype;
  out->dong_porf_porf_todo_METRIC_OFFSET_LEFT = dong_porf_porf_todo_METRIC_OFFSET_LEFT;
  out->dong_porf_porf_todo_METRIC_OFFSET_LEFTjjtype = dong_porf_porf_todo_METRIC_OFFSET_LEFTjjtype;
  out->dong_porf_porf_todo_METRIC_CLIENT_WIDTH = dong_porf_porf_todo_METRIC_CLIENT_WIDTH;
  out->dong_porf_porf_todo_METRIC_CLIENT_WIDTHjjtype = dong_porf_porf_todo_METRIC_CLIENT_WIDTHjjtype;
  out->dong_porf_porf_todo_METRIC_CLIENT_HEIGHT = dong_porf_porf_todo_METRIC_CLIENT_HEIGHT;
  out->dong_porf_porf_todo_METRIC_CLIENT_HEIGHTjjtype = dong_porf_porf_todo_METRIC_CLIENT_HEIGHTjjtype;
  out->dong_porf_porf_todo_METRIC_SCROLL_WIDTH = dong_porf_porf_todo_METRIC_SCROLL_WIDTH;
  out->dong_porf_porf_todo_METRIC_SCROLL_WIDTHjjtype = dong_porf_porf_todo_METRIC_SCROLL_WIDTHjjtype;
  out->dong_porf_porf_todo_METRIC_SCROLL_HEIGHT = dong_porf_porf_todo_METRIC_SCROLL_HEIGHT;
  out->dong_porf_porf_todo_METRIC_SCROLL_HEIGHTjjtype = dong_porf_porf_todo_METRIC_SCROLL_HEIGHTjjtype;
  out->dong_porf_porf_todo_MAX_TODOS = dong_porf_porf_todo_MAX_TODOS;
  out->dong_porf_porf_todo_MAX_TODOSjjtype = dong_porf_porf_todo_MAX_TODOSjjtype;
  out->dong_porf_porf_todo_todoCount = dong_porf_porf_todo_todoCount;
  out->dong_porf_porf_todo_todoCountjjtype = dong_porf_porf_todo_todoCountjjtype;
  out->dong_porf_porf_todo_nextId = dong_porf_porf_todo_nextId;
  out->dong_porf_porf_todo_nextIdjjtype = dong_porf_porf_todo_nextIdjjtype;
  out->dong_porf_porf_todo_filterMode = dong_porf_porf_todo_filterMode;
  out->dong_porf_porf_todo_filterModejjtype = dong_porf_porf_todo_filterModejjtype;
  out->dong_porf_porf_todo_inputText = dong_porf_porf_todo_inputText;
  out->dong_porf_porf_todo_inputTextjjtype = dong_porf_porf_todo_inputTextjjtype;
  out->dong_porf_porf_todo_todoId0 = dong_porf_porf_todo_todoId0;
  out->dong_porf_porf_todo_todoId0jjtype = dong_porf_porf_todo_todoId0jjtype;
  out->dong_porf_porf_todo_todoText0 = dong_porf_porf_todo_todoText0;
  out->dong_porf_porf_todo_todoText0jjtype = dong_porf_porf_todo_todoText0jjtype;
  out->dong_porf_porf_todo_todoDone0 = dong_porf_porf_todo_todoDone0;
  out->dong_porf_porf_todo_todoDone0jjtype = dong_porf_porf_todo_todoDone0jjtype;
  out->dong_porf_porf_todo_todoId1 = dong_porf_porf_todo_todoId1;
  out->dong_porf_porf_todo_todoId1jjtype = dong_porf_porf_todo_todoId1jjtype;
  out->dong_porf_porf_todo_todoText1 = dong_porf_porf_todo_todoText1;
  out->dong_porf_porf_todo_todoText1jjtype = dong_porf_porf_todo_todoText1jjtype;
  out->dong_porf_porf_todo_todoDone1 = dong_porf_porf_todo_todoDone1;
  out->dong_porf_porf_todo_todoDone1jjtype = dong_porf_porf_todo_todoDone1jjtype;
  out->dong_porf_porf_todo_todoId2 = dong_porf_porf_todo_todoId2;
  out->dong_porf_porf_todo_todoId2jjtype = dong_porf_porf_todo_todoId2jjtype;
  out->dong_porf_porf_todo_todoText2 = dong_porf_porf_todo_todoText2;
  out->dong_porf_porf_todo_todoText2jjtype = dong_porf_porf_todo_todoText2jjtype;
  out->dong_porf_porf_todo_todoDone2 = dong_porf_porf_todo_todoDone2;
  out->dong_porf_porf_todo_todoDone2jjtype = dong_porf_porf_todo_todoDone2jjtype;
  out->dong_porf_porf_todo_todoId3 = dong_porf_porf_todo_todoId3;
  out->dong_porf_porf_todo_todoId3jjtype = dong_porf_porf_todo_todoId3jjtype;
  out->dong_porf_porf_todo_todoText3 = dong_porf_porf_todo_todoText3;
  out->dong_porf_porf_todo_todoText3jjtype = dong_porf_porf_todo_todoText3jjtype;
  out->dong_porf_porf_todo_todoDone3 = dong_porf_porf_todo_todoDone3;
  out->dong_porf_porf_todo_todoDone3jjtype = dong_porf_porf_todo_todoDone3jjtype;
  out->dong_porf_porf_todo_todoInputId = dong_porf_porf_todo_todoInputId;
  out->dong_porf_porf_todo_todoInputIdjjtype = dong_porf_porf_todo_todoInputIdjjtype;
  out->dong_porf_porf_todo_btnAddId = dong_porf_porf_todo_btnAddId;
  out->dong_porf_porf_todo_btnAddIdjjtype = dong_porf_porf_todo_btnAddIdjjtype;
  out->dong_porf_porf_todo_filterAllId = dong_porf_porf_todo_filterAllId;
  out->dong_porf_porf_todo_filterAllIdjjtype = dong_porf_porf_todo_filterAllIdjjtype;
  out->dong_porf_porf_todo_filterActiveId = dong_porf_porf_todo_filterActiveId;
  out->dong_porf_porf_todo_filterActiveIdjjtype = dong_porf_porf_todo_filterActiveIdjjtype;
  out->dong_porf_porf_todo_filterDoneId = dong_porf_porf_todo_filterDoneId;
  out->dong_porf_porf_todo_filterDoneIdjjtype = dong_porf_porf_todo_filterDoneIdjjtype;
  out->dong_porf_porf_todo_todoListId = dong_porf_porf_todo_todoListId;
  out->dong_porf_porf_todo_todoListIdjjtype = dong_porf_porf_todo_todoListIdjjtype;
  out->dong_porf_porf_todo_clearWrapId = dong_porf_porf_todo_clearWrapId;
  out->dong_porf_porf_todo_clearWrapIdjjtype = dong_porf_porf_todo_clearWrapIdjjtype;
  out->dong_porf_porf_todo_btnClearId = dong_porf_porf_todo_btnClearId;
  out->dong_porf_porf_todo_btnClearIdjjtype = dong_porf_porf_todo_btnClearIdjjtype;
  out->dong_porf_porf_todo_todoId4 = dong_porf_porf_todo_todoId4;
  out->dong_porf_porf_todo_todoId4jjtype = dong_porf_porf_todo_todoId4jjtype;
  out->dong_porf_porf_todo_todoText4 = dong_porf_porf_todo_todoText4;
  out->dong_porf_porf_todo_todoText4jjtype = dong_porf_porf_todo_todoText4jjtype;
  out->dong_porf_porf_todo_todoDone4 = dong_porf_porf_todo_todoDone4;
  out->dong_porf_porf_todo_todoDone4jjtype = dong_porf_porf_todo_todoDone4jjtype;
  out->dong_porf_porf_todo_todoId5 = dong_porf_porf_todo_todoId5;
  out->dong_porf_porf_todo_todoId5jjtype = dong_porf_porf_todo_todoId5jjtype;
  out->dong_porf_porf_todo_todoText5 = dong_porf_porf_todo_todoText5;
  out->dong_porf_porf_todo_todoText5jjtype = dong_porf_porf_todo_todoText5jjtype;
  out->dong_porf_porf_todo_todoDone5 = dong_porf_porf_todo_todoDone5;
  out->dong_porf_porf_todo_todoDone5jjtype = dong_porf_porf_todo_todoDone5jjtype;
  out->dong_porf_porf_todo_todoId6 = dong_porf_porf_todo_todoId6;
  out->dong_porf_porf_todo_todoId6jjtype = dong_porf_porf_todo_todoId6jjtype;
  out->dong_porf_porf_todo_todoText6 = dong_porf_porf_todo_todoText6;
  out->dong_porf_porf_todo_todoText6jjtype = dong_porf_porf_todo_todoText6jjtype;
  out->dong_porf_porf_todo_todoDone6 = dong_porf_porf_todo_todoDone6;
  out->dong_porf_porf_todo_todoDone6jjtype = dong_porf_porf_todo_todoDone6jjtype;
  out->dong_porf_porf_todo_todoId7 = dong_porf_porf_todo_todoId7;
  out->dong_porf_porf_todo_todoId7jjtype = dong_porf_porf_todo_todoId7jjtype;
  out->dong_porf_porf_todo_todoText7 = dong_porf_porf_todo_todoText7;
  out->dong_porf_porf_todo_todoText7jjtype = dong_porf_porf_todo_todoText7jjtype;
  out->dong_porf_porf_todo_todoDone7 = dong_porf_porf_todo_todoDone7;
  out->dong_porf_porf_todo_todoDone7jjtype = dong_porf_porf_todo_todoDone7jjtype;
  out->dong_porf_porf_todo_todoId8 = dong_porf_porf_todo_todoId8;
  out->dong_porf_porf_todo_todoId8jjtype = dong_porf_porf_todo_todoId8jjtype;
  out->dong_porf_porf_todo_todoText8 = dong_porf_porf_todo_todoText8;
  out->dong_porf_porf_todo_todoText8jjtype = dong_porf_porf_todo_todoText8jjtype;
  out->dong_porf_porf_todo_todoDone8 = dong_porf_porf_todo_todoDone8;
  out->dong_porf_porf_todo_todoDone8jjtype = dong_porf_porf_todo_todoDone8jjtype;
  out->dong_porf_porf_todo_todoId9 = dong_porf_porf_todo_todoId9;
  out->dong_porf_porf_todo_todoId9jjtype = dong_porf_porf_todo_todoId9jjtype;
  out->dong_porf_porf_todo_todoText9 = dong_porf_porf_todo_todoText9;
  out->dong_porf_porf_todo_todoText9jjtype = dong_porf_porf_todo_todoText9jjtype;
  out->dong_porf_porf_todo_todoDone9 = dong_porf_porf_todo_todoDone9;
  out->dong_porf_porf_todo_todoDone9jjtype = dong_porf_porf_todo_todoDone9jjtype;
  out->dong_porf_porf_todo_todoId10 = dong_porf_porf_todo_todoId10;
  out->dong_porf_porf_todo_todoId10jjtype = dong_porf_porf_todo_todoId10jjtype;
  out->dong_porf_porf_todo_todoText10 = dong_porf_porf_todo_todoText10;
  out->dong_porf_porf_todo_todoText10jjtype = dong_porf_porf_todo_todoText10jjtype;
  out->dong_porf_porf_todo_todoDone10 = dong_porf_porf_todo_todoDone10;
  out->dong_porf_porf_todo_todoDone10jjtype = dong_porf_porf_todo_todoDone10jjtype;
  out->dong_porf_porf_todo_todoId11 = dong_porf_porf_todo_todoId11;
  out->dong_porf_porf_todo_todoId11jjtype = dong_porf_porf_todo_todoId11jjtype;
  out->dong_porf_porf_todo_todoText11 = dong_porf_porf_todo_todoText11;
  out->dong_porf_porf_todo_todoText11jjtype = dong_porf_porf_todo_todoText11jjtype;
  out->dong_porf_porf_todo_todoDone11 = dong_porf_porf_todo_todoDone11;
  out->dong_porf_porf_todo_todoDone11jjtype = dong_porf_porf_todo_todoDone11jjtype;
  out->dong_porf_porf_todo_todoId12 = dong_porf_porf_todo_todoId12;
  out->dong_porf_porf_todo_todoId12jjtype = dong_porf_porf_todo_todoId12jjtype;
  out->dong_porf_porf_todo_todoText12 = dong_porf_porf_todo_todoText12;
  out->dong_porf_porf_todo_todoText12jjtype = dong_porf_porf_todo_todoText12jjtype;
  out->dong_porf_porf_todo_todoDone12 = dong_porf_porf_todo_todoDone12;
  out->dong_porf_porf_todo_todoDone12jjtype = dong_porf_porf_todo_todoDone12jjtype;
  out->dong_porf_porf_todo_todoId13 = dong_porf_porf_todo_todoId13;
  out->dong_porf_porf_todo_todoId13jjtype = dong_porf_porf_todo_todoId13jjtype;
  out->dong_porf_porf_todo_todoText13 = dong_porf_porf_todo_todoText13;
  out->dong_porf_porf_todo_todoText13jjtype = dong_porf_porf_todo_todoText13jjtype;
  out->dong_porf_porf_todo_todoDone13 = dong_porf_porf_todo_todoDone13;
  out->dong_porf_porf_todo_todoDone13jjtype = dong_porf_porf_todo_todoDone13jjtype;
  out->dong_porf_porf_todo_todoId14 = dong_porf_porf_todo_todoId14;
  out->dong_porf_porf_todo_todoId14jjtype = dong_porf_porf_todo_todoId14jjtype;
  out->dong_porf_porf_todo_todoText14 = dong_porf_porf_todo_todoText14;
  out->dong_porf_porf_todo_todoText14jjtype = dong_porf_porf_todo_todoText14jjtype;
  out->dong_porf_porf_todo_todoDone14 = dong_porf_porf_todo_todoDone14;
  out->dong_porf_porf_todo_todoDone14jjtype = dong_porf_porf_todo_todoDone14jjtype;
  out->dong_porf_porf_todo_todoId15 = dong_porf_porf_todo_todoId15;
  out->dong_porf_porf_todo_todoId15jjtype = dong_porf_porf_todo_todoId15jjtype;
  out->dong_porf_porf_todo_todoText15 = dong_porf_porf_todo_todoText15;
  out->dong_porf_porf_todo_todoText15jjtype = dong_porf_porf_todo_todoText15jjtype;
  out->dong_porf_porf_todo_todoDone15 = dong_porf_porf_todo_todoDone15;
  out->dong_porf_porf_todo_todoDone15jjtype = dong_porf_porf_todo_todoDone15jjtype;
  out->dong_porf_porf_todo_jjporfjjcurrentPtr = dong_porf_porf_todo_jjporfjjcurrentPtr;
  out->dong_porf_porf_todo_jjporfjjcurrentPtrjjglbl_inited = dong_porf_porf_todo_jjporfjjcurrentPtrjjglbl_inited;
  out->dong_porf_porf_todo_jjporfjjendPtr = dong_porf_porf_todo_jjporfjjendPtr;
  out->dong_porf_porf_todo_jjporfjjendPtrjjglbl_inited = dong_porf_porf_todo_jjporfjjendPtrjjglbl_inited;
  out->dong_porf_porf_todo_jjporfjjunderlyingStore = dong_porf_porf_todo_jjporfjjunderlyingStore;
  out->dong_porf_porf_todo_jjporfjjunderlyingStorejjglbl_inited = dong_porf_porf_todo_jjporfjjunderlyingStorejjglbl_inited;
  out->dong_porf_porf_todo_jjporfjjgetptr___Object_prototype = dong_porf_porf_todo_jjporfjjgetptr___Object_prototype;
  out->dong_porf_porf_todo_jjporfjjgetptr___Object_prototypejjglbl_inited = dong_porf_porf_todo_jjporfjjgetptr___Object_prototypejjglbl_inited;
}

void dong_porf_porf_todo_state_apply(const dong_porf_porf_todo_state_t* in) {
  dong_porf_porf_todo_METRIC_OFFSET_WIDTH = in->dong_porf_porf_todo_METRIC_OFFSET_WIDTH;
  dong_porf_porf_todo_METRIC_OFFSET_WIDTHjjtype = in->dong_porf_porf_todo_METRIC_OFFSET_WIDTHjjtype;
  dong_porf_porf_todo_METRIC_OFFSET_HEIGHT = in->dong_porf_porf_todo_METRIC_OFFSET_HEIGHT;
  dong_porf_porf_todo_METRIC_OFFSET_HEIGHTjjtype = in->dong_porf_porf_todo_METRIC_OFFSET_HEIGHTjjtype;
  dong_porf_porf_todo_METRIC_OFFSET_TOP = in->dong_porf_porf_todo_METRIC_OFFSET_TOP;
  dong_porf_porf_todo_METRIC_OFFSET_TOPjjtype = in->dong_porf_porf_todo_METRIC_OFFSET_TOPjjtype;
  dong_porf_porf_todo_METRIC_OFFSET_LEFT = in->dong_porf_porf_todo_METRIC_OFFSET_LEFT;
  dong_porf_porf_todo_METRIC_OFFSET_LEFTjjtype = in->dong_porf_porf_todo_METRIC_OFFSET_LEFTjjtype;
  dong_porf_porf_todo_METRIC_CLIENT_WIDTH = in->dong_porf_porf_todo_METRIC_CLIENT_WIDTH;
  dong_porf_porf_todo_METRIC_CLIENT_WIDTHjjtype = in->dong_porf_porf_todo_METRIC_CLIENT_WIDTHjjtype;
  dong_porf_porf_todo_METRIC_CLIENT_HEIGHT = in->dong_porf_porf_todo_METRIC_CLIENT_HEIGHT;
  dong_porf_porf_todo_METRIC_CLIENT_HEIGHTjjtype = in->dong_porf_porf_todo_METRIC_CLIENT_HEIGHTjjtype;
  dong_porf_porf_todo_METRIC_SCROLL_WIDTH = in->dong_porf_porf_todo_METRIC_SCROLL_WIDTH;
  dong_porf_porf_todo_METRIC_SCROLL_WIDTHjjtype = in->dong_porf_porf_todo_METRIC_SCROLL_WIDTHjjtype;
  dong_porf_porf_todo_METRIC_SCROLL_HEIGHT = in->dong_porf_porf_todo_METRIC_SCROLL_HEIGHT;
  dong_porf_porf_todo_METRIC_SCROLL_HEIGHTjjtype = in->dong_porf_porf_todo_METRIC_SCROLL_HEIGHTjjtype;
  dong_porf_porf_todo_MAX_TODOS = in->dong_porf_porf_todo_MAX_TODOS;
  dong_porf_porf_todo_MAX_TODOSjjtype = in->dong_porf_porf_todo_MAX_TODOSjjtype;
  dong_porf_porf_todo_todoCount = in->dong_porf_porf_todo_todoCount;
  dong_porf_porf_todo_todoCountjjtype = in->dong_porf_porf_todo_todoCountjjtype;
  dong_porf_porf_todo_nextId = in->dong_porf_porf_todo_nextId;
  dong_porf_porf_todo_nextIdjjtype = in->dong_porf_porf_todo_nextIdjjtype;
  dong_porf_porf_todo_filterMode = in->dong_porf_porf_todo_filterMode;
  dong_porf_porf_todo_filterModejjtype = in->dong_porf_porf_todo_filterModejjtype;
  dong_porf_porf_todo_inputText = in->dong_porf_porf_todo_inputText;
  dong_porf_porf_todo_inputTextjjtype = in->dong_porf_porf_todo_inputTextjjtype;
  dong_porf_porf_todo_todoId0 = in->dong_porf_porf_todo_todoId0;
  dong_porf_porf_todo_todoId0jjtype = in->dong_porf_porf_todo_todoId0jjtype;
  dong_porf_porf_todo_todoText0 = in->dong_porf_porf_todo_todoText0;
  dong_porf_porf_todo_todoText0jjtype = in->dong_porf_porf_todo_todoText0jjtype;
  dong_porf_porf_todo_todoDone0 = in->dong_porf_porf_todo_todoDone0;
  dong_porf_porf_todo_todoDone0jjtype = in->dong_porf_porf_todo_todoDone0jjtype;
  dong_porf_porf_todo_todoId1 = in->dong_porf_porf_todo_todoId1;
  dong_porf_porf_todo_todoId1jjtype = in->dong_porf_porf_todo_todoId1jjtype;
  dong_porf_porf_todo_todoText1 = in->dong_porf_porf_todo_todoText1;
  dong_porf_porf_todo_todoText1jjtype = in->dong_porf_porf_todo_todoText1jjtype;
  dong_porf_porf_todo_todoDone1 = in->dong_porf_porf_todo_todoDone1;
  dong_porf_porf_todo_todoDone1jjtype = in->dong_porf_porf_todo_todoDone1jjtype;
  dong_porf_porf_todo_todoId2 = in->dong_porf_porf_todo_todoId2;
  dong_porf_porf_todo_todoId2jjtype = in->dong_porf_porf_todo_todoId2jjtype;
  dong_porf_porf_todo_todoText2 = in->dong_porf_porf_todo_todoText2;
  dong_porf_porf_todo_todoText2jjtype = in->dong_porf_porf_todo_todoText2jjtype;
  dong_porf_porf_todo_todoDone2 = in->dong_porf_porf_todo_todoDone2;
  dong_porf_porf_todo_todoDone2jjtype = in->dong_porf_porf_todo_todoDone2jjtype;
  dong_porf_porf_todo_todoId3 = in->dong_porf_porf_todo_todoId3;
  dong_porf_porf_todo_todoId3jjtype = in->dong_porf_porf_todo_todoId3jjtype;
  dong_porf_porf_todo_todoText3 = in->dong_porf_porf_todo_todoText3;
  dong_porf_porf_todo_todoText3jjtype = in->dong_porf_porf_todo_todoText3jjtype;
  dong_porf_porf_todo_todoDone3 = in->dong_porf_porf_todo_todoDone3;
  dong_porf_porf_todo_todoDone3jjtype = in->dong_porf_porf_todo_todoDone3jjtype;
  dong_porf_porf_todo_todoInputId = in->dong_porf_porf_todo_todoInputId;
  dong_porf_porf_todo_todoInputIdjjtype = in->dong_porf_porf_todo_todoInputIdjjtype;
  dong_porf_porf_todo_btnAddId = in->dong_porf_porf_todo_btnAddId;
  dong_porf_porf_todo_btnAddIdjjtype = in->dong_porf_porf_todo_btnAddIdjjtype;
  dong_porf_porf_todo_filterAllId = in->dong_porf_porf_todo_filterAllId;
  dong_porf_porf_todo_filterAllIdjjtype = in->dong_porf_porf_todo_filterAllIdjjtype;
  dong_porf_porf_todo_filterActiveId = in->dong_porf_porf_todo_filterActiveId;
  dong_porf_porf_todo_filterActiveIdjjtype = in->dong_porf_porf_todo_filterActiveIdjjtype;
  dong_porf_porf_todo_filterDoneId = in->dong_porf_porf_todo_filterDoneId;
  dong_porf_porf_todo_filterDoneIdjjtype = in->dong_porf_porf_todo_filterDoneIdjjtype;
  dong_porf_porf_todo_todoListId = in->dong_porf_porf_todo_todoListId;
  dong_porf_porf_todo_todoListIdjjtype = in->dong_porf_porf_todo_todoListIdjjtype;
  dong_porf_porf_todo_clearWrapId = in->dong_porf_porf_todo_clearWrapId;
  dong_porf_porf_todo_clearWrapIdjjtype = in->dong_porf_porf_todo_clearWrapIdjjtype;
  dong_porf_porf_todo_btnClearId = in->dong_porf_porf_todo_btnClearId;
  dong_porf_porf_todo_btnClearIdjjtype = in->dong_porf_porf_todo_btnClearIdjjtype;
  dong_porf_porf_todo_todoId4 = in->dong_porf_porf_todo_todoId4;
  dong_porf_porf_todo_todoId4jjtype = in->dong_porf_porf_todo_todoId4jjtype;
  dong_porf_porf_todo_todoText4 = in->dong_porf_porf_todo_todoText4;
  dong_porf_porf_todo_todoText4jjtype = in->dong_porf_porf_todo_todoText4jjtype;
  dong_porf_porf_todo_todoDone4 = in->dong_porf_porf_todo_todoDone4;
  dong_porf_porf_todo_todoDone4jjtype = in->dong_porf_porf_todo_todoDone4jjtype;
  dong_porf_porf_todo_todoId5 = in->dong_porf_porf_todo_todoId5;
  dong_porf_porf_todo_todoId5jjtype = in->dong_porf_porf_todo_todoId5jjtype;
  dong_porf_porf_todo_todoText5 = in->dong_porf_porf_todo_todoText5;
  dong_porf_porf_todo_todoText5jjtype = in->dong_porf_porf_todo_todoText5jjtype;
  dong_porf_porf_todo_todoDone5 = in->dong_porf_porf_todo_todoDone5;
  dong_porf_porf_todo_todoDone5jjtype = in->dong_porf_porf_todo_todoDone5jjtype;
  dong_porf_porf_todo_todoId6 = in->dong_porf_porf_todo_todoId6;
  dong_porf_porf_todo_todoId6jjtype = in->dong_porf_porf_todo_todoId6jjtype;
  dong_porf_porf_todo_todoText6 = in->dong_porf_porf_todo_todoText6;
  dong_porf_porf_todo_todoText6jjtype = in->dong_porf_porf_todo_todoText6jjtype;
  dong_porf_porf_todo_todoDone6 = in->dong_porf_porf_todo_todoDone6;
  dong_porf_porf_todo_todoDone6jjtype = in->dong_porf_porf_todo_todoDone6jjtype;
  dong_porf_porf_todo_todoId7 = in->dong_porf_porf_todo_todoId7;
  dong_porf_porf_todo_todoId7jjtype = in->dong_porf_porf_todo_todoId7jjtype;
  dong_porf_porf_todo_todoText7 = in->dong_porf_porf_todo_todoText7;
  dong_porf_porf_todo_todoText7jjtype = in->dong_porf_porf_todo_todoText7jjtype;
  dong_porf_porf_todo_todoDone7 = in->dong_porf_porf_todo_todoDone7;
  dong_porf_porf_todo_todoDone7jjtype = in->dong_porf_porf_todo_todoDone7jjtype;
  dong_porf_porf_todo_todoId8 = in->dong_porf_porf_todo_todoId8;
  dong_porf_porf_todo_todoId8jjtype = in->dong_porf_porf_todo_todoId8jjtype;
  dong_porf_porf_todo_todoText8 = in->dong_porf_porf_todo_todoText8;
  dong_porf_porf_todo_todoText8jjtype = in->dong_porf_porf_todo_todoText8jjtype;
  dong_porf_porf_todo_todoDone8 = in->dong_porf_porf_todo_todoDone8;
  dong_porf_porf_todo_todoDone8jjtype = in->dong_porf_porf_todo_todoDone8jjtype;
  dong_porf_porf_todo_todoId9 = in->dong_porf_porf_todo_todoId9;
  dong_porf_porf_todo_todoId9jjtype = in->dong_porf_porf_todo_todoId9jjtype;
  dong_porf_porf_todo_todoText9 = in->dong_porf_porf_todo_todoText9;
  dong_porf_porf_todo_todoText9jjtype = in->dong_porf_porf_todo_todoText9jjtype;
  dong_porf_porf_todo_todoDone9 = in->dong_porf_porf_todo_todoDone9;
  dong_porf_porf_todo_todoDone9jjtype = in->dong_porf_porf_todo_todoDone9jjtype;
  dong_porf_porf_todo_todoId10 = in->dong_porf_porf_todo_todoId10;
  dong_porf_porf_todo_todoId10jjtype = in->dong_porf_porf_todo_todoId10jjtype;
  dong_porf_porf_todo_todoText10 = in->dong_porf_porf_todo_todoText10;
  dong_porf_porf_todo_todoText10jjtype = in->dong_porf_porf_todo_todoText10jjtype;
  dong_porf_porf_todo_todoDone10 = in->dong_porf_porf_todo_todoDone10;
  dong_porf_porf_todo_todoDone10jjtype = in->dong_porf_porf_todo_todoDone10jjtype;
  dong_porf_porf_todo_todoId11 = in->dong_porf_porf_todo_todoId11;
  dong_porf_porf_todo_todoId11jjtype = in->dong_porf_porf_todo_todoId11jjtype;
  dong_porf_porf_todo_todoText11 = in->dong_porf_porf_todo_todoText11;
  dong_porf_porf_todo_todoText11jjtype = in->dong_porf_porf_todo_todoText11jjtype;
  dong_porf_porf_todo_todoDone11 = in->dong_porf_porf_todo_todoDone11;
  dong_porf_porf_todo_todoDone11jjtype = in->dong_porf_porf_todo_todoDone11jjtype;
  dong_porf_porf_todo_todoId12 = in->dong_porf_porf_todo_todoId12;
  dong_porf_porf_todo_todoId12jjtype = in->dong_porf_porf_todo_todoId12jjtype;
  dong_porf_porf_todo_todoText12 = in->dong_porf_porf_todo_todoText12;
  dong_porf_porf_todo_todoText12jjtype = in->dong_porf_porf_todo_todoText12jjtype;
  dong_porf_porf_todo_todoDone12 = in->dong_porf_porf_todo_todoDone12;
  dong_porf_porf_todo_todoDone12jjtype = in->dong_porf_porf_todo_todoDone12jjtype;
  dong_porf_porf_todo_todoId13 = in->dong_porf_porf_todo_todoId13;
  dong_porf_porf_todo_todoId13jjtype = in->dong_porf_porf_todo_todoId13jjtype;
  dong_porf_porf_todo_todoText13 = in->dong_porf_porf_todo_todoText13;
  dong_porf_porf_todo_todoText13jjtype = in->dong_porf_porf_todo_todoText13jjtype;
  dong_porf_porf_todo_todoDone13 = in->dong_porf_porf_todo_todoDone13;
  dong_porf_porf_todo_todoDone13jjtype = in->dong_porf_porf_todo_todoDone13jjtype;
  dong_porf_porf_todo_todoId14 = in->dong_porf_porf_todo_todoId14;
  dong_porf_porf_todo_todoId14jjtype = in->dong_porf_porf_todo_todoId14jjtype;
  dong_porf_porf_todo_todoText14 = in->dong_porf_porf_todo_todoText14;
  dong_porf_porf_todo_todoText14jjtype = in->dong_porf_porf_todo_todoText14jjtype;
  dong_porf_porf_todo_todoDone14 = in->dong_porf_porf_todo_todoDone14;
  dong_porf_porf_todo_todoDone14jjtype = in->dong_porf_porf_todo_todoDone14jjtype;
  dong_porf_porf_todo_todoId15 = in->dong_porf_porf_todo_todoId15;
  dong_porf_porf_todo_todoId15jjtype = in->dong_porf_porf_todo_todoId15jjtype;
  dong_porf_porf_todo_todoText15 = in->dong_porf_porf_todo_todoText15;
  dong_porf_porf_todo_todoText15jjtype = in->dong_porf_porf_todo_todoText15jjtype;
  dong_porf_porf_todo_todoDone15 = in->dong_porf_porf_todo_todoDone15;
  dong_porf_porf_todo_todoDone15jjtype = in->dong_porf_porf_todo_todoDone15jjtype;
  dong_porf_porf_todo_jjporfjjcurrentPtr = in->dong_porf_porf_todo_jjporfjjcurrentPtr;
  dong_porf_porf_todo_jjporfjjcurrentPtrjjglbl_inited = in->dong_porf_porf_todo_jjporfjjcurrentPtrjjglbl_inited;
  dong_porf_porf_todo_jjporfjjendPtr = in->dong_porf_porf_todo_jjporfjjendPtr;
  dong_porf_porf_todo_jjporfjjendPtrjjglbl_inited = in->dong_porf_porf_todo_jjporfjjendPtrjjglbl_inited;
  dong_porf_porf_todo_jjporfjjunderlyingStore = in->dong_porf_porf_todo_jjporfjjunderlyingStore;
  dong_porf_porf_todo_jjporfjjunderlyingStorejjglbl_inited = in->dong_porf_porf_todo_jjporfjjunderlyingStorejjglbl_inited;
  dong_porf_porf_todo_jjporfjjgetptr___Object_prototype = in->dong_porf_porf_todo_jjporfjjgetptr___Object_prototype;
  dong_porf_porf_todo_jjporfjjgetptr___Object_prototypejjglbl_inited = in->dong_porf_porf_todo_jjporfjjgetptr___Object_prototypejjglbl_inited;
}
