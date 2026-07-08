// Porffor module: porf_todo
#include "dong_porf_runtime.h"
#ifndef __porf_nan
#define __porf_nan (NAN)
#endif
#ifndef __porf_infinity
#define __porf_infinity (INFINITY)
#endif

char* dong_porf_porf_todo_memory; u32 dong_porf_porf_todo_memory_pages = 3;

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

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_remove_attribute(f64, f64);

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_set_attribute(f64, f64, f64);

__attribute__((import_module(""), import_name("i")))
extern void __porf_import_dong_print(f64);

__attribute__((import_module(""), import_name("é")))
extern void __porf_import_dong_num_to_str(f64);

__attribute__((import_module(""), import_name("m")))
extern f64 __porf_import_dong_str_pull();

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_set_inner_html(f64, f64);
extern void __porf_import_dong_set_inner_html_typed(f64, f64, f64);

__attribute__((import_module(""), import_name("g")))
extern void __porf_import_dong_commit_set_textContent();

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_style_set(f64, f64, f64);

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_get_value(f64);

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_set_value(f64, f64);

__attribute__((import_module(""), import_name("²")))
extern void __porf_import_dong_event_key();

__attribute__((import_module(""), import_name("±")))
extern f64 __porf_import_dong_event_target();

__attribute__((import_module(""), import_name("")))
extern void __porf_import_dong_get_attribute(f64, f64);

__attribute__((import_module(""), import_name("")))
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
  dong_porf_porf_todo_memory[878]=(u8)43;dong_porf_porf_todo_memory[882]=(u8)67;dong_porf_porf_todo_memory[883]=(u8)97;dong_porf_porf_todo_memory[884]=(u8)110;dong_porf_porf_todo_memory[885]=(u8)110;dong_porf_porf_todo_memory[886]=(u8)111;dong_porf_porf_todo_memory[887]=(u8)116;dong_porf_porf_todo_memory[888]=(u8)32;dong_porf_porf_todo_memory[889]=(u8)99;dong_porf_porf_todo_memory[890]=(u8)111;dong_porf_porf_todo_memory[891]=(u8)110;dong_porf_porf_todo_memory[892]=(u8)118;dong_porf_porf_todo_memory[893]=(u8)101;dong_porf_porf_todo_memory[894]=(u8)114;dong_porf_porf_todo_memory[895]=(u8)116;dong_porf_porf_todo_memory[896]=(u8)32;dong_porf_porf_todo_memory[897]=(u8)83;dong_porf_porf_todo_memory[898]=(u8)121;dong_porf_porf_todo_memory[899]=(u8)109;dong_porf_porf_todo_memory[900]=(u8)98;dong_porf_porf_todo_memory[901]=(u8)111;dong_porf_porf_todo_memory[902]=(u8)108;dong_porf_porf_todo_memory[903]=(u8)32;dong_porf_porf_todo_memory[904]=(u8)111;dong_porf_porf_todo_memory[905]=(u8)114;dong_porf_porf_todo_memory[906]=(u8)32;dong_porf_porf_todo_memory[907]=(u8)66;dong_porf_porf_todo_memory[908]=(u8)105;dong_porf_porf_todo_memory[909]=(u8)103;dong_porf_porf_todo_memory[910]=(u8)73;dong_porf_porf_todo_memory[911]=(u8)110;dong_porf_porf_todo_memory[912]=(u8)116;dong_porf_porf_todo_memory[913]=(u8)32;dong_porf_porf_todo_memory[914]=(u8)116;dong_porf_porf_todo_memory[915]=(u8)111;dong_porf_porf_todo_memory[916]=(u8)32;dong_porf_porf_todo_memory[917]=(u8)97;dong_porf_porf_todo_memory[918]=(u8)32;dong_porf_porf_todo_memory[919]=(u8)110;dong_porf_porf_todo_memory[920]=(u8)117;dong_porf_porf_todo_memory[921]=(u8)109;dong_porf_porf_todo_memory[922]=(u8)98;dong_porf_porf_todo_memory[923]=(u8)101;dong_porf_porf_todo_memory[924]=(u8)114;
  dong_porf_porf_todo_memory[927]=(u8)62;dong_porf_porf_todo_memory[931]=(u8)39;dong_porf_porf_todo_memory[932]=(u8)116;dong_porf_porf_todo_memory[933]=(u8)114;dong_porf_porf_todo_memory[934]=(u8)105;dong_porf_porf_todo_memory[935]=(u8)109;dong_porf_porf_todo_memory[936]=(u8)39;dong_porf_porf_todo_memory[937]=(u8)32;dong_porf_porf_todo_memory[938]=(u8)112;dong_porf_porf_todo_memory[939]=(u8)114;dong_porf_porf_todo_memory[940]=(u8)111;dong_porf_porf_todo_memory[941]=(u8)116;dong_porf_porf_todo_memory[942]=(u8)111;dong_porf_porf_todo_memory[943]=(u8)32;dong_porf_porf_todo_memory[944]=(u8)102;dong_porf_porf_todo_memory[945]=(u8)117;dong_porf_porf_todo_memory[946]=(u8)110;dong_porf_porf_todo_memory[947]=(u8)99;dong_porf_porf_todo_memory[948]=(u8)32;dong_porf_porf_todo_memory[949]=(u8)116;dong_porf_porf_todo_memory[950]=(u8)114;dong_porf_porf_todo_memory[951]=(u8)105;dong_porf_porf_todo_memory[952]=(u8)101;dong_porf_porf_todo_memory[953]=(u8)100;dong_porf_porf_todo_memory[954]=(u8)32;dong_porf_porf_todo_memory[955]=(u8)116;dong_porf_porf_todo_memory[956]=(u8)111;dong_porf_porf_todo_memory[957]=(u8)32;dong_porf_porf_todo_memory[958]=(u8)98;dong_porf_porf_todo_memory[959]=(u8)101;dong_porf_porf_todo_memory[960]=(u8)32;dong_porf_porf_todo_memory[961]=(u8)99;dong_porf_porf_todo_memory[962]=(u8)97;dong_porf_porf_todo_memory[963]=(u8)108;dong_porf_porf_todo_memory[964]=(u8)108;dong_porf_porf_todo_memory[965]=(u8)101;dong_porf_porf_todo_memory[966]=(u8)100;dong_porf_porf_todo_memory[967]=(u8)32;dong_porf_porf_todo_memory[968]=(u8)111;dong_porf_porf_todo_memory[969]=(u8)110;dong_porf_porf_todo_memory[970]=(u8)32;dong_porf_porf_todo_memory[971]=(u8)97;dong_porf_porf_todo_memory[972]=(u8)32;dong_porf_porf_todo_memory[973]=(u8)116;dong_porf_porf_todo_memory[974]=(u8)121;dong_porf_porf_todo_memory[975]=(u8)112;dong_porf_porf_todo_memory[976]=(u8)101;dong_porf_porf_todo_memory[977]=(u8)32;dong_porf_porf_todo_memory[978]=(u8)119;dong_porf_porf_todo_memory[979]=(u8)105;dong_porf_porf_todo_memory[980]=(u8)116;dong_porf_porf_todo_memory[981]=(u8)104;dong_porf_porf_todo_memory[982]=(u8)111;dong_porf_porf_todo_memory[983]=(u8)117;dong_porf_porf_todo_memory[984]=(u8)116;dong_porf_porf_todo_memory[985]=(u8)32;dong_porf_porf_todo_memory[986]=(u8)97;dong_porf_porf_todo_memory[987]=(u8)110;dong_porf_porf_todo_memory[988]=(u8)32;dong_porf_porf_todo_memory[989]=(u8)105;dong_porf_porf_todo_memory[990]=(u8)109;dong_porf_porf_todo_memory[991]=(u8)112;dong_porf_porf_todo_memory[992]=(u8)108;
  dong_porf_porf_todo_memory[995]=(u8)68;dong_porf_porf_todo_memory[999]=(u8)39;dong_porf_porf_todo_memory[1000]=(u8)99;dong_porf_porf_todo_memory[1001]=(u8)104;dong_porf_porf_todo_memory[1002]=(u8)97;dong_porf_porf_todo_memory[1003]=(u8)114;dong_porf_porf_todo_memory[1004]=(u8)67;dong_porf_porf_todo_memory[1005]=(u8)111;dong_porf_porf_todo_memory[1006]=(u8)100;dong_porf_porf_todo_memory[1007]=(u8)101;dong_porf_porf_todo_memory[1008]=(u8)65;dong_porf_porf_todo_memory[1009]=(u8)116;dong_porf_porf_todo_memory[1010]=(u8)39;dong_porf_porf_todo_memory[1011]=(u8)32;dong_porf_porf_todo_memory[1012]=(u8)112;dong_porf_porf_todo_memory[1013]=(u8)114;dong_porf_porf_todo_memory[1014]=(u8)111;dong_porf_porf_todo_memory[1015]=(u8)116;dong_porf_porf_todo_memory[1016]=(u8)111;dong_porf_porf_todo_memory[1017]=(u8)32;dong_porf_porf_todo_memory[1018]=(u8)102;dong_porf_porf_todo_memory[1019]=(u8)117;dong_porf_porf_todo_memory[1020]=(u8)110;dong_porf_porf_todo_memory[1021]=(u8)99;dong_porf_porf_todo_memory[1022]=(u8)32;dong_porf_porf_todo_memory[1023]=(u8)116;dong_porf_porf_todo_memory[1024]=(u8)114;dong_porf_porf_todo_memory[1025]=(u8)105;dong_porf_porf_todo_memory[1026]=(u8)101;dong_porf_porf_todo_memory[1027]=(u8)100;dong_porf_porf_todo_memory[1028]=(u8)32;dong_porf_porf_todo_memory[1029]=(u8)116;dong_porf_porf_todo_memory[1030]=(u8)111;dong_porf_porf_todo_memory[1031]=(u8)32;dong_porf_porf_todo_memory[1032]=(u8)98;dong_porf_porf_todo_memory[1033]=(u8)101;dong_porf_porf_todo_memory[1034]=(u8)32;dong_porf_porf_todo_memory[1035]=(u8)99;dong_porf_porf_todo_memory[1036]=(u8)97;dong_porf_porf_todo_memory[1037]=(u8)108;dong_porf_porf_todo_memory[1038]=(u8)108;dong_porf_porf_todo_memory[1039]=(u8)101;dong_porf_porf_todo_memory[1040]=(u8)100;dong_porf_porf_todo_memory[1041]=(u8)32;dong_porf_porf_todo_memory[1042]=(u8)111;dong_porf_porf_todo_memory[1043]=(u8)110;dong_porf_porf_todo_memory[1044]=(u8)32;dong_porf_porf_todo_memory[1045]=(u8)97;dong_porf_porf_todo_memory[1046]=(u8)32;dong_porf_porf_todo_memory[1047]=(u8)116;dong_porf_porf_todo_memory[1048]=(u8)121;dong_porf_porf_todo_memory[1049]=(u8)112;dong_porf_porf_todo_memory[1050]=(u8)101;dong_porf_porf_todo_memory[1051]=(u8)32;dong_porf_porf_todo_memory[1052]=(u8)119;dong_porf_porf_todo_memory[1053]=(u8)105;dong_porf_porf_todo_memory[1054]=(u8)116;dong_porf_porf_todo_memory[1055]=(u8)104;dong_porf_porf_todo_memory[1056]=(u8)111;dong_porf_porf_todo_memory[1057]=(u8)117;dong_porf_porf_todo_memory[1058]=(u8)116;dong_porf_porf_todo_memory[1059]=(u8)32;dong_porf_porf_todo_memory[1060]=(u8)97;dong_porf_porf_todo_memory[1061]=(u8)110;dong_porf_porf_todo_memory[1062]=(u8)32;dong_porf_porf_todo_memory[1063]=(u8)105;dong_porf_porf_todo_memory[1064]=(u8)109;dong_porf_porf_todo_memory[1065]=(u8)112;dong_porf_porf_todo_memory[1066]=(u8)108;
  dong_porf_porf_todo_memory[1069]=(u8)54;dong_porf_porf_todo_memory[1073]=(u8)78;dong_porf_porf_todo_memory[1074]=(u8)117;dong_porf_porf_todo_memory[1075]=(u8)109;dong_porf_porf_todo_memory[1076]=(u8)98;dong_porf_porf_todo_memory[1077]=(u8)101;dong_porf_porf_todo_memory[1078]=(u8)114;dong_porf_porf_todo_memory[1079]=(u8)46;dong_porf_porf_todo_memory[1080]=(u8)112;dong_porf_porf_todo_memory[1081]=(u8)114;dong_porf_porf_todo_memory[1082]=(u8)111;dong_porf_porf_todo_memory[1083]=(u8)116;dong_porf_porf_todo_memory[1084]=(u8)111;dong_porf_porf_todo_memory[1085]=(u8)116;dong_porf_porf_todo_memory[1086]=(u8)121;dong_porf_porf_todo_memory[1087]=(u8)112;dong_porf_porf_todo_memory[1088]=(u8)101;dong_porf_porf_todo_memory[1089]=(u8)46;dong_porf_porf_todo_memory[1090]=(u8)118;dong_porf_porf_todo_memory[1091]=(u8)97;dong_porf_porf_todo_memory[1092]=(u8)108;dong_porf_porf_todo_memory[1093]=(u8)117;dong_porf_porf_todo_memory[1094]=(u8)101;dong_porf_porf_todo_memory[1095]=(u8)79;dong_porf_porf_todo_memory[1096]=(u8)102;dong_porf_porf_todo_memory[1097]=(u8)32;dong_porf_porf_todo_memory[1098]=(u8)101;dong_porf_porf_todo_memory[1099]=(u8)120;dong_porf_porf_todo_memory[1100]=(u8)112;dong_porf_porf_todo_memory[1101]=(u8)101;dong_porf_porf_todo_memory[1102]=(u8)99;dong_porf_porf_todo_memory[1103]=(u8)116;dong_porf_porf_todo_memory[1104]=(u8)115;dong_porf_porf_todo_memory[1105]=(u8)32;dong_porf_porf_todo_memory[1106]=(u8)39;dong_porf_porf_todo_memory[1107]=(u8)116;dong_porf_porf_todo_memory[1108]=(u8)104;dong_porf_porf_todo_memory[1109]=(u8)105;dong_porf_porf_todo_memory[1110]=(u8)115;dong_porf_porf_todo_memory[1111]=(u8)39;dong_porf_porf_todo_memory[1112]=(u8)32;dong_porf_porf_todo_memory[1113]=(u8)116;dong_porf_porf_todo_memory[1114]=(u8)111;dong_porf_porf_todo_memory[1115]=(u8)32;dong_porf_porf_todo_memory[1116]=(u8)98;dong_porf_porf_todo_memory[1117]=(u8)101;dong_porf_porf_todo_memory[1118]=(u8)32;dong_porf_porf_todo_memory[1119]=(u8)97;dong_porf_porf_todo_memory[1120]=(u8)32;dong_porf_porf_todo_memory[1121]=(u8)78;dong_porf_porf_todo_memory[1122]=(u8)117;dong_porf_porf_todo_memory[1123]=(u8)109;dong_porf_porf_todo_memory[1124]=(u8)98;dong_porf_porf_todo_memory[1125]=(u8)101;dong_porf_porf_todo_memory[1126]=(u8)114;
  dong_porf_porf_todo_memory[1129]=(u8)7;dong_porf_porf_todo_memory[1133]=(u8)118;dong_porf_porf_todo_memory[1134]=(u8)97;dong_porf_porf_todo_memory[1135]=(u8)108;dong_porf_porf_todo_memory[1136]=(u8)117;dong_porf_porf_todo_memory[1137]=(u8)101;dong_porf_porf_todo_memory[1138]=(u8)79;dong_porf_porf_todo_memory[1139]=(u8)102;
  dong_porf_porf_todo_memory[1142]=(u8)29;dong_porf_porf_todo_memory[1146]=(u8)70;dong_porf_porf_todo_memory[1147]=(u8)117;dong_porf_porf_todo_memory[1148]=(u8)110;dong_porf_porf_todo_memory[1149]=(u8)99;dong_porf_porf_todo_memory[1150]=(u8)116;dong_porf_porf_todo_memory[1151]=(u8)105;dong_porf_porf_todo_memory[1152]=(u8)111;dong_porf_porf_todo_memory[1153]=(u8)110;dong_porf_porf_todo_memory[1154]=(u8)32;dong_porf_porf_todo_memory[1155]=(u8)105;dong_porf_porf_todo_memory[1156]=(u8)115;dong_porf_porf_todo_memory[1157]=(u8)32;dong_porf_porf_todo_memory[1158]=(u8)110;dong_porf_porf_todo_memory[1159]=(u8)111;dong_porf_porf_todo_memory[1160]=(u8)116;dong_porf_porf_todo_memory[1161]=(u8)32;dong_porf_porf_todo_memory[1162]=(u8)97;dong_porf_porf_todo_memory[1163]=(u8)32;dong_porf_porf_todo_memory[1164]=(u8)99;dong_porf_porf_todo_memory[1165]=(u8)111;dong_porf_porf_todo_memory[1166]=(u8)110;dong_porf_porf_todo_memory[1167]=(u8)115;dong_porf_porf_todo_memory[1168]=(u8)116;dong_porf_porf_todo_memory[1169]=(u8)114;dong_porf_porf_todo_memory[1170]=(u8)117;dong_porf_porf_todo_memory[1171]=(u8)99;dong_porf_porf_todo_memory[1172]=(u8)116;dong_porf_porf_todo_memory[1173]=(u8)111;dong_porf_porf_todo_memory[1174]=(u8)114;
  dong_porf_porf_todo_memory[1177]=(u8)21;dong_porf_porf_todo_memory[1181]=(u8)111;dong_porf_porf_todo_memory[1182]=(u8)118;dong_porf_porf_todo_memory[1183]=(u8)114;dong_porf_porf_todo_memory[1184]=(u8)32;dong_porf_porf_todo_memory[1185]=(u8)105;dong_porf_porf_todo_memory[1186]=(u8)115;dong_porf_porf_todo_memory[1187]=(u8)32;dong_porf_porf_todo_memory[1188]=(u8)110;dong_porf_porf_todo_memory[1189]=(u8)111;dong_porf_porf_todo_memory[1190]=(u8)116;dong_porf_porf_todo_memory[1191]=(u8)32;dong_porf_porf_todo_memory[1192]=(u8)97;dong_porf_porf_todo_memory[1193]=(u8)32;dong_porf_porf_todo_memory[1194]=(u8)102;dong_porf_porf_todo_memory[1195]=(u8)117;dong_porf_porf_todo_memory[1196]=(u8)110;dong_porf_porf_todo_memory[1197]=(u8)99;dong_porf_porf_todo_memory[1198]=(u8)116;dong_porf_porf_todo_memory[1199]=(u8)105;dong_porf_porf_todo_memory[1200]=(u8)111;dong_porf_porf_todo_memory[1201]=(u8)110;
  dong_porf_porf_todo_memory[1204]=(u8)37;dong_porf_porf_todo_memory[1208]=(u8)67;dong_porf_porf_todo_memory[1209]=(u8)97;dong_porf_porf_todo_memory[1210]=(u8)110;dong_porf_porf_todo_memory[1211]=(u8)110;dong_porf_porf_todo_memory[1212]=(u8)111;dong_porf_porf_todo_memory[1213]=(u8)116;dong_porf_porf_todo_memory[1214]=(u8)32;dong_porf_porf_todo_memory[1215]=(u8)99;dong_porf_porf_todo_memory[1216]=(u8)111;dong_porf_porf_todo_memory[1217]=(u8)110;dong_porf_porf_todo_memory[1218]=(u8)118;dong_porf_porf_todo_memory[1219]=(u8)101;dong_porf_porf_todo_memory[1220]=(u8)114;dong_porf_porf_todo_memory[1221]=(u8)116;dong_porf_porf_todo_memory[1222]=(u8)32;dong_porf_porf_todo_memory[1223]=(u8)97;dong_porf_porf_todo_memory[1224]=(u8)110;dong_porf_porf_todo_memory[1225]=(u8)32;dong_porf_porf_todo_memory[1226]=(u8)111;dong_porf_porf_todo_memory[1227]=(u8)98;dong_porf_porf_todo_memory[1228]=(u8)106;dong_porf_porf_todo_memory[1229]=(u8)101;dong_porf_porf_todo_memory[1230]=(u8)99;dong_porf_porf_todo_memory[1231]=(u8)116;dong_porf_porf_todo_memory[1232]=(u8)32;dong_porf_porf_todo_memory[1233]=(u8)116;dong_porf_porf_todo_memory[1234]=(u8)111;dong_porf_porf_todo_memory[1235]=(u8)32;dong_porf_porf_todo_memory[1236]=(u8)112;dong_porf_porf_todo_memory[1237]=(u8)114;dong_porf_porf_todo_memory[1238]=(u8)105;dong_porf_porf_todo_memory[1239]=(u8)109;dong_porf_porf_todo_memory[1240]=(u8)105;dong_porf_porf_todo_memory[1241]=(u8)116;dong_porf_porf_todo_memory[1242]=(u8)105;dong_porf_porf_todo_memory[1243]=(u8)118;dong_porf_porf_todo_memory[1244]=(u8)101;
  dong_porf_porf_todo_memory[1247]=(u8)20;dong_porf_porf_todo_memory[1251]=(u8)73;dong_porf_porf_todo_memory[1252]=(u8)110;dong_porf_porf_todo_memory[1253]=(u8)118;dong_porf_porf_todo_memory[1254]=(u8)97;dong_porf_porf_todo_memory[1255]=(u8)108;dong_porf_porf_todo_memory[1256]=(u8)105;dong_porf_porf_todo_memory[1257]=(u8)100;dong_porf_porf_todo_memory[1258]=(u8)32;dong_porf_porf_todo_memory[1259]=(u8)97;dong_porf_porf_todo_memory[1260]=(u8)114;dong_porf_porf_todo_memory[1261]=(u8)114;dong_porf_porf_todo_memory[1262]=(u8)97;dong_porf_porf_todo_memory[1263]=(u8)121;dong_porf_porf_todo_memory[1264]=(u8)32;dong_porf_porf_todo_memory[1265]=(u8)108;dong_porf_porf_todo_memory[1266]=(u8)101;dong_porf_porf_todo_memory[1267]=(u8)110;dong_porf_porf_todo_memory[1268]=(u8)103;dong_porf_porf_todo_memory[1269]=(u8)116;dong_porf_porf_todo_memory[1270]=(u8)104;
  dong_porf_porf_todo_memory[1273]=(u8)20;dong_porf_porf_todo_memory[1277]=(u8)112;dong_porf_porf_todo_memory[1278]=(u8)114;dong_porf_porf_todo_memory[1279]=(u8)111;dong_porf_porf_todo_memory[1280]=(u8)112;dong_porf_porf_todo_memory[1281]=(u8)101;dong_porf_porf_todo_memory[1282]=(u8)114;dong_porf_porf_todo_memory[1283]=(u8)116;dong_porf_porf_todo_memory[1284]=(u8)121;dong_porf_porf_todo_memory[1285]=(u8)73;dong_porf_porf_todo_memory[1286]=(u8)115;dong_porf_porf_todo_memory[1287]=(u8)69;dong_porf_porf_todo_memory[1288]=(u8)110;dong_porf_porf_todo_memory[1289]=(u8)117;dong_porf_porf_todo_memory[1290]=(u8)109;dong_porf_porf_todo_memory[1291]=(u8)101;dong_porf_porf_todo_memory[1292]=(u8)114;dong_porf_porf_todo_memory[1293]=(u8)97;dong_porf_porf_todo_memory[1294]=(u8)98;dong_porf_porf_todo_memory[1295]=(u8)108;dong_porf_porf_todo_memory[1296]=(u8)101;
  dong_porf_porf_todo_memory[1299]=(u8)13;dong_porf_porf_todo_memory[1303]=(u8)105;dong_porf_porf_todo_memory[1304]=(u8)115;dong_porf_porf_todo_memory[1305]=(u8)80;dong_porf_porf_todo_memory[1306]=(u8)114;dong_porf_porf_todo_memory[1307]=(u8)111;dong_porf_porf_todo_memory[1308]=(u8)116;dong_porf_porf_todo_memory[1309]=(u8)111;dong_porf_porf_todo_memory[1310]=(u8)116;dong_porf_porf_todo_memory[1311]=(u8)121;dong_porf_porf_todo_memory[1312]=(u8)112;dong_porf_porf_todo_memory[1313]=(u8)101;dong_porf_porf_todo_memory[1314]=(u8)79;dong_porf_porf_todo_memory[1315]=(u8)102;
  dong_porf_porf_todo_memory[1318]=(u8)32;dong_porf_porf_todo_memory[1322]=(u8)84;dong_porf_porf_todo_memory[1323]=(u8)104;dong_porf_porf_todo_memory[1324]=(u8)105;dong_porf_porf_todo_memory[1325]=(u8)115;dong_porf_porf_todo_memory[1326]=(u8)32;dong_porf_porf_todo_memory[1327]=(u8)105;dong_porf_porf_todo_memory[1328]=(u8)115;dong_porf_porf_todo_memory[1329]=(u8)32;dong_porf_porf_todo_memory[1330]=(u8)110;dong_porf_porf_todo_memory[1331]=(u8)117;dong_porf_porf_todo_memory[1332]=(u8)108;dong_porf_porf_todo_memory[1333]=(u8)108;dong_porf_porf_todo_memory[1334]=(u8)105;dong_porf_porf_todo_memory[1335]=(u8)115;dong_porf_porf_todo_memory[1336]=(u8)104;dong_porf_porf_todo_memory[1337]=(u8)44;dong_porf_porf_todo_memory[1338]=(u8)32;dong_porf_porf_todo_memory[1339]=(u8)101;dong_porf_porf_todo_memory[1340]=(u8)120;dong_porf_porf_todo_memory[1341]=(u8)112;dong_porf_porf_todo_memory[1342]=(u8)101;dong_porf_porf_todo_memory[1343]=(u8)99;dong_porf_porf_todo_memory[1344]=(u8)116;dong_porf_porf_todo_memory[1345]=(u8)101;dong_porf_porf_todo_memory[1346]=(u8)100;dong_porf_porf_todo_memory[1347]=(u8)32;dong_porf_porf_todo_memory[1348]=(u8)111;dong_porf_porf_todo_memory[1349]=(u8)98;dong_porf_porf_todo_memory[1350]=(u8)106;dong_porf_porf_todo_memory[1351]=(u8)101;dong_porf_porf_todo_memory[1352]=(u8)99;dong_porf_porf_todo_memory[1353]=(u8)116;
  dong_porf_porf_todo_memory[1356]=(u8)14;dong_porf_porf_todo_memory[1360]=(u8)116;dong_porf_porf_todo_memory[1361]=(u8)111;dong_porf_porf_todo_memory[1362]=(u8)76;dong_porf_porf_todo_memory[1363]=(u8)111;dong_porf_porf_todo_memory[1364]=(u8)99;dong_porf_porf_todo_memory[1365]=(u8)97;dong_porf_porf_todo_memory[1366]=(u8)108;dong_porf_porf_todo_memory[1367]=(u8)101;dong_porf_porf_todo_memory[1368]=(u8)83;dong_porf_porf_todo_memory[1369]=(u8)116;dong_porf_porf_todo_memory[1370]=(u8)114;dong_porf_porf_todo_memory[1371]=(u8)105;dong_porf_porf_todo_memory[1372]=(u8)110;dong_porf_porf_todo_memory[1373]=(u8)103;
  dong_porf_porf_todo_memory[1376]=(u8)55;dong_porf_porf_todo_memory[1380]=(u8)83;dong_porf_porf_todo_memory[1381]=(u8)121;dong_porf_porf_todo_memory[1382]=(u8)109;dong_porf_porf_todo_memory[1383]=(u8)98;dong_porf_porf_todo_memory[1384]=(u8)111;dong_porf_porf_todo_memory[1385]=(u8)108;dong_porf_porf_todo_memory[1386]=(u8)46;dong_porf_porf_todo_memory[1387]=(u8)112;dong_porf_porf_todo_memory[1388]=(u8)114;dong_porf_porf_todo_memory[1389]=(u8)111;dong_porf_porf_todo_memory[1390]=(u8)116;dong_porf_porf_todo_memory[1391]=(u8)111;dong_porf_porf_todo_memory[1392]=(u8)116;dong_porf_porf_todo_memory[1393]=(u8)121;dong_porf_porf_todo_memory[1394]=(u8)112;dong_porf_porf_todo_memory[1395]=(u8)101;dong_porf_porf_todo_memory[1396]=(u8)46;dong_porf_porf_todo_memory[1397]=(u8)116;dong_porf_porf_todo_memory[1398]=(u8)111;dong_porf_porf_todo_memory[1399]=(u8)83;dong_porf_porf_todo_memory[1400]=(u8)116;dong_porf_porf_todo_memory[1401]=(u8)114;dong_porf_porf_todo_memory[1402]=(u8)105;dong_porf_porf_todo_memory[1403]=(u8)110;dong_porf_porf_todo_memory[1404]=(u8)103;dong_porf_porf_todo_memory[1405]=(u8)32;dong_porf_porf_todo_memory[1406]=(u8)101;dong_porf_porf_todo_memory[1407]=(u8)120;dong_porf_porf_todo_memory[1408]=(u8)112;dong_porf_porf_todo_memory[1409]=(u8)101;dong_porf_porf_todo_memory[1410]=(u8)99;dong_porf_porf_todo_memory[1411]=(u8)116;dong_porf_porf_todo_memory[1412]=(u8)115;dong_porf_porf_todo_memory[1413]=(u8)32;dong_porf_porf_todo_memory[1414]=(u8)39;dong_porf_porf_todo_memory[1415]=(u8)116;dong_porf_porf_todo_memory[1416]=(u8)104;dong_porf_porf_todo_memory[1417]=(u8)105;dong_porf_porf_todo_memory[1418]=(u8)115;dong_porf_porf_todo_memory[1419]=(u8)39;dong_porf_porf_todo_memory[1420]=(u8)32;dong_porf_porf_todo_memory[1421]=(u8)116;dong_porf_porf_todo_memory[1422]=(u8)111;dong_porf_porf_todo_memory[1423]=(u8)32;dong_porf_porf_todo_memory[1424]=(u8)98;dong_porf_porf_todo_memory[1425]=(u8)101;dong_porf_porf_todo_memory[1426]=(u8)32;dong_porf_porf_todo_memory[1427]=(u8)97;dong_porf_porf_todo_memory[1428]=(u8)32;dong_porf_porf_todo_memory[1429]=(u8)83;dong_porf_porf_todo_memory[1430]=(u8)121;dong_porf_porf_todo_memory[1431]=(u8)109;dong_porf_porf_todo_memory[1432]=(u8)98;dong_porf_porf_todo_memory[1433]=(u8)111;dong_porf_porf_todo_memory[1434]=(u8)108;
  dong_porf_porf_todo_memory[1437]=(u8)62;dong_porf_porf_todo_memory[1441]=(u8)83;dong_porf_porf_todo_memory[1442]=(u8)121;dong_porf_porf_todo_memory[1443]=(u8)109;dong_porf_porf_todo_memory[1444]=(u8)98;dong_porf_porf_todo_memory[1445]=(u8)111;dong_porf_porf_todo_memory[1446]=(u8)108;dong_porf_porf_todo_memory[1447]=(u8)46;dong_porf_porf_todo_memory[1448]=(u8)112;dong_porf_porf_todo_memory[1449]=(u8)114;dong_porf_porf_todo_memory[1450]=(u8)111;dong_porf_porf_todo_memory[1451]=(u8)116;dong_porf_porf_todo_memory[1452]=(u8)111;dong_porf_porf_todo_memory[1453]=(u8)116;dong_porf_porf_todo_memory[1454]=(u8)121;dong_porf_porf_todo_memory[1455]=(u8)112;dong_porf_porf_todo_memory[1456]=(u8)101;dong_porf_porf_todo_memory[1457]=(u8)46;dong_porf_porf_todo_memory[1458]=(u8)100;dong_porf_porf_todo_memory[1459]=(u8)101;dong_porf_porf_todo_memory[1460]=(u8)115;dong_porf_porf_todo_memory[1461]=(u8)99;dong_porf_porf_todo_memory[1462]=(u8)114;dong_porf_porf_todo_memory[1463]=(u8)105;dong_porf_porf_todo_memory[1464]=(u8)112;dong_porf_porf_todo_memory[1465]=(u8)116;dong_porf_porf_todo_memory[1466]=(u8)105;dong_porf_porf_todo_memory[1467]=(u8)111;dong_porf_porf_todo_memory[1468]=(u8)110;dong_porf_porf_todo_memory[1469]=(u8)36;dong_porf_porf_todo_memory[1470]=(u8)103;dong_porf_porf_todo_memory[1471]=(u8)101;dong_porf_porf_todo_memory[1472]=(u8)116;dong_porf_porf_todo_memory[1473]=(u8)32;dong_porf_porf_todo_memory[1474]=(u8)101;dong_porf_porf_todo_memory[1475]=(u8)120;dong_porf_porf_todo_memory[1476]=(u8)112;dong_porf_porf_todo_memory[1477]=(u8)101;dong_porf_porf_todo_memory[1478]=(u8)99;dong_porf_porf_todo_memory[1479]=(u8)116;dong_porf_porf_todo_memory[1480]=(u8)115;dong_porf_porf_todo_memory[1481]=(u8)32;dong_porf_porf_todo_memory[1482]=(u8)39;dong_porf_porf_todo_memory[1483]=(u8)116;dong_porf_porf_todo_memory[1484]=(u8)104;dong_porf_porf_todo_memory[1485]=(u8)105;dong_porf_porf_todo_memory[1486]=(u8)115;dong_porf_porf_todo_memory[1487]=(u8)39;dong_porf_porf_todo_memory[1488]=(u8)32;dong_porf_porf_todo_memory[1489]=(u8)116;dong_porf_porf_todo_memory[1490]=(u8)111;dong_porf_porf_todo_memory[1491]=(u8)32;dong_porf_porf_todo_memory[1492]=(u8)98;dong_porf_porf_todo_memory[1493]=(u8)101;dong_porf_porf_todo_memory[1494]=(u8)32;dong_porf_porf_todo_memory[1495]=(u8)97;dong_porf_porf_todo_memory[1496]=(u8)32;dong_porf_porf_todo_memory[1497]=(u8)83;dong_porf_porf_todo_memory[1498]=(u8)121;dong_porf_porf_todo_memory[1499]=(u8)109;dong_porf_porf_todo_memory[1500]=(u8)98;dong_porf_porf_todo_memory[1501]=(u8)111;dong_porf_porf_todo_memory[1502]=(u8)108;
  dong_porf_porf_todo_memory[1505]=(u8)18;dong_porf_porf_todo_memory[1509]=(u8)91;dong_porf_porf_todo_memory[1510]=(u8)111;dong_porf_porf_todo_memory[1511]=(u8)98;dong_porf_porf_todo_memory[1512]=(u8)106;dong_porf_porf_todo_memory[1513]=(u8)101;dong_porf_porf_todo_memory[1514]=(u8)99;dong_porf_porf_todo_memory[1515]=(u8)116;dong_porf_porf_todo_memory[1516]=(u8)32;dong_porf_porf_todo_memory[1517]=(u8)85;dong_porf_porf_todo_memory[1518]=(u8)110;dong_porf_porf_todo_memory[1519]=(u8)100;dong_porf_porf_todo_memory[1520]=(u8)101;dong_porf_porf_todo_memory[1521]=(u8)102;dong_porf_porf_todo_memory[1522]=(u8)105;dong_porf_porf_todo_memory[1523]=(u8)110;dong_porf_porf_todo_memory[1524]=(u8)101;dong_porf_porf_todo_memory[1525]=(u8)100;dong_porf_porf_todo_memory[1526]=(u8)93;
  dong_porf_porf_todo_memory[1529]=(u8)13;dong_porf_porf_todo_memory[1533]=(u8)91;dong_porf_porf_todo_memory[1534]=(u8)111;dong_porf_porf_todo_memory[1535]=(u8)98;dong_porf_porf_todo_memory[1536]=(u8)106;dong_porf_porf_todo_memory[1537]=(u8)101;dong_porf_porf_todo_memory[1538]=(u8)99;dong_porf_porf_todo_memory[1539]=(u8)116;dong_porf_porf_todo_memory[1540]=(u8)32;dong_porf_porf_todo_memory[1541]=(u8)78;dong_porf_porf_todo_memory[1542]=(u8)117;dong_porf_porf_todo_memory[1543]=(u8)108;dong_porf_porf_todo_memory[1544]=(u8)108;dong_porf_porf_todo_memory[1545]=(u8)93;
  dong_porf_porf_todo_memory[1548]=(u8)14;dong_porf_porf_todo_memory[1552]=(u8)91;dong_porf_porf_todo_memory[1553]=(u8)111;dong_porf_porf_todo_memory[1554]=(u8)98;dong_porf_porf_todo_memory[1555]=(u8)106;dong_porf_porf_todo_memory[1556]=(u8)101;dong_porf_porf_todo_memory[1557]=(u8)99;dong_porf_porf_todo_memory[1558]=(u8)116;dong_porf_porf_todo_memory[1559]=(u8)32;dong_porf_porf_todo_memory[1560]=(u8)65;dong_porf_porf_todo_memory[1561]=(u8)114;dong_porf_porf_todo_memory[1562]=(u8)114;dong_porf_porf_todo_memory[1563]=(u8)97;dong_porf_porf_todo_memory[1564]=(u8)121;dong_porf_porf_todo_memory[1565]=(u8)93;
  dong_porf_porf_todo_memory[1568]=(u8)17;dong_porf_porf_todo_memory[1572]=(u8)91;dong_porf_porf_todo_memory[1573]=(u8)111;dong_porf_porf_todo_memory[1574]=(u8)98;dong_porf_porf_todo_memory[1575]=(u8)106;dong_porf_porf_todo_memory[1576]=(u8)101;dong_porf_porf_todo_memory[1577]=(u8)99;dong_porf_porf_todo_memory[1578]=(u8)116;dong_porf_porf_todo_memory[1579]=(u8)32;dong_porf_porf_todo_memory[1580]=(u8)70;dong_porf_porf_todo_memory[1581]=(u8)117;dong_porf_porf_todo_memory[1582]=(u8)110;dong_porf_porf_todo_memory[1583]=(u8)99;dong_porf_porf_todo_memory[1584]=(u8)116;dong_porf_porf_todo_memory[1585]=(u8)105;dong_porf_porf_todo_memory[1586]=(u8)111;dong_porf_porf_todo_memory[1587]=(u8)110;dong_porf_porf_todo_memory[1588]=(u8)93;
  dong_porf_porf_todo_memory[1591]=(u8)16;dong_porf_porf_todo_memory[1595]=(u8)91;dong_porf_porf_todo_memory[1596]=(u8)111;dong_porf_porf_todo_memory[1597]=(u8)98;dong_porf_porf_todo_memory[1598]=(u8)106;dong_porf_porf_todo_memory[1599]=(u8)101;dong_porf_porf_todo_memory[1600]=(u8)99;dong_porf_porf_todo_memory[1601]=(u8)116;dong_porf_porf_todo_memory[1602]=(u8)32;dong_porf_porf_todo_memory[1603]=(u8)66;dong_porf_porf_todo_memory[1604]=(u8)111;dong_porf_porf_todo_memory[1605]=(u8)111;dong_porf_porf_todo_memory[1606]=(u8)108;dong_porf_porf_todo_memory[1607]=(u8)101;dong_porf_porf_todo_memory[1608]=(u8)97;dong_porf_porf_todo_memory[1609]=(u8)110;dong_porf_porf_todo_memory[1610]=(u8)93;
  dong_porf_porf_todo_memory[1613]=(u8)15;dong_porf_porf_todo_memory[1617]=(u8)91;dong_porf_porf_todo_memory[1618]=(u8)111;dong_porf_porf_todo_memory[1619]=(u8)98;dong_porf_porf_todo_memory[1620]=(u8)106;dong_porf_porf_todo_memory[1621]=(u8)101;dong_porf_porf_todo_memory[1622]=(u8)99;dong_porf_porf_todo_memory[1623]=(u8)116;dong_porf_porf_todo_memory[1624]=(u8)32;dong_porf_porf_todo_memory[1625]=(u8)78;dong_porf_porf_todo_memory[1626]=(u8)117;dong_porf_porf_todo_memory[1627]=(u8)109;dong_porf_porf_todo_memory[1628]=(u8)98;dong_porf_porf_todo_memory[1629]=(u8)101;dong_porf_porf_todo_memory[1630]=(u8)114;dong_porf_porf_todo_memory[1631]=(u8)93;
  dong_porf_porf_todo_memory[1634]=(u8)15;dong_porf_porf_todo_memory[1638]=(u8)91;dong_porf_porf_todo_memory[1639]=(u8)111;dong_porf_porf_todo_memory[1640]=(u8)98;dong_porf_porf_todo_memory[1641]=(u8)106;dong_porf_porf_todo_memory[1642]=(u8)101;dong_porf_porf_todo_memory[1643]=(u8)99;dong_porf_porf_todo_memory[1644]=(u8)116;dong_porf_porf_todo_memory[1645]=(u8)32;dong_porf_porf_todo_memory[1646]=(u8)83;dong_porf_porf_todo_memory[1647]=(u8)116;dong_porf_porf_todo_memory[1648]=(u8)114;dong_porf_porf_todo_memory[1649]=(u8)105;dong_porf_porf_todo_memory[1650]=(u8)110;dong_porf_porf_todo_memory[1651]=(u8)103;dong_porf_porf_todo_memory[1652]=(u8)93;
  dong_porf_porf_todo_memory[1655]=(u8)13;dong_porf_porf_todo_memory[1659]=(u8)91;dong_porf_porf_todo_memory[1660]=(u8)111;dong_porf_porf_todo_memory[1661]=(u8)98;dong_porf_porf_todo_memory[1662]=(u8)106;dong_porf_porf_todo_memory[1663]=(u8)101;dong_porf_porf_todo_memory[1664]=(u8)99;dong_porf_porf_todo_memory[1665]=(u8)116;dong_porf_porf_todo_memory[1666]=(u8)32;dong_porf_porf_todo_memory[1667]=(u8)68;dong_porf_porf_todo_memory[1668]=(u8)97;dong_porf_porf_todo_memory[1669]=(u8)116;dong_porf_porf_todo_memory[1670]=(u8)101;dong_porf_porf_todo_memory[1671]=(u8)93;
  dong_porf_porf_todo_memory[1674]=(u8)15;dong_porf_porf_todo_memory[1678]=(u8)91;dong_porf_porf_todo_memory[1679]=(u8)111;dong_porf_porf_todo_memory[1680]=(u8)98;dong_porf_porf_todo_memory[1681]=(u8)106;dong_porf_porf_todo_memory[1682]=(u8)101;dong_porf_porf_todo_memory[1683]=(u8)99;dong_porf_porf_todo_memory[1684]=(u8)116;dong_porf_porf_todo_memory[1685]=(u8)32;dong_porf_porf_todo_memory[1686]=(u8)82;dong_porf_porf_todo_memory[1687]=(u8)101;dong_porf_porf_todo_memory[1688]=(u8)103;dong_porf_porf_todo_memory[1689]=(u8)69;dong_porf_porf_todo_memory[1690]=(u8)120;dong_porf_porf_todo_memory[1691]=(u8)112;dong_porf_porf_todo_memory[1692]=(u8)93;
  dong_porf_porf_todo_memory[1695]=(u8)15;dong_porf_porf_todo_memory[1699]=(u8)91;dong_porf_porf_todo_memory[1700]=(u8)111;dong_porf_porf_todo_memory[1701]=(u8)98;dong_porf_porf_todo_memory[1702]=(u8)106;dong_porf_porf_todo_memory[1703]=(u8)101;dong_porf_porf_todo_memory[1704]=(u8)99;dong_porf_porf_todo_memory[1705]=(u8)116;dong_porf_porf_todo_memory[1706]=(u8)32;dong_porf_porf_todo_memory[1707]=(u8)79;dong_porf_porf_todo_memory[1708]=(u8)98;dong_porf_porf_todo_memory[1709]=(u8)106;dong_porf_porf_todo_memory[1710]=(u8)101;dong_porf_porf_todo_memory[1711]=(u8)99;dong_porf_porf_todo_memory[1712]=(u8)116;dong_porf_porf_todo_memory[1713]=(u8)93;
  dong_porf_porf_todo_memory[1716]=(u8)57;dong_porf_porf_todo_memory[1720]=(u8)83;dong_porf_porf_todo_memory[1721]=(u8)116;dong_porf_porf_todo_memory[1722]=(u8)114;dong_porf_porf_todo_memory[1723]=(u8)105;dong_porf_porf_todo_memory[1724]=(u8)110;dong_porf_porf_todo_memory[1725]=(u8)103;dong_porf_porf_todo_memory[1726]=(u8)46;dong_porf_porf_todo_memory[1727]=(u8)112;dong_porf_porf_todo_memory[1728]=(u8)114;dong_porf_porf_todo_memory[1729]=(u8)111;dong_porf_porf_todo_memory[1730]=(u8)116;dong_porf_porf_todo_memory[1731]=(u8)111;dong_porf_porf_todo_memory[1732]=(u8)116;dong_porf_porf_todo_memory[1733]=(u8)121;dong_porf_porf_todo_memory[1734]=(u8)112;dong_porf_porf_todo_memory[1735]=(u8)101;dong_porf_porf_todo_memory[1736]=(u8)46;dong_porf_porf_todo_memory[1737]=(u8)118;dong_porf_porf_todo_memory[1738]=(u8)97;dong_porf_porf_todo_memory[1739]=(u8)108;dong_porf_porf_todo_memory[1740]=(u8)117;dong_porf_porf_todo_memory[1741]=(u8)101;dong_porf_porf_todo_memory[1742]=(u8)79;dong_porf_porf_todo_memory[1743]=(u8)102;dong_porf_porf_todo_memory[1744]=(u8)32;dong_porf_porf_todo_memory[1745]=(u8)101;dong_porf_porf_todo_memory[1746]=(u8)120;dong_porf_porf_todo_memory[1747]=(u8)112;dong_porf_porf_todo_memory[1748]=(u8)101;dong_porf_porf_todo_memory[1749]=(u8)99;dong_porf_porf_todo_memory[1750]=(u8)116;dong_porf_porf_todo_memory[1751]=(u8)115;dong_porf_porf_todo_memory[1752]=(u8)32;dong_porf_porf_todo_memory[1753]=(u8)39;dong_porf_porf_todo_memory[1754]=(u8)116;dong_porf_porf_todo_memory[1755]=(u8)104;dong_porf_porf_todo_memory[1756]=(u8)105;dong_porf_porf_todo_memory[1757]=(u8)115;dong_porf_porf_todo_memory[1758]=(u8)39;dong_porf_porf_todo_memory[1759]=(u8)32;dong_porf_porf_todo_memory[1760]=(u8)116;dong_porf_porf_todo_memory[1761]=(u8)111;dong_porf_porf_todo_memory[1762]=(u8)32;dong_porf_porf_todo_memory[1763]=(u8)98;dong_porf_porf_todo_memory[1764]=(u8)101;dong_porf_porf_todo_memory[1765]=(u8)32;dong_porf_porf_todo_memory[1766]=(u8)110;dong_porf_porf_todo_memory[1767]=(u8)111;dong_porf_porf_todo_memory[1768]=(u8)110;dong_porf_porf_todo_memory[1769]=(u8)45;dong_porf_porf_todo_memory[1770]=(u8)110;dong_porf_porf_todo_memory[1771]=(u8)117;dong_porf_porf_todo_memory[1772]=(u8)108;dong_porf_porf_todo_memory[1773]=(u8)108;dong_porf_porf_todo_memory[1774]=(u8)105;dong_porf_porf_todo_memory[1775]=(u8)115;dong_porf_porf_todo_memory[1776]=(u8)104;
  dong_porf_porf_todo_memory[1779]=(u8)33;dong_porf_porf_todo_memory[1783]=(u8)67;dong_porf_porf_todo_memory[1784]=(u8)97;dong_porf_porf_todo_memory[1785]=(u8)110;dong_porf_porf_todo_memory[1786]=(u8)110;dong_porf_porf_todo_memory[1787]=(u8)111;dong_porf_porf_todo_memory[1788]=(u8)116;dong_porf_porf_todo_memory[1789]=(u8)32;dong_porf_porf_todo_memory[1790]=(u8)114;dong_porf_porf_todo_memory[1791]=(u8)101;dong_porf_porf_todo_memory[1792]=(u8)97;dong_porf_porf_todo_memory[1793]=(u8)100;dong_porf_porf_todo_memory[1794]=(u8)32;dong_porf_porf_todo_memory[1795]=(u8)112;dong_porf_porf_todo_memory[1796]=(u8)114;dong_porf_porf_todo_memory[1797]=(u8)111;dong_porf_porf_todo_memory[1798]=(u8)112;dong_porf_porf_todo_memory[1799]=(u8)101;dong_porf_porf_todo_memory[1800]=(u8)114;dong_porf_porf_todo_memory[1801]=(u8)116;dong_porf_porf_todo_memory[1802]=(u8)121;dong_porf_porf_todo_memory[1803]=(u8)32;dong_porf_porf_todo_memory[1804]=(u8)111;dong_porf_porf_todo_memory[1805]=(u8)102;dong_porf_porf_todo_memory[1806]=(u8)32;dong_porf_porf_todo_memory[1807]=(u8)117;dong_porf_porf_todo_memory[1808]=(u8)110;dong_porf_porf_todo_memory[1809]=(u8)100;dong_porf_porf_todo_memory[1810]=(u8)101;dong_porf_porf_todo_memory[1811]=(u8)102;dong_porf_porf_todo_memory[1812]=(u8)105;dong_porf_porf_todo_memory[1813]=(u8)110;dong_porf_porf_todo_memory[1814]=(u8)101;dong_porf_porf_todo_memory[1815]=(u8)100;
  dong_porf_porf_todo_memory[1818]=(u8)10;dong_porf_porf_todo_memory[1822]=(u8)99;dong_porf_porf_todo_memory[1823]=(u8)104;dong_porf_porf_todo_memory[1824]=(u8)97;dong_porf_porf_todo_memory[1825]=(u8)114;dong_porf_porf_todo_memory[1826]=(u8)67;dong_porf_porf_todo_memory[1827]=(u8)111;dong_porf_porf_todo_memory[1828]=(u8)100;dong_porf_porf_todo_memory[1829]=(u8)101;dong_porf_porf_todo_memory[1830]=(u8)65;dong_porf_porf_todo_memory[1831]=(u8)116;
  dong_porf_porf_todo_memory[1834]=(u8)27;dong_porf_porf_todo_memory[1838]=(u8)117;dong_porf_porf_todo_memory[1839]=(u8)110;dong_porf_porf_todo_memory[1840]=(u8)100;dong_porf_porf_todo_memory[1841]=(u8)101;dong_porf_porf_todo_memory[1842]=(u8)102;dong_porf_porf_todo_memory[1843]=(u8)105;dong_porf_porf_todo_memory[1844]=(u8)110;dong_porf_porf_todo_memory[1845]=(u8)101;dong_porf_porf_todo_memory[1846]=(u8)100;dong_porf_porf_todo_memory[1847]=(u8)32;dong_porf_porf_todo_memory[1848]=(u8)105;dong_porf_porf_todo_memory[1849]=(u8)115;dong_porf_porf_todo_memory[1850]=(u8)32;dong_porf_porf_todo_memory[1851]=(u8)110;dong_porf_porf_todo_memory[1852]=(u8)111;dong_porf_porf_todo_memory[1853]=(u8)116;dong_porf_porf_todo_memory[1854]=(u8)32;dong_porf_porf_todo_memory[1855]=(u8)97;dong_porf_porf_todo_memory[1856]=(u8)32;dong_porf_porf_todo_memory[1857]=(u8)102;dong_porf_porf_todo_memory[1858]=(u8)117;dong_porf_porf_todo_memory[1859]=(u8)110;dong_porf_porf_todo_memory[1860]=(u8)99;dong_porf_porf_todo_memory[1861]=(u8)116;dong_porf_porf_todo_memory[1862]=(u8)105;dong_porf_porf_todo_memory[1863]=(u8)111;dong_porf_porf_todo_memory[1864]=(u8)110;
  dong_porf_porf_todo_memory[1867]=(u8)60;dong_porf_porf_todo_memory[1871]=(u8)83;dong_porf_porf_todo_memory[1872]=(u8)116;dong_porf_porf_todo_memory[1873]=(u8)114;dong_porf_porf_todo_memory[1874]=(u8)105;dong_porf_porf_todo_memory[1875]=(u8)110;dong_porf_porf_todo_memory[1876]=(u8)103;dong_porf_porf_todo_memory[1877]=(u8)46;dong_porf_porf_todo_memory[1878]=(u8)112;dong_porf_porf_todo_memory[1879]=(u8)114;dong_porf_porf_todo_memory[1880]=(u8)111;dong_porf_porf_todo_memory[1881]=(u8)116;dong_porf_porf_todo_memory[1882]=(u8)111;dong_porf_porf_todo_memory[1883]=(u8)116;dong_porf_porf_todo_memory[1884]=(u8)121;dong_porf_porf_todo_memory[1885]=(u8)112;dong_porf_porf_todo_memory[1886]=(u8)101;dong_porf_porf_todo_memory[1887]=(u8)46;dong_porf_porf_todo_memory[1888]=(u8)99;dong_porf_porf_todo_memory[1889]=(u8)104;dong_porf_porf_todo_memory[1890]=(u8)97;dong_porf_porf_todo_memory[1891]=(u8)114;dong_porf_porf_todo_memory[1892]=(u8)67;dong_porf_porf_todo_memory[1893]=(u8)111;dong_porf_porf_todo_memory[1894]=(u8)100;dong_porf_porf_todo_memory[1895]=(u8)101;dong_porf_porf_todo_memory[1896]=(u8)65;dong_porf_porf_todo_memory[1897]=(u8)116;dong_porf_porf_todo_memory[1898]=(u8)32;dong_porf_porf_todo_memory[1899]=(u8)101;dong_porf_porf_todo_memory[1900]=(u8)120;dong_porf_porf_todo_memory[1901]=(u8)112;dong_porf_porf_todo_memory[1902]=(u8)101;dong_porf_porf_todo_memory[1903]=(u8)99;dong_porf_porf_todo_memory[1904]=(u8)116;dong_porf_porf_todo_memory[1905]=(u8)115;dong_porf_porf_todo_memory[1906]=(u8)32;dong_porf_porf_todo_memory[1907]=(u8)39;dong_porf_porf_todo_memory[1908]=(u8)116;dong_porf_porf_todo_memory[1909]=(u8)104;dong_porf_porf_todo_memory[1910]=(u8)105;dong_porf_porf_todo_memory[1911]=(u8)115;dong_porf_porf_todo_memory[1912]=(u8)39;dong_porf_porf_todo_memory[1913]=(u8)32;dong_porf_porf_todo_memory[1914]=(u8)116;dong_porf_porf_todo_memory[1915]=(u8)111;dong_porf_porf_todo_memory[1916]=(u8)32;dong_porf_porf_todo_memory[1917]=(u8)98;dong_porf_porf_todo_memory[1918]=(u8)101;dong_porf_porf_todo_memory[1919]=(u8)32;dong_porf_porf_todo_memory[1920]=(u8)110;dong_porf_porf_todo_memory[1921]=(u8)111;dong_porf_porf_todo_memory[1922]=(u8)110;dong_porf_porf_todo_memory[1923]=(u8)45;dong_porf_porf_todo_memory[1924]=(u8)110;dong_porf_porf_todo_memory[1925]=(u8)117;dong_porf_porf_todo_memory[1926]=(u8)108;dong_porf_porf_todo_memory[1927]=(u8)108;dong_porf_porf_todo_memory[1928]=(u8)105;dong_porf_porf_todo_memory[1929]=(u8)115;dong_porf_porf_todo_memory[1930]=(u8)104;
  dong_porf_porf_todo_memory[1933]=(u8)16;dong_porf_porf_todo_memory[1937]=(u8)116;dong_porf_porf_todo_memory[1938]=(u8)111;dong_porf_porf_todo_memory[1939]=(u8)100;dong_porf_porf_todo_memory[1940]=(u8)111;dong_porf_porf_todo_memory[1941]=(u8)32;dong_porf_porf_todo_memory[1942]=(u8)109;dong_porf_porf_todo_memory[1943]=(u8)97;dong_porf_porf_todo_memory[1944]=(u8)120;dong_porf_porf_todo_memory[1945]=(u8)32;dong_porf_porf_todo_memory[1946]=(u8)114;dong_porf_porf_todo_memory[1947]=(u8)101;dong_porf_porf_todo_memory[1948]=(u8)97;dong_porf_porf_todo_memory[1949]=(u8)99;dong_porf_porf_todo_memory[1950]=(u8)104;dong_porf_porf_todo_memory[1951]=(u8)101;dong_porf_porf_todo_memory[1952]=(u8)100;
  dong_porf_porf_todo_memory[1955]=(u8)5;dong_porf_porf_todo_memory[1959]=(u8)65;dong_porf_porf_todo_memory[1960]=(u8)108;dong_porf_porf_todo_memory[1961]=(u8)108;dong_porf_porf_todo_memory[1962]=(u8)32;dong_porf_porf_todo_memory[1963]=(u8)40;
  dong_porf_porf_todo_memory[1966]=(u8)1;dong_porf_porf_todo_memory[1970]=(u8)41;
  dong_porf_porf_todo_memory[1973]=(u8)8;dong_porf_porf_todo_memory[1977]=(u8)65;dong_porf_porf_todo_memory[1978]=(u8)99;dong_porf_porf_todo_memory[1979]=(u8)116;dong_porf_porf_todo_memory[1980]=(u8)105;dong_porf_porf_todo_memory[1981]=(u8)118;dong_porf_porf_todo_memory[1982]=(u8)101;dong_porf_porf_todo_memory[1983]=(u8)32;dong_porf_porf_todo_memory[1984]=(u8)40;
  dong_porf_porf_todo_memory[1987]=(u8)6;dong_porf_porf_todo_memory[1991]=(u8)68;dong_porf_porf_todo_memory[1992]=(u8)111;dong_porf_porf_todo_memory[1993]=(u8)110;dong_porf_porf_todo_memory[1994]=(u8)101;dong_porf_porf_todo_memory[1995]=(u8)32;dong_porf_porf_todo_memory[1996]=(u8)40;
  dong_porf_porf_todo_memory[1999]=(u8)7;dong_porf_porf_todo_memory[2003]=(u8)35;dong_porf_porf_todo_memory[2004]=(u8)51;dong_porf_porf_todo_memory[2005]=(u8)52;dong_porf_porf_todo_memory[2006]=(u8)57;dong_porf_porf_todo_memory[2007]=(u8)56;dong_porf_porf_todo_memory[2008]=(u8)100;dong_porf_porf_todo_memory[2009]=(u8)98;
  dong_porf_porf_todo_memory[2012]=(u8)7;dong_porf_porf_todo_memory[2016]=(u8)35;dong_porf_porf_todo_memory[2017]=(u8)101;dong_porf_porf_todo_memory[2018]=(u8)99;dong_porf_porf_todo_memory[2019]=(u8)102;dong_porf_porf_todo_memory[2020]=(u8)48;dong_porf_porf_todo_memory[2021]=(u8)102;dong_porf_porf_todo_memory[2022]=(u8)49;
  dong_porf_porf_todo_memory[2025]=(u8)4;dong_porf_porf_todo_memory[2029]=(u8)35;dong_porf_porf_todo_memory[2030]=(u8)102;dong_porf_porf_todo_memory[2031]=(u8)102;dong_porf_porf_todo_memory[2032]=(u8)102;
  dong_porf_porf_todo_memory[2035]=(u8)7;dong_porf_porf_todo_memory[2039]=(u8)35;dong_porf_porf_todo_memory[2040]=(u8)55;dong_porf_porf_todo_memory[2041]=(u8)102;dong_porf_porf_todo_memory[2042]=(u8)56;dong_porf_porf_todo_memory[2043]=(u8)99;dong_porf_porf_todo_memory[2044]=(u8)56;dong_porf_porf_todo_memory[2045]=(u8)100;
  dong_porf_porf_todo_memory[2048]=(u8)16;dong_porf_porf_todo_memory[2052]=(u8)98;dong_porf_porf_todo_memory[2053]=(u8)97;dong_porf_porf_todo_memory[2054]=(u8)99;dong_porf_porf_todo_memory[2055]=(u8)107;dong_porf_porf_todo_memory[2056]=(u8)103;dong_porf_porf_todo_memory[2057]=(u8)114;dong_porf_porf_todo_memory[2058]=(u8)111;dong_porf_porf_todo_memory[2059]=(u8)117;dong_porf_porf_todo_memory[2060]=(u8)110;dong_porf_porf_todo_memory[2061]=(u8)100;dong_porf_porf_todo_memory[2062]=(u8)45;dong_porf_porf_todo_memory[2063]=(u8)99;dong_porf_porf_todo_memory[2064]=(u8)111;dong_porf_porf_todo_memory[2065]=(u8)108;dong_porf_porf_todo_memory[2066]=(u8)111;dong_porf_porf_todo_memory[2067]=(u8)114;
  dong_porf_porf_todo_memory[2070]=(u8)5;dong_porf_porf_todo_memory[2074]=(u8)99;dong_porf_porf_todo_memory[2075]=(u8)111;dong_porf_porf_todo_memory[2076]=(u8)108;dong_porf_porf_todo_memory[2077]=(u8)111;dong_porf_porf_todo_memory[2078]=(u8)114;
  dong_porf_porf_todo_memory[2081]=(u8)12;dong_porf_porf_todo_memory[2085]=(u8)67;dong_porf_porf_todo_memory[2086]=(u8)108;dong_porf_porf_todo_memory[2087]=(u8)101;dong_porf_porf_todo_memory[2088]=(u8)97;dong_porf_porf_todo_memory[2089]=(u8)114;dong_porf_porf_todo_memory[2090]=(u8)32;dong_porf_porf_todo_memory[2091]=(u8)100;dong_porf_porf_todo_memory[2092]=(u8)111;dong_porf_porf_todo_memory[2093]=(u8)110;dong_porf_porf_todo_memory[2094]=(u8)101;dong_porf_porf_todo_memory[2095]=(u8)32;dong_porf_porf_todo_memory[2096]=(u8)40;
  dong_porf_porf_todo_memory[2099]=(u8)6;dong_porf_porf_todo_memory[2103]=(u8)104;dong_porf_porf_todo_memory[2104]=(u8)105;dong_porf_porf_todo_memory[2105]=(u8)100;dong_porf_porf_todo_memory[2106]=(u8)100;dong_porf_porf_todo_memory[2107]=(u8)101;dong_porf_porf_todo_memory[2108]=(u8)110;
  dong_porf_porf_todo_memory[2111]=(u8)1;dong_porf_porf_todo_memory[2115]=(u8)49;
  dong_porf_porf_todo_memory[2118]=(u8)5;dong_porf_porf_todo_memory[2122]=(u8)60;dong_porf_porf_todo_memory[2123]=(u8)100;dong_porf_porf_todo_memory[2124]=(u8)105;dong_porf_porf_todo_memory[2125]=(u8)118;dong_porf_porf_todo_memory[2126]=(u8)62;
  dong_porf_porf_todo_memory[2129]=(u8)41;dong_porf_porf_todo_memory[2133]=(u8)112;dong_porf_porf_todo_memory[2134]=(u8)111;dong_porf_porf_todo_memory[2135]=(u8)114;dong_porf_porf_todo_memory[2136]=(u8)102;dong_porf_porf_todo_memory[2137]=(u8)82;dong_porf_porf_todo_memory[2138]=(u8)101;dong_porf_porf_todo_memory[2139]=(u8)98;dong_porf_porf_todo_memory[2140]=(u8)117;dong_porf_porf_todo_memory[2141]=(u8)105;dong_porf_porf_todo_memory[2142]=(u8)108;dong_porf_porf_todo_memory[2143]=(u8)100;dong_porf_porf_todo_memory[2144]=(u8)95;dong_porf_porf_todo_memory[2145]=(u8)116;dong_porf_porf_todo_memory[2146]=(u8)111;dong_porf_porf_todo_memory[2147]=(u8)100;dong_porf_porf_todo_memory[2148]=(u8)111;dong_porf_porf_todo_memory[2149]=(u8)115;dong_porf_porf_todo_memory[2150]=(u8)58;dong_porf_porf_todo_memory[2151]=(u8)32;dong_porf_porf_todo_memory[2152]=(u8)77;dong_porf_porf_todo_memory[2153]=(u8)65;dong_porf_porf_todo_memory[2154]=(u8)88;dong_porf_porf_todo_memory[2155]=(u8)95;dong_porf_porf_todo_memory[2156]=(u8)73;dong_porf_porf_todo_memory[2157]=(u8)84;dong_porf_porf_todo_memory[2158]=(u8)69;dong_porf_porf_todo_memory[2159]=(u8)77;dong_porf_porf_todo_memory[2160]=(u8)83;dong_porf_porf_todo_memory[2161]=(u8)32;dong_porf_porf_todo_memory[2162]=(u8)51;dong_porf_porf_todo_memory[2163]=(u8)50;dong_porf_porf_todo_memory[2164]=(u8)32;dong_porf_porf_todo_memory[2165]=(u8)116;dong_porf_porf_todo_memory[2166]=(u8)114;dong_porf_porf_todo_memory[2167]=(u8)117;dong_porf_porf_todo_memory[2168]=(u8)110;dong_porf_porf_todo_memory[2169]=(u8)99;dong_porf_porf_todo_memory[2170]=(u8)97;dong_porf_porf_todo_memory[2171]=(u8)116;dong_porf_porf_todo_memory[2172]=(u8)101;dong_porf_porf_todo_memory[2173]=(u8)100;
  dong_porf_porf_todo_memory[2176]=(u8)5;dong_porf_porf_todo_memory[2180]=(u8)32;dong_porf_porf_todo_memory[2181]=(u8)100;dong_porf_porf_todo_memory[2182]=(u8)111;dong_porf_porf_todo_memory[2183]=(u8)110;dong_porf_porf_todo_memory[2184]=(u8)101;
  dong_porf_porf_todo_memory[2187]=(u8)1;dong_porf_porf_todo_memory[2191]=(u8)86;
  dong_porf_porf_todo_memory[2194]=(u8)20;dong_porf_porf_todo_memory[2198]=(u8)60;dong_porf_porf_todo_memory[2199]=(u8)100;dong_porf_porf_todo_memory[2200]=(u8)105;dong_porf_porf_todo_memory[2201]=(u8)118;dong_porf_porf_todo_memory[2202]=(u8)32;dong_porf_porf_todo_memory[2203]=(u8)99;dong_porf_porf_todo_memory[2204]=(u8)108;dong_porf_porf_todo_memory[2205]=(u8)97;dong_porf_porf_todo_memory[2206]=(u8)115;dong_porf_porf_todo_memory[2207]=(u8)115;dong_porf_porf_todo_memory[2208]=(u8)61;dong_porf_porf_todo_memory[2209]=(u8)34;dong_porf_porf_todo_memory[2210]=(u8)116;dong_porf_porf_todo_memory[2211]=(u8)111;dong_porf_porf_todo_memory[2212]=(u8)100;dong_porf_porf_todo_memory[2213]=(u8)111;dong_porf_porf_todo_memory[2214]=(u8)45;dong_porf_porf_todo_memory[2215]=(u8)114;dong_porf_porf_todo_memory[2216]=(u8)111;dong_porf_porf_todo_memory[2217]=(u8)119;
  dong_porf_porf_todo_memory[2220]=(u8)2;dong_porf_porf_todo_memory[2224]=(u8)34;dong_porf_porf_todo_memory[2225]=(u8)62;
  dong_porf_porf_todo_memory[2228]=(u8)48;dong_porf_porf_todo_memory[2232]=(u8)60;dong_porf_porf_todo_memory[2233]=(u8)100;dong_porf_porf_todo_memory[2234]=(u8)105;dong_porf_porf_todo_memory[2235]=(u8)118;dong_porf_porf_todo_memory[2236]=(u8)32;dong_porf_porf_todo_memory[2237]=(u8)99;dong_porf_porf_todo_memory[2238]=(u8)108;dong_porf_porf_todo_memory[2239]=(u8)97;dong_porf_porf_todo_memory[2240]=(u8)115;dong_porf_porf_todo_memory[2241]=(u8)115;dong_porf_porf_todo_memory[2242]=(u8)61;dong_porf_porf_todo_memory[2243]=(u8)34;dong_porf_porf_todo_memory[2244]=(u8)116;dong_porf_porf_todo_memory[2245]=(u8)111;dong_porf_porf_todo_memory[2246]=(u8)100;dong_porf_porf_todo_memory[2247]=(u8)111;dong_porf_porf_todo_memory[2248]=(u8)45;dong_porf_porf_todo_memory[2249]=(u8)99;dong_porf_porf_todo_memory[2250]=(u8)104;dong_porf_porf_todo_memory[2251]=(u8)101;dong_porf_porf_todo_memory[2252]=(u8)99;dong_porf_porf_todo_memory[2253]=(u8)107;dong_porf_porf_todo_memory[2254]=(u8)34;dong_porf_porf_todo_memory[2255]=(u8)32;dong_porf_porf_todo_memory[2256]=(u8)100;dong_porf_porf_todo_memory[2257]=(u8)97;dong_porf_porf_todo_memory[2258]=(u8)116;dong_porf_porf_todo_memory[2259]=(u8)97;dong_porf_porf_todo_memory[2260]=(u8)45;dong_porf_porf_todo_memory[2261]=(u8)116;dong_porf_porf_todo_memory[2262]=(u8)111;dong_porf_porf_todo_memory[2263]=(u8)100;dong_porf_porf_todo_memory[2264]=(u8)111;dong_porf_porf_todo_memory[2265]=(u8)45;dong_porf_porf_todo_memory[2266]=(u8)116;dong_porf_porf_todo_memory[2267]=(u8)111;dong_porf_porf_todo_memory[2268]=(u8)103;dong_porf_porf_todo_memory[2269]=(u8)103;dong_porf_porf_todo_memory[2270]=(u8)108;dong_porf_porf_todo_memory[2271]=(u8)101;dong_porf_porf_todo_memory[2272]=(u8)45;dong_porf_porf_todo_memory[2273]=(u8)105;dong_porf_porf_todo_memory[2274]=(u8)110;dong_porf_porf_todo_memory[2275]=(u8)100;dong_porf_porf_todo_memory[2276]=(u8)101;dong_porf_porf_todo_memory[2277]=(u8)120;dong_porf_porf_todo_memory[2278]=(u8)61;dong_porf_porf_todo_memory[2279]=(u8)34;
  dong_porf_porf_todo_memory[2282]=(u8)6;dong_porf_porf_todo_memory[2286]=(u8)60;dong_porf_porf_todo_memory[2287]=(u8)47;dong_porf_porf_todo_memory[2288]=(u8)100;dong_porf_porf_todo_memory[2289]=(u8)105;dong_porf_porf_todo_memory[2290]=(u8)118;dong_porf_porf_todo_memory[2291]=(u8)62;
  dong_porf_porf_todo_memory[2294]=(u8)24;dong_porf_porf_todo_memory[2298]=(u8)60;dong_porf_porf_todo_memory[2299]=(u8)115;dong_porf_porf_todo_memory[2300]=(u8)112;dong_porf_porf_todo_memory[2301]=(u8)97;dong_porf_porf_todo_memory[2302]=(u8)110;dong_porf_porf_todo_memory[2303]=(u8)32;dong_porf_porf_todo_memory[2304]=(u8)99;dong_porf_porf_todo_memory[2305]=(u8)108;dong_porf_porf_todo_memory[2306]=(u8)97;dong_porf_porf_todo_memory[2307]=(u8)115;dong_porf_porf_todo_memory[2308]=(u8)115;dong_porf_porf_todo_memory[2309]=(u8)61;dong_porf_porf_todo_memory[2310]=(u8)34;dong_porf_porf_todo_memory[2311]=(u8)116;dong_porf_porf_todo_memory[2312]=(u8)111;dong_porf_porf_todo_memory[2313]=(u8)100;dong_porf_porf_todo_memory[2314]=(u8)111;dong_porf_porf_todo_memory[2315]=(u8)45;dong_porf_porf_todo_memory[2316]=(u8)116;dong_porf_porf_todo_memory[2317]=(u8)101;dong_porf_porf_todo_memory[2318]=(u8)120;dong_porf_porf_todo_memory[2319]=(u8)116;dong_porf_porf_todo_memory[2320]=(u8)34;dong_porf_porf_todo_memory[2321]=(u8)62;
  dong_porf_porf_todo_memory[2324]=(u8)7;dong_porf_porf_todo_memory[2328]=(u8)60;dong_porf_porf_todo_memory[2329]=(u8)47;dong_porf_porf_todo_memory[2330]=(u8)115;dong_porf_porf_todo_memory[2331]=(u8)112;dong_porf_porf_todo_memory[2332]=(u8)97;dong_porf_porf_todo_memory[2333]=(u8)110;dong_porf_porf_todo_memory[2334]=(u8)62;
  dong_porf_porf_todo_memory[2337]=(u8)52;dong_porf_porf_todo_memory[2341]=(u8)60;dong_porf_porf_todo_memory[2342]=(u8)98;dong_porf_porf_todo_memory[2343]=(u8)117;dong_porf_porf_todo_memory[2344]=(u8)116;dong_porf_porf_todo_memory[2345]=(u8)116;dong_porf_porf_todo_memory[2346]=(u8)111;dong_porf_porf_todo_memory[2347]=(u8)110;dong_porf_porf_todo_memory[2348]=(u8)32;dong_porf_porf_todo_memory[2349]=(u8)99;dong_porf_porf_todo_memory[2350]=(u8)108;dong_porf_porf_todo_memory[2351]=(u8)97;dong_porf_porf_todo_memory[2352]=(u8)115;dong_porf_porf_todo_memory[2353]=(u8)115;dong_porf_porf_todo_memory[2354]=(u8)61;dong_porf_porf_todo_memory[2355]=(u8)34;dong_porf_porf_todo_memory[2356]=(u8)116;dong_porf_porf_todo_memory[2357]=(u8)111;dong_porf_porf_todo_memory[2358]=(u8)100;dong_porf_porf_todo_memory[2359]=(u8)111;dong_porf_porf_todo_memory[2360]=(u8)45;dong_porf_porf_todo_memory[2361]=(u8)100;dong_porf_porf_todo_memory[2362]=(u8)101;dong_porf_porf_todo_memory[2363]=(u8)108;dong_porf_porf_todo_memory[2364]=(u8)101;dong_porf_porf_todo_memory[2365]=(u8)116;dong_porf_porf_todo_memory[2366]=(u8)101;dong_porf_porf_todo_memory[2367]=(u8)34;dong_porf_porf_todo_memory[2368]=(u8)32;dong_porf_porf_todo_memory[2369]=(u8)100;dong_porf_porf_todo_memory[2370]=(u8)97;dong_porf_porf_todo_memory[2371]=(u8)116;dong_porf_porf_todo_memory[2372]=(u8)97;dong_porf_porf_todo_memory[2373]=(u8)45;dong_porf_porf_todo_memory[2374]=(u8)116;dong_porf_porf_todo_memory[2375]=(u8)111;dong_porf_porf_todo_memory[2376]=(u8)100;dong_porf_porf_todo_memory[2377]=(u8)111;dong_porf_porf_todo_memory[2378]=(u8)45;dong_porf_porf_todo_memory[2379]=(u8)100;dong_porf_porf_todo_memory[2380]=(u8)101;dong_porf_porf_todo_memory[2381]=(u8)108;dong_porf_porf_todo_memory[2382]=(u8)101;dong_porf_porf_todo_memory[2383]=(u8)116;dong_porf_porf_todo_memory[2384]=(u8)101;dong_porf_porf_todo_memory[2385]=(u8)45;dong_porf_porf_todo_memory[2386]=(u8)105;dong_porf_porf_todo_memory[2387]=(u8)110;dong_porf_porf_todo_memory[2388]=(u8)100;dong_porf_porf_todo_memory[2389]=(u8)101;dong_porf_porf_todo_memory[2390]=(u8)120;dong_porf_porf_todo_memory[2391]=(u8)61;dong_porf_porf_todo_memory[2392]=(u8)34;
  dong_porf_porf_todo_memory[2395]=(u8)12;dong_porf_porf_todo_memory[2399]=(u8)34;dong_porf_porf_todo_memory[2400]=(u8)62;dong_porf_porf_todo_memory[2401]=(u8)88;dong_porf_porf_todo_memory[2402]=(u8)60;dong_porf_porf_todo_memory[2403]=(u8)47;dong_porf_porf_todo_memory[2404]=(u8)98;dong_porf_porf_todo_memory[2405]=(u8)117;dong_porf_porf_todo_memory[2406]=(u8)116;dong_porf_porf_todo_memory[2407]=(u8)116;dong_porf_porf_todo_memory[2408]=(u8)111;dong_porf_porf_todo_memory[2409]=(u8)110;dong_porf_porf_todo_memory[2410]=(u8)62;
  dong_porf_porf_todo_memory[2413]=(u8)18;dong_porf_porf_todo_memory[2417]=(u8)80;dong_porf_porf_todo_memory[2418]=(u8)111;dong_porf_porf_todo_memory[2419]=(u8)114;dong_porf_porf_todo_memory[2420]=(u8)102;dong_porf_porf_todo_memory[2421]=(u8)102;dong_porf_porf_todo_memory[2422]=(u8)111;dong_porf_porf_todo_memory[2423]=(u8)114;dong_porf_porf_todo_memory[2424]=(u8)32;dong_porf_porf_todo_memory[2425]=(u8)115;dong_porf_porf_todo_memory[2426]=(u8)109;dong_porf_porf_todo_memory[2427]=(u8)111;dong_porf_porf_todo_memory[2428]=(u8)107;dong_porf_porf_todo_memory[2429]=(u8)101;dong_porf_porf_todo_memory[2430]=(u8)32;dong_porf_porf_todo_memory[2431]=(u8)116;dong_porf_porf_todo_memory[2432]=(u8)97;dong_porf_porf_todo_memory[2433]=(u8)115;dong_porf_porf_todo_memory[2434]=(u8)107;
  dong_porf_porf_todo_memory[2437]=(u8)5;dong_porf_porf_todo_memory[2441]=(u8)69;dong_porf_porf_todo_memory[2442]=(u8)110;dong_porf_porf_todo_memory[2443]=(u8)116;dong_porf_porf_todo_memory[2444]=(u8)101;dong_porf_porf_todo_memory[2445]=(u8)114;
  dong_porf_porf_todo_memory[2448]=(u8)22;dong_porf_porf_todo_memory[2452]=(u8)100;dong_porf_porf_todo_memory[2453]=(u8)97;dong_porf_porf_todo_memory[2454]=(u8)116;dong_porf_porf_todo_memory[2455]=(u8)97;dong_porf_porf_todo_memory[2456]=(u8)45;dong_porf_porf_todo_memory[2457]=(u8)116;dong_porf_porf_todo_memory[2458]=(u8)111;dong_porf_porf_todo_memory[2459]=(u8)100;dong_porf_porf_todo_memory[2460]=(u8)111;dong_porf_porf_todo_memory[2461]=(u8)45;dong_porf_porf_todo_memory[2462]=(u8)116;dong_porf_porf_todo_memory[2463]=(u8)111;dong_porf_porf_todo_memory[2464]=(u8)103;dong_porf_porf_todo_memory[2465]=(u8)103;dong_porf_porf_todo_memory[2466]=(u8)108;dong_porf_porf_todo_memory[2467]=(u8)101;dong_porf_porf_todo_memory[2468]=(u8)45;dong_porf_porf_todo_memory[2469]=(u8)105;dong_porf_porf_todo_memory[2470]=(u8)110;dong_porf_porf_todo_memory[2471]=(u8)100;dong_porf_porf_todo_memory[2472]=(u8)101;dong_porf_porf_todo_memory[2473]=(u8)120;
  dong_porf_porf_todo_memory[2476]=(u8)1;dong_porf_porf_todo_memory[2480]=(u8)50;
  dong_porf_porf_todo_memory[2483]=(u8)1;dong_porf_porf_todo_memory[2487]=(u8)51;
  dong_porf_porf_todo_memory[2490]=(u8)1;dong_porf_porf_todo_memory[2494]=(u8)52;
  dong_porf_porf_todo_memory[2497]=(u8)1;dong_porf_porf_todo_memory[2501]=(u8)53;
  dong_porf_porf_todo_memory[2504]=(u8)1;dong_porf_porf_todo_memory[2508]=(u8)54;
  dong_porf_porf_todo_memory[2511]=(u8)1;dong_porf_porf_todo_memory[2515]=(u8)55;
  dong_porf_porf_todo_memory[2518]=(u8)1;dong_porf_porf_todo_memory[2522]=(u8)56;
  dong_porf_porf_todo_memory[2525]=(u8)1;dong_porf_porf_todo_memory[2529]=(u8)57;
  dong_porf_porf_todo_memory[2532]=(u8)2;dong_porf_porf_todo_memory[2536]=(u8)49;dong_porf_porf_todo_memory[2537]=(u8)48;
  dong_porf_porf_todo_memory[2540]=(u8)2;dong_porf_porf_todo_memory[2544]=(u8)49;dong_porf_porf_todo_memory[2545]=(u8)49;
  dong_porf_porf_todo_memory[2548]=(u8)2;dong_porf_porf_todo_memory[2552]=(u8)49;dong_porf_porf_todo_memory[2553]=(u8)50;
  dong_porf_porf_todo_memory[2556]=(u8)2;dong_porf_porf_todo_memory[2560]=(u8)49;dong_porf_porf_todo_memory[2561]=(u8)51;
  dong_porf_porf_todo_memory[2564]=(u8)2;dong_porf_porf_todo_memory[2568]=(u8)49;dong_porf_porf_todo_memory[2569]=(u8)52;
  dong_porf_porf_todo_memory[2572]=(u8)2;dong_porf_porf_todo_memory[2576]=(u8)49;dong_porf_porf_todo_memory[2577]=(u8)53;
  dong_porf_porf_todo_memory[2580]=(u8)24;dong_porf_porf_todo_memory[2584]=(u8)91;dong_porf_porf_todo_memory[2585]=(u8)100;dong_porf_porf_todo_memory[2586]=(u8)97;dong_porf_porf_todo_memory[2587]=(u8)116;dong_porf_porf_todo_memory[2588]=(u8)97;dong_porf_porf_todo_memory[2589]=(u8)45;dong_porf_porf_todo_memory[2590]=(u8)116;dong_porf_porf_todo_memory[2591]=(u8)111;dong_porf_porf_todo_memory[2592]=(u8)100;dong_porf_porf_todo_memory[2593]=(u8)111;dong_porf_porf_todo_memory[2594]=(u8)45;dong_porf_porf_todo_memory[2595]=(u8)116;dong_porf_porf_todo_memory[2596]=(u8)111;dong_porf_porf_todo_memory[2597]=(u8)103;dong_porf_porf_todo_memory[2598]=(u8)103;dong_porf_porf_todo_memory[2599]=(u8)108;dong_porf_porf_todo_memory[2600]=(u8)101;dong_porf_porf_todo_memory[2601]=(u8)45;dong_porf_porf_todo_memory[2602]=(u8)105;dong_porf_porf_todo_memory[2603]=(u8)110;dong_porf_porf_todo_memory[2604]=(u8)100;dong_porf_porf_todo_memory[2605]=(u8)101;dong_porf_porf_todo_memory[2606]=(u8)120;dong_porf_porf_todo_memory[2607]=(u8)93;
  dong_porf_porf_todo_memory[2610]=(u8)22;dong_porf_porf_todo_memory[2614]=(u8)100;dong_porf_porf_todo_memory[2615]=(u8)97;dong_porf_porf_todo_memory[2616]=(u8)116;dong_porf_porf_todo_memory[2617]=(u8)97;dong_porf_porf_todo_memory[2618]=(u8)45;dong_porf_porf_todo_memory[2619]=(u8)116;dong_porf_porf_todo_memory[2620]=(u8)111;dong_porf_porf_todo_memory[2621]=(u8)100;dong_porf_porf_todo_memory[2622]=(u8)111;dong_porf_porf_todo_memory[2623]=(u8)45;dong_porf_porf_todo_memory[2624]=(u8)100;dong_porf_porf_todo_memory[2625]=(u8)101;dong_porf_porf_todo_memory[2626]=(u8)108;dong_porf_porf_todo_memory[2627]=(u8)101;dong_porf_porf_todo_memory[2628]=(u8)116;dong_porf_porf_todo_memory[2629]=(u8)101;dong_porf_porf_todo_memory[2630]=(u8)45;dong_porf_porf_todo_memory[2631]=(u8)105;dong_porf_porf_todo_memory[2632]=(u8)110;dong_porf_porf_todo_memory[2633]=(u8)100;dong_porf_porf_todo_memory[2634]=(u8)101;dong_porf_porf_todo_memory[2635]=(u8)120;
  dong_porf_porf_todo_memory[2638]=(u8)24;dong_porf_porf_todo_memory[2642]=(u8)91;dong_porf_porf_todo_memory[2643]=(u8)100;dong_porf_porf_todo_memory[2644]=(u8)97;dong_porf_porf_todo_memory[2645]=(u8)116;dong_porf_porf_todo_memory[2646]=(u8)97;dong_porf_porf_todo_memory[2647]=(u8)45;dong_porf_porf_todo_memory[2648]=(u8)116;dong_porf_porf_todo_memory[2649]=(u8)111;dong_porf_porf_todo_memory[2650]=(u8)100;dong_porf_porf_todo_memory[2651]=(u8)111;dong_porf_porf_todo_memory[2652]=(u8)45;dong_porf_porf_todo_memory[2653]=(u8)100;dong_porf_porf_todo_memory[2654]=(u8)101;dong_porf_porf_todo_memory[2655]=(u8)108;dong_porf_porf_todo_memory[2656]=(u8)101;dong_porf_porf_todo_memory[2657]=(u8)116;dong_porf_porf_todo_memory[2658]=(u8)101;dong_porf_porf_todo_memory[2659]=(u8)45;dong_porf_porf_todo_memory[2660]=(u8)105;dong_porf_porf_todo_memory[2661]=(u8)110;dong_porf_porf_todo_memory[2662]=(u8)100;dong_porf_porf_todo_memory[2663]=(u8)101;dong_porf_porf_todo_memory[2664]=(u8)120;dong_porf_porf_todo_memory[2665]=(u8)93;
  dong_porf_porf_todo_memory[2668]=(u8)9;dong_porf_porf_todo_memory[2672]=(u8)112;dong_porf_porf_todo_memory[2673]=(u8)111;dong_porf_porf_todo_memory[2674]=(u8)114;dong_porf_porf_todo_memory[2675]=(u8)102;dong_porf_porf_todo_memory[2676]=(u8)45;dong_porf_porf_todo_memory[2677]=(u8)114;dong_porf_porf_todo_memory[2678]=(u8)111;dong_porf_porf_todo_memory[2679]=(u8)111;dong_porf_porf_todo_memory[2680]=(u8)116;
  dong_porf_porf_todo_memory[2683]=(u8)5;dong_porf_porf_todo_memory[2687]=(u8)116;dong_porf_porf_todo_memory[2688]=(u8)105;dong_porf_porf_todo_memory[2689]=(u8)116;dong_porf_porf_todo_memory[2690]=(u8)108;dong_porf_porf_todo_memory[2691]=(u8)101;
  dong_porf_porf_todo_memory[2694]=(u8)10;dong_porf_porf_todo_memory[2698]=(u8)116;dong_porf_porf_todo_memory[2699]=(u8)111;dong_porf_porf_todo_memory[2700]=(u8)100;dong_porf_porf_todo_memory[2701]=(u8)111;dong_porf_porf_todo_memory[2702]=(u8)45;dong_porf_porf_todo_memory[2703]=(u8)105;dong_porf_porf_todo_memory[2704]=(u8)110;dong_porf_porf_todo_memory[2705]=(u8)112;dong_porf_porf_todo_memory[2706]=(u8)117;dong_porf_porf_todo_memory[2707]=(u8)116;
  dong_porf_porf_todo_memory[2710]=(u8)7;dong_porf_porf_todo_memory[2714]=(u8)98;dong_porf_porf_todo_memory[2715]=(u8)116;dong_porf_porf_todo_memory[2716]=(u8)110;dong_porf_porf_todo_memory[2717]=(u8)45;dong_porf_porf_todo_memory[2718]=(u8)97;dong_porf_porf_todo_memory[2719]=(u8)100;dong_porf_porf_todo_memory[2720]=(u8)100;
  dong_porf_porf_todo_memory[2723]=(u8)10;dong_porf_porf_todo_memory[2727]=(u8)102;dong_porf_porf_todo_memory[2728]=(u8)105;dong_porf_porf_todo_memory[2729]=(u8)108;dong_porf_porf_todo_memory[2730]=(u8)116;dong_porf_porf_todo_memory[2731]=(u8)101;dong_porf_porf_todo_memory[2732]=(u8)114;dong_porf_porf_todo_memory[2733]=(u8)45;dong_porf_porf_todo_memory[2734]=(u8)98;dong_porf_porf_todo_memory[2735]=(u8)97;dong_porf_porf_todo_memory[2736]=(u8)114;
  dong_porf_porf_todo_memory[2739]=(u8)10;dong_porf_porf_todo_memory[2743]=(u8)102;dong_porf_porf_todo_memory[2744]=(u8)105;dong_porf_porf_todo_memory[2745]=(u8)108;dong_porf_porf_todo_memory[2746]=(u8)116;dong_porf_porf_todo_memory[2747]=(u8)101;dong_porf_porf_todo_memory[2748]=(u8)114;dong_porf_porf_todo_memory[2749]=(u8)45;dong_porf_porf_todo_memory[2750]=(u8)97;dong_porf_porf_todo_memory[2751]=(u8)108;dong_porf_porf_todo_memory[2752]=(u8)108;
  dong_porf_porf_todo_memory[2755]=(u8)13;dong_porf_porf_todo_memory[2759]=(u8)102;dong_porf_porf_todo_memory[2760]=(u8)105;dong_porf_porf_todo_memory[2761]=(u8)108;dong_porf_porf_todo_memory[2762]=(u8)116;dong_porf_porf_todo_memory[2763]=(u8)101;dong_porf_porf_todo_memory[2764]=(u8)114;dong_porf_porf_todo_memory[2765]=(u8)45;dong_porf_porf_todo_memory[2766]=(u8)97;dong_porf_porf_todo_memory[2767]=(u8)99;dong_porf_porf_todo_memory[2768]=(u8)116;dong_porf_porf_todo_memory[2769]=(u8)105;dong_porf_porf_todo_memory[2770]=(u8)118;dong_porf_porf_todo_memory[2771]=(u8)101;
  dong_porf_porf_todo_memory[2774]=(u8)11;dong_porf_porf_todo_memory[2778]=(u8)102;dong_porf_porf_todo_memory[2779]=(u8)105;dong_porf_porf_todo_memory[2780]=(u8)108;dong_porf_porf_todo_memory[2781]=(u8)116;dong_porf_porf_todo_memory[2782]=(u8)101;dong_porf_porf_todo_memory[2783]=(u8)114;dong_porf_porf_todo_memory[2784]=(u8)45;dong_porf_porf_todo_memory[2785]=(u8)100;dong_porf_porf_todo_memory[2786]=(u8)111;dong_porf_porf_todo_memory[2787]=(u8)110;dong_porf_porf_todo_memory[2788]=(u8)101;
  dong_porf_porf_todo_memory[2791]=(u8)9;dong_porf_porf_todo_memory[2795]=(u8)116;dong_porf_porf_todo_memory[2796]=(u8)111;dong_porf_porf_todo_memory[2797]=(u8)100;dong_porf_porf_todo_memory[2798]=(u8)111;dong_porf_porf_todo_memory[2799]=(u8)45;dong_porf_porf_todo_memory[2800]=(u8)108;dong_porf_porf_todo_memory[2801]=(u8)105;dong_porf_porf_todo_memory[2802]=(u8)115;dong_porf_porf_todo_memory[2803]=(u8)116;
  dong_porf_porf_todo_memory[2806]=(u8)10;dong_porf_porf_todo_memory[2810]=(u8)99;dong_porf_porf_todo_memory[2811]=(u8)108;dong_porf_porf_todo_memory[2812]=(u8)101;dong_porf_porf_todo_memory[2813]=(u8)97;dong_porf_porf_todo_memory[2814]=(u8)114;dong_porf_porf_todo_memory[2815]=(u8)45;dong_porf_porf_todo_memory[2816]=(u8)119;dong_porf_porf_todo_memory[2817]=(u8)114;dong_porf_porf_todo_memory[2818]=(u8)97;dong_porf_porf_todo_memory[2819]=(u8)112;
  dong_porf_porf_todo_memory[2822]=(u8)9;dong_porf_porf_todo_memory[2826]=(u8)98;dong_porf_porf_todo_memory[2827]=(u8)116;dong_porf_porf_todo_memory[2828]=(u8)110;dong_porf_porf_todo_memory[2829]=(u8)45;dong_porf_porf_todo_memory[2830]=(u8)99;dong_porf_porf_todo_memory[2831]=(u8)108;dong_porf_porf_todo_memory[2832]=(u8)101;dong_porf_porf_todo_memory[2833]=(u8)97;dong_porf_porf_todo_memory[2834]=(u8)114;
  dong_porf_porf_todo_memory[2837]=(u8)5;dong_porf_porf_todo_memory[2841]=(u8)105;dong_porf_porf_todo_memory[2842]=(u8)110;dong_porf_porf_todo_memory[2843]=(u8)112;dong_porf_porf_todo_memory[2844]=(u8)117;dong_porf_porf_todo_memory[2845]=(u8)116;
  dong_porf_porf_todo_memory[2848]=(u8)13;dong_porf_porf_todo_memory[2852]=(u8)111;dong_porf_porf_todo_memory[2853]=(u8)110;dong_porf_porf_todo_memory[2854]=(u8)73;dong_porf_porf_todo_memory[2855]=(u8)110;dong_porf_porf_todo_memory[2856]=(u8)112;dong_porf_porf_todo_memory[2857]=(u8)117;dong_porf_porf_todo_memory[2858]=(u8)116;dong_porf_porf_todo_memory[2859]=(u8)67;dong_porf_porf_todo_memory[2860]=(u8)104;dong_porf_porf_todo_memory[2861]=(u8)97;dong_porf_porf_todo_memory[2862]=(u8)110;dong_porf_porf_todo_memory[2863]=(u8)103;dong_porf_porf_todo_memory[2864]=(u8)101;
  dong_porf_porf_todo_memory[2867]=(u8)7;dong_porf_porf_todo_memory[2871]=(u8)107;dong_porf_porf_todo_memory[2872]=(u8)101;dong_porf_porf_todo_memory[2873]=(u8)121;dong_porf_porf_todo_memory[2874]=(u8)100;dong_porf_porf_todo_memory[2875]=(u8)111;dong_porf_porf_todo_memory[2876]=(u8)119;dong_porf_porf_todo_memory[2877]=(u8)110;
  dong_porf_porf_todo_memory[2880]=(u8)9;dong_porf_porf_todo_memory[2884]=(u8)111;dong_porf_porf_todo_memory[2885]=(u8)110;dong_porf_porf_todo_memory[2886]=(u8)75;dong_porf_porf_todo_memory[2887]=(u8)101;dong_porf_porf_todo_memory[2888]=(u8)121;dong_porf_porf_todo_memory[2889]=(u8)68;dong_porf_porf_todo_memory[2890]=(u8)111;dong_porf_porf_todo_memory[2891]=(u8)119;dong_porf_porf_todo_memory[2892]=(u8)110;
  dong_porf_porf_todo_memory[2895]=(u8)5;dong_porf_porf_todo_memory[2899]=(u8)99;dong_porf_porf_todo_memory[2900]=(u8)108;dong_porf_porf_todo_memory[2901]=(u8)105;dong_porf_porf_todo_memory[2902]=(u8)99;dong_porf_porf_todo_memory[2903]=(u8)107;
  dong_porf_porf_todo_memory[2906]=(u8)5;dong_porf_porf_todo_memory[2910]=(u8)111;dong_porf_porf_todo_memory[2911]=(u8)110;dong_porf_porf_todo_memory[2912]=(u8)65;dong_porf_porf_todo_memory[2913]=(u8)100;dong_porf_porf_todo_memory[2914]=(u8)100;
  dong_porf_porf_todo_memory[2917]=(u8)11;dong_porf_porf_todo_memory[2921]=(u8)111;dong_porf_porf_todo_memory[2922]=(u8)110;dong_porf_porf_todo_memory[2923]=(u8)70;dong_porf_porf_todo_memory[2924]=(u8)105;dong_porf_porf_todo_memory[2925]=(u8)108;dong_porf_porf_todo_memory[2926]=(u8)116;dong_porf_porf_todo_memory[2927]=(u8)101;dong_porf_porf_todo_memory[2928]=(u8)114;dong_porf_porf_todo_memory[2929]=(u8)65;dong_porf_porf_todo_memory[2930]=(u8)108;dong_porf_porf_todo_memory[2931]=(u8)108;
  dong_porf_porf_todo_memory[2934]=(u8)14;dong_porf_porf_todo_memory[2938]=(u8)111;dong_porf_porf_todo_memory[2939]=(u8)110;dong_porf_porf_todo_memory[2940]=(u8)70;dong_porf_porf_todo_memory[2941]=(u8)105;dong_porf_porf_todo_memory[2942]=(u8)108;dong_porf_porf_todo_memory[2943]=(u8)116;dong_porf_porf_todo_memory[2944]=(u8)101;dong_porf_porf_todo_memory[2945]=(u8)114;dong_porf_porf_todo_memory[2946]=(u8)65;dong_porf_porf_todo_memory[2947]=(u8)99;dong_porf_porf_todo_memory[2948]=(u8)116;dong_porf_porf_todo_memory[2949]=(u8)105;dong_porf_porf_todo_memory[2950]=(u8)118;dong_porf_porf_todo_memory[2951]=(u8)101;
  dong_porf_porf_todo_memory[2954]=(u8)12;dong_porf_porf_todo_memory[2958]=(u8)111;dong_porf_porf_todo_memory[2959]=(u8)110;dong_porf_porf_todo_memory[2960]=(u8)70;dong_porf_porf_todo_memory[2961]=(u8)105;dong_porf_porf_todo_memory[2962]=(u8)108;dong_porf_porf_todo_memory[2963]=(u8)116;dong_porf_porf_todo_memory[2964]=(u8)101;dong_porf_porf_todo_memory[2965]=(u8)114;dong_porf_porf_todo_memory[2966]=(u8)68;dong_porf_porf_todo_memory[2967]=(u8)111;dong_porf_porf_todo_memory[2968]=(u8)110;dong_porf_porf_todo_memory[2969]=(u8)101;
  dong_porf_porf_todo_memory[2972]=(u8)11;dong_porf_porf_todo_memory[2976]=(u8)111;dong_porf_porf_todo_memory[2977]=(u8)110;dong_porf_porf_todo_memory[2978]=(u8)76;dong_porf_porf_todo_memory[2979]=(u8)105;dong_porf_porf_todo_memory[2980]=(u8)115;dong_porf_porf_todo_memory[2981]=(u8)116;dong_porf_porf_todo_memory[2982]=(u8)67;dong_porf_porf_todo_memory[2983]=(u8)108;dong_porf_porf_todo_memory[2984]=(u8)105;dong_porf_porf_todo_memory[2985]=(u8)99;dong_porf_porf_todo_memory[2986]=(u8)107;
  dong_porf_porf_todo_memory[2989]=(u8)11;dong_porf_porf_todo_memory[2993]=(u8)111;dong_porf_porf_todo_memory[2994]=(u8)110;dong_porf_porf_todo_memory[2995]=(u8)67;dong_porf_porf_todo_memory[2996]=(u8)108;dong_porf_porf_todo_memory[2997]=(u8)101;dong_porf_porf_todo_memory[2998]=(u8)97;dong_porf_porf_todo_memory[2999]=(u8)114;dong_porf_porf_todo_memory[3000]=(u8)68;dong_porf_porf_todo_memory[3001]=(u8)111;dong_porf_porf_todo_memory[3002]=(u8)110;dong_porf_porf_todo_memory[3003]=(u8)101;
  dong_porf_porf_todo_memory[3006]=(u8)8;dong_porf_porf_todo_memory[3010]=(u8)112;dong_porf_porf_todo_memory[3011]=(u8)111;dong_porf_porf_todo_memory[3012]=(u8)114;dong_porf_porf_todo_memory[3013]=(u8)102;dong_porf_porf_todo_memory[3014]=(u8)73;dong_porf_porf_todo_memory[3015]=(u8)110;dong_porf_porf_todo_memory[3016]=(u8)105;dong_porf_porf_todo_memory[3017]=(u8)116;
  dong_porf_porf_todo_memory[3020]=(u8)58;dong_porf_porf_todo_memory[3024]=(u8)83;dong_porf_porf_todo_memory[3025]=(u8)116;dong_porf_porf_todo_memory[3026]=(u8)114;dong_porf_porf_todo_memory[3027]=(u8)105;dong_porf_porf_todo_memory[3028]=(u8)110;dong_porf_porf_todo_memory[3029]=(u8)103;dong_porf_porf_todo_memory[3030]=(u8)46;dong_porf_porf_todo_memory[3031]=(u8)112;dong_porf_porf_todo_memory[3032]=(u8)114;dong_porf_porf_todo_memory[3033]=(u8)111;dong_porf_porf_todo_memory[3034]=(u8)116;dong_porf_porf_todo_memory[3035]=(u8)111;dong_porf_porf_todo_memory[3036]=(u8)116;dong_porf_porf_todo_memory[3037]=(u8)121;dong_porf_porf_todo_memory[3038]=(u8)112;dong_porf_porf_todo_memory[3039]=(u8)101;dong_porf_porf_todo_memory[3040]=(u8)46;dong_porf_porf_todo_memory[3041]=(u8)116;dong_porf_porf_todo_memory[3042]=(u8)111;dong_porf_porf_todo_memory[3043]=(u8)83;dong_porf_porf_todo_memory[3044]=(u8)116;dong_porf_porf_todo_memory[3045]=(u8)114;dong_porf_porf_todo_memory[3046]=(u8)105;dong_porf_porf_todo_memory[3047]=(u8)110;dong_porf_porf_todo_memory[3048]=(u8)103;dong_porf_porf_todo_memory[3049]=(u8)32;dong_porf_porf_todo_memory[3050]=(u8)101;dong_porf_porf_todo_memory[3051]=(u8)120;dong_porf_porf_todo_memory[3052]=(u8)112;dong_porf_porf_todo_memory[3053]=(u8)101;dong_porf_porf_todo_memory[3054]=(u8)99;dong_porf_porf_todo_memory[3055]=(u8)116;dong_porf_porf_todo_memory[3056]=(u8)115;dong_porf_porf_todo_memory[3057]=(u8)32;dong_porf_porf_todo_memory[3058]=(u8)39;dong_porf_porf_todo_memory[3059]=(u8)116;dong_porf_porf_todo_memory[3060]=(u8)104;dong_porf_porf_todo_memory[3061]=(u8)105;dong_porf_porf_todo_memory[3062]=(u8)115;dong_porf_porf_todo_memory[3063]=(u8)39;dong_porf_porf_todo_memory[3064]=(u8)32;dong_porf_porf_todo_memory[3065]=(u8)116;dong_porf_porf_todo_memory[3066]=(u8)111;dong_porf_porf_todo_memory[3067]=(u8)32;dong_porf_porf_todo_memory[3068]=(u8)98;dong_porf_porf_todo_memory[3069]=(u8)101;dong_porf_porf_todo_memory[3070]=(u8)32;dong_porf_porf_todo_memory[3071]=(u8)110;dong_porf_porf_todo_memory[3072]=(u8)111;dong_porf_porf_todo_memory[3073]=(u8)110;dong_porf_porf_todo_memory[3074]=(u8)45;dong_porf_porf_todo_memory[3075]=(u8)110;dong_porf_porf_todo_memory[3076]=(u8)117;dong_porf_porf_todo_memory[3077]=(u8)108;dong_porf_porf_todo_memory[3078]=(u8)108;dong_porf_porf_todo_memory[3079]=(u8)105;dong_porf_porf_todo_memory[3080]=(u8)115;dong_porf_porf_todo_memory[3081]=(u8)104;
  dong_porf_porf_todo_memory[3084]=(u8)54;dong_porf_porf_todo_memory[3088]=(u8)83;dong_porf_porf_todo_memory[3089]=(u8)116;dong_porf_porf_todo_memory[3090]=(u8)114;dong_porf_porf_todo_memory[3091]=(u8)105;dong_porf_porf_todo_memory[3092]=(u8)110;dong_porf_porf_todo_memory[3093]=(u8)103;dong_porf_porf_todo_memory[3094]=(u8)46;dong_porf_porf_todo_memory[3095]=(u8)112;dong_porf_porf_todo_memory[3096]=(u8)114;dong_porf_porf_todo_memory[3097]=(u8)111;dong_porf_porf_todo_memory[3098]=(u8)116;dong_porf_porf_todo_memory[3099]=(u8)111;dong_porf_porf_todo_memory[3100]=(u8)116;dong_porf_porf_todo_memory[3101]=(u8)121;dong_porf_porf_todo_memory[3102]=(u8)112;dong_porf_porf_todo_memory[3103]=(u8)101;dong_porf_porf_todo_memory[3104]=(u8)46;dong_porf_porf_todo_memory[3105]=(u8)116;dong_porf_porf_todo_memory[3106]=(u8)114;dong_porf_porf_todo_memory[3107]=(u8)105;dong_porf_porf_todo_memory[3108]=(u8)109;dong_porf_porf_todo_memory[3109]=(u8)32;dong_porf_porf_todo_memory[3110]=(u8)101;dong_porf_porf_todo_memory[3111]=(u8)120;dong_porf_porf_todo_memory[3112]=(u8)112;dong_porf_porf_todo_memory[3113]=(u8)101;dong_porf_porf_todo_memory[3114]=(u8)99;dong_porf_porf_todo_memory[3115]=(u8)116;dong_porf_porf_todo_memory[3116]=(u8)115;dong_porf_porf_todo_memory[3117]=(u8)32;dong_porf_porf_todo_memory[3118]=(u8)39;dong_porf_porf_todo_memory[3119]=(u8)116;dong_porf_porf_todo_memory[3120]=(u8)104;dong_porf_porf_todo_memory[3121]=(u8)105;dong_porf_porf_todo_memory[3122]=(u8)115;dong_porf_porf_todo_memory[3123]=(u8)39;dong_porf_porf_todo_memory[3124]=(u8)32;dong_porf_porf_todo_memory[3125]=(u8)116;dong_porf_porf_todo_memory[3126]=(u8)111;dong_porf_porf_todo_memory[3127]=(u8)32;dong_porf_porf_todo_memory[3128]=(u8)98;dong_porf_porf_todo_memory[3129]=(u8)101;dong_porf_porf_todo_memory[3130]=(u8)32;dong_porf_porf_todo_memory[3131]=(u8)110;dong_porf_porf_todo_memory[3132]=(u8)111;dong_porf_porf_todo_memory[3133]=(u8)110;dong_porf_porf_todo_memory[3134]=(u8)45;dong_porf_porf_todo_memory[3135]=(u8)110;dong_porf_porf_todo_memory[3136]=(u8)117;dong_porf_porf_todo_memory[3137]=(u8)108;dong_porf_porf_todo_memory[3138]=(u8)108;dong_porf_porf_todo_memory[3139]=(u8)105;dong_porf_porf_todo_memory[3140]=(u8)115;dong_porf_porf_todo_memory[3141]=(u8)104;
  dong_porf_porf_todo_memory[3144]=(u8)57;dong_porf_porf_todo_memory[3148]=(u8)83;dong_porf_porf_todo_memory[3149]=(u8)116;dong_porf_porf_todo_memory[3150]=(u8)114;dong_porf_porf_todo_memory[3151]=(u8)105;dong_porf_porf_todo_memory[3152]=(u8)110;dong_porf_porf_todo_memory[3153]=(u8)103;dong_porf_porf_todo_memory[3154]=(u8)46;dong_porf_porf_todo_memory[3155]=(u8)112;dong_porf_porf_todo_memory[3156]=(u8)114;dong_porf_porf_todo_memory[3157]=(u8)111;dong_porf_porf_todo_memory[3158]=(u8)116;dong_porf_porf_todo_memory[3159]=(u8)111;dong_porf_porf_todo_memory[3160]=(u8)116;dong_porf_porf_todo_memory[3161]=(u8)121;dong_porf_porf_todo_memory[3162]=(u8)112;dong_porf_porf_todo_memory[3163]=(u8)101;dong_porf_porf_todo_memory[3164]=(u8)46;dong_porf_porf_todo_memory[3165]=(u8)116;dong_porf_porf_todo_memory[3166]=(u8)114;dong_porf_porf_todo_memory[3167]=(u8)105;dong_porf_porf_todo_memory[3168]=(u8)109;dong_porf_porf_todo_memory[3169]=(u8)69;dong_porf_porf_todo_memory[3170]=(u8)110;dong_porf_porf_todo_memory[3171]=(u8)100;dong_porf_porf_todo_memory[3172]=(u8)32;dong_porf_porf_todo_memory[3173]=(u8)101;dong_porf_porf_todo_memory[3174]=(u8)120;dong_porf_porf_todo_memory[3175]=(u8)112;dong_porf_porf_todo_memory[3176]=(u8)101;dong_porf_porf_todo_memory[3177]=(u8)99;dong_porf_porf_todo_memory[3178]=(u8)116;dong_porf_porf_todo_memory[3179]=(u8)115;dong_porf_porf_todo_memory[3180]=(u8)32;dong_porf_porf_todo_memory[3181]=(u8)39;dong_porf_porf_todo_memory[3182]=(u8)116;dong_porf_porf_todo_memory[3183]=(u8)104;dong_porf_porf_todo_memory[3184]=(u8)105;dong_porf_porf_todo_memory[3185]=(u8)115;dong_porf_porf_todo_memory[3186]=(u8)39;dong_porf_porf_todo_memory[3187]=(u8)32;dong_porf_porf_todo_memory[3188]=(u8)116;dong_porf_porf_todo_memory[3189]=(u8)111;dong_porf_porf_todo_memory[3190]=(u8)32;dong_porf_porf_todo_memory[3191]=(u8)98;dong_porf_porf_todo_memory[3192]=(u8)101;dong_porf_porf_todo_memory[3193]=(u8)32;dong_porf_porf_todo_memory[3194]=(u8)110;dong_porf_porf_todo_memory[3195]=(u8)111;dong_porf_porf_todo_memory[3196]=(u8)110;dong_porf_porf_todo_memory[3197]=(u8)45;dong_porf_porf_todo_memory[3198]=(u8)110;dong_porf_porf_todo_memory[3199]=(u8)117;dong_porf_porf_todo_memory[3200]=(u8)108;dong_porf_porf_todo_memory[3201]=(u8)108;dong_porf_porf_todo_memory[3202]=(u8)105;dong_porf_porf_todo_memory[3203]=(u8)115;dong_porf_porf_todo_memory[3204]=(u8)104;
  dong_porf_porf_todo_memory[3207]=(u8)59;dong_porf_porf_todo_memory[3211]=(u8)83;dong_porf_porf_todo_memory[3212]=(u8)116;dong_porf_porf_todo_memory[3213]=(u8)114;dong_porf_porf_todo_memory[3214]=(u8)105;dong_porf_porf_todo_memory[3215]=(u8)110;dong_porf_porf_todo_memory[3216]=(u8)103;dong_porf_porf_todo_memory[3217]=(u8)46;dong_porf_porf_todo_memory[3218]=(u8)112;dong_porf_porf_todo_memory[3219]=(u8)114;dong_porf_porf_todo_memory[3220]=(u8)111;dong_porf_porf_todo_memory[3221]=(u8)116;dong_porf_porf_todo_memory[3222]=(u8)111;dong_porf_porf_todo_memory[3223]=(u8)116;dong_porf_porf_todo_memory[3224]=(u8)121;dong_porf_porf_todo_memory[3225]=(u8)112;dong_porf_porf_todo_memory[3226]=(u8)101;dong_porf_porf_todo_memory[3227]=(u8)46;dong_porf_porf_todo_memory[3228]=(u8)116;dong_porf_porf_todo_memory[3229]=(u8)114;dong_porf_porf_todo_memory[3230]=(u8)105;dong_porf_porf_todo_memory[3231]=(u8)109;dong_porf_porf_todo_memory[3232]=(u8)83;dong_porf_porf_todo_memory[3233]=(u8)116;dong_porf_porf_todo_memory[3234]=(u8)97;dong_porf_porf_todo_memory[3235]=(u8)114;dong_porf_porf_todo_memory[3236]=(u8)116;dong_porf_porf_todo_memory[3237]=(u8)32;dong_porf_porf_todo_memory[3238]=(u8)101;dong_porf_porf_todo_memory[3239]=(u8)120;dong_porf_porf_todo_memory[3240]=(u8)112;dong_porf_porf_todo_memory[3241]=(u8)101;dong_porf_porf_todo_memory[3242]=(u8)99;dong_porf_porf_todo_memory[3243]=(u8)116;dong_porf_porf_todo_memory[3244]=(u8)115;dong_porf_porf_todo_memory[3245]=(u8)32;dong_porf_porf_todo_memory[3246]=(u8)39;dong_porf_porf_todo_memory[3247]=(u8)116;dong_porf_porf_todo_memory[3248]=(u8)104;dong_porf_porf_todo_memory[3249]=(u8)105;dong_porf_porf_todo_memory[3250]=(u8)115;dong_porf_porf_todo_memory[3251]=(u8)39;dong_porf_porf_todo_memory[3252]=(u8)32;dong_porf_porf_todo_memory[3253]=(u8)116;dong_porf_porf_todo_memory[3254]=(u8)111;dong_porf_porf_todo_memory[3255]=(u8)32;dong_porf_porf_todo_memory[3256]=(u8)98;dong_porf_porf_todo_memory[3257]=(u8)101;dong_porf_porf_todo_memory[3258]=(u8)32;dong_porf_porf_todo_memory[3259]=(u8)110;dong_porf_porf_todo_memory[3260]=(u8)111;dong_porf_porf_todo_memory[3261]=(u8)110;dong_porf_porf_todo_memory[3262]=(u8)45;dong_porf_porf_todo_memory[3263]=(u8)110;dong_porf_porf_todo_memory[3264]=(u8)117;dong_porf_porf_todo_memory[3265]=(u8)108;dong_porf_porf_todo_memory[3266]=(u8)108;dong_porf_porf_todo_memory[3267]=(u8)105;dong_porf_porf_todo_memory[3268]=(u8)115;dong_porf_porf_todo_memory[3269]=(u8)104;
  dong_porf_porf_todo_memory[16501]=(u8)7;dong_porf_porf_todo_memory[16505]=(u8)118;dong_porf_porf_todo_memory[16506]=(u8)97;dong_porf_porf_todo_memory[16507]=(u8)108;dong_porf_porf_todo_memory[16508]=(u8)117;dong_porf_porf_todo_memory[16509]=(u8)101;dong_porf_porf_todo_memory[16510]=(u8)79;dong_porf_porf_todo_memory[16511]=(u8)102;dong_porf_porf_todo_memory[16555]=(u8)1;dong_porf_porf_todo_memory[16558]=(u8)14;dong_porf_porf_todo_memory[16562]=(u8)104;dong_porf_porf_todo_memory[16563]=(u8)97;dong_porf_porf_todo_memory[16564]=(u8)115;dong_porf_porf_todo_memory[16565]=(u8)79;dong_porf_porf_todo_memory[16566]=(u8)119;dong_porf_porf_todo_memory[16567]=(u8)110;dong_porf_porf_todo_memory[16568]=(u8)80;dong_porf_porf_todo_memory[16569]=(u8)114;dong_porf_porf_todo_memory[16570]=(u8)111;dong_porf_porf_todo_memory[16571]=(u8)112;dong_porf_porf_todo_memory[16572]=(u8)101;dong_porf_porf_todo_memory[16573]=(u8)114;dong_porf_porf_todo_memory[16574]=(u8)116;dong_porf_porf_todo_memory[16575]=(u8)121;dong_porf_porf_todo_memory[16612]=(u8)1;dong_porf_porf_todo_memory[16615]=(u8)20;dong_porf_porf_todo_memory[16619]=(u8)112;dong_porf_porf_todo_memory[16620]=(u8)114;dong_porf_porf_todo_memory[16621]=(u8)111;dong_porf_porf_todo_memory[16622]=(u8)112;dong_porf_porf_todo_memory[16623]=(u8)101;dong_porf_porf_todo_memory[16624]=(u8)114;dong_porf_porf_todo_memory[16625]=(u8)116;dong_porf_porf_todo_memory[16626]=(u8)121;dong_porf_porf_todo_memory[16627]=(u8)73;dong_porf_porf_todo_memory[16628]=(u8)115;dong_porf_porf_todo_memory[16629]=(u8)69;dong_porf_porf_todo_memory[16630]=(u8)110;dong_porf_porf_todo_memory[16631]=(u8)117;dong_porf_porf_todo_memory[16632]=(u8)109;dong_porf_porf_todo_memory[16633]=(u8)101;dong_porf_porf_todo_memory[16634]=(u8)114;dong_porf_porf_todo_memory[16635]=(u8)97;dong_porf_porf_todo_memory[16636]=(u8)98;dong_porf_porf_todo_memory[16637]=(u8)108;dong_porf_porf_todo_memory[16638]=(u8)101;dong_porf_porf_todo_memory[16669]=(u8)1;dong_porf_porf_todo_memory[16672]=(u8)13;dong_porf_porf_todo_memory[16676]=(u8)105;dong_porf_porf_todo_memory[16677]=(u8)115;dong_porf_porf_todo_memory[16678]=(u8)80;dong_porf_porf_todo_memory[16679]=(u8)114;dong_porf_porf_todo_memory[16680]=(u8)111;dong_porf_porf_todo_memory[16681]=(u8)116;dong_porf_porf_todo_memory[16682]=(u8)111;dong_porf_porf_todo_memory[16683]=(u8)116;dong_porf_porf_todo_memory[16684]=(u8)121;dong_porf_porf_todo_memory[16685]=(u8)112;dong_porf_porf_todo_memory[16686]=(u8)101;dong_porf_porf_todo_memory[16687]=(u8)79;dong_porf_porf_todo_memory[16688]=(u8)102;dong_porf_porf_todo_memory[16729]=(u8)8;dong_porf_porf_todo_memory[16733]=(u8)116;dong_porf_porf_todo_memory[16734]=(u8)111;dong_porf_porf_todo_memory[16735]=(u8)83;dong_porf_porf_todo_memory[16736]=(u8)116;dong_porf_porf_todo_memory[16737]=(u8)114;dong_porf_porf_todo_memory[16738]=(u8)105;dong_porf_porf_todo_memory[16739]=(u8)110;dong_porf_porf_todo_memory[16740]=(u8)103;dong_porf_porf_todo_memory[16786]=(u8)14;dong_porf_porf_todo_memory[16790]=(u8)116;dong_porf_porf_todo_memory[16791]=(u8)111;dong_porf_porf_todo_memory[16792]=(u8)76;dong_porf_porf_todo_memory[16793]=(u8)111;dong_porf_porf_todo_memory[16794]=(u8)99;dong_porf_porf_todo_memory[16795]=(u8)97;dong_porf_porf_todo_memory[16796]=(u8)108;dong_porf_porf_todo_memory[16797]=(u8)101;dong_porf_porf_todo_memory[16798]=(u8)83;dong_porf_porf_todo_memory[16799]=(u8)116;dong_porf_porf_todo_memory[16800]=(u8)114;dong_porf_porf_todo_memory[16801]=(u8)105;dong_porf_porf_todo_memory[16802]=(u8)110;dong_porf_porf_todo_memory[16803]=(u8)103;dong_porf_porf_todo_memory[16840]=(u8)1;dong_porf_porf_todo_memory[16842]=(u8)2;dong_porf_porf_todo_memory[16843]=(u8)6;dong_porf_porf_todo_memory[16847]=(u8)79;dong_porf_porf_todo_memory[16848]=(u8)98;dong_porf_porf_todo_memory[16849]=(u8)106;dong_porf_porf_todo_memory[16850]=(u8)101;dong_porf_porf_todo_memory[16851]=(u8)99;dong_porf_porf_todo_memory[16852]=(u8)116;
}

static struct ReturnValue dong_porf_porf_todo_utf8AppendCodePoint(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 out, i32 outjjtype, f64 cp, i32 cpjjtype);
static struct ReturnValue dong_porf_porf_todo_toUtf8(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype);
static struct ReturnValue dong_porf_porf_todo_pullHostString(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_persistStr(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype);
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
static struct ReturnValue dong_porf_porf_todo_numToStr(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 n, i32 njjtype);
static struct ReturnValue dong_porf_porf_todo_todoTextAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype);
static struct ReturnValue dong_porf_porf_todo_todoDoneAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype);
static struct ReturnValue dong_porf_porf_todo_setTodoDoneAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype, f64 v, i32 vjjtype);
static struct ReturnValue dong_porf_porf_todo_todoIdAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype);
static struct ReturnValue dong_porf_porf_todo_porfRebuild_todos(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_setTodoSlot(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype, f64 text, i32 textjjtype, f64 done, i32 donejjtype, f64 id, i32 idjjtype);
static struct ReturnValue dong_porf_porf_todo_porfPatchIf_clear_wrap(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_porfInit(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_normalizeInitialTodoText(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_countActive(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_countDone(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_shouldShow(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype);
static struct ReturnValue dong_porf_porf_todo_buildTodoRow(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype);
static struct ReturnValue dong_porf_porf_todo_parseIndexStr(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype);
static struct ReturnValue dong_porf_porf_todo_readToggleIndex(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_readDeleteIndex(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_porfPatchFilters(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_porfRefresh(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo_removeAtIndex(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 idx, i32 idxjjtype);
struct ReturnValue dong_porf_porf_todo_onAdd(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static f64 dong_porf_porf_todo_TypeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype);
static struct ReturnValue dong_porf_porf_todo__ecma262_ToString(f64 argument, i32 argumentjjtype);
static struct ReturnValue dong_porf_porf_todo__Number_prototype_toString(f64 _this, i32 _thisjjtype, f64 radix, i32 radixjjtype);
static f64 dong_porf_porf_todo__Math_trunc(f64 l0);
static f64 dong_porf_porf_todo_RangeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype);
static i32 dong_porf_porf_todo__Porffor_malloc(i32 l0);
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
static i32 dong_porf_porf_todo__Porffor_object_isObject(i32 arg, i32 argjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_get(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_getHiddenPrototype(i32 trueType, i32 trueTypejjtype);
static i32 dong_porf_porf_todo__Porffor_strcmp(i32 a, i32 ajjtype, i32 b, i32 bjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_getPrototype(i32 obj, i32 objjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_accessorGet(i32 entryPtr, i32 entryPtrjjtype);
static f64 dong_porf_porf_todo__ecma262_ToIntegerOrInfinity(f64 argument, i32 argumentjjtype);
static f64 dong_porf_porf_todo__ecma262_ToNumber(f64 argument, i32 argumentjjtype);
static f64 dong_porf_porf_todo__ecma262_StringToNumber(f64 str, i32 strjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_trim(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_trimEnd(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_trimStart(i32 _this, i32 _thisjjtype);
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
static struct ReturnValue dong_porf_porf_todo__ecma262_ToPrimitive_Number(f64 input, i32 inputjjtype);
static struct ReturnValue dong_porf_porf_todo__Number_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Boolean_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Array_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__ByteString_prototype_valueOf(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Object_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Porffor_object_readValue(i32 entryPtr, i32 entryPtrjjtype);
static i32 dong_porf_porf_todo__Porffor_object_isObjectOrNull(i32 arg, i32 argjjtype);
static struct ReturnValue dong_porf_porf_todo__Array_prototype_toString(f64 _this, i32 _thisjjtype);
static f64 dong_porf_porf_todo__Porffor_bytestring_appendChar(f64 str, i32 strjjtype, f64 _char, i32 charjjtype);
static f64 dong_porf_porf_todo__Porffor_compareStrings(f64 a, i32 ajjtype, f64 b, i32 bjjtype);
static void dong_porf_porf_todo__Porffor_object_setPrototype(i32 obj, i32 objjjtype, i32 proto, i32 protojjtype);
static struct ReturnValue dong_porf_porf_todo_String(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 args, i32 argsjjtype);
static struct ReturnValue dong_porf_porf_todo__Symbol_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__Symbol_prototype_descriptionkkget(f64 _this, i32 _thisjjtype);
static i32 dong_porf_porf_todo__Porffor_bytestringToString(i32 src);
static struct ReturnValue dong_porf_porf_todo__String_prototype_valueOf(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_charCodeAt(f64 _this, i32 _thisjjtype, f64 index, i32 indexjjtype);
static struct ReturnValue dong_porf_porf_todo__String_fromCharCode(f64 codes, i32 codesjjtype);
struct ReturnValue dong_porf_porf_todo_onInputChange(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_headlessAddSample(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onKeyDown(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onFilterAll(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onFilterActive(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onFilterDone(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onClearDone(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
struct ReturnValue dong_porf_porf_todo_onListClick(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_toString(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_trim(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_trimEnd(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_porf_todo__String_prototype_trimStart(i32 _this, i32 _thisjjtype);

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
static f64 dong_porf_porf_todo_porf_rootId = 0;
static i32 dong_porf_porf_todo_porf_rootIdjjtype = 0;
static f64 dong_porf_porf_todo_titleId = 0;
static i32 dong_porf_porf_todo_titleIdjjtype = 0;
static f64 dong_porf_porf_todo_todo_inputId = 0;
static i32 dong_porf_porf_todo_todo_inputIdjjtype = 0;
static f64 dong_porf_porf_todo_btn_addId = 0;
static i32 dong_porf_porf_todo_btn_addIdjjtype = 0;
static f64 dong_porf_porf_todo_filter_barId = 0;
static i32 dong_porf_porf_todo_filter_barIdjjtype = 0;
static f64 dong_porf_porf_todo_filter_allId = 0;
static i32 dong_porf_porf_todo_filter_allIdjjtype = 0;
static f64 dong_porf_porf_todo_filter_activeId = 0;
static i32 dong_porf_porf_todo_filter_activeIdjjtype = 0;
static f64 dong_porf_porf_todo_filter_doneId = 0;
static i32 dong_porf_porf_todo_filter_doneIdjjtype = 0;
static f64 dong_porf_porf_todo_todo_listId = 0;
static i32 dong_porf_porf_todo_todo_listIdjjtype = 0;
static f64 dong_porf_porf_todo_clear_wrapId = 0;
static i32 dong_porf_porf_todo_clear_wrapIdjjtype = 0;
static f64 dong_porf_porf_todo_btn_clearId = 0;
static i32 dong_porf_porf_todo_btn_clearIdjjtype = 0;
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
static f64 dong_porf_porf_todo_showClear = 0;
static i32 dong_porf_porf_todo_showClearjjtype = 0;
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
static f64 dong_porf_porf_todo_todoText16 = 0;
static i32 dong_porf_porf_todo_todoText16jjtype = 0;
static f64 dong_porf_porf_todo_todoDone16 = 0;
static i32 dong_porf_porf_todo_todoDone16jjtype = 0;
static f64 dong_porf_porf_todo_todoId16 = 0;
static i32 dong_porf_porf_todo_todoId16jjtype = 0;
static f64 dong_porf_porf_todo_todoText17 = 0;
static i32 dong_porf_porf_todo_todoText17jjtype = 0;
static f64 dong_porf_porf_todo_todoDone17 = 0;
static i32 dong_porf_porf_todo_todoDone17jjtype = 0;
static f64 dong_porf_porf_todo_todoId17 = 0;
static i32 dong_porf_porf_todo_todoId17jjtype = 0;
static f64 dong_porf_porf_todo_todoText18 = 0;
static i32 dong_porf_porf_todo_todoText18jjtype = 0;
static f64 dong_porf_porf_todo_todoDone18 = 0;
static i32 dong_porf_porf_todo_todoDone18jjtype = 0;
static f64 dong_porf_porf_todo_todoId18 = 0;
static i32 dong_porf_porf_todo_todoId18jjtype = 0;
static f64 dong_porf_porf_todo_todoText19 = 0;
static i32 dong_porf_porf_todo_todoText19jjtype = 0;
static f64 dong_porf_porf_todo_todoDone19 = 0;
static i32 dong_porf_porf_todo_todoDone19jjtype = 0;
static f64 dong_porf_porf_todo_todoId19 = 0;
static i32 dong_porf_porf_todo_todoId19jjtype = 0;
static f64 dong_porf_porf_todo_todoText20 = 0;
static i32 dong_porf_porf_todo_todoText20jjtype = 0;
static f64 dong_porf_porf_todo_todoDone20 = 0;
static i32 dong_porf_porf_todo_todoDone20jjtype = 0;
static f64 dong_porf_porf_todo_todoId20 = 0;
static i32 dong_porf_porf_todo_todoId20jjtype = 0;
static f64 dong_porf_porf_todo_todoText21 = 0;
static i32 dong_porf_porf_todo_todoText21jjtype = 0;
static f64 dong_porf_porf_todo_todoDone21 = 0;
static i32 dong_porf_porf_todo_todoDone21jjtype = 0;
static f64 dong_porf_porf_todo_todoId21 = 0;
static i32 dong_porf_porf_todo_todoId21jjtype = 0;
static f64 dong_porf_porf_todo_todoText22 = 0;
static i32 dong_porf_porf_todo_todoText22jjtype = 0;
static f64 dong_porf_porf_todo_todoDone22 = 0;
static i32 dong_porf_porf_todo_todoDone22jjtype = 0;
static f64 dong_porf_porf_todo_todoId22 = 0;
static i32 dong_porf_porf_todo_todoId22jjtype = 0;
static f64 dong_porf_porf_todo_todoText23 = 0;
static i32 dong_porf_porf_todo_todoText23jjtype = 0;
static f64 dong_porf_porf_todo_todoDone23 = 0;
static i32 dong_porf_porf_todo_todoDone23jjtype = 0;
static f64 dong_porf_porf_todo_todoId23 = 0;
static i32 dong_porf_porf_todo_todoId23jjtype = 0;
static f64 dong_porf_porf_todo_todoText24 = 0;
static i32 dong_porf_porf_todo_todoText24jjtype = 0;
static f64 dong_porf_porf_todo_todoDone24 = 0;
static i32 dong_porf_porf_todo_todoDone24jjtype = 0;
static f64 dong_porf_porf_todo_todoId24 = 0;
static i32 dong_porf_porf_todo_todoId24jjtype = 0;
static f64 dong_porf_porf_todo_todoText25 = 0;
static i32 dong_porf_porf_todo_todoText25jjtype = 0;
static f64 dong_porf_porf_todo_todoDone25 = 0;
static i32 dong_porf_porf_todo_todoDone25jjtype = 0;
static f64 dong_porf_porf_todo_todoId25 = 0;
static i32 dong_porf_porf_todo_todoId25jjtype = 0;
static f64 dong_porf_porf_todo_todoText26 = 0;
static i32 dong_porf_porf_todo_todoText26jjtype = 0;
static f64 dong_porf_porf_todo_todoDone26 = 0;
static i32 dong_porf_porf_todo_todoDone26jjtype = 0;
static f64 dong_porf_porf_todo_todoId26 = 0;
static i32 dong_porf_porf_todo_todoId26jjtype = 0;
static f64 dong_porf_porf_todo_todoText27 = 0;
static i32 dong_porf_porf_todo_todoText27jjtype = 0;
static f64 dong_porf_porf_todo_todoDone27 = 0;
static i32 dong_porf_porf_todo_todoDone27jjtype = 0;
static f64 dong_porf_porf_todo_todoId27 = 0;
static i32 dong_porf_porf_todo_todoId27jjtype = 0;
static f64 dong_porf_porf_todo_todoText28 = 0;
static i32 dong_porf_porf_todo_todoText28jjtype = 0;
static f64 dong_porf_porf_todo_todoDone28 = 0;
static i32 dong_porf_porf_todo_todoDone28jjtype = 0;
static f64 dong_porf_porf_todo_todoId28 = 0;
static i32 dong_porf_porf_todo_todoId28jjtype = 0;
static f64 dong_porf_porf_todo_todoText29 = 0;
static i32 dong_porf_porf_todo_todoText29jjtype = 0;
static f64 dong_porf_porf_todo_todoDone29 = 0;
static i32 dong_porf_porf_todo_todoDone29jjtype = 0;
static f64 dong_porf_porf_todo_todoId29 = 0;
static i32 dong_porf_porf_todo_todoId29jjtype = 0;
static f64 dong_porf_porf_todo_todoText30 = 0;
static i32 dong_porf_porf_todo_todoText30jjtype = 0;
static f64 dong_porf_porf_todo_todoDone30 = 0;
static i32 dong_porf_porf_todo_todoDone30jjtype = 0;
static f64 dong_porf_porf_todo_todoId30 = 0;
static i32 dong_porf_porf_todo_todoId30jjtype = 0;
static f64 dong_porf_porf_todo_todoText31 = 0;
static i32 dong_porf_porf_todo_todoText31jjtype = 0;
static f64 dong_porf_porf_todo_todoDone31 = 0;
static i32 dong_porf_porf_todo_todoDone31jjtype = 0;
static f64 dong_porf_porf_todo_todoId31 = 0;
static i32 dong_porf_porf_todo_todoId31jjtype = 0;

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
      _r15 = dong_porf_porf_todo_jjporfjjcurrentPtr - _get5;
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
        if (_get10 == INFINITY) {
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
  dong_porf_porf_todo__Porffor_object_fastAdd(_get2, 7, 651, 195, 3, 6, 10, 1);
  _get3 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get3, 7, 1273, 195, 4, 6, 10, 1);
  _get4 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get4, 7, 1299, 195, 5, 6, 10, 1);
  _get5 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get5, 7, 550, 195, 6, 6, 10, 1);
  _get6 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get6, 7, 1356, 195, 7, 6, 10, 1);
  _get7 = l0;
  dong_porf_porf_todo__Porffor_object_fastAdd(_get7, 7, 1129, 195, 2, 6, 10, 1);
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
              _r135 = _get9 != 6;
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
      return (struct ReturnValue){ 1505, 195 };
      (void) 0;
    }
  // end
  j141:;
  _get34 = _this;
  _get35 = _thisjjtype;
  // if 
    if (((_get34 == 0) & ((_get35 | 128) == (7 | 128))) != 0) {
      return (struct ReturnValue){ 1529, 195 };
      (void) 0;
    }
  // end
  j142:;
  _get36 = _thisjjtype;
  // if 
    if ((f64)(_get36) == 72) {
      return (struct ReturnValue){ 1548, 195 };
      (void) 0;
    }
  // end
  j143:;
  _get37 = _thisjjtype;
  // if 
    if ((f64)(_get37) == 6) {
      return (struct ReturnValue){ 1568, 195 };
      (void) 0;
    }
  // end
  j144:;
  _get38 = _thisjjtype;
  _get39 = _thisjjtype;
  // if 
    if ((((f64)(_get38) == 2) | ((f64)(_get39) == 31)) != 0) {
      return (struct ReturnValue){ 1591, 195 };
      (void) 0;
    }
  // end
  j145:;
  _get40 = _thisjjtype;
  _get41 = _thisjjtype;
  // if 
    if ((((f64)(_get40) == 1) | ((f64)(_get41) == 32)) != 0) {
      return (struct ReturnValue){ 1613, 195 };
      (void) 0;
    }
  // end
  j146:;
  _get42 = _thisjjtype;
  _get43 = _thisjjtype;
  // if 
    if ((((f64)(_get42 | 128) == 195) | ((f64)(_get43) == 33)) != 0) {
      return (struct ReturnValue){ 1634, 195 };
      (void) 0;
    }
  // end
  j147:;
  _get44 = _thisjjtype;
  // if 
    if ((f64)(_get44) == 10) {
      return (struct ReturnValue){ 1655, 195 };
      (void) 0;
    }
  // end
  j148:;
  _get45 = _thisjjtype;
  // if 
    if ((f64)(_get45) == 9) {
      return (struct ReturnValue){ 1674, 195 };
      (void) 0;
    }
  // end
  j149:;
  return (struct ReturnValue){ 1695, 195 };
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
      return (struct ReturnValue){ NAN, 1 };
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
      return (struct ReturnValue){ NAN, 1 };
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
      return NAN;
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
                        return NAN;
                      }
                    // end
                    j319:;
                  }
                // end
                j317:;
              } else {
                return NAN;
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
      return NAN;
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
                return NAN;
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
      return NAN;
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
        if (_get1 == (-INFINITY)) {
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
      return NAN;
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
          return -INFINITY;
        }
      // end
      j454:;
      return NAN;
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
      return NAN;
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
        if (_get10 == INFINITY) {
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
              return -INFINITY;
            }
          // end
          j425:;
          return INFINITY;
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
        if ((1 / _get22) == INFINITY) {
          _get23 = exponent;
          // if 
            if (_get23 > 0) {
              return 0;
            }
          // end
          j429:;
          return INFINITY;
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
          return -INFINITY;
        }
      // end
      j432:;
      return INFINITY;
    }
  // end
  j427:;
  _get32 = exponent;
  // if 
    if (_get32 == INFINITY) {
      _get33 = base;
      abs = dong_porf_porf_todo__Math_abs(_get33);
      absjjtype = 1;
      _get34 = abs;
      // if 
        if (_get34 > 1) {
          return INFINITY;
        }
      // end
      j434:;
      _get35 = abs;
      // if 
        if (_get35 == 1) {
          return NAN;
        }
      // end
      j435:;
      return 0;
    }
  // end
  j433:;
  _get36 = exponent;
  // if 
    if (_get36 == (-INFINITY)) {
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
          return NAN;
        }
      // end
      j438:;
      return INFINITY;
    }
  // end
  j436:;
  _get40 = base;
  // if 
    if (_get40 < 0) {
      _get41 = exponent;
      // if 
        if (dong_porf_porf_todo__Number_isInteger(_get41) == 0) {
          return NAN;
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
      return NAN;
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
                    return NAN;
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
                        return NAN;
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
                    return NAN;
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
          n = INFINITY;
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
      return NAN;
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
          jjmember_prop_257 = 1129;
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
              _r481 = _get9 != 2;
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
          entryPtr = (f64)(dong_porf_porf_todo__Porffor_object_lookup((i32)(_get19), 7, 1129, 195, dong_porf_porf_todo__Porffor_object_hash(1129, 195), 1));
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
      return NAN;
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
        f64_store(0, 4, 65536, _get3);
        _get4 = cpjjtype;
        i32_store8(0, 12, 65536, _get4);
        i32_store(1, 0, 65536, 1);
        const struct ReturnValue _0 = dong_porf_porf_todo__String_fromCharCode(65536, 72);
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
        f64_store(0, 4, 81920, (f64)(_r597 | _r600));
        i32_store8(0, 12, 81920, 1);
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
        f64_store(0, 13, 81920, (f64)(_r601 | _r604));
        i32_store8(0, 21, 81920, 1);
        i32_store(1, 0, 81920, 2);
        const struct ReturnValue _2 = dong_porf_porf_todo__String_fromCharCode(81920, 72);
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
        f64_store(0, 4, 98304, (f64)(_r610 | _r613));
        i32_store8(0, 12, 98304, 1);
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
        f64_store(0, 13, 98304, (f64)(_r614 | _r619));
        i32_store8(0, 21, 98304, 1);
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
        f64_store(0, 22, 98304, (f64)(_r620 | _r623));
        i32_store8(0, 30, 98304, 1);
        i32_store(1, 0, 98304, 3);
        const struct ReturnValue _4 = dong_porf_porf_todo__String_fromCharCode(98304, 72);
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
    f64_store(0, 4, 114688, (f64)(_r628 | _r631));
    i32_store8(0, 12, 114688, 1);
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
    f64_store(0, 13, 114688, (f64)(_r632 | _r637));
    i32_store8(0, 21, 114688, 1);
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
    f64_store(0, 22, 114688, (f64)(_r638 | _r643));
    i32_store8(0, 30, 114688, 1);
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
    f64_store(0, 31, 114688, (f64)(_r644 | _r647));
    i32_store8(0, 39, 114688, 1);
    i32_store(1, 0, 114688, 4);
    const struct ReturnValue _6 = dong_porf_porf_todo__String_fromCharCode(114688, 72);
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
  f64 jjmember_obj_146 = 0;
  f64 jjmember_prop_146 = 0;
  f64 cp = 0;
  i32 cpjjtype = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  f64 jjindirect_147_callee = 0;
  f64 jjindirect_147_caller = 0;
  i32 jjindirect_147_callerjjtype = 0;
  f64 jjmember_obj_148 = 0;
  f64 jjmember_prop_148 = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 logictmpi = 0;
  i32 jjlogicinner_tmp_int = 0;
  f64 next = 0;
  i32 nextjjtype = 0;
  f64 jjindirect_149_callee = 0;
  f64 jjindirect_149_caller = 0;
  i32 jjindirect_149_callerjjtype = 0;
  f64 jjmember_obj_150 = 0;
  f64 jjmember_prop_150 = 0;
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
      jjmember_prop_146 = 564;
      _get3 = s;
      jjmember_obj_146 = _get3;
      _get4 = sjjtype;
      // if f64
      f64 _r1;
        if (_get4 == 0) {
          _r1 = 0;
        } else {
          _get5 = jjmember_obj_146;
          _get6 = sjjtype;
          _get7 = jjmember_prop_146;
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
          jjmember_prop_148 = 1818;
          _get29 = s;
          jjindirect_147_caller = _get29;
          _get30 = jjindirect_147_caller;
          _get31 = sjjtype;
          jjindirect_147_callerjjtype = _get31;
          jjmember_obj_148 = _get30;
          _get32 = jjindirect_147_callerjjtype;
          // if f64
          f64 _r566;
            if (_get32 == 0) {
              _r566 = 0;
            } else {
              _get33 = jjmember_obj_148;
              _get34 = jjindirect_147_callerjjtype;
              _get35 = jjmember_prop_148;
              const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get33), _get34, (u32)(_get35), 195, -1592872053, 1);
              jjlast_type = _4.type;
              _r566 = _4.value;
            }
          // end
          j566:;
          jjindirect_147_callee = _r566;
          _get36 = jjlast_type;
          // if f64
          f64 _r567;
            if (_get36 == 6) {
              _get37 = jjindirect_147_caller;
              _get38 = jjindirect_147_callerjjtype;
              _get39 = i;
              _get40 = ijjtype;
              _get41 = jjindirect_147_callee;
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
              jjmember_prop_150 = 1818;
              _get74 = s;
              jjindirect_149_caller = _get74;
              _get75 = jjindirect_149_caller;
              _get76 = sjjtype;
              jjindirect_149_callerjjtype = _get76;
              jjmember_obj_150 = _get75;
              _get77 = jjindirect_149_callerjjtype;
              // if f64
              f64 _r578;
                if (_get77 == 0) {
                  _r578 = 0;
                } else {
                  _get78 = jjmember_obj_150;
                  _get79 = jjindirect_149_callerjjtype;
                  _get80 = jjmember_prop_150;
                  const struct ReturnValue _8 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get78), _get79, (u32)(_get80), 195, -1592872053, 1);
                  jjlast_type = _8.type;
                  _r578 = _8.value;
                }
              // end
              j578:;
              jjindirect_149_callee = _r578;
              _get81 = jjlast_type;
              // if f64
              f64 _r579;
                if (_get81 == 6) {
                  _get82 = jjindirect_149_caller;
                  _get83 = jjindirect_149_callerjjtype;
                  _get84 = i;
                  _get85 = jjindirect_149_callee;
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

static struct ReturnValue dong_porf_porf_todo_persistStr(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype) {
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

  _get0 = s;
  _get1 = sjjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_toUtf8(0, 0, 0, 0, _get0, _get1);
  jjlast_type = _0.type;
  jjreturn = _0.value;
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

static struct ReturnValue dong_porf_porf_todo_normalizeInitialTodoText(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get6;
  f64 _get5;
  f64 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 jjlast_type = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_persistStr(0, 0, 0, 0, dong_porf_porf_todo_todoText0, dong_porf_porf_todo_todoText0jjtype);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  dong_porf_porf_todo_todoText0jjtype = _get0;
  dong_porf_porf_todo_todoText0 = _0.value;
  const struct ReturnValue _1 = dong_porf_porf_todo_persistStr(0, 0, 0, 0, dong_porf_porf_todo_todoText1, dong_porf_porf_todo_todoText1jjtype);
  jjlast_type = _1.type;
  _get1 = jjlast_type;
  dong_porf_porf_todo_todoText1jjtype = _get1;
  dong_porf_porf_todo_todoText1 = _1.value;
  const struct ReturnValue _2 = dong_porf_porf_todo_persistStr(0, 0, 0, 0, dong_porf_porf_todo_todoText2, dong_porf_porf_todo_todoText2jjtype);
  jjlast_type = _2.type;
  _get2 = jjlast_type;
  dong_porf_porf_todo_todoText2jjtype = _get2;
  dong_porf_porf_todo_todoText2 = _2.value;
  const struct ReturnValue _3 = dong_porf_porf_todo_persistStr(0, 0, 0, 0, dong_porf_porf_todo_todoText3, dong_porf_porf_todo_todoText3jjtype);
  jjlast_type = _3.type;
  _get3 = jjlast_type;
  dong_porf_porf_todo_todoText3jjtype = _get3;
  dong_porf_porf_todo_todoText3 = _3.value;
  _get4 = jjnewtarget;
  // if 
    if (((u32)(_get4)) != 0) {
      _get5 = jjthis;
      _get6 = jjthisjjtype;
      return (struct ReturnValue){ _get5, _get6 };
    }
  // end
  j655:;
  return (struct ReturnValue){ 0, 0 };
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
      j657:;
    }
  // end
  j656:;
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
  j658:;
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
  j663:;
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
  j664:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_porfPatchIf_clear_wrap(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  f64 _get2;
  i32 _get1;
  i32 _get0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 jjlast_type = 0;

  jjlogicinner_tmp = dong_porf_porf_todo_showClear;
  jjtypeswitch_tmp1 = dong_porf_porf_todo_showClearjjtype;
  // block i32
  i32 _r659;
    _get0 = jjtypeswitch_tmp1;
    _get1 = jjtypeswitch_tmp1;
    // if 
      if (((_get0 == 67) | (_get1 == 195)) != 0) {
        _get2 = jjlogicinner_tmp;
        _r659 = i32_load(1, 0, (u32)(_get2));
        goto j659;
      }
    // end
    j660:;
    _get3 = jjtypeswitch_tmp1;
    _get4 = jjtypeswitch_tmp1;
    // if 
      if (((_get3 == 31) | (_get4 == 32)) != 0) {
        _r659 = 1;
        goto j659;
      }
    // end
    j661:;
    _get5 = jjlogicinner_tmp;
    const f64 _tmp0 = _get5;
    _r659 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
  // end
  j659:;
  // if 
    if ((_r659) != 0) {
      const struct ReturnValue _0 = dong_porf_porf_todo_removeAttribute(0, 0, 0, 0, dong_porf_porf_todo_clear_wrapId, 1, 2099, 195);
      (void) _0.type;
      (void) _0.value;
    } else {
      const struct ReturnValue _1 = dong_porf_porf_todo_setAttribute(0, 0, 0, 0, dong_porf_porf_todo_clear_wrapId, 1, 2099, 195, 2111, 195);
      (void) _1.type;
      (void) _1.value;
    }
  // end
  j662:;
  _get6 = jjnewtarget;
  // if 
    if (((u32)(_get6)) != 0) {
      _get7 = jjthis;
      _get8 = jjthisjjtype;
      return (struct ReturnValue){ _get7, _get8 };
    }
  // end
  j665:;
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
  j669:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_todoDoneAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype) {
  i32 _get294;
  f64 _get293;
  i32 _get292;
  f64 _get291;
  i32 _get290;
  f64 _get289;
  f64 _get288;
  i32 _get287;
  f64 _get286;
  i32 _get285;
  f64 _get284;
  i32 _get283;
  f64 _get282;
  f64 _get281;
  i32 _get280;
  f64 _get279;
  i32 _get278;
  f64 _get277;
  i32 _get276;
  f64 _get275;
  i32 _get274;
  f64 _get273;
  f64 _get272;
  i32 _get271;
  f64 _get270;
  i32 _get269;
  f64 _get268;
  i32 _get267;
  f64 _get266;
  i32 _get265;
  f64 _get264;
  f64 _get263;
  i32 _get262;
  f64 _get261;
  i32 _get260;
  f64 _get259;
  i32 _get258;
  f64 _get257;
  i32 _get256;
  f64 _get255;
  f64 _get254;
  i32 _get253;
  f64 _get252;
  i32 _get251;
  f64 _get250;
  i32 _get249;
  f64 _get248;
  i32 _get247;
  f64 _get246;
  f64 _get245;
  i32 _get244;
  f64 _get243;
  i32 _get242;
  f64 _get241;
  i32 _get240;
  f64 _get239;
  i32 _get238;
  f64 _get237;
  f64 _get236;
  i32 _get235;
  f64 _get234;
  i32 _get233;
  f64 _get232;
  i32 _get231;
  f64 _get230;
  i32 _get229;
  f64 _get228;
  f64 _get227;
  i32 _get226;
  f64 _get225;
  i32 _get224;
  f64 _get223;
  i32 _get222;
  f64 _get221;
  i32 _get220;
  f64 _get219;
  f64 _get218;
  i32 _get217;
  f64 _get216;
  i32 _get215;
  f64 _get214;
  i32 _get213;
  f64 _get212;
  i32 _get211;
  f64 _get210;
  f64 _get209;
  i32 _get208;
  f64 _get207;
  i32 _get206;
  f64 _get205;
  i32 _get204;
  f64 _get203;
  i32 _get202;
  f64 _get201;
  f64 _get200;
  i32 _get199;
  f64 _get198;
  i32 _get197;
  f64 _get196;
  i32 _get195;
  f64 _get194;
  i32 _get193;
  f64 _get192;
  f64 _get191;
  i32 _get190;
  f64 _get189;
  i32 _get188;
  f64 _get187;
  i32 _get186;
  f64 _get185;
  i32 _get184;
  f64 _get183;
  f64 _get182;
  i32 _get181;
  f64 _get180;
  i32 _get179;
  f64 _get178;
  i32 _get177;
  f64 _get176;
  i32 _get175;
  f64 _get174;
  f64 _get173;
  i32 _get172;
  f64 _get171;
  i32 _get170;
  f64 _get169;
  i32 _get168;
  f64 _get167;
  i32 _get166;
  f64 _get165;
  f64 _get164;
  i32 _get163;
  f64 _get162;
  i32 _get161;
  f64 _get160;
  i32 _get159;
  f64 _get158;
  i32 _get157;
  f64 _get156;
  f64 _get155;
  i32 _get154;
  f64 _get153;
  i32 _get152;
  f64 _get151;
  i32 _get150;
  f64 _get149;
  i32 _get148;
  f64 _get147;
  f64 _get146;
  i32 _get145;
  f64 _get144;
  i32 _get143;
  f64 _get142;
  i32 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  f64 _get137;
  i32 _get136;
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
          j672:;
        }
      // end
      j671:;
      _get7 = jjreturn;
      _get8 = jjreturnjjtype;
      return (struct ReturnValue){ _get7, _get8 };
    }
  // end
  j670:;
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
          j675:;
        }
      // end
      j674:;
      _get16 = jjreturn;
      _get17 = jjreturnjjtype;
      return (struct ReturnValue){ _get16, _get17 };
    }
  // end
  j673:;
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
          j678:;
        }
      // end
      j677:;
      _get25 = jjreturn;
      _get26 = jjreturnjjtype;
      return (struct ReturnValue){ _get25, _get26 };
    }
  // end
  j676:;
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
          j681:;
        }
      // end
      j680:;
      _get34 = jjreturn;
      _get35 = jjreturnjjtype;
      return (struct ReturnValue){ _get34, _get35 };
    }
  // end
  j679:;
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
          j684:;
        }
      // end
      j683:;
      _get43 = jjreturn;
      _get44 = jjreturnjjtype;
      return (struct ReturnValue){ _get43, _get44 };
    }
  // end
  j682:;
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
          j687:;
        }
      // end
      j686:;
      _get52 = jjreturn;
      _get53 = jjreturnjjtype;
      return (struct ReturnValue){ _get52, _get53 };
    }
  // end
  j685:;
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
          j690:;
        }
      // end
      j689:;
      _get61 = jjreturn;
      _get62 = jjreturnjjtype;
      return (struct ReturnValue){ _get61, _get62 };
    }
  // end
  j688:;
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
          j693:;
        }
      // end
      j692:;
      _get70 = jjreturn;
      _get71 = jjreturnjjtype;
      return (struct ReturnValue){ _get70, _get71 };
    }
  // end
  j691:;
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
          j696:;
        }
      // end
      j695:;
      _get79 = jjreturn;
      _get80 = jjreturnjjtype;
      return (struct ReturnValue){ _get79, _get80 };
    }
  // end
  j694:;
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
          j699:;
        }
      // end
      j698:;
      _get88 = jjreturn;
      _get89 = jjreturnjjtype;
      return (struct ReturnValue){ _get88, _get89 };
    }
  // end
  j697:;
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
          j702:;
        }
      // end
      j701:;
      _get97 = jjreturn;
      _get98 = jjreturnjjtype;
      return (struct ReturnValue){ _get97, _get98 };
    }
  // end
  j700:;
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
          j705:;
        }
      // end
      j704:;
      _get106 = jjreturn;
      _get107 = jjreturnjjtype;
      return (struct ReturnValue){ _get106, _get107 };
    }
  // end
  j703:;
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
          j708:;
        }
      // end
      j707:;
      _get115 = jjreturn;
      _get116 = jjreturnjjtype;
      return (struct ReturnValue){ _get115, _get116 };
    }
  // end
  j706:;
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
          j711:;
        }
      // end
      j710:;
      _get124 = jjreturn;
      _get125 = jjreturnjjtype;
      return (struct ReturnValue){ _get124, _get125 };
    }
  // end
  j709:;
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
          j714:;
        }
      // end
      j713:;
      _get133 = jjreturn;
      _get134 = jjreturnjjtype;
      return (struct ReturnValue){ _get133, _get134 };
    }
  // end
  j712:;
  _get135 = i;
  _get136 = ijjtype;
  // if 
    if ((f64)((_get135 == 15) & ((_get136 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone15;
      jjreturnjjtype = dong_porf_porf_todo_todoDone15jjtype;
      _get137 = jjnewtarget;
      // if 
        if (((u32)(_get137)) != 0) {
          _get138 = jjreturn;
          _get139 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get138), _get139)) == 0) {
              _get140 = jjthis;
              _get141 = jjthisjjtype;
              return (struct ReturnValue){ _get140, _get141 };
            }
          // end
          j717:;
        }
      // end
      j716:;
      _get142 = jjreturn;
      _get143 = jjreturnjjtype;
      return (struct ReturnValue){ _get142, _get143 };
    }
  // end
  j715:;
  _get144 = i;
  _get145 = ijjtype;
  // if 
    if ((f64)((_get144 == 16) & ((_get145 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone16;
      jjreturnjjtype = dong_porf_porf_todo_todoDone16jjtype;
      _get146 = jjnewtarget;
      // if 
        if (((u32)(_get146)) != 0) {
          _get147 = jjreturn;
          _get148 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get147), _get148)) == 0) {
              _get149 = jjthis;
              _get150 = jjthisjjtype;
              return (struct ReturnValue){ _get149, _get150 };
            }
          // end
          j720:;
        }
      // end
      j719:;
      _get151 = jjreturn;
      _get152 = jjreturnjjtype;
      return (struct ReturnValue){ _get151, _get152 };
    }
  // end
  j718:;
  _get153 = i;
  _get154 = ijjtype;
  // if 
    if ((f64)((_get153 == 17) & ((_get154 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone17;
      jjreturnjjtype = dong_porf_porf_todo_todoDone17jjtype;
      _get155 = jjnewtarget;
      // if 
        if (((u32)(_get155)) != 0) {
          _get156 = jjreturn;
          _get157 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get156), _get157)) == 0) {
              _get158 = jjthis;
              _get159 = jjthisjjtype;
              return (struct ReturnValue){ _get158, _get159 };
            }
          // end
          j723:;
        }
      // end
      j722:;
      _get160 = jjreturn;
      _get161 = jjreturnjjtype;
      return (struct ReturnValue){ _get160, _get161 };
    }
  // end
  j721:;
  _get162 = i;
  _get163 = ijjtype;
  // if 
    if ((f64)((_get162 == 18) & ((_get163 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone18;
      jjreturnjjtype = dong_porf_porf_todo_todoDone18jjtype;
      _get164 = jjnewtarget;
      // if 
        if (((u32)(_get164)) != 0) {
          _get165 = jjreturn;
          _get166 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get165), _get166)) == 0) {
              _get167 = jjthis;
              _get168 = jjthisjjtype;
              return (struct ReturnValue){ _get167, _get168 };
            }
          // end
          j726:;
        }
      // end
      j725:;
      _get169 = jjreturn;
      _get170 = jjreturnjjtype;
      return (struct ReturnValue){ _get169, _get170 };
    }
  // end
  j724:;
  _get171 = i;
  _get172 = ijjtype;
  // if 
    if ((f64)((_get171 == 19) & ((_get172 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone19;
      jjreturnjjtype = dong_porf_porf_todo_todoDone19jjtype;
      _get173 = jjnewtarget;
      // if 
        if (((u32)(_get173)) != 0) {
          _get174 = jjreturn;
          _get175 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get174), _get175)) == 0) {
              _get176 = jjthis;
              _get177 = jjthisjjtype;
              return (struct ReturnValue){ _get176, _get177 };
            }
          // end
          j729:;
        }
      // end
      j728:;
      _get178 = jjreturn;
      _get179 = jjreturnjjtype;
      return (struct ReturnValue){ _get178, _get179 };
    }
  // end
  j727:;
  _get180 = i;
  _get181 = ijjtype;
  // if 
    if ((f64)((_get180 == 20) & ((_get181 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone20;
      jjreturnjjtype = dong_porf_porf_todo_todoDone20jjtype;
      _get182 = jjnewtarget;
      // if 
        if (((u32)(_get182)) != 0) {
          _get183 = jjreturn;
          _get184 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get183), _get184)) == 0) {
              _get185 = jjthis;
              _get186 = jjthisjjtype;
              return (struct ReturnValue){ _get185, _get186 };
            }
          // end
          j732:;
        }
      // end
      j731:;
      _get187 = jjreturn;
      _get188 = jjreturnjjtype;
      return (struct ReturnValue){ _get187, _get188 };
    }
  // end
  j730:;
  _get189 = i;
  _get190 = ijjtype;
  // if 
    if ((f64)((_get189 == 21) & ((_get190 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone21;
      jjreturnjjtype = dong_porf_porf_todo_todoDone21jjtype;
      _get191 = jjnewtarget;
      // if 
        if (((u32)(_get191)) != 0) {
          _get192 = jjreturn;
          _get193 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get192), _get193)) == 0) {
              _get194 = jjthis;
              _get195 = jjthisjjtype;
              return (struct ReturnValue){ _get194, _get195 };
            }
          // end
          j735:;
        }
      // end
      j734:;
      _get196 = jjreturn;
      _get197 = jjreturnjjtype;
      return (struct ReturnValue){ _get196, _get197 };
    }
  // end
  j733:;
  _get198 = i;
  _get199 = ijjtype;
  // if 
    if ((f64)((_get198 == 22) & ((_get199 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone22;
      jjreturnjjtype = dong_porf_porf_todo_todoDone22jjtype;
      _get200 = jjnewtarget;
      // if 
        if (((u32)(_get200)) != 0) {
          _get201 = jjreturn;
          _get202 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get201), _get202)) == 0) {
              _get203 = jjthis;
              _get204 = jjthisjjtype;
              return (struct ReturnValue){ _get203, _get204 };
            }
          // end
          j738:;
        }
      // end
      j737:;
      _get205 = jjreturn;
      _get206 = jjreturnjjtype;
      return (struct ReturnValue){ _get205, _get206 };
    }
  // end
  j736:;
  _get207 = i;
  _get208 = ijjtype;
  // if 
    if ((f64)((_get207 == 23) & ((_get208 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone23;
      jjreturnjjtype = dong_porf_porf_todo_todoDone23jjtype;
      _get209 = jjnewtarget;
      // if 
        if (((u32)(_get209)) != 0) {
          _get210 = jjreturn;
          _get211 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get210), _get211)) == 0) {
              _get212 = jjthis;
              _get213 = jjthisjjtype;
              return (struct ReturnValue){ _get212, _get213 };
            }
          // end
          j741:;
        }
      // end
      j740:;
      _get214 = jjreturn;
      _get215 = jjreturnjjtype;
      return (struct ReturnValue){ _get214, _get215 };
    }
  // end
  j739:;
  _get216 = i;
  _get217 = ijjtype;
  // if 
    if ((f64)((_get216 == 24) & ((_get217 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone24;
      jjreturnjjtype = dong_porf_porf_todo_todoDone24jjtype;
      _get218 = jjnewtarget;
      // if 
        if (((u32)(_get218)) != 0) {
          _get219 = jjreturn;
          _get220 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get219), _get220)) == 0) {
              _get221 = jjthis;
              _get222 = jjthisjjtype;
              return (struct ReturnValue){ _get221, _get222 };
            }
          // end
          j744:;
        }
      // end
      j743:;
      _get223 = jjreturn;
      _get224 = jjreturnjjtype;
      return (struct ReturnValue){ _get223, _get224 };
    }
  // end
  j742:;
  _get225 = i;
  _get226 = ijjtype;
  // if 
    if ((f64)((_get225 == 25) & ((_get226 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone25;
      jjreturnjjtype = dong_porf_porf_todo_todoDone25jjtype;
      _get227 = jjnewtarget;
      // if 
        if (((u32)(_get227)) != 0) {
          _get228 = jjreturn;
          _get229 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get228), _get229)) == 0) {
              _get230 = jjthis;
              _get231 = jjthisjjtype;
              return (struct ReturnValue){ _get230, _get231 };
            }
          // end
          j747:;
        }
      // end
      j746:;
      _get232 = jjreturn;
      _get233 = jjreturnjjtype;
      return (struct ReturnValue){ _get232, _get233 };
    }
  // end
  j745:;
  _get234 = i;
  _get235 = ijjtype;
  // if 
    if ((f64)((_get234 == 26) & ((_get235 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone26;
      jjreturnjjtype = dong_porf_porf_todo_todoDone26jjtype;
      _get236 = jjnewtarget;
      // if 
        if (((u32)(_get236)) != 0) {
          _get237 = jjreturn;
          _get238 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get237), _get238)) == 0) {
              _get239 = jjthis;
              _get240 = jjthisjjtype;
              return (struct ReturnValue){ _get239, _get240 };
            }
          // end
          j750:;
        }
      // end
      j749:;
      _get241 = jjreturn;
      _get242 = jjreturnjjtype;
      return (struct ReturnValue){ _get241, _get242 };
    }
  // end
  j748:;
  _get243 = i;
  _get244 = ijjtype;
  // if 
    if ((f64)((_get243 == 27) & ((_get244 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone27;
      jjreturnjjtype = dong_porf_porf_todo_todoDone27jjtype;
      _get245 = jjnewtarget;
      // if 
        if (((u32)(_get245)) != 0) {
          _get246 = jjreturn;
          _get247 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get246), _get247)) == 0) {
              _get248 = jjthis;
              _get249 = jjthisjjtype;
              return (struct ReturnValue){ _get248, _get249 };
            }
          // end
          j753:;
        }
      // end
      j752:;
      _get250 = jjreturn;
      _get251 = jjreturnjjtype;
      return (struct ReturnValue){ _get250, _get251 };
    }
  // end
  j751:;
  _get252 = i;
  _get253 = ijjtype;
  // if 
    if ((f64)((_get252 == 28) & ((_get253 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone28;
      jjreturnjjtype = dong_porf_porf_todo_todoDone28jjtype;
      _get254 = jjnewtarget;
      // if 
        if (((u32)(_get254)) != 0) {
          _get255 = jjreturn;
          _get256 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get255), _get256)) == 0) {
              _get257 = jjthis;
              _get258 = jjthisjjtype;
              return (struct ReturnValue){ _get257, _get258 };
            }
          // end
          j756:;
        }
      // end
      j755:;
      _get259 = jjreturn;
      _get260 = jjreturnjjtype;
      return (struct ReturnValue){ _get259, _get260 };
    }
  // end
  j754:;
  _get261 = i;
  _get262 = ijjtype;
  // if 
    if ((f64)((_get261 == 29) & ((_get262 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone29;
      jjreturnjjtype = dong_porf_porf_todo_todoDone29jjtype;
      _get263 = jjnewtarget;
      // if 
        if (((u32)(_get263)) != 0) {
          _get264 = jjreturn;
          _get265 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get264), _get265)) == 0) {
              _get266 = jjthis;
              _get267 = jjthisjjtype;
              return (struct ReturnValue){ _get266, _get267 };
            }
          // end
          j759:;
        }
      // end
      j758:;
      _get268 = jjreturn;
      _get269 = jjreturnjjtype;
      return (struct ReturnValue){ _get268, _get269 };
    }
  // end
  j757:;
  _get270 = i;
  _get271 = ijjtype;
  // if 
    if ((f64)((_get270 == 30) & ((_get271 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone30;
      jjreturnjjtype = dong_porf_porf_todo_todoDone30jjtype;
      _get272 = jjnewtarget;
      // if 
        if (((u32)(_get272)) != 0) {
          _get273 = jjreturn;
          _get274 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get273), _get274)) == 0) {
              _get275 = jjthis;
              _get276 = jjthisjjtype;
              return (struct ReturnValue){ _get275, _get276 };
            }
          // end
          j762:;
        }
      // end
      j761:;
      _get277 = jjreturn;
      _get278 = jjreturnjjtype;
      return (struct ReturnValue){ _get277, _get278 };
    }
  // end
  j760:;
  _get279 = i;
  _get280 = ijjtype;
  // if 
    if ((f64)((_get279 == 31) & ((_get280 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoDone31;
      jjreturnjjtype = dong_porf_porf_todo_todoDone31jjtype;
      _get281 = jjnewtarget;
      // if 
        if (((u32)(_get281)) != 0) {
          _get282 = jjreturn;
          _get283 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get282), _get283)) == 0) {
              _get284 = jjthis;
              _get285 = jjthisjjtype;
              return (struct ReturnValue){ _get284, _get285 };
            }
          // end
          j765:;
        }
      // end
      j764:;
      _get286 = jjreturn;
      _get287 = jjreturnjjtype;
      return (struct ReturnValue){ _get286, _get287 };
    }
  // end
  j763:;
  jjreturn = dong_porf_porf_todo_todoDone31;
  jjreturnjjtype = dong_porf_porf_todo_todoDone31jjtype;
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
      j767:;
    }
  // end
  j766:;
  _get293 = jjreturn;
  _get294 = jjreturnjjtype;
  return (struct ReturnValue){ _get293, _get294 };
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
      i32 _r769;
        _get5 = jjtypeswitch_tmp1;
        _get6 = jjtypeswitch_tmp1;
        // if 
          if (((_get5 == 67) | (_get6 == 195)) != 0) {
            _get7 = jjlogicinner_tmp;
            _r769 = i32_load(1, 0, (u32)(_get7));
            goto j769;
          }
        // end
        j770:;
        _get8 = jjtypeswitch_tmp1;
        _get9 = jjtypeswitch_tmp1;
        // if 
          if (((_get8 == 31) | (_get9 == 32)) != 0) {
            _r769 = 1;
            goto j769;
          }
        // end
        j771:;
        _get10 = jjlogicinner_tmp;
        const f64 _tmp0 = _get10;
        _r769 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
      // end
      j769:;
      // if 
        if ((_r769) != 0) {
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
              j774:;
            }
          // end
          j773:;
          _get16 = jjreturn;
          _get17 = jjreturnjjtype;
          return (struct ReturnValue){ _get16, _get17 };
        }
      // end
      j772:;
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
          j776:;
        }
      // end
      j775:;
      _get23 = jjreturn;
      _get24 = jjreturnjjtype;
      return (struct ReturnValue){ _get23, _get24 };
    }
  // end
  j768:;
  // if 
    if ((f64)((dong_porf_porf_todo_filterMode == 2) & ((dong_porf_porf_todo_filterModejjtype | 128) == (1 | 128))) != 0) {
      _get25 = done;
      jjlogicinner_tmp = _get25;
      _get26 = donejjtype;
      jjtypeswitch_tmp1 = _get26;
      // block i32
      i32 _r778;
        _get27 = jjtypeswitch_tmp1;
        _get28 = jjtypeswitch_tmp1;
        // if 
          if (((_get27 == 67) | (_get28 == 195)) != 0) {
            _get29 = jjlogicinner_tmp;
            _r778 = i32_load(1, 0, (u32)(_get29));
            goto j778;
          }
        // end
        j779:;
        _get30 = jjtypeswitch_tmp1;
        _get31 = jjtypeswitch_tmp1;
        // if 
          if (((_get30 == 31) | (_get31 == 32)) != 0) {
            _r778 = 1;
            goto j778;
          }
        // end
        j780:;
        _get32 = jjlogicinner_tmp;
        const f64 _tmp1 = _get32;
        _r778 = (_tmp1 < 0 ? -_tmp1 : _tmp1) > 0;
      // end
      j778:;
      // if 
        if ((_r778) != 0) {
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
              j783:;
            }
          // end
          j782:;
          _get38 = jjreturn;
          _get39 = jjreturnjjtype;
          return (struct ReturnValue){ _get38, _get39 };
        }
      // end
      j781:;
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
          j785:;
        }
      // end
      j784:;
      _get45 = jjreturn;
      _get46 = jjreturnjjtype;
      return (struct ReturnValue){ _get45, _get46 };
    }
  // end
  j777:;
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
      j787:;
    }
  // end
  j786:;
  _get52 = jjreturn;
  _get53 = jjreturnjjtype;
  return (struct ReturnValue){ _get52, _get53 };
}

static struct ReturnValue dong_porf_porf_todo_todoTextAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype) {
  i32 _get294;
  f64 _get293;
  i32 _get292;
  f64 _get291;
  i32 _get290;
  f64 _get289;
  f64 _get288;
  i32 _get287;
  f64 _get286;
  i32 _get285;
  f64 _get284;
  i32 _get283;
  f64 _get282;
  f64 _get281;
  i32 _get280;
  f64 _get279;
  i32 _get278;
  f64 _get277;
  i32 _get276;
  f64 _get275;
  i32 _get274;
  f64 _get273;
  f64 _get272;
  i32 _get271;
  f64 _get270;
  i32 _get269;
  f64 _get268;
  i32 _get267;
  f64 _get266;
  i32 _get265;
  f64 _get264;
  f64 _get263;
  i32 _get262;
  f64 _get261;
  i32 _get260;
  f64 _get259;
  i32 _get258;
  f64 _get257;
  i32 _get256;
  f64 _get255;
  f64 _get254;
  i32 _get253;
  f64 _get252;
  i32 _get251;
  f64 _get250;
  i32 _get249;
  f64 _get248;
  i32 _get247;
  f64 _get246;
  f64 _get245;
  i32 _get244;
  f64 _get243;
  i32 _get242;
  f64 _get241;
  i32 _get240;
  f64 _get239;
  i32 _get238;
  f64 _get237;
  f64 _get236;
  i32 _get235;
  f64 _get234;
  i32 _get233;
  f64 _get232;
  i32 _get231;
  f64 _get230;
  i32 _get229;
  f64 _get228;
  f64 _get227;
  i32 _get226;
  f64 _get225;
  i32 _get224;
  f64 _get223;
  i32 _get222;
  f64 _get221;
  i32 _get220;
  f64 _get219;
  f64 _get218;
  i32 _get217;
  f64 _get216;
  i32 _get215;
  f64 _get214;
  i32 _get213;
  f64 _get212;
  i32 _get211;
  f64 _get210;
  f64 _get209;
  i32 _get208;
  f64 _get207;
  i32 _get206;
  f64 _get205;
  i32 _get204;
  f64 _get203;
  i32 _get202;
  f64 _get201;
  f64 _get200;
  i32 _get199;
  f64 _get198;
  i32 _get197;
  f64 _get196;
  i32 _get195;
  f64 _get194;
  i32 _get193;
  f64 _get192;
  f64 _get191;
  i32 _get190;
  f64 _get189;
  i32 _get188;
  f64 _get187;
  i32 _get186;
  f64 _get185;
  i32 _get184;
  f64 _get183;
  f64 _get182;
  i32 _get181;
  f64 _get180;
  i32 _get179;
  f64 _get178;
  i32 _get177;
  f64 _get176;
  i32 _get175;
  f64 _get174;
  f64 _get173;
  i32 _get172;
  f64 _get171;
  i32 _get170;
  f64 _get169;
  i32 _get168;
  f64 _get167;
  i32 _get166;
  f64 _get165;
  f64 _get164;
  i32 _get163;
  f64 _get162;
  i32 _get161;
  f64 _get160;
  i32 _get159;
  f64 _get158;
  i32 _get157;
  f64 _get156;
  f64 _get155;
  i32 _get154;
  f64 _get153;
  i32 _get152;
  f64 _get151;
  i32 _get150;
  f64 _get149;
  i32 _get148;
  f64 _get147;
  f64 _get146;
  i32 _get145;
  f64 _get144;
  i32 _get143;
  f64 _get142;
  i32 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  f64 _get137;
  i32 _get136;
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
          j795:;
        }
      // end
      j794:;
      _get7 = jjreturn;
      _get8 = jjreturnjjtype;
      return (struct ReturnValue){ _get7, _get8 };
    }
  // end
  j793:;
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
          j798:;
        }
      // end
      j797:;
      _get16 = jjreturn;
      _get17 = jjreturnjjtype;
      return (struct ReturnValue){ _get16, _get17 };
    }
  // end
  j796:;
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
          j801:;
        }
      // end
      j800:;
      _get25 = jjreturn;
      _get26 = jjreturnjjtype;
      return (struct ReturnValue){ _get25, _get26 };
    }
  // end
  j799:;
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
          j804:;
        }
      // end
      j803:;
      _get34 = jjreturn;
      _get35 = jjreturnjjtype;
      return (struct ReturnValue){ _get34, _get35 };
    }
  // end
  j802:;
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
          j807:;
        }
      // end
      j806:;
      _get43 = jjreturn;
      _get44 = jjreturnjjtype;
      return (struct ReturnValue){ _get43, _get44 };
    }
  // end
  j805:;
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
          j810:;
        }
      // end
      j809:;
      _get52 = jjreturn;
      _get53 = jjreturnjjtype;
      return (struct ReturnValue){ _get52, _get53 };
    }
  // end
  j808:;
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
          j813:;
        }
      // end
      j812:;
      _get61 = jjreturn;
      _get62 = jjreturnjjtype;
      return (struct ReturnValue){ _get61, _get62 };
    }
  // end
  j811:;
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
          j816:;
        }
      // end
      j815:;
      _get70 = jjreturn;
      _get71 = jjreturnjjtype;
      return (struct ReturnValue){ _get70, _get71 };
    }
  // end
  j814:;
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
          j819:;
        }
      // end
      j818:;
      _get79 = jjreturn;
      _get80 = jjreturnjjtype;
      return (struct ReturnValue){ _get79, _get80 };
    }
  // end
  j817:;
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
          j822:;
        }
      // end
      j821:;
      _get88 = jjreturn;
      _get89 = jjreturnjjtype;
      return (struct ReturnValue){ _get88, _get89 };
    }
  // end
  j820:;
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
          j825:;
        }
      // end
      j824:;
      _get97 = jjreturn;
      _get98 = jjreturnjjtype;
      return (struct ReturnValue){ _get97, _get98 };
    }
  // end
  j823:;
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
          j828:;
        }
      // end
      j827:;
      _get106 = jjreturn;
      _get107 = jjreturnjjtype;
      return (struct ReturnValue){ _get106, _get107 };
    }
  // end
  j826:;
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
          j831:;
        }
      // end
      j830:;
      _get115 = jjreturn;
      _get116 = jjreturnjjtype;
      return (struct ReturnValue){ _get115, _get116 };
    }
  // end
  j829:;
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
          j834:;
        }
      // end
      j833:;
      _get124 = jjreturn;
      _get125 = jjreturnjjtype;
      return (struct ReturnValue){ _get124, _get125 };
    }
  // end
  j832:;
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
          j837:;
        }
      // end
      j836:;
      _get133 = jjreturn;
      _get134 = jjreturnjjtype;
      return (struct ReturnValue){ _get133, _get134 };
    }
  // end
  j835:;
  _get135 = i;
  _get136 = ijjtype;
  // if 
    if ((f64)((_get135 == 15) & ((_get136 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText15;
      jjreturnjjtype = dong_porf_porf_todo_todoText15jjtype;
      _get137 = jjnewtarget;
      // if 
        if (((u32)(_get137)) != 0) {
          _get138 = jjreturn;
          _get139 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get138), _get139)) == 0) {
              _get140 = jjthis;
              _get141 = jjthisjjtype;
              return (struct ReturnValue){ _get140, _get141 };
            }
          // end
          j840:;
        }
      // end
      j839:;
      _get142 = jjreturn;
      _get143 = jjreturnjjtype;
      return (struct ReturnValue){ _get142, _get143 };
    }
  // end
  j838:;
  _get144 = i;
  _get145 = ijjtype;
  // if 
    if ((f64)((_get144 == 16) & ((_get145 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText16;
      jjreturnjjtype = dong_porf_porf_todo_todoText16jjtype;
      _get146 = jjnewtarget;
      // if 
        if (((u32)(_get146)) != 0) {
          _get147 = jjreturn;
          _get148 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get147), _get148)) == 0) {
              _get149 = jjthis;
              _get150 = jjthisjjtype;
              return (struct ReturnValue){ _get149, _get150 };
            }
          // end
          j843:;
        }
      // end
      j842:;
      _get151 = jjreturn;
      _get152 = jjreturnjjtype;
      return (struct ReturnValue){ _get151, _get152 };
    }
  // end
  j841:;
  _get153 = i;
  _get154 = ijjtype;
  // if 
    if ((f64)((_get153 == 17) & ((_get154 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText17;
      jjreturnjjtype = dong_porf_porf_todo_todoText17jjtype;
      _get155 = jjnewtarget;
      // if 
        if (((u32)(_get155)) != 0) {
          _get156 = jjreturn;
          _get157 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get156), _get157)) == 0) {
              _get158 = jjthis;
              _get159 = jjthisjjtype;
              return (struct ReturnValue){ _get158, _get159 };
            }
          // end
          j846:;
        }
      // end
      j845:;
      _get160 = jjreturn;
      _get161 = jjreturnjjtype;
      return (struct ReturnValue){ _get160, _get161 };
    }
  // end
  j844:;
  _get162 = i;
  _get163 = ijjtype;
  // if 
    if ((f64)((_get162 == 18) & ((_get163 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText18;
      jjreturnjjtype = dong_porf_porf_todo_todoText18jjtype;
      _get164 = jjnewtarget;
      // if 
        if (((u32)(_get164)) != 0) {
          _get165 = jjreturn;
          _get166 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get165), _get166)) == 0) {
              _get167 = jjthis;
              _get168 = jjthisjjtype;
              return (struct ReturnValue){ _get167, _get168 };
            }
          // end
          j849:;
        }
      // end
      j848:;
      _get169 = jjreturn;
      _get170 = jjreturnjjtype;
      return (struct ReturnValue){ _get169, _get170 };
    }
  // end
  j847:;
  _get171 = i;
  _get172 = ijjtype;
  // if 
    if ((f64)((_get171 == 19) & ((_get172 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText19;
      jjreturnjjtype = dong_porf_porf_todo_todoText19jjtype;
      _get173 = jjnewtarget;
      // if 
        if (((u32)(_get173)) != 0) {
          _get174 = jjreturn;
          _get175 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get174), _get175)) == 0) {
              _get176 = jjthis;
              _get177 = jjthisjjtype;
              return (struct ReturnValue){ _get176, _get177 };
            }
          // end
          j852:;
        }
      // end
      j851:;
      _get178 = jjreturn;
      _get179 = jjreturnjjtype;
      return (struct ReturnValue){ _get178, _get179 };
    }
  // end
  j850:;
  _get180 = i;
  _get181 = ijjtype;
  // if 
    if ((f64)((_get180 == 20) & ((_get181 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText20;
      jjreturnjjtype = dong_porf_porf_todo_todoText20jjtype;
      _get182 = jjnewtarget;
      // if 
        if (((u32)(_get182)) != 0) {
          _get183 = jjreturn;
          _get184 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get183), _get184)) == 0) {
              _get185 = jjthis;
              _get186 = jjthisjjtype;
              return (struct ReturnValue){ _get185, _get186 };
            }
          // end
          j855:;
        }
      // end
      j854:;
      _get187 = jjreturn;
      _get188 = jjreturnjjtype;
      return (struct ReturnValue){ _get187, _get188 };
    }
  // end
  j853:;
  _get189 = i;
  _get190 = ijjtype;
  // if 
    if ((f64)((_get189 == 21) & ((_get190 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText21;
      jjreturnjjtype = dong_porf_porf_todo_todoText21jjtype;
      _get191 = jjnewtarget;
      // if 
        if (((u32)(_get191)) != 0) {
          _get192 = jjreturn;
          _get193 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get192), _get193)) == 0) {
              _get194 = jjthis;
              _get195 = jjthisjjtype;
              return (struct ReturnValue){ _get194, _get195 };
            }
          // end
          j858:;
        }
      // end
      j857:;
      _get196 = jjreturn;
      _get197 = jjreturnjjtype;
      return (struct ReturnValue){ _get196, _get197 };
    }
  // end
  j856:;
  _get198 = i;
  _get199 = ijjtype;
  // if 
    if ((f64)((_get198 == 22) & ((_get199 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText22;
      jjreturnjjtype = dong_porf_porf_todo_todoText22jjtype;
      _get200 = jjnewtarget;
      // if 
        if (((u32)(_get200)) != 0) {
          _get201 = jjreturn;
          _get202 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get201), _get202)) == 0) {
              _get203 = jjthis;
              _get204 = jjthisjjtype;
              return (struct ReturnValue){ _get203, _get204 };
            }
          // end
          j861:;
        }
      // end
      j860:;
      _get205 = jjreturn;
      _get206 = jjreturnjjtype;
      return (struct ReturnValue){ _get205, _get206 };
    }
  // end
  j859:;
  _get207 = i;
  _get208 = ijjtype;
  // if 
    if ((f64)((_get207 == 23) & ((_get208 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText23;
      jjreturnjjtype = dong_porf_porf_todo_todoText23jjtype;
      _get209 = jjnewtarget;
      // if 
        if (((u32)(_get209)) != 0) {
          _get210 = jjreturn;
          _get211 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get210), _get211)) == 0) {
              _get212 = jjthis;
              _get213 = jjthisjjtype;
              return (struct ReturnValue){ _get212, _get213 };
            }
          // end
          j864:;
        }
      // end
      j863:;
      _get214 = jjreturn;
      _get215 = jjreturnjjtype;
      return (struct ReturnValue){ _get214, _get215 };
    }
  // end
  j862:;
  _get216 = i;
  _get217 = ijjtype;
  // if 
    if ((f64)((_get216 == 24) & ((_get217 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText24;
      jjreturnjjtype = dong_porf_porf_todo_todoText24jjtype;
      _get218 = jjnewtarget;
      // if 
        if (((u32)(_get218)) != 0) {
          _get219 = jjreturn;
          _get220 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get219), _get220)) == 0) {
              _get221 = jjthis;
              _get222 = jjthisjjtype;
              return (struct ReturnValue){ _get221, _get222 };
            }
          // end
          j867:;
        }
      // end
      j866:;
      _get223 = jjreturn;
      _get224 = jjreturnjjtype;
      return (struct ReturnValue){ _get223, _get224 };
    }
  // end
  j865:;
  _get225 = i;
  _get226 = ijjtype;
  // if 
    if ((f64)((_get225 == 25) & ((_get226 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText25;
      jjreturnjjtype = dong_porf_porf_todo_todoText25jjtype;
      _get227 = jjnewtarget;
      // if 
        if (((u32)(_get227)) != 0) {
          _get228 = jjreturn;
          _get229 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get228), _get229)) == 0) {
              _get230 = jjthis;
              _get231 = jjthisjjtype;
              return (struct ReturnValue){ _get230, _get231 };
            }
          // end
          j870:;
        }
      // end
      j869:;
      _get232 = jjreturn;
      _get233 = jjreturnjjtype;
      return (struct ReturnValue){ _get232, _get233 };
    }
  // end
  j868:;
  _get234 = i;
  _get235 = ijjtype;
  // if 
    if ((f64)((_get234 == 26) & ((_get235 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText26;
      jjreturnjjtype = dong_porf_porf_todo_todoText26jjtype;
      _get236 = jjnewtarget;
      // if 
        if (((u32)(_get236)) != 0) {
          _get237 = jjreturn;
          _get238 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get237), _get238)) == 0) {
              _get239 = jjthis;
              _get240 = jjthisjjtype;
              return (struct ReturnValue){ _get239, _get240 };
            }
          // end
          j873:;
        }
      // end
      j872:;
      _get241 = jjreturn;
      _get242 = jjreturnjjtype;
      return (struct ReturnValue){ _get241, _get242 };
    }
  // end
  j871:;
  _get243 = i;
  _get244 = ijjtype;
  // if 
    if ((f64)((_get243 == 27) & ((_get244 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText27;
      jjreturnjjtype = dong_porf_porf_todo_todoText27jjtype;
      _get245 = jjnewtarget;
      // if 
        if (((u32)(_get245)) != 0) {
          _get246 = jjreturn;
          _get247 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get246), _get247)) == 0) {
              _get248 = jjthis;
              _get249 = jjthisjjtype;
              return (struct ReturnValue){ _get248, _get249 };
            }
          // end
          j876:;
        }
      // end
      j875:;
      _get250 = jjreturn;
      _get251 = jjreturnjjtype;
      return (struct ReturnValue){ _get250, _get251 };
    }
  // end
  j874:;
  _get252 = i;
  _get253 = ijjtype;
  // if 
    if ((f64)((_get252 == 28) & ((_get253 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText28;
      jjreturnjjtype = dong_porf_porf_todo_todoText28jjtype;
      _get254 = jjnewtarget;
      // if 
        if (((u32)(_get254)) != 0) {
          _get255 = jjreturn;
          _get256 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get255), _get256)) == 0) {
              _get257 = jjthis;
              _get258 = jjthisjjtype;
              return (struct ReturnValue){ _get257, _get258 };
            }
          // end
          j879:;
        }
      // end
      j878:;
      _get259 = jjreturn;
      _get260 = jjreturnjjtype;
      return (struct ReturnValue){ _get259, _get260 };
    }
  // end
  j877:;
  _get261 = i;
  _get262 = ijjtype;
  // if 
    if ((f64)((_get261 == 29) & ((_get262 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText29;
      jjreturnjjtype = dong_porf_porf_todo_todoText29jjtype;
      _get263 = jjnewtarget;
      // if 
        if (((u32)(_get263)) != 0) {
          _get264 = jjreturn;
          _get265 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get264), _get265)) == 0) {
              _get266 = jjthis;
              _get267 = jjthisjjtype;
              return (struct ReturnValue){ _get266, _get267 };
            }
          // end
          j882:;
        }
      // end
      j881:;
      _get268 = jjreturn;
      _get269 = jjreturnjjtype;
      return (struct ReturnValue){ _get268, _get269 };
    }
  // end
  j880:;
  _get270 = i;
  _get271 = ijjtype;
  // if 
    if ((f64)((_get270 == 30) & ((_get271 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText30;
      jjreturnjjtype = dong_porf_porf_todo_todoText30jjtype;
      _get272 = jjnewtarget;
      // if 
        if (((u32)(_get272)) != 0) {
          _get273 = jjreturn;
          _get274 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get273), _get274)) == 0) {
              _get275 = jjthis;
              _get276 = jjthisjjtype;
              return (struct ReturnValue){ _get275, _get276 };
            }
          // end
          j885:;
        }
      // end
      j884:;
      _get277 = jjreturn;
      _get278 = jjreturnjjtype;
      return (struct ReturnValue){ _get277, _get278 };
    }
  // end
  j883:;
  _get279 = i;
  _get280 = ijjtype;
  // if 
    if ((f64)((_get279 == 31) & ((_get280 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoText31;
      jjreturnjjtype = dong_porf_porf_todo_todoText31jjtype;
      _get281 = jjnewtarget;
      // if 
        if (((u32)(_get281)) != 0) {
          _get282 = jjreturn;
          _get283 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get282), _get283)) == 0) {
              _get284 = jjthis;
              _get285 = jjthisjjtype;
              return (struct ReturnValue){ _get284, _get285 };
            }
          // end
          j888:;
        }
      // end
      j887:;
      _get286 = jjreturn;
      _get287 = jjreturnjjtype;
      return (struct ReturnValue){ _get286, _get287 };
    }
  // end
  j886:;
  jjreturn = dong_porf_porf_todo_todoText31;
  jjreturnjjtype = dong_porf_porf_todo_todoText31jjtype;
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
      j890:;
    }
  // end
  j889:;
  _get293 = jjreturn;
  _get294 = jjreturnjjtype;
  return (struct ReturnValue){ _get293, _get294 };
}

static struct ReturnValue dong_porf_porf_todo_pullHostString(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  f64 _get1;
  f64 _get0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  jjreturn = __porf_import_dong_str_pull();
  jjreturnjjtype = 195;
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
      j896:;
    }
  // end
  j895:;
  _get5 = jjreturn;
  _get6 = jjreturnjjtype;
  return (struct ReturnValue){ _get5, _get6 };
}

static struct ReturnValue dong_porf_porf_todo_numToStr(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 n, i32 njjtype) {
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

  _get0 = n;
  __porf_import_dong_num_to_str(_get0);
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
      j898:;
    }
  // end
  j897:;
  _get7 = jjreturn;
  _get8 = jjreturnjjtype;
  return (struct ReturnValue){ _get7, _get8 };
}

static struct ReturnValue dong_porf_porf_todo_buildTodoRow(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype) {
  i32 _get50;
  f64 _get49;
  i32 _get48;
  f64 _get47;
  i32 _get46;
  f64 _get45;
  f64 _get44;
  i32 _get43;
  f64 _get42;
  i32 _get41;
  i32 _get40;
  f64 _get39;
  i32 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  f64 _get34;
  i32 _get33;
  i32 _get32;
  f64 _get31;
  i32 _get30;
  i32 _get29;
  i32 _get28;
  f64 _get27;
  f64 _get26;
  f64 _get25;
  f64 _get24;
  i32 _get23;
  i32 _get22;
  i32 _get21;
  f64 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  f64 _get16;
  f64 _get15;
  f64 _get14;
  f64 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  i32 _get9;
  i32 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  i32 _get4;
  f64 _get3;
  i32 _get2;
  i32 _get1;
  f64 _get0;
  f64 text = 0;
  i32 textjjtype = 0;
  i32 jjlast_type = 0;
  f64 done = 0;
  i32 donejjtype = 0;
  f64 doneClass = 0;
  i32 doneClassjjtype = 0;
  f64 checkMark = 0;
  i32 checkMarkjjtype = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 html = 0;
  i32 htmljjtype = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = i;
  _get1 = ijjtype;
  const struct ReturnValue _0 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get0, _get1);
  jjlast_type = _0.type;
  _get2 = jjlast_type;
  textjjtype = _get2;
  text = _0.value;
  _get3 = i;
  _get4 = ijjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get3, _get4);
  jjlast_type = _1.type;
  _get5 = jjlast_type;
  donejjtype = _get5;
  done = _1.value;
  doneClass = 0;
  doneClassjjtype = 195;
  checkMark = 0;
  checkMarkjjtype = 195;
  _get6 = done;
  jjlogicinner_tmp = _get6;
  _get7 = donejjtype;
  jjtypeswitch_tmp1 = _get7;
  // block i32
  i32 _r891;
    _get8 = jjtypeswitch_tmp1;
    _get9 = jjtypeswitch_tmp1;
    // if 
      if (((_get8 == 67) | (_get9 == 195)) != 0) {
        _get10 = jjlogicinner_tmp;
        _r891 = i32_load(1, 0, (u32)(_get10));
        goto j891;
      }
    // end
    j892:;
    _get11 = jjtypeswitch_tmp1;
    _get12 = jjtypeswitch_tmp1;
    // if 
      if (((_get11 == 31) | (_get12 == 32)) != 0) {
        _r891 = 1;
        goto j891;
      }
    // end
    j893:;
    _get13 = jjlogicinner_tmp;
    const f64 _tmp0 = _get13;
    _r891 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
  // end
  j891:;
  // if 
    if ((_r891) != 0) {
      doneClass = 2176;
      _get14 = doneClass;
      doneClassjjtype = 195;
      (void) _get14;
      checkMark = 2187;
      _get15 = checkMark;
      checkMarkjjtype = 195;
      (void) _get15;
    }
  // end
  j894:;
  html = 0;
  htmljjtype = 195;
  _get16 = html;
  const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_strcat((u32)(_get16), htmljjtype, 2194, 195);
  jjlast_type = _2.type;
  _get17 = doneClass;
  const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_strcat(_2.value, _2.type, (u32)(_get17), 195);
  jjlast_type = _3.type;
  const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_strcat(_3.value, _3.type, 2220, 195);
  jjlast_type = _4.type;
  html = (f64)(_4.value);
  _get18 = html;
  htmljjtype = _4.type;
  (void) _get18;
  _get19 = html;
  const struct ReturnValue _5 = dong_porf_porf_todo__Porffor_strcat((u32)(_get19), htmljjtype, 2228, 195);
  jjlast_type = _5.type;
  _get20 = i;
  _get21 = ijjtype;
  const struct ReturnValue _6 = dong_porf_porf_todo_numToStr(0, 0, 0, 0, _get20, _get21);
  jjlast_type = _6.type;
  _get22 = jjlast_type;
  const struct ReturnValue _7 = dong_porf_porf_todo__Porffor_concatStrings((f64)(_5.value), _5.type, _6.value, _get22);
  jjlast_type = _7.type;
  _get23 = jjlast_type;
  const struct ReturnValue _8 = dong_porf_porf_todo__Porffor_concatStrings(_7.value, _get23, 2220, 195);
  jjlast_type = _8.type;
  _get24 = checkMark;
  const struct ReturnValue _9 = dong_porf_porf_todo__Porffor_strcat((u32)(_8.value), _8.type, (u32)(_get24), 195);
  jjlast_type = _9.type;
  const struct ReturnValue _10 = dong_porf_porf_todo__Porffor_strcat(_9.value, _9.type, 2282, 195);
  jjlast_type = _10.type;
  html = (f64)(_10.value);
  _get25 = html;
  htmljjtype = _10.type;
  (void) _get25;
  _get26 = html;
  const struct ReturnValue _11 = dong_porf_porf_todo__Porffor_strcat((u32)(_get26), htmljjtype, 2294, 195);
  jjlast_type = _11.type;
  _get27 = text;
  _get28 = textjjtype;
  const struct ReturnValue _12 = dong_porf_porf_todo__Porffor_concatStrings((f64)(_11.value), _11.type, _get27, _get28);
  jjlast_type = _12.type;
  _get29 = jjlast_type;
  const struct ReturnValue _13 = dong_porf_porf_todo__Porffor_concatStrings(_12.value, _get29, 2324, 195);
  jjlast_type = _13.type;
  _get30 = jjlast_type;
  htmljjtype = _get30;
  html = _13.value;
  _get31 = html;
  _get32 = htmljjtype;
  const struct ReturnValue _14 = dong_porf_porf_todo__Porffor_concatStrings(_get31, _get32, 2337, 195);
  jjlast_type = _14.type;
  _get33 = jjlast_type;
  _get34 = i;
  _get35 = ijjtype;
  const struct ReturnValue _15 = dong_porf_porf_todo_numToStr(0, 0, 0, 0, _get34, _get35);
  jjlast_type = _15.type;
  _get36 = jjlast_type;
  const struct ReturnValue _16 = dong_porf_porf_todo__Porffor_concatStrings(_14.value, _get33, _15.value, _get36);
  jjlast_type = _16.type;
  _get37 = jjlast_type;
  const struct ReturnValue _17 = dong_porf_porf_todo__Porffor_concatStrings(_16.value, _get37, 2395, 195);
  jjlast_type = _17.type;
  _get38 = jjlast_type;
  htmljjtype = _get38;
  html = _17.value;
  _get39 = html;
  _get40 = htmljjtype;
  const struct ReturnValue _18 = dong_porf_porf_todo__Porffor_concatStrings(_get39, _get40, 2282, 195);
  jjlast_type = _18.type;
  _get41 = jjlast_type;
  htmljjtype = _get41;
  html = _18.value;
  _get42 = html;
  jjreturn = _get42;
  _get43 = htmljjtype;
  jjreturnjjtype = _get43;
  _get44 = jjnewtarget;
  // if 
    if (((u32)(_get44)) != 0) {
      _get45 = jjreturn;
      _get46 = jjreturnjjtype;
      // if 
        if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get45), _get46)) == 0) {
          _get47 = jjthis;
          _get48 = jjthisjjtype;
          return (struct ReturnValue){ _get47, _get48 };
        }
      // end
      j900:;
    }
  // end
  j899:;
  _get49 = jjreturn;
  _get50 = jjreturnjjtype;
  return (struct ReturnValue){ _get49, _get50 };
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
    __porf_import_dong_set_inner_html_typed(_get0, _get1, _get2);
  _get3 = jjnewtarget;
  // if 
    if (((u32)(_get3)) != 0) {
      _get4 = jjthis;
      _get5 = jjthisjjtype;
      return (struct ReturnValue){ _get4, _get5 };
    }
  // end
  j902:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_porfRebuild_todos(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get31;
  f64 _get30;
  f64 _get29;
  f64 _get28;
  f64 _get27;
  f64 _get26;
  f64 _get25;
  f64 _get24;
  i32 _get23;
  f64 _get22;
  i32 _get21;
  f64 _get20;
  i32 _get19;
  f64 _get18;
  i32 _get17;
  i32 _get16;
  f64 _get15;
  i32 _get14;
  f64 _get13;
  f64 _get12;
  f64 _get11;
  f64 _get10;
  i32 _get9;
  i32 _get8;
  f64 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 html = 0;
  i32 htmljjtype = 0;
  f64 i = 0;
  i32 ijjtype = 0;
  i32 jjlast_type = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 __tmpop_left = 0;
  f64 __tmpop_right = 0;

  html = 2118;
  htmljjtype = 195;
  i = 0;
  ijjtype = 1;
  // loop 
  j666:;
    _get0 = i;
    // if 
      if (_get0 < dong_porf_porf_todo_todoCount) {
        _get1 = i;
        // if 
          if ((f64)(_get1 >= 32) != 0) {
            const struct ReturnValue _0 = dong_porf_porf_todo_dongLog(0, 0, 0, 0, 2129, 195);
            jjlast_type = _0.type;
            (void) _0.value;
            goto j667;
          }
        // end
        j668:;
        _get2 = i;
        _get3 = ijjtype;
        const struct ReturnValue _1 = dong_porf_porf_todo_shouldShow(0, 0, 0, 0, _get2, _get3);
        jjlast_type = _1.type;
        jjlogicinner_tmp = _1.value;
        _get4 = jjlast_type;
        jjtypeswitch_tmp1 = _get4;
        // block i32
        i32 _r788;
          _get5 = jjtypeswitch_tmp1;
          _get6 = jjtypeswitch_tmp1;
          // if 
            if (((_get5 == 67) | (_get6 == 195)) != 0) {
              _get7 = jjlogicinner_tmp;
              _r788 = i32_load(1, 0, (u32)(_get7));
              goto j788;
            }
          // end
          j789:;
          _get8 = jjtypeswitch_tmp1;
          _get9 = jjtypeswitch_tmp1;
          // if 
            if (((_get8 == 31) | (_get9 == 32)) != 0) {
              _r788 = 1;
              goto j788;
            }
          // end
          j790:;
          _get10 = jjlogicinner_tmp;
          const f64 _tmp0 = _get10;
          _r788 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
        // end
        j788:;
        // if 
          if ((_r788) != 0) {
            // block f64
            f64 _r792;
              _get11 = html;
              __tmpop_left = _get11;
              _get12 = __tmpop_left;
              _get13 = i;
              _get14 = ijjtype;
              const struct ReturnValue _2 = dong_porf_porf_todo_buildTodoRow(0, 0, 0, 0, _get13, _get14);
              jjlast_type = _2.type;
              __tmpop_right = _2.value;
              _get15 = __tmpop_right;
              _get16 = htmljjtype;
              _get17 = jjlast_type;
              // if 
                if ((((_get16 | 128) == 195) | ((_get17 | 128) == 195)) != 0) {
                  _get18 = __tmpop_left;
                  _get19 = htmljjtype;
                  _get20 = __tmpop_right;
                  _get21 = jjlast_type;
                  const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_concatStrings(_get18, _get19, _get20, _get21);
                  jjlast_type = _3.type;
                  _r792 = _3.value;
                  goto j792;
                }
              // end
              j901:;
              jjlast_type = 1;
              _r792 = _get12 + _get15;
            // end
            j792:;
            html = _r792;
            _get22 = html;
            _get23 = jjlast_type;
            htmljjtype = _get23;
            (void) _get22;
          }
        // end
        j791:;
        _get24 = i;
        i = _get24 + 1;
        _get25 = i;
        ijjtype = 1;
        (void) _get25;
        goto j666;
      }
    // end
    j667:;
  // end
  _get26 = html;
  const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_strcat((u32)(_get26), htmljjtype, 2282, 195);
  jjlast_type = _4.type;
  html = (f64)(_4.value);
  _get27 = html;
  htmljjtype = _4.type;
  (void) _get27;
  _get28 = html;
  const struct ReturnValue _5 = dong_porf_porf_todo_setInnerHTML(0, 0, 0, 0, dong_porf_porf_todo_todo_listId, 1, _get28, htmljjtype);
  jjlast_type = _5.type;
  (void) _5.value;
  _get29 = jjnewtarget;
  // if 
    if (((u32)(_get29)) != 0) {
      _get30 = jjthis;
      _get31 = jjthisjjtype;
      return (struct ReturnValue){ _get30, _get31 };
    }
  // end
  j903:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_countActive(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
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
  f64 n = 0;
  i32 njjtype = 0;
  f64 i = 0;
  i32 ijjtype = 0;
  i32 jjlast_type = 0;
  f64 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  n = 0;
  njjtype = 1;
  i = 0;
  ijjtype = 1;
  // loop 
  j904:;
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
        // block f64
        f64 _r906;
          _get4 = jjtypeswitch_tmp1;
          _get5 = jjtypeswitch_tmp1;
          // if 
            if (((_get4 == 67) | (_get5 == 195)) != 0) {
              _get6 = jjlogicinner_tmp;
              _r906 = (f64)((i32_load(1, 0, (u32)(_get6))) == 0);
              goto j906;
            }
          // end
          j907:;
          _get7 = jjtypeswitch_tmp1;
          _get8 = jjtypeswitch_tmp1;
          // if 
            if (((_get7 == 31) | (_get8 == 32)) != 0) {
              _r906 = 0;
              goto j906;
            }
          // end
          j908:;
          _get9 = jjlogicinner_tmp;
          const f64 _tmp0 = _get9;
          _r906 = (f64)(!((_tmp0 < 0 ? -_tmp0 : _tmp0) > 0));
        // end
        j906:;
        const f64 _tmp1 = _r906;
        // if 
          if ((_tmp1 < 0 ? -_tmp1 : _tmp1) > 0) {
            _get10 = n;
            n = _get10 + 1;
            _get11 = n;
            njjtype = 1;
            (void) _get11;
          }
        // end
        j909:;
        _get12 = i;
        i = _get12 + 1;
        _get13 = i;
        ijjtype = 1;
        (void) _get13;
        _get14 = i;
        if (!(_get14 < dong_porf_porf_todo_todoCount)) {
          goto j905;
        }
        _get15 = i;
        _get16 = ijjtype;
        const struct ReturnValue _1 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get15, _get16);
        jjlast_type = _1.type;
        jjlogicinner_tmp = _1.value;
        _get17 = jjlast_type;
        jjtypeswitch_tmp1 = _get17;
        // block f64
        f64 _r910;
          _get18 = jjtypeswitch_tmp1;
          _get19 = jjtypeswitch_tmp1;
          // if 
            if (((_get18 == 67) | (_get19 == 195)) != 0) {
              _get20 = jjlogicinner_tmp;
              _r910 = (f64)((i32_load(1, 0, (u32)(_get20))) == 0);
              goto j910;
            }
          // end
          j911:;
          _get21 = jjtypeswitch_tmp1;
          _get22 = jjtypeswitch_tmp1;
          // if 
            if (((_get21 == 31) | (_get22 == 32)) != 0) {
              _r910 = 0;
              goto j910;
            }
          // end
          j912:;
          _get23 = jjlogicinner_tmp;
          const f64 _tmp2 = _get23;
          _r910 = (f64)(!((_tmp2 < 0 ? -_tmp2 : _tmp2) > 0));
        // end
        j910:;
        const f64 _tmp3 = _r910;
        // if 
          if ((_tmp3 < 0 ? -_tmp3 : _tmp3) > 0) {
            _get24 = n;
            n = _get24 + 1;
            _get25 = n;
            njjtype = 1;
            (void) _get25;
          }
        // end
        j913:;
        _get26 = i;
        i = _get26 + 1;
        _get27 = i;
        ijjtype = 1;
        (void) _get27;
        _get28 = i;
        if (!(_get28 < dong_porf_porf_todo_todoCount)) {
          goto j905;
        }
        _get29 = i;
        _get30 = ijjtype;
        const struct ReturnValue _2 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get29, _get30);
        jjlast_type = _2.type;
        jjlogicinner_tmp = _2.value;
        _get31 = jjlast_type;
        jjtypeswitch_tmp1 = _get31;
        // block f64
        f64 _r914;
          _get32 = jjtypeswitch_tmp1;
          _get33 = jjtypeswitch_tmp1;
          // if 
            if (((_get32 == 67) | (_get33 == 195)) != 0) {
              _get34 = jjlogicinner_tmp;
              _r914 = (f64)((i32_load(1, 0, (u32)(_get34))) == 0);
              goto j914;
            }
          // end
          j915:;
          _get35 = jjtypeswitch_tmp1;
          _get36 = jjtypeswitch_tmp1;
          // if 
            if (((_get35 == 31) | (_get36 == 32)) != 0) {
              _r914 = 0;
              goto j914;
            }
          // end
          j916:;
          _get37 = jjlogicinner_tmp;
          const f64 _tmp4 = _get37;
          _r914 = (f64)(!((_tmp4 < 0 ? -_tmp4 : _tmp4) > 0));
        // end
        j914:;
        const f64 _tmp5 = _r914;
        // if 
          if ((_tmp5 < 0 ? -_tmp5 : _tmp5) > 0) {
            _get38 = n;
            n = _get38 + 1;
            _get39 = n;
            njjtype = 1;
            (void) _get39;
          }
        // end
        j917:;
        _get40 = i;
        i = _get40 + 1;
        _get41 = i;
        ijjtype = 1;
        (void) _get41;
        _get42 = i;
        if (!(_get42 < dong_porf_porf_todo_todoCount)) {
          goto j905;
        }
        _get43 = i;
        _get44 = ijjtype;
        const struct ReturnValue _3 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get43, _get44);
        jjlast_type = _3.type;
        jjlogicinner_tmp = _3.value;
        _get45 = jjlast_type;
        jjtypeswitch_tmp1 = _get45;
        // block f64
        f64 _r918;
          _get46 = jjtypeswitch_tmp1;
          _get47 = jjtypeswitch_tmp1;
          // if 
            if (((_get46 == 67) | (_get47 == 195)) != 0) {
              _get48 = jjlogicinner_tmp;
              _r918 = (f64)((i32_load(1, 0, (u32)(_get48))) == 0);
              goto j918;
            }
          // end
          j919:;
          _get49 = jjtypeswitch_tmp1;
          _get50 = jjtypeswitch_tmp1;
          // if 
            if (((_get49 == 31) | (_get50 == 32)) != 0) {
              _r918 = 0;
              goto j918;
            }
          // end
          j920:;
          _get51 = jjlogicinner_tmp;
          const f64 _tmp6 = _get51;
          _r918 = (f64)(!((_tmp6 < 0 ? -_tmp6 : _tmp6) > 0));
        // end
        j918:;
        const f64 _tmp7 = _r918;
        // if 
          if ((_tmp7 < 0 ? -_tmp7 : _tmp7) > 0) {
            _get52 = n;
            n = _get52 + 1;
            _get53 = n;
            njjtype = 1;
            (void) _get53;
          }
        // end
        j921:;
        _get54 = i;
        i = _get54 + 1;
        _get55 = i;
        ijjtype = 1;
        (void) _get55;
        goto j904;
      }
    // end
    j905:;
  // end
  _get56 = n;
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
      j923:;
    }
  // end
  j922:;
  _get62 = jjreturn;
  _get63 = jjreturnjjtype;
  return (struct ReturnValue){ _get62, _get63 };
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
  j924:;
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
        i32 _r926;
          _get4 = jjtypeswitch_tmp1;
          _get5 = jjtypeswitch_tmp1;
          // if 
            if (((_get4 == 67) | (_get5 == 195)) != 0) {
              _get6 = jjlogicinner_tmp;
              _r926 = i32_load(1, 0, (u32)(_get6));
              goto j926;
            }
          // end
          j927:;
          _get7 = jjtypeswitch_tmp1;
          _get8 = jjtypeswitch_tmp1;
          // if 
            if (((_get7 == 31) | (_get8 == 32)) != 0) {
              _r926 = 1;
              goto j926;
            }
          // end
          j928:;
          _get9 = jjlogicinner_tmp;
          const f64 _tmp0 = _get9;
          _r926 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
        // end
        j926:;
        // if 
          if ((_r926) != 0) {
            _get10 = d;
            d = _get10 + 1;
            _get11 = d;
            djjtype = 1;
            (void) _get11;
          }
        // end
        j929:;
        _get12 = i;
        i = _get12 + 1;
        _get13 = i;
        ijjtype = 1;
        (void) _get13;
        _get14 = i;
        if (!(_get14 < dong_porf_porf_todo_todoCount)) {
          goto j925;
        }
        _get15 = i;
        _get16 = ijjtype;
        const struct ReturnValue _1 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get15, _get16);
        jjlast_type = _1.type;
        jjlogicinner_tmp = _1.value;
        _get17 = jjlast_type;
        jjtypeswitch_tmp1 = _get17;
        // block i32
        i32 _r930;
          _get18 = jjtypeswitch_tmp1;
          _get19 = jjtypeswitch_tmp1;
          // if 
            if (((_get18 == 67) | (_get19 == 195)) != 0) {
              _get20 = jjlogicinner_tmp;
              _r930 = i32_load(1, 0, (u32)(_get20));
              goto j930;
            }
          // end
          j931:;
          _get21 = jjtypeswitch_tmp1;
          _get22 = jjtypeswitch_tmp1;
          // if 
            if (((_get21 == 31) | (_get22 == 32)) != 0) {
              _r930 = 1;
              goto j930;
            }
          // end
          j932:;
          _get23 = jjlogicinner_tmp;
          const f64 _tmp1 = _get23;
          _r930 = (_tmp1 < 0 ? -_tmp1 : _tmp1) > 0;
        // end
        j930:;
        // if 
          if ((_r930) != 0) {
            _get24 = d;
            d = _get24 + 1;
            _get25 = d;
            djjtype = 1;
            (void) _get25;
          }
        // end
        j933:;
        _get26 = i;
        i = _get26 + 1;
        _get27 = i;
        ijjtype = 1;
        (void) _get27;
        _get28 = i;
        if (!(_get28 < dong_porf_porf_todo_todoCount)) {
          goto j925;
        }
        _get29 = i;
        _get30 = ijjtype;
        const struct ReturnValue _2 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get29, _get30);
        jjlast_type = _2.type;
        jjlogicinner_tmp = _2.value;
        _get31 = jjlast_type;
        jjtypeswitch_tmp1 = _get31;
        // block i32
        i32 _r934;
          _get32 = jjtypeswitch_tmp1;
          _get33 = jjtypeswitch_tmp1;
          // if 
            if (((_get32 == 67) | (_get33 == 195)) != 0) {
              _get34 = jjlogicinner_tmp;
              _r934 = i32_load(1, 0, (u32)(_get34));
              goto j934;
            }
          // end
          j935:;
          _get35 = jjtypeswitch_tmp1;
          _get36 = jjtypeswitch_tmp1;
          // if 
            if (((_get35 == 31) | (_get36 == 32)) != 0) {
              _r934 = 1;
              goto j934;
            }
          // end
          j936:;
          _get37 = jjlogicinner_tmp;
          const f64 _tmp2 = _get37;
          _r934 = (_tmp2 < 0 ? -_tmp2 : _tmp2) > 0;
        // end
        j934:;
        // if 
          if ((_r934) != 0) {
            _get38 = d;
            d = _get38 + 1;
            _get39 = d;
            djjtype = 1;
            (void) _get39;
          }
        // end
        j937:;
        _get40 = i;
        i = _get40 + 1;
        _get41 = i;
        ijjtype = 1;
        (void) _get41;
        _get42 = i;
        if (!(_get42 < dong_porf_porf_todo_todoCount)) {
          goto j925;
        }
        _get43 = i;
        _get44 = ijjtype;
        const struct ReturnValue _3 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get43, _get44);
        jjlast_type = _3.type;
        jjlogicinner_tmp = _3.value;
        _get45 = jjlast_type;
        jjtypeswitch_tmp1 = _get45;
        // block i32
        i32 _r938;
          _get46 = jjtypeswitch_tmp1;
          _get47 = jjtypeswitch_tmp1;
          // if 
            if (((_get46 == 67) | (_get47 == 195)) != 0) {
              _get48 = jjlogicinner_tmp;
              _r938 = i32_load(1, 0, (u32)(_get48));
              goto j938;
            }
          // end
          j939:;
          _get49 = jjtypeswitch_tmp1;
          _get50 = jjtypeswitch_tmp1;
          // if 
            if (((_get49 == 31) | (_get50 == 32)) != 0) {
              _r938 = 1;
              goto j938;
            }
          // end
          j940:;
          _get51 = jjlogicinner_tmp;
          const f64 _tmp3 = _get51;
          _r938 = (_tmp3 < 0 ? -_tmp3 : _tmp3) > 0;
        // end
        j938:;
        // if 
          if ((_r938) != 0) {
            _get52 = d;
            d = _get52 + 1;
            _get53 = d;
            djjtype = 1;
            (void) _get53;
          }
        // end
        j941:;
        _get54 = i;
        i = _get54 + 1;
        _get55 = i;
        ijjtype = 1;
        (void) _get55;
        goto j924;
      }
    // end
    j925:;
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
      j943:;
    }
  // end
  j942:;
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
  j952:;
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
  j951:;
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
      j954:;
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
            goto j954;
          }
        // end
        j955:;
      // end
    }
  // end
  j953:;
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
      f64 _r945;
        _get9 = jjtypeswitch_tmp1;
        _get10 = jjtypeswitch_tmp1;
        // if 
          if (((_get9 == 67) | (_get10 == 195)) != 0) {
            _get11 = jjlogicinner_tmp;
            _r945 = (f64)((i32_load(1, 0, (u32)(_get11))) == 0);
            goto j945;
          }
        // end
        j946:;
        _get12 = jjlogicinner_tmp;
        _r945 = (f64)(_get12 == 0);
      // end
      j945:;
      logictmp = _r945;
      _get13 = logictmp;
      // if f64
      f64 _r947;
        if (((u32)(_get13)) != 0) {
          _get14 = valuejjtype;
          jjlast_type = 2;
          _r947 = (f64)((f64)(_get14) == 5);
        } else {
          _get15 = logictmp;
          jjlast_type = 2;
          _r947 = _get15;
        }
      // end
      j947:;
      jjlogicinner_tmp = _r947;
      _get16 = jjlast_type;
      jjtypeswitch_tmp1 = _get16;
      // block i32
      i32 _r948;
        _get17 = jjtypeswitch_tmp1;
        _get18 = jjtypeswitch_tmp1;
        // if 
          if (((_get17 == 67) | (_get18 == 195)) != 0) {
            _get19 = jjlogicinner_tmp;
            _r948 = i32_load(1, 0, (u32)(_get19));
            goto j948;
          }
        // end
        j949:;
        _get20 = jjlogicinner_tmp;
        _r948 = (u32)(_get20);
      // end
      j948:;
      // if 
        if ((_r948) != 0) {
          _get21 = value;
          _get22 = valuejjtype;
          const struct ReturnValue _0 = dong_porf_porf_todo__Symbol_prototype_toString(_get21, _get22);
          jjlast_type = _0.type;
          _get23 = jjlast_type;
          return (struct ReturnValue){ _0.value, _get23 };
        }
      // end
      j950:;
      _get24 = value;
      _get25 = valuejjtype;
      const struct ReturnValue _1 = dong_porf_porf_todo__ecma262_ToString(_get24, _get25);
      jjlast_type = _1.type;
      _get26 = jjlast_type;
      sjjtype = _get26;
      s = _1.value;
    }
  // end
  j944:;
  _get27 = jjnewtarget;
  jjlogicinner_tmp = _get27;
  _get28 = jjnewtargetjjtype;
  jjtypeswitch_tmp1 = _get28;
  // block f64
  f64 _r956;
    _get29 = jjtypeswitch_tmp1;
    _get30 = jjtypeswitch_tmp1;
    // if 
      if (((_get29 == 67) | (_get30 == 195)) != 0) {
        _get31 = jjlogicinner_tmp;
        _r956 = (f64)((i32_load(1, 0, (u32)(_get31))) == 0);
        goto j956;
      }
    // end
    j957:;
    _get32 = jjlogicinner_tmp;
    _r956 = (f64)(_get32 == 0);
  // end
  j956:;
  // if 
    if (((u32)(_r956)) != 0) {
      _get33 = s;
      _get34 = sjjtype;
      return (struct ReturnValue){ _get33, _get34 };
    }
  // end
  j958:;
  _get35 = sjjtype;
  // if 
    if ((f64)(_get35) == 195) {
      _get36 = s;
      s = (f64)(dong_porf_porf_todo__Porffor_bytestringToString((i32)(_get36)));
      sjjtype = 67;
    }
  // end
  j959:;
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
  j960:;
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
  j967:;
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
  f64_store(0, 4, 131072, _get2);
  _get3 = totaljjtype;
  i32_store8(0, 12, 131072, _get3);
  i32_store(1, 0, 131072, 1);
  const struct ReturnValue _2 = dong_porf_porf_todo_String(0, 0, 0, 0, 131072, 72);
  jjlast_type = _2.type;
  _get4 = jjlast_type;
  const struct ReturnValue _3 = dong_porf_porf_todo__Porffor_concatStrings(1955, 195, _2.value, _get4);
  jjlast_type = _3.type;
  _get5 = jjlast_type;
  const struct ReturnValue _4 = dong_porf_porf_todo__Porffor_concatStrings(_3.value, _get5, 1966, 195);
  jjlast_type = _4.type;
  const struct ReturnValue _5 = dong_porf_porf_todo_setTextContent(0, 0, 0, 0, dong_porf_porf_todo_filter_allId, 1, _4.value, 195);
  jjlast_type = _5.type;
  (void) _5.value;
  _get6 = active;
  f64_store(0, 4, 147456, _get6);
  _get7 = activejjtype;
  i32_store8(0, 12, 147456, _get7);
  i32_store(1, 0, 147456, 1);
  const struct ReturnValue _6 = dong_porf_porf_todo_String(0, 0, 0, 0, 147456, 72);
  jjlast_type = _6.type;
  _get8 = jjlast_type;
  const struct ReturnValue _7 = dong_porf_porf_todo__Porffor_concatStrings(1973, 195, _6.value, _get8);
  jjlast_type = _7.type;
  _get9 = jjlast_type;
  const struct ReturnValue _8 = dong_porf_porf_todo__Porffor_concatStrings(_7.value, _get9, 1966, 195);
  jjlast_type = _8.type;
  const struct ReturnValue _9 = dong_porf_porf_todo_setTextContent(0, 0, 0, 0, dong_porf_porf_todo_filter_activeId, 1, _8.value, 195);
  jjlast_type = _9.type;
  (void) _9.value;
  _get10 = done;
  f64_store(0, 4, 163840, _get10);
  _get11 = donejjtype;
  i32_store8(0, 12, 163840, _get11);
  i32_store(1, 0, 163840, 1);
  const struct ReturnValue _10 = dong_porf_porf_todo_String(0, 0, 0, 0, 163840, 72);
  jjlast_type = _10.type;
  _get12 = jjlast_type;
  const struct ReturnValue _11 = dong_porf_porf_todo__Porffor_concatStrings(1987, 195, _10.value, _get12);
  jjlast_type = _11.type;
  _get13 = jjlast_type;
  const struct ReturnValue _12 = dong_porf_porf_todo__Porffor_concatStrings(_11.value, _get13, 1966, 195);
  jjlast_type = _12.type;
  const struct ReturnValue _13 = dong_porf_porf_todo_setTextContent(0, 0, 0, 0, dong_porf_porf_todo_filter_doneId, 1, _12.value, 195);
  jjlast_type = _13.type;
  (void) _13.value;
  // if f64
  f64 _r961;
    if ((f64)(dong_porf_porf_todo_filterMode == 0) != 0) {
      jjlast_type = 195;
      _r961 = 1999;
    } else {
      jjlast_type = 195;
      _r961 = 2012;
    }
  // end
  j961:;
  allBg = _r961;
  _get14 = jjlast_type;
  allBgjjtype = _get14;
  // if f64
  f64 _r962;
    if ((f64)(dong_porf_porf_todo_filterMode == 0) != 0) {
      jjlast_type = 195;
      _r962 = 2025;
    } else {
      jjlast_type = 195;
      _r962 = 2035;
    }
  // end
  j962:;
  allColor = _r962;
  _get15 = jjlast_type;
  allColorjjtype = _get15;
  // if f64
  f64 _r963;
    if ((f64)(dong_porf_porf_todo_filterMode == 1) != 0) {
      jjlast_type = 195;
      _r963 = 1999;
    } else {
      jjlast_type = 195;
      _r963 = 2012;
    }
  // end
  j963:;
  actBg = _r963;
  _get16 = jjlast_type;
  actBgjjtype = _get16;
  // if f64
  f64 _r964;
    if ((f64)(dong_porf_porf_todo_filterMode == 1) != 0) {
      jjlast_type = 195;
      _r964 = 2025;
    } else {
      jjlast_type = 195;
      _r964 = 2035;
    }
  // end
  j964:;
  actColor = _r964;
  _get17 = jjlast_type;
  actColorjjtype = _get17;
  // if f64
  f64 _r965;
    if ((f64)(dong_porf_porf_todo_filterMode == 2) != 0) {
      jjlast_type = 195;
      _r965 = 1999;
    } else {
      jjlast_type = 195;
      _r965 = 2012;
    }
  // end
  j965:;
  doneBg = _r965;
  _get18 = jjlast_type;
  doneBgjjtype = _get18;
  // if f64
  f64 _r966;
    if ((f64)(dong_porf_porf_todo_filterMode == 2) != 0) {
      jjlast_type = 195;
      _r966 = 2025;
    } else {
      jjlast_type = 195;
      _r966 = 2035;
    }
  // end
  j966:;
  doneColor = _r966;
  _get19 = jjlast_type;
  doneColorjjtype = _get19;
  _get20 = allBg;
  _get21 = allBgjjtype;
  const struct ReturnValue _14 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filter_allId, 1, 2048, 195, _get20, _get21);
  jjlast_type = _14.type;
  (void) _14.value;
  _get22 = allColor;
  _get23 = allColorjjtype;
  const struct ReturnValue _15 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filter_allId, 1, 2070, 195, _get22, _get23);
  jjlast_type = _15.type;
  (void) _15.value;
  _get24 = actBg;
  _get25 = actBgjjtype;
  const struct ReturnValue _16 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filter_activeId, 1, 2048, 195, _get24, _get25);
  jjlast_type = _16.type;
  (void) _16.value;
  _get26 = actColor;
  _get27 = actColorjjtype;
  const struct ReturnValue _17 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filter_activeId, 1, 2070, 195, _get26, _get27);
  jjlast_type = _17.type;
  (void) _17.value;
  _get28 = doneBg;
  _get29 = doneBgjjtype;
  const struct ReturnValue _18 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filter_doneId, 1, 2048, 195, _get28, _get29);
  jjlast_type = _18.type;
  (void) _18.value;
  _get30 = doneColor;
  _get31 = doneColorjjtype;
  const struct ReturnValue _19 = dong_porf_porf_todo_setStyle(0, 0, 0, 0, dong_porf_porf_todo_filter_doneId, 1, 2070, 195, _get30, _get31);
  jjlast_type = _19.type;
  (void) _19.value;
  _get32 = done;
  // if 
    if ((f64)(_get32 > 0) != 0) {
      dong_porf_porf_todo_showClear = 1;
      dong_porf_porf_todo_showClearjjtype = 1;
      (void) dong_porf_porf_todo_showClear;
      _get33 = done;
      f64_store(0, 4, 180224, _get33);
      _get34 = donejjtype;
      i32_store8(0, 12, 180224, _get34);
      i32_store(1, 0, 180224, 1);
      const struct ReturnValue _20 = dong_porf_porf_todo_String(0, 0, 0, 0, 180224, 72);
      jjlast_type = _20.type;
      _get35 = jjlast_type;
      const struct ReturnValue _21 = dong_porf_porf_todo__Porffor_concatStrings(2081, 195, _20.value, _get35);
      jjlast_type = _21.type;
      _get36 = jjlast_type;
      const struct ReturnValue _22 = dong_porf_porf_todo__Porffor_concatStrings(_21.value, _get36, 1966, 195);
      jjlast_type = _22.type;
      const struct ReturnValue _23 = dong_porf_porf_todo_setTextContent(0, 0, 0, 0, dong_porf_porf_todo_btn_clearId, 1, _22.value, 195);
      jjlast_type = _23.type;
      (void) _23.value;
    } else {
      dong_porf_porf_todo_showClear = 0;
      dong_porf_porf_todo_showClearjjtype = 1;
      (void) dong_porf_porf_todo_showClear;
    }
  // end
  j968:;
  const struct ReturnValue _24 = dong_porf_porf_todo_porfPatchIf_clear_wrap(0, 0, 0, 0);
  jjlast_type = _24.type;
  (void) _24.value;
  _get37 = jjnewtarget;
  // if 
    if (((u32)(_get37)) != 0) {
      _get38 = jjthis;
      _get39 = jjthisjjtype;
      return (struct ReturnValue){ _get38, _get39 };
    }
  // end
  j969:;
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
  const struct ReturnValue _1 = dong_porf_porf_todo_porfRebuild_todos(0, 0, 0, 0);
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
  j970:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_porfInit(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get13;
  f64 _get12;
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
  i32 jjlast_type = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2668, 195);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  dong_porf_porf_todo_porf_rootIdjjtype = _get0;
  dong_porf_porf_todo_porf_rootId = _0.value;
  const struct ReturnValue _1 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2683, 195);
  jjlast_type = _1.type;
  _get1 = jjlast_type;
  dong_porf_porf_todo_titleIdjjtype = _get1;
  dong_porf_porf_todo_titleId = _1.value;
  const struct ReturnValue _2 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2694, 195);
  jjlast_type = _2.type;
  _get2 = jjlast_type;
  dong_porf_porf_todo_todo_inputIdjjtype = _get2;
  dong_porf_porf_todo_todo_inputId = _2.value;
  const struct ReturnValue _3 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2710, 195);
  jjlast_type = _3.type;
  _get3 = jjlast_type;
  dong_porf_porf_todo_btn_addIdjjtype = _get3;
  dong_porf_porf_todo_btn_addId = _3.value;
  const struct ReturnValue _4 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2723, 195);
  jjlast_type = _4.type;
  _get4 = jjlast_type;
  dong_porf_porf_todo_filter_barIdjjtype = _get4;
  dong_porf_porf_todo_filter_barId = _4.value;
  const struct ReturnValue _5 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2739, 195);
  jjlast_type = _5.type;
  _get5 = jjlast_type;
  dong_porf_porf_todo_filter_allIdjjtype = _get5;
  dong_porf_porf_todo_filter_allId = _5.value;
  const struct ReturnValue _6 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2755, 195);
  jjlast_type = _6.type;
  _get6 = jjlast_type;
  dong_porf_porf_todo_filter_activeIdjjtype = _get6;
  dong_porf_porf_todo_filter_activeId = _6.value;
  const struct ReturnValue _7 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2774, 195);
  jjlast_type = _7.type;
  _get7 = jjlast_type;
  dong_porf_porf_todo_filter_doneIdjjtype = _get7;
  dong_porf_porf_todo_filter_doneId = _7.value;
  const struct ReturnValue _8 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2791, 195);
  jjlast_type = _8.type;
  _get8 = jjlast_type;
  dong_porf_porf_todo_todo_listIdjjtype = _get8;
  dong_porf_porf_todo_todo_listId = _8.value;
  const struct ReturnValue _9 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2806, 195);
  jjlast_type = _9.type;
  _get9 = jjlast_type;
  dong_porf_porf_todo_clear_wrapIdjjtype = _get9;
  dong_porf_porf_todo_clear_wrapId = _9.value;
  const struct ReturnValue _10 = dong_porf_porf_todo_getElementById(0, 0, 0, 0, 2822, 195);
  jjlast_type = _10.type;
  _get10 = jjlast_type;
  dong_porf_porf_todo_btn_clearIdjjtype = _get10;
  dong_porf_porf_todo_btn_clearId = _10.value;
  const struct ReturnValue _11 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_todo_inputId, dong_porf_porf_todo_todo_inputIdjjtype, 2837, 195, 2848, 195);
  (void) _11.type;
  (void) _11.value;
  const struct ReturnValue _12 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_todo_inputId, dong_porf_porf_todo_todo_inputIdjjtype, 2867, 195, 2880, 195);
  (void) _12.type;
  (void) _12.value;
  const struct ReturnValue _13 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_btn_addId, dong_porf_porf_todo_btn_addIdjjtype, 2895, 195, 2906, 195);
  (void) _13.type;
  (void) _13.value;
  const struct ReturnValue _14 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_filter_allId, dong_porf_porf_todo_filter_allIdjjtype, 2895, 195, 2917, 195);
  (void) _14.type;
  (void) _14.value;
  const struct ReturnValue _15 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_filter_activeId, dong_porf_porf_todo_filter_activeIdjjtype, 2895, 195, 2934, 195);
  (void) _15.type;
  (void) _15.value;
  const struct ReturnValue _16 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_filter_doneId, dong_porf_porf_todo_filter_doneIdjjtype, 2895, 195, 2954, 195);
  (void) _16.type;
  (void) _16.value;
  const struct ReturnValue _17 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_todo_listId, dong_porf_porf_todo_todo_listIdjjtype, 2895, 195, 2972, 195);
  (void) _17.type;
  (void) _17.value;
  const struct ReturnValue _18 = dong_porf_porf_todo_addEventListener(0, 0, 0, 0, dong_porf_porf_todo_btn_clearId, dong_porf_porf_todo_btn_clearIdjjtype, 2895, 195, 2989, 195);
  (void) _18.type;
  (void) _18.value;
  const struct ReturnValue _19 = dong_porf_porf_todo_porfPatchIf_clear_wrap(0, 0, 0, 0);
  (void) _19.type;
  (void) _19.value;
  const struct ReturnValue _20 = dong_porf_porf_todo_porfRebuild_todos(0, 0, 0, 0);
  (void) _20.type;
  (void) _20.value;
  const struct ReturnValue _21 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
  (void) _21.type;
  (void) _21.value;
  const struct ReturnValue _22 = dong_porf_porf_todo_dongLog(0, 0, 0, 0, 3006, 195);
  (void) _22.type;
  (void) _22.value;
  _get11 = jjnewtarget;
  // if 
    if (((u32)(_get11)) != 0) {
      _get12 = jjthis;
      _get13 = jjthisjjtype;
      return (struct ReturnValue){ _get12, _get13 };
    }
  // end
  j971:;
  return (struct ReturnValue){ 0, 0 };
}

int dong_porf_porf_todo_main() {
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
  dong_porf_porf_todo_porf_rootId = 0;
  dong_porf_porf_todo_porf_rootIdjjtype = 1;
  dong_porf_porf_todo_titleId = 0;
  dong_porf_porf_todo_titleIdjjtype = 1;
  dong_porf_porf_todo_todo_inputId = 0;
  dong_porf_porf_todo_todo_inputIdjjtype = 1;
  dong_porf_porf_todo_btn_addId = 0;
  dong_porf_porf_todo_btn_addIdjjtype = 1;
  dong_porf_porf_todo_filter_barId = 0;
  dong_porf_porf_todo_filter_barIdjjtype = 1;
  dong_porf_porf_todo_filter_allId = 0;
  dong_porf_porf_todo_filter_allIdjjtype = 1;
  dong_porf_porf_todo_filter_activeId = 0;
  dong_porf_porf_todo_filter_activeIdjjtype = 1;
  dong_porf_porf_todo_filter_doneId = 0;
  dong_porf_porf_todo_filter_doneIdjjtype = 1;
  dong_porf_porf_todo_todo_listId = 0;
  dong_porf_porf_todo_todo_listIdjjtype = 1;
  dong_porf_porf_todo_clear_wrapId = 0;
  dong_porf_porf_todo_clear_wrapIdjjtype = 1;
  dong_porf_porf_todo_btn_clearId = 0;
  dong_porf_porf_todo_btn_clearIdjjtype = 1;
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
  dong_porf_porf_todo_showClear = 0;
  dong_porf_porf_todo_showClearjjtype = 1;
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
  const struct ReturnValue _0 = dong_porf_porf_todo_normalizeInitialTodoText(0, 0, 0, 0);
  jjlast_type = _0.type;
  (void) _0.value;
  const struct ReturnValue _1 = dong_porf_porf_todo_porfInit(0, 0, 0, 0);
  jjlast_type = _1.type;
  _get0 = jjlast_type;

  return 0;
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
      j973:;
    }
  // end
  j972:;
  _get7 = jjreturn;
  _get8 = jjreturnjjtype;
  return (struct ReturnValue){ _get7, _get8 };
}

static struct ReturnValue dong_porf_porf_todo_setTodoSlot(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype, f64 text, i32 textjjtype, f64 done, i32 donejjtype, f64 id, i32 idjjtype) {
  i32 _get360;
  f64 _get359;
  f64 _get358;
  i32 _get357;
  f64 _get356;
  i32 _get355;
  f64 _get354;
  i32 _get353;
  f64 _get352;
  i32 _get351;
  f64 _get350;
  f64 _get349;
  i32 _get348;
  f64 _get347;
  i32 _get346;
  f64 _get345;
  i32 _get344;
  f64 _get343;
  i32 _get342;
  f64 _get341;
  i32 _get340;
  f64 _get339;
  f64 _get338;
  i32 _get337;
  f64 _get336;
  i32 _get335;
  f64 _get334;
  i32 _get333;
  f64 _get332;
  i32 _get331;
  f64 _get330;
  i32 _get329;
  f64 _get328;
  f64 _get327;
  i32 _get326;
  f64 _get325;
  i32 _get324;
  f64 _get323;
  i32 _get322;
  f64 _get321;
  i32 _get320;
  f64 _get319;
  i32 _get318;
  f64 _get317;
  f64 _get316;
  i32 _get315;
  f64 _get314;
  i32 _get313;
  f64 _get312;
  i32 _get311;
  f64 _get310;
  i32 _get309;
  f64 _get308;
  i32 _get307;
  f64 _get306;
  f64 _get305;
  i32 _get304;
  f64 _get303;
  i32 _get302;
  f64 _get301;
  i32 _get300;
  f64 _get299;
  i32 _get298;
  f64 _get297;
  i32 _get296;
  f64 _get295;
  f64 _get294;
  i32 _get293;
  f64 _get292;
  i32 _get291;
  f64 _get290;
  i32 _get289;
  f64 _get288;
  i32 _get287;
  f64 _get286;
  i32 _get285;
  f64 _get284;
  f64 _get283;
  i32 _get282;
  f64 _get281;
  i32 _get280;
  f64 _get279;
  i32 _get278;
  f64 _get277;
  i32 _get276;
  f64 _get275;
  i32 _get274;
  f64 _get273;
  f64 _get272;
  i32 _get271;
  f64 _get270;
  i32 _get269;
  f64 _get268;
  i32 _get267;
  f64 _get266;
  i32 _get265;
  f64 _get264;
  i32 _get263;
  f64 _get262;
  f64 _get261;
  i32 _get260;
  f64 _get259;
  i32 _get258;
  f64 _get257;
  i32 _get256;
  f64 _get255;
  i32 _get254;
  f64 _get253;
  i32 _get252;
  f64 _get251;
  f64 _get250;
  i32 _get249;
  f64 _get248;
  i32 _get247;
  f64 _get246;
  i32 _get245;
  f64 _get244;
  i32 _get243;
  f64 _get242;
  i32 _get241;
  f64 _get240;
  f64 _get239;
  i32 _get238;
  f64 _get237;
  i32 _get236;
  f64 _get235;
  i32 _get234;
  f64 _get233;
  i32 _get232;
  f64 _get231;
  i32 _get230;
  f64 _get229;
  f64 _get228;
  i32 _get227;
  f64 _get226;
  i32 _get225;
  f64 _get224;
  i32 _get223;
  f64 _get222;
  i32 _get221;
  f64 _get220;
  i32 _get219;
  f64 _get218;
  f64 _get217;
  i32 _get216;
  f64 _get215;
  i32 _get214;
  f64 _get213;
  i32 _get212;
  f64 _get211;
  i32 _get210;
  f64 _get209;
  i32 _get208;
  f64 _get207;
  f64 _get206;
  i32 _get205;
  f64 _get204;
  i32 _get203;
  f64 _get202;
  i32 _get201;
  f64 _get200;
  i32 _get199;
  f64 _get198;
  i32 _get197;
  f64 _get196;
  f64 _get195;
  i32 _get194;
  f64 _get193;
  i32 _get192;
  f64 _get191;
  i32 _get190;
  f64 _get189;
  i32 _get188;
  f64 _get187;
  i32 _get186;
  f64 _get185;
  f64 _get184;
  i32 _get183;
  f64 _get182;
  i32 _get181;
  f64 _get180;
  i32 _get179;
  f64 _get178;
  i32 _get177;
  f64 _get176;
  i32 _get175;
  f64 _get174;
  f64 _get173;
  i32 _get172;
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
      _get2 = text;
      dong_porf_porf_todo_todoText0 = _get2;
      _get3 = textjjtype;
      dong_porf_porf_todo_todoText0jjtype = _get3;
      (void) dong_porf_porf_todo_todoText0;
      _get4 = done;
      dong_porf_porf_todo_todoDone0 = _get4;
      _get5 = donejjtype;
      dong_porf_porf_todo_todoDone0jjtype = _get5;
      (void) dong_porf_porf_todo_todoDone0;
      _get6 = id;
      dong_porf_porf_todo_todoId0 = _get6;
      _get7 = idjjtype;
      dong_porf_porf_todo_todoId0jjtype = _get7;
      (void) dong_porf_porf_todo_todoId0;
      _get8 = jjnewtarget;
      // if 
        if (((u32)(_get8)) != 0) {
          _get9 = jjthis;
          _get10 = jjthisjjtype;
          return (struct ReturnValue){ _get9, _get10 };
        }
      // end
      j981:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j980:;
  _get11 = i;
  _get12 = ijjtype;
  // if 
    if ((f64)((_get11 == 1) & ((_get12 | 128) == (1 | 128))) != 0) {
      _get13 = text;
      dong_porf_porf_todo_todoText1 = _get13;
      _get14 = textjjtype;
      dong_porf_porf_todo_todoText1jjtype = _get14;
      (void) dong_porf_porf_todo_todoText1;
      _get15 = done;
      dong_porf_porf_todo_todoDone1 = _get15;
      _get16 = donejjtype;
      dong_porf_porf_todo_todoDone1jjtype = _get16;
      (void) dong_porf_porf_todo_todoDone1;
      _get17 = id;
      dong_porf_porf_todo_todoId1 = _get17;
      _get18 = idjjtype;
      dong_porf_porf_todo_todoId1jjtype = _get18;
      (void) dong_porf_porf_todo_todoId1;
      _get19 = jjnewtarget;
      // if 
        if (((u32)(_get19)) != 0) {
          _get20 = jjthis;
          _get21 = jjthisjjtype;
          return (struct ReturnValue){ _get20, _get21 };
        }
      // end
      j983:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j982:;
  _get22 = i;
  _get23 = ijjtype;
  // if 
    if ((f64)((_get22 == 2) & ((_get23 | 128) == (1 | 128))) != 0) {
      _get24 = text;
      dong_porf_porf_todo_todoText2 = _get24;
      _get25 = textjjtype;
      dong_porf_porf_todo_todoText2jjtype = _get25;
      (void) dong_porf_porf_todo_todoText2;
      _get26 = done;
      dong_porf_porf_todo_todoDone2 = _get26;
      _get27 = donejjtype;
      dong_porf_porf_todo_todoDone2jjtype = _get27;
      (void) dong_porf_porf_todo_todoDone2;
      _get28 = id;
      dong_porf_porf_todo_todoId2 = _get28;
      _get29 = idjjtype;
      dong_porf_porf_todo_todoId2jjtype = _get29;
      (void) dong_porf_porf_todo_todoId2;
      _get30 = jjnewtarget;
      // if 
        if (((u32)(_get30)) != 0) {
          _get31 = jjthis;
          _get32 = jjthisjjtype;
          return (struct ReturnValue){ _get31, _get32 };
        }
      // end
      j985:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j984:;
  _get33 = i;
  _get34 = ijjtype;
  // if 
    if ((f64)((_get33 == 3) & ((_get34 | 128) == (1 | 128))) != 0) {
      _get35 = text;
      dong_porf_porf_todo_todoText3 = _get35;
      _get36 = textjjtype;
      dong_porf_porf_todo_todoText3jjtype = _get36;
      (void) dong_porf_porf_todo_todoText3;
      _get37 = done;
      dong_porf_porf_todo_todoDone3 = _get37;
      _get38 = donejjtype;
      dong_porf_porf_todo_todoDone3jjtype = _get38;
      (void) dong_porf_porf_todo_todoDone3;
      _get39 = id;
      dong_porf_porf_todo_todoId3 = _get39;
      _get40 = idjjtype;
      dong_porf_porf_todo_todoId3jjtype = _get40;
      (void) dong_porf_porf_todo_todoId3;
      _get41 = jjnewtarget;
      // if 
        if (((u32)(_get41)) != 0) {
          _get42 = jjthis;
          _get43 = jjthisjjtype;
          return (struct ReturnValue){ _get42, _get43 };
        }
      // end
      j987:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j986:;
  _get44 = i;
  _get45 = ijjtype;
  // if 
    if ((f64)((_get44 == 4) & ((_get45 | 128) == (1 | 128))) != 0) {
      _get46 = text;
      dong_porf_porf_todo_todoText4 = _get46;
      _get47 = textjjtype;
      dong_porf_porf_todo_todoText4jjtype = _get47;
      (void) dong_porf_porf_todo_todoText4;
      _get48 = done;
      dong_porf_porf_todo_todoDone4 = _get48;
      _get49 = donejjtype;
      dong_porf_porf_todo_todoDone4jjtype = _get49;
      (void) dong_porf_porf_todo_todoDone4;
      _get50 = id;
      dong_porf_porf_todo_todoId4 = _get50;
      _get51 = idjjtype;
      dong_porf_porf_todo_todoId4jjtype = _get51;
      (void) dong_porf_porf_todo_todoId4;
      _get52 = jjnewtarget;
      // if 
        if (((u32)(_get52)) != 0) {
          _get53 = jjthis;
          _get54 = jjthisjjtype;
          return (struct ReturnValue){ _get53, _get54 };
        }
      // end
      j989:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j988:;
  _get55 = i;
  _get56 = ijjtype;
  // if 
    if ((f64)((_get55 == 5) & ((_get56 | 128) == (1 | 128))) != 0) {
      _get57 = text;
      dong_porf_porf_todo_todoText5 = _get57;
      _get58 = textjjtype;
      dong_porf_porf_todo_todoText5jjtype = _get58;
      (void) dong_porf_porf_todo_todoText5;
      _get59 = done;
      dong_porf_porf_todo_todoDone5 = _get59;
      _get60 = donejjtype;
      dong_porf_porf_todo_todoDone5jjtype = _get60;
      (void) dong_porf_porf_todo_todoDone5;
      _get61 = id;
      dong_porf_porf_todo_todoId5 = _get61;
      _get62 = idjjtype;
      dong_porf_porf_todo_todoId5jjtype = _get62;
      (void) dong_porf_porf_todo_todoId5;
      _get63 = jjnewtarget;
      // if 
        if (((u32)(_get63)) != 0) {
          _get64 = jjthis;
          _get65 = jjthisjjtype;
          return (struct ReturnValue){ _get64, _get65 };
        }
      // end
      j991:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j990:;
  _get66 = i;
  _get67 = ijjtype;
  // if 
    if ((f64)((_get66 == 6) & ((_get67 | 128) == (1 | 128))) != 0) {
      _get68 = text;
      dong_porf_porf_todo_todoText6 = _get68;
      _get69 = textjjtype;
      dong_porf_porf_todo_todoText6jjtype = _get69;
      (void) dong_porf_porf_todo_todoText6;
      _get70 = done;
      dong_porf_porf_todo_todoDone6 = _get70;
      _get71 = donejjtype;
      dong_porf_porf_todo_todoDone6jjtype = _get71;
      (void) dong_porf_porf_todo_todoDone6;
      _get72 = id;
      dong_porf_porf_todo_todoId6 = _get72;
      _get73 = idjjtype;
      dong_porf_porf_todo_todoId6jjtype = _get73;
      (void) dong_porf_porf_todo_todoId6;
      _get74 = jjnewtarget;
      // if 
        if (((u32)(_get74)) != 0) {
          _get75 = jjthis;
          _get76 = jjthisjjtype;
          return (struct ReturnValue){ _get75, _get76 };
        }
      // end
      j993:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j992:;
  _get77 = i;
  _get78 = ijjtype;
  // if 
    if ((f64)((_get77 == 7) & ((_get78 | 128) == (1 | 128))) != 0) {
      _get79 = text;
      dong_porf_porf_todo_todoText7 = _get79;
      _get80 = textjjtype;
      dong_porf_porf_todo_todoText7jjtype = _get80;
      (void) dong_porf_porf_todo_todoText7;
      _get81 = done;
      dong_porf_porf_todo_todoDone7 = _get81;
      _get82 = donejjtype;
      dong_porf_porf_todo_todoDone7jjtype = _get82;
      (void) dong_porf_porf_todo_todoDone7;
      _get83 = id;
      dong_porf_porf_todo_todoId7 = _get83;
      _get84 = idjjtype;
      dong_porf_porf_todo_todoId7jjtype = _get84;
      (void) dong_porf_porf_todo_todoId7;
      _get85 = jjnewtarget;
      // if 
        if (((u32)(_get85)) != 0) {
          _get86 = jjthis;
          _get87 = jjthisjjtype;
          return (struct ReturnValue){ _get86, _get87 };
        }
      // end
      j995:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j994:;
  _get88 = i;
  _get89 = ijjtype;
  // if 
    if ((f64)((_get88 == 8) & ((_get89 | 128) == (1 | 128))) != 0) {
      _get90 = text;
      dong_porf_porf_todo_todoText8 = _get90;
      _get91 = textjjtype;
      dong_porf_porf_todo_todoText8jjtype = _get91;
      (void) dong_porf_porf_todo_todoText8;
      _get92 = done;
      dong_porf_porf_todo_todoDone8 = _get92;
      _get93 = donejjtype;
      dong_porf_porf_todo_todoDone8jjtype = _get93;
      (void) dong_porf_porf_todo_todoDone8;
      _get94 = id;
      dong_porf_porf_todo_todoId8 = _get94;
      _get95 = idjjtype;
      dong_porf_porf_todo_todoId8jjtype = _get95;
      (void) dong_porf_porf_todo_todoId8;
      _get96 = jjnewtarget;
      // if 
        if (((u32)(_get96)) != 0) {
          _get97 = jjthis;
          _get98 = jjthisjjtype;
          return (struct ReturnValue){ _get97, _get98 };
        }
      // end
      j997:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j996:;
  _get99 = i;
  _get100 = ijjtype;
  // if 
    if ((f64)((_get99 == 9) & ((_get100 | 128) == (1 | 128))) != 0) {
      _get101 = text;
      dong_porf_porf_todo_todoText9 = _get101;
      _get102 = textjjtype;
      dong_porf_porf_todo_todoText9jjtype = _get102;
      (void) dong_porf_porf_todo_todoText9;
      _get103 = done;
      dong_porf_porf_todo_todoDone9 = _get103;
      _get104 = donejjtype;
      dong_porf_porf_todo_todoDone9jjtype = _get104;
      (void) dong_porf_porf_todo_todoDone9;
      _get105 = id;
      dong_porf_porf_todo_todoId9 = _get105;
      _get106 = idjjtype;
      dong_porf_porf_todo_todoId9jjtype = _get106;
      (void) dong_porf_porf_todo_todoId9;
      _get107 = jjnewtarget;
      // if 
        if (((u32)(_get107)) != 0) {
          _get108 = jjthis;
          _get109 = jjthisjjtype;
          return (struct ReturnValue){ _get108, _get109 };
        }
      // end
      j999:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j998:;
  _get110 = i;
  _get111 = ijjtype;
  // if 
    if ((f64)((_get110 == 10) & ((_get111 | 128) == (1 | 128))) != 0) {
      _get112 = text;
      dong_porf_porf_todo_todoText10 = _get112;
      _get113 = textjjtype;
      dong_porf_porf_todo_todoText10jjtype = _get113;
      (void) dong_porf_porf_todo_todoText10;
      _get114 = done;
      dong_porf_porf_todo_todoDone10 = _get114;
      _get115 = donejjtype;
      dong_porf_porf_todo_todoDone10jjtype = _get115;
      (void) dong_porf_porf_todo_todoDone10;
      _get116 = id;
      dong_porf_porf_todo_todoId10 = _get116;
      _get117 = idjjtype;
      dong_porf_porf_todo_todoId10jjtype = _get117;
      (void) dong_porf_porf_todo_todoId10;
      _get118 = jjnewtarget;
      // if 
        if (((u32)(_get118)) != 0) {
          _get119 = jjthis;
          _get120 = jjthisjjtype;
          return (struct ReturnValue){ _get119, _get120 };
        }
      // end
      j1001:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1000:;
  _get121 = i;
  _get122 = ijjtype;
  // if 
    if ((f64)((_get121 == 11) & ((_get122 | 128) == (1 | 128))) != 0) {
      _get123 = text;
      dong_porf_porf_todo_todoText11 = _get123;
      _get124 = textjjtype;
      dong_porf_porf_todo_todoText11jjtype = _get124;
      (void) dong_porf_porf_todo_todoText11;
      _get125 = done;
      dong_porf_porf_todo_todoDone11 = _get125;
      _get126 = donejjtype;
      dong_porf_porf_todo_todoDone11jjtype = _get126;
      (void) dong_porf_porf_todo_todoDone11;
      _get127 = id;
      dong_porf_porf_todo_todoId11 = _get127;
      _get128 = idjjtype;
      dong_porf_porf_todo_todoId11jjtype = _get128;
      (void) dong_porf_porf_todo_todoId11;
      _get129 = jjnewtarget;
      // if 
        if (((u32)(_get129)) != 0) {
          _get130 = jjthis;
          _get131 = jjthisjjtype;
          return (struct ReturnValue){ _get130, _get131 };
        }
      // end
      j1003:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1002:;
  _get132 = i;
  _get133 = ijjtype;
  // if 
    if ((f64)((_get132 == 12) & ((_get133 | 128) == (1 | 128))) != 0) {
      _get134 = text;
      dong_porf_porf_todo_todoText12 = _get134;
      _get135 = textjjtype;
      dong_porf_porf_todo_todoText12jjtype = _get135;
      (void) dong_porf_porf_todo_todoText12;
      _get136 = done;
      dong_porf_porf_todo_todoDone12 = _get136;
      _get137 = donejjtype;
      dong_porf_porf_todo_todoDone12jjtype = _get137;
      (void) dong_porf_porf_todo_todoDone12;
      _get138 = id;
      dong_porf_porf_todo_todoId12 = _get138;
      _get139 = idjjtype;
      dong_porf_porf_todo_todoId12jjtype = _get139;
      (void) dong_porf_porf_todo_todoId12;
      _get140 = jjnewtarget;
      // if 
        if (((u32)(_get140)) != 0) {
          _get141 = jjthis;
          _get142 = jjthisjjtype;
          return (struct ReturnValue){ _get141, _get142 };
        }
      // end
      j1005:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1004:;
  _get143 = i;
  _get144 = ijjtype;
  // if 
    if ((f64)((_get143 == 13) & ((_get144 | 128) == (1 | 128))) != 0) {
      _get145 = text;
      dong_porf_porf_todo_todoText13 = _get145;
      _get146 = textjjtype;
      dong_porf_porf_todo_todoText13jjtype = _get146;
      (void) dong_porf_porf_todo_todoText13;
      _get147 = done;
      dong_porf_porf_todo_todoDone13 = _get147;
      _get148 = donejjtype;
      dong_porf_porf_todo_todoDone13jjtype = _get148;
      (void) dong_porf_porf_todo_todoDone13;
      _get149 = id;
      dong_porf_porf_todo_todoId13 = _get149;
      _get150 = idjjtype;
      dong_porf_porf_todo_todoId13jjtype = _get150;
      (void) dong_porf_porf_todo_todoId13;
      _get151 = jjnewtarget;
      // if 
        if (((u32)(_get151)) != 0) {
          _get152 = jjthis;
          _get153 = jjthisjjtype;
          return (struct ReturnValue){ _get152, _get153 };
        }
      // end
      j1007:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1006:;
  _get154 = i;
  _get155 = ijjtype;
  // if 
    if ((f64)((_get154 == 14) & ((_get155 | 128) == (1 | 128))) != 0) {
      _get156 = text;
      dong_porf_porf_todo_todoText14 = _get156;
      _get157 = textjjtype;
      dong_porf_porf_todo_todoText14jjtype = _get157;
      (void) dong_porf_porf_todo_todoText14;
      _get158 = done;
      dong_porf_porf_todo_todoDone14 = _get158;
      _get159 = donejjtype;
      dong_porf_porf_todo_todoDone14jjtype = _get159;
      (void) dong_porf_porf_todo_todoDone14;
      _get160 = id;
      dong_porf_porf_todo_todoId14 = _get160;
      _get161 = idjjtype;
      dong_porf_porf_todo_todoId14jjtype = _get161;
      (void) dong_porf_porf_todo_todoId14;
      _get162 = jjnewtarget;
      // if 
        if (((u32)(_get162)) != 0) {
          _get163 = jjthis;
          _get164 = jjthisjjtype;
          return (struct ReturnValue){ _get163, _get164 };
        }
      // end
      j1009:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1008:;
  _get165 = i;
  _get166 = ijjtype;
  // if 
    if ((f64)((_get165 == 15) & ((_get166 | 128) == (1 | 128))) != 0) {
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
      _get171 = id;
      dong_porf_porf_todo_todoId15 = _get171;
      _get172 = idjjtype;
      dong_porf_porf_todo_todoId15jjtype = _get172;
      (void) dong_porf_porf_todo_todoId15;
      _get173 = jjnewtarget;
      // if 
        if (((u32)(_get173)) != 0) {
          _get174 = jjthis;
          _get175 = jjthisjjtype;
          return (struct ReturnValue){ _get174, _get175 };
        }
      // end
      j1011:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1010:;
  _get176 = i;
  _get177 = ijjtype;
  // if 
    if ((f64)((_get176 == 16) & ((_get177 | 128) == (1 | 128))) != 0) {
      _get178 = text;
      dong_porf_porf_todo_todoText16 = _get178;
      _get179 = textjjtype;
      dong_porf_porf_todo_todoText16jjtype = _get179;
      _get180 = done;
      dong_porf_porf_todo_todoDone16 = _get180;
      _get181 = donejjtype;
      dong_porf_porf_todo_todoDone16jjtype = _get181;
      _get182 = id;
      dong_porf_porf_todo_todoId16 = _get182;
      _get183 = idjjtype;
      dong_porf_porf_todo_todoId16jjtype = _get183;
      _get184 = jjnewtarget;
      // if 
        if (((u32)(_get184)) != 0) {
          _get185 = jjthis;
          _get186 = jjthisjjtype;
          return (struct ReturnValue){ _get185, _get186 };
        }
      // end
      j1013:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1012:;
  _get187 = i;
  _get188 = ijjtype;
  // if 
    if ((f64)((_get187 == 17) & ((_get188 | 128) == (1 | 128))) != 0) {
      _get189 = text;
      dong_porf_porf_todo_todoText17 = _get189;
      _get190 = textjjtype;
      dong_porf_porf_todo_todoText17jjtype = _get190;
      _get191 = done;
      dong_porf_porf_todo_todoDone17 = _get191;
      _get192 = donejjtype;
      dong_porf_porf_todo_todoDone17jjtype = _get192;
      _get193 = id;
      dong_porf_porf_todo_todoId17 = _get193;
      _get194 = idjjtype;
      dong_porf_porf_todo_todoId17jjtype = _get194;
      _get195 = jjnewtarget;
      // if 
        if (((u32)(_get195)) != 0) {
          _get196 = jjthis;
          _get197 = jjthisjjtype;
          return (struct ReturnValue){ _get196, _get197 };
        }
      // end
      j1015:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1014:;
  _get198 = i;
  _get199 = ijjtype;
  // if 
    if ((f64)((_get198 == 18) & ((_get199 | 128) == (1 | 128))) != 0) {
      _get200 = text;
      dong_porf_porf_todo_todoText18 = _get200;
      _get201 = textjjtype;
      dong_porf_porf_todo_todoText18jjtype = _get201;
      _get202 = done;
      dong_porf_porf_todo_todoDone18 = _get202;
      _get203 = donejjtype;
      dong_porf_porf_todo_todoDone18jjtype = _get203;
      _get204 = id;
      dong_porf_porf_todo_todoId18 = _get204;
      _get205 = idjjtype;
      dong_porf_porf_todo_todoId18jjtype = _get205;
      _get206 = jjnewtarget;
      // if 
        if (((u32)(_get206)) != 0) {
          _get207 = jjthis;
          _get208 = jjthisjjtype;
          return (struct ReturnValue){ _get207, _get208 };
        }
      // end
      j1017:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1016:;
  _get209 = i;
  _get210 = ijjtype;
  // if 
    if ((f64)((_get209 == 19) & ((_get210 | 128) == (1 | 128))) != 0) {
      _get211 = text;
      dong_porf_porf_todo_todoText19 = _get211;
      _get212 = textjjtype;
      dong_porf_porf_todo_todoText19jjtype = _get212;
      _get213 = done;
      dong_porf_porf_todo_todoDone19 = _get213;
      _get214 = donejjtype;
      dong_porf_porf_todo_todoDone19jjtype = _get214;
      _get215 = id;
      dong_porf_porf_todo_todoId19 = _get215;
      _get216 = idjjtype;
      dong_porf_porf_todo_todoId19jjtype = _get216;
      _get217 = jjnewtarget;
      // if 
        if (((u32)(_get217)) != 0) {
          _get218 = jjthis;
          _get219 = jjthisjjtype;
          return (struct ReturnValue){ _get218, _get219 };
        }
      // end
      j1019:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1018:;
  _get220 = i;
  _get221 = ijjtype;
  // if 
    if ((f64)((_get220 == 20) & ((_get221 | 128) == (1 | 128))) != 0) {
      _get222 = text;
      dong_porf_porf_todo_todoText20 = _get222;
      _get223 = textjjtype;
      dong_porf_porf_todo_todoText20jjtype = _get223;
      _get224 = done;
      dong_porf_porf_todo_todoDone20 = _get224;
      _get225 = donejjtype;
      dong_porf_porf_todo_todoDone20jjtype = _get225;
      _get226 = id;
      dong_porf_porf_todo_todoId20 = _get226;
      _get227 = idjjtype;
      dong_porf_porf_todo_todoId20jjtype = _get227;
      _get228 = jjnewtarget;
      // if 
        if (((u32)(_get228)) != 0) {
          _get229 = jjthis;
          _get230 = jjthisjjtype;
          return (struct ReturnValue){ _get229, _get230 };
        }
      // end
      j1021:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1020:;
  _get231 = i;
  _get232 = ijjtype;
  // if 
    if ((f64)((_get231 == 21) & ((_get232 | 128) == (1 | 128))) != 0) {
      _get233 = text;
      dong_porf_porf_todo_todoText21 = _get233;
      _get234 = textjjtype;
      dong_porf_porf_todo_todoText21jjtype = _get234;
      _get235 = done;
      dong_porf_porf_todo_todoDone21 = _get235;
      _get236 = donejjtype;
      dong_porf_porf_todo_todoDone21jjtype = _get236;
      _get237 = id;
      dong_porf_porf_todo_todoId21 = _get237;
      _get238 = idjjtype;
      dong_porf_porf_todo_todoId21jjtype = _get238;
      _get239 = jjnewtarget;
      // if 
        if (((u32)(_get239)) != 0) {
          _get240 = jjthis;
          _get241 = jjthisjjtype;
          return (struct ReturnValue){ _get240, _get241 };
        }
      // end
      j1023:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1022:;
  _get242 = i;
  _get243 = ijjtype;
  // if 
    if ((f64)((_get242 == 22) & ((_get243 | 128) == (1 | 128))) != 0) {
      _get244 = text;
      dong_porf_porf_todo_todoText22 = _get244;
      _get245 = textjjtype;
      dong_porf_porf_todo_todoText22jjtype = _get245;
      _get246 = done;
      dong_porf_porf_todo_todoDone22 = _get246;
      _get247 = donejjtype;
      dong_porf_porf_todo_todoDone22jjtype = _get247;
      _get248 = id;
      dong_porf_porf_todo_todoId22 = _get248;
      _get249 = idjjtype;
      dong_porf_porf_todo_todoId22jjtype = _get249;
      _get250 = jjnewtarget;
      // if 
        if (((u32)(_get250)) != 0) {
          _get251 = jjthis;
          _get252 = jjthisjjtype;
          return (struct ReturnValue){ _get251, _get252 };
        }
      // end
      j1025:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1024:;
  _get253 = i;
  _get254 = ijjtype;
  // if 
    if ((f64)((_get253 == 23) & ((_get254 | 128) == (1 | 128))) != 0) {
      _get255 = text;
      dong_porf_porf_todo_todoText23 = _get255;
      _get256 = textjjtype;
      dong_porf_porf_todo_todoText23jjtype = _get256;
      _get257 = done;
      dong_porf_porf_todo_todoDone23 = _get257;
      _get258 = donejjtype;
      dong_porf_porf_todo_todoDone23jjtype = _get258;
      _get259 = id;
      dong_porf_porf_todo_todoId23 = _get259;
      _get260 = idjjtype;
      dong_porf_porf_todo_todoId23jjtype = _get260;
      _get261 = jjnewtarget;
      // if 
        if (((u32)(_get261)) != 0) {
          _get262 = jjthis;
          _get263 = jjthisjjtype;
          return (struct ReturnValue){ _get262, _get263 };
        }
      // end
      j1027:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1026:;
  _get264 = i;
  _get265 = ijjtype;
  // if 
    if ((f64)((_get264 == 24) & ((_get265 | 128) == (1 | 128))) != 0) {
      _get266 = text;
      dong_porf_porf_todo_todoText24 = _get266;
      _get267 = textjjtype;
      dong_porf_porf_todo_todoText24jjtype = _get267;
      _get268 = done;
      dong_porf_porf_todo_todoDone24 = _get268;
      _get269 = donejjtype;
      dong_porf_porf_todo_todoDone24jjtype = _get269;
      _get270 = id;
      dong_porf_porf_todo_todoId24 = _get270;
      _get271 = idjjtype;
      dong_porf_porf_todo_todoId24jjtype = _get271;
      _get272 = jjnewtarget;
      // if 
        if (((u32)(_get272)) != 0) {
          _get273 = jjthis;
          _get274 = jjthisjjtype;
          return (struct ReturnValue){ _get273, _get274 };
        }
      // end
      j1029:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1028:;
  _get275 = i;
  _get276 = ijjtype;
  // if 
    if ((f64)((_get275 == 25) & ((_get276 | 128) == (1 | 128))) != 0) {
      _get277 = text;
      dong_porf_porf_todo_todoText25 = _get277;
      _get278 = textjjtype;
      dong_porf_porf_todo_todoText25jjtype = _get278;
      _get279 = done;
      dong_porf_porf_todo_todoDone25 = _get279;
      _get280 = donejjtype;
      dong_porf_porf_todo_todoDone25jjtype = _get280;
      _get281 = id;
      dong_porf_porf_todo_todoId25 = _get281;
      _get282 = idjjtype;
      dong_porf_porf_todo_todoId25jjtype = _get282;
      _get283 = jjnewtarget;
      // if 
        if (((u32)(_get283)) != 0) {
          _get284 = jjthis;
          _get285 = jjthisjjtype;
          return (struct ReturnValue){ _get284, _get285 };
        }
      // end
      j1031:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1030:;
  _get286 = i;
  _get287 = ijjtype;
  // if 
    if ((f64)((_get286 == 26) & ((_get287 | 128) == (1 | 128))) != 0) {
      _get288 = text;
      dong_porf_porf_todo_todoText26 = _get288;
      _get289 = textjjtype;
      dong_porf_porf_todo_todoText26jjtype = _get289;
      _get290 = done;
      dong_porf_porf_todo_todoDone26 = _get290;
      _get291 = donejjtype;
      dong_porf_porf_todo_todoDone26jjtype = _get291;
      _get292 = id;
      dong_porf_porf_todo_todoId26 = _get292;
      _get293 = idjjtype;
      dong_porf_porf_todo_todoId26jjtype = _get293;
      _get294 = jjnewtarget;
      // if 
        if (((u32)(_get294)) != 0) {
          _get295 = jjthis;
          _get296 = jjthisjjtype;
          return (struct ReturnValue){ _get295, _get296 };
        }
      // end
      j1033:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1032:;
  _get297 = i;
  _get298 = ijjtype;
  // if 
    if ((f64)((_get297 == 27) & ((_get298 | 128) == (1 | 128))) != 0) {
      _get299 = text;
      dong_porf_porf_todo_todoText27 = _get299;
      _get300 = textjjtype;
      dong_porf_porf_todo_todoText27jjtype = _get300;
      _get301 = done;
      dong_porf_porf_todo_todoDone27 = _get301;
      _get302 = donejjtype;
      dong_porf_porf_todo_todoDone27jjtype = _get302;
      _get303 = id;
      dong_porf_porf_todo_todoId27 = _get303;
      _get304 = idjjtype;
      dong_porf_porf_todo_todoId27jjtype = _get304;
      _get305 = jjnewtarget;
      // if 
        if (((u32)(_get305)) != 0) {
          _get306 = jjthis;
          _get307 = jjthisjjtype;
          return (struct ReturnValue){ _get306, _get307 };
        }
      // end
      j1035:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1034:;
  _get308 = i;
  _get309 = ijjtype;
  // if 
    if ((f64)((_get308 == 28) & ((_get309 | 128) == (1 | 128))) != 0) {
      _get310 = text;
      dong_porf_porf_todo_todoText28 = _get310;
      _get311 = textjjtype;
      dong_porf_porf_todo_todoText28jjtype = _get311;
      _get312 = done;
      dong_porf_porf_todo_todoDone28 = _get312;
      _get313 = donejjtype;
      dong_porf_porf_todo_todoDone28jjtype = _get313;
      _get314 = id;
      dong_porf_porf_todo_todoId28 = _get314;
      _get315 = idjjtype;
      dong_porf_porf_todo_todoId28jjtype = _get315;
      _get316 = jjnewtarget;
      // if 
        if (((u32)(_get316)) != 0) {
          _get317 = jjthis;
          _get318 = jjthisjjtype;
          return (struct ReturnValue){ _get317, _get318 };
        }
      // end
      j1037:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1036:;
  _get319 = i;
  _get320 = ijjtype;
  // if 
    if ((f64)((_get319 == 29) & ((_get320 | 128) == (1 | 128))) != 0) {
      _get321 = text;
      dong_porf_porf_todo_todoText29 = _get321;
      _get322 = textjjtype;
      dong_porf_porf_todo_todoText29jjtype = _get322;
      _get323 = done;
      dong_porf_porf_todo_todoDone29 = _get323;
      _get324 = donejjtype;
      dong_porf_porf_todo_todoDone29jjtype = _get324;
      _get325 = id;
      dong_porf_porf_todo_todoId29 = _get325;
      _get326 = idjjtype;
      dong_porf_porf_todo_todoId29jjtype = _get326;
      _get327 = jjnewtarget;
      // if 
        if (((u32)(_get327)) != 0) {
          _get328 = jjthis;
          _get329 = jjthisjjtype;
          return (struct ReturnValue){ _get328, _get329 };
        }
      // end
      j1039:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1038:;
  _get330 = i;
  _get331 = ijjtype;
  // if 
    if ((f64)((_get330 == 30) & ((_get331 | 128) == (1 | 128))) != 0) {
      _get332 = text;
      dong_porf_porf_todo_todoText30 = _get332;
      _get333 = textjjtype;
      dong_porf_porf_todo_todoText30jjtype = _get333;
      _get334 = done;
      dong_porf_porf_todo_todoDone30 = _get334;
      _get335 = donejjtype;
      dong_porf_porf_todo_todoDone30jjtype = _get335;
      _get336 = id;
      dong_porf_porf_todo_todoId30 = _get336;
      _get337 = idjjtype;
      dong_porf_porf_todo_todoId30jjtype = _get337;
      _get338 = jjnewtarget;
      // if 
        if (((u32)(_get338)) != 0) {
          _get339 = jjthis;
          _get340 = jjthisjjtype;
          return (struct ReturnValue){ _get339, _get340 };
        }
      // end
      j1041:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1040:;
  _get341 = i;
  _get342 = ijjtype;
  // if 
    if ((f64)((_get341 == 31) & ((_get342 | 128) == (1 | 128))) != 0) {
      _get343 = text;
      dong_porf_porf_todo_todoText31 = _get343;
      _get344 = textjjtype;
      dong_porf_porf_todo_todoText31jjtype = _get344;
      _get345 = done;
      dong_porf_porf_todo_todoDone31 = _get345;
      _get346 = donejjtype;
      dong_porf_porf_todo_todoDone31jjtype = _get346;
      _get347 = id;
      dong_porf_porf_todo_todoId31 = _get347;
      _get348 = idjjtype;
      dong_porf_porf_todo_todoId31jjtype = _get348;
      _get349 = jjnewtarget;
      // if 
        if (((u32)(_get349)) != 0) {
          _get350 = jjthis;
          _get351 = jjthisjjtype;
          return (struct ReturnValue){ _get350, _get351 };
        }
      // end
      j1043:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1042:;
  _get352 = text;
  dong_porf_porf_todo_todoText31 = _get352;
  _get353 = textjjtype;
  dong_porf_porf_todo_todoText31jjtype = _get353;
  (void) dong_porf_porf_todo_todoText31;
  _get354 = done;
  dong_porf_porf_todo_todoDone31 = _get354;
  _get355 = donejjtype;
  dong_porf_porf_todo_todoDone31jjtype = _get355;
  (void) dong_porf_porf_todo_todoDone31;
  _get356 = id;
  dong_porf_porf_todo_todoId31 = _get356;
  _get357 = idjjtype;
  dong_porf_porf_todo_todoId31jjtype = _get357;
  (void) dong_porf_porf_todo_todoId31;
  _get358 = jjnewtarget;
  // if 
    if (((u32)(_get358)) != 0) {
      _get359 = jjthis;
      _get360 = jjthisjjtype;
      return (struct ReturnValue){ _get359, _get360 };
    }
  // end
  j1044:;
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
  j1045:;
  return (struct ReturnValue){ 0, 0 };
}

struct ReturnValue dong_porf_porf_todo_onAdd(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get21;
  f64 _get20;
  f64 _get19;
  i32 _get18;
  f64 _get17;
  i32 _get16;
  f64 _get15;
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
  i32 _get3;
  f64 _get2;
  i32 _get1;
  i32 _get0;
  f64 text = 0;
  i32 textjjtype = 0;
  i32 jjlast_type = 0;
  i32 jjlength_tmp = 0;
  f64 jjmember_obj_155 = 0;
  f64 jjmember_prop_155 = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_getValue(0, 0, 0, 0, dong_porf_porf_todo_todo_inputId, 1);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  const struct ReturnValue _1 = dong_porf_porf_todo_persistStr(0, 0, 0, 0, _0.value, _get0);
  jjlast_type = _1.type;
  _get1 = jjlast_type;
  textjjtype = _get1;
  text = _1.value;
  _get2 = text;
  jjlength_tmp = (u32)(_get2);
  _get3 = textjjtype;
  // if f64
  f64 _r974;
    if ((_get3 & 64) != 0) {
      _get4 = jjlength_tmp;
      jjlast_type = 1;
      _r974 = (f64)(i32_load(1, 0, _get4));
    } else {
      jjmember_prop_155 = 564;
      _get5 = text;
      jjmember_obj_155 = _get5;
      _get6 = textjjtype;
      // if f64
      f64 _r975;
        if (_get6 == 0) {
          _r975 = 0;
        } else {
          _get7 = jjmember_obj_155;
          _get8 = textjjtype;
          _get9 = jjmember_prop_155;
          const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get7), _get8, (u32)(_get9), 195, -2086110260, 1);
          jjlast_type = _2.type;
          _r975 = _2.value;
        }
      // end
      j975:;
      _r974 = _r975;
    }
  // end
  j974:;
  _get10 = jjlast_type;
  // if 
    if ((f64)((_r974 == 0) & ((_get10 | 128) == (1 | 128))) != 0) {
      _get11 = jjnewtarget;
      // if 
        if (((u32)(_get11)) != 0) {
          _get12 = jjthis;
          _get13 = jjthisjjtype;
          return (struct ReturnValue){ _get12, _get13 };
        }
      // end
      j977:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j976:;
  // if 
    if ((f64)(dong_porf_porf_todo_todoCount >= dong_porf_porf_todo_MAX_TODOS) != 0) {
      const struct ReturnValue _3 = dong_porf_porf_todo_dongLog(0, 0, 0, 0, 1933, 195);
      jjlast_type = _3.type;
      (void) _3.value;
      _get14 = jjnewtarget;
      // if 
        if (((u32)(_get14)) != 0) {
          _get15 = jjthis;
          _get16 = jjthisjjtype;
          return (struct ReturnValue){ _get15, _get16 };
        }
      // end
      j979:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j978:;
  _get17 = text;
  _get18 = textjjtype;
  const struct ReturnValue _4 = dong_porf_porf_todo_setTodoSlot(0, 0, 0, 0, dong_porf_porf_todo_todoCount, 1, _get17, _get18, 0, 1, dong_porf_porf_todo_nextId, 1);
  jjlast_type = _4.type;
  (void) _4.value;
  dong_porf_porf_todo_todoCount = dong_porf_porf_todo_todoCount + 1;
  dong_porf_porf_todo_todoCountjjtype = 1;
  (void) dong_porf_porf_todo_todoCount;
  dong_porf_porf_todo_nextId = dong_porf_porf_todo_nextId + 1;
  dong_porf_porf_todo_nextIdjjtype = 1;
  (void) dong_porf_porf_todo_nextId;
  const struct ReturnValue _5 = dong_porf_porf_todo_setValue(0, 0, 0, 0, dong_porf_porf_todo_todo_inputId, 1, 0, 195);
  jjlast_type = _5.type;
  (void) _5.value;
  const struct ReturnValue _6 = dong_porf_porf_todo_porfRefresh(0, 0, 0, 0);
  jjlast_type = _6.type;
  (void) _6.value;
  _get19 = jjnewtarget;
  // if 
    if (((u32)(_get19)) != 0) {
      _get20 = jjthis;
      _get21 = jjthisjjtype;
      return (struct ReturnValue){ _get20, _get21 };
    }
  // end
  j1046:;
  return (struct ReturnValue){ 0, 0 };
}

struct ReturnValue dong_porf_porf_todo_onInputChange(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  i32 _get0;
  i32 jjlast_type = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_getValue(0, 0, 0, 0, dong_porf_porf_todo_todo_inputId, 1);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  const struct ReturnValue _1 = dong_porf_porf_todo_persistStr(0, 0, 0, 0, _0.value, _get0);
  jjlast_type = _1.type;
  _get1 = jjlast_type;
  dong_porf_porf_todo_inputTextjjtype = _get1;
  dong_porf_porf_todo_inputText = _1.value;
  _get2 = jjnewtarget;
  // if 
    if (((u32)(_get2)) != 0) {
      _get3 = jjthis;
      _get4 = jjthisjjtype;
      return (struct ReturnValue){ _get3, _get4 };
    }
  // end
  j1047:;
  return (struct ReturnValue){ 0, 0 };
}

struct ReturnValue dong_porf_porf_todo_headlessAddSample(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get2;
  f64 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  const struct ReturnValue _0 = dong_porf_porf_todo_setValue(0, 0, 0, 0, dong_porf_porf_todo_todo_inputId, 1, 2413, 195);
  (void) _0.type;
  (void) _0.value;
  const struct ReturnValue _1 = dong_porf_porf_todo_onAdd(0, 0, 0, 0);
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
  j1048:;
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
      j1050:;
    }
  // end
  j1049:;
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
      i32 _r1052;
        _get3 = jjtypeswitch_tmp1;
        // if 
          if (_get3 == 0) {
            _r1052 = 1;
            goto j1052;
          }
        // end
        j1053:;
        _get4 = jjtypeswitch_tmp1;
        // if 
          if (_get4 == 7) {
            _get5 = jjlogicinner_tmp;
            _r1052 = _get5 == 0;
            goto j1052;
          }
        // end
        j1054:;
        _r1052 = 0;
      // end
      j1052:;
      _get6 = ajjtype;
      _get7 = ajjtype;
      // if 
        if (((_r1052 | ((f64)(_get6) == 5)) | ((f64)(_get7) == 2)) != 0) {
          return 0;
        }
      // end
      j1055:;
      _get8 = a;
      _get9 = ajjtype;
      const struct ReturnValue _0 = dong_porf_porf_todo__ecma262_ToString(_get8, _get9);
      jjlast_type = _0.type;
      _get10 = jjlast_type;
      ajjtype = _get10;
      a = _0.value;
    }
  // end
  j1051:;
  _get11 = bjjtype;
  // if 
    if ((f64)(_get11 | 128) != 195) {
      _get12 = b;
      jjlogicinner_tmp = _get12;
      _get13 = bjjtype;
      jjtypeswitch_tmp1 = _get13;
      // block i32
      i32 _r1057;
        _get14 = jjtypeswitch_tmp1;
        // if 
          if (_get14 == 0) {
            _r1057 = 1;
            goto j1057;
          }
        // end
        j1058:;
        _get15 = jjtypeswitch_tmp1;
        // if 
          if (_get15 == 7) {
            _get16 = jjlogicinner_tmp;
            _r1057 = _get16 == 0;
            goto j1057;
          }
        // end
        j1059:;
        _r1057 = 0;
      // end
      j1057:;
      _get17 = bjjtype;
      _get18 = bjjtype;
      // if 
        if (((_r1057 | ((f64)(_get17) == 5)) | ((f64)(_get18) == 2)) != 0) {
          return 0;
        }
      // end
      j1060:;
      _get19 = b;
      _get20 = bjjtype;
      const struct ReturnValue _1 = dong_porf_porf_todo__ecma262_ToString(_get19, _get20);
      jjlast_type = _1.type;
      _get21 = jjlast_type;
      bjjtype = _get21;
      b = _1.value;
    }
  // end
  j1056:;
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
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get1, _get2, 2437, 195)) & ((_get3 | 128) == (195 | 128))) != 0) {
      const struct ReturnValue _1 = dong_porf_porf_todo_onAdd(0, 0, 0, 0);
      jjlast_type = _1.type;
      (void) _1.value;
    }
  // end
  j1061:;
  _get4 = jjnewtarget;
  // if 
    if (((u32)(_get4)) != 0) {
      _get5 = jjthis;
      _get6 = jjthisjjtype;
      return (struct ReturnValue){ _get5, _get6 };
    }
  // end
  j1062:;
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
  j1063:;
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
  j1064:;
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
  j1065:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_porf_todo_todoIdAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype) {
  i32 _get294;
  f64 _get293;
  i32 _get292;
  f64 _get291;
  i32 _get290;
  f64 _get289;
  f64 _get288;
  i32 _get287;
  f64 _get286;
  i32 _get285;
  f64 _get284;
  i32 _get283;
  f64 _get282;
  f64 _get281;
  i32 _get280;
  f64 _get279;
  i32 _get278;
  f64 _get277;
  i32 _get276;
  f64 _get275;
  i32 _get274;
  f64 _get273;
  f64 _get272;
  i32 _get271;
  f64 _get270;
  i32 _get269;
  f64 _get268;
  i32 _get267;
  f64 _get266;
  i32 _get265;
  f64 _get264;
  f64 _get263;
  i32 _get262;
  f64 _get261;
  i32 _get260;
  f64 _get259;
  i32 _get258;
  f64 _get257;
  i32 _get256;
  f64 _get255;
  f64 _get254;
  i32 _get253;
  f64 _get252;
  i32 _get251;
  f64 _get250;
  i32 _get249;
  f64 _get248;
  i32 _get247;
  f64 _get246;
  f64 _get245;
  i32 _get244;
  f64 _get243;
  i32 _get242;
  f64 _get241;
  i32 _get240;
  f64 _get239;
  i32 _get238;
  f64 _get237;
  f64 _get236;
  i32 _get235;
  f64 _get234;
  i32 _get233;
  f64 _get232;
  i32 _get231;
  f64 _get230;
  i32 _get229;
  f64 _get228;
  f64 _get227;
  i32 _get226;
  f64 _get225;
  i32 _get224;
  f64 _get223;
  i32 _get222;
  f64 _get221;
  i32 _get220;
  f64 _get219;
  f64 _get218;
  i32 _get217;
  f64 _get216;
  i32 _get215;
  f64 _get214;
  i32 _get213;
  f64 _get212;
  i32 _get211;
  f64 _get210;
  f64 _get209;
  i32 _get208;
  f64 _get207;
  i32 _get206;
  f64 _get205;
  i32 _get204;
  f64 _get203;
  i32 _get202;
  f64 _get201;
  f64 _get200;
  i32 _get199;
  f64 _get198;
  i32 _get197;
  f64 _get196;
  i32 _get195;
  f64 _get194;
  i32 _get193;
  f64 _get192;
  f64 _get191;
  i32 _get190;
  f64 _get189;
  i32 _get188;
  f64 _get187;
  i32 _get186;
  f64 _get185;
  i32 _get184;
  f64 _get183;
  f64 _get182;
  i32 _get181;
  f64 _get180;
  i32 _get179;
  f64 _get178;
  i32 _get177;
  f64 _get176;
  i32 _get175;
  f64 _get174;
  f64 _get173;
  i32 _get172;
  f64 _get171;
  i32 _get170;
  f64 _get169;
  i32 _get168;
  f64 _get167;
  i32 _get166;
  f64 _get165;
  f64 _get164;
  i32 _get163;
  f64 _get162;
  i32 _get161;
  f64 _get160;
  i32 _get159;
  f64 _get158;
  i32 _get157;
  f64 _get156;
  f64 _get155;
  i32 _get154;
  f64 _get153;
  i32 _get152;
  f64 _get151;
  i32 _get150;
  f64 _get149;
  i32 _get148;
  f64 _get147;
  f64 _get146;
  i32 _get145;
  f64 _get144;
  i32 _get143;
  f64 _get142;
  i32 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  f64 _get137;
  i32 _get136;
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
          j1076:;
        }
      // end
      j1075:;
      _get7 = jjreturn;
      _get8 = jjreturnjjtype;
      return (struct ReturnValue){ _get7, _get8 };
    }
  // end
  j1074:;
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
          j1079:;
        }
      // end
      j1078:;
      _get16 = jjreturn;
      _get17 = jjreturnjjtype;
      return (struct ReturnValue){ _get16, _get17 };
    }
  // end
  j1077:;
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
          j1082:;
        }
      // end
      j1081:;
      _get25 = jjreturn;
      _get26 = jjreturnjjtype;
      return (struct ReturnValue){ _get25, _get26 };
    }
  // end
  j1080:;
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
          j1085:;
        }
      // end
      j1084:;
      _get34 = jjreturn;
      _get35 = jjreturnjjtype;
      return (struct ReturnValue){ _get34, _get35 };
    }
  // end
  j1083:;
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
          j1088:;
        }
      // end
      j1087:;
      _get43 = jjreturn;
      _get44 = jjreturnjjtype;
      return (struct ReturnValue){ _get43, _get44 };
    }
  // end
  j1086:;
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
          j1091:;
        }
      // end
      j1090:;
      _get52 = jjreturn;
      _get53 = jjreturnjjtype;
      return (struct ReturnValue){ _get52, _get53 };
    }
  // end
  j1089:;
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
          j1094:;
        }
      // end
      j1093:;
      _get61 = jjreturn;
      _get62 = jjreturnjjtype;
      return (struct ReturnValue){ _get61, _get62 };
    }
  // end
  j1092:;
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
          j1097:;
        }
      // end
      j1096:;
      _get70 = jjreturn;
      _get71 = jjreturnjjtype;
      return (struct ReturnValue){ _get70, _get71 };
    }
  // end
  j1095:;
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
          j1100:;
        }
      // end
      j1099:;
      _get79 = jjreturn;
      _get80 = jjreturnjjtype;
      return (struct ReturnValue){ _get79, _get80 };
    }
  // end
  j1098:;
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
          j1103:;
        }
      // end
      j1102:;
      _get88 = jjreturn;
      _get89 = jjreturnjjtype;
      return (struct ReturnValue){ _get88, _get89 };
    }
  // end
  j1101:;
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
          j1106:;
        }
      // end
      j1105:;
      _get97 = jjreturn;
      _get98 = jjreturnjjtype;
      return (struct ReturnValue){ _get97, _get98 };
    }
  // end
  j1104:;
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
          j1109:;
        }
      // end
      j1108:;
      _get106 = jjreturn;
      _get107 = jjreturnjjtype;
      return (struct ReturnValue){ _get106, _get107 };
    }
  // end
  j1107:;
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
          j1112:;
        }
      // end
      j1111:;
      _get115 = jjreturn;
      _get116 = jjreturnjjtype;
      return (struct ReturnValue){ _get115, _get116 };
    }
  // end
  j1110:;
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
          j1115:;
        }
      // end
      j1114:;
      _get124 = jjreturn;
      _get125 = jjreturnjjtype;
      return (struct ReturnValue){ _get124, _get125 };
    }
  // end
  j1113:;
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
          j1118:;
        }
      // end
      j1117:;
      _get133 = jjreturn;
      _get134 = jjreturnjjtype;
      return (struct ReturnValue){ _get133, _get134 };
    }
  // end
  j1116:;
  _get135 = i;
  _get136 = ijjtype;
  // if 
    if ((f64)((_get135 == 15) & ((_get136 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId15;
      jjreturnjjtype = dong_porf_porf_todo_todoId15jjtype;
      _get137 = jjnewtarget;
      // if 
        if (((u32)(_get137)) != 0) {
          _get138 = jjreturn;
          _get139 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get138), _get139)) == 0) {
              _get140 = jjthis;
              _get141 = jjthisjjtype;
              return (struct ReturnValue){ _get140, _get141 };
            }
          // end
          j1121:;
        }
      // end
      j1120:;
      _get142 = jjreturn;
      _get143 = jjreturnjjtype;
      return (struct ReturnValue){ _get142, _get143 };
    }
  // end
  j1119:;
  _get144 = i;
  _get145 = ijjtype;
  // if 
    if ((f64)((_get144 == 16) & ((_get145 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId16;
      jjreturnjjtype = dong_porf_porf_todo_todoId16jjtype;
      _get146 = jjnewtarget;
      // if 
        if (((u32)(_get146)) != 0) {
          _get147 = jjreturn;
          _get148 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get147), _get148)) == 0) {
              _get149 = jjthis;
              _get150 = jjthisjjtype;
              return (struct ReturnValue){ _get149, _get150 };
            }
          // end
          j1124:;
        }
      // end
      j1123:;
      _get151 = jjreturn;
      _get152 = jjreturnjjtype;
      return (struct ReturnValue){ _get151, _get152 };
    }
  // end
  j1122:;
  _get153 = i;
  _get154 = ijjtype;
  // if 
    if ((f64)((_get153 == 17) & ((_get154 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId17;
      jjreturnjjtype = dong_porf_porf_todo_todoId17jjtype;
      _get155 = jjnewtarget;
      // if 
        if (((u32)(_get155)) != 0) {
          _get156 = jjreturn;
          _get157 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get156), _get157)) == 0) {
              _get158 = jjthis;
              _get159 = jjthisjjtype;
              return (struct ReturnValue){ _get158, _get159 };
            }
          // end
          j1127:;
        }
      // end
      j1126:;
      _get160 = jjreturn;
      _get161 = jjreturnjjtype;
      return (struct ReturnValue){ _get160, _get161 };
    }
  // end
  j1125:;
  _get162 = i;
  _get163 = ijjtype;
  // if 
    if ((f64)((_get162 == 18) & ((_get163 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId18;
      jjreturnjjtype = dong_porf_porf_todo_todoId18jjtype;
      _get164 = jjnewtarget;
      // if 
        if (((u32)(_get164)) != 0) {
          _get165 = jjreturn;
          _get166 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get165), _get166)) == 0) {
              _get167 = jjthis;
              _get168 = jjthisjjtype;
              return (struct ReturnValue){ _get167, _get168 };
            }
          // end
          j1130:;
        }
      // end
      j1129:;
      _get169 = jjreturn;
      _get170 = jjreturnjjtype;
      return (struct ReturnValue){ _get169, _get170 };
    }
  // end
  j1128:;
  _get171 = i;
  _get172 = ijjtype;
  // if 
    if ((f64)((_get171 == 19) & ((_get172 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId19;
      jjreturnjjtype = dong_porf_porf_todo_todoId19jjtype;
      _get173 = jjnewtarget;
      // if 
        if (((u32)(_get173)) != 0) {
          _get174 = jjreturn;
          _get175 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get174), _get175)) == 0) {
              _get176 = jjthis;
              _get177 = jjthisjjtype;
              return (struct ReturnValue){ _get176, _get177 };
            }
          // end
          j1133:;
        }
      // end
      j1132:;
      _get178 = jjreturn;
      _get179 = jjreturnjjtype;
      return (struct ReturnValue){ _get178, _get179 };
    }
  // end
  j1131:;
  _get180 = i;
  _get181 = ijjtype;
  // if 
    if ((f64)((_get180 == 20) & ((_get181 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId20;
      jjreturnjjtype = dong_porf_porf_todo_todoId20jjtype;
      _get182 = jjnewtarget;
      // if 
        if (((u32)(_get182)) != 0) {
          _get183 = jjreturn;
          _get184 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get183), _get184)) == 0) {
              _get185 = jjthis;
              _get186 = jjthisjjtype;
              return (struct ReturnValue){ _get185, _get186 };
            }
          // end
          j1136:;
        }
      // end
      j1135:;
      _get187 = jjreturn;
      _get188 = jjreturnjjtype;
      return (struct ReturnValue){ _get187, _get188 };
    }
  // end
  j1134:;
  _get189 = i;
  _get190 = ijjtype;
  // if 
    if ((f64)((_get189 == 21) & ((_get190 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId21;
      jjreturnjjtype = dong_porf_porf_todo_todoId21jjtype;
      _get191 = jjnewtarget;
      // if 
        if (((u32)(_get191)) != 0) {
          _get192 = jjreturn;
          _get193 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get192), _get193)) == 0) {
              _get194 = jjthis;
              _get195 = jjthisjjtype;
              return (struct ReturnValue){ _get194, _get195 };
            }
          // end
          j1139:;
        }
      // end
      j1138:;
      _get196 = jjreturn;
      _get197 = jjreturnjjtype;
      return (struct ReturnValue){ _get196, _get197 };
    }
  // end
  j1137:;
  _get198 = i;
  _get199 = ijjtype;
  // if 
    if ((f64)((_get198 == 22) & ((_get199 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId22;
      jjreturnjjtype = dong_porf_porf_todo_todoId22jjtype;
      _get200 = jjnewtarget;
      // if 
        if (((u32)(_get200)) != 0) {
          _get201 = jjreturn;
          _get202 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get201), _get202)) == 0) {
              _get203 = jjthis;
              _get204 = jjthisjjtype;
              return (struct ReturnValue){ _get203, _get204 };
            }
          // end
          j1142:;
        }
      // end
      j1141:;
      _get205 = jjreturn;
      _get206 = jjreturnjjtype;
      return (struct ReturnValue){ _get205, _get206 };
    }
  // end
  j1140:;
  _get207 = i;
  _get208 = ijjtype;
  // if 
    if ((f64)((_get207 == 23) & ((_get208 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId23;
      jjreturnjjtype = dong_porf_porf_todo_todoId23jjtype;
      _get209 = jjnewtarget;
      // if 
        if (((u32)(_get209)) != 0) {
          _get210 = jjreturn;
          _get211 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get210), _get211)) == 0) {
              _get212 = jjthis;
              _get213 = jjthisjjtype;
              return (struct ReturnValue){ _get212, _get213 };
            }
          // end
          j1145:;
        }
      // end
      j1144:;
      _get214 = jjreturn;
      _get215 = jjreturnjjtype;
      return (struct ReturnValue){ _get214, _get215 };
    }
  // end
  j1143:;
  _get216 = i;
  _get217 = ijjtype;
  // if 
    if ((f64)((_get216 == 24) & ((_get217 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId24;
      jjreturnjjtype = dong_porf_porf_todo_todoId24jjtype;
      _get218 = jjnewtarget;
      // if 
        if (((u32)(_get218)) != 0) {
          _get219 = jjreturn;
          _get220 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get219), _get220)) == 0) {
              _get221 = jjthis;
              _get222 = jjthisjjtype;
              return (struct ReturnValue){ _get221, _get222 };
            }
          // end
          j1148:;
        }
      // end
      j1147:;
      _get223 = jjreturn;
      _get224 = jjreturnjjtype;
      return (struct ReturnValue){ _get223, _get224 };
    }
  // end
  j1146:;
  _get225 = i;
  _get226 = ijjtype;
  // if 
    if ((f64)((_get225 == 25) & ((_get226 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId25;
      jjreturnjjtype = dong_porf_porf_todo_todoId25jjtype;
      _get227 = jjnewtarget;
      // if 
        if (((u32)(_get227)) != 0) {
          _get228 = jjreturn;
          _get229 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get228), _get229)) == 0) {
              _get230 = jjthis;
              _get231 = jjthisjjtype;
              return (struct ReturnValue){ _get230, _get231 };
            }
          // end
          j1151:;
        }
      // end
      j1150:;
      _get232 = jjreturn;
      _get233 = jjreturnjjtype;
      return (struct ReturnValue){ _get232, _get233 };
    }
  // end
  j1149:;
  _get234 = i;
  _get235 = ijjtype;
  // if 
    if ((f64)((_get234 == 26) & ((_get235 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId26;
      jjreturnjjtype = dong_porf_porf_todo_todoId26jjtype;
      _get236 = jjnewtarget;
      // if 
        if (((u32)(_get236)) != 0) {
          _get237 = jjreturn;
          _get238 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get237), _get238)) == 0) {
              _get239 = jjthis;
              _get240 = jjthisjjtype;
              return (struct ReturnValue){ _get239, _get240 };
            }
          // end
          j1154:;
        }
      // end
      j1153:;
      _get241 = jjreturn;
      _get242 = jjreturnjjtype;
      return (struct ReturnValue){ _get241, _get242 };
    }
  // end
  j1152:;
  _get243 = i;
  _get244 = ijjtype;
  // if 
    if ((f64)((_get243 == 27) & ((_get244 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId27;
      jjreturnjjtype = dong_porf_porf_todo_todoId27jjtype;
      _get245 = jjnewtarget;
      // if 
        if (((u32)(_get245)) != 0) {
          _get246 = jjreturn;
          _get247 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get246), _get247)) == 0) {
              _get248 = jjthis;
              _get249 = jjthisjjtype;
              return (struct ReturnValue){ _get248, _get249 };
            }
          // end
          j1157:;
        }
      // end
      j1156:;
      _get250 = jjreturn;
      _get251 = jjreturnjjtype;
      return (struct ReturnValue){ _get250, _get251 };
    }
  // end
  j1155:;
  _get252 = i;
  _get253 = ijjtype;
  // if 
    if ((f64)((_get252 == 28) & ((_get253 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId28;
      jjreturnjjtype = dong_porf_porf_todo_todoId28jjtype;
      _get254 = jjnewtarget;
      // if 
        if (((u32)(_get254)) != 0) {
          _get255 = jjreturn;
          _get256 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get255), _get256)) == 0) {
              _get257 = jjthis;
              _get258 = jjthisjjtype;
              return (struct ReturnValue){ _get257, _get258 };
            }
          // end
          j1160:;
        }
      // end
      j1159:;
      _get259 = jjreturn;
      _get260 = jjreturnjjtype;
      return (struct ReturnValue){ _get259, _get260 };
    }
  // end
  j1158:;
  _get261 = i;
  _get262 = ijjtype;
  // if 
    if ((f64)((_get261 == 29) & ((_get262 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId29;
      jjreturnjjtype = dong_porf_porf_todo_todoId29jjtype;
      _get263 = jjnewtarget;
      // if 
        if (((u32)(_get263)) != 0) {
          _get264 = jjreturn;
          _get265 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get264), _get265)) == 0) {
              _get266 = jjthis;
              _get267 = jjthisjjtype;
              return (struct ReturnValue){ _get266, _get267 };
            }
          // end
          j1163:;
        }
      // end
      j1162:;
      _get268 = jjreturn;
      _get269 = jjreturnjjtype;
      return (struct ReturnValue){ _get268, _get269 };
    }
  // end
  j1161:;
  _get270 = i;
  _get271 = ijjtype;
  // if 
    if ((f64)((_get270 == 30) & ((_get271 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId30;
      jjreturnjjtype = dong_porf_porf_todo_todoId30jjtype;
      _get272 = jjnewtarget;
      // if 
        if (((u32)(_get272)) != 0) {
          _get273 = jjreturn;
          _get274 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get273), _get274)) == 0) {
              _get275 = jjthis;
              _get276 = jjthisjjtype;
              return (struct ReturnValue){ _get275, _get276 };
            }
          // end
          j1166:;
        }
      // end
      j1165:;
      _get277 = jjreturn;
      _get278 = jjreturnjjtype;
      return (struct ReturnValue){ _get277, _get278 };
    }
  // end
  j1164:;
  _get279 = i;
  _get280 = ijjtype;
  // if 
    if ((f64)((_get279 == 31) & ((_get280 | 128) == (1 | 128))) != 0) {
      jjreturn = dong_porf_porf_todo_todoId31;
      jjreturnjjtype = dong_porf_porf_todo_todoId31jjtype;
      _get281 = jjnewtarget;
      // if 
        if (((u32)(_get281)) != 0) {
          _get282 = jjreturn;
          _get283 = jjreturnjjtype;
          // if 
            if ((dong_porf_porf_todo__Porffor_object_isObject((i32)(_get282), _get283)) == 0) {
              _get284 = jjthis;
              _get285 = jjthisjjtype;
              return (struct ReturnValue){ _get284, _get285 };
            }
          // end
          j1169:;
        }
      // end
      j1168:;
      _get286 = jjreturn;
      _get287 = jjreturnjjtype;
      return (struct ReturnValue){ _get286, _get287 };
    }
  // end
  j1167:;
  jjreturn = dong_porf_porf_todo_todoId31;
  jjreturnjjtype = dong_porf_porf_todo_todoId31jjtype;
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
      j1171:;
    }
  // end
  j1170:;
  _get293 = jjreturn;
  _get294 = jjreturnjjtype;
  return (struct ReturnValue){ _get293, _get294 };
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
  j1072:;
    _get2 = j;
    // if 
      if ((_get2 + 1) < dong_porf_porf_todo_todoCount) {
        _get3 = j;
        _get4 = jjjtype;
        _get5 = j;
        const struct ReturnValue _0 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get5 + 1, 1);
        jjlast_type = _0.type;
        _get6 = jjlast_type;
        _get7 = j;
        const struct ReturnValue _1 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get7 + 1, 1);
        jjlast_type = _1.type;
        _get8 = jjlast_type;
        _get9 = j;
        const struct ReturnValue _2 = dong_porf_porf_todo_todoIdAt(0, 0, 0, 0, _get9 + 1, 1);
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
          goto j1073;
        }
        _get14 = j;
        _get15 = jjjtype;
        _get16 = j;
        const struct ReturnValue _4 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get16 + 1, 1);
        jjlast_type = _4.type;
        _get17 = jjlast_type;
        _get18 = j;
        const struct ReturnValue _5 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get18 + 1, 1);
        jjlast_type = _5.type;
        _get19 = jjlast_type;
        _get20 = j;
        const struct ReturnValue _6 = dong_porf_porf_todo_todoIdAt(0, 0, 0, 0, _get20 + 1, 1);
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
          goto j1073;
        }
        _get25 = j;
        _get26 = jjjtype;
        _get27 = j;
        const struct ReturnValue _8 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get27 + 1, 1);
        jjlast_type = _8.type;
        _get28 = jjlast_type;
        _get29 = j;
        const struct ReturnValue _9 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get29 + 1, 1);
        jjlast_type = _9.type;
        _get30 = jjlast_type;
        _get31 = j;
        const struct ReturnValue _10 = dong_porf_porf_todo_todoIdAt(0, 0, 0, 0, _get31 + 1, 1);
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
          goto j1073;
        }
        _get36 = j;
        _get37 = jjjtype;
        _get38 = j;
        const struct ReturnValue _12 = dong_porf_porf_todo_todoTextAt(0, 0, 0, 0, _get38 + 1, 1);
        jjlast_type = _12.type;
        _get39 = jjlast_type;
        _get40 = j;
        const struct ReturnValue _13 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get40 + 1, 1);
        jjlast_type = _13.type;
        _get41 = jjlast_type;
        _get42 = j;
        const struct ReturnValue _14 = dong_porf_porf_todo_todoIdAt(0, 0, 0, 0, _get42 + 1, 1);
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
        goto j1072;
      }
    // end
    j1073:;
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
  j1172:;
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
  j1066:;
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
        i32 _r1068;
          _get4 = jjtypeswitch_tmp1;
          _get5 = jjtypeswitch_tmp1;
          // if 
            if (((_get4 == 67) | (_get5 == 195)) != 0) {
              _get6 = jjlogicinner_tmp;
              _r1068 = i32_load(1, 0, (u32)(_get6));
              goto j1068;
            }
          // end
          j1069:;
          _get7 = jjtypeswitch_tmp1;
          _get8 = jjtypeswitch_tmp1;
          // if 
            if (((_get7 == 31) | (_get8 == 32)) != 0) {
              _r1068 = 1;
              goto j1068;
            }
          // end
          j1070:;
          _get9 = jjlogicinner_tmp;
          const f64 _tmp0 = _get9;
          _r1068 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
        // end
        j1068:;
        // if 
          if ((_r1068) != 0) {
            _get10 = i;
            _get11 = ijjtype;
            const struct ReturnValue _1 = dong_porf_porf_todo_removeAtIndex(0, 0, 0, 0, _get10, _get11);
            jjlast_type = _1.type;
            (void) _1.value;
          }
        // end
        j1071:;
        _get12 = i;
        i = _get12 - 1;
        _get13 = i;
        ijjtype = 1;
        (void) _get13;
        _get14 = i;
        if (!(_get14 >= 0)) {
          goto j1067;
        }
        _get15 = i;
        _get16 = ijjtype;
        const struct ReturnValue _2 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get15, _get16);
        jjlast_type = _2.type;
        jjlogicinner_tmp = _2.value;
        _get17 = jjlast_type;
        jjtypeswitch_tmp1 = _get17;
        // block i32
        i32 _r1173;
          _get18 = jjtypeswitch_tmp1;
          _get19 = jjtypeswitch_tmp1;
          // if 
            if (((_get18 == 67) | (_get19 == 195)) != 0) {
              _get20 = jjlogicinner_tmp;
              _r1173 = i32_load(1, 0, (u32)(_get20));
              goto j1173;
            }
          // end
          j1174:;
          _get21 = jjtypeswitch_tmp1;
          _get22 = jjtypeswitch_tmp1;
          // if 
            if (((_get21 == 31) | (_get22 == 32)) != 0) {
              _r1173 = 1;
              goto j1173;
            }
          // end
          j1175:;
          _get23 = jjlogicinner_tmp;
          const f64 _tmp1 = _get23;
          _r1173 = (_tmp1 < 0 ? -_tmp1 : _tmp1) > 0;
        // end
        j1173:;
        // if 
          if ((_r1173) != 0) {
            _get24 = i;
            _get25 = ijjtype;
            const struct ReturnValue _3 = dong_porf_porf_todo_removeAtIndex(0, 0, 0, 0, _get24, _get25);
            jjlast_type = _3.type;
            (void) _3.value;
          }
        // end
        j1176:;
        _get26 = i;
        i = _get26 - 1;
        _get27 = i;
        ijjtype = 1;
        (void) _get27;
        _get28 = i;
        if (!(_get28 >= 0)) {
          goto j1067;
        }
        _get29 = i;
        _get30 = ijjtype;
        const struct ReturnValue _4 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get29, _get30);
        jjlast_type = _4.type;
        jjlogicinner_tmp = _4.value;
        _get31 = jjlast_type;
        jjtypeswitch_tmp1 = _get31;
        // block i32
        i32 _r1177;
          _get32 = jjtypeswitch_tmp1;
          _get33 = jjtypeswitch_tmp1;
          // if 
            if (((_get32 == 67) | (_get33 == 195)) != 0) {
              _get34 = jjlogicinner_tmp;
              _r1177 = i32_load(1, 0, (u32)(_get34));
              goto j1177;
            }
          // end
          j1178:;
          _get35 = jjtypeswitch_tmp1;
          _get36 = jjtypeswitch_tmp1;
          // if 
            if (((_get35 == 31) | (_get36 == 32)) != 0) {
              _r1177 = 1;
              goto j1177;
            }
          // end
          j1179:;
          _get37 = jjlogicinner_tmp;
          const f64 _tmp2 = _get37;
          _r1177 = (_tmp2 < 0 ? -_tmp2 : _tmp2) > 0;
        // end
        j1177:;
        // if 
          if ((_r1177) != 0) {
            _get38 = i;
            _get39 = ijjtype;
            const struct ReturnValue _5 = dong_porf_porf_todo_removeAtIndex(0, 0, 0, 0, _get38, _get39);
            jjlast_type = _5.type;
            (void) _5.value;
          }
        // end
        j1180:;
        _get40 = i;
        i = _get40 - 1;
        _get41 = i;
        ijjtype = 1;
        (void) _get41;
        _get42 = i;
        if (!(_get42 >= 0)) {
          goto j1067;
        }
        _get43 = i;
        _get44 = ijjtype;
        const struct ReturnValue _6 = dong_porf_porf_todo_todoDoneAt(0, 0, 0, 0, _get43, _get44);
        jjlast_type = _6.type;
        jjlogicinner_tmp = _6.value;
        _get45 = jjlast_type;
        jjtypeswitch_tmp1 = _get45;
        // block i32
        i32 _r1181;
          _get46 = jjtypeswitch_tmp1;
          _get47 = jjtypeswitch_tmp1;
          // if 
            if (((_get46 == 67) | (_get47 == 195)) != 0) {
              _get48 = jjlogicinner_tmp;
              _r1181 = i32_load(1, 0, (u32)(_get48));
              goto j1181;
            }
          // end
          j1182:;
          _get49 = jjtypeswitch_tmp1;
          _get50 = jjtypeswitch_tmp1;
          // if 
            if (((_get49 == 31) | (_get50 == 32)) != 0) {
              _r1181 = 1;
              goto j1181;
            }
          // end
          j1183:;
          _get51 = jjlogicinner_tmp;
          const f64 _tmp3 = _get51;
          _r1181 = (_tmp3 < 0 ? -_tmp3 : _tmp3) > 0;
        // end
        j1181:;
        // if 
          if ((_r1181) != 0) {
            _get52 = i;
            _get53 = ijjtype;
            const struct ReturnValue _7 = dong_porf_porf_todo_removeAtIndex(0, 0, 0, 0, _get52, _get53);
            jjlast_type = _7.type;
            (void) _7.value;
          }
        // end
        j1184:;
        _get54 = i;
        i = _get54 - 1;
        _get55 = i;
        ijjtype = 1;
        (void) _get55;
        goto j1066;
      }
    // end
    j1067:;
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
  j1185:;
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
      j1187:;
    }
  // end
  j1186:;
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
      j1192:;
    }
  // end
  j1191:;
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
          j1198:;
        }
      // end
      j1197:;
      _get8 = jjreturn;
      _get9 = jjreturnjjtype;
      return (struct ReturnValue){ _get8, _get9 };
    }
  // end
  j1196:;
  _get10 = s;
  _get11 = sjjtype;
  _get12 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get10, _get11, 2111, 195)) & ((_get12 | 128) == (195 | 128))) != 0) {
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
          j1201:;
        }
      // end
      j1200:;
      _get18 = jjreturn;
      _get19 = jjreturnjjtype;
      return (struct ReturnValue){ _get18, _get19 };
    }
  // end
  j1199:;
  _get20 = s;
  _get21 = sjjtype;
  _get22 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get20, _get21, 2476, 195)) & ((_get22 | 128) == (195 | 128))) != 0) {
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
          j1204:;
        }
      // end
      j1203:;
      _get28 = jjreturn;
      _get29 = jjreturnjjtype;
      return (struct ReturnValue){ _get28, _get29 };
    }
  // end
  j1202:;
  _get30 = s;
  _get31 = sjjtype;
  _get32 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get30, _get31, 2483, 195)) & ((_get32 | 128) == (195 | 128))) != 0) {
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
          j1207:;
        }
      // end
      j1206:;
      _get38 = jjreturn;
      _get39 = jjreturnjjtype;
      return (struct ReturnValue){ _get38, _get39 };
    }
  // end
  j1205:;
  _get40 = s;
  _get41 = sjjtype;
  _get42 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get40, _get41, 2490, 195)) & ((_get42 | 128) == (195 | 128))) != 0) {
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
          j1210:;
        }
      // end
      j1209:;
      _get48 = jjreturn;
      _get49 = jjreturnjjtype;
      return (struct ReturnValue){ _get48, _get49 };
    }
  // end
  j1208:;
  _get50 = s;
  _get51 = sjjtype;
  _get52 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get50, _get51, 2497, 195)) & ((_get52 | 128) == (195 | 128))) != 0) {
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
          j1213:;
        }
      // end
      j1212:;
      _get58 = jjreturn;
      _get59 = jjreturnjjtype;
      return (struct ReturnValue){ _get58, _get59 };
    }
  // end
  j1211:;
  _get60 = s;
  _get61 = sjjtype;
  _get62 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get60, _get61, 2504, 195)) & ((_get62 | 128) == (195 | 128))) != 0) {
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
          j1216:;
        }
      // end
      j1215:;
      _get68 = jjreturn;
      _get69 = jjreturnjjtype;
      return (struct ReturnValue){ _get68, _get69 };
    }
  // end
  j1214:;
  _get70 = s;
  _get71 = sjjtype;
  _get72 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get70, _get71, 2511, 195)) & ((_get72 | 128) == (195 | 128))) != 0) {
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
          j1219:;
        }
      // end
      j1218:;
      _get78 = jjreturn;
      _get79 = jjreturnjjtype;
      return (struct ReturnValue){ _get78, _get79 };
    }
  // end
  j1217:;
  _get80 = s;
  _get81 = sjjtype;
  _get82 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get80, _get81, 2518, 195)) & ((_get82 | 128) == (195 | 128))) != 0) {
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
          j1222:;
        }
      // end
      j1221:;
      _get88 = jjreturn;
      _get89 = jjreturnjjtype;
      return (struct ReturnValue){ _get88, _get89 };
    }
  // end
  j1220:;
  _get90 = s;
  _get91 = sjjtype;
  _get92 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get90, _get91, 2525, 195)) & ((_get92 | 128) == (195 | 128))) != 0) {
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
          j1225:;
        }
      // end
      j1224:;
      _get98 = jjreturn;
      _get99 = jjreturnjjtype;
      return (struct ReturnValue){ _get98, _get99 };
    }
  // end
  j1223:;
  _get100 = s;
  _get101 = sjjtype;
  _get102 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get100, _get101, 2532, 195)) & ((_get102 | 128) == (195 | 128))) != 0) {
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
          j1228:;
        }
      // end
      j1227:;
      _get108 = jjreturn;
      _get109 = jjreturnjjtype;
      return (struct ReturnValue){ _get108, _get109 };
    }
  // end
  j1226:;
  _get110 = s;
  _get111 = sjjtype;
  _get112 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get110, _get111, 2540, 195)) & ((_get112 | 128) == (195 | 128))) != 0) {
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
          j1231:;
        }
      // end
      j1230:;
      _get118 = jjreturn;
      _get119 = jjreturnjjtype;
      return (struct ReturnValue){ _get118, _get119 };
    }
  // end
  j1229:;
  _get120 = s;
  _get121 = sjjtype;
  _get122 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get120, _get121, 2548, 195)) & ((_get122 | 128) == (195 | 128))) != 0) {
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
          j1234:;
        }
      // end
      j1233:;
      _get128 = jjreturn;
      _get129 = jjreturnjjtype;
      return (struct ReturnValue){ _get128, _get129 };
    }
  // end
  j1232:;
  _get130 = s;
  _get131 = sjjtype;
  _get132 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get130, _get131, 2556, 195)) & ((_get132 | 128) == (195 | 128))) != 0) {
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
          j1237:;
        }
      // end
      j1236:;
      _get138 = jjreturn;
      _get139 = jjreturnjjtype;
      return (struct ReturnValue){ _get138, _get139 };
    }
  // end
  j1235:;
  _get140 = s;
  _get141 = sjjtype;
  _get142 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get140, _get141, 2564, 195)) & ((_get142 | 128) == (195 | 128))) != 0) {
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
          j1240:;
        }
      // end
      j1239:;
      _get148 = jjreturn;
      _get149 = jjreturnjjtype;
      return (struct ReturnValue){ _get148, _get149 };
    }
  // end
  j1238:;
  _get150 = s;
  _get151 = sjjtype;
  _get152 = sjjtype;
  // if 
    if ((f64)((u32)(dong_porf_porf_todo__Porffor_compareStrings(_get150, _get151, 2572, 195)) & ((_get152 | 128) == (195 | 128))) != 0) {
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
          j1243:;
        }
      // end
      j1242:;
      _get158 = jjreturn;
      _get159 = jjreturnjjtype;
      return (struct ReturnValue){ _get158, _get159 };
    }
  // end
  j1241:;
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
      j1245:;
    }
  // end
  j1244:;
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
      j1249:;
    }
  // end
  j1248:;
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
  f64 jjmember_obj_160 = 0;
  f64 jjmember_prop_160 = 0;
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
          j1190:;
        }
      // end
      j1189:;
      _get8 = jjreturn;
      _get9 = jjreturnjjtype;
      return (struct ReturnValue){ _get8, _get9 };
    }
  // end
  j1188:;
  _get10 = target;
  _get11 = targetjjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo_getAttribute(0, 0, 0, 0, _get10, _get11, 2448, 195);
  jjlast_type = _1.type;
  _get12 = jjlast_type;
  vjjtype = _get12;
  v = _1.value;
  _get13 = v;
  jjlength_tmp = (u32)(_get13);
  _get14 = vjjtype;
  // if f64
  f64 _r1193;
    if ((_get14 & 64) != 0) {
      _get15 = jjlength_tmp;
      jjlast_type = 1;
      _r1193 = (f64)(i32_load(1, 0, _get15));
    } else {
      jjmember_prop_160 = 564;
      _get16 = v;
      jjmember_obj_160 = _get16;
      _get17 = vjjtype;
      // if f64
      f64 _r1194;
        if (_get17 == 0) {
          _r1194 = 0;
        } else {
          _get18 = jjmember_obj_160;
          _get19 = vjjtype;
          _get20 = jjmember_prop_160;
          const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get18), _get19, (u32)(_get20), 195, -2086110260, 1);
          jjlast_type = _2.type;
          _r1194 = _2.value;
        }
      // end
      j1194:;
      _r1193 = _r1194;
    }
  // end
  j1193:;
  // if 
    if ((f64)(_r1193 > 0) != 0) {
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
          j1247:;
        }
      // end
      j1246:;
      _get29 = jjreturn;
      _get30 = jjreturnjjtype;
      return (struct ReturnValue){ _get29, _get30 };
    }
  // end
  j1195:;
  _get31 = target;
  _get32 = targetjjtype;
  const struct ReturnValue _4 = dong_porf_porf_todo_closestSelector(0, 0, 0, 0, _get31, _get32, 2580, 195);
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
      const struct ReturnValue _5 = dong_porf_porf_todo_getAttribute(0, 0, 0, 0, _get36, _get37, 2448, 195);
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
          j1252:;
        }
      // end
      j1251:;
      _get47 = jjreturn;
      _get48 = jjreturnjjtype;
      return (struct ReturnValue){ _get47, _get48 };
    }
  // end
  j1250:;
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
      j1254:;
    }
  // end
  j1253:;
  _get54 = jjreturn;
  _get55 = jjreturnjjtype;
  return (struct ReturnValue){ _get54, _get55 };
}

static struct ReturnValue dong_porf_porf_todo_setTodoDoneAt(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 i, i32 ijjtype, f64 v, i32 vjjtype) {
  i32 _get228;
  f64 _get227;
  f64 _get226;
  i32 _get225;
  f64 _get224;
  i32 _get223;
  f64 _get222;
  f64 _get221;
  i32 _get220;
  f64 _get219;
  i32 _get218;
  f64 _get217;
  i32 _get216;
  f64 _get215;
  f64 _get214;
  i32 _get213;
  f64 _get212;
  i32 _get211;
  f64 _get210;
  i32 _get209;
  f64 _get208;
  f64 _get207;
  i32 _get206;
  f64 _get205;
  i32 _get204;
  f64 _get203;
  i32 _get202;
  f64 _get201;
  f64 _get200;
  i32 _get199;
  f64 _get198;
  i32 _get197;
  f64 _get196;
  i32 _get195;
  f64 _get194;
  f64 _get193;
  i32 _get192;
  f64 _get191;
  i32 _get190;
  f64 _get189;
  i32 _get188;
  f64 _get187;
  f64 _get186;
  i32 _get185;
  f64 _get184;
  i32 _get183;
  f64 _get182;
  i32 _get181;
  f64 _get180;
  f64 _get179;
  i32 _get178;
  f64 _get177;
  i32 _get176;
  f64 _get175;
  i32 _get174;
  f64 _get173;
  f64 _get172;
  i32 _get171;
  f64 _get170;
  i32 _get169;
  f64 _get168;
  i32 _get167;
  f64 _get166;
  f64 _get165;
  i32 _get164;
  f64 _get163;
  i32 _get162;
  f64 _get161;
  i32 _get160;
  f64 _get159;
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
  f64 _get144;
  i32 _get143;
  f64 _get142;
  i32 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  f64 _get137;
  i32 _get136;
  f64 _get135;
  i32 _get134;
  f64 _get133;
  i32 _get132;
  f64 _get131;
  f64 _get130;
  i32 _get129;
  f64 _get128;
  i32 _get127;
  f64 _get126;
  i32 _get125;
  f64 _get124;
  f64 _get123;
  i32 _get122;
  f64 _get121;
  i32 _get120;
  f64 _get119;
  i32 _get118;
  f64 _get117;
  f64 _get116;
  i32 _get115;
  f64 _get114;
  i32 _get113;
  f64 _get112;
  i32 _get111;
  f64 _get110;
  f64 _get109;
  i32 _get108;
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
      j1262:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1261:;
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
      j1264:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1263:;
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
      j1266:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1265:;
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
      j1268:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1267:;
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
      j1270:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1269:;
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
      j1272:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1271:;
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
      j1274:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1273:;
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
      j1276:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1275:;
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
      j1278:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1277:;
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
      j1280:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1279:;
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
      j1282:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1281:;
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
      j1284:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1283:;
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
      j1286:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1285:;
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
      j1288:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1287:;
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
      j1290:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1289:;
  _get105 = i;
  _get106 = ijjtype;
  // if 
    if ((f64)((_get105 == 15) & ((_get106 | 128) == (1 | 128))) != 0) {
      _get107 = v;
      dong_porf_porf_todo_todoDone15 = _get107;
      _get108 = vjjtype;
      dong_porf_porf_todo_todoDone15jjtype = _get108;
      (void) dong_porf_porf_todo_todoDone15;
      _get109 = jjnewtarget;
      // if 
        if (((u32)(_get109)) != 0) {
          _get110 = jjthis;
          _get111 = jjthisjjtype;
          return (struct ReturnValue){ _get110, _get111 };
        }
      // end
      j1292:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1291:;
  _get112 = i;
  _get113 = ijjtype;
  // if 
    if ((f64)((_get112 == 16) & ((_get113 | 128) == (1 | 128))) != 0) {
      _get114 = v;
      dong_porf_porf_todo_todoDone16 = _get114;
      _get115 = vjjtype;
      dong_porf_porf_todo_todoDone16jjtype = _get115;
      (void) dong_porf_porf_todo_todoDone16;
      _get116 = jjnewtarget;
      // if 
        if (((u32)(_get116)) != 0) {
          _get117 = jjthis;
          _get118 = jjthisjjtype;
          return (struct ReturnValue){ _get117, _get118 };
        }
      // end
      j1294:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1293:;
  _get119 = i;
  _get120 = ijjtype;
  // if 
    if ((f64)((_get119 == 17) & ((_get120 | 128) == (1 | 128))) != 0) {
      _get121 = v;
      dong_porf_porf_todo_todoDone17 = _get121;
      _get122 = vjjtype;
      dong_porf_porf_todo_todoDone17jjtype = _get122;
      (void) dong_porf_porf_todo_todoDone17;
      _get123 = jjnewtarget;
      // if 
        if (((u32)(_get123)) != 0) {
          _get124 = jjthis;
          _get125 = jjthisjjtype;
          return (struct ReturnValue){ _get124, _get125 };
        }
      // end
      j1296:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1295:;
  _get126 = i;
  _get127 = ijjtype;
  // if 
    if ((f64)((_get126 == 18) & ((_get127 | 128) == (1 | 128))) != 0) {
      _get128 = v;
      dong_porf_porf_todo_todoDone18 = _get128;
      _get129 = vjjtype;
      dong_porf_porf_todo_todoDone18jjtype = _get129;
      (void) dong_porf_porf_todo_todoDone18;
      _get130 = jjnewtarget;
      // if 
        if (((u32)(_get130)) != 0) {
          _get131 = jjthis;
          _get132 = jjthisjjtype;
          return (struct ReturnValue){ _get131, _get132 };
        }
      // end
      j1298:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1297:;
  _get133 = i;
  _get134 = ijjtype;
  // if 
    if ((f64)((_get133 == 19) & ((_get134 | 128) == (1 | 128))) != 0) {
      _get135 = v;
      dong_porf_porf_todo_todoDone19 = _get135;
      _get136 = vjjtype;
      dong_porf_porf_todo_todoDone19jjtype = _get136;
      (void) dong_porf_porf_todo_todoDone19;
      _get137 = jjnewtarget;
      // if 
        if (((u32)(_get137)) != 0) {
          _get138 = jjthis;
          _get139 = jjthisjjtype;
          return (struct ReturnValue){ _get138, _get139 };
        }
      // end
      j1300:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1299:;
  _get140 = i;
  _get141 = ijjtype;
  // if 
    if ((f64)((_get140 == 20) & ((_get141 | 128) == (1 | 128))) != 0) {
      _get142 = v;
      dong_porf_porf_todo_todoDone20 = _get142;
      _get143 = vjjtype;
      dong_porf_porf_todo_todoDone20jjtype = _get143;
      (void) dong_porf_porf_todo_todoDone20;
      _get144 = jjnewtarget;
      // if 
        if (((u32)(_get144)) != 0) {
          _get145 = jjthis;
          _get146 = jjthisjjtype;
          return (struct ReturnValue){ _get145, _get146 };
        }
      // end
      j1302:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1301:;
  _get147 = i;
  _get148 = ijjtype;
  // if 
    if ((f64)((_get147 == 21) & ((_get148 | 128) == (1 | 128))) != 0) {
      _get149 = v;
      dong_porf_porf_todo_todoDone21 = _get149;
      _get150 = vjjtype;
      dong_porf_porf_todo_todoDone21jjtype = _get150;
      (void) dong_porf_porf_todo_todoDone21;
      _get151 = jjnewtarget;
      // if 
        if (((u32)(_get151)) != 0) {
          _get152 = jjthis;
          _get153 = jjthisjjtype;
          return (struct ReturnValue){ _get152, _get153 };
        }
      // end
      j1304:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1303:;
  _get154 = i;
  _get155 = ijjtype;
  // if 
    if ((f64)((_get154 == 22) & ((_get155 | 128) == (1 | 128))) != 0) {
      _get156 = v;
      dong_porf_porf_todo_todoDone22 = _get156;
      _get157 = vjjtype;
      dong_porf_porf_todo_todoDone22jjtype = _get157;
      (void) dong_porf_porf_todo_todoDone22;
      _get158 = jjnewtarget;
      // if 
        if (((u32)(_get158)) != 0) {
          _get159 = jjthis;
          _get160 = jjthisjjtype;
          return (struct ReturnValue){ _get159, _get160 };
        }
      // end
      j1306:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1305:;
  _get161 = i;
  _get162 = ijjtype;
  // if 
    if ((f64)((_get161 == 23) & ((_get162 | 128) == (1 | 128))) != 0) {
      _get163 = v;
      dong_porf_porf_todo_todoDone23 = _get163;
      _get164 = vjjtype;
      dong_porf_porf_todo_todoDone23jjtype = _get164;
      (void) dong_porf_porf_todo_todoDone23;
      _get165 = jjnewtarget;
      // if 
        if (((u32)(_get165)) != 0) {
          _get166 = jjthis;
          _get167 = jjthisjjtype;
          return (struct ReturnValue){ _get166, _get167 };
        }
      // end
      j1308:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1307:;
  _get168 = i;
  _get169 = ijjtype;
  // if 
    if ((f64)((_get168 == 24) & ((_get169 | 128) == (1 | 128))) != 0) {
      _get170 = v;
      dong_porf_porf_todo_todoDone24 = _get170;
      _get171 = vjjtype;
      dong_porf_porf_todo_todoDone24jjtype = _get171;
      (void) dong_porf_porf_todo_todoDone24;
      _get172 = jjnewtarget;
      // if 
        if (((u32)(_get172)) != 0) {
          _get173 = jjthis;
          _get174 = jjthisjjtype;
          return (struct ReturnValue){ _get173, _get174 };
        }
      // end
      j1310:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1309:;
  _get175 = i;
  _get176 = ijjtype;
  // if 
    if ((f64)((_get175 == 25) & ((_get176 | 128) == (1 | 128))) != 0) {
      _get177 = v;
      dong_porf_porf_todo_todoDone25 = _get177;
      _get178 = vjjtype;
      dong_porf_porf_todo_todoDone25jjtype = _get178;
      (void) dong_porf_porf_todo_todoDone25;
      _get179 = jjnewtarget;
      // if 
        if (((u32)(_get179)) != 0) {
          _get180 = jjthis;
          _get181 = jjthisjjtype;
          return (struct ReturnValue){ _get180, _get181 };
        }
      // end
      j1312:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1311:;
  _get182 = i;
  _get183 = ijjtype;
  // if 
    if ((f64)((_get182 == 26) & ((_get183 | 128) == (1 | 128))) != 0) {
      _get184 = v;
      dong_porf_porf_todo_todoDone26 = _get184;
      _get185 = vjjtype;
      dong_porf_porf_todo_todoDone26jjtype = _get185;
      (void) dong_porf_porf_todo_todoDone26;
      _get186 = jjnewtarget;
      // if 
        if (((u32)(_get186)) != 0) {
          _get187 = jjthis;
          _get188 = jjthisjjtype;
          return (struct ReturnValue){ _get187, _get188 };
        }
      // end
      j1314:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1313:;
  _get189 = i;
  _get190 = ijjtype;
  // if 
    if ((f64)((_get189 == 27) & ((_get190 | 128) == (1 | 128))) != 0) {
      _get191 = v;
      dong_porf_porf_todo_todoDone27 = _get191;
      _get192 = vjjtype;
      dong_porf_porf_todo_todoDone27jjtype = _get192;
      (void) dong_porf_porf_todo_todoDone27;
      _get193 = jjnewtarget;
      // if 
        if (((u32)(_get193)) != 0) {
          _get194 = jjthis;
          _get195 = jjthisjjtype;
          return (struct ReturnValue){ _get194, _get195 };
        }
      // end
      j1316:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1315:;
  _get196 = i;
  _get197 = ijjtype;
  // if 
    if ((f64)((_get196 == 28) & ((_get197 | 128) == (1 | 128))) != 0) {
      _get198 = v;
      dong_porf_porf_todo_todoDone28 = _get198;
      _get199 = vjjtype;
      dong_porf_porf_todo_todoDone28jjtype = _get199;
      (void) dong_porf_porf_todo_todoDone28;
      _get200 = jjnewtarget;
      // if 
        if (((u32)(_get200)) != 0) {
          _get201 = jjthis;
          _get202 = jjthisjjtype;
          return (struct ReturnValue){ _get201, _get202 };
        }
      // end
      j1318:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1317:;
  _get203 = i;
  _get204 = ijjtype;
  // if 
    if ((f64)((_get203 == 29) & ((_get204 | 128) == (1 | 128))) != 0) {
      _get205 = v;
      dong_porf_porf_todo_todoDone29 = _get205;
      _get206 = vjjtype;
      dong_porf_porf_todo_todoDone29jjtype = _get206;
      (void) dong_porf_porf_todo_todoDone29;
      _get207 = jjnewtarget;
      // if 
        if (((u32)(_get207)) != 0) {
          _get208 = jjthis;
          _get209 = jjthisjjtype;
          return (struct ReturnValue){ _get208, _get209 };
        }
      // end
      j1320:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1319:;
  _get210 = i;
  _get211 = ijjtype;
  // if 
    if ((f64)((_get210 == 30) & ((_get211 | 128) == (1 | 128))) != 0) {
      _get212 = v;
      dong_porf_porf_todo_todoDone30 = _get212;
      _get213 = vjjtype;
      dong_porf_porf_todo_todoDone30jjtype = _get213;
      (void) dong_porf_porf_todo_todoDone30;
      _get214 = jjnewtarget;
      // if 
        if (((u32)(_get214)) != 0) {
          _get215 = jjthis;
          _get216 = jjthisjjtype;
          return (struct ReturnValue){ _get215, _get216 };
        }
      // end
      j1322:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1321:;
  _get217 = i;
  _get218 = ijjtype;
  // if 
    if ((f64)((_get217 == 31) & ((_get218 | 128) == (1 | 128))) != 0) {
      _get219 = v;
      dong_porf_porf_todo_todoDone31 = _get219;
      _get220 = vjjtype;
      dong_porf_porf_todo_todoDone31jjtype = _get220;
      (void) dong_porf_porf_todo_todoDone31;
      _get221 = jjnewtarget;
      // if 
        if (((u32)(_get221)) != 0) {
          _get222 = jjthis;
          _get223 = jjthisjjtype;
          return (struct ReturnValue){ _get222, _get223 };
        }
      // end
      j1324:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1323:;
  _get224 = v;
  dong_porf_porf_todo_todoDone31 = _get224;
  _get225 = vjjtype;
  dong_porf_porf_todo_todoDone31jjtype = _get225;
  (void) dong_porf_porf_todo_todoDone31;
  _get226 = jjnewtarget;
  // if 
    if (((u32)(_get226)) != 0) {
      _get227 = jjthis;
      _get228 = jjthisjjtype;
      return (struct ReturnValue){ _get227, _get228 };
    }
  // end
  j1325:;
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
  f64 jjmember_obj_161 = 0;
  f64 jjmember_prop_161 = 0;
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
          j1329:;
        }
      // end
      j1328:;
      _get8 = jjreturn;
      _get9 = jjreturnjjtype;
      return (struct ReturnValue){ _get8, _get9 };
    }
  // end
  j1327:;
  _get10 = target;
  _get11 = targetjjtype;
  const struct ReturnValue _1 = dong_porf_porf_todo_getAttribute(0, 0, 0, 0, _get10, _get11, 2610, 195);
  jjlast_type = _1.type;
  _get12 = jjlast_type;
  vjjtype = _get12;
  v = _1.value;
  _get13 = v;
  jjlength_tmp = (u32)(_get13);
  _get14 = vjjtype;
  // if f64
  f64 _r1330;
    if ((_get14 & 64) != 0) {
      _get15 = jjlength_tmp;
      jjlast_type = 1;
      _r1330 = (f64)(i32_load(1, 0, _get15));
    } else {
      jjmember_prop_161 = 564;
      _get16 = v;
      jjmember_obj_161 = _get16;
      _get17 = vjjtype;
      // if f64
      f64 _r1331;
        if (_get17 == 0) {
          _r1331 = 0;
        } else {
          _get18 = jjmember_obj_161;
          _get19 = vjjtype;
          _get20 = jjmember_prop_161;
          const struct ReturnValue _2 = dong_porf_porf_todo__Porffor_object_get_withHash((i32)(_get18), _get19, (u32)(_get20), 195, -2086110260, 1);
          jjlast_type = _2.type;
          _r1331 = _2.value;
        }
      // end
      j1331:;
      _r1330 = _r1331;
    }
  // end
  j1330:;
  // if 
    if ((f64)(_r1330 > 0) != 0) {
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
          j1334:;
        }
      // end
      j1333:;
      _get29 = jjreturn;
      _get30 = jjreturnjjtype;
      return (struct ReturnValue){ _get29, _get30 };
    }
  // end
  j1332:;
  _get31 = target;
  _get32 = targetjjtype;
  const struct ReturnValue _4 = dong_porf_porf_todo_closestSelector(0, 0, 0, 0, _get31, _get32, 2638, 195);
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
      const struct ReturnValue _5 = dong_porf_porf_todo_getAttribute(0, 0, 0, 0, _get36, _get37, 2610, 195);
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
          j1337:;
        }
      // end
      j1336:;
      _get47 = jjreturn;
      _get48 = jjreturnjjtype;
      return (struct ReturnValue){ _get47, _get48 };
    }
  // end
  j1335:;
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
      j1339:;
    }
  // end
  j1338:;
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
  i32 _r1255;
    if (_get2 != 0) {
      _get3 = idx;
      jjlast_type = 2;
      _r1255 = _get3 < dong_porf_porf_todo_todoCount;
    } else {
      _get4 = logictmpi;
      jjlast_type = 2;
      _r1255 = _get4;
    }
  // end
  j1255:;
  // if 
    if ((f64)(_r1255) != 0) {
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
      i32 _r1257;
        _get10 = jjtypeswitch_tmp1;
        _get11 = jjtypeswitch_tmp1;
        // if 
          if (((_get10 == 67) | (_get11 == 195)) != 0) {
            _get12 = jjlogicinner_tmp;
            _r1257 = i32_load(1, 0, (u32)(_get12));
            goto j1257;
          }
        // end
        j1258:;
        _get13 = jjtypeswitch_tmp1;
        _get14 = jjtypeswitch_tmp1;
        // if 
          if (((_get13 == 31) | (_get14 == 32)) != 0) {
            _r1257 = 1;
            goto j1257;
          }
        // end
        j1259:;
        _get15 = jjlogicinner_tmp;
        const f64 _tmp0 = _get15;
        _r1257 = (_tmp0 < 0 ? -_tmp0 : _tmp0) > 0;
      // end
      j1257:;
      // if 
        if ((_r1257) != 0) {
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
      j1260:;
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
      j1326:;
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j1256:;
  const struct ReturnValue _5 = dong_porf_porf_todo_readDeleteIndex(0, 0, 0, 0);
  jjlast_type = _5.type;
  _get23 = jjlast_type;
  idx2jjtype = _get23;
  idx2 = _5.value;
  _get24 = idx2;
  logictmpi = _get24 >= 0;
  _get25 = logictmpi;
  // if i32
  i32 _r1340;
    if (_get25 != 0) {
      _get26 = idx2;
      jjlast_type = 2;
      _r1340 = _get26 < dong_porf_porf_todo_todoCount;
    } else {
      _get27 = logictmpi;
      jjlast_type = 2;
      _r1340 = _get27;
    }
  // end
  j1340:;
  // if 
    if ((f64)(_r1340) != 0) {
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
  j1341:;
  _get30 = jjnewtarget;
  // if 
    if (((u32)(_get30)) != 0) {
      _get31 = jjthis;
      _get32 = jjthisjjtype;
      return (struct ReturnValue){ _get31, _get32 };
    }
  // end
  j1342:;
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

int dong_porf_porf_todo_export_headlessAddSample(void) {
  dong_porf_porf_todo__porf_init();
  (void)dong_porf_porf_todo_headlessAddSample(0, 0, 0, 0);
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
  f64 dong_porf_porf_todo_porf_rootId;
  i32 dong_porf_porf_todo_porf_rootIdjjtype;
  f64 dong_porf_porf_todo_titleId;
  i32 dong_porf_porf_todo_titleIdjjtype;
  f64 dong_porf_porf_todo_todo_inputId;
  i32 dong_porf_porf_todo_todo_inputIdjjtype;
  f64 dong_porf_porf_todo_btn_addId;
  i32 dong_porf_porf_todo_btn_addIdjjtype;
  f64 dong_porf_porf_todo_filter_barId;
  i32 dong_porf_porf_todo_filter_barIdjjtype;
  f64 dong_porf_porf_todo_filter_allId;
  i32 dong_porf_porf_todo_filter_allIdjjtype;
  f64 dong_porf_porf_todo_filter_activeId;
  i32 dong_porf_porf_todo_filter_activeIdjjtype;
  f64 dong_porf_porf_todo_filter_doneId;
  i32 dong_porf_porf_todo_filter_doneIdjjtype;
  f64 dong_porf_porf_todo_todo_listId;
  i32 dong_porf_porf_todo_todo_listIdjjtype;
  f64 dong_porf_porf_todo_clear_wrapId;
  i32 dong_porf_porf_todo_clear_wrapIdjjtype;
  f64 dong_porf_porf_todo_btn_clearId;
  i32 dong_porf_porf_todo_btn_clearIdjjtype;
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
  f64 dong_porf_porf_todo_showClear;
  i32 dong_porf_porf_todo_showClearjjtype;
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
  f64 dong_porf_porf_todo_todoText16;
  i32 dong_porf_porf_todo_todoText16jjtype;
  f64 dong_porf_porf_todo_todoDone16;
  i32 dong_porf_porf_todo_todoDone16jjtype;
  f64 dong_porf_porf_todo_todoId16;
  i32 dong_porf_porf_todo_todoId16jjtype;
  f64 dong_porf_porf_todo_todoText17;
  i32 dong_porf_porf_todo_todoText17jjtype;
  f64 dong_porf_porf_todo_todoDone17;
  i32 dong_porf_porf_todo_todoDone17jjtype;
  f64 dong_porf_porf_todo_todoId17;
  i32 dong_porf_porf_todo_todoId17jjtype;
  f64 dong_porf_porf_todo_todoText18;
  i32 dong_porf_porf_todo_todoText18jjtype;
  f64 dong_porf_porf_todo_todoDone18;
  i32 dong_porf_porf_todo_todoDone18jjtype;
  f64 dong_porf_porf_todo_todoId18;
  i32 dong_porf_porf_todo_todoId18jjtype;
  f64 dong_porf_porf_todo_todoText19;
  i32 dong_porf_porf_todo_todoText19jjtype;
  f64 dong_porf_porf_todo_todoDone19;
  i32 dong_porf_porf_todo_todoDone19jjtype;
  f64 dong_porf_porf_todo_todoId19;
  i32 dong_porf_porf_todo_todoId19jjtype;
  f64 dong_porf_porf_todo_todoText20;
  i32 dong_porf_porf_todo_todoText20jjtype;
  f64 dong_porf_porf_todo_todoDone20;
  i32 dong_porf_porf_todo_todoDone20jjtype;
  f64 dong_porf_porf_todo_todoId20;
  i32 dong_porf_porf_todo_todoId20jjtype;
  f64 dong_porf_porf_todo_todoText21;
  i32 dong_porf_porf_todo_todoText21jjtype;
  f64 dong_porf_porf_todo_todoDone21;
  i32 dong_porf_porf_todo_todoDone21jjtype;
  f64 dong_porf_porf_todo_todoId21;
  i32 dong_porf_porf_todo_todoId21jjtype;
  f64 dong_porf_porf_todo_todoText22;
  i32 dong_porf_porf_todo_todoText22jjtype;
  f64 dong_porf_porf_todo_todoDone22;
  i32 dong_porf_porf_todo_todoDone22jjtype;
  f64 dong_porf_porf_todo_todoId22;
  i32 dong_porf_porf_todo_todoId22jjtype;
  f64 dong_porf_porf_todo_todoText23;
  i32 dong_porf_porf_todo_todoText23jjtype;
  f64 dong_porf_porf_todo_todoDone23;
  i32 dong_porf_porf_todo_todoDone23jjtype;
  f64 dong_porf_porf_todo_todoId23;
  i32 dong_porf_porf_todo_todoId23jjtype;
  f64 dong_porf_porf_todo_todoText24;
  i32 dong_porf_porf_todo_todoText24jjtype;
  f64 dong_porf_porf_todo_todoDone24;
  i32 dong_porf_porf_todo_todoDone24jjtype;
  f64 dong_porf_porf_todo_todoId24;
  i32 dong_porf_porf_todo_todoId24jjtype;
  f64 dong_porf_porf_todo_todoText25;
  i32 dong_porf_porf_todo_todoText25jjtype;
  f64 dong_porf_porf_todo_todoDone25;
  i32 dong_porf_porf_todo_todoDone25jjtype;
  f64 dong_porf_porf_todo_todoId25;
  i32 dong_porf_porf_todo_todoId25jjtype;
  f64 dong_porf_porf_todo_todoText26;
  i32 dong_porf_porf_todo_todoText26jjtype;
  f64 dong_porf_porf_todo_todoDone26;
  i32 dong_porf_porf_todo_todoDone26jjtype;
  f64 dong_porf_porf_todo_todoId26;
  i32 dong_porf_porf_todo_todoId26jjtype;
  f64 dong_porf_porf_todo_todoText27;
  i32 dong_porf_porf_todo_todoText27jjtype;
  f64 dong_porf_porf_todo_todoDone27;
  i32 dong_porf_porf_todo_todoDone27jjtype;
  f64 dong_porf_porf_todo_todoId27;
  i32 dong_porf_porf_todo_todoId27jjtype;
  f64 dong_porf_porf_todo_todoText28;
  i32 dong_porf_porf_todo_todoText28jjtype;
  f64 dong_porf_porf_todo_todoDone28;
  i32 dong_porf_porf_todo_todoDone28jjtype;
  f64 dong_porf_porf_todo_todoId28;
  i32 dong_porf_porf_todo_todoId28jjtype;
  f64 dong_porf_porf_todo_todoText29;
  i32 dong_porf_porf_todo_todoText29jjtype;
  f64 dong_porf_porf_todo_todoDone29;
  i32 dong_porf_porf_todo_todoDone29jjtype;
  f64 dong_porf_porf_todo_todoId29;
  i32 dong_porf_porf_todo_todoId29jjtype;
  f64 dong_porf_porf_todo_todoText30;
  i32 dong_porf_porf_todo_todoText30jjtype;
  f64 dong_porf_porf_todo_todoDone30;
  i32 dong_porf_porf_todo_todoDone30jjtype;
  f64 dong_porf_porf_todo_todoId30;
  i32 dong_porf_porf_todo_todoId30jjtype;
  f64 dong_porf_porf_todo_todoText31;
  i32 dong_porf_porf_todo_todoText31jjtype;
  f64 dong_porf_porf_todo_todoDone31;
  i32 dong_porf_porf_todo_todoDone31jjtype;
  f64 dong_porf_porf_todo_todoId31;
  i32 dong_porf_porf_todo_todoId31jjtype;
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
  out->dong_porf_porf_todo_porf_rootId = dong_porf_porf_todo_porf_rootId;
  out->dong_porf_porf_todo_porf_rootIdjjtype = dong_porf_porf_todo_porf_rootIdjjtype;
  out->dong_porf_porf_todo_titleId = dong_porf_porf_todo_titleId;
  out->dong_porf_porf_todo_titleIdjjtype = dong_porf_porf_todo_titleIdjjtype;
  out->dong_porf_porf_todo_todo_inputId = dong_porf_porf_todo_todo_inputId;
  out->dong_porf_porf_todo_todo_inputIdjjtype = dong_porf_porf_todo_todo_inputIdjjtype;
  out->dong_porf_porf_todo_btn_addId = dong_porf_porf_todo_btn_addId;
  out->dong_porf_porf_todo_btn_addIdjjtype = dong_porf_porf_todo_btn_addIdjjtype;
  out->dong_porf_porf_todo_filter_barId = dong_porf_porf_todo_filter_barId;
  out->dong_porf_porf_todo_filter_barIdjjtype = dong_porf_porf_todo_filter_barIdjjtype;
  out->dong_porf_porf_todo_filter_allId = dong_porf_porf_todo_filter_allId;
  out->dong_porf_porf_todo_filter_allIdjjtype = dong_porf_porf_todo_filter_allIdjjtype;
  out->dong_porf_porf_todo_filter_activeId = dong_porf_porf_todo_filter_activeId;
  out->dong_porf_porf_todo_filter_activeIdjjtype = dong_porf_porf_todo_filter_activeIdjjtype;
  out->dong_porf_porf_todo_filter_doneId = dong_porf_porf_todo_filter_doneId;
  out->dong_porf_porf_todo_filter_doneIdjjtype = dong_porf_porf_todo_filter_doneIdjjtype;
  out->dong_porf_porf_todo_todo_listId = dong_porf_porf_todo_todo_listId;
  out->dong_porf_porf_todo_todo_listIdjjtype = dong_porf_porf_todo_todo_listIdjjtype;
  out->dong_porf_porf_todo_clear_wrapId = dong_porf_porf_todo_clear_wrapId;
  out->dong_porf_porf_todo_clear_wrapIdjjtype = dong_porf_porf_todo_clear_wrapIdjjtype;
  out->dong_porf_porf_todo_btn_clearId = dong_porf_porf_todo_btn_clearId;
  out->dong_porf_porf_todo_btn_clearIdjjtype = dong_porf_porf_todo_btn_clearIdjjtype;
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
  out->dong_porf_porf_todo_showClear = dong_porf_porf_todo_showClear;
  out->dong_porf_porf_todo_showClearjjtype = dong_porf_porf_todo_showClearjjtype;
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
  out->dong_porf_porf_todo_todoText16 = dong_porf_porf_todo_todoText16;
  out->dong_porf_porf_todo_todoText16jjtype = dong_porf_porf_todo_todoText16jjtype;
  out->dong_porf_porf_todo_todoDone16 = dong_porf_porf_todo_todoDone16;
  out->dong_porf_porf_todo_todoDone16jjtype = dong_porf_porf_todo_todoDone16jjtype;
  out->dong_porf_porf_todo_todoId16 = dong_porf_porf_todo_todoId16;
  out->dong_porf_porf_todo_todoId16jjtype = dong_porf_porf_todo_todoId16jjtype;
  out->dong_porf_porf_todo_todoText17 = dong_porf_porf_todo_todoText17;
  out->dong_porf_porf_todo_todoText17jjtype = dong_porf_porf_todo_todoText17jjtype;
  out->dong_porf_porf_todo_todoDone17 = dong_porf_porf_todo_todoDone17;
  out->dong_porf_porf_todo_todoDone17jjtype = dong_porf_porf_todo_todoDone17jjtype;
  out->dong_porf_porf_todo_todoId17 = dong_porf_porf_todo_todoId17;
  out->dong_porf_porf_todo_todoId17jjtype = dong_porf_porf_todo_todoId17jjtype;
  out->dong_porf_porf_todo_todoText18 = dong_porf_porf_todo_todoText18;
  out->dong_porf_porf_todo_todoText18jjtype = dong_porf_porf_todo_todoText18jjtype;
  out->dong_porf_porf_todo_todoDone18 = dong_porf_porf_todo_todoDone18;
  out->dong_porf_porf_todo_todoDone18jjtype = dong_porf_porf_todo_todoDone18jjtype;
  out->dong_porf_porf_todo_todoId18 = dong_porf_porf_todo_todoId18;
  out->dong_porf_porf_todo_todoId18jjtype = dong_porf_porf_todo_todoId18jjtype;
  out->dong_porf_porf_todo_todoText19 = dong_porf_porf_todo_todoText19;
  out->dong_porf_porf_todo_todoText19jjtype = dong_porf_porf_todo_todoText19jjtype;
  out->dong_porf_porf_todo_todoDone19 = dong_porf_porf_todo_todoDone19;
  out->dong_porf_porf_todo_todoDone19jjtype = dong_porf_porf_todo_todoDone19jjtype;
  out->dong_porf_porf_todo_todoId19 = dong_porf_porf_todo_todoId19;
  out->dong_porf_porf_todo_todoId19jjtype = dong_porf_porf_todo_todoId19jjtype;
  out->dong_porf_porf_todo_todoText20 = dong_porf_porf_todo_todoText20;
  out->dong_porf_porf_todo_todoText20jjtype = dong_porf_porf_todo_todoText20jjtype;
  out->dong_porf_porf_todo_todoDone20 = dong_porf_porf_todo_todoDone20;
  out->dong_porf_porf_todo_todoDone20jjtype = dong_porf_porf_todo_todoDone20jjtype;
  out->dong_porf_porf_todo_todoId20 = dong_porf_porf_todo_todoId20;
  out->dong_porf_porf_todo_todoId20jjtype = dong_porf_porf_todo_todoId20jjtype;
  out->dong_porf_porf_todo_todoText21 = dong_porf_porf_todo_todoText21;
  out->dong_porf_porf_todo_todoText21jjtype = dong_porf_porf_todo_todoText21jjtype;
  out->dong_porf_porf_todo_todoDone21 = dong_porf_porf_todo_todoDone21;
  out->dong_porf_porf_todo_todoDone21jjtype = dong_porf_porf_todo_todoDone21jjtype;
  out->dong_porf_porf_todo_todoId21 = dong_porf_porf_todo_todoId21;
  out->dong_porf_porf_todo_todoId21jjtype = dong_porf_porf_todo_todoId21jjtype;
  out->dong_porf_porf_todo_todoText22 = dong_porf_porf_todo_todoText22;
  out->dong_porf_porf_todo_todoText22jjtype = dong_porf_porf_todo_todoText22jjtype;
  out->dong_porf_porf_todo_todoDone22 = dong_porf_porf_todo_todoDone22;
  out->dong_porf_porf_todo_todoDone22jjtype = dong_porf_porf_todo_todoDone22jjtype;
  out->dong_porf_porf_todo_todoId22 = dong_porf_porf_todo_todoId22;
  out->dong_porf_porf_todo_todoId22jjtype = dong_porf_porf_todo_todoId22jjtype;
  out->dong_porf_porf_todo_todoText23 = dong_porf_porf_todo_todoText23;
  out->dong_porf_porf_todo_todoText23jjtype = dong_porf_porf_todo_todoText23jjtype;
  out->dong_porf_porf_todo_todoDone23 = dong_porf_porf_todo_todoDone23;
  out->dong_porf_porf_todo_todoDone23jjtype = dong_porf_porf_todo_todoDone23jjtype;
  out->dong_porf_porf_todo_todoId23 = dong_porf_porf_todo_todoId23;
  out->dong_porf_porf_todo_todoId23jjtype = dong_porf_porf_todo_todoId23jjtype;
  out->dong_porf_porf_todo_todoText24 = dong_porf_porf_todo_todoText24;
  out->dong_porf_porf_todo_todoText24jjtype = dong_porf_porf_todo_todoText24jjtype;
  out->dong_porf_porf_todo_todoDone24 = dong_porf_porf_todo_todoDone24;
  out->dong_porf_porf_todo_todoDone24jjtype = dong_porf_porf_todo_todoDone24jjtype;
  out->dong_porf_porf_todo_todoId24 = dong_porf_porf_todo_todoId24;
  out->dong_porf_porf_todo_todoId24jjtype = dong_porf_porf_todo_todoId24jjtype;
  out->dong_porf_porf_todo_todoText25 = dong_porf_porf_todo_todoText25;
  out->dong_porf_porf_todo_todoText25jjtype = dong_porf_porf_todo_todoText25jjtype;
  out->dong_porf_porf_todo_todoDone25 = dong_porf_porf_todo_todoDone25;
  out->dong_porf_porf_todo_todoDone25jjtype = dong_porf_porf_todo_todoDone25jjtype;
  out->dong_porf_porf_todo_todoId25 = dong_porf_porf_todo_todoId25;
  out->dong_porf_porf_todo_todoId25jjtype = dong_porf_porf_todo_todoId25jjtype;
  out->dong_porf_porf_todo_todoText26 = dong_porf_porf_todo_todoText26;
  out->dong_porf_porf_todo_todoText26jjtype = dong_porf_porf_todo_todoText26jjtype;
  out->dong_porf_porf_todo_todoDone26 = dong_porf_porf_todo_todoDone26;
  out->dong_porf_porf_todo_todoDone26jjtype = dong_porf_porf_todo_todoDone26jjtype;
  out->dong_porf_porf_todo_todoId26 = dong_porf_porf_todo_todoId26;
  out->dong_porf_porf_todo_todoId26jjtype = dong_porf_porf_todo_todoId26jjtype;
  out->dong_porf_porf_todo_todoText27 = dong_porf_porf_todo_todoText27;
  out->dong_porf_porf_todo_todoText27jjtype = dong_porf_porf_todo_todoText27jjtype;
  out->dong_porf_porf_todo_todoDone27 = dong_porf_porf_todo_todoDone27;
  out->dong_porf_porf_todo_todoDone27jjtype = dong_porf_porf_todo_todoDone27jjtype;
  out->dong_porf_porf_todo_todoId27 = dong_porf_porf_todo_todoId27;
  out->dong_porf_porf_todo_todoId27jjtype = dong_porf_porf_todo_todoId27jjtype;
  out->dong_porf_porf_todo_todoText28 = dong_porf_porf_todo_todoText28;
  out->dong_porf_porf_todo_todoText28jjtype = dong_porf_porf_todo_todoText28jjtype;
  out->dong_porf_porf_todo_todoDone28 = dong_porf_porf_todo_todoDone28;
  out->dong_porf_porf_todo_todoDone28jjtype = dong_porf_porf_todo_todoDone28jjtype;
  out->dong_porf_porf_todo_todoId28 = dong_porf_porf_todo_todoId28;
  out->dong_porf_porf_todo_todoId28jjtype = dong_porf_porf_todo_todoId28jjtype;
  out->dong_porf_porf_todo_todoText29 = dong_porf_porf_todo_todoText29;
  out->dong_porf_porf_todo_todoText29jjtype = dong_porf_porf_todo_todoText29jjtype;
  out->dong_porf_porf_todo_todoDone29 = dong_porf_porf_todo_todoDone29;
  out->dong_porf_porf_todo_todoDone29jjtype = dong_porf_porf_todo_todoDone29jjtype;
  out->dong_porf_porf_todo_todoId29 = dong_porf_porf_todo_todoId29;
  out->dong_porf_porf_todo_todoId29jjtype = dong_porf_porf_todo_todoId29jjtype;
  out->dong_porf_porf_todo_todoText30 = dong_porf_porf_todo_todoText30;
  out->dong_porf_porf_todo_todoText30jjtype = dong_porf_porf_todo_todoText30jjtype;
  out->dong_porf_porf_todo_todoDone30 = dong_porf_porf_todo_todoDone30;
  out->dong_porf_porf_todo_todoDone30jjtype = dong_porf_porf_todo_todoDone30jjtype;
  out->dong_porf_porf_todo_todoId30 = dong_porf_porf_todo_todoId30;
  out->dong_porf_porf_todo_todoId30jjtype = dong_porf_porf_todo_todoId30jjtype;
  out->dong_porf_porf_todo_todoText31 = dong_porf_porf_todo_todoText31;
  out->dong_porf_porf_todo_todoText31jjtype = dong_porf_porf_todo_todoText31jjtype;
  out->dong_porf_porf_todo_todoDone31 = dong_porf_porf_todo_todoDone31;
  out->dong_porf_porf_todo_todoDone31jjtype = dong_porf_porf_todo_todoDone31jjtype;
  out->dong_porf_porf_todo_todoId31 = dong_porf_porf_todo_todoId31;
  out->dong_porf_porf_todo_todoId31jjtype = dong_porf_porf_todo_todoId31jjtype;
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
  dong_porf_porf_todo_porf_rootId = in->dong_porf_porf_todo_porf_rootId;
  dong_porf_porf_todo_porf_rootIdjjtype = in->dong_porf_porf_todo_porf_rootIdjjtype;
  dong_porf_porf_todo_titleId = in->dong_porf_porf_todo_titleId;
  dong_porf_porf_todo_titleIdjjtype = in->dong_porf_porf_todo_titleIdjjtype;
  dong_porf_porf_todo_todo_inputId = in->dong_porf_porf_todo_todo_inputId;
  dong_porf_porf_todo_todo_inputIdjjtype = in->dong_porf_porf_todo_todo_inputIdjjtype;
  dong_porf_porf_todo_btn_addId = in->dong_porf_porf_todo_btn_addId;
  dong_porf_porf_todo_btn_addIdjjtype = in->dong_porf_porf_todo_btn_addIdjjtype;
  dong_porf_porf_todo_filter_barId = in->dong_porf_porf_todo_filter_barId;
  dong_porf_porf_todo_filter_barIdjjtype = in->dong_porf_porf_todo_filter_barIdjjtype;
  dong_porf_porf_todo_filter_allId = in->dong_porf_porf_todo_filter_allId;
  dong_porf_porf_todo_filter_allIdjjtype = in->dong_porf_porf_todo_filter_allIdjjtype;
  dong_porf_porf_todo_filter_activeId = in->dong_porf_porf_todo_filter_activeId;
  dong_porf_porf_todo_filter_activeIdjjtype = in->dong_porf_porf_todo_filter_activeIdjjtype;
  dong_porf_porf_todo_filter_doneId = in->dong_porf_porf_todo_filter_doneId;
  dong_porf_porf_todo_filter_doneIdjjtype = in->dong_porf_porf_todo_filter_doneIdjjtype;
  dong_porf_porf_todo_todo_listId = in->dong_porf_porf_todo_todo_listId;
  dong_porf_porf_todo_todo_listIdjjtype = in->dong_porf_porf_todo_todo_listIdjjtype;
  dong_porf_porf_todo_clear_wrapId = in->dong_porf_porf_todo_clear_wrapId;
  dong_porf_porf_todo_clear_wrapIdjjtype = in->dong_porf_porf_todo_clear_wrapIdjjtype;
  dong_porf_porf_todo_btn_clearId = in->dong_porf_porf_todo_btn_clearId;
  dong_porf_porf_todo_btn_clearIdjjtype = in->dong_porf_porf_todo_btn_clearIdjjtype;
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
  dong_porf_porf_todo_showClear = in->dong_porf_porf_todo_showClear;
  dong_porf_porf_todo_showClearjjtype = in->dong_porf_porf_todo_showClearjjtype;
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
  dong_porf_porf_todo_todoText16 = in->dong_porf_porf_todo_todoText16;
  dong_porf_porf_todo_todoText16jjtype = in->dong_porf_porf_todo_todoText16jjtype;
  dong_porf_porf_todo_todoDone16 = in->dong_porf_porf_todo_todoDone16;
  dong_porf_porf_todo_todoDone16jjtype = in->dong_porf_porf_todo_todoDone16jjtype;
  dong_porf_porf_todo_todoId16 = in->dong_porf_porf_todo_todoId16;
  dong_porf_porf_todo_todoId16jjtype = in->dong_porf_porf_todo_todoId16jjtype;
  dong_porf_porf_todo_todoText17 = in->dong_porf_porf_todo_todoText17;
  dong_porf_porf_todo_todoText17jjtype = in->dong_porf_porf_todo_todoText17jjtype;
  dong_porf_porf_todo_todoDone17 = in->dong_porf_porf_todo_todoDone17;
  dong_porf_porf_todo_todoDone17jjtype = in->dong_porf_porf_todo_todoDone17jjtype;
  dong_porf_porf_todo_todoId17 = in->dong_porf_porf_todo_todoId17;
  dong_porf_porf_todo_todoId17jjtype = in->dong_porf_porf_todo_todoId17jjtype;
  dong_porf_porf_todo_todoText18 = in->dong_porf_porf_todo_todoText18;
  dong_porf_porf_todo_todoText18jjtype = in->dong_porf_porf_todo_todoText18jjtype;
  dong_porf_porf_todo_todoDone18 = in->dong_porf_porf_todo_todoDone18;
  dong_porf_porf_todo_todoDone18jjtype = in->dong_porf_porf_todo_todoDone18jjtype;
  dong_porf_porf_todo_todoId18 = in->dong_porf_porf_todo_todoId18;
  dong_porf_porf_todo_todoId18jjtype = in->dong_porf_porf_todo_todoId18jjtype;
  dong_porf_porf_todo_todoText19 = in->dong_porf_porf_todo_todoText19;
  dong_porf_porf_todo_todoText19jjtype = in->dong_porf_porf_todo_todoText19jjtype;
  dong_porf_porf_todo_todoDone19 = in->dong_porf_porf_todo_todoDone19;
  dong_porf_porf_todo_todoDone19jjtype = in->dong_porf_porf_todo_todoDone19jjtype;
  dong_porf_porf_todo_todoId19 = in->dong_porf_porf_todo_todoId19;
  dong_porf_porf_todo_todoId19jjtype = in->dong_porf_porf_todo_todoId19jjtype;
  dong_porf_porf_todo_todoText20 = in->dong_porf_porf_todo_todoText20;
  dong_porf_porf_todo_todoText20jjtype = in->dong_porf_porf_todo_todoText20jjtype;
  dong_porf_porf_todo_todoDone20 = in->dong_porf_porf_todo_todoDone20;
  dong_porf_porf_todo_todoDone20jjtype = in->dong_porf_porf_todo_todoDone20jjtype;
  dong_porf_porf_todo_todoId20 = in->dong_porf_porf_todo_todoId20;
  dong_porf_porf_todo_todoId20jjtype = in->dong_porf_porf_todo_todoId20jjtype;
  dong_porf_porf_todo_todoText21 = in->dong_porf_porf_todo_todoText21;
  dong_porf_porf_todo_todoText21jjtype = in->dong_porf_porf_todo_todoText21jjtype;
  dong_porf_porf_todo_todoDone21 = in->dong_porf_porf_todo_todoDone21;
  dong_porf_porf_todo_todoDone21jjtype = in->dong_porf_porf_todo_todoDone21jjtype;
  dong_porf_porf_todo_todoId21 = in->dong_porf_porf_todo_todoId21;
  dong_porf_porf_todo_todoId21jjtype = in->dong_porf_porf_todo_todoId21jjtype;
  dong_porf_porf_todo_todoText22 = in->dong_porf_porf_todo_todoText22;
  dong_porf_porf_todo_todoText22jjtype = in->dong_porf_porf_todo_todoText22jjtype;
  dong_porf_porf_todo_todoDone22 = in->dong_porf_porf_todo_todoDone22;
  dong_porf_porf_todo_todoDone22jjtype = in->dong_porf_porf_todo_todoDone22jjtype;
  dong_porf_porf_todo_todoId22 = in->dong_porf_porf_todo_todoId22;
  dong_porf_porf_todo_todoId22jjtype = in->dong_porf_porf_todo_todoId22jjtype;
  dong_porf_porf_todo_todoText23 = in->dong_porf_porf_todo_todoText23;
  dong_porf_porf_todo_todoText23jjtype = in->dong_porf_porf_todo_todoText23jjtype;
  dong_porf_porf_todo_todoDone23 = in->dong_porf_porf_todo_todoDone23;
  dong_porf_porf_todo_todoDone23jjtype = in->dong_porf_porf_todo_todoDone23jjtype;
  dong_porf_porf_todo_todoId23 = in->dong_porf_porf_todo_todoId23;
  dong_porf_porf_todo_todoId23jjtype = in->dong_porf_porf_todo_todoId23jjtype;
  dong_porf_porf_todo_todoText24 = in->dong_porf_porf_todo_todoText24;
  dong_porf_porf_todo_todoText24jjtype = in->dong_porf_porf_todo_todoText24jjtype;
  dong_porf_porf_todo_todoDone24 = in->dong_porf_porf_todo_todoDone24;
  dong_porf_porf_todo_todoDone24jjtype = in->dong_porf_porf_todo_todoDone24jjtype;
  dong_porf_porf_todo_todoId24 = in->dong_porf_porf_todo_todoId24;
  dong_porf_porf_todo_todoId24jjtype = in->dong_porf_porf_todo_todoId24jjtype;
  dong_porf_porf_todo_todoText25 = in->dong_porf_porf_todo_todoText25;
  dong_porf_porf_todo_todoText25jjtype = in->dong_porf_porf_todo_todoText25jjtype;
  dong_porf_porf_todo_todoDone25 = in->dong_porf_porf_todo_todoDone25;
  dong_porf_porf_todo_todoDone25jjtype = in->dong_porf_porf_todo_todoDone25jjtype;
  dong_porf_porf_todo_todoId25 = in->dong_porf_porf_todo_todoId25;
  dong_porf_porf_todo_todoId25jjtype = in->dong_porf_porf_todo_todoId25jjtype;
  dong_porf_porf_todo_todoText26 = in->dong_porf_porf_todo_todoText26;
  dong_porf_porf_todo_todoText26jjtype = in->dong_porf_porf_todo_todoText26jjtype;
  dong_porf_porf_todo_todoDone26 = in->dong_porf_porf_todo_todoDone26;
  dong_porf_porf_todo_todoDone26jjtype = in->dong_porf_porf_todo_todoDone26jjtype;
  dong_porf_porf_todo_todoId26 = in->dong_porf_porf_todo_todoId26;
  dong_porf_porf_todo_todoId26jjtype = in->dong_porf_porf_todo_todoId26jjtype;
  dong_porf_porf_todo_todoText27 = in->dong_porf_porf_todo_todoText27;
  dong_porf_porf_todo_todoText27jjtype = in->dong_porf_porf_todo_todoText27jjtype;
  dong_porf_porf_todo_todoDone27 = in->dong_porf_porf_todo_todoDone27;
  dong_porf_porf_todo_todoDone27jjtype = in->dong_porf_porf_todo_todoDone27jjtype;
  dong_porf_porf_todo_todoId27 = in->dong_porf_porf_todo_todoId27;
  dong_porf_porf_todo_todoId27jjtype = in->dong_porf_porf_todo_todoId27jjtype;
  dong_porf_porf_todo_todoText28 = in->dong_porf_porf_todo_todoText28;
  dong_porf_porf_todo_todoText28jjtype = in->dong_porf_porf_todo_todoText28jjtype;
  dong_porf_porf_todo_todoDone28 = in->dong_porf_porf_todo_todoDone28;
  dong_porf_porf_todo_todoDone28jjtype = in->dong_porf_porf_todo_todoDone28jjtype;
  dong_porf_porf_todo_todoId28 = in->dong_porf_porf_todo_todoId28;
  dong_porf_porf_todo_todoId28jjtype = in->dong_porf_porf_todo_todoId28jjtype;
  dong_porf_porf_todo_todoText29 = in->dong_porf_porf_todo_todoText29;
  dong_porf_porf_todo_todoText29jjtype = in->dong_porf_porf_todo_todoText29jjtype;
  dong_porf_porf_todo_todoDone29 = in->dong_porf_porf_todo_todoDone29;
  dong_porf_porf_todo_todoDone29jjtype = in->dong_porf_porf_todo_todoDone29jjtype;
  dong_porf_porf_todo_todoId29 = in->dong_porf_porf_todo_todoId29;
  dong_porf_porf_todo_todoId29jjtype = in->dong_porf_porf_todo_todoId29jjtype;
  dong_porf_porf_todo_todoText30 = in->dong_porf_porf_todo_todoText30;
  dong_porf_porf_todo_todoText30jjtype = in->dong_porf_porf_todo_todoText30jjtype;
  dong_porf_porf_todo_todoDone30 = in->dong_porf_porf_todo_todoDone30;
  dong_porf_porf_todo_todoDone30jjtype = in->dong_porf_porf_todo_todoDone30jjtype;
  dong_porf_porf_todo_todoId30 = in->dong_porf_porf_todo_todoId30;
  dong_porf_porf_todo_todoId30jjtype = in->dong_porf_porf_todo_todoId30jjtype;
  dong_porf_porf_todo_todoText31 = in->dong_porf_porf_todo_todoText31;
  dong_porf_porf_todo_todoText31jjtype = in->dong_porf_porf_todo_todoText31jjtype;
  dong_porf_porf_todo_todoDone31 = in->dong_porf_porf_todo_todoDone31;
  dong_porf_porf_todo_todoDone31jjtype = in->dong_porf_porf_todo_todoDone31jjtype;
  dong_porf_porf_todo_todoId31 = in->dong_porf_porf_todo_todoId31;
  dong_porf_porf_todo_todoId31jjtype = in->dong_porf_porf_todo_todoId31jjtype;
}
