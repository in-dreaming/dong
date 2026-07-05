// Porffor module: js_microbench
#include "dong_porf_runtime.h"
#ifndef __porf_nan
#define __porf_nan (NAN)
#endif
#ifndef __porf_infinity
#define __porf_infinity (INFINITY)
#endif

char* dong_porf_js_microbench_memory; u32 dong_porf_js_microbench_memory_pages = 2;


__attribute__((import_module(""), import_name("r")))
extern f64 __porf_import_dong_time_now();
static i32 i32_load(i32 align, i32 offset, i32 pointer) {
  return *((i32*)(dong_porf_js_microbench_memory + offset + pointer));
}

static f64 f64_load(i32 align, i32 offset, i32 pointer) {
  return *((f64*)(dong_porf_js_microbench_memory + offset + pointer));
}

static i32 i32_load16_u(i32 align, i32 offset, i32 pointer) {
  return *((u16*)(dong_porf_js_microbench_memory + offset + pointer));
}

static void i32_store16(i32 align, i32 offset, i32 pointer, u16 value) {
  *((u16*)(dong_porf_js_microbench_memory + offset + pointer)) = value;
}

static void i32_store(i32 align, i32 offset, i32 pointer, i32 value) {
  *((i32*)(dong_porf_js_microbench_memory + offset + pointer)) = value;
}

static void f64_store(i32 align, i32 offset, i32 pointer, f64 value) {
  *((f64*)(dong_porf_js_microbench_memory + offset + pointer)) = value;
}

static i32 i32_load8_u(i32 align, i32 offset, i32 pointer) {
  return *((u8*)(dong_porf_js_microbench_memory + offset + pointer));
}

static void i32_store8(i32 align, i32 offset, i32 pointer, u8 value) {
  *((u8*)(dong_porf_js_microbench_memory + offset + pointer)) = value;
}


__attribute__((import_module(""), import_name("q")))
extern void __porf_import_dong_bench_log(f64);

__attribute__((import_module(""), import_name("i")))
extern void __porf_import_dong_print(f64);
void dong_porf_js_microbench__porf_init(void) {
  if (dong_porf_js_microbench_memory) return;
  dong_porf_js_microbench_memory = calloc(1, dong_porf_js_microbench_memory_pages * 65536);

  dong_porf_js_microbench_memory[16]=(u8)15;dong_porf_js_microbench_memory[20]=(u8)112;dong_porf_js_microbench_memory[21]=(u8)114;dong_porf_js_microbench_memory[22]=(u8)111;dong_porf_js_microbench_memory[23]=(u8)112;dong_porf_js_microbench_memory[24]=(u8)101;dong_porf_js_microbench_memory[25]=(u8)114;dong_porf_js_microbench_memory[26]=(u8)116;dong_porf_js_microbench_memory[27]=(u8)121;dong_porf_js_microbench_memory[28]=(u8)95;dong_porf_js_microbench_memory[29]=(u8)97;dong_porf_js_microbench_memory[30]=(u8)99;dong_porf_js_microbench_memory[31]=(u8)99;dong_porf_js_microbench_memory[32]=(u8)101;dong_porf_js_microbench_memory[33]=(u8)115;dong_porf_js_microbench_memory[34]=(u8)115;
  dong_porf_js_microbench_memory[37]=(u8)1;dong_porf_js_microbench_memory[41]=(u8)120;
  dong_porf_js_microbench_memory[44]=(u8)6;dong_porf_js_microbench_memory[48]=(u8)108;dong_porf_js_microbench_memory[49]=(u8)101;dong_porf_js_microbench_memory[50]=(u8)110;dong_porf_js_microbench_memory[51]=(u8)103;dong_porf_js_microbench_memory[52]=(u8)116;dong_porf_js_microbench_memory[53]=(u8)104;
  dong_porf_js_microbench_memory[56]=(u8)4;dong_porf_js_microbench_memory[60]=(u8)110;dong_porf_js_microbench_memory[61]=(u8)97;dong_porf_js_microbench_memory[62]=(u8)109;dong_porf_js_microbench_memory[63]=(u8)101;
  dong_porf_js_microbench_memory[66]=(u8)9;dong_porf_js_microbench_memory[70]=(u8)112;dong_porf_js_microbench_memory[71]=(u8)114;dong_porf_js_microbench_memory[72]=(u8)111;dong_porf_js_microbench_memory[73]=(u8)116;dong_porf_js_microbench_memory[74]=(u8)111;dong_porf_js_microbench_memory[75]=(u8)116;dong_porf_js_microbench_memory[76]=(u8)121;dong_porf_js_microbench_memory[77]=(u8)112;dong_porf_js_microbench_memory[78]=(u8)101;
  dong_porf_js_microbench_memory[81]=(u8)11;dong_porf_js_microbench_memory[85]=(u8)99;dong_porf_js_microbench_memory[86]=(u8)111;dong_porf_js_microbench_memory[87]=(u8)110;dong_porf_js_microbench_memory[88]=(u8)115;dong_porf_js_microbench_memory[89]=(u8)116;dong_porf_js_microbench_memory[90]=(u8)114;dong_porf_js_microbench_memory[91]=(u8)117;dong_porf_js_microbench_memory[92]=(u8)99;dong_porf_js_microbench_memory[93]=(u8)116;dong_porf_js_microbench_memory[94]=(u8)111;dong_porf_js_microbench_memory[95]=(u8)114;
  dong_porf_js_microbench_memory[98]=(u8)41;dong_porf_js_microbench_memory[102]=(u8)67;dong_porf_js_microbench_memory[103]=(u8)97;dong_porf_js_microbench_memory[104]=(u8)110;dong_porf_js_microbench_memory[105]=(u8)110;dong_porf_js_microbench_memory[106]=(u8)111;dong_porf_js_microbench_memory[107]=(u8)116;dong_porf_js_microbench_memory[108]=(u8)32;dong_porf_js_microbench_memory[109]=(u8)99;dong_porf_js_microbench_memory[110]=(u8)111;dong_porf_js_microbench_memory[111]=(u8)110;dong_porf_js_microbench_memory[112]=(u8)118;dong_porf_js_microbench_memory[113]=(u8)101;dong_porf_js_microbench_memory[114]=(u8)114;dong_porf_js_microbench_memory[115]=(u8)116;dong_porf_js_microbench_memory[116]=(u8)32;dong_porf_js_microbench_memory[117]=(u8)97;dong_porf_js_microbench_memory[118]=(u8)32;dong_porf_js_microbench_memory[119]=(u8)83;dong_porf_js_microbench_memory[120]=(u8)121;dong_porf_js_microbench_memory[121]=(u8)109;dong_porf_js_microbench_memory[122]=(u8)98;dong_porf_js_microbench_memory[123]=(u8)111;dong_porf_js_microbench_memory[124]=(u8)108;dong_porf_js_microbench_memory[125]=(u8)32;dong_porf_js_microbench_memory[126]=(u8)118;dong_porf_js_microbench_memory[127]=(u8)97;dong_porf_js_microbench_memory[128]=(u8)108;dong_porf_js_microbench_memory[129]=(u8)117;dong_porf_js_microbench_memory[130]=(u8)101;dong_porf_js_microbench_memory[131]=(u8)32;dong_porf_js_microbench_memory[132]=(u8)116;dong_porf_js_microbench_memory[133]=(u8)111;dong_porf_js_microbench_memory[134]=(u8)32;dong_porf_js_microbench_memory[135]=(u8)97;dong_porf_js_microbench_memory[136]=(u8)32;dong_porf_js_microbench_memory[137]=(u8)115;dong_porf_js_microbench_memory[138]=(u8)116;dong_porf_js_microbench_memory[139]=(u8)114;dong_porf_js_microbench_memory[140]=(u8)105;dong_porf_js_microbench_memory[141]=(u8)110;dong_porf_js_microbench_memory[142]=(u8)103;
  dong_porf_js_microbench_memory[145]=(u8)9;dong_porf_js_microbench_memory[149]=(u8)117;dong_porf_js_microbench_memory[150]=(u8)110;dong_porf_js_microbench_memory[151]=(u8)100;dong_porf_js_microbench_memory[152]=(u8)101;dong_porf_js_microbench_memory[153]=(u8)102;dong_porf_js_microbench_memory[154]=(u8)105;dong_porf_js_microbench_memory[155]=(u8)110;dong_porf_js_microbench_memory[156]=(u8)101;dong_porf_js_microbench_memory[157]=(u8)100;
  dong_porf_js_microbench_memory[160]=(u8)4;dong_porf_js_microbench_memory[164]=(u8)110;dong_porf_js_microbench_memory[165]=(u8)117;dong_porf_js_microbench_memory[166]=(u8)108;dong_porf_js_microbench_memory[167]=(u8)108;
  dong_porf_js_microbench_memory[170]=(u8)4;dong_porf_js_microbench_memory[174]=(u8)116;dong_porf_js_microbench_memory[175]=(u8)114;dong_porf_js_microbench_memory[176]=(u8)117;dong_porf_js_microbench_memory[177]=(u8)101;
  dong_porf_js_microbench_memory[180]=(u8)5;dong_porf_js_microbench_memory[184]=(u8)102;dong_porf_js_microbench_memory[185]=(u8)97;dong_porf_js_microbench_memory[186]=(u8)108;dong_porf_js_microbench_memory[187]=(u8)115;dong_porf_js_microbench_memory[188]=(u8)101;
  dong_porf_js_microbench_memory[191]=(u8)59;dong_porf_js_microbench_memory[195]=(u8)70;dong_porf_js_microbench_memory[196]=(u8)117;dong_porf_js_microbench_memory[197]=(u8)110;dong_porf_js_microbench_memory[198]=(u8)99;dong_porf_js_microbench_memory[199]=(u8)116;dong_porf_js_microbench_memory[200]=(u8)105;dong_porf_js_microbench_memory[201]=(u8)111;dong_porf_js_microbench_memory[202]=(u8)110;dong_porf_js_microbench_memory[203]=(u8)46;dong_porf_js_microbench_memory[204]=(u8)112;dong_porf_js_microbench_memory[205]=(u8)114;dong_porf_js_microbench_memory[206]=(u8)111;dong_porf_js_microbench_memory[207]=(u8)116;dong_porf_js_microbench_memory[208]=(u8)111;dong_porf_js_microbench_memory[209]=(u8)116;dong_porf_js_microbench_memory[210]=(u8)121;dong_porf_js_microbench_memory[211]=(u8)112;dong_porf_js_microbench_memory[212]=(u8)101;dong_porf_js_microbench_memory[213]=(u8)46;dong_porf_js_microbench_memory[214]=(u8)116;dong_porf_js_microbench_memory[215]=(u8)111;dong_porf_js_microbench_memory[216]=(u8)83;dong_porf_js_microbench_memory[217]=(u8)116;dong_porf_js_microbench_memory[218]=(u8)114;dong_porf_js_microbench_memory[219]=(u8)105;dong_porf_js_microbench_memory[220]=(u8)110;dong_porf_js_microbench_memory[221]=(u8)103;dong_porf_js_microbench_memory[222]=(u8)32;dong_porf_js_microbench_memory[223]=(u8)101;dong_porf_js_microbench_memory[224]=(u8)120;dong_porf_js_microbench_memory[225]=(u8)112;dong_porf_js_microbench_memory[226]=(u8)101;dong_porf_js_microbench_memory[227]=(u8)99;dong_porf_js_microbench_memory[228]=(u8)116;dong_porf_js_microbench_memory[229]=(u8)115;dong_porf_js_microbench_memory[230]=(u8)32;dong_porf_js_microbench_memory[231]=(u8)39;dong_porf_js_microbench_memory[232]=(u8)116;dong_porf_js_microbench_memory[233]=(u8)104;dong_porf_js_microbench_memory[234]=(u8)105;dong_porf_js_microbench_memory[235]=(u8)115;dong_porf_js_microbench_memory[236]=(u8)39;dong_porf_js_microbench_memory[237]=(u8)32;dong_porf_js_microbench_memory[238]=(u8)116;dong_porf_js_microbench_memory[239]=(u8)111;dong_porf_js_microbench_memory[240]=(u8)32;dong_porf_js_microbench_memory[241]=(u8)98;dong_porf_js_microbench_memory[242]=(u8)101;dong_porf_js_microbench_memory[243]=(u8)32;dong_porf_js_microbench_memory[244]=(u8)97;dong_porf_js_microbench_memory[245]=(u8)32;dong_porf_js_microbench_memory[246]=(u8)70;dong_porf_js_microbench_memory[247]=(u8)117;dong_porf_js_microbench_memory[248]=(u8)110;dong_porf_js_microbench_memory[249]=(u8)99;dong_porf_js_microbench_memory[250]=(u8)116;dong_porf_js_microbench_memory[251]=(u8)105;dong_porf_js_microbench_memory[252]=(u8)111;dong_porf_js_microbench_memory[253]=(u8)110;
  dong_porf_js_microbench_memory[256]=(u8)9;dong_porf_js_microbench_memory[260]=(u8)102;dong_porf_js_microbench_memory[261]=(u8)117;dong_porf_js_microbench_memory[262]=(u8)110;dong_porf_js_microbench_memory[263]=(u8)99;dong_porf_js_microbench_memory[264]=(u8)116;dong_porf_js_microbench_memory[265]=(u8)105;dong_porf_js_microbench_memory[266]=(u8)111;dong_porf_js_microbench_memory[267]=(u8)110;dong_porf_js_microbench_memory[268]=(u8)32;
  dong_porf_js_microbench_memory[271]=(u8)20;dong_porf_js_microbench_memory[275]=(u8)40;dong_porf_js_microbench_memory[276]=(u8)41;dong_porf_js_microbench_memory[277]=(u8)32;dong_porf_js_microbench_memory[278]=(u8)123;dong_porf_js_microbench_memory[279]=(u8)32;dong_porf_js_microbench_memory[280]=(u8)91;dong_porf_js_microbench_memory[281]=(u8)110;dong_porf_js_microbench_memory[282]=(u8)97;dong_porf_js_microbench_memory[283]=(u8)116;dong_porf_js_microbench_memory[284]=(u8)105;dong_porf_js_microbench_memory[285]=(u8)118;dong_porf_js_microbench_memory[286]=(u8)101;dong_porf_js_microbench_memory[287]=(u8)32;dong_porf_js_microbench_memory[288]=(u8)99;dong_porf_js_microbench_memory[289]=(u8)111;dong_porf_js_microbench_memory[290]=(u8)100;dong_porf_js_microbench_memory[291]=(u8)101;dong_porf_js_microbench_memory[292]=(u8)93;dong_porf_js_microbench_memory[293]=(u8)32;dong_porf_js_microbench_memory[294]=(u8)125;
  dong_porf_js_microbench_memory[297]=(u8)9;dong_porf_js_microbench_memory[301]=(u8)84;dong_porf_js_microbench_memory[302]=(u8)121;dong_porf_js_microbench_memory[303]=(u8)112;dong_porf_js_microbench_memory[304]=(u8)101;dong_porf_js_microbench_memory[305]=(u8)69;dong_porf_js_microbench_memory[306]=(u8)114;dong_porf_js_microbench_memory[307]=(u8)114;dong_porf_js_microbench_memory[308]=(u8)111;dong_porf_js_microbench_memory[309]=(u8)114;
  dong_porf_js_microbench_memory[312]=(u8)2;dong_porf_js_microbench_memory[316]=(u8)58;dong_porf_js_microbench_memory[317]=(u8)32;
  dong_porf_js_microbench_memory[320]=(u8)8;dong_porf_js_microbench_memory[324]=(u8)116;dong_porf_js_microbench_memory[325]=(u8)111;dong_porf_js_microbench_memory[326]=(u8)83;dong_porf_js_microbench_memory[327]=(u8)116;dong_porf_js_microbench_memory[328]=(u8)114;dong_porf_js_microbench_memory[329]=(u8)105;dong_porf_js_microbench_memory[330]=(u8)110;dong_porf_js_microbench_memory[331]=(u8)103;
  dong_porf_js_microbench_memory[334]=(u8)27;dong_porf_js_microbench_memory[338]=(u8)67;dong_porf_js_microbench_memory[339]=(u8)97;dong_porf_js_microbench_memory[340]=(u8)110;dong_porf_js_microbench_memory[341]=(u8)110;dong_porf_js_microbench_memory[342]=(u8)111;dong_porf_js_microbench_memory[343]=(u8)116;dong_porf_js_microbench_memory[344]=(u8)32;dong_porf_js_microbench_memory[345]=(u8)103;dong_porf_js_microbench_memory[346]=(u8)101;dong_porf_js_microbench_memory[347]=(u8)116;dong_porf_js_microbench_memory[348]=(u8)32;dong_porf_js_microbench_memory[349]=(u8)112;dong_porf_js_microbench_memory[350]=(u8)114;dong_porf_js_microbench_memory[351]=(u8)111;dong_porf_js_microbench_memory[352]=(u8)112;dong_porf_js_microbench_memory[353]=(u8)101;dong_porf_js_microbench_memory[354]=(u8)114;dong_porf_js_microbench_memory[355]=(u8)116;dong_porf_js_microbench_memory[356]=(u8)121;dong_porf_js_microbench_memory[357]=(u8)32;dong_porf_js_microbench_memory[358]=(u8)111;dong_porf_js_microbench_memory[359]=(u8)102;dong_porf_js_microbench_memory[360]=(u8)32;dong_porf_js_microbench_memory[361]=(u8)110;dong_porf_js_microbench_memory[362]=(u8)117;dong_porf_js_microbench_memory[363]=(u8)108;dong_porf_js_microbench_memory[364]=(u8)108;
  dong_porf_js_microbench_memory[367]=(u8)14;dong_porf_js_microbench_memory[371]=(u8)104;dong_porf_js_microbench_memory[372]=(u8)97;dong_porf_js_microbench_memory[373]=(u8)115;dong_porf_js_microbench_memory[374]=(u8)79;dong_porf_js_microbench_memory[375]=(u8)119;dong_porf_js_microbench_memory[376]=(u8)110;dong_porf_js_microbench_memory[377]=(u8)80;dong_porf_js_microbench_memory[378]=(u8)114;dong_porf_js_microbench_memory[379]=(u8)111;dong_porf_js_microbench_memory[380]=(u8)112;dong_porf_js_microbench_memory[381]=(u8)101;dong_porf_js_microbench_memory[382]=(u8)114;dong_porf_js_microbench_memory[383]=(u8)116;dong_porf_js_microbench_memory[384]=(u8)121;
  dong_porf_js_microbench_memory[387]=(u8)36;dong_porf_js_microbench_memory[391]=(u8)65;dong_porf_js_microbench_memory[392]=(u8)114;dong_porf_js_microbench_memory[393]=(u8)103;dong_porf_js_microbench_memory[394]=(u8)117;dong_porf_js_microbench_memory[395]=(u8)109;dong_porf_js_microbench_memory[396]=(u8)101;dong_porf_js_microbench_memory[397]=(u8)110;dong_porf_js_microbench_memory[398]=(u8)116;dong_porf_js_microbench_memory[399]=(u8)32;dong_porf_js_microbench_memory[400]=(u8)105;dong_porf_js_microbench_memory[401]=(u8)115;dong_porf_js_microbench_memory[402]=(u8)32;dong_porf_js_microbench_memory[403]=(u8)110;dong_porf_js_microbench_memory[404]=(u8)117;dong_porf_js_microbench_memory[405]=(u8)108;dong_porf_js_microbench_memory[406]=(u8)108;dong_porf_js_microbench_memory[407]=(u8)105;dong_porf_js_microbench_memory[408]=(u8)115;dong_porf_js_microbench_memory[409]=(u8)104;dong_porf_js_microbench_memory[410]=(u8)44;dong_porf_js_microbench_memory[411]=(u8)32;dong_porf_js_microbench_memory[412]=(u8)101;dong_porf_js_microbench_memory[413]=(u8)120;dong_porf_js_microbench_memory[414]=(u8)112;dong_porf_js_microbench_memory[415]=(u8)101;dong_porf_js_microbench_memory[416]=(u8)99;dong_porf_js_microbench_memory[417]=(u8)116;dong_porf_js_microbench_memory[418]=(u8)101;dong_porf_js_microbench_memory[419]=(u8)100;dong_porf_js_microbench_memory[420]=(u8)32;dong_porf_js_microbench_memory[421]=(u8)111;dong_porf_js_microbench_memory[422]=(u8)98;dong_porf_js_microbench_memory[423]=(u8)106;dong_porf_js_microbench_memory[424]=(u8)101;dong_porf_js_microbench_memory[425]=(u8)99;dong_porf_js_microbench_memory[426]=(u8)116;
  dong_porf_js_microbench_memory[429]=(u8)26;dong_porf_js_microbench_memory[433]=(u8)65;dong_porf_js_microbench_memory[434]=(u8)114;dong_porf_js_microbench_memory[435]=(u8)103;dong_porf_js_microbench_memory[436]=(u8)117;dong_porf_js_microbench_memory[437]=(u8)109;dong_porf_js_microbench_memory[438]=(u8)101;dong_porf_js_microbench_memory[439]=(u8)110;dong_porf_js_microbench_memory[440]=(u8)116;dong_porf_js_microbench_memory[441]=(u8)32;dong_porf_js_microbench_memory[442]=(u8)99;dong_porf_js_microbench_memory[443]=(u8)97;dong_porf_js_microbench_memory[444]=(u8)110;dong_porf_js_microbench_memory[445]=(u8)110;dong_porf_js_microbench_memory[446]=(u8)111;dong_porf_js_microbench_memory[447]=(u8)116;dong_porf_js_microbench_memory[448]=(u8)32;dong_porf_js_microbench_memory[449]=(u8)98;dong_porf_js_microbench_memory[450]=(u8)101;dong_porf_js_microbench_memory[451]=(u8)32;dong_porf_js_microbench_memory[452]=(u8)110;dong_porf_js_microbench_memory[453]=(u8)117;dong_porf_js_microbench_memory[454]=(u8)108;dong_porf_js_microbench_memory[455]=(u8)108;dong_porf_js_microbench_memory[456]=(u8)105;dong_porf_js_microbench_memory[457]=(u8)115;dong_porf_js_microbench_memory[458]=(u8)104;
  dong_porf_js_microbench_memory[461]=(u8)43;dong_porf_js_microbench_memory[465]=(u8)67;dong_porf_js_microbench_memory[466]=(u8)97;dong_porf_js_microbench_memory[467]=(u8)108;dong_porf_js_microbench_memory[468]=(u8)108;dong_porf_js_microbench_memory[469]=(u8)101;dong_porf_js_microbench_memory[470]=(u8)100;dong_porf_js_microbench_memory[471]=(u8)32;dong_porf_js_microbench_memory[472]=(u8)65;dong_porf_js_microbench_memory[473]=(u8)114;dong_porf_js_microbench_memory[474]=(u8)114;dong_porf_js_microbench_memory[475]=(u8)97;dong_porf_js_microbench_memory[476]=(u8)121;dong_porf_js_microbench_memory[477]=(u8)46;dong_porf_js_microbench_memory[478]=(u8)102;dong_porf_js_microbench_memory[479]=(u8)114;dong_porf_js_microbench_memory[480]=(u8)111;dong_porf_js_microbench_memory[481]=(u8)109;dong_porf_js_microbench_memory[482]=(u8)32;dong_porf_js_microbench_memory[483]=(u8)119;dong_porf_js_microbench_memory[484]=(u8)105;dong_porf_js_microbench_memory[485]=(u8)116;dong_porf_js_microbench_memory[486]=(u8)104;dong_porf_js_microbench_memory[487]=(u8)32;dong_porf_js_microbench_memory[488]=(u8)97;dong_porf_js_microbench_memory[489]=(u8)32;dong_porf_js_microbench_memory[490]=(u8)110;dong_porf_js_microbench_memory[491]=(u8)111;dong_porf_js_microbench_memory[492]=(u8)110;dong_porf_js_microbench_memory[493]=(u8)45;dong_porf_js_microbench_memory[494]=(u8)102;dong_porf_js_microbench_memory[495]=(u8)117;dong_porf_js_microbench_memory[496]=(u8)110;dong_porf_js_microbench_memory[497]=(u8)99;dong_porf_js_microbench_memory[498]=(u8)116;dong_porf_js_microbench_memory[499]=(u8)105;dong_porf_js_microbench_memory[500]=(u8)111;dong_porf_js_microbench_memory[501]=(u8)110;dong_porf_js_microbench_memory[502]=(u8)32;dong_porf_js_microbench_memory[503]=(u8)109;dong_porf_js_microbench_memory[504]=(u8)97;dong_porf_js_microbench_memory[505]=(u8)112;dong_porf_js_microbench_memory[506]=(u8)70;dong_porf_js_microbench_memory[507]=(u8)110;
  dong_porf_js_microbench_memory[510]=(u8)34;dong_porf_js_microbench_memory[514]=(u8)84;dong_porf_js_microbench_memory[515]=(u8)114;dong_porf_js_microbench_memory[516]=(u8)105;dong_porf_js_microbench_memory[517]=(u8)101;dong_porf_js_microbench_memory[518]=(u8)100;dong_porf_js_microbench_memory[519]=(u8)32;dong_porf_js_microbench_memory[520]=(u8)102;dong_porf_js_microbench_memory[521]=(u8)111;dong_porf_js_microbench_memory[522]=(u8)114;dong_porf_js_microbench_memory[523]=(u8)46;dong_porf_js_microbench_memory[524]=(u8)46;dong_porf_js_microbench_memory[525]=(u8)111;dong_porf_js_microbench_memory[526]=(u8)102;dong_porf_js_microbench_memory[527]=(u8)32;dong_porf_js_microbench_memory[528]=(u8)111;dong_porf_js_microbench_memory[529]=(u8)110;dong_porf_js_microbench_memory[530]=(u8)32;dong_porf_js_microbench_memory[531]=(u8)110;dong_porf_js_microbench_memory[532]=(u8)111;dong_porf_js_microbench_memory[533]=(u8)110;dong_porf_js_microbench_memory[534]=(u8)45;dong_porf_js_microbench_memory[535]=(u8)105;dong_porf_js_microbench_memory[536]=(u8)116;dong_porf_js_microbench_memory[537]=(u8)101;dong_porf_js_microbench_memory[538]=(u8)114;dong_porf_js_microbench_memory[539]=(u8)97;dong_porf_js_microbench_memory[540]=(u8)98;dong_porf_js_microbench_memory[541]=(u8)108;dong_porf_js_microbench_memory[542]=(u8)101;dong_porf_js_microbench_memory[543]=(u8)32;dong_porf_js_microbench_memory[544]=(u8)116;dong_porf_js_microbench_memory[545]=(u8)121;dong_porf_js_microbench_memory[546]=(u8)112;dong_porf_js_microbench_memory[547]=(u8)101;
  dong_porf_js_microbench_memory[550]=(u8)23;dong_porf_js_microbench_memory[554]=(u8)109;dong_porf_js_microbench_memory[555]=(u8)97;dong_porf_js_microbench_memory[556]=(u8)112;dong_porf_js_microbench_memory[557]=(u8)70;dong_porf_js_microbench_memory[558]=(u8)110;dong_porf_js_microbench_memory[559]=(u8)32;dong_porf_js_microbench_memory[560]=(u8)105;dong_porf_js_microbench_memory[561]=(u8)115;dong_porf_js_microbench_memory[562]=(u8)32;dong_porf_js_microbench_memory[563]=(u8)110;dong_porf_js_microbench_memory[564]=(u8)111;dong_porf_js_microbench_memory[565]=(u8)116;dong_porf_js_microbench_memory[566]=(u8)32;dong_porf_js_microbench_memory[567]=(u8)97;dong_porf_js_microbench_memory[568]=(u8)32;dong_porf_js_microbench_memory[569]=(u8)102;dong_porf_js_microbench_memory[570]=(u8)117;dong_porf_js_microbench_memory[571]=(u8)110;dong_porf_js_microbench_memory[572]=(u8)99;dong_porf_js_microbench_memory[573]=(u8)116;dong_porf_js_microbench_memory[574]=(u8)105;dong_porf_js_microbench_memory[575]=(u8)111;dong_porf_js_microbench_memory[576]=(u8)110;
  dong_porf_js_microbench_memory[579]=(u8)9;dong_porf_js_microbench_memory[583]=(u8)95;dong_porf_js_microbench_memory[584]=(u8)95;dong_porf_js_microbench_memory[585]=(u8)112;dong_porf_js_microbench_memory[586]=(u8)114;dong_porf_js_microbench_memory[587]=(u8)111;dong_porf_js_microbench_memory[588]=(u8)116;dong_porf_js_microbench_memory[589]=(u8)111;dong_porf_js_microbench_memory[590]=(u8)95;dong_porf_js_microbench_memory[591]=(u8)95;
  dong_porf_js_microbench_memory[594]=(u8)43;dong_porf_js_microbench_memory[598]=(u8)67;dong_porf_js_microbench_memory[599]=(u8)97;dong_porf_js_microbench_memory[600]=(u8)110;dong_porf_js_microbench_memory[601]=(u8)110;dong_porf_js_microbench_memory[602]=(u8)111;dong_porf_js_microbench_memory[603]=(u8)116;dong_porf_js_microbench_memory[604]=(u8)32;dong_porf_js_microbench_memory[605]=(u8)99;dong_porf_js_microbench_memory[606]=(u8)111;dong_porf_js_microbench_memory[607]=(u8)110;dong_porf_js_microbench_memory[608]=(u8)118;dong_porf_js_microbench_memory[609]=(u8)101;dong_porf_js_microbench_memory[610]=(u8)114;dong_porf_js_microbench_memory[611]=(u8)116;dong_porf_js_microbench_memory[612]=(u8)32;dong_porf_js_microbench_memory[613]=(u8)83;dong_porf_js_microbench_memory[614]=(u8)121;dong_porf_js_microbench_memory[615]=(u8)109;dong_porf_js_microbench_memory[616]=(u8)98;dong_porf_js_microbench_memory[617]=(u8)111;dong_porf_js_microbench_memory[618]=(u8)108;dong_porf_js_microbench_memory[619]=(u8)32;dong_porf_js_microbench_memory[620]=(u8)111;dong_porf_js_microbench_memory[621]=(u8)114;dong_porf_js_microbench_memory[622]=(u8)32;dong_porf_js_microbench_memory[623]=(u8)66;dong_porf_js_microbench_memory[624]=(u8)105;dong_porf_js_microbench_memory[625]=(u8)103;dong_porf_js_microbench_memory[626]=(u8)73;dong_porf_js_microbench_memory[627]=(u8)110;dong_porf_js_microbench_memory[628]=(u8)116;dong_porf_js_microbench_memory[629]=(u8)32;dong_porf_js_microbench_memory[630]=(u8)116;dong_porf_js_microbench_memory[631]=(u8)111;dong_porf_js_microbench_memory[632]=(u8)32;dong_porf_js_microbench_memory[633]=(u8)97;dong_porf_js_microbench_memory[634]=(u8)32;dong_porf_js_microbench_memory[635]=(u8)110;dong_porf_js_microbench_memory[636]=(u8)117;dong_porf_js_microbench_memory[637]=(u8)109;dong_porf_js_microbench_memory[638]=(u8)98;dong_porf_js_microbench_memory[639]=(u8)101;dong_porf_js_microbench_memory[640]=(u8)114;
  dong_porf_js_microbench_memory[643]=(u8)62;dong_porf_js_microbench_memory[647]=(u8)39;dong_porf_js_microbench_memory[648]=(u8)116;dong_porf_js_microbench_memory[649]=(u8)114;dong_porf_js_microbench_memory[650]=(u8)105;dong_porf_js_microbench_memory[651]=(u8)109;dong_porf_js_microbench_memory[652]=(u8)39;dong_porf_js_microbench_memory[653]=(u8)32;dong_porf_js_microbench_memory[654]=(u8)112;dong_porf_js_microbench_memory[655]=(u8)114;dong_porf_js_microbench_memory[656]=(u8)111;dong_porf_js_microbench_memory[657]=(u8)116;dong_porf_js_microbench_memory[658]=(u8)111;dong_porf_js_microbench_memory[659]=(u8)32;dong_porf_js_microbench_memory[660]=(u8)102;dong_porf_js_microbench_memory[661]=(u8)117;dong_porf_js_microbench_memory[662]=(u8)110;dong_porf_js_microbench_memory[663]=(u8)99;dong_porf_js_microbench_memory[664]=(u8)32;dong_porf_js_microbench_memory[665]=(u8)116;dong_porf_js_microbench_memory[666]=(u8)114;dong_porf_js_microbench_memory[667]=(u8)105;dong_porf_js_microbench_memory[668]=(u8)101;dong_porf_js_microbench_memory[669]=(u8)100;dong_porf_js_microbench_memory[670]=(u8)32;dong_porf_js_microbench_memory[671]=(u8)116;dong_porf_js_microbench_memory[672]=(u8)111;dong_porf_js_microbench_memory[673]=(u8)32;dong_porf_js_microbench_memory[674]=(u8)98;dong_porf_js_microbench_memory[675]=(u8)101;dong_porf_js_microbench_memory[676]=(u8)32;dong_porf_js_microbench_memory[677]=(u8)99;dong_porf_js_microbench_memory[678]=(u8)97;dong_porf_js_microbench_memory[679]=(u8)108;dong_porf_js_microbench_memory[680]=(u8)108;dong_porf_js_microbench_memory[681]=(u8)101;dong_porf_js_microbench_memory[682]=(u8)100;dong_porf_js_microbench_memory[683]=(u8)32;dong_porf_js_microbench_memory[684]=(u8)111;dong_porf_js_microbench_memory[685]=(u8)110;dong_porf_js_microbench_memory[686]=(u8)32;dong_porf_js_microbench_memory[687]=(u8)97;dong_porf_js_microbench_memory[688]=(u8)32;dong_porf_js_microbench_memory[689]=(u8)116;dong_porf_js_microbench_memory[690]=(u8)121;dong_porf_js_microbench_memory[691]=(u8)112;dong_porf_js_microbench_memory[692]=(u8)101;dong_porf_js_microbench_memory[693]=(u8)32;dong_porf_js_microbench_memory[694]=(u8)119;dong_porf_js_microbench_memory[695]=(u8)105;dong_porf_js_microbench_memory[696]=(u8)116;dong_porf_js_microbench_memory[697]=(u8)104;dong_porf_js_microbench_memory[698]=(u8)111;dong_porf_js_microbench_memory[699]=(u8)117;dong_porf_js_microbench_memory[700]=(u8)116;dong_porf_js_microbench_memory[701]=(u8)32;dong_porf_js_microbench_memory[702]=(u8)97;dong_porf_js_microbench_memory[703]=(u8)110;dong_porf_js_microbench_memory[704]=(u8)32;dong_porf_js_microbench_memory[705]=(u8)105;dong_porf_js_microbench_memory[706]=(u8)109;dong_porf_js_microbench_memory[707]=(u8)112;dong_porf_js_microbench_memory[708]=(u8)108;
  dong_porf_js_microbench_memory[711]=(u8)68;dong_porf_js_microbench_memory[715]=(u8)39;dong_porf_js_microbench_memory[716]=(u8)99;dong_porf_js_microbench_memory[717]=(u8)104;dong_porf_js_microbench_memory[718]=(u8)97;dong_porf_js_microbench_memory[719]=(u8)114;dong_porf_js_microbench_memory[720]=(u8)67;dong_porf_js_microbench_memory[721]=(u8)111;dong_porf_js_microbench_memory[722]=(u8)100;dong_porf_js_microbench_memory[723]=(u8)101;dong_porf_js_microbench_memory[724]=(u8)65;dong_porf_js_microbench_memory[725]=(u8)116;dong_porf_js_microbench_memory[726]=(u8)39;dong_porf_js_microbench_memory[727]=(u8)32;dong_porf_js_microbench_memory[728]=(u8)112;dong_porf_js_microbench_memory[729]=(u8)114;dong_porf_js_microbench_memory[730]=(u8)111;dong_porf_js_microbench_memory[731]=(u8)116;dong_porf_js_microbench_memory[732]=(u8)111;dong_porf_js_microbench_memory[733]=(u8)32;dong_porf_js_microbench_memory[734]=(u8)102;dong_porf_js_microbench_memory[735]=(u8)117;dong_porf_js_microbench_memory[736]=(u8)110;dong_porf_js_microbench_memory[737]=(u8)99;dong_porf_js_microbench_memory[738]=(u8)32;dong_porf_js_microbench_memory[739]=(u8)116;dong_porf_js_microbench_memory[740]=(u8)114;dong_porf_js_microbench_memory[741]=(u8)105;dong_porf_js_microbench_memory[742]=(u8)101;dong_porf_js_microbench_memory[743]=(u8)100;dong_porf_js_microbench_memory[744]=(u8)32;dong_porf_js_microbench_memory[745]=(u8)116;dong_porf_js_microbench_memory[746]=(u8)111;dong_porf_js_microbench_memory[747]=(u8)32;dong_porf_js_microbench_memory[748]=(u8)98;dong_porf_js_microbench_memory[749]=(u8)101;dong_porf_js_microbench_memory[750]=(u8)32;dong_porf_js_microbench_memory[751]=(u8)99;dong_porf_js_microbench_memory[752]=(u8)97;dong_porf_js_microbench_memory[753]=(u8)108;dong_porf_js_microbench_memory[754]=(u8)108;dong_porf_js_microbench_memory[755]=(u8)101;dong_porf_js_microbench_memory[756]=(u8)100;dong_porf_js_microbench_memory[757]=(u8)32;dong_porf_js_microbench_memory[758]=(u8)111;dong_porf_js_microbench_memory[759]=(u8)110;dong_porf_js_microbench_memory[760]=(u8)32;dong_porf_js_microbench_memory[761]=(u8)97;dong_porf_js_microbench_memory[762]=(u8)32;dong_porf_js_microbench_memory[763]=(u8)116;dong_porf_js_microbench_memory[764]=(u8)121;dong_porf_js_microbench_memory[765]=(u8)112;dong_porf_js_microbench_memory[766]=(u8)101;dong_porf_js_microbench_memory[767]=(u8)32;dong_porf_js_microbench_memory[768]=(u8)119;dong_porf_js_microbench_memory[769]=(u8)105;dong_porf_js_microbench_memory[770]=(u8)116;dong_porf_js_microbench_memory[771]=(u8)104;dong_porf_js_microbench_memory[772]=(u8)111;dong_porf_js_microbench_memory[773]=(u8)117;dong_porf_js_microbench_memory[774]=(u8)116;dong_porf_js_microbench_memory[775]=(u8)32;dong_porf_js_microbench_memory[776]=(u8)97;dong_porf_js_microbench_memory[777]=(u8)110;dong_porf_js_microbench_memory[778]=(u8)32;dong_porf_js_microbench_memory[779]=(u8)105;dong_porf_js_microbench_memory[780]=(u8)109;dong_porf_js_microbench_memory[781]=(u8)112;dong_porf_js_microbench_memory[782]=(u8)108;
  dong_porf_js_microbench_memory[785]=(u8)54;dong_porf_js_microbench_memory[789]=(u8)78;dong_porf_js_microbench_memory[790]=(u8)117;dong_porf_js_microbench_memory[791]=(u8)109;dong_porf_js_microbench_memory[792]=(u8)98;dong_porf_js_microbench_memory[793]=(u8)101;dong_porf_js_microbench_memory[794]=(u8)114;dong_porf_js_microbench_memory[795]=(u8)46;dong_porf_js_microbench_memory[796]=(u8)112;dong_porf_js_microbench_memory[797]=(u8)114;dong_porf_js_microbench_memory[798]=(u8)111;dong_porf_js_microbench_memory[799]=(u8)116;dong_porf_js_microbench_memory[800]=(u8)111;dong_porf_js_microbench_memory[801]=(u8)116;dong_porf_js_microbench_memory[802]=(u8)121;dong_porf_js_microbench_memory[803]=(u8)112;dong_porf_js_microbench_memory[804]=(u8)101;dong_porf_js_microbench_memory[805]=(u8)46;dong_porf_js_microbench_memory[806]=(u8)118;dong_porf_js_microbench_memory[807]=(u8)97;dong_porf_js_microbench_memory[808]=(u8)108;dong_porf_js_microbench_memory[809]=(u8)117;dong_porf_js_microbench_memory[810]=(u8)101;dong_porf_js_microbench_memory[811]=(u8)79;dong_porf_js_microbench_memory[812]=(u8)102;dong_porf_js_microbench_memory[813]=(u8)32;dong_porf_js_microbench_memory[814]=(u8)101;dong_porf_js_microbench_memory[815]=(u8)120;dong_porf_js_microbench_memory[816]=(u8)112;dong_porf_js_microbench_memory[817]=(u8)101;dong_porf_js_microbench_memory[818]=(u8)99;dong_porf_js_microbench_memory[819]=(u8)116;dong_porf_js_microbench_memory[820]=(u8)115;dong_porf_js_microbench_memory[821]=(u8)32;dong_porf_js_microbench_memory[822]=(u8)39;dong_porf_js_microbench_memory[823]=(u8)116;dong_porf_js_microbench_memory[824]=(u8)104;dong_porf_js_microbench_memory[825]=(u8)105;dong_porf_js_microbench_memory[826]=(u8)115;dong_porf_js_microbench_memory[827]=(u8)39;dong_porf_js_microbench_memory[828]=(u8)32;dong_porf_js_microbench_memory[829]=(u8)116;dong_porf_js_microbench_memory[830]=(u8)111;dong_porf_js_microbench_memory[831]=(u8)32;dong_porf_js_microbench_memory[832]=(u8)98;dong_porf_js_microbench_memory[833]=(u8)101;dong_porf_js_microbench_memory[834]=(u8)32;dong_porf_js_microbench_memory[835]=(u8)97;dong_porf_js_microbench_memory[836]=(u8)32;dong_porf_js_microbench_memory[837]=(u8)78;dong_porf_js_microbench_memory[838]=(u8)117;dong_porf_js_microbench_memory[839]=(u8)109;dong_porf_js_microbench_memory[840]=(u8)98;dong_porf_js_microbench_memory[841]=(u8)101;dong_porf_js_microbench_memory[842]=(u8)114;
  dong_porf_js_microbench_memory[845]=(u8)7;dong_porf_js_microbench_memory[849]=(u8)118;dong_porf_js_microbench_memory[850]=(u8)97;dong_porf_js_microbench_memory[851]=(u8)108;dong_porf_js_microbench_memory[852]=(u8)117;dong_porf_js_microbench_memory[853]=(u8)101;dong_porf_js_microbench_memory[854]=(u8)79;dong_porf_js_microbench_memory[855]=(u8)102;
  dong_porf_js_microbench_memory[858]=(u8)29;dong_porf_js_microbench_memory[862]=(u8)70;dong_porf_js_microbench_memory[863]=(u8)117;dong_porf_js_microbench_memory[864]=(u8)110;dong_porf_js_microbench_memory[865]=(u8)99;dong_porf_js_microbench_memory[866]=(u8)116;dong_porf_js_microbench_memory[867]=(u8)105;dong_porf_js_microbench_memory[868]=(u8)111;dong_porf_js_microbench_memory[869]=(u8)110;dong_porf_js_microbench_memory[870]=(u8)32;dong_porf_js_microbench_memory[871]=(u8)105;dong_porf_js_microbench_memory[872]=(u8)115;dong_porf_js_microbench_memory[873]=(u8)32;dong_porf_js_microbench_memory[874]=(u8)110;dong_porf_js_microbench_memory[875]=(u8)111;dong_porf_js_microbench_memory[876]=(u8)116;dong_porf_js_microbench_memory[877]=(u8)32;dong_porf_js_microbench_memory[878]=(u8)97;dong_porf_js_microbench_memory[879]=(u8)32;dong_porf_js_microbench_memory[880]=(u8)99;dong_porf_js_microbench_memory[881]=(u8)111;dong_porf_js_microbench_memory[882]=(u8)110;dong_porf_js_microbench_memory[883]=(u8)115;dong_porf_js_microbench_memory[884]=(u8)116;dong_porf_js_microbench_memory[885]=(u8)114;dong_porf_js_microbench_memory[886]=(u8)117;dong_porf_js_microbench_memory[887]=(u8)99;dong_porf_js_microbench_memory[888]=(u8)116;dong_porf_js_microbench_memory[889]=(u8)111;dong_porf_js_microbench_memory[890]=(u8)114;
  dong_porf_js_microbench_memory[893]=(u8)21;dong_porf_js_microbench_memory[897]=(u8)111;dong_porf_js_microbench_memory[898]=(u8)118;dong_porf_js_microbench_memory[899]=(u8)114;dong_porf_js_microbench_memory[900]=(u8)32;dong_porf_js_microbench_memory[901]=(u8)105;dong_porf_js_microbench_memory[902]=(u8)115;dong_porf_js_microbench_memory[903]=(u8)32;dong_porf_js_microbench_memory[904]=(u8)110;dong_porf_js_microbench_memory[905]=(u8)111;dong_porf_js_microbench_memory[906]=(u8)116;dong_porf_js_microbench_memory[907]=(u8)32;dong_porf_js_microbench_memory[908]=(u8)97;dong_porf_js_microbench_memory[909]=(u8)32;dong_porf_js_microbench_memory[910]=(u8)102;dong_porf_js_microbench_memory[911]=(u8)117;dong_porf_js_microbench_memory[912]=(u8)110;dong_porf_js_microbench_memory[913]=(u8)99;dong_porf_js_microbench_memory[914]=(u8)116;dong_porf_js_microbench_memory[915]=(u8)105;dong_porf_js_microbench_memory[916]=(u8)111;dong_porf_js_microbench_memory[917]=(u8)110;
  dong_porf_js_microbench_memory[920]=(u8)37;dong_porf_js_microbench_memory[924]=(u8)67;dong_porf_js_microbench_memory[925]=(u8)97;dong_porf_js_microbench_memory[926]=(u8)110;dong_porf_js_microbench_memory[927]=(u8)110;dong_porf_js_microbench_memory[928]=(u8)111;dong_porf_js_microbench_memory[929]=(u8)116;dong_porf_js_microbench_memory[930]=(u8)32;dong_porf_js_microbench_memory[931]=(u8)99;dong_porf_js_microbench_memory[932]=(u8)111;dong_porf_js_microbench_memory[933]=(u8)110;dong_porf_js_microbench_memory[934]=(u8)118;dong_porf_js_microbench_memory[935]=(u8)101;dong_porf_js_microbench_memory[936]=(u8)114;dong_porf_js_microbench_memory[937]=(u8)116;dong_porf_js_microbench_memory[938]=(u8)32;dong_porf_js_microbench_memory[939]=(u8)97;dong_porf_js_microbench_memory[940]=(u8)110;dong_porf_js_microbench_memory[941]=(u8)32;dong_porf_js_microbench_memory[942]=(u8)111;dong_porf_js_microbench_memory[943]=(u8)98;dong_porf_js_microbench_memory[944]=(u8)106;dong_porf_js_microbench_memory[945]=(u8)101;dong_porf_js_microbench_memory[946]=(u8)99;dong_porf_js_microbench_memory[947]=(u8)116;dong_porf_js_microbench_memory[948]=(u8)32;dong_porf_js_microbench_memory[949]=(u8)116;dong_porf_js_microbench_memory[950]=(u8)111;dong_porf_js_microbench_memory[951]=(u8)32;dong_porf_js_microbench_memory[952]=(u8)112;dong_porf_js_microbench_memory[953]=(u8)114;dong_porf_js_microbench_memory[954]=(u8)105;dong_porf_js_microbench_memory[955]=(u8)109;dong_porf_js_microbench_memory[956]=(u8)105;dong_porf_js_microbench_memory[957]=(u8)116;dong_porf_js_microbench_memory[958]=(u8)105;dong_porf_js_microbench_memory[959]=(u8)118;dong_porf_js_microbench_memory[960]=(u8)101;
  dong_porf_js_microbench_memory[963]=(u8)20;dong_porf_js_microbench_memory[967]=(u8)73;dong_porf_js_microbench_memory[968]=(u8)110;dong_porf_js_microbench_memory[969]=(u8)118;dong_porf_js_microbench_memory[970]=(u8)97;dong_porf_js_microbench_memory[971]=(u8)108;dong_porf_js_microbench_memory[972]=(u8)105;dong_porf_js_microbench_memory[973]=(u8)100;dong_porf_js_microbench_memory[974]=(u8)32;dong_porf_js_microbench_memory[975]=(u8)97;dong_porf_js_microbench_memory[976]=(u8)114;dong_porf_js_microbench_memory[977]=(u8)114;dong_porf_js_microbench_memory[978]=(u8)97;dong_porf_js_microbench_memory[979]=(u8)121;dong_porf_js_microbench_memory[980]=(u8)32;dong_porf_js_microbench_memory[981]=(u8)108;dong_porf_js_microbench_memory[982]=(u8)101;dong_porf_js_microbench_memory[983]=(u8)110;dong_porf_js_microbench_memory[984]=(u8)103;dong_porf_js_microbench_memory[985]=(u8)116;dong_porf_js_microbench_memory[986]=(u8)104;
  dong_porf_js_microbench_memory[989]=(u8)20;dong_porf_js_microbench_memory[993]=(u8)112;dong_porf_js_microbench_memory[994]=(u8)114;dong_porf_js_microbench_memory[995]=(u8)111;dong_porf_js_microbench_memory[996]=(u8)112;dong_porf_js_microbench_memory[997]=(u8)101;dong_porf_js_microbench_memory[998]=(u8)114;dong_porf_js_microbench_memory[999]=(u8)116;dong_porf_js_microbench_memory[1000]=(u8)121;dong_porf_js_microbench_memory[1001]=(u8)73;dong_porf_js_microbench_memory[1002]=(u8)115;dong_porf_js_microbench_memory[1003]=(u8)69;dong_porf_js_microbench_memory[1004]=(u8)110;dong_porf_js_microbench_memory[1005]=(u8)117;dong_porf_js_microbench_memory[1006]=(u8)109;dong_porf_js_microbench_memory[1007]=(u8)101;dong_porf_js_microbench_memory[1008]=(u8)114;dong_porf_js_microbench_memory[1009]=(u8)97;dong_porf_js_microbench_memory[1010]=(u8)98;dong_porf_js_microbench_memory[1011]=(u8)108;dong_porf_js_microbench_memory[1012]=(u8)101;
  dong_porf_js_microbench_memory[1015]=(u8)13;dong_porf_js_microbench_memory[1019]=(u8)105;dong_porf_js_microbench_memory[1020]=(u8)115;dong_porf_js_microbench_memory[1021]=(u8)80;dong_porf_js_microbench_memory[1022]=(u8)114;dong_porf_js_microbench_memory[1023]=(u8)111;dong_porf_js_microbench_memory[1024]=(u8)116;dong_porf_js_microbench_memory[1025]=(u8)111;dong_porf_js_microbench_memory[1026]=(u8)116;dong_porf_js_microbench_memory[1027]=(u8)121;dong_porf_js_microbench_memory[1028]=(u8)112;dong_porf_js_microbench_memory[1029]=(u8)101;dong_porf_js_microbench_memory[1030]=(u8)79;dong_porf_js_microbench_memory[1031]=(u8)102;
  dong_porf_js_microbench_memory[1034]=(u8)32;dong_porf_js_microbench_memory[1038]=(u8)84;dong_porf_js_microbench_memory[1039]=(u8)104;dong_porf_js_microbench_memory[1040]=(u8)105;dong_porf_js_microbench_memory[1041]=(u8)115;dong_porf_js_microbench_memory[1042]=(u8)32;dong_porf_js_microbench_memory[1043]=(u8)105;dong_porf_js_microbench_memory[1044]=(u8)115;dong_porf_js_microbench_memory[1045]=(u8)32;dong_porf_js_microbench_memory[1046]=(u8)110;dong_porf_js_microbench_memory[1047]=(u8)117;dong_porf_js_microbench_memory[1048]=(u8)108;dong_porf_js_microbench_memory[1049]=(u8)108;dong_porf_js_microbench_memory[1050]=(u8)105;dong_porf_js_microbench_memory[1051]=(u8)115;dong_porf_js_microbench_memory[1052]=(u8)104;dong_porf_js_microbench_memory[1053]=(u8)44;dong_porf_js_microbench_memory[1054]=(u8)32;dong_porf_js_microbench_memory[1055]=(u8)101;dong_porf_js_microbench_memory[1056]=(u8)120;dong_porf_js_microbench_memory[1057]=(u8)112;dong_porf_js_microbench_memory[1058]=(u8)101;dong_porf_js_microbench_memory[1059]=(u8)99;dong_porf_js_microbench_memory[1060]=(u8)116;dong_porf_js_microbench_memory[1061]=(u8)101;dong_porf_js_microbench_memory[1062]=(u8)100;dong_porf_js_microbench_memory[1063]=(u8)32;dong_porf_js_microbench_memory[1064]=(u8)111;dong_porf_js_microbench_memory[1065]=(u8)98;dong_porf_js_microbench_memory[1066]=(u8)106;dong_porf_js_microbench_memory[1067]=(u8)101;dong_porf_js_microbench_memory[1068]=(u8)99;dong_porf_js_microbench_memory[1069]=(u8)116;
  dong_porf_js_microbench_memory[1072]=(u8)14;dong_porf_js_microbench_memory[1076]=(u8)116;dong_porf_js_microbench_memory[1077]=(u8)111;dong_porf_js_microbench_memory[1078]=(u8)76;dong_porf_js_microbench_memory[1079]=(u8)111;dong_porf_js_microbench_memory[1080]=(u8)99;dong_porf_js_microbench_memory[1081]=(u8)97;dong_porf_js_microbench_memory[1082]=(u8)108;dong_porf_js_microbench_memory[1083]=(u8)101;dong_porf_js_microbench_memory[1084]=(u8)83;dong_porf_js_microbench_memory[1085]=(u8)116;dong_porf_js_microbench_memory[1086]=(u8)114;dong_porf_js_microbench_memory[1087]=(u8)105;dong_porf_js_microbench_memory[1088]=(u8)110;dong_porf_js_microbench_memory[1089]=(u8)103;
  dong_porf_js_microbench_memory[1092]=(u8)55;dong_porf_js_microbench_memory[1096]=(u8)83;dong_porf_js_microbench_memory[1097]=(u8)121;dong_porf_js_microbench_memory[1098]=(u8)109;dong_porf_js_microbench_memory[1099]=(u8)98;dong_porf_js_microbench_memory[1100]=(u8)111;dong_porf_js_microbench_memory[1101]=(u8)108;dong_porf_js_microbench_memory[1102]=(u8)46;dong_porf_js_microbench_memory[1103]=(u8)112;dong_porf_js_microbench_memory[1104]=(u8)114;dong_porf_js_microbench_memory[1105]=(u8)111;dong_porf_js_microbench_memory[1106]=(u8)116;dong_porf_js_microbench_memory[1107]=(u8)111;dong_porf_js_microbench_memory[1108]=(u8)116;dong_porf_js_microbench_memory[1109]=(u8)121;dong_porf_js_microbench_memory[1110]=(u8)112;dong_porf_js_microbench_memory[1111]=(u8)101;dong_porf_js_microbench_memory[1112]=(u8)46;dong_porf_js_microbench_memory[1113]=(u8)116;dong_porf_js_microbench_memory[1114]=(u8)111;dong_porf_js_microbench_memory[1115]=(u8)83;dong_porf_js_microbench_memory[1116]=(u8)116;dong_porf_js_microbench_memory[1117]=(u8)114;dong_porf_js_microbench_memory[1118]=(u8)105;dong_porf_js_microbench_memory[1119]=(u8)110;dong_porf_js_microbench_memory[1120]=(u8)103;dong_porf_js_microbench_memory[1121]=(u8)32;dong_porf_js_microbench_memory[1122]=(u8)101;dong_porf_js_microbench_memory[1123]=(u8)120;dong_porf_js_microbench_memory[1124]=(u8)112;dong_porf_js_microbench_memory[1125]=(u8)101;dong_porf_js_microbench_memory[1126]=(u8)99;dong_porf_js_microbench_memory[1127]=(u8)116;dong_porf_js_microbench_memory[1128]=(u8)115;dong_porf_js_microbench_memory[1129]=(u8)32;dong_porf_js_microbench_memory[1130]=(u8)39;dong_porf_js_microbench_memory[1131]=(u8)116;dong_porf_js_microbench_memory[1132]=(u8)104;dong_porf_js_microbench_memory[1133]=(u8)105;dong_porf_js_microbench_memory[1134]=(u8)115;dong_porf_js_microbench_memory[1135]=(u8)39;dong_porf_js_microbench_memory[1136]=(u8)32;dong_porf_js_microbench_memory[1137]=(u8)116;dong_porf_js_microbench_memory[1138]=(u8)111;dong_porf_js_microbench_memory[1139]=(u8)32;dong_porf_js_microbench_memory[1140]=(u8)98;dong_porf_js_microbench_memory[1141]=(u8)101;dong_porf_js_microbench_memory[1142]=(u8)32;dong_porf_js_microbench_memory[1143]=(u8)97;dong_porf_js_microbench_memory[1144]=(u8)32;dong_porf_js_microbench_memory[1145]=(u8)83;dong_porf_js_microbench_memory[1146]=(u8)121;dong_porf_js_microbench_memory[1147]=(u8)109;dong_porf_js_microbench_memory[1148]=(u8)98;dong_porf_js_microbench_memory[1149]=(u8)111;dong_porf_js_microbench_memory[1150]=(u8)108;
  dong_porf_js_microbench_memory[1153]=(u8)62;dong_porf_js_microbench_memory[1157]=(u8)83;dong_porf_js_microbench_memory[1158]=(u8)121;dong_porf_js_microbench_memory[1159]=(u8)109;dong_porf_js_microbench_memory[1160]=(u8)98;dong_porf_js_microbench_memory[1161]=(u8)111;dong_porf_js_microbench_memory[1162]=(u8)108;dong_porf_js_microbench_memory[1163]=(u8)46;dong_porf_js_microbench_memory[1164]=(u8)112;dong_porf_js_microbench_memory[1165]=(u8)114;dong_porf_js_microbench_memory[1166]=(u8)111;dong_porf_js_microbench_memory[1167]=(u8)116;dong_porf_js_microbench_memory[1168]=(u8)111;dong_porf_js_microbench_memory[1169]=(u8)116;dong_porf_js_microbench_memory[1170]=(u8)121;dong_porf_js_microbench_memory[1171]=(u8)112;dong_porf_js_microbench_memory[1172]=(u8)101;dong_porf_js_microbench_memory[1173]=(u8)46;dong_porf_js_microbench_memory[1174]=(u8)100;dong_porf_js_microbench_memory[1175]=(u8)101;dong_porf_js_microbench_memory[1176]=(u8)115;dong_porf_js_microbench_memory[1177]=(u8)99;dong_porf_js_microbench_memory[1178]=(u8)114;dong_porf_js_microbench_memory[1179]=(u8)105;dong_porf_js_microbench_memory[1180]=(u8)112;dong_porf_js_microbench_memory[1181]=(u8)116;dong_porf_js_microbench_memory[1182]=(u8)105;dong_porf_js_microbench_memory[1183]=(u8)111;dong_porf_js_microbench_memory[1184]=(u8)110;dong_porf_js_microbench_memory[1185]=(u8)36;dong_porf_js_microbench_memory[1186]=(u8)103;dong_porf_js_microbench_memory[1187]=(u8)101;dong_porf_js_microbench_memory[1188]=(u8)116;dong_porf_js_microbench_memory[1189]=(u8)32;dong_porf_js_microbench_memory[1190]=(u8)101;dong_porf_js_microbench_memory[1191]=(u8)120;dong_porf_js_microbench_memory[1192]=(u8)112;dong_porf_js_microbench_memory[1193]=(u8)101;dong_porf_js_microbench_memory[1194]=(u8)99;dong_porf_js_microbench_memory[1195]=(u8)116;dong_porf_js_microbench_memory[1196]=(u8)115;dong_porf_js_microbench_memory[1197]=(u8)32;dong_porf_js_microbench_memory[1198]=(u8)39;dong_porf_js_microbench_memory[1199]=(u8)116;dong_porf_js_microbench_memory[1200]=(u8)104;dong_porf_js_microbench_memory[1201]=(u8)105;dong_porf_js_microbench_memory[1202]=(u8)115;dong_porf_js_microbench_memory[1203]=(u8)39;dong_porf_js_microbench_memory[1204]=(u8)32;dong_porf_js_microbench_memory[1205]=(u8)116;dong_porf_js_microbench_memory[1206]=(u8)111;dong_porf_js_microbench_memory[1207]=(u8)32;dong_porf_js_microbench_memory[1208]=(u8)98;dong_porf_js_microbench_memory[1209]=(u8)101;dong_porf_js_microbench_memory[1210]=(u8)32;dong_porf_js_microbench_memory[1211]=(u8)97;dong_porf_js_microbench_memory[1212]=(u8)32;dong_porf_js_microbench_memory[1213]=(u8)83;dong_porf_js_microbench_memory[1214]=(u8)121;dong_porf_js_microbench_memory[1215]=(u8)109;dong_porf_js_microbench_memory[1216]=(u8)98;dong_porf_js_microbench_memory[1217]=(u8)111;dong_porf_js_microbench_memory[1218]=(u8)108;
  dong_porf_js_microbench_memory[1221]=(u8)18;dong_porf_js_microbench_memory[1225]=(u8)91;dong_porf_js_microbench_memory[1226]=(u8)111;dong_porf_js_microbench_memory[1227]=(u8)98;dong_porf_js_microbench_memory[1228]=(u8)106;dong_porf_js_microbench_memory[1229]=(u8)101;dong_porf_js_microbench_memory[1230]=(u8)99;dong_porf_js_microbench_memory[1231]=(u8)116;dong_porf_js_microbench_memory[1232]=(u8)32;dong_porf_js_microbench_memory[1233]=(u8)85;dong_porf_js_microbench_memory[1234]=(u8)110;dong_porf_js_microbench_memory[1235]=(u8)100;dong_porf_js_microbench_memory[1236]=(u8)101;dong_porf_js_microbench_memory[1237]=(u8)102;dong_porf_js_microbench_memory[1238]=(u8)105;dong_porf_js_microbench_memory[1239]=(u8)110;dong_porf_js_microbench_memory[1240]=(u8)101;dong_porf_js_microbench_memory[1241]=(u8)100;dong_porf_js_microbench_memory[1242]=(u8)93;
  dong_porf_js_microbench_memory[1245]=(u8)13;dong_porf_js_microbench_memory[1249]=(u8)91;dong_porf_js_microbench_memory[1250]=(u8)111;dong_porf_js_microbench_memory[1251]=(u8)98;dong_porf_js_microbench_memory[1252]=(u8)106;dong_porf_js_microbench_memory[1253]=(u8)101;dong_porf_js_microbench_memory[1254]=(u8)99;dong_porf_js_microbench_memory[1255]=(u8)116;dong_porf_js_microbench_memory[1256]=(u8)32;dong_porf_js_microbench_memory[1257]=(u8)78;dong_porf_js_microbench_memory[1258]=(u8)117;dong_porf_js_microbench_memory[1259]=(u8)108;dong_porf_js_microbench_memory[1260]=(u8)108;dong_porf_js_microbench_memory[1261]=(u8)93;
  dong_porf_js_microbench_memory[1264]=(u8)14;dong_porf_js_microbench_memory[1268]=(u8)91;dong_porf_js_microbench_memory[1269]=(u8)111;dong_porf_js_microbench_memory[1270]=(u8)98;dong_porf_js_microbench_memory[1271]=(u8)106;dong_porf_js_microbench_memory[1272]=(u8)101;dong_porf_js_microbench_memory[1273]=(u8)99;dong_porf_js_microbench_memory[1274]=(u8)116;dong_porf_js_microbench_memory[1275]=(u8)32;dong_porf_js_microbench_memory[1276]=(u8)65;dong_porf_js_microbench_memory[1277]=(u8)114;dong_porf_js_microbench_memory[1278]=(u8)114;dong_porf_js_microbench_memory[1279]=(u8)97;dong_porf_js_microbench_memory[1280]=(u8)121;dong_porf_js_microbench_memory[1281]=(u8)93;
  dong_porf_js_microbench_memory[1284]=(u8)17;dong_porf_js_microbench_memory[1288]=(u8)91;dong_porf_js_microbench_memory[1289]=(u8)111;dong_porf_js_microbench_memory[1290]=(u8)98;dong_porf_js_microbench_memory[1291]=(u8)106;dong_porf_js_microbench_memory[1292]=(u8)101;dong_porf_js_microbench_memory[1293]=(u8)99;dong_porf_js_microbench_memory[1294]=(u8)116;dong_porf_js_microbench_memory[1295]=(u8)32;dong_porf_js_microbench_memory[1296]=(u8)70;dong_porf_js_microbench_memory[1297]=(u8)117;dong_porf_js_microbench_memory[1298]=(u8)110;dong_porf_js_microbench_memory[1299]=(u8)99;dong_porf_js_microbench_memory[1300]=(u8)116;dong_porf_js_microbench_memory[1301]=(u8)105;dong_porf_js_microbench_memory[1302]=(u8)111;dong_porf_js_microbench_memory[1303]=(u8)110;dong_porf_js_microbench_memory[1304]=(u8)93;
  dong_porf_js_microbench_memory[1307]=(u8)16;dong_porf_js_microbench_memory[1311]=(u8)91;dong_porf_js_microbench_memory[1312]=(u8)111;dong_porf_js_microbench_memory[1313]=(u8)98;dong_porf_js_microbench_memory[1314]=(u8)106;dong_porf_js_microbench_memory[1315]=(u8)101;dong_porf_js_microbench_memory[1316]=(u8)99;dong_porf_js_microbench_memory[1317]=(u8)116;dong_porf_js_microbench_memory[1318]=(u8)32;dong_porf_js_microbench_memory[1319]=(u8)66;dong_porf_js_microbench_memory[1320]=(u8)111;dong_porf_js_microbench_memory[1321]=(u8)111;dong_porf_js_microbench_memory[1322]=(u8)108;dong_porf_js_microbench_memory[1323]=(u8)101;dong_porf_js_microbench_memory[1324]=(u8)97;dong_porf_js_microbench_memory[1325]=(u8)110;dong_porf_js_microbench_memory[1326]=(u8)93;
  dong_porf_js_microbench_memory[1329]=(u8)15;dong_porf_js_microbench_memory[1333]=(u8)91;dong_porf_js_microbench_memory[1334]=(u8)111;dong_porf_js_microbench_memory[1335]=(u8)98;dong_porf_js_microbench_memory[1336]=(u8)106;dong_porf_js_microbench_memory[1337]=(u8)101;dong_porf_js_microbench_memory[1338]=(u8)99;dong_porf_js_microbench_memory[1339]=(u8)116;dong_porf_js_microbench_memory[1340]=(u8)32;dong_porf_js_microbench_memory[1341]=(u8)78;dong_porf_js_microbench_memory[1342]=(u8)117;dong_porf_js_microbench_memory[1343]=(u8)109;dong_porf_js_microbench_memory[1344]=(u8)98;dong_porf_js_microbench_memory[1345]=(u8)101;dong_porf_js_microbench_memory[1346]=(u8)114;dong_porf_js_microbench_memory[1347]=(u8)93;
  dong_porf_js_microbench_memory[1350]=(u8)15;dong_porf_js_microbench_memory[1354]=(u8)91;dong_porf_js_microbench_memory[1355]=(u8)111;dong_porf_js_microbench_memory[1356]=(u8)98;dong_porf_js_microbench_memory[1357]=(u8)106;dong_porf_js_microbench_memory[1358]=(u8)101;dong_porf_js_microbench_memory[1359]=(u8)99;dong_porf_js_microbench_memory[1360]=(u8)116;dong_porf_js_microbench_memory[1361]=(u8)32;dong_porf_js_microbench_memory[1362]=(u8)83;dong_porf_js_microbench_memory[1363]=(u8)116;dong_porf_js_microbench_memory[1364]=(u8)114;dong_porf_js_microbench_memory[1365]=(u8)105;dong_porf_js_microbench_memory[1366]=(u8)110;dong_porf_js_microbench_memory[1367]=(u8)103;dong_porf_js_microbench_memory[1368]=(u8)93;
  dong_porf_js_microbench_memory[1371]=(u8)13;dong_porf_js_microbench_memory[1375]=(u8)91;dong_porf_js_microbench_memory[1376]=(u8)111;dong_porf_js_microbench_memory[1377]=(u8)98;dong_porf_js_microbench_memory[1378]=(u8)106;dong_porf_js_microbench_memory[1379]=(u8)101;dong_porf_js_microbench_memory[1380]=(u8)99;dong_porf_js_microbench_memory[1381]=(u8)116;dong_porf_js_microbench_memory[1382]=(u8)32;dong_porf_js_microbench_memory[1383]=(u8)68;dong_porf_js_microbench_memory[1384]=(u8)97;dong_porf_js_microbench_memory[1385]=(u8)116;dong_porf_js_microbench_memory[1386]=(u8)101;dong_porf_js_microbench_memory[1387]=(u8)93;
  dong_porf_js_microbench_memory[1390]=(u8)15;dong_porf_js_microbench_memory[1394]=(u8)91;dong_porf_js_microbench_memory[1395]=(u8)111;dong_porf_js_microbench_memory[1396]=(u8)98;dong_porf_js_microbench_memory[1397]=(u8)106;dong_porf_js_microbench_memory[1398]=(u8)101;dong_porf_js_microbench_memory[1399]=(u8)99;dong_porf_js_microbench_memory[1400]=(u8)116;dong_porf_js_microbench_memory[1401]=(u8)32;dong_porf_js_microbench_memory[1402]=(u8)82;dong_porf_js_microbench_memory[1403]=(u8)101;dong_porf_js_microbench_memory[1404]=(u8)103;dong_porf_js_microbench_memory[1405]=(u8)69;dong_porf_js_microbench_memory[1406]=(u8)120;dong_porf_js_microbench_memory[1407]=(u8)112;dong_porf_js_microbench_memory[1408]=(u8)93;
  dong_porf_js_microbench_memory[1411]=(u8)15;dong_porf_js_microbench_memory[1415]=(u8)91;dong_porf_js_microbench_memory[1416]=(u8)111;dong_porf_js_microbench_memory[1417]=(u8)98;dong_porf_js_microbench_memory[1418]=(u8)106;dong_porf_js_microbench_memory[1419]=(u8)101;dong_porf_js_microbench_memory[1420]=(u8)99;dong_porf_js_microbench_memory[1421]=(u8)116;dong_porf_js_microbench_memory[1422]=(u8)32;dong_porf_js_microbench_memory[1423]=(u8)79;dong_porf_js_microbench_memory[1424]=(u8)98;dong_porf_js_microbench_memory[1425]=(u8)106;dong_porf_js_microbench_memory[1426]=(u8)101;dong_porf_js_microbench_memory[1427]=(u8)99;dong_porf_js_microbench_memory[1428]=(u8)116;dong_porf_js_microbench_memory[1429]=(u8)93;
  dong_porf_js_microbench_memory[1432]=(u8)57;dong_porf_js_microbench_memory[1436]=(u8)83;dong_porf_js_microbench_memory[1437]=(u8)116;dong_porf_js_microbench_memory[1438]=(u8)114;dong_porf_js_microbench_memory[1439]=(u8)105;dong_porf_js_microbench_memory[1440]=(u8)110;dong_porf_js_microbench_memory[1441]=(u8)103;dong_porf_js_microbench_memory[1442]=(u8)46;dong_porf_js_microbench_memory[1443]=(u8)112;dong_porf_js_microbench_memory[1444]=(u8)114;dong_porf_js_microbench_memory[1445]=(u8)111;dong_porf_js_microbench_memory[1446]=(u8)116;dong_porf_js_microbench_memory[1447]=(u8)111;dong_porf_js_microbench_memory[1448]=(u8)116;dong_porf_js_microbench_memory[1449]=(u8)121;dong_porf_js_microbench_memory[1450]=(u8)112;dong_porf_js_microbench_memory[1451]=(u8)101;dong_porf_js_microbench_memory[1452]=(u8)46;dong_porf_js_microbench_memory[1453]=(u8)118;dong_porf_js_microbench_memory[1454]=(u8)97;dong_porf_js_microbench_memory[1455]=(u8)108;dong_porf_js_microbench_memory[1456]=(u8)117;dong_porf_js_microbench_memory[1457]=(u8)101;dong_porf_js_microbench_memory[1458]=(u8)79;dong_porf_js_microbench_memory[1459]=(u8)102;dong_porf_js_microbench_memory[1460]=(u8)32;dong_porf_js_microbench_memory[1461]=(u8)101;dong_porf_js_microbench_memory[1462]=(u8)120;dong_porf_js_microbench_memory[1463]=(u8)112;dong_porf_js_microbench_memory[1464]=(u8)101;dong_porf_js_microbench_memory[1465]=(u8)99;dong_porf_js_microbench_memory[1466]=(u8)116;dong_porf_js_microbench_memory[1467]=(u8)115;dong_porf_js_microbench_memory[1468]=(u8)32;dong_porf_js_microbench_memory[1469]=(u8)39;dong_porf_js_microbench_memory[1470]=(u8)116;dong_porf_js_microbench_memory[1471]=(u8)104;dong_porf_js_microbench_memory[1472]=(u8)105;dong_porf_js_microbench_memory[1473]=(u8)115;dong_porf_js_microbench_memory[1474]=(u8)39;dong_porf_js_microbench_memory[1475]=(u8)32;dong_porf_js_microbench_memory[1476]=(u8)116;dong_porf_js_microbench_memory[1477]=(u8)111;dong_porf_js_microbench_memory[1478]=(u8)32;dong_porf_js_microbench_memory[1479]=(u8)98;dong_porf_js_microbench_memory[1480]=(u8)101;dong_porf_js_microbench_memory[1481]=(u8)32;dong_porf_js_microbench_memory[1482]=(u8)110;dong_porf_js_microbench_memory[1483]=(u8)111;dong_porf_js_microbench_memory[1484]=(u8)110;dong_porf_js_microbench_memory[1485]=(u8)45;dong_porf_js_microbench_memory[1486]=(u8)110;dong_porf_js_microbench_memory[1487]=(u8)117;dong_porf_js_microbench_memory[1488]=(u8)108;dong_porf_js_microbench_memory[1489]=(u8)108;dong_porf_js_microbench_memory[1490]=(u8)105;dong_porf_js_microbench_memory[1491]=(u8)115;dong_porf_js_microbench_memory[1492]=(u8)104;
  dong_porf_js_microbench_memory[1495]=(u8)55;dong_porf_js_microbench_memory[1499]=(u8)78;dong_porf_js_microbench_memory[1500]=(u8)117;dong_porf_js_microbench_memory[1501]=(u8)109;dong_porf_js_microbench_memory[1502]=(u8)98;dong_porf_js_microbench_memory[1503]=(u8)101;dong_porf_js_microbench_memory[1504]=(u8)114;dong_porf_js_microbench_memory[1505]=(u8)46;dong_porf_js_microbench_memory[1506]=(u8)112;dong_porf_js_microbench_memory[1507]=(u8)114;dong_porf_js_microbench_memory[1508]=(u8)111;dong_porf_js_microbench_memory[1509]=(u8)116;dong_porf_js_microbench_memory[1510]=(u8)111;dong_porf_js_microbench_memory[1511]=(u8)116;dong_porf_js_microbench_memory[1512]=(u8)121;dong_porf_js_microbench_memory[1513]=(u8)112;dong_porf_js_microbench_memory[1514]=(u8)101;dong_porf_js_microbench_memory[1515]=(u8)46;dong_porf_js_microbench_memory[1516]=(u8)116;dong_porf_js_microbench_memory[1517]=(u8)111;dong_porf_js_microbench_memory[1518]=(u8)83;dong_porf_js_microbench_memory[1519]=(u8)116;dong_porf_js_microbench_memory[1520]=(u8)114;dong_porf_js_microbench_memory[1521]=(u8)105;dong_porf_js_microbench_memory[1522]=(u8)110;dong_porf_js_microbench_memory[1523]=(u8)103;dong_porf_js_microbench_memory[1524]=(u8)32;dong_porf_js_microbench_memory[1525]=(u8)101;dong_porf_js_microbench_memory[1526]=(u8)120;dong_porf_js_microbench_memory[1527]=(u8)112;dong_porf_js_microbench_memory[1528]=(u8)101;dong_porf_js_microbench_memory[1529]=(u8)99;dong_porf_js_microbench_memory[1530]=(u8)116;dong_porf_js_microbench_memory[1531]=(u8)115;dong_porf_js_microbench_memory[1532]=(u8)32;dong_porf_js_microbench_memory[1533]=(u8)39;dong_porf_js_microbench_memory[1534]=(u8)116;dong_porf_js_microbench_memory[1535]=(u8)104;dong_porf_js_microbench_memory[1536]=(u8)105;dong_porf_js_microbench_memory[1537]=(u8)115;dong_porf_js_microbench_memory[1538]=(u8)39;dong_porf_js_microbench_memory[1539]=(u8)32;dong_porf_js_microbench_memory[1540]=(u8)116;dong_porf_js_microbench_memory[1541]=(u8)111;dong_porf_js_microbench_memory[1542]=(u8)32;dong_porf_js_microbench_memory[1543]=(u8)98;dong_porf_js_microbench_memory[1544]=(u8)101;dong_porf_js_microbench_memory[1545]=(u8)32;dong_porf_js_microbench_memory[1546]=(u8)97;dong_porf_js_microbench_memory[1547]=(u8)32;dong_porf_js_microbench_memory[1548]=(u8)78;dong_porf_js_microbench_memory[1549]=(u8)117;dong_porf_js_microbench_memory[1550]=(u8)109;dong_porf_js_microbench_memory[1551]=(u8)98;dong_porf_js_microbench_memory[1552]=(u8)101;dong_porf_js_microbench_memory[1553]=(u8)114;
  dong_porf_js_microbench_memory[1556]=(u8)50;dong_porf_js_microbench_memory[1560]=(u8)116;dong_porf_js_microbench_memory[1561]=(u8)111;dong_porf_js_microbench_memory[1562]=(u8)83;dong_porf_js_microbench_memory[1563]=(u8)116;dong_porf_js_microbench_memory[1564]=(u8)114;dong_porf_js_microbench_memory[1565]=(u8)105;dong_porf_js_microbench_memory[1566]=(u8)110;dong_porf_js_microbench_memory[1567]=(u8)103;dong_porf_js_microbench_memory[1568]=(u8)40;dong_porf_js_microbench_memory[1569]=(u8)41;dong_porf_js_microbench_memory[1570]=(u8)32;dong_porf_js_microbench_memory[1571]=(u8)114;dong_porf_js_microbench_memory[1572]=(u8)97;dong_porf_js_microbench_memory[1573]=(u8)100;dong_porf_js_microbench_memory[1574]=(u8)105;dong_porf_js_microbench_memory[1575]=(u8)120;dong_porf_js_microbench_memory[1576]=(u8)32;dong_porf_js_microbench_memory[1577]=(u8)97;dong_porf_js_microbench_memory[1578]=(u8)114;dong_porf_js_microbench_memory[1579]=(u8)103;dong_porf_js_microbench_memory[1580]=(u8)117;dong_porf_js_microbench_memory[1581]=(u8)109;dong_porf_js_microbench_memory[1582]=(u8)101;dong_porf_js_microbench_memory[1583]=(u8)110;dong_porf_js_microbench_memory[1584]=(u8)116;dong_porf_js_microbench_memory[1585]=(u8)32;dong_porf_js_microbench_memory[1586]=(u8)109;dong_porf_js_microbench_memory[1587]=(u8)117;dong_porf_js_microbench_memory[1588]=(u8)115;dong_porf_js_microbench_memory[1589]=(u8)116;dong_porf_js_microbench_memory[1590]=(u8)32;dong_porf_js_microbench_memory[1591]=(u8)98;dong_porf_js_microbench_memory[1592]=(u8)101;dong_porf_js_microbench_memory[1593]=(u8)32;dong_porf_js_microbench_memory[1594]=(u8)98;dong_porf_js_microbench_memory[1595]=(u8)101;dong_porf_js_microbench_memory[1596]=(u8)116;dong_porf_js_microbench_memory[1597]=(u8)119;dong_porf_js_microbench_memory[1598]=(u8)101;dong_porf_js_microbench_memory[1599]=(u8)101;dong_porf_js_microbench_memory[1600]=(u8)110;dong_porf_js_microbench_memory[1601]=(u8)32;dong_porf_js_microbench_memory[1602]=(u8)50;dong_porf_js_microbench_memory[1603]=(u8)32;dong_porf_js_microbench_memory[1604]=(u8)97;dong_porf_js_microbench_memory[1605]=(u8)110;dong_porf_js_microbench_memory[1606]=(u8)100;dong_porf_js_microbench_memory[1607]=(u8)32;dong_porf_js_microbench_memory[1608]=(u8)51;dong_porf_js_microbench_memory[1609]=(u8)54;
  dong_porf_js_microbench_memory[1612]=(u8)3;dong_porf_js_microbench_memory[1616]=(u8)78;dong_porf_js_microbench_memory[1617]=(u8)97;dong_porf_js_microbench_memory[1618]=(u8)78;
  dong_porf_js_microbench_memory[1621]=(u8)8;dong_porf_js_microbench_memory[1625]=(u8)73;dong_porf_js_microbench_memory[1626]=(u8)110;dong_porf_js_microbench_memory[1627]=(u8)102;dong_porf_js_microbench_memory[1628]=(u8)105;dong_porf_js_microbench_memory[1629]=(u8)110;dong_porf_js_microbench_memory[1630]=(u8)105;dong_porf_js_microbench_memory[1631]=(u8)116;dong_porf_js_microbench_memory[1632]=(u8)121;
  dong_porf_js_microbench_memory[1635]=(u8)9;dong_porf_js_microbench_memory[1639]=(u8)45;dong_porf_js_microbench_memory[1640]=(u8)73;dong_porf_js_microbench_memory[1641]=(u8)110;dong_porf_js_microbench_memory[1642]=(u8)102;dong_porf_js_microbench_memory[1643]=(u8)105;dong_porf_js_microbench_memory[1644]=(u8)110;dong_porf_js_microbench_memory[1645]=(u8)105;dong_porf_js_microbench_memory[1646]=(u8)116;dong_porf_js_microbench_memory[1647]=(u8)121;
  dong_porf_js_microbench_memory[1650]=(u8)1;dong_porf_js_microbench_memory[1654]=(u8)48;
  dong_porf_js_microbench_memory[1657]=(u8)1;dong_porf_js_microbench_memory[1661]=(u8)121;
  dong_porf_js_microbench_memory[1664]=(u8)1;dong_porf_js_microbench_memory[1668]=(u8)122;
  dong_porf_js_microbench_memory[1671]=(u8)33;dong_porf_js_microbench_memory[1675]=(u8)67;dong_porf_js_microbench_memory[1676]=(u8)97;dong_porf_js_microbench_memory[1677]=(u8)110;dong_porf_js_microbench_memory[1678]=(u8)110;dong_porf_js_microbench_memory[1679]=(u8)111;dong_porf_js_microbench_memory[1680]=(u8)116;dong_porf_js_microbench_memory[1681]=(u8)32;dong_porf_js_microbench_memory[1682]=(u8)114;dong_porf_js_microbench_memory[1683]=(u8)101;dong_porf_js_microbench_memory[1684]=(u8)97;dong_porf_js_microbench_memory[1685]=(u8)100;dong_porf_js_microbench_memory[1686]=(u8)32;dong_porf_js_microbench_memory[1687]=(u8)112;dong_porf_js_microbench_memory[1688]=(u8)114;dong_porf_js_microbench_memory[1689]=(u8)111;dong_porf_js_microbench_memory[1690]=(u8)112;dong_porf_js_microbench_memory[1691]=(u8)101;dong_porf_js_microbench_memory[1692]=(u8)114;dong_porf_js_microbench_memory[1693]=(u8)116;dong_porf_js_microbench_memory[1694]=(u8)121;dong_porf_js_microbench_memory[1695]=(u8)32;dong_porf_js_microbench_memory[1696]=(u8)111;dong_porf_js_microbench_memory[1697]=(u8)102;dong_porf_js_microbench_memory[1698]=(u8)32;dong_porf_js_microbench_memory[1699]=(u8)117;dong_porf_js_microbench_memory[1700]=(u8)110;dong_porf_js_microbench_memory[1701]=(u8)100;dong_porf_js_microbench_memory[1702]=(u8)101;dong_porf_js_microbench_memory[1703]=(u8)102;dong_porf_js_microbench_memory[1704]=(u8)105;dong_porf_js_microbench_memory[1705]=(u8)110;dong_porf_js_microbench_memory[1706]=(u8)101;dong_porf_js_microbench_memory[1707]=(u8)100;
  dong_porf_js_microbench_memory[1710]=(u8)7;dong_porf_js_microbench_memory[1714]=(u8)116;dong_porf_js_microbench_memory[1715]=(u8)111;dong_porf_js_microbench_memory[1716]=(u8)70;dong_porf_js_microbench_memory[1717]=(u8)105;dong_porf_js_microbench_memory[1718]=(u8)120;dong_porf_js_microbench_memory[1719]=(u8)101;dong_porf_js_microbench_memory[1720]=(u8)100;
  dong_porf_js_microbench_memory[1723]=(u8)27;dong_porf_js_microbench_memory[1727]=(u8)117;dong_porf_js_microbench_memory[1728]=(u8)110;dong_porf_js_microbench_memory[1729]=(u8)100;dong_porf_js_microbench_memory[1730]=(u8)101;dong_porf_js_microbench_memory[1731]=(u8)102;dong_porf_js_microbench_memory[1732]=(u8)105;dong_porf_js_microbench_memory[1733]=(u8)110;dong_porf_js_microbench_memory[1734]=(u8)101;dong_porf_js_microbench_memory[1735]=(u8)100;dong_porf_js_microbench_memory[1736]=(u8)32;dong_porf_js_microbench_memory[1737]=(u8)105;dong_porf_js_microbench_memory[1738]=(u8)115;dong_porf_js_microbench_memory[1739]=(u8)32;dong_porf_js_microbench_memory[1740]=(u8)110;dong_porf_js_microbench_memory[1741]=(u8)111;dong_porf_js_microbench_memory[1742]=(u8)116;dong_porf_js_microbench_memory[1743]=(u8)32;dong_porf_js_microbench_memory[1744]=(u8)97;dong_porf_js_microbench_memory[1745]=(u8)32;dong_porf_js_microbench_memory[1746]=(u8)102;dong_porf_js_microbench_memory[1747]=(u8)117;dong_porf_js_microbench_memory[1748]=(u8)110;dong_porf_js_microbench_memory[1749]=(u8)99;dong_porf_js_microbench_memory[1750]=(u8)116;dong_porf_js_microbench_memory[1751]=(u8)105;dong_porf_js_microbench_memory[1752]=(u8)111;dong_porf_js_microbench_memory[1753]=(u8)110;
  dong_porf_js_microbench_memory[1756]=(u8)54;dong_porf_js_microbench_memory[1760]=(u8)78;dong_porf_js_microbench_memory[1761]=(u8)117;dong_porf_js_microbench_memory[1762]=(u8)109;dong_porf_js_microbench_memory[1763]=(u8)98;dong_porf_js_microbench_memory[1764]=(u8)101;dong_porf_js_microbench_memory[1765]=(u8)114;dong_porf_js_microbench_memory[1766]=(u8)46;dong_porf_js_microbench_memory[1767]=(u8)112;dong_porf_js_microbench_memory[1768]=(u8)114;dong_porf_js_microbench_memory[1769]=(u8)111;dong_porf_js_microbench_memory[1770]=(u8)116;dong_porf_js_microbench_memory[1771]=(u8)111;dong_porf_js_microbench_memory[1772]=(u8)116;dong_porf_js_microbench_memory[1773]=(u8)121;dong_porf_js_microbench_memory[1774]=(u8)112;dong_porf_js_microbench_memory[1775]=(u8)101;dong_porf_js_microbench_memory[1776]=(u8)46;dong_porf_js_microbench_memory[1777]=(u8)116;dong_porf_js_microbench_memory[1778]=(u8)111;dong_porf_js_microbench_memory[1779]=(u8)70;dong_porf_js_microbench_memory[1780]=(u8)105;dong_porf_js_microbench_memory[1781]=(u8)120;dong_porf_js_microbench_memory[1782]=(u8)101;dong_porf_js_microbench_memory[1783]=(u8)100;dong_porf_js_microbench_memory[1784]=(u8)32;dong_porf_js_microbench_memory[1785]=(u8)101;dong_porf_js_microbench_memory[1786]=(u8)120;dong_porf_js_microbench_memory[1787]=(u8)112;dong_porf_js_microbench_memory[1788]=(u8)101;dong_porf_js_microbench_memory[1789]=(u8)99;dong_porf_js_microbench_memory[1790]=(u8)116;dong_porf_js_microbench_memory[1791]=(u8)115;dong_porf_js_microbench_memory[1792]=(u8)32;dong_porf_js_microbench_memory[1793]=(u8)39;dong_porf_js_microbench_memory[1794]=(u8)116;dong_porf_js_microbench_memory[1795]=(u8)104;dong_porf_js_microbench_memory[1796]=(u8)105;dong_porf_js_microbench_memory[1797]=(u8)115;dong_porf_js_microbench_memory[1798]=(u8)39;dong_porf_js_microbench_memory[1799]=(u8)32;dong_porf_js_microbench_memory[1800]=(u8)116;dong_porf_js_microbench_memory[1801]=(u8)111;dong_porf_js_microbench_memory[1802]=(u8)32;dong_porf_js_microbench_memory[1803]=(u8)98;dong_porf_js_microbench_memory[1804]=(u8)101;dong_porf_js_microbench_memory[1805]=(u8)32;dong_porf_js_microbench_memory[1806]=(u8)97;dong_porf_js_microbench_memory[1807]=(u8)32;dong_porf_js_microbench_memory[1808]=(u8)78;dong_porf_js_microbench_memory[1809]=(u8)117;dong_porf_js_microbench_memory[1810]=(u8)109;dong_porf_js_microbench_memory[1811]=(u8)98;dong_porf_js_microbench_memory[1812]=(u8)101;dong_porf_js_microbench_memory[1813]=(u8)114;
  dong_porf_js_microbench_memory[1816]=(u8)59;dong_porf_js_microbench_memory[1820]=(u8)116;dong_porf_js_microbench_memory[1821]=(u8)111;dong_porf_js_microbench_memory[1822]=(u8)70;dong_porf_js_microbench_memory[1823]=(u8)105;dong_porf_js_microbench_memory[1824]=(u8)120;dong_porf_js_microbench_memory[1825]=(u8)101;dong_porf_js_microbench_memory[1826]=(u8)100;dong_porf_js_microbench_memory[1827]=(u8)40;dong_porf_js_microbench_memory[1828]=(u8)41;dong_porf_js_microbench_memory[1829]=(u8)32;dong_porf_js_microbench_memory[1830]=(u8)102;dong_porf_js_microbench_memory[1831]=(u8)114;dong_porf_js_microbench_memory[1832]=(u8)97;dong_porf_js_microbench_memory[1833]=(u8)99;dong_porf_js_microbench_memory[1834]=(u8)116;dong_porf_js_microbench_memory[1835]=(u8)105;dong_porf_js_microbench_memory[1836]=(u8)111;dong_porf_js_microbench_memory[1837]=(u8)110;dong_porf_js_microbench_memory[1838]=(u8)68;dong_porf_js_microbench_memory[1839]=(u8)105;dong_porf_js_microbench_memory[1840]=(u8)103;dong_porf_js_microbench_memory[1841]=(u8)105;dong_porf_js_microbench_memory[1842]=(u8)116;dong_porf_js_microbench_memory[1843]=(u8)115;dong_porf_js_microbench_memory[1844]=(u8)32;dong_porf_js_microbench_memory[1845]=(u8)97;dong_porf_js_microbench_memory[1846]=(u8)114;dong_porf_js_microbench_memory[1847]=(u8)103;dong_porf_js_microbench_memory[1848]=(u8)117;dong_porf_js_microbench_memory[1849]=(u8)109;dong_porf_js_microbench_memory[1850]=(u8)101;dong_porf_js_microbench_memory[1851]=(u8)110;dong_porf_js_microbench_memory[1852]=(u8)116;dong_porf_js_microbench_memory[1853]=(u8)32;dong_porf_js_microbench_memory[1854]=(u8)109;dong_porf_js_microbench_memory[1855]=(u8)117;dong_porf_js_microbench_memory[1856]=(u8)115;dong_porf_js_microbench_memory[1857]=(u8)116;dong_porf_js_microbench_memory[1858]=(u8)32;dong_porf_js_microbench_memory[1859]=(u8)98;dong_porf_js_microbench_memory[1860]=(u8)101;dong_porf_js_microbench_memory[1861]=(u8)32;dong_porf_js_microbench_memory[1862]=(u8)98;dong_porf_js_microbench_memory[1863]=(u8)101;dong_porf_js_microbench_memory[1864]=(u8)116;dong_porf_js_microbench_memory[1865]=(u8)119;dong_porf_js_microbench_memory[1866]=(u8)101;dong_porf_js_microbench_memory[1867]=(u8)101;dong_porf_js_microbench_memory[1868]=(u8)110;dong_porf_js_microbench_memory[1869]=(u8)32;dong_porf_js_microbench_memory[1870]=(u8)48;dong_porf_js_microbench_memory[1871]=(u8)32;dong_porf_js_microbench_memory[1872]=(u8)97;dong_porf_js_microbench_memory[1873]=(u8)110;dong_porf_js_microbench_memory[1874]=(u8)100;dong_porf_js_microbench_memory[1875]=(u8)32;dong_porf_js_microbench_memory[1876]=(u8)49;dong_porf_js_microbench_memory[1877]=(u8)48;dong_porf_js_microbench_memory[1878]=(u8)48;
  dong_porf_js_microbench_memory[1881]=(u8)8;dong_porf_js_microbench_memory[1885]=(u8)91;dong_porf_js_microbench_memory[1886]=(u8)66;dong_porf_js_microbench_memory[1887]=(u8)69;dong_porf_js_microbench_memory[1888]=(u8)78;dong_porf_js_microbench_memory[1889]=(u8)67;dong_porf_js_microbench_memory[1890]=(u8)72;dong_porf_js_microbench_memory[1891]=(u8)93;dong_porf_js_microbench_memory[1892]=(u8)32;
  dong_porf_js_microbench_memory[1895]=(u8)4;dong_porf_js_microbench_memory[1899]=(u8)109;dong_porf_js_microbench_memory[1900]=(u8)115;dong_porf_js_microbench_memory[1901]=(u8)32;dong_porf_js_microbench_memory[1902]=(u8)40;
  dong_porf_js_microbench_memory[1905]=(u8)13;dong_porf_js_microbench_memory[1909]=(u8)32;dong_porf_js_microbench_memory[1910]=(u8)105;dong_porf_js_microbench_memory[1911]=(u8)116;dong_porf_js_microbench_memory[1912]=(u8)101;dong_porf_js_microbench_memory[1913]=(u8)114;dong_porf_js_microbench_memory[1914]=(u8)97;dong_porf_js_microbench_memory[1915]=(u8)116;dong_porf_js_microbench_memory[1916]=(u8)105;dong_porf_js_microbench_memory[1917]=(u8)111;dong_porf_js_microbench_memory[1918]=(u8)110;dong_porf_js_microbench_memory[1919]=(u8)115;dong_porf_js_microbench_memory[1920]=(u8)44;dong_porf_js_microbench_memory[1921]=(u8)32;
  dong_porf_js_microbench_memory[1924]=(u8)9;dong_porf_js_microbench_memory[1928]=(u8)32;dong_porf_js_microbench_memory[1929]=(u8)110;dong_porf_js_microbench_memory[1930]=(u8)115;dong_porf_js_microbench_memory[1931]=(u8)47;dong_porf_js_microbench_memory[1932]=(u8)105;dong_porf_js_microbench_memory[1933]=(u8)116;dong_porf_js_microbench_memory[1934]=(u8)101;dong_porf_js_microbench_memory[1935]=(u8)114;dong_porf_js_microbench_memory[1936]=(u8)41;
  dong_porf_js_microbench_memory[1939]=(u8)10;dong_porf_js_microbench_memory[1943]=(u8)99;dong_porf_js_microbench_memory[1944]=(u8)104;dong_porf_js_microbench_memory[1945]=(u8)97;dong_porf_js_microbench_memory[1946]=(u8)114;dong_porf_js_microbench_memory[1947]=(u8)67;dong_porf_js_microbench_memory[1948]=(u8)111;dong_porf_js_microbench_memory[1949]=(u8)100;dong_porf_js_microbench_memory[1950]=(u8)101;dong_porf_js_microbench_memory[1951]=(u8)65;dong_porf_js_microbench_memory[1952]=(u8)116;
  dong_porf_js_microbench_memory[1955]=(u8)60;dong_porf_js_microbench_memory[1959]=(u8)83;dong_porf_js_microbench_memory[1960]=(u8)116;dong_porf_js_microbench_memory[1961]=(u8)114;dong_porf_js_microbench_memory[1962]=(u8)105;dong_porf_js_microbench_memory[1963]=(u8)110;dong_porf_js_microbench_memory[1964]=(u8)103;dong_porf_js_microbench_memory[1965]=(u8)46;dong_porf_js_microbench_memory[1966]=(u8)112;dong_porf_js_microbench_memory[1967]=(u8)114;dong_porf_js_microbench_memory[1968]=(u8)111;dong_porf_js_microbench_memory[1969]=(u8)116;dong_porf_js_microbench_memory[1970]=(u8)111;dong_porf_js_microbench_memory[1971]=(u8)116;dong_porf_js_microbench_memory[1972]=(u8)121;dong_porf_js_microbench_memory[1973]=(u8)112;dong_porf_js_microbench_memory[1974]=(u8)101;dong_porf_js_microbench_memory[1975]=(u8)46;dong_porf_js_microbench_memory[1976]=(u8)99;dong_porf_js_microbench_memory[1977]=(u8)104;dong_porf_js_microbench_memory[1978]=(u8)97;dong_porf_js_microbench_memory[1979]=(u8)114;dong_porf_js_microbench_memory[1980]=(u8)67;dong_porf_js_microbench_memory[1981]=(u8)111;dong_porf_js_microbench_memory[1982]=(u8)100;dong_porf_js_microbench_memory[1983]=(u8)101;dong_porf_js_microbench_memory[1984]=(u8)65;dong_porf_js_microbench_memory[1985]=(u8)116;dong_porf_js_microbench_memory[1986]=(u8)32;dong_porf_js_microbench_memory[1987]=(u8)101;dong_porf_js_microbench_memory[1988]=(u8)120;dong_porf_js_microbench_memory[1989]=(u8)112;dong_porf_js_microbench_memory[1990]=(u8)101;dong_porf_js_microbench_memory[1991]=(u8)99;dong_porf_js_microbench_memory[1992]=(u8)116;dong_porf_js_microbench_memory[1993]=(u8)115;dong_porf_js_microbench_memory[1994]=(u8)32;dong_porf_js_microbench_memory[1995]=(u8)39;dong_porf_js_microbench_memory[1996]=(u8)116;dong_porf_js_microbench_memory[1997]=(u8)104;dong_porf_js_microbench_memory[1998]=(u8)105;dong_porf_js_microbench_memory[1999]=(u8)115;dong_porf_js_microbench_memory[2000]=(u8)39;dong_porf_js_microbench_memory[2001]=(u8)32;dong_porf_js_microbench_memory[2002]=(u8)116;dong_porf_js_microbench_memory[2003]=(u8)111;dong_porf_js_microbench_memory[2004]=(u8)32;dong_porf_js_microbench_memory[2005]=(u8)98;dong_porf_js_microbench_memory[2006]=(u8)101;dong_porf_js_microbench_memory[2007]=(u8)32;dong_porf_js_microbench_memory[2008]=(u8)110;dong_porf_js_microbench_memory[2009]=(u8)111;dong_porf_js_microbench_memory[2010]=(u8)110;dong_porf_js_microbench_memory[2011]=(u8)45;dong_porf_js_microbench_memory[2012]=(u8)110;dong_porf_js_microbench_memory[2013]=(u8)117;dong_porf_js_microbench_memory[2014]=(u8)108;dong_porf_js_microbench_memory[2015]=(u8)108;dong_porf_js_microbench_memory[2016]=(u8)105;dong_porf_js_microbench_memory[2017]=(u8)115;dong_porf_js_microbench_memory[2018]=(u8)104;
  dong_porf_js_microbench_memory[2021]=(u8)13;dong_porf_js_microbench_memory[2025]=(u8)102;dong_porf_js_microbench_memory[2026]=(u8)117;dong_porf_js_microbench_memory[2027]=(u8)110;dong_porf_js_microbench_memory[2028]=(u8)99;dong_porf_js_microbench_memory[2029]=(u8)116;dong_porf_js_microbench_memory[2030]=(u8)105;dong_porf_js_microbench_memory[2031]=(u8)111;dong_porf_js_microbench_memory[2032]=(u8)110;dong_porf_js_microbench_memory[2033]=(u8)95;dong_porf_js_microbench_memory[2034]=(u8)99;dong_porf_js_microbench_memory[2035]=(u8)97;dong_porf_js_microbench_memory[2036]=(u8)108;dong_porf_js_microbench_memory[2037]=(u8)108;
  dong_porf_js_microbench_memory[2040]=(u8)13;dong_porf_js_microbench_memory[2044]=(u8)111;dong_porf_js_microbench_memory[2045]=(u8)98;dong_porf_js_microbench_memory[2046]=(u8)106;dong_porf_js_microbench_memory[2047]=(u8)101;dong_porf_js_microbench_memory[2048]=(u8)99;dong_porf_js_microbench_memory[2049]=(u8)116;dong_porf_js_microbench_memory[2050]=(u8)95;dong_porf_js_microbench_memory[2051]=(u8)99;dong_porf_js_microbench_memory[2052]=(u8)114;dong_porf_js_microbench_memory[2053]=(u8)101;dong_porf_js_microbench_memory[2054]=(u8)97;dong_porf_js_microbench_memory[2055]=(u8)116;dong_porf_js_microbench_memory[2056]=(u8)101;
  dong_porf_js_microbench_memory[2059]=(u8)1;dong_porf_js_microbench_memory[2063]=(u8)97;
  dong_porf_js_microbench_memory[2066]=(u8)1;dong_porf_js_microbench_memory[2070]=(u8)98;
  dong_porf_js_microbench_memory[2073]=(u8)1;dong_porf_js_microbench_memory[2077]=(u8)99;
  dong_porf_js_microbench_memory[2080]=(u8)10;dong_porf_js_microbench_memory[2084]=(u8)106;dong_porf_js_microbench_memory[2085]=(u8)115;dong_porf_js_microbench_memory[2086]=(u8)111;dong_porf_js_microbench_memory[2087]=(u8)110;dong_porf_js_microbench_memory[2088]=(u8)95;dong_porf_js_microbench_memory[2089]=(u8)112;dong_porf_js_microbench_memory[2090]=(u8)97;dong_porf_js_microbench_memory[2091]=(u8)114;dong_porf_js_microbench_memory[2092]=(u8)115;dong_porf_js_microbench_memory[2093]=(u8)101;
  dong_porf_js_microbench_memory[2096]=(u8)35;dong_porf_js_microbench_memory[2100]=(u8)123;dong_porf_js_microbench_memory[2101]=(u8)34;dong_porf_js_microbench_memory[2102]=(u8)105;dong_porf_js_microbench_memory[2103]=(u8)116;dong_porf_js_microbench_memory[2104]=(u8)101;dong_porf_js_microbench_memory[2105]=(u8)109;dong_porf_js_microbench_memory[2106]=(u8)115;dong_porf_js_microbench_memory[2107]=(u8)34;dong_porf_js_microbench_memory[2108]=(u8)58;dong_porf_js_microbench_memory[2109]=(u8)91;dong_porf_js_microbench_memory[2110]=(u8)123;dong_porf_js_microbench_memory[2111]=(u8)34;dong_porf_js_microbench_memory[2112]=(u8)105;dong_porf_js_microbench_memory[2113]=(u8)100;dong_porf_js_microbench_memory[2114]=(u8)34;dong_porf_js_microbench_memory[2115]=(u8)58;dong_porf_js_microbench_memory[2116]=(u8)49;dong_porf_js_microbench_memory[2117]=(u8)44;dong_porf_js_microbench_memory[2118]=(u8)34;dong_porf_js_microbench_memory[2119]=(u8)110;dong_porf_js_microbench_memory[2120]=(u8)97;dong_porf_js_microbench_memory[2121]=(u8)109;dong_porf_js_microbench_memory[2122]=(u8)101;dong_porf_js_microbench_memory[2123]=(u8)34;dong_porf_js_microbench_memory[2124]=(u8)58;dong_porf_js_microbench_memory[2125]=(u8)34;dong_porf_js_microbench_memory[2126]=(u8)105;dong_porf_js_microbench_memory[2127]=(u8)116;dong_porf_js_microbench_memory[2128]=(u8)101;dong_porf_js_microbench_memory[2129]=(u8)109;dong_porf_js_microbench_memory[2130]=(u8)49;dong_porf_js_microbench_memory[2131]=(u8)34;dong_porf_js_microbench_memory[2132]=(u8)125;dong_porf_js_microbench_memory[2133]=(u8)93;dong_porf_js_microbench_memory[2134]=(u8)125;
  dong_porf_js_microbench_memory[2137]=(u8)28;dong_porf_js_microbench_memory[2141]=(u8)85;dong_porf_js_microbench_memory[2142]=(u8)110;dong_porf_js_microbench_memory[2143]=(u8)101;dong_porf_js_microbench_memory[2144]=(u8)120;dong_porf_js_microbench_memory[2145]=(u8)112;dong_porf_js_microbench_memory[2146]=(u8)101;dong_porf_js_microbench_memory[2147]=(u8)99;dong_porf_js_microbench_memory[2148]=(u8)116;dong_porf_js_microbench_memory[2149]=(u8)101;dong_porf_js_microbench_memory[2150]=(u8)100;dong_porf_js_microbench_memory[2151]=(u8)32;dong_porf_js_microbench_memory[2152]=(u8)101;dong_porf_js_microbench_memory[2153]=(u8)110;dong_porf_js_microbench_memory[2154]=(u8)100;dong_porf_js_microbench_memory[2155]=(u8)32;dong_porf_js_microbench_memory[2156]=(u8)111;dong_porf_js_microbench_memory[2157]=(u8)102;dong_porf_js_microbench_memory[2158]=(u8)32;dong_porf_js_microbench_memory[2159]=(u8)74;dong_porf_js_microbench_memory[2160]=(u8)83;dong_porf_js_microbench_memory[2161]=(u8)79;dong_porf_js_microbench_memory[2162]=(u8)78;dong_porf_js_microbench_memory[2163]=(u8)32;dong_porf_js_microbench_memory[2164]=(u8)105;dong_porf_js_microbench_memory[2165]=(u8)110;dong_porf_js_microbench_memory[2166]=(u8)112;dong_porf_js_microbench_memory[2167]=(u8)117;dong_porf_js_microbench_memory[2168]=(u8)116;
  dong_porf_js_microbench_memory[2171]=(u8)16;dong_porf_js_microbench_memory[2175]=(u8)85;dong_porf_js_microbench_memory[2176]=(u8)110;dong_porf_js_microbench_memory[2177]=(u8)101;dong_porf_js_microbench_memory[2178]=(u8)120;dong_porf_js_microbench_memory[2179]=(u8)112;dong_porf_js_microbench_memory[2180]=(u8)101;dong_porf_js_microbench_memory[2181]=(u8)99;dong_porf_js_microbench_memory[2182]=(u8)116;dong_porf_js_microbench_memory[2183]=(u8)101;dong_porf_js_microbench_memory[2184]=(u8)100;dong_porf_js_microbench_memory[2185]=(u8)32;dong_porf_js_microbench_memory[2186]=(u8)116;dong_porf_js_microbench_memory[2187]=(u8)111;dong_porf_js_microbench_memory[2188]=(u8)107;dong_porf_js_microbench_memory[2189]=(u8)101;dong_porf_js_microbench_memory[2190]=(u8)110;
  dong_porf_js_microbench_memory[2193]=(u8)19;dong_porf_js_microbench_memory[2197]=(u8)85;dong_porf_js_microbench_memory[2198]=(u8)110;dong_porf_js_microbench_memory[2199]=(u8)116;dong_porf_js_microbench_memory[2200]=(u8)101;dong_porf_js_microbench_memory[2201]=(u8)114;dong_porf_js_microbench_memory[2202]=(u8)109;dong_porf_js_microbench_memory[2203]=(u8)105;dong_porf_js_microbench_memory[2204]=(u8)110;dong_porf_js_microbench_memory[2205]=(u8)97;dong_porf_js_microbench_memory[2206]=(u8)116;dong_porf_js_microbench_memory[2207]=(u8)101;dong_porf_js_microbench_memory[2208]=(u8)100;dong_porf_js_microbench_memory[2209]=(u8)32;dong_porf_js_microbench_memory[2210]=(u8)115;dong_porf_js_microbench_memory[2211]=(u8)116;dong_porf_js_microbench_memory[2212]=(u8)114;dong_porf_js_microbench_memory[2213]=(u8)105;dong_porf_js_microbench_memory[2214]=(u8)110;dong_porf_js_microbench_memory[2215]=(u8)103;
  dong_porf_js_microbench_memory[2218]=(u8)22;dong_porf_js_microbench_memory[2222]=(u8)73;dong_porf_js_microbench_memory[2223]=(u8)110;dong_porf_js_microbench_memory[2224]=(u8)118;dong_porf_js_microbench_memory[2225]=(u8)97;dong_porf_js_microbench_memory[2226]=(u8)108;dong_porf_js_microbench_memory[2227]=(u8)105;dong_porf_js_microbench_memory[2228]=(u8)100;dong_porf_js_microbench_memory[2229]=(u8)32;dong_porf_js_microbench_memory[2230]=(u8)117;dong_porf_js_microbench_memory[2231]=(u8)110;dong_porf_js_microbench_memory[2232]=(u8)105;dong_porf_js_microbench_memory[2233]=(u8)99;dong_porf_js_microbench_memory[2234]=(u8)111;dong_porf_js_microbench_memory[2235]=(u8)100;dong_porf_js_microbench_memory[2236]=(u8)101;dong_porf_js_microbench_memory[2237]=(u8)32;dong_porf_js_microbench_memory[2238]=(u8)101;dong_porf_js_microbench_memory[2239]=(u8)115;dong_porf_js_microbench_memory[2240]=(u8)99;dong_porf_js_microbench_memory[2241]=(u8)97;dong_porf_js_microbench_memory[2242]=(u8)112;dong_porf_js_microbench_memory[2243]=(u8)101;
  dong_porf_js_microbench_memory[2246]=(u8)23;dong_porf_js_microbench_memory[2250]=(u8)73;dong_porf_js_microbench_memory[2251]=(u8)110;dong_porf_js_microbench_memory[2252]=(u8)118;dong_porf_js_microbench_memory[2253]=(u8)97;dong_porf_js_microbench_memory[2254]=(u8)108;dong_porf_js_microbench_memory[2255]=(u8)105;dong_porf_js_microbench_memory[2256]=(u8)100;dong_porf_js_microbench_memory[2257]=(u8)32;dong_porf_js_microbench_memory[2258]=(u8)101;dong_porf_js_microbench_memory[2259]=(u8)115;dong_porf_js_microbench_memory[2260]=(u8)99;dong_porf_js_microbench_memory[2261]=(u8)97;dong_porf_js_microbench_memory[2262]=(u8)112;dong_porf_js_microbench_memory[2263]=(u8)101;dong_porf_js_microbench_memory[2264]=(u8)32;dong_porf_js_microbench_memory[2265]=(u8)115;dong_porf_js_microbench_memory[2266]=(u8)101;dong_porf_js_microbench_memory[2267]=(u8)113;dong_porf_js_microbench_memory[2268]=(u8)117;dong_porf_js_microbench_memory[2269]=(u8)101;dong_porf_js_microbench_memory[2270]=(u8)110;dong_porf_js_microbench_memory[2271]=(u8)99;dong_porf_js_microbench_memory[2272]=(u8)101;
  dong_porf_js_microbench_memory[2275]=(u8)27;dong_porf_js_microbench_memory[2279]=(u8)85;dong_porf_js_microbench_memory[2280]=(u8)110;dong_porf_js_microbench_memory[2281]=(u8)101;dong_porf_js_microbench_memory[2282]=(u8)115;dong_porf_js_microbench_memory[2283]=(u8)99;dong_porf_js_microbench_memory[2284]=(u8)97;dong_porf_js_microbench_memory[2285]=(u8)112;dong_porf_js_microbench_memory[2286]=(u8)101;dong_porf_js_microbench_memory[2287]=(u8)100;dong_porf_js_microbench_memory[2288]=(u8)32;dong_porf_js_microbench_memory[2289]=(u8)99;dong_porf_js_microbench_memory[2290]=(u8)111;dong_porf_js_microbench_memory[2291]=(u8)110;dong_porf_js_microbench_memory[2292]=(u8)116;dong_porf_js_microbench_memory[2293]=(u8)114;dong_porf_js_microbench_memory[2294]=(u8)111;dong_porf_js_microbench_memory[2295]=(u8)108;dong_porf_js_microbench_memory[2296]=(u8)32;dong_porf_js_microbench_memory[2297]=(u8)99;dong_porf_js_microbench_memory[2298]=(u8)104;dong_porf_js_microbench_memory[2299]=(u8)97;dong_porf_js_microbench_memory[2300]=(u8)114;dong_porf_js_microbench_memory[2301]=(u8)97;dong_porf_js_microbench_memory[2302]=(u8)99;dong_porf_js_microbench_memory[2303]=(u8)116;dong_porf_js_microbench_memory[2304]=(u8)101;dong_porf_js_microbench_memory[2305]=(u8)114;
  dong_porf_js_microbench_memory[2308]=(u8)18;dong_porf_js_microbench_memory[2312]=(u8)85;dong_porf_js_microbench_memory[2313]=(u8)110;dong_porf_js_microbench_memory[2314]=(u8)116;dong_porf_js_microbench_memory[2315]=(u8)101;dong_porf_js_microbench_memory[2316]=(u8)114;dong_porf_js_microbench_memory[2317]=(u8)109;dong_porf_js_microbench_memory[2318]=(u8)105;dong_porf_js_microbench_memory[2319]=(u8)110;dong_porf_js_microbench_memory[2320]=(u8)97;dong_porf_js_microbench_memory[2321]=(u8)116;dong_porf_js_microbench_memory[2322]=(u8)101;dong_porf_js_microbench_memory[2323]=(u8)100;dong_porf_js_microbench_memory[2324]=(u8)32;dong_porf_js_microbench_memory[2325]=(u8)97;dong_porf_js_microbench_memory[2326]=(u8)114;dong_porf_js_microbench_memory[2327]=(u8)114;dong_porf_js_microbench_memory[2328]=(u8)97;dong_porf_js_microbench_memory[2329]=(u8)121;
  dong_porf_js_microbench_memory[2332]=(u8)15;dong_porf_js_microbench_memory[2336]=(u8)69;dong_porf_js_microbench_memory[2337]=(u8)120;dong_porf_js_microbench_memory[2338]=(u8)112;dong_porf_js_microbench_memory[2339]=(u8)101;dong_porf_js_microbench_memory[2340]=(u8)99;dong_porf_js_microbench_memory[2341]=(u8)116;dong_porf_js_microbench_memory[2342]=(u8)101;dong_porf_js_microbench_memory[2343]=(u8)100;dong_porf_js_microbench_memory[2344]=(u8)32;dong_porf_js_microbench_memory[2345]=(u8)44;dong_porf_js_microbench_memory[2346]=(u8)32;dong_porf_js_microbench_memory[2347]=(u8)111;dong_porf_js_microbench_memory[2348]=(u8)114;dong_porf_js_microbench_memory[2349]=(u8)32;dong_porf_js_microbench_memory[2350]=(u8)93;
  dong_porf_js_microbench_memory[2353]=(u8)19;dong_porf_js_microbench_memory[2357]=(u8)69;dong_porf_js_microbench_memory[2358]=(u8)120;dong_porf_js_microbench_memory[2359]=(u8)112;dong_porf_js_microbench_memory[2360]=(u8)101;dong_porf_js_microbench_memory[2361]=(u8)99;dong_porf_js_microbench_memory[2362]=(u8)116;dong_porf_js_microbench_memory[2363]=(u8)101;dong_porf_js_microbench_memory[2364]=(u8)100;dong_porf_js_microbench_memory[2365]=(u8)32;dong_porf_js_microbench_memory[2366]=(u8)115;dong_porf_js_microbench_memory[2367]=(u8)116;dong_porf_js_microbench_memory[2368]=(u8)114;dong_porf_js_microbench_memory[2369]=(u8)105;dong_porf_js_microbench_memory[2370]=(u8)110;dong_porf_js_microbench_memory[2371]=(u8)103;dong_porf_js_microbench_memory[2372]=(u8)32;dong_porf_js_microbench_memory[2373]=(u8)107;dong_porf_js_microbench_memory[2374]=(u8)101;dong_porf_js_microbench_memory[2375]=(u8)121;
  dong_porf_js_microbench_memory[2378]=(u8)10;dong_porf_js_microbench_memory[2382]=(u8)69;dong_porf_js_microbench_memory[2383]=(u8)120;dong_porf_js_microbench_memory[2384]=(u8)112;dong_porf_js_microbench_memory[2385]=(u8)101;dong_porf_js_microbench_memory[2386]=(u8)99;dong_porf_js_microbench_memory[2387]=(u8)116;dong_porf_js_microbench_memory[2388]=(u8)101;dong_porf_js_microbench_memory[2389]=(u8)100;dong_porf_js_microbench_memory[2390]=(u8)32;dong_porf_js_microbench_memory[2391]=(u8)58;
  dong_porf_js_microbench_memory[2394]=(u8)32;dong_porf_js_microbench_memory[2398]=(u8)67;dong_porf_js_microbench_memory[2399]=(u8)97;dong_porf_js_microbench_memory[2400]=(u8)110;dong_porf_js_microbench_memory[2401]=(u8)110;dong_porf_js_microbench_memory[2402]=(u8)111;dong_porf_js_microbench_memory[2403]=(u8)116;dong_porf_js_microbench_memory[2404]=(u8)32;dong_porf_js_microbench_memory[2405]=(u8)115;dong_porf_js_microbench_memory[2406]=(u8)101;dong_porf_js_microbench_memory[2407]=(u8)116;dong_porf_js_microbench_memory[2408]=(u8)32;dong_porf_js_microbench_memory[2409]=(u8)112;dong_porf_js_microbench_memory[2410]=(u8)114;dong_porf_js_microbench_memory[2411]=(u8)111;dong_porf_js_microbench_memory[2412]=(u8)112;dong_porf_js_microbench_memory[2413]=(u8)101;dong_porf_js_microbench_memory[2414]=(u8)114;dong_porf_js_microbench_memory[2415]=(u8)116;dong_porf_js_microbench_memory[2416]=(u8)121;dong_porf_js_microbench_memory[2417]=(u8)32;dong_porf_js_microbench_memory[2418]=(u8)111;dong_porf_js_microbench_memory[2419]=(u8)102;dong_porf_js_microbench_memory[2420]=(u8)32;dong_porf_js_microbench_memory[2421]=(u8)117;dong_porf_js_microbench_memory[2422]=(u8)110;dong_porf_js_microbench_memory[2423]=(u8)100;dong_porf_js_microbench_memory[2424]=(u8)101;dong_porf_js_microbench_memory[2425]=(u8)102;dong_porf_js_microbench_memory[2426]=(u8)105;dong_porf_js_microbench_memory[2427]=(u8)110;dong_porf_js_microbench_memory[2428]=(u8)101;dong_porf_js_microbench_memory[2429]=(u8)100;
  dong_porf_js_microbench_memory[2432]=(u8)27;dong_porf_js_microbench_memory[2436]=(u8)67;dong_porf_js_microbench_memory[2437]=(u8)97;dong_porf_js_microbench_memory[2438]=(u8)110;dong_porf_js_microbench_memory[2439]=(u8)110;dong_porf_js_microbench_memory[2440]=(u8)111;dong_porf_js_microbench_memory[2441]=(u8)116;dong_porf_js_microbench_memory[2442]=(u8)32;dong_porf_js_microbench_memory[2443]=(u8)115;dong_porf_js_microbench_memory[2444]=(u8)101;dong_porf_js_microbench_memory[2445]=(u8)116;dong_porf_js_microbench_memory[2446]=(u8)32;dong_porf_js_microbench_memory[2447]=(u8)112;dong_porf_js_microbench_memory[2448]=(u8)114;dong_porf_js_microbench_memory[2449]=(u8)111;dong_porf_js_microbench_memory[2450]=(u8)112;dong_porf_js_microbench_memory[2451]=(u8)101;dong_porf_js_microbench_memory[2452]=(u8)114;dong_porf_js_microbench_memory[2453]=(u8)116;dong_porf_js_microbench_memory[2454]=(u8)121;dong_porf_js_microbench_memory[2455]=(u8)32;dong_porf_js_microbench_memory[2456]=(u8)111;dong_porf_js_microbench_memory[2457]=(u8)102;dong_porf_js_microbench_memory[2458]=(u8)32;dong_porf_js_microbench_memory[2459]=(u8)110;dong_porf_js_microbench_memory[2460]=(u8)117;dong_porf_js_microbench_memory[2461]=(u8)108;dong_porf_js_microbench_memory[2462]=(u8)108;
  dong_porf_js_microbench_memory[2465]=(u8)19;dong_porf_js_microbench_memory[2469]=(u8)85;dong_porf_js_microbench_memory[2470]=(u8)110;dong_porf_js_microbench_memory[2471]=(u8)116;dong_porf_js_microbench_memory[2472]=(u8)101;dong_porf_js_microbench_memory[2473]=(u8)114;dong_porf_js_microbench_memory[2474]=(u8)109;dong_porf_js_microbench_memory[2475]=(u8)105;dong_porf_js_microbench_memory[2476]=(u8)110;dong_porf_js_microbench_memory[2477]=(u8)97;dong_porf_js_microbench_memory[2478]=(u8)116;dong_porf_js_microbench_memory[2479]=(u8)101;dong_porf_js_microbench_memory[2480]=(u8)100;dong_porf_js_microbench_memory[2481]=(u8)32;dong_porf_js_microbench_memory[2482]=(u8)111;dong_porf_js_microbench_memory[2483]=(u8)98;dong_porf_js_microbench_memory[2484]=(u8)106;dong_porf_js_microbench_memory[2485]=(u8)101;dong_porf_js_microbench_memory[2486]=(u8)99;dong_porf_js_microbench_memory[2487]=(u8)116;
  dong_porf_js_microbench_memory[2490]=(u8)15;dong_porf_js_microbench_memory[2494]=(u8)69;dong_porf_js_microbench_memory[2495]=(u8)120;dong_porf_js_microbench_memory[2496]=(u8)112;dong_porf_js_microbench_memory[2497]=(u8)101;dong_porf_js_microbench_memory[2498]=(u8)99;dong_porf_js_microbench_memory[2499]=(u8)116;dong_porf_js_microbench_memory[2500]=(u8)101;dong_porf_js_microbench_memory[2501]=(u8)100;dong_porf_js_microbench_memory[2502]=(u8)32;dong_porf_js_microbench_memory[2503]=(u8)44;dong_porf_js_microbench_memory[2504]=(u8)32;dong_porf_js_microbench_memory[2505]=(u8)111;dong_porf_js_microbench_memory[2506]=(u8)114;dong_porf_js_microbench_memory[2507]=(u8)32;dong_porf_js_microbench_memory[2508]=(u8)125;
  dong_porf_js_microbench_memory[2511]=(u8)13;dong_porf_js_microbench_memory[2515]=(u8)115;dong_porf_js_microbench_memory[2516]=(u8)116;dong_porf_js_microbench_memory[2517]=(u8)114;dong_porf_js_microbench_memory[2518]=(u8)105;dong_porf_js_microbench_memory[2519]=(u8)110;dong_porf_js_microbench_memory[2520]=(u8)103;dong_porf_js_microbench_memory[2521]=(u8)95;dong_porf_js_microbench_memory[2522]=(u8)99;dong_porf_js_microbench_memory[2523]=(u8)111;dong_porf_js_microbench_memory[2524]=(u8)110;dong_porf_js_microbench_memory[2525]=(u8)99;dong_porf_js_microbench_memory[2526]=(u8)97;dong_porf_js_microbench_memory[2527]=(u8)116;
  dong_porf_js_microbench_memory[2530]=(u8)12;dong_porf_js_microbench_memory[2534]=(u8)91;dong_porf_js_microbench_memory[2535]=(u8)66;dong_porf_js_microbench_memory[2536]=(u8)69;dong_porf_js_microbench_memory[2537]=(u8)78;dong_porf_js_microbench_memory[2538]=(u8)67;dong_porf_js_microbench_memory[2539]=(u8)72;dong_porf_js_microbench_memory[2540]=(u8)93;dong_porf_js_microbench_memory[2541]=(u8)32;dong_porf_js_microbench_memory[2542]=(u8)68;dong_porf_js_microbench_memory[2543]=(u8)79;dong_porf_js_microbench_memory[2544]=(u8)78;dong_porf_js_microbench_memory[2545]=(u8)69;
  dong_porf_js_microbench_memory[2548]=(u8)58;dong_porf_js_microbench_memory[2552]=(u8)83;dong_porf_js_microbench_memory[2553]=(u8)116;dong_porf_js_microbench_memory[2554]=(u8)114;dong_porf_js_microbench_memory[2555]=(u8)105;dong_porf_js_microbench_memory[2556]=(u8)110;dong_porf_js_microbench_memory[2557]=(u8)103;dong_porf_js_microbench_memory[2558]=(u8)46;dong_porf_js_microbench_memory[2559]=(u8)112;dong_porf_js_microbench_memory[2560]=(u8)114;dong_porf_js_microbench_memory[2561]=(u8)111;dong_porf_js_microbench_memory[2562]=(u8)116;dong_porf_js_microbench_memory[2563]=(u8)111;dong_porf_js_microbench_memory[2564]=(u8)116;dong_porf_js_microbench_memory[2565]=(u8)121;dong_porf_js_microbench_memory[2566]=(u8)112;dong_porf_js_microbench_memory[2567]=(u8)101;dong_porf_js_microbench_memory[2568]=(u8)46;dong_porf_js_microbench_memory[2569]=(u8)116;dong_porf_js_microbench_memory[2570]=(u8)111;dong_porf_js_microbench_memory[2571]=(u8)83;dong_porf_js_microbench_memory[2572]=(u8)116;dong_porf_js_microbench_memory[2573]=(u8)114;dong_porf_js_microbench_memory[2574]=(u8)105;dong_porf_js_microbench_memory[2575]=(u8)110;dong_porf_js_microbench_memory[2576]=(u8)103;dong_porf_js_microbench_memory[2577]=(u8)32;dong_porf_js_microbench_memory[2578]=(u8)101;dong_porf_js_microbench_memory[2579]=(u8)120;dong_porf_js_microbench_memory[2580]=(u8)112;dong_porf_js_microbench_memory[2581]=(u8)101;dong_porf_js_microbench_memory[2582]=(u8)99;dong_porf_js_microbench_memory[2583]=(u8)116;dong_porf_js_microbench_memory[2584]=(u8)115;dong_porf_js_microbench_memory[2585]=(u8)32;dong_porf_js_microbench_memory[2586]=(u8)39;dong_porf_js_microbench_memory[2587]=(u8)116;dong_porf_js_microbench_memory[2588]=(u8)104;dong_porf_js_microbench_memory[2589]=(u8)105;dong_porf_js_microbench_memory[2590]=(u8)115;dong_porf_js_microbench_memory[2591]=(u8)39;dong_porf_js_microbench_memory[2592]=(u8)32;dong_porf_js_microbench_memory[2593]=(u8)116;dong_porf_js_microbench_memory[2594]=(u8)111;dong_porf_js_microbench_memory[2595]=(u8)32;dong_porf_js_microbench_memory[2596]=(u8)98;dong_porf_js_microbench_memory[2597]=(u8)101;dong_porf_js_microbench_memory[2598]=(u8)32;dong_porf_js_microbench_memory[2599]=(u8)110;dong_porf_js_microbench_memory[2600]=(u8)111;dong_porf_js_microbench_memory[2601]=(u8)110;dong_porf_js_microbench_memory[2602]=(u8)45;dong_porf_js_microbench_memory[2603]=(u8)110;dong_porf_js_microbench_memory[2604]=(u8)117;dong_porf_js_microbench_memory[2605]=(u8)108;dong_porf_js_microbench_memory[2606]=(u8)108;dong_porf_js_microbench_memory[2607]=(u8)105;dong_porf_js_microbench_memory[2608]=(u8)115;dong_porf_js_microbench_memory[2609]=(u8)104;
  dong_porf_js_microbench_memory[2612]=(u8)11;dong_porf_js_microbench_memory[2616]=(u8)83;dong_porf_js_microbench_memory[2617]=(u8)121;dong_porf_js_microbench_memory[2618]=(u8)110;dong_porf_js_microbench_memory[2619]=(u8)116;dong_porf_js_microbench_memory[2620]=(u8)97;dong_porf_js_microbench_memory[2621]=(u8)120;dong_porf_js_microbench_memory[2622]=(u8)69;dong_porf_js_microbench_memory[2623]=(u8)114;dong_porf_js_microbench_memory[2624]=(u8)114;dong_porf_js_microbench_memory[2625]=(u8)111;dong_porf_js_microbench_memory[2626]=(u8)114;
  dong_porf_js_microbench_memory[2629]=(u8)10;dong_porf_js_microbench_memory[2633]=(u8)82;dong_porf_js_microbench_memory[2634]=(u8)97;dong_porf_js_microbench_memory[2635]=(u8)110;dong_porf_js_microbench_memory[2636]=(u8)103;dong_porf_js_microbench_memory[2637]=(u8)101;dong_porf_js_microbench_memory[2638]=(u8)69;dong_porf_js_microbench_memory[2639]=(u8)114;dong_porf_js_microbench_memory[2640]=(u8)114;dong_porf_js_microbench_memory[2641]=(u8)111;dong_porf_js_microbench_memory[2642]=(u8)114;
  dong_porf_js_microbench_memory[2645]=(u8)54;dong_porf_js_microbench_memory[2649]=(u8)83;dong_porf_js_microbench_memory[2650]=(u8)116;dong_porf_js_microbench_memory[2651]=(u8)114;dong_porf_js_microbench_memory[2652]=(u8)105;dong_porf_js_microbench_memory[2653]=(u8)110;dong_porf_js_microbench_memory[2654]=(u8)103;dong_porf_js_microbench_memory[2655]=(u8)46;dong_porf_js_microbench_memory[2656]=(u8)112;dong_porf_js_microbench_memory[2657]=(u8)114;dong_porf_js_microbench_memory[2658]=(u8)111;dong_porf_js_microbench_memory[2659]=(u8)116;dong_porf_js_microbench_memory[2660]=(u8)111;dong_porf_js_microbench_memory[2661]=(u8)116;dong_porf_js_microbench_memory[2662]=(u8)121;dong_porf_js_microbench_memory[2663]=(u8)112;dong_porf_js_microbench_memory[2664]=(u8)101;dong_porf_js_microbench_memory[2665]=(u8)46;dong_porf_js_microbench_memory[2666]=(u8)116;dong_porf_js_microbench_memory[2667]=(u8)114;dong_porf_js_microbench_memory[2668]=(u8)105;dong_porf_js_microbench_memory[2669]=(u8)109;dong_porf_js_microbench_memory[2670]=(u8)32;dong_porf_js_microbench_memory[2671]=(u8)101;dong_porf_js_microbench_memory[2672]=(u8)120;dong_porf_js_microbench_memory[2673]=(u8)112;dong_porf_js_microbench_memory[2674]=(u8)101;dong_porf_js_microbench_memory[2675]=(u8)99;dong_porf_js_microbench_memory[2676]=(u8)116;dong_porf_js_microbench_memory[2677]=(u8)115;dong_porf_js_microbench_memory[2678]=(u8)32;dong_porf_js_microbench_memory[2679]=(u8)39;dong_porf_js_microbench_memory[2680]=(u8)116;dong_porf_js_microbench_memory[2681]=(u8)104;dong_porf_js_microbench_memory[2682]=(u8)105;dong_porf_js_microbench_memory[2683]=(u8)115;dong_porf_js_microbench_memory[2684]=(u8)39;dong_porf_js_microbench_memory[2685]=(u8)32;dong_porf_js_microbench_memory[2686]=(u8)116;dong_porf_js_microbench_memory[2687]=(u8)111;dong_porf_js_microbench_memory[2688]=(u8)32;dong_porf_js_microbench_memory[2689]=(u8)98;dong_porf_js_microbench_memory[2690]=(u8)101;dong_porf_js_microbench_memory[2691]=(u8)32;dong_porf_js_microbench_memory[2692]=(u8)110;dong_porf_js_microbench_memory[2693]=(u8)111;dong_porf_js_microbench_memory[2694]=(u8)110;dong_porf_js_microbench_memory[2695]=(u8)45;dong_porf_js_microbench_memory[2696]=(u8)110;dong_porf_js_microbench_memory[2697]=(u8)117;dong_porf_js_microbench_memory[2698]=(u8)108;dong_porf_js_microbench_memory[2699]=(u8)108;dong_porf_js_microbench_memory[2700]=(u8)105;dong_porf_js_microbench_memory[2701]=(u8)115;dong_porf_js_microbench_memory[2702]=(u8)104;
  dong_porf_js_microbench_memory[2705]=(u8)57;dong_porf_js_microbench_memory[2709]=(u8)83;dong_porf_js_microbench_memory[2710]=(u8)116;dong_porf_js_microbench_memory[2711]=(u8)114;dong_porf_js_microbench_memory[2712]=(u8)105;dong_porf_js_microbench_memory[2713]=(u8)110;dong_porf_js_microbench_memory[2714]=(u8)103;dong_porf_js_microbench_memory[2715]=(u8)46;dong_porf_js_microbench_memory[2716]=(u8)112;dong_porf_js_microbench_memory[2717]=(u8)114;dong_porf_js_microbench_memory[2718]=(u8)111;dong_porf_js_microbench_memory[2719]=(u8)116;dong_porf_js_microbench_memory[2720]=(u8)111;dong_porf_js_microbench_memory[2721]=(u8)116;dong_porf_js_microbench_memory[2722]=(u8)121;dong_porf_js_microbench_memory[2723]=(u8)112;dong_porf_js_microbench_memory[2724]=(u8)101;dong_porf_js_microbench_memory[2725]=(u8)46;dong_porf_js_microbench_memory[2726]=(u8)116;dong_porf_js_microbench_memory[2727]=(u8)114;dong_porf_js_microbench_memory[2728]=(u8)105;dong_porf_js_microbench_memory[2729]=(u8)109;dong_porf_js_microbench_memory[2730]=(u8)69;dong_porf_js_microbench_memory[2731]=(u8)110;dong_porf_js_microbench_memory[2732]=(u8)100;dong_porf_js_microbench_memory[2733]=(u8)32;dong_porf_js_microbench_memory[2734]=(u8)101;dong_porf_js_microbench_memory[2735]=(u8)120;dong_porf_js_microbench_memory[2736]=(u8)112;dong_porf_js_microbench_memory[2737]=(u8)101;dong_porf_js_microbench_memory[2738]=(u8)99;dong_porf_js_microbench_memory[2739]=(u8)116;dong_porf_js_microbench_memory[2740]=(u8)115;dong_porf_js_microbench_memory[2741]=(u8)32;dong_porf_js_microbench_memory[2742]=(u8)39;dong_porf_js_microbench_memory[2743]=(u8)116;dong_porf_js_microbench_memory[2744]=(u8)104;dong_porf_js_microbench_memory[2745]=(u8)105;dong_porf_js_microbench_memory[2746]=(u8)115;dong_porf_js_microbench_memory[2747]=(u8)39;dong_porf_js_microbench_memory[2748]=(u8)32;dong_porf_js_microbench_memory[2749]=(u8)116;dong_porf_js_microbench_memory[2750]=(u8)111;dong_porf_js_microbench_memory[2751]=(u8)32;dong_porf_js_microbench_memory[2752]=(u8)98;dong_porf_js_microbench_memory[2753]=(u8)101;dong_porf_js_microbench_memory[2754]=(u8)32;dong_porf_js_microbench_memory[2755]=(u8)110;dong_porf_js_microbench_memory[2756]=(u8)111;dong_porf_js_microbench_memory[2757]=(u8)110;dong_porf_js_microbench_memory[2758]=(u8)45;dong_porf_js_microbench_memory[2759]=(u8)110;dong_porf_js_microbench_memory[2760]=(u8)117;dong_porf_js_microbench_memory[2761]=(u8)108;dong_porf_js_microbench_memory[2762]=(u8)108;dong_porf_js_microbench_memory[2763]=(u8)105;dong_porf_js_microbench_memory[2764]=(u8)115;dong_porf_js_microbench_memory[2765]=(u8)104;
  dong_porf_js_microbench_memory[2768]=(u8)59;dong_porf_js_microbench_memory[2772]=(u8)83;dong_porf_js_microbench_memory[2773]=(u8)116;dong_porf_js_microbench_memory[2774]=(u8)114;dong_porf_js_microbench_memory[2775]=(u8)105;dong_porf_js_microbench_memory[2776]=(u8)110;dong_porf_js_microbench_memory[2777]=(u8)103;dong_porf_js_microbench_memory[2778]=(u8)46;dong_porf_js_microbench_memory[2779]=(u8)112;dong_porf_js_microbench_memory[2780]=(u8)114;dong_porf_js_microbench_memory[2781]=(u8)111;dong_porf_js_microbench_memory[2782]=(u8)116;dong_porf_js_microbench_memory[2783]=(u8)111;dong_porf_js_microbench_memory[2784]=(u8)116;dong_porf_js_microbench_memory[2785]=(u8)121;dong_porf_js_microbench_memory[2786]=(u8)112;dong_porf_js_microbench_memory[2787]=(u8)101;dong_porf_js_microbench_memory[2788]=(u8)46;dong_porf_js_microbench_memory[2789]=(u8)116;dong_porf_js_microbench_memory[2790]=(u8)114;dong_porf_js_microbench_memory[2791]=(u8)105;dong_porf_js_microbench_memory[2792]=(u8)109;dong_porf_js_microbench_memory[2793]=(u8)83;dong_porf_js_microbench_memory[2794]=(u8)116;dong_porf_js_microbench_memory[2795]=(u8)97;dong_porf_js_microbench_memory[2796]=(u8)114;dong_porf_js_microbench_memory[2797]=(u8)116;dong_porf_js_microbench_memory[2798]=(u8)32;dong_porf_js_microbench_memory[2799]=(u8)101;dong_porf_js_microbench_memory[2800]=(u8)120;dong_porf_js_microbench_memory[2801]=(u8)112;dong_porf_js_microbench_memory[2802]=(u8)101;dong_porf_js_microbench_memory[2803]=(u8)99;dong_porf_js_microbench_memory[2804]=(u8)116;dong_porf_js_microbench_memory[2805]=(u8)115;dong_porf_js_microbench_memory[2806]=(u8)32;dong_porf_js_microbench_memory[2807]=(u8)39;dong_porf_js_microbench_memory[2808]=(u8)116;dong_porf_js_microbench_memory[2809]=(u8)104;dong_porf_js_microbench_memory[2810]=(u8)105;dong_porf_js_microbench_memory[2811]=(u8)115;dong_porf_js_microbench_memory[2812]=(u8)39;dong_porf_js_microbench_memory[2813]=(u8)32;dong_porf_js_microbench_memory[2814]=(u8)116;dong_porf_js_microbench_memory[2815]=(u8)111;dong_porf_js_microbench_memory[2816]=(u8)32;dong_porf_js_microbench_memory[2817]=(u8)98;dong_porf_js_microbench_memory[2818]=(u8)101;dong_porf_js_microbench_memory[2819]=(u8)32;dong_porf_js_microbench_memory[2820]=(u8)110;dong_porf_js_microbench_memory[2821]=(u8)111;dong_porf_js_microbench_memory[2822]=(u8)110;dong_porf_js_microbench_memory[2823]=(u8)45;dong_porf_js_microbench_memory[2824]=(u8)110;dong_porf_js_microbench_memory[2825]=(u8)117;dong_porf_js_microbench_memory[2826]=(u8)108;dong_porf_js_microbench_memory[2827]=(u8)108;dong_porf_js_microbench_memory[2828]=(u8)105;dong_porf_js_microbench_memory[2829]=(u8)115;dong_porf_js_microbench_memory[2830]=(u8)104;
  dong_porf_js_microbench_memory[16501]=(u8)7;dong_porf_js_microbench_memory[16505]=(u8)118;dong_porf_js_microbench_memory[16506]=(u8)97;dong_porf_js_microbench_memory[16507]=(u8)108;dong_porf_js_microbench_memory[16508]=(u8)117;dong_porf_js_microbench_memory[16509]=(u8)101;dong_porf_js_microbench_memory[16510]=(u8)79;dong_porf_js_microbench_memory[16511]=(u8)102;dong_porf_js_microbench_memory[16555]=(u8)1;dong_porf_js_microbench_memory[16558]=(u8)14;dong_porf_js_microbench_memory[16562]=(u8)104;dong_porf_js_microbench_memory[16563]=(u8)97;dong_porf_js_microbench_memory[16564]=(u8)115;dong_porf_js_microbench_memory[16565]=(u8)79;dong_porf_js_microbench_memory[16566]=(u8)119;dong_porf_js_microbench_memory[16567]=(u8)110;dong_porf_js_microbench_memory[16568]=(u8)80;dong_porf_js_microbench_memory[16569]=(u8)114;dong_porf_js_microbench_memory[16570]=(u8)111;dong_porf_js_microbench_memory[16571]=(u8)112;dong_porf_js_microbench_memory[16572]=(u8)101;dong_porf_js_microbench_memory[16573]=(u8)114;dong_porf_js_microbench_memory[16574]=(u8)116;dong_porf_js_microbench_memory[16575]=(u8)121;dong_porf_js_microbench_memory[16612]=(u8)1;dong_porf_js_microbench_memory[16615]=(u8)20;dong_porf_js_microbench_memory[16619]=(u8)112;dong_porf_js_microbench_memory[16620]=(u8)114;dong_porf_js_microbench_memory[16621]=(u8)111;dong_porf_js_microbench_memory[16622]=(u8)112;dong_porf_js_microbench_memory[16623]=(u8)101;dong_porf_js_microbench_memory[16624]=(u8)114;dong_porf_js_microbench_memory[16625]=(u8)116;dong_porf_js_microbench_memory[16626]=(u8)121;dong_porf_js_microbench_memory[16627]=(u8)73;dong_porf_js_microbench_memory[16628]=(u8)115;dong_porf_js_microbench_memory[16629]=(u8)69;dong_porf_js_microbench_memory[16630]=(u8)110;dong_porf_js_microbench_memory[16631]=(u8)117;dong_porf_js_microbench_memory[16632]=(u8)109;dong_porf_js_microbench_memory[16633]=(u8)101;dong_porf_js_microbench_memory[16634]=(u8)114;dong_porf_js_microbench_memory[16635]=(u8)97;dong_porf_js_microbench_memory[16636]=(u8)98;dong_porf_js_microbench_memory[16637]=(u8)108;dong_porf_js_microbench_memory[16638]=(u8)101;dong_porf_js_microbench_memory[16669]=(u8)1;dong_porf_js_microbench_memory[16672]=(u8)13;dong_porf_js_microbench_memory[16676]=(u8)105;dong_porf_js_microbench_memory[16677]=(u8)115;dong_porf_js_microbench_memory[16678]=(u8)80;dong_porf_js_microbench_memory[16679]=(u8)114;dong_porf_js_microbench_memory[16680]=(u8)111;dong_porf_js_microbench_memory[16681]=(u8)116;dong_porf_js_microbench_memory[16682]=(u8)111;dong_porf_js_microbench_memory[16683]=(u8)116;dong_porf_js_microbench_memory[16684]=(u8)121;dong_porf_js_microbench_memory[16685]=(u8)112;dong_porf_js_microbench_memory[16686]=(u8)101;dong_porf_js_microbench_memory[16687]=(u8)79;dong_porf_js_microbench_memory[16688]=(u8)102;dong_porf_js_microbench_memory[16729]=(u8)8;dong_porf_js_microbench_memory[16733]=(u8)116;dong_porf_js_microbench_memory[16734]=(u8)111;dong_porf_js_microbench_memory[16735]=(u8)83;dong_porf_js_microbench_memory[16736]=(u8)116;dong_porf_js_microbench_memory[16737]=(u8)114;dong_porf_js_microbench_memory[16738]=(u8)105;dong_porf_js_microbench_memory[16739]=(u8)110;dong_porf_js_microbench_memory[16740]=(u8)103;dong_porf_js_microbench_memory[16786]=(u8)14;dong_porf_js_microbench_memory[16790]=(u8)116;dong_porf_js_microbench_memory[16791]=(u8)111;dong_porf_js_microbench_memory[16792]=(u8)76;dong_porf_js_microbench_memory[16793]=(u8)111;dong_porf_js_microbench_memory[16794]=(u8)99;dong_porf_js_microbench_memory[16795]=(u8)97;dong_porf_js_microbench_memory[16796]=(u8)108;dong_porf_js_microbench_memory[16797]=(u8)101;dong_porf_js_microbench_memory[16798]=(u8)83;dong_porf_js_microbench_memory[16799]=(u8)116;dong_porf_js_microbench_memory[16800]=(u8)114;dong_porf_js_microbench_memory[16801]=(u8)105;dong_porf_js_microbench_memory[16802]=(u8)110;dong_porf_js_microbench_memory[16803]=(u8)103;dong_porf_js_microbench_memory[16840]=(u8)1;dong_porf_js_microbench_memory[16842]=(u8)2;dong_porf_js_microbench_memory[16843]=(u8)6;dong_porf_js_microbench_memory[16847]=(u8)79;dong_porf_js_microbench_memory[16848]=(u8)98;dong_porf_js_microbench_memory[16849]=(u8)106;dong_porf_js_microbench_memory[16850]=(u8)101;dong_porf_js_microbench_memory[16851]=(u8)99;dong_porf_js_microbench_memory[16852]=(u8)116;
}

static struct ReturnValue dong_porf_js_microbench_utf8AppendCodePoint(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 out, i32 outjjtype, f64 cp, i32 cpjjtype);
static struct ReturnValue dong_porf_js_microbench_toUtf8(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype);
static struct ReturnValue dong_porf_js_microbench_dongLog(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 msg, i32 msgjjtype);
static struct ReturnValue dong_porf_js_microbench_benchLog(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 msg, i32 msgjjtype);
static struct ReturnValue dong_porf_js_microbench_bench_finish(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static struct ReturnValue dong_porf_js_microbench_noop(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype);
static i32 dong_porf_js_microbench__Porffor_malloc(i32 l0);
static void dong_porf_js_microbench__Porffor_object_expr_init(i32 obj, i32 objjjtype, i32 key, i32 keyjjtype, f64 value, i32 valuejjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_object_underlying(f64 _obj, i32 _objjjtype);
static i32 dong_porf_js_microbench__Porffor_funcLut_length(i32 l0);
static void dong_porf_js_microbench__Porffor_object_fastAdd(i32 obj, i32 objjjtype, i32 key, i32 keyjjtype, f64 value, i32 valuejjtype, i32 flags, i32 flagsjjtype);
static i32 dong_porf_js_microbench__Porffor_object_hash(i32 key, i32 keyjjtype);
static void dong_porf_js_microbench__Porffor_object_writeKey(i32 ptr, i32 ptrjjtype, i32 key, i32 keyjjtype, i32 hash, i32 hashjjtype);
static i32 dong_porf_js_microbench__Porffor_funcLut_name(i32 l0);
static f64 dong_porf_js_microbench__ecma262_IsConstructor(f64 argument, i32 argumentjjtype);
static i32 dong_porf_js_microbench__Porffor_funcLut_flags(i32 l0);
static struct ReturnValue dong_porf_js_microbench__Number_prototype_toString(f64 _this, i32 _thisjjtype, f64 radix, i32 radixjjtype);
static f64 dong_porf_js_microbench_TypeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype);
static struct ReturnValue dong_porf_js_microbench__ecma262_ToString(f64 argument, i32 argumentjjtype);
static struct ReturnValue dong_porf_js_microbench__ecma262_ToPrimitive_String(f64 input, i32 inputjjtype);
static struct ReturnValue dong_porf_js_microbench__Boolean_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__Function_prototype_toString(f64 _this, i32 _thisjjtype);
static f64 dong_porf_js_microbench__Porffor_bytestring_appendStr(f64 str, i32 strjjtype, f64 appendage, i32 appendagejjtype);
static struct ReturnValue dong_porf_js_microbench__TypeError_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__TypeError_prototype_namekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__TypeError_prototype_messagekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_concatStrings(f64 a, i32 ajjtype, f64 b, i32 bjjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_strcat(i32 a, i32 ajjtype, i32 b, i32 bjjtype);
static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_toString(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__Object_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_object_get_withHash(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype, i32 hash, i32 hashjjtype);
static i32 dong_porf_js_microbench__Porffor_object_lookup(i32 obj, i32 objjjtype, i32 target, i32 targetjjtype, i32 targetHash, i32 targetHashjjtype);
static i32 dong_porf_js_microbench_jjget___Object_prototype();
static struct ReturnValue dong_porf_js_microbench__ecma262_ToPropertyKey(f64 argument, i32 argumentjjtype);
static f64 dong_porf_js_microbench__Array_from(f64 arg, i32 argjjtype, f64 mapFn, i32 mapFnjjtype, f64 thisArg, i32 thisArgjjtype);
static i32 dong_porf_js_microbench__Porffor_object_isObject(i32 arg, i32 argjjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_object_get(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_object_getHiddenPrototype(i32 trueType, i32 trueTypejjtype);
static i32 dong_porf_js_microbench__Porffor_strcmp(i32 a, i32 ajjtype, i32 b, i32 bjjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_object_getPrototype(i32 obj, i32 objjjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_object_accessorGet(i32 entryPtr, i32 entryPtrjjtype);
static f64 dong_porf_js_microbench__ecma262_ToIntegerOrInfinity(f64 argument, i32 argumentjjtype);
static f64 dong_porf_js_microbench__ecma262_ToNumber(f64 argument, i32 argumentjjtype);
static f64 dong_porf_js_microbench__ecma262_StringToNumber(f64 str, i32 strjjtype);
static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_trim(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_trimEnd(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_trimStart(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_charCodeAt(f64 _this, i32 _thisjjtype, f64 index, i32 indexjjtype);
static f64 dong_porf_js_microbench__Math_trunc(f64 l0);
static f64 dong_porf_js_microbench__Porffor_stn_int(f64 str, i32 strjjtype, f64 radix, i32 radixjjtype, f64 i, i32 ijjtype);
static f64 dong_porf_js_microbench__Porffor_stn_float(f64 str, i32 strjjtype, f64 i, i32 ijjtype);
static f64 dong_porf_js_microbench__Porffor_parseExp(f64 str, i32 strjjtype, f64 i, i32 ijjtype, f64 len, i32 lenjjtype, f64 strict, i32 strictjjtype);
static f64 dong_porf_js_microbench__Number_isNaN(f64 l0);
static f64 dong_porf_js_microbench__Math_pow(f64 base, i32 basejjtype, f64 exponent, i32 exponentjjtype);
static f64 dong_porf_js_microbench__Number_isInteger(f64 l0);
static f64 dong_porf_js_microbench__Number_isFinite(f64 l0);
static f64 dong_porf_js_microbench__Math_abs(f64 l0);
static f64 dong_porf_js_microbench__Math_exp(f64 x, i32 xjjtype);
static f64 dong_porf_js_microbench__Math_floor(f64 l0);
static f64 dong_porf_js_microbench__Math_log(f64 y, i32 yjjtype);
static f64 dong_porf_js_microbench__Math_log2(f64 y, i32 yjjtype);
static struct ReturnValue dong_porf_js_microbench__ecma262_ToPrimitive_Number(f64 input, i32 inputjjtype);
static struct ReturnValue dong_porf_js_microbench__Number_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__Boolean_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__Array_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_valueOf(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__Object_prototype_valueOf(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_object_readValue(i32 entryPtr, i32 entryPtrjjtype);
static i32 dong_porf_js_microbench__Porffor_object_isObjectOrNull(i32 arg, i32 argjjtype);
static struct ReturnValue dong_porf_js_microbench__Array_prototype_toString(f64 _this, i32 _thisjjtype);
static f64 dong_porf_js_microbench__Porffor_bytestring_appendChar(f64 str, i32 strjjtype, f64 _char, i32 charjjtype);
static f64 dong_porf_js_microbench_RangeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype);
static void dong_porf_js_microbench__Porffor_object_setPrototype(i32 obj, i32 objjjtype, i32 proto, i32 protojjtype);
static i32 dong_porf_js_microbench__Porffor_bytestringToString(i32 src);
static struct ReturnValue dong_porf_js_microbench__String_prototype_valueOf(i32 _this, i32 _thisjjtype);
static f64 dong_porf_js_microbench__Math_round(f64 l0);
static struct ReturnValue dong_porf_js_microbench__Number_prototype_toFixed(f64 _this, i32 _thisjjtype, f64 fractionDigits, i32 fractionDigitsjjtype);
static struct ReturnValue dong_porf_js_microbench__String_prototype_charCodeAt(f64 _this, i32 _thisjjtype, f64 index, i32 indexjjtype);
static struct ReturnValue dong_porf_js_microbench__String_fromCharCode(f64 codes, i32 codesjjtype);
static struct ReturnValue dong_porf_js_microbench__JSON_parse(f64 _, i32 _jjtype);
static struct ReturnValue dong_porf_js_microbench_parseValue();
static struct ReturnValue dong_porf_js_microbench_skipWhitespace();
static f64 dong_porf_js_microbench_SyntaxError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype);
static f64 dong_porf_js_microbench__Porffor_array_fastPush(f64 arr, i32 arrjjtype, f64 el, i32 eljjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_object_set(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype, f64 value, i32 valuejjtype);
static struct ReturnValue dong_porf_js_microbench__Porffor_object_accessorSet(i32 entryPtr, i32 entryPtrjjtype);
static i32 dong_porf_js_microbench__Porffor_object_isInextensible(i32 obj, i32 objjjtype);
static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_slice(i32 _this, i32 _thisjjtype, i32 start, i32 startjjtype, i32 end, i32 endjjtype);
static struct ReturnValue dong_porf_js_microbench__String_prototype_toString(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__SyntaxError_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__SyntaxError_prototype_namekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__SyntaxError_prototype_messagekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__RangeError_prototype_toString(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__RangeError_prototype_namekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__RangeError_prototype_messagekkget(f64 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__String_prototype_trim(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__String_prototype_trimEnd(i32 _this, i32 _thisjjtype);
static struct ReturnValue dong_porf_js_microbench__String_prototype_trimStart(i32 _this, i32 _thisjjtype);

static f64 dong_porf_js_microbench_METRIC_OFFSET_WIDTH = 0;
static i32 dong_porf_js_microbench_METRIC_OFFSET_WIDTHjjtype = 0;
static f64 dong_porf_js_microbench_METRIC_OFFSET_HEIGHT = 0;
static i32 dong_porf_js_microbench_METRIC_OFFSET_HEIGHTjjtype = 0;
static f64 dong_porf_js_microbench_METRIC_OFFSET_TOP = 0;
static i32 dong_porf_js_microbench_METRIC_OFFSET_TOPjjtype = 0;
static f64 dong_porf_js_microbench_METRIC_OFFSET_LEFT = 0;
static i32 dong_porf_js_microbench_METRIC_OFFSET_LEFTjjtype = 0;
static f64 dong_porf_js_microbench_METRIC_CLIENT_WIDTH = 0;
static i32 dong_porf_js_microbench_METRIC_CLIENT_WIDTHjjtype = 0;
static f64 dong_porf_js_microbench_METRIC_CLIENT_HEIGHT = 0;
static i32 dong_porf_js_microbench_METRIC_CLIENT_HEIGHTjjtype = 0;
static f64 dong_porf_js_microbench_METRIC_SCROLL_WIDTH = 0;
static i32 dong_porf_js_microbench_METRIC_SCROLL_WIDTHjjtype = 0;
static f64 dong_porf_js_microbench_METRIC_SCROLL_HEIGHT = 0;
static i32 dong_porf_js_microbench_METRIC_SCROLL_HEIGHTjjtype = 0;
static f64 dong_porf_js_microbench_bench_name = 0;
static i32 dong_porf_js_microbench_bench_namejjtype = 0;
static f64 dong_porf_js_microbench_bench_iterations = 0;
static i32 dong_porf_js_microbench_bench_iterationsjjtype = 0;
static f64 dong_porf_js_microbench_bench_start = 0;
static i32 dong_porf_js_microbench_bench_startjjtype = 0;
static f64 dong_porf_js_microbench_i = 0;
static i32 dong_porf_js_microbench_ijjtype = 0;
static f64 dong_porf_js_microbench_obj_x = 0;
static i32 dong_porf_js_microbench_obj_xjjtype = 0;
static f64 dong_porf_js_microbench_obj = 0;
static i32 dong_porf_js_microbench_objjjtype = 0;
static i32 dong_porf_js_microbench_jjporfjjcurrentPtr = 0;
static i32 dong_porf_js_microbench_jjporfjjcurrentPtrjjglbl_inited = 0;
static i32 dong_porf_js_microbench_jjporfjjendPtr = 0;
static i32 dong_porf_js_microbench_jjporfjjendPtrjjglbl_inited = 0;
static i32 dong_porf_js_microbench_jjporfjjunderlyingStore = 0;
static i32 dong_porf_js_microbench_jjporfjjunderlyingStorejjglbl_inited = 0;
static i32 dong_porf_js_microbench_jjporfjjgetptr___Object_prototype = 0;
static i32 dong_porf_js_microbench_jjporfjjgetptr___Object_prototypejjglbl_inited = 0;
static f64 dong_porf_js_microbench_o = 0;
static i32 dong_porf_js_microbench_ojjtype = 0;
static f64 dong_porf_js_microbench_jsonStr = 0;
static i32 dong_porf_js_microbench_jsonStrjjtype = 0;
static f64 dong_porf_js_microbench_parsed = 0;
static i32 dong_porf_js_microbench_parsedjjtype = 0;
static f64 dong_porf_js_microbench_jjporfjjtext = 0;
static i32 dong_porf_js_microbench_jjporfjjtextjjglbl_inited = 0;
static f64 dong_porf_js_microbench_jjporfjjpos = 0;
static i32 dong_porf_js_microbench_jjporfjjposjjglbl_inited = 0;
static f64 dong_porf_js_microbench_jjporfjjlen = 0;
static i32 dong_porf_js_microbench_jjporfjjlenjjglbl_inited = 0;
static f64 dong_porf_js_microbench_s = 0;
static i32 dong_porf_js_microbench_sjjtype = 0;
static f64 dong_porf_js_microbench_j = 0;
static i32 dong_porf_js_microbench_jjjtype = 0;

static i32 dong_porf_js_microbench__Porffor_malloc(i32 l0) {
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get0;
  _get0 = l0;
  // if i32
  i32 _r2;
    if ((dong_porf_js_microbench_jjporfjjcurrentPtr + _get0) >= dong_porf_js_microbench_jjporfjjendPtr) {
      const u32 _oldPages1 = dong_porf_js_microbench_memory_pages;
      dong_porf_js_microbench_memory_pages += 16;
      dong_porf_js_microbench_memory = realloc(dong_porf_js_microbench_memory, dong_porf_js_microbench_memory_pages * 65536);
      memset(dong_porf_js_microbench_memory + _oldPages1 * 65536, 0, (dong_porf_js_microbench_memory_pages - _oldPages1) * 65536);
      _get2 = l0;
      dong_porf_js_microbench_jjporfjjcurrentPtr = (_oldPages1 * 65536) + _get2;
      _get3 = l0;
      dong_porf_js_microbench_jjporfjjendPtr = (dong_porf_js_microbench_jjporfjjcurrentPtr + 1048576) - _get3;
      _get4 = l0;
      _r2 = dong_porf_js_microbench_jjporfjjcurrentPtr - _get4;
    } else {
      _get5 = l0;
      dong_porf_js_microbench_jjporfjjcurrentPtr = dong_porf_js_microbench_jjporfjjcurrentPtr + _get5;
      _r2 = dong_porf_js_microbench_jjporfjjcurrentPtr;
    }
  // end
  j2:;
  return _r2;
}

static i32 dong_porf_js_microbench__Porffor_funcLut_length(i32 l0) {
  i32 _get0;
  _get0 = l0;
  return i32_load16_u(0, 16384, _get0 * 57);
}

static i32 dong_porf_js_microbench__Porffor_object_hash(i32 key, i32 keyjjtype) {
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
  j12:;
  _get4 = len;
  hash = 374761393 + _get4;
  _get5 = p;
  _get6 = len;
  end = _get5 + _get6;
  // loop 
  j13:;
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
        goto j13;
      }
    // end
    j14:;
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

static void dong_porf_js_microbench__Porffor_object_writeKey(i32 ptr, i32 ptrjjtype, i32 key, i32 keyjjtype, i32 hash, i32 hashjjtype) {
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
  j15:;
  _get5 = ptr;
  _get6 = keyEnc;
  i32_store(0, 4, _get5, _get6);
  return;
}

static void dong_porf_js_microbench__Porffor_object_fastAdd(i32 obj, i32 objjjtype, i32 key, i32 keyjjtype, f64 value, i32 valuejjtype, i32 flags, i32 flagsjjtype) {
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
  dong_porf_js_microbench__Porffor_object_writeKey(_get5, 1, _get6, _get7, dong_porf_js_microbench__Porffor_object_hash(_get8, _get9), 1);
  _get10 = entryPtr;
  _get11 = value;
  f64_store(0, 8, _get10, _get11);
  _get12 = entryPtr;
  _get13 = flags;
  _get14 = valuejjtype;
  i32_store16(0, 16, _get12, _get13 + (_get14 << 8));
  return;
}

static i32 dong_porf_js_microbench__Porffor_funcLut_name(i32 l0) {
  i32 _get0;
  _get0 = l0;
  return (((_get0 * 57) + 3) + 16384);
}

static i32 dong_porf_js_microbench__Porffor_funcLut_flags(i32 l0) {
  i32 _get0;
  _get0 = l0;
  return i32_load8_u(0, 16384, (_get0 * 57) + 2);
}

static f64 dong_porf_js_microbench__ecma262_IsConstructor(f64 argument, i32 argumentjjtype) {
  f64 _get1;
  i32 _get0;
  _get0 = argumentjjtype;
  // if 
    if ((f64)(_get0) != 6) {
      return 0;
    }
  // end
  j16:;
  _get1 = argument;
  return (f64)((f64)(dong_porf_js_microbench__Porffor_funcLut_flags((i32)(_get1)) & 2) == 2);
}

static struct ReturnValue dong_porf_js_microbench__Boolean_prototype_toString(f64 _this, i32 _thisjjtype) {
  f64 _get0;
  _get0 = _this;
  // if 
    if (((u32)(_get0)) != 0) {
      return (struct ReturnValue){ 170, 195 };
    }
  // end
  j35:;
  return (struct ReturnValue){ 180, 195 };
}

static f64 dong_porf_js_microbench__Porffor_bytestring_appendStr(f64 str, i32 strjjtype, f64 appendage, i32 appendagejjtype) {
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
  j38:;
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
        goto j38;
      }
    // end
    j39:;
  // end
  _get13 = str;
  _get14 = strLen;
  _get15 = appendageLen;
  i32_store(1, 0, (u32)(_get13), (u32)((_get14 + _get15)));
  return 1;
}

static struct ReturnValue dong_porf_js_microbench__Function_prototype_toString(f64 _this, i32 _thisjjtype) {
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
  j37:;
  out = (f64)(dong_porf_js_microbench__Porffor_malloc(256));
  _get1 = out;
  (void) dong_porf_js_microbench__Porffor_bytestring_appendStr(_get1, 195, 256, 195);
  _get2 = out;
  _get3 = _this;
  (void) dong_porf_js_microbench__Porffor_bytestring_appendStr(_get2, 195, (f64)(dong_porf_js_microbench__Porffor_funcLut_name((i32)(_get3))), 195);
  _get4 = out;
  (void) dong_porf_js_microbench__Porffor_bytestring_appendStr(_get4, 195, 271, 195);
  _get5 = out;
  return (struct ReturnValue){ _get5, 195 };
}

static i32 dong_porf_js_microbench__Porffor_object_lookup(i32 obj, i32 objjjtype, i32 target, i32 targetjjtype, i32 targetHash, i32 targetHashjjtype) {
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
  j45:;
  _get1 = obj;
  ptr = _get1 + 8;
  _get2 = ptr;
  _get3 = obj;
  endPtr = _get2 + (i32_load16_u(0, 0, _get3) * 18);
  // loop 
  j46:;
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
        j48:;
        _get9 = ptr;
        ptr = _get9 + 18;
        goto j46;
      }
    // end
    j47:;
  // end
  return -1;
}

static i32 dong_porf_js_microbench__Porffor_object_isObjectOrNull(i32 arg, i32 argjjtype) {
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

static void dong_porf_js_microbench__Porffor_object_setPrototype(i32 obj, i32 objjjtype, i32 proto, i32 protojjtype) {
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
      const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get1), _get2);
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
      j54:;
    }
  // end
  j53:;
  _get5 = proto;
  _get6 = protojjtype;
  // if 
    if ((dong_porf_js_microbench__Porffor_object_isObjectOrNull(_get5, _get6)) != 0) {
      _get7 = obj;
      _get8 = proto;
      i32_store(0, 4, _get7, _get8);
      _get9 = obj;
      _get10 = protojjtype;
      i32_store8(0, 3, _get9, _get10);
    }
  // end
  j55:;
  return;
}

static i32 dong_porf_js_microbench_jjget___Object_prototype() {
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
    if ((dong_porf_js_microbench_jjporfjjgetptr___Object_prototype) != 0) {
      return dong_porf_js_microbench_jjporfjjgetptr___Object_prototype;
    }
  // end
  j52:;
  l0 = 49152;
  _get0 = l0;
  dong_porf_js_microbench_jjporfjjgetptr___Object_prototype = _get0;
  _get1 = l0;
  const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get1), 7);
  (void) _0.type;
  l0 = _0.value;
  _get2 = l0;
  dong_porf_js_microbench__Porffor_object_fastAdd(_get2, 7, 367, 195, 3, 6, 10, 1);
  _get3 = l0;
  dong_porf_js_microbench__Porffor_object_fastAdd(_get3, 7, 989, 195, 4, 6, 10, 1);
  _get4 = l0;
  dong_porf_js_microbench__Porffor_object_fastAdd(_get4, 7, 1015, 195, 5, 6, 10, 1);
  _get5 = l0;
  dong_porf_js_microbench__Porffor_object_fastAdd(_get5, 7, 320, 195, 6, 6, 10, 1);
  _get6 = l0;
  dong_porf_js_microbench__Porffor_object_fastAdd(_get6, 7, 1072, 195, 7, 6, 10, 1);
  _get7 = l0;
  dong_porf_js_microbench__Porffor_object_fastAdd(_get7, 7, 845, 195, 2, 6, 10, 1);
  _get8 = l0;
  dong_porf_js_microbench__Porffor_object_setPrototype(_get8, 7, 0, 7);
  _get9 = l0;
  dong_porf_js_microbench__Porffor_object_fastAdd(_get9, 7, 81, 195, 8, 6, 10, 1);
  return dong_porf_js_microbench_jjporfjjgetptr___Object_prototype;
}

static struct ReturnValue dong_porf_js_microbench__Porffor_object_getHiddenPrototype(i32 trueType, i32 trueTypejjtype) {
  return (struct ReturnValue){ dong_porf_js_microbench_jjget___Object_prototype(), 7 };
}

static struct ReturnValue dong_porf_js_microbench__Porffor_object_getPrototype(i32 obj, i32 objjjtype) {
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
      const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get1), _get2);
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
      j63:;
    }
  // end
  j62:;
  _get5 = obj;
  _get6 = obj;
  return (struct ReturnValue){ i32_load(0, 4, _get5), i32_load8_u(0, 3, _get6) };
}

static struct ReturnValue dong_porf_js_microbench__Porffor_object_accessorGet(i32 entryPtr, i32 entryPtrjjtype) {
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
  j71:;
  _get2 = out;
  return (struct ReturnValue){ _get2, 6 };
}

static struct ReturnValue dong_porf_js_microbench__Porffor_object_get_withHash(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype, i32 hash, i32 hashjjtype) {
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
      const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get4), _get5);
      jjlast_type = _0.type;
      _get6 = jjlast_type;
      objjjtype = _get6;
      obj = _0.value;
    }
  // end
  j43:;
  _get7 = obj;
  // if 
    if ((_get7) == 0) {
    }
  // end
  j44:;
  _get8 = obj;
  _get9 = objjjtype;
  _get10 = key;
  _get11 = keyjjtype;
  _get12 = hash;
  entryPtr = dong_porf_js_microbench__Porffor_object_lookup(_get8, _get9, _get10, _get11, _get12, 1);
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
              obj = dong_porf_js_microbench_jjget___Object_prototype();
              objjjtype = 7;
            }
          // end
          j51:;
        } else {
          _get18 = trueType;
          const struct ReturnValue _1 = dong_porf_js_microbench__Porffor_object_getHiddenPrototype(_get18, 1);
          jjlast_type = _1.type;
          _get19 = jjlast_type;
          objjjtype = _get19;
          obj = _1.value;
        }
      // end
      j50:;
      _get20 = objjjtype;
      // if 
        if (_get20 != 7) {
          _get21 = obj;
          _get22 = objjjtype;
          const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get21), _get22);
          jjlast_type = _2.type;
          _get23 = jjlast_type;
          objjjtype = _get23;
          obj = _2.value;
        }
      // end
      j56:;
      _get24 = obj;
      lastProto = _get24;
      _get25 = objjjtype;
      lastProtojjtype = _get25;
      // loop 
      j57:;
        // if 
          if ((1) != 0) {
            _get26 = obj;
            _get27 = objjjtype;
            _get28 = key;
            _get29 = keyjjtype;
            _get30 = hash;
            entryPtr = dong_porf_js_microbench__Porffor_object_lookup(_get26, _get27, _get28, _get29, _get30, 1);
            _get31 = entryPtr;
            // if 
              if (_get31 != -1) {
                goto j58;
              }
            // end
            j59:;
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
                    obj = dong_porf_js_microbench_jjget___Object_prototype();
                    objjjtype = 7;
                  }
                // end
                j61:;
              } else {
                _get36 = obj;
                _get37 = objjjtype;
                const struct ReturnValue _3 = dong_porf_js_microbench__Porffor_object_getPrototype(_get36, _get37);
                jjlast_type = _3.type;
                _get38 = jjlast_type;
                objjjtype = _get38;
                obj = _3.value;
              }
            // end
            j60:;
            _get39 = objjjtype;
            // if 
              if (_get39 != 7) {
                _get40 = obj;
                _get41 = objjjtype;
                const struct ReturnValue _4 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get40), _get41);
                jjlast_type = _4.type;
                _get42 = jjlast_type;
                objjjtype = _get42;
                obj = _4.value;
              }
            // end
            j64:;
            _get43 = obj;
            jjlogicinner_tmp = _get43;
            _get44 = objjjtype;
            jjtypeswitch_tmp1 = _get44;
            // block i32
            i32 _r65;
              _get45 = jjtypeswitch_tmp1;
              // if 
                if ((_get45) == 0) {
                  _r65 = 1;
                  goto j65;
                }
              // end
              j66:;
              _get46 = jjtypeswitch_tmp1;
              // if 
                if (_get46 == 7) {
                  _get47 = jjlogicinner_tmp;
                  _r65 = (_get47) == 0;
                  goto j65;
                }
              // end
              j67:;
              _r65 = 0;
            // end
            j65:;
            _get48 = obj;
            _get49 = lastProto;
            // if 
              if ((_r65 | (_get48 == _get49)) != 0) {
                goto j58;
              }
            // end
            j68:;
            _get50 = obj;
            lastProto = _get50;
            _get51 = objjjtype;
            lastProtojjtype = _get51;
            goto j57;
          }
        // end
        j58:;
      // end
      _get52 = entryPtr;
      // if 
        if (_get52 == -1) {
          return (struct ReturnValue){ 0, 0 };
        }
      // end
      j69:;
    }
  // end
  j49:;
  _get53 = entryPtr;
  tail = i32_load16_u(0, 16, _get53);
  _get54 = tail;
  // if 
    if ((_get54 & 1) != 0) {
      _get55 = entryPtr;
      const struct ReturnValue _5 = dong_porf_js_microbench__Porffor_object_accessorGet(_get55, 1);
      jjlast_type = _5.type;
      get = _5.value;
      _get56 = get;
      // if 
        if ((_get56) == 0) {
          return (struct ReturnValue){ 0, 0 };
        }
      // end
      j72:;
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
  j70:;
  _get66 = entryPtr;
  _get67 = tail;
  return (struct ReturnValue){ f64_load(0, 8, _get66), (u32)(_get67) >> 8 };
}

static struct ReturnValue dong_porf_js_microbench__Porffor_object_readValue(i32 entryPtr, i32 entryPtrjjtype) {
  i32 _get1;
  i32 _get0;
  _get0 = entryPtr;
  _get1 = entryPtr;
  return (struct ReturnValue){ f64_load(0, 8, _get0), i32_load8_u(0, 17, _get1) };
}

static struct ReturnValue dong_porf_js_microbench__Object_prototype_toString(f64 _this, i32 _thisjjtype) {
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
          jjmember_prop_254 = 320;
          _get3 = obj;
          jjmember_obj_254 = _get3;
          _get4 = jjmember_obj_254;
          _get5 = jjmember_prop_254;
          const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_get_withHash((i32)(_get4), 7, (u32)(_get5), 195, -2133638001, 1);
          jjlast_type = _0.type;
          _get6 = jjlast_type;
          ovrjjtype = _get6;
          ovr = _0.value;
          _get7 = ovrjjtype;
          logictmpi = (f64)(_get7) == 6;
          _get8 = logictmpi;
          // if i32
          i32 _r73;
            if ((_get8) != 0) {
              _get9 = ovr;
              jjlast_type = 2;
              _r73 = _get9 != 6;
            } else {
              _get10 = logictmpi;
              jjlast_type = 2;
              _r73 = _get10;
            }
          // end
          j73:;
          // if 
            if ((_r73) != 0) {
              _get11 = ovr;
              jjindirect_255_callee = _get11;
              _get12 = ovrjjtype;
              // if f64
              f64 _r75;
                if (_get12 == 6) {
                  _get13 = _this;
                  jjcall_val = _get13;
                  _get14 = jjcall_val;
                  _get15 = _thisjjtype;
                  jjcall_type = _get15;
                  _get16 = jjcall_type;
                  _get17 = jjindirect_255_callee;
                  jjlast_type = 0;
                  _r75 = 0;
                } else {
                  _r75 = 0;
                }
              // end
              j75:;
              _get18 = jjlast_type;
              return (struct ReturnValue){ _r75, _get18 };
              (void) 0;
            }
          // end
          j74:;
          _get19 = obj;
          entryPtr = (f64)(dong_porf_js_microbench__Porffor_object_lookup((i32)(_get19), 7, 320, 195, dong_porf_js_microbench__Porffor_object_hash(320, 195), 1));
          _get20 = entryPtr;
          // if 
            if (_get20 != -1) {
              _get21 = entryPtr;
              const struct ReturnValue _1 = dong_porf_js_microbench__Porffor_object_readValue((i32)(_get21), 1);
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
                  f64 _r78;
                    if (_get25 == 6) {
                      _get26 = _this;
                      jjcall_val = _get26;
                      _get27 = jjcall_val;
                      _get28 = _thisjjtype;
                      jjcall_type = _get28;
                      _get29 = jjcall_type;
                      _get30 = jjindirect_256_callee;
                      jjlast_type = 0;
                      _r78 = 0;
                    } else {
                      _r78 = 0;
                    }
                  // end
                  j78:;
                  _get31 = jjlast_type;
                  return (struct ReturnValue){ _r78, _get31 };
                  (void) 0;
                } else {
                  return (struct ReturnValue){ 0, 0 };
                  (void) 0;
                }
              // end
              j77:;
            }
          // end
          j76:;
        }
      // end
      j42:;
    }
  // end
  j41:;
  _get32 = _this;
  _get33 = _thisjjtype;
  // if 
    if (((_get32 == 0) & ((_get33 | 128) == (0 | 128))) != 0) {
      return (struct ReturnValue){ 1221, 195 };
      (void) 0;
    }
  // end
  j79:;
  _get34 = _this;
  _get35 = _thisjjtype;
  // if 
    if (((_get34 == 0) & ((_get35 | 128) == (7 | 128))) != 0) {
      return (struct ReturnValue){ 1245, 195 };
      (void) 0;
    }
  // end
  j80:;
  _get36 = _thisjjtype;
  // if 
    if ((f64)(_get36) == 72) {
      return (struct ReturnValue){ 1264, 195 };
      (void) 0;
    }
  // end
  j81:;
  _get37 = _thisjjtype;
  // if 
    if ((f64)(_get37) == 6) {
      return (struct ReturnValue){ 1284, 195 };
      (void) 0;
    }
  // end
  j82:;
  _get38 = _thisjjtype;
  _get39 = _thisjjtype;
  // if 
    if ((((f64)(_get38) == 2) | ((f64)(_get39) == 31)) != 0) {
      return (struct ReturnValue){ 1307, 195 };
      (void) 0;
    }
  // end
  j83:;
  _get40 = _thisjjtype;
  _get41 = _thisjjtype;
  // if 
    if ((((f64)(_get40) == 1) | ((f64)(_get41) == 32)) != 0) {
      return (struct ReturnValue){ 1329, 195 };
      (void) 0;
    }
  // end
  j84:;
  _get42 = _thisjjtype;
  _get43 = _thisjjtype;
  // if 
    if ((((f64)(_get42 | 128) == 195) | ((f64)(_get43) == 33)) != 0) {
      return (struct ReturnValue){ 1350, 195 };
      (void) 0;
    }
  // end
  j85:;
  _get44 = _thisjjtype;
  // if 
    if ((f64)(_get44) == 10) {
      return (struct ReturnValue){ 1371, 195 };
      (void) 0;
    }
  // end
  j86:;
  _get45 = _thisjjtype;
  // if 
    if ((f64)(_get45) == 9) {
      return (struct ReturnValue){ 1390, 195 };
      (void) 0;
    }
  // end
  j87:;
  return (struct ReturnValue){ 1411, 195 };
}

static i32 dong_porf_js_microbench__Porffor_bytestringToString(i32 src) {
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
  dst = dong_porf_js_microbench__Porffor_malloc((_get1 * 2) + 6);
  _get2 = dst;
  _get3 = len;
  i32_store(0, 0, _get2, _get3);
  // loop 
  j94:;
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
      goto j94;
    }
  // end
  _get11 = dst;
  return _get11;
}

static struct ReturnValue dong_porf_js_microbench__String_prototype_toString(i32 _this, i32 _thisjjtype) {
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
      j92:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_js_microbench__Porffor_bytestringToString(_get7);
        }
      // end
      j93:;
    }
  // end
  j91:;
  _get8 = _this;
  return (struct ReturnValue){ _get8, 67 };
}

static struct ReturnValue dong_porf_js_microbench__TypeError_prototype_namekkget(f64 _this, i32 _thisjjtype) {
  return (struct ReturnValue){ 297, 195 };
}

static struct ReturnValue dong_porf_js_microbench__TypeError_prototype_messagekkget(f64 _this, i32 _thisjjtype) {
  f64 _get1;
  f64 _get0;
  _get0 = _this;
  _get1 = _this;
  return (struct ReturnValue){ (f64)(i32_load(0, 0, (u32)(_get0))), i32_load8_u(0, 4, (u32)(_get1)) };
}

static struct ReturnValue dong_porf_js_microbench__Porffor_strcat(i32 a, i32 ajjtype, i32 b, i32 bjjtype) {
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
          out = dong_porf_js_microbench__Porffor_malloc((6 + _get4) + _get5);
          _get6 = out;
          _get7 = al;
          _get8 = bl;
          i32_store(0, 0, _get6, _get7 + _get8);
          _get9 = out;
          _get10 = a;
          _get11 = al;
          memcpy(dong_porf_js_microbench_memory + (_get9 + 4), dong_porf_js_microbench_memory + (_get10 + 4), _get11);
          _get12 = out;
          _get13 = al;
          _get14 = b;
          _get15 = bl;
          memcpy(dong_porf_js_microbench_memory + ((_get12 + 4) + _get13), dong_porf_js_microbench_memory + (_get14 + 4), _get15);
          _get16 = out;
          return (struct ReturnValue){ _get16, 195 };
        } else {
          _get17 = al;
          _get18 = bl;
          out = dong_porf_js_microbench__Porffor_malloc(6 + ((_get17 + _get18) * 2));
          _get19 = out;
          _get20 = al;
          _get21 = bl;
          i32_store(0, 0, _get19, _get20 + _get21);
          i = 0;
          // loop 
          j101:;
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
                goto j101;
              }
            // end
            j102:;
          // end
          _get29 = out;
          _get30 = al;
          _get31 = b;
          _get32 = bl;
          memcpy(dong_porf_js_microbench_memory + ((_get29 + 4) + (_get30 * 2)), dong_porf_js_microbench_memory + (_get31 + 4), (_get32 * 2));
          _get33 = out;
          return (struct ReturnValue){ _get33, 67 };
        }
      // end
      j100:;
    } else {
      _get34 = bjjtype;
      // if 
        if (_get34 == 195) {
          _get35 = al;
          _get36 = bl;
          out = dong_porf_js_microbench__Porffor_malloc(6 + ((_get35 + _get36) * 2));
          _get37 = out;
          _get38 = al;
          _get39 = bl;
          i32_store(0, 0, _get37, _get38 + _get39);
          _get40 = out;
          _get41 = a;
          _get42 = al;
          memcpy(dong_porf_js_microbench_memory + (_get40 + 4), dong_porf_js_microbench_memory + (_get41 + 4), (_get42 * 2));
          _get43 = out;
          _get44 = al;
          ptr = _get43 + (_get44 * 2);
          i = 0;
          // loop 
          j104:;
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
                goto j104;
              }
            // end
            j105:;
          // end
          _get52 = out;
          return (struct ReturnValue){ _get52, 67 };
        } else {
          _get53 = al;
          _get54 = bl;
          out = dong_porf_js_microbench__Porffor_malloc(6 + ((_get53 + _get54) * 2));
          _get55 = out;
          _get56 = al;
          _get57 = bl;
          i32_store(0, 0, _get55, _get56 + _get57);
          _get58 = out;
          _get59 = a;
          _get60 = al;
          memcpy(dong_porf_js_microbench_memory + (_get58 + 4), dong_porf_js_microbench_memory + (_get59 + 4), (_get60 * 2));
          _get61 = out;
          _get62 = al;
          _get63 = b;
          _get64 = bl;
          memcpy(dong_porf_js_microbench_memory + ((_get61 + 4) + (_get62 * 2)), dong_porf_js_microbench_memory + (_get63 + 4), (_get64 * 2));
          _get65 = out;
          return (struct ReturnValue){ _get65, 67 };
        }
      // end
      j103:;
    }
  // end
  j99:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_js_microbench__Porffor_concatStrings(f64 a, i32 ajjtype, f64 b, i32 bjjtype) {
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
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString(_get1, _get2);
      jjlast_type = _0.type;
      _get3 = jjlast_type;
      ajjtype = _get3;
      a = _0.value;
    }
  // end
  j97:;
  _get4 = bjjtype;
  // if 
    if ((f64)(_get4 | 128) != 195) {
      _get5 = b;
      _get6 = bjjtype;
      const struct ReturnValue _1 = dong_porf_js_microbench__ecma262_ToString(_get5, _get6);
      jjlast_type = _1.type;
      _get7 = jjlast_type;
      bjjtype = _get7;
      b = _1.value;
    }
  // end
  j98:;
  _get8 = a;
  _get9 = ajjtype;
  _get10 = b;
  _get11 = bjjtype;
  const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_strcat((i32)(_get8), _get9, (i32)(_get10), _get11);
  jjlast_type = _2.type;
  _get12 = jjlast_type;
  return (struct ReturnValue){ (f64)(_2.value), _get12 };
}

static struct ReturnValue dong_porf_js_microbench__TypeError_prototype_toString(f64 _this, i32 _thisjjtype) {
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
  const struct ReturnValue _0 = dong_porf_js_microbench__TypeError_prototype_namekkget(_get0, 38);
  jjlast_type = _0.type;
  _get1 = jjlast_type;
  namejjtype = _get1;
  name = _0.value;
  _get2 = _this;
  const struct ReturnValue _1 = dong_porf_js_microbench__TypeError_prototype_messagekkget(_get2, 38);
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
  j96:;
  _get7 = name;
  _get8 = namejjtype;
  const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_concatStrings(_get7, _get8, 312, 195);
  jjlast_type = _2.type;
  _get9 = jjlast_type;
  _get10 = message;
  _get11 = messagejjtype;
  const struct ReturnValue _3 = dong_porf_js_microbench__Porffor_concatStrings(_2.value, _get9, _get10, _get11);
  jjlast_type = _3.type;
  _get12 = jjlast_type;
  return (struct ReturnValue){ _3.value, _get12 };
}

static struct ReturnValue dong_porf_js_microbench__SyntaxError_prototype_namekkget(f64 _this, i32 _thisjjtype) {
  return (struct ReturnValue){ 2612, 195 };
}

static struct ReturnValue dong_porf_js_microbench__SyntaxError_prototype_messagekkget(f64 _this, i32 _thisjjtype) {
  f64 _get1;
  f64 _get0;
  _get0 = _this;
  _get1 = _this;
  return (struct ReturnValue){ (f64)(i32_load(0, 0, (u32)(_get0))), i32_load8_u(0, 4, (u32)(_get1)) };
}

static struct ReturnValue dong_porf_js_microbench__SyntaxError_prototype_toString(f64 _this, i32 _thisjjtype) {
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
  f64 jjmember_obj_148 = 0;
  f64 jjmember_prop_148 = 0;
  i32 jjlast_type = 0;
  f64 message = 0;
  i32 messagejjtype = 0;
  f64 jjmember_obj_149 = 0;
  f64 jjmember_prop_149 = 0;

  _get0 = _this;
  const struct ReturnValue _0 = dong_porf_js_microbench__SyntaxError_prototype_namekkget(_get0, 40);
  jjlast_type = _0.type;
  _get1 = jjlast_type;
  namejjtype = _get1;
  name = _0.value;
  _get2 = _this;
  const struct ReturnValue _1 = dong_porf_js_microbench__SyntaxError_prototype_messagekkget(_get2, 40);
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
  j107:;
  _get7 = name;
  _get8 = namejjtype;
  const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_concatStrings(_get7, _get8, 312, 195);
  jjlast_type = _2.type;
  _get9 = jjlast_type;
  _get10 = message;
  _get11 = messagejjtype;
  const struct ReturnValue _3 = dong_porf_js_microbench__Porffor_concatStrings(_2.value, _get9, _get10, _get11);
  jjlast_type = _3.type;
  _get12 = jjlast_type;
  return (struct ReturnValue){ _3.value, _get12 };
}

static struct ReturnValue dong_porf_js_microbench__RangeError_prototype_namekkget(f64 _this, i32 _thisjjtype) {
  return (struct ReturnValue){ 2629, 195 };
}

static struct ReturnValue dong_porf_js_microbench__RangeError_prototype_messagekkget(f64 _this, i32 _thisjjtype) {
  f64 _get1;
  f64 _get0;
  _get0 = _this;
  _get1 = _this;
  return (struct ReturnValue){ (f64)(i32_load(0, 0, (u32)(_get0))), i32_load8_u(0, 4, (u32)(_get1)) };
}

static struct ReturnValue dong_porf_js_microbench__RangeError_prototype_toString(f64 _this, i32 _thisjjtype) {
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
  const struct ReturnValue _0 = dong_porf_js_microbench__RangeError_prototype_namekkget(_get0, 41);
  jjlast_type = _0.type;
  _get1 = jjlast_type;
  namejjtype = _get1;
  name = _0.value;
  _get2 = _this;
  const struct ReturnValue _1 = dong_porf_js_microbench__RangeError_prototype_messagekkget(_get2, 41);
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
  j109:;
  _get7 = name;
  _get8 = namejjtype;
  const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_concatStrings(_get7, _get8, 312, 195);
  jjlast_type = _2.type;
  _get9 = jjlast_type;
  _get10 = message;
  _get11 = messagejjtype;
  const struct ReturnValue _3 = dong_porf_js_microbench__Porffor_concatStrings(_2.value, _get9, _get10, _get11);
  jjlast_type = _3.type;
  _get12 = jjlast_type;
  return (struct ReturnValue){ _3.value, _get12 };
}

static i32 dong_porf_js_microbench__Porffor_object_isObject(i32 arg, i32 argjjtype) {
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

static struct ReturnValue dong_porf_js_microbench__ecma262_ToPropertyKey(f64 argument, i32 argumentjjtype) {
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
  i32 _r138;
    if ((_get3) != 0) {
      _get4 = argument;
      jjlast_type = 2;
      _r138 = _get4 != 0;
    } else {
      _get5 = logictmpi;
      jjlast_type = 2;
      _r138 = _get5;
    }
  // end
  j138:;
  // if 
    if ((_r138) != 0) {
      _get6 = argument;
      _get7 = argumentjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToPrimitive_String(_get6, _get7);
      jjlast_type = _0.type;
      _get8 = jjlast_type;
      keyjjtype = _get8;
      key = _0.value;
    }
  // end
  j139:;
  _get9 = keyjjtype;
  // if 
    if ((f64)(_get9) == 5) {
      _get10 = key;
      _get11 = keyjjtype;
      return (struct ReturnValue){ _get10, _get11 };
    }
  // end
  j140:;
  _get12 = key;
  _get13 = keyjjtype;
  const struct ReturnValue _1 = dong_porf_js_microbench__ecma262_ToString(_get12, _get13);
  jjlast_type = _1.type;
  _get14 = jjlast_type;
  return (struct ReturnValue){ _1.value, _get14 };
}

static i32 dong_porf_js_microbench__Porffor_strcmp(i32 a, i32 ajjtype, i32 b, i32 bjjtype) {
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
  j147:;
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
  j148:;
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
              j152:;
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
                j153:;
                _get21 = al;
                al = _get21 - 32;
                _get22 = al;
                if (_get22 >= 32) {
                  goto j152;
                }
              // end
            }
          // end
          j151:;
          _get23 = al;
          // if 
            if (_get23 >= 8) {
              // loop 
              j155:;
                _get24 = ap8;
                _get25 = al;
                _get26 = bp8;
                _get27 = al;
                // if 
                  if ((_get26 + _get27) != 0) {
                    return 0;
                  }
                // end
                j156:;
                _get28 = al;
                al = _get28 - 8;
                _get29 = al;
                if (_get29 >= 8) {
                  goto j155;
                }
              // end
            }
          // end
          j154:;
          _get30 = al;
          // if 
            if (_get30 >= 2) {
              // loop 
              j158:;
                _get31 = a;
                _get32 = al;
                _get33 = b;
                _get34 = al;
                // if 
                  if (i32_load16_u(0, 2, _get31 + _get32) != i32_load16_u(0, 2, _get33 + _get34)) {
                    return 0;
                  }
                // end
                j159:;
                _get35 = al;
                al = _get35 - 2;
                _get36 = al;
                if (_get36 >= 2) {
                  goto j158;
                }
              // end
            }
          // end
          j157:;
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
              j161:;
            }
          // end
          j160:;
          return 1;
          (void) (_get17 + _get18);
        } else {
          i = 0;
          // loop 
          j162:;
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
                j164:;
                _get46 = i;
                i = _get46 + 1;
                goto j162;
              }
            // end
            j163:;
          // end
          return 1;
          (void) (_get13 + _get14);
        }
      // end
      j150:;
    } else {
      _get47 = bjjtype;
      // if 
        if (_get47 == 195) {
          i = 0;
          // loop 
          j166:;
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
                j168:;
                _get54 = i;
                i = _get54 + 1;
                goto j166;
              }
            // end
            j167:;
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
              j170:;
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
                j171:;
                _get70 = al;
                al = _get70 - 32;
                _get71 = al;
                if (_get71 >= 32) {
                  goto j170;
                }
              // end
            }
          // end
          j169:;
          _get72 = al;
          // if 
            if (_get72 >= 8) {
              // loop 
              j173:;
                _get73 = ap8;
                _get74 = al;
                _get75 = bp8;
                _get76 = al;
                // if 
                  if ((_get75 + _get76) != 0) {
                    return 0;
                  }
                // end
                j174:;
                _get77 = al;
                al = _get77 - 8;
                _get78 = al;
                if (_get78 >= 8) {
                  goto j173;
                }
              // end
            }
          // end
          j172:;
          _get79 = al;
          // if 
            if (_get79 >= 2) {
              // loop 
              j176:;
                _get80 = a;
                _get81 = al;
                _get82 = b;
                _get83 = al;
                // if 
                  if (i32_load16_u(0, 2, _get80 + _get81) != i32_load16_u(0, 2, _get82 + _get83)) {
                    return 0;
                  }
                // end
                j177:;
                _get84 = al;
                al = _get84 - 2;
                _get85 = al;
                if (_get85 >= 2) {
                  goto j176;
                }
              // end
            }
          // end
          j175:;
          return 1;
          (void) (_get73 + _get74);
        }
      // end
      j165:;
    }
  // end
  j149:;
  return 0;
}

static struct ReturnValue dong_porf_js_microbench__Porffor_object_get(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype) {
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
      const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get4), _get5);
      jjlast_type = _0.type;
      _get6 = jjlast_type;
      objjjtype = _get6;
      obj = _0.value;
    }
  // end
  j141:;
  _get7 = obj;
  // if 
    if ((_get7) == 0) {
    }
  // end
  j142:;
  _get8 = key;
  _get9 = keyjjtype;
  hash = dong_porf_js_microbench__Porffor_object_hash(_get8, _get9);
  _get10 = obj;
  _get11 = objjjtype;
  _get12 = key;
  _get13 = keyjjtype;
  _get14 = hash;
  entryPtr = dong_porf_js_microbench__Porffor_object_lookup(_get10, _get11, _get12, _get13, _get14, 1);
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
              obj = dong_porf_js_microbench_jjget___Object_prototype();
              objjjtype = 7;
            }
          // end
          j145:;
        } else {
          _get20 = trueType;
          const struct ReturnValue _1 = dong_porf_js_microbench__Porffor_object_getHiddenPrototype(_get20, 1);
          jjlast_type = _1.type;
          _get21 = jjlast_type;
          objjjtype = _get21;
          obj = _1.value;
        }
      // end
      j144:;
      _get22 = hash;
      // if 
        if (_get22 == 593337848) {
          _get23 = key;
          _get24 = keyjjtype;
          // if 
            if ((dong_porf_js_microbench__Porffor_strcmp(_get23, _get24, 579, 195)) != 0) {
              _get25 = obj;
              _get26 = objjjtype;
              return (struct ReturnValue){ (f64)(_get25), _get26 };
            }
          // end
          j178:;
        }
      // end
      j146:;
      _get27 = objjjtype;
      // if 
        if (_get27 != 7) {
          _get28 = obj;
          _get29 = objjjtype;
          const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get28), _get29);
          jjlast_type = _2.type;
          _get30 = jjlast_type;
          objjjtype = _get30;
          obj = _2.value;
        }
      // end
      j179:;
      _get31 = obj;
      lastProto = _get31;
      _get32 = objjjtype;
      lastProtojjtype = _get32;
      // loop 
      j180:;
        // if 
          if ((1) != 0) {
            _get33 = obj;
            _get34 = objjjtype;
            _get35 = key;
            _get36 = keyjjtype;
            _get37 = hash;
            entryPtr = dong_porf_js_microbench__Porffor_object_lookup(_get33, _get34, _get35, _get36, _get37, 1);
            _get38 = entryPtr;
            // if 
              if (_get38 != -1) {
                goto j181;
              }
            // end
            j182:;
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
                    obj = dong_porf_js_microbench_jjget___Object_prototype();
                    objjjtype = 7;
                  }
                // end
                j184:;
              } else {
                _get43 = obj;
                _get44 = objjjtype;
                const struct ReturnValue _3 = dong_porf_js_microbench__Porffor_object_getPrototype(_get43, _get44);
                jjlast_type = _3.type;
                _get45 = jjlast_type;
                objjjtype = _get45;
                obj = _3.value;
              }
            // end
            j183:;
            _get46 = objjjtype;
            // if 
              if (_get46 != 7) {
                _get47 = obj;
                _get48 = objjjtype;
                const struct ReturnValue _4 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get47), _get48);
                jjlast_type = _4.type;
                _get49 = jjlast_type;
                objjjtype = _get49;
                obj = _4.value;
              }
            // end
            j185:;
            _get50 = obj;
            jjlogicinner_tmp = _get50;
            _get51 = objjjtype;
            jjtypeswitch_tmp1 = _get51;
            // block i32
            i32 _r186;
              _get52 = jjtypeswitch_tmp1;
              // if 
                if ((_get52) == 0) {
                  _r186 = 1;
                  goto j186;
                }
              // end
              j187:;
              _get53 = jjtypeswitch_tmp1;
              // if 
                if (_get53 == 7) {
                  _get54 = jjlogicinner_tmp;
                  _r186 = (_get54) == 0;
                  goto j186;
                }
              // end
              j188:;
              _r186 = 0;
            // end
            j186:;
            _get55 = obj;
            _get56 = lastProto;
            // if 
              if ((_r186 | (_get55 == _get56)) != 0) {
                goto j181;
              }
            // end
            j189:;
            _get57 = obj;
            lastProto = _get57;
            _get58 = objjjtype;
            lastProtojjtype = _get58;
            goto j180;
          }
        // end
        j181:;
      // end
      _get59 = entryPtr;
      // if 
        if (_get59 == -1) {
          return (struct ReturnValue){ 0, 0 };
        }
      // end
      j190:;
    }
  // end
  j143:;
  _get60 = entryPtr;
  tail = i32_load16_u(0, 16, _get60);
  _get61 = tail;
  // if 
    if ((_get61 & 1) != 0) {
      _get62 = entryPtr;
      const struct ReturnValue _5 = dong_porf_js_microbench__Porffor_object_accessorGet(_get62, 1);
      jjlast_type = _5.type;
      get = _5.value;
      _get63 = get;
      // if 
        if ((_get63) == 0) {
          return (struct ReturnValue){ 0, 0 };
        }
      // end
      j192:;
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
  j191:;
  _get73 = entryPtr;
  _get74 = tail;
  return (struct ReturnValue){ f64_load(0, 8, _get73), (u32)(_get74) >> 8 };
}

static struct ReturnValue dong_porf_js_microbench__String_prototype_trimEnd(i32 _this, i32 _thisjjtype) {
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
      j205:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_js_microbench__Porffor_bytestringToString(_get7);
        }
      // end
      j206:;
    }
  // end
  j204:;
  _get8 = _this;
  len = i32_load(1, 0, _get8);
  _get9 = len;
  out = dong_porf_js_microbench__Porffor_malloc(6 + (_get9 * 2));
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
  j207:;
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
                goto j207;
              }
            // end
            j210:;
            start = 0;
          }
        // end
        j209:;
        _get49 = outPtr;
        _get50 = chr;
        i32_store16(0, 4, _get49, _get50);
        goto j207;
      }
    // end
    j208:;
  // end
  _get51 = out;
  _get52 = len;
  _get53 = n;
  i32_store(1, 0, _get51, _get52 - _get53);
  _get54 = out;
  return (struct ReturnValue){ _get54, 67 };
}

static struct ReturnValue dong_porf_js_microbench__String_prototype_trimStart(i32 _this, i32 _thisjjtype) {
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
      j212:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_js_microbench__Porffor_bytestringToString(_get7);
        }
      // end
      j213:;
    }
  // end
  j211:;
  _get8 = _this;
  len = i32_load(1, 0, _get8);
  _get9 = len;
  out = dong_porf_js_microbench__Porffor_malloc(6 + (_get9 * 2));
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
  j214:;
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
                goto j214;
              }
            // end
            j217:;
            start = 0;
          }
        // end
        j216:;
        _get45 = outPtr;
        _get46 = chr;
        i32_store16(0, 4, _get45, _get46);
        _get47 = outPtr;
        outPtr = _get47 + 2;
        goto j214;
      }
    // end
    j215:;
  // end
  _get48 = out;
  _get49 = len;
  _get50 = n;
  i32_store(1, 0, _get48, _get49 - _get50);
  _get51 = out;
  return (struct ReturnValue){ _get51, 67 };
}

static struct ReturnValue dong_porf_js_microbench__String_prototype_trim(i32 _this, i32 _thisjjtype) {
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
      j202:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_js_microbench__Porffor_bytestringToString(_get7);
        }
      // end
      j203:;
    }
  // end
  j201:;
  _get8 = _this;
  const struct ReturnValue _1 = dong_porf_js_microbench__String_prototype_trimEnd(_get8, 67);
  jjlast_type = _1.type;
  _get9 = jjlast_type;
  const struct ReturnValue _2 = dong_porf_js_microbench__String_prototype_trimStart(_1.value, _get9);
  jjlast_type = _2.type;
  _get10 = jjlast_type;
  return (struct ReturnValue){ _2.value, _get10 };
}

static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_trimEnd(i32 _this, i32 _thisjjtype) {
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
  out = dong_porf_js_microbench__Porffor_malloc(6 + _get1);
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
  j220:;
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
                goto j220;
              }
            // end
            j223:;
            start = 0;
          }
        // end
        j222:;
        _get41 = outPtr;
        _get42 = chr;
        i32_store8(0, 4, _get41, _get42);
        goto j220;
      }
    // end
    j221:;
  // end
  _get43 = out;
  _get44 = len;
  _get45 = n;
  i32_store(1, 0, _get43, _get44 - _get45);
  _get46 = out;
  return (struct ReturnValue){ _get46, 195 };
}

static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_trimStart(i32 _this, i32 _thisjjtype) {
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
  out = dong_porf_js_microbench__Porffor_malloc(6 + _get1);
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
  j224:;
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
                goto j224;
              }
            // end
            j227:;
            start = 0;
          }
        // end
        j226:;
        _get37 = outPtr;
        _get38 = outPtr;
        outPtr = _get38 + 1;
        _get39 = chr;
        i32_store8(0, 4, _get37, _get39);
        goto j224;
      }
    // end
    j225:;
  // end
  _get40 = out;
  _get41 = len;
  _get42 = n;
  i32_store(1, 0, _get40, _get41 - _get42);
  _get43 = out;
  return (struct ReturnValue){ _get43, 195 };
}

static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_trim(i32 _this, i32 _thisjjtype) {
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 jjlast_type = 0;

  _get0 = _this;
  const struct ReturnValue _0 = dong_porf_js_microbench__ByteString_prototype_trimEnd(_get0, 195);
  jjlast_type = _0.type;
  _get1 = jjlast_type;
  const struct ReturnValue _1 = dong_porf_js_microbench__ByteString_prototype_trimStart(_0.value, _get1);
  jjlast_type = _1.type;
  _get2 = jjlast_type;
  return (struct ReturnValue){ _1.value, _get2 };
}

static f64 dong_porf_js_microbench__Math_trunc(f64 l0) {
  f64 _get0;
  _get0 = l0;
  return trunc(_get0);
}

static struct ReturnValue dong_porf_js_microbench__String_prototype_charCodeAt(f64 _this, i32 _thisjjtype, f64 index, i32 indexjjtype) {
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
      j232:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString(_get4, _get5);
      _thisjjtype = _0.type;
      _this = _0.value;
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = (f64)(dong_porf_js_microbench__Porffor_bytestringToString((u32)(_get7)));
        }
      // end
      j233:;
    }
  // end
  j231:;
  _get8 = _this;
  len = (f64)(i32_load(1, 0, (u32)(_get8)));
  _get9 = index;
  index = dong_porf_js_microbench__Math_trunc(_get9);
  _get10 = index;
  _get11 = index;
  _get12 = len;
  // if 
    if (((_get10 < 0) | (_get11 >= _get12)) != 0) {
      return (struct ReturnValue){ NaN, 1 };
    }
  // end
  j234:;
  _get13 = _this;
  _get14 = index;
  return (struct ReturnValue){ (f64)(i32_load16_u(0, 4, (i32)((_get13 + (_get14 * 2))))), 1 };
}

static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_charCodeAt(f64 _this, i32 _thisjjtype, f64 index, i32 indexjjtype) {
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
  index = dong_porf_js_microbench__Math_trunc(_get1);
  _get2 = index;
  _get3 = index;
  _get4 = len;
  // if 
    if (((_get2 < 0) | (_get3 >= _get4)) != 0) {
      return (struct ReturnValue){ NaN, 1 };
    }
  // end
  j237:;
  _get5 = _this;
  _get6 = index;
  return (struct ReturnValue){ (f64)(i32_load8_u(0, 4, (i32)((_get5 + _get6)))), 1 };
}

static f64 dong_porf_js_microbench__Porffor_stn_int(f64 str, i32 strjjtype, f64 radix, i32 radixjjtype, f64 i, i32 ijjtype) {
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
  j245:;
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
  j246:;
  // loop 
  j247:;
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
        f64 _r249;
          _get10 = jjtypeswitch_tmp1;
          // if 
            if (_get10 == 33) {
              _get11 = jjproto_target;
              _get12 = jjproto_targetjjtype;
              _get13 = i;
              _get14 = i;
              i = _get14 + 1;
              const struct ReturnValue _0 = dong_porf_js_microbench__String_prototype_charCodeAt(_get11, _get12, _get13, 1);
              jjlast_type = _0.type;
              _r249 = _0.value;
              goto j249;
            }
          // end
          j250:;
          _get15 = jjtypeswitch_tmp1;
          // if 
            if (_get15 == 67) {
              _get16 = jjproto_target;
              _get17 = jjproto_targetjjtype;
              _get18 = i;
              _get19 = i;
              i = _get19 + 1;
              const struct ReturnValue _1 = dong_porf_js_microbench__String_prototype_charCodeAt(_get16, _get17, _get18, 1);
              jjlast_type = _1.type;
              _r249 = _1.value;
              goto j249;
            }
          // end
          j251:;
          _get20 = jjtypeswitch_tmp1;
          // if 
            if (_get20 == 195) {
              _get21 = jjproto_target;
              _get22 = jjproto_targetjjtype;
              _get23 = i;
              _get24 = i;
              i = _get24 + 1;
              const struct ReturnValue _2 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get21, _get22, _get23, 1);
              jjlast_type = _2.type;
              _r249 = _2.value;
              goto j249;
            }
          // end
          j252:;
          _r249 = 0;
        // end
        j249:;
        chr = _r249;
        _get25 = chr;
        logictmpi = _get25 >= 48;
        _get26 = logictmpi;
        // if i32
        i32 _r253;
          if ((_get26) != 0) {
            _get27 = chr;
            _get28 = nMax;
            jjlast_type = 2;
            _r253 = _get27 < _get28;
          } else {
            _get29 = logictmpi;
            jjlast_type = 2;
            _r253 = _get29;
          }
        // end
        j253:;
        // if 
          if ((_r253) != 0) {
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
                i32 _r256;
                  if ((_get35) != 0) {
                    _get36 = chr;
                    _get37 = radix;
                    jjlast_type = 2;
                    _r256 = _get36 < (87 + _get37);
                  } else {
                    _get38 = logictmpi;
                    jjlast_type = 2;
                    _r256 = _get38;
                  }
                // end
                j256:;
                // if 
                  if ((_r256) != 0) {
                    _get39 = n;
                    _get40 = radix;
                    _get41 = chr;
                    n = ((_get39 * _get40) + _get41) - 87;
                  } else {
                    _get42 = chr;
                    logictmpi = _get42 >= 65;
                    _get43 = logictmpi;
                    // if i32
                    i32 _r258;
                      if ((_get43) != 0) {
                        _get44 = chr;
                        _get45 = radix;
                        jjlast_type = 2;
                        _r258 = _get44 < (55 + _get45);
                      } else {
                        _get46 = logictmpi;
                        jjlast_type = 2;
                        _r258 = _get46;
                      }
                    // end
                    j258:;
                    // if 
                      if ((_r258) != 0) {
                        _get47 = n;
                        _get48 = radix;
                        _get49 = chr;
                        n = ((_get47 * _get48) + _get49) - 55;
                      } else {
                        return NaN;
                      }
                    // end
                    j259:;
                  }
                // end
                j257:;
              } else {
                return NaN;
              }
            // end
            j255:;
          }
        // end
        j254:;
        goto j247;
      }
    // end
    j248:;
  // end
  _get50 = n;
  return _get50;
}

static f64 dong_porf_js_microbench__Porffor_parseExp(f64 str, i32 strjjtype, f64 i, i32 ijjtype, f64 len, i32 lenjjtype, f64 strict, i32 strictjjtype) {
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
      f64 _r333;
        _get5 = jjtypeswitch_tmp1;
        // if 
          if (_get5 == 33) {
            _get6 = jjproto_target;
            _get7 = jjproto_targetjjtype;
            _get8 = i;
            const struct ReturnValue _0 = dong_porf_js_microbench__String_prototype_charCodeAt(_get6, _get7, _get8, 1);
            jjlast_type = _0.type;
            _r333 = _0.value;
            goto j333;
          }
        // end
        j334:;
        _get9 = jjtypeswitch_tmp1;
        // if 
          if (_get9 == 67) {
            _get10 = jjproto_target;
            _get11 = jjproto_targetjjtype;
            _get12 = i;
            const struct ReturnValue _1 = dong_porf_js_microbench__String_prototype_charCodeAt(_get10, _get11, _get12, 1);
            jjlast_type = _1.type;
            _r333 = _1.value;
            goto j333;
          }
        // end
        j335:;
        _get13 = jjtypeswitch_tmp1;
        // if 
          if (_get13 == 195) {
            _get14 = jjproto_target;
            _get15 = jjproto_targetjjtype;
            _get16 = i;
            const struct ReturnValue _2 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get14, _get15, _get16, 1);
            jjlast_type = _2.type;
            _r333 = _2.value;
            goto j333;
          }
        // end
        j336:;
        _r333 = 0;
      // end
      j333:;
      sign = _r333;
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
          j338:;
        }
      // end
      j337:;
    }
  // end
  j332:;
  _get21 = strict;
  logictmp = _get21;
  _get22 = logictmp;
  // if f64
  f64 _r339;
    if (((u32)(_get22)) != 0) {
      _get23 = i;
      _get24 = len;
      jjlast_type = 2;
      _r339 = (f64)(_get23 >= _get24);
    } else {
      _get25 = logictmp;
      jjlast_type = 2;
      _r339 = _get25;
    }
  // end
  j339:;
  jjlogicinner_tmp = _r339;
  _get26 = jjlast_type;
  jjtypeswitch_tmp1 = _get26;
  // block i32
  i32 _r340;
    _get27 = jjtypeswitch_tmp1;
    _get28 = jjtypeswitch_tmp1;
    // if 
      if (((_get27 == 67) | (_get28 == 195)) != 0) {
        _get29 = jjlogicinner_tmp;
        _r340 = i32_load(1, 0, (u32)(_get29));
        goto j340;
      }
    // end
    j341:;
    _get30 = jjlogicinner_tmp;
    _r340 = (u32)(_get30);
  // end
  j340:;
  // if 
    if ((_r340) != 0) {
      return NaN;
    }
  // end
  j342:;
  // loop 
  j343:;
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
        f64 _r345;
          _get36 = jjtypeswitch_tmp1;
          // if 
            if (_get36 == 33) {
              _get37 = jjproto_target;
              _get38 = jjproto_targetjjtype;
              _get39 = i;
              const struct ReturnValue _3 = dong_porf_js_microbench__String_prototype_charCodeAt(_get37, _get38, _get39, 1);
              jjlast_type = _3.type;
              _r345 = _3.value;
              goto j345;
            }
          // end
          j346:;
          _get40 = jjtypeswitch_tmp1;
          // if 
            if (_get40 == 67) {
              _get41 = jjproto_target;
              _get42 = jjproto_targetjjtype;
              _get43 = i;
              const struct ReturnValue _4 = dong_porf_js_microbench__String_prototype_charCodeAt(_get41, _get42, _get43, 1);
              jjlast_type = _4.type;
              _r345 = _4.value;
              goto j345;
            }
          // end
          j347:;
          _get44 = jjtypeswitch_tmp1;
          // if 
            if (_get44 == 195) {
              _get45 = jjproto_target;
              _get46 = jjproto_targetjjtype;
              _get47 = i;
              const struct ReturnValue _5 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get45, _get46, _get47, 1);
              jjlast_type = _5.type;
              _r345 = _5.value;
              goto j345;
            }
          // end
          j348:;
          _r345 = 0;
        // end
        j345:;
        chr = _r345;
        _get48 = chr;
        logictmpi = _get48 >= 48;
        _get49 = logictmpi;
        // if i32
        i32 _r349;
          if ((_get49) != 0) {
            _get50 = chr;
            jjlast_type = 2;
            _r349 = _get50 <= 57;
          } else {
            _get51 = logictmpi;
            jjlast_type = 2;
            _r349 = _get51;
          }
        // end
        j349:;
        // if 
          if ((_r349) != 0) {
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
            j351:;
            goto j344;
          }
        // end
        j350:;
        goto j343;
      }
    // end
    j344:;
  // end
  _get56 = hasDigit;
  // if 
    if (_get56 == 0) {
      return NaN;
    }
  // end
  j352:;
  _get57 = expNeg;
  // if 
    if (((u32)(_get57)) != 0) {
      _get58 = exp;
      return -_get58;
    }
  // end
  j353:;
  _get59 = exp;
  return _get59;
}

static f64 dong_porf_js_microbench__Number_isNaN(f64 l0) {
  f64 _get1;
  f64 _get0;
  _get0 = l0;
  _get1 = l0;
  return (f64)(_get0 != _get1);
}

static f64 dong_porf_js_microbench__Number_isInteger(f64 l0) {
  f64 _get1;
  f64 _get0;
  _get0 = l0;
  _get1 = l0;
  return (f64)(_get0 == trunc(_get1));
}

static f64 dong_porf_js_microbench__Number_isFinite(f64 l0) {
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

static f64 dong_porf_js_microbench__Math_abs(f64 l0) {
  f64 _get0;
  _get0 = l0;
  const f64 _tmp0 = _get0;
  return (_tmp0 < 0 ? -_tmp0 : _tmp0);
}

static f64 dong_porf_js_microbench__Math_floor(f64 l0) {
  f64 _get0;
  _get0 = l0;
  return floor(_get0);
}

static f64 dong_porf_js_microbench__Math_exp(f64 x, i32 xjjtype) {
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
    if (dong_porf_js_microbench__Number_isFinite(_get0) == 0) {
      _get1 = x;
      // if 
        if (_get1 == (-Infinity)) {
          return 0;
        }
      // end
      j383:;
      _get2 = x;
      return _get2;
    }
  // end
  j382:;
  _get3 = x;
  // if 
    if (_get3 < 0) {
      _get4 = x;
      return 1 / dong_porf_js_microbench__Math_exp(-_get4, 1);
    }
  // end
  j384:;
  _get5 = x;
  k = dong_porf_js_microbench__Math_floor(_get5 / 0.6931471805599453);
  _get6 = x;
  _get7 = k;
  r = _get6 - (_get7 * 0.6931471805599453);
  _get8 = r;
  term = _get8;
  _get9 = r;
  sum = 1 + _get9;
  i = 2;
  // loop 
  j385:;
    _get10 = term;
    // if 
      if (dong_porf_js_microbench__Math_abs(_get10) > 1e-15) {
        _get11 = term;
        _get12 = r;
        _get13 = i;
        term = _get11 * (_get12 / _get13);
        _get14 = sum;
        _get15 = term;
        sum = _get14 + _get15;
        _get16 = i;
        i = _get16 + 1;
        goto j385;
      }
    // end
    j386:;
  // end
  // loop 
  j387:;
    _get17 = k;
    _get18 = k;
    k = _get18 - 1;
    // if 
      if (_get17 > 0) {
        _get19 = sum;
        sum = _get19 * 2;
        goto j387;
      }
    // end
    j388:;
  // end
  _get20 = sum;
  return _get20;
}

static f64 dong_porf_js_microbench__Math_log2(f64 y, i32 yjjtype) {
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
  j397:;
  _get1 = y;
  // if 
    if (dong_porf_js_microbench__Number_isFinite(_get1) == 0) {
      _get2 = y;
      return _get2;
    }
  // end
  j398:;
  _get3 = y;
  x = _get3;
  exponent = 0;
  // loop 
  j399:;
    _get4 = x;
    // if 
      if (_get4 >= 2) {
        _get5 = x;
        x = _get5 / 2;
        _get6 = exponent;
        exponent = _get6 + 1;
        goto j399;
      }
    // end
    j400:;
  // end
  // loop 
  j401:;
    _get7 = x;
    // if 
      if (_get7 < 1) {
        _get8 = x;
        x = _get8 * 2;
        _get9 = exponent;
        exponent = _get9 - 1;
        goto j401;
      }
    // end
    j402:;
  // end
  _get10 = x;
  x = _get10 - 1;
  // loop 
  j403:;
    // block 
      _get11 = x;
      e_x = dong_porf_js_microbench__Math_exp(_get11 * 0.6931471805599453, 1);
      _get12 = e_x;
      _get13 = y;
      _get14 = e_x;
      delta = (_get12 - _get13) / (_get14 * 0.6931471805599453);
      _get15 = x;
      _get16 = delta;
      x = _get15 - _get16;
      _get17 = delta;
      if (dong_porf_js_microbench__Math_abs(_get17) > 1e-15) {
        goto j403;
      }
    // end
    j404:;
  // end
  _get18 = x;
  _get19 = exponent;
  return _get18 + _get19;
}

static f64 dong_porf_js_microbench__Math_log(f64 y, i32 yjjtype) {
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
      j394:;
      return NaN;
    }
  // end
  j393:;
  _get2 = y;
  // if 
    if (dong_porf_js_microbench__Number_isFinite(_get2) == 0) {
      _get3 = y;
      return _get3;
    }
  // end
  j395:;
  _get4 = y;
  // if 
    if (_get4 > 1e+308) {
      _get5 = y;
      n = dong_porf_js_microbench__Math_floor(dong_porf_js_microbench__Math_log2(_get5, 1));
      njjtype = 1;
      _get6 = n;
      _get7 = y;
      _get8 = n;
      return (0.6931471805599453 * _get6) + dong_porf_js_microbench__Math_log((_get7 / (f64)(1 << 30)) / (f64)(1 << (i32)((_get8 - 30))), 1);
    }
  // end
  j396:;
  m = 0;
  mjjtype = 1;
  // loop 
  j405:;
    _get9 = y;
    // if 
      if (_get9 >= 2) {
        _get10 = y;
        y = _get10 / 2;
        _get11 = m;
        m = _get11 + 1;
        goto j405;
      }
    // end
    j406:;
  // end
  // loop 
  j407:;
    _get12 = y;
    // if 
      if (_get12 < 1) {
        _get13 = y;
        y = _get13 * 2;
        _get14 = m;
        m = _get14 - 1;
        goto j407;
      }
    // end
    j408:;
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
  j409:;
    _get22 = term;
    // if 
      if (dong_porf_js_microbench__Math_abs(_get22) > 1e-15) {
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
        goto j409;
      }
    // end
    j410:;
  // end
  _get30 = sum;
  _get31 = m;
  return (2 * _get30) + (_get31 * 0.6931471805599453);
}

static f64 dong_porf_js_microbench__Math_pow(f64 base, i32 basejjtype, f64 exponent, i32 exponentjjtype) {
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
    if (((u32)(dong_porf_js_microbench__Number_isNaN(_get0))) != 0) {
      return NaN;
    }
  // end
  j356:;
  _get1 = exponent;
  // if 
    if (_get1 == 0) {
      return 1;
    }
  // end
  j357:;
  _get2 = base;
  // if 
    if (_get2 == 2) {
      _get3 = exponent;
      _get4 = exponent;
      _get5 = exponent;
      // if 
        if ((((u32)(dong_porf_js_microbench__Number_isInteger(_get3)) & (_get4 > 0)) & (_get5 < 31)) != 0) {
          _get6 = exponent;
          return (f64)(2 << (i32)((_get6 - 1)));
        }
      // end
      j359:;
    }
  // end
  j358:;
  _get7 = base;
  // if 
    if (dong_porf_js_microbench__Number_isFinite(_get7) == 0) {
      _get8 = base;
      // if 
        if (((u32)(dong_porf_js_microbench__Number_isNaN(_get8))) != 0) {
          _get9 = base;
          return _get9;
        }
      // end
      j361:;
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
          j363:;
          return 0;
        }
      // end
      j362:;
      _get13 = exponent;
      jjmath_a = dong_porf_js_microbench__Math_abs(_get13);
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
          j365:;
          return Infinity;
        }
      // end
      j364:;
      _get20 = isOdd;
      // if 
        if (((u32)(_get20)) != 0) {
          return 0;
        }
      // end
      j366:;
      return 0;
    }
  // end
  j360:;
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
          j369:;
          return Infinity;
        }
      // end
      j368:;
      _get24 = exponent;
      jjmath_a = dong_porf_js_microbench__Math_abs(_get24);
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
          j371:;
          return 0;
        }
      // end
      j370:;
      _get31 = isOdd;
      // if 
        if (((u32)(_get31)) != 0) {
          return -Infinity;
        }
      // end
      j372:;
      return Infinity;
    }
  // end
  j367:;
  _get32 = exponent;
  // if 
    if (_get32 == Infinity) {
      _get33 = base;
      abs = dong_porf_js_microbench__Math_abs(_get33);
      absjjtype = 1;
      _get34 = abs;
      // if 
        if (_get34 > 1) {
          return Infinity;
        }
      // end
      j374:;
      _get35 = abs;
      // if 
        if (_get35 == 1) {
          return NaN;
        }
      // end
      j375:;
      return 0;
    }
  // end
  j373:;
  _get36 = exponent;
  // if 
    if (_get36 == (-Infinity)) {
      _get37 = base;
      abs = dong_porf_js_microbench__Math_abs(_get37);
      absjjtype = 1;
      _get38 = abs;
      // if 
        if (_get38 > 1) {
          return 0;
        }
      // end
      j377:;
      _get39 = abs;
      // if 
        if (_get39 == 1) {
          return NaN;
        }
      // end
      j378:;
      return Infinity;
    }
  // end
  j376:;
  _get40 = base;
  // if 
    if (_get40 < 0) {
      _get41 = exponent;
      // if 
        if (dong_porf_js_microbench__Number_isInteger(_get41) == 0) {
          return NaN;
        }
      // end
      j380:;
    }
  // end
  j379:;
  _get42 = base;
  // if 
    if (_get42 == 2.718281828459045) {
      _get43 = exponent;
      return dong_porf_js_microbench__Math_exp(_get43, 1);
    }
  // end
  j381:;
  _get44 = base;
  currentBase = _get44;
  _get45 = exponent;
  currentExponent = dong_porf_js_microbench__Math_abs(_get45);
  result = 1;
  // loop 
  j389:;
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
            j392:;
            _get51 = currentBase;
            _get52 = currentBase;
            currentBase = _get51 * _get52;
            _get53 = currentExponent;
            currentExponent = dong_porf_js_microbench__Math_trunc(_get53 / 2);
          } else {
            _get54 = result;
            _get55 = currentExponent;
            _get56 = currentBase;
            result = _get54 * dong_porf_js_microbench__Math_exp(_get55 * dong_porf_js_microbench__Math_log(dong_porf_js_microbench__Math_abs(_get56), 1), 1);
            goto j390;
          }
        // end
        j391:;
        goto j389;
      }
    // end
    j390:;
  // end
  _get57 = exponent;
  // if f64
  f64 _r411;
    if (_get57 < 0) {
      _get58 = result;
      _r411 = 1 / _get58;
    } else {
      _get59 = result;
      _r411 = _get59;
    }
  // end
  j411:;
  return _r411;
}

static f64 dong_porf_js_microbench__Porffor_stn_float(f64 str, i32 strjjtype, f64 i, i32 ijjtype) {
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
  j318:;
  // loop 
  j319:;
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
        f64 _r321;
          _get8 = jjtypeswitch_tmp1;
          // if 
            if (_get8 == 33) {
              _get9 = jjproto_target;
              _get10 = jjproto_targetjjtype;
              _get11 = i;
              _get12 = i;
              i = _get12 + 1;
              const struct ReturnValue _0 = dong_porf_js_microbench__String_prototype_charCodeAt(_get9, _get10, _get11, 1);
              jjlast_type = _0.type;
              _r321 = _0.value;
              goto j321;
            }
          // end
          j322:;
          _get13 = jjtypeswitch_tmp1;
          // if 
            if (_get13 == 67) {
              _get14 = jjproto_target;
              _get15 = jjproto_targetjjtype;
              _get16 = i;
              _get17 = i;
              i = _get17 + 1;
              const struct ReturnValue _1 = dong_porf_js_microbench__String_prototype_charCodeAt(_get14, _get15, _get16, 1);
              jjlast_type = _1.type;
              _r321 = _1.value;
              goto j321;
            }
          // end
          j323:;
          _get18 = jjtypeswitch_tmp1;
          // if 
            if (_get18 == 195) {
              _get19 = jjproto_target;
              _get20 = jjproto_targetjjtype;
              _get21 = i;
              _get22 = i;
              i = _get22 + 1;
              const struct ReturnValue _2 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get19, _get20, _get21, 1);
              jjlast_type = _2.type;
              _r321 = _2.value;
              goto j321;
            }
          // end
          j324:;
          _r321 = 0;
        // end
        j321:;
        chr = _r321;
        _get23 = chr;
        logictmpi = _get23 >= 48;
        _get24 = logictmpi;
        // if i32
        i32 _r325;
          if ((_get24) != 0) {
            _get25 = chr;
            jjlast_type = 2;
            _r325 = _get25 <= 57;
          } else {
            _get26 = logictmpi;
            jjlast_type = 2;
            _r325 = _get26;
          }
        // end
        j325:;
        // if 
          if ((_r325) != 0) {
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
            j327:;
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
                j329:;
                dec = 1;
              } else {
                _get36 = chr;
                logictmpi = _get36 == 69;
                _get37 = logictmpi;
                // if i32
                i32 _r330;
                  if ((_get37) == 0) {
                    _get38 = chr;
                    jjlast_type = 2;
                    _r330 = _get38 == 101;
                  } else {
                    _get39 = logictmpi;
                    jjlast_type = 2;
                    _r330 = _get39;
                  }
                // end
                j330:;
                // if 
                  if ((_r330) != 0) {
                    _get40 = str;
                    _get41 = strjjtype;
                    _get42 = i;
                    _get43 = len;
                    exp = dong_porf_js_microbench__Porffor_parseExp(_get40, _get41, _get42, 1, _get43, 1, 1, 2);
                    _get44 = exp;
                    // if 
                      if (((u32)(dong_porf_js_microbench__Number_isNaN(_get44))) != 0) {
                        return NaN;
                      }
                    // end
                    j354:;
                    _get45 = exp;
                    // if 
                      if (_get45 < 0) {
                        _get46 = n;
                        _get47 = exp;
                        return _get46 / dong_porf_js_microbench__Math_pow(10, 1, -_get47, 1);
                      }
                    // end
                    j355:;
                    _get48 = n;
                    _get49 = exp;
                    return _get48 * dong_porf_js_microbench__Math_pow(10, 1, _get49, 1);
                  } else {
                    return NaN;
                  }
                // end
                j331:;
              }
            // end
            j328:;
          }
        // end
        j326:;
        goto j319;
      }
    // end
    j320:;
  // end
  _get50 = n;
  return _get50;
}

static f64 dong_porf_js_microbench__ecma262_StringToNumber(f64 str, i32 strjjtype) {
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
  f64 _r199;
    _get3 = jjtypeswitch_tmp1;
    // if 
      if (_get3 == 33) {
        _get4 = jjproto_target;
        _get5 = jjproto_targetjjtype;
        const struct ReturnValue _0 = dong_porf_js_microbench__String_prototype_trim((i32)(_get4), _get5);
        jjlast_type = _0.type;
        _r199 = (f64)(_0.value);
        goto j199;
      }
    // end
    j200:;
    _get6 = jjtypeswitch_tmp1;
    // if 
      if (_get6 == 67) {
        _get7 = jjproto_target;
        _get8 = jjproto_targetjjtype;
        const struct ReturnValue _1 = dong_porf_js_microbench__String_prototype_trim((i32)(_get7), _get8);
        jjlast_type = _1.type;
        _r199 = (f64)(_1.value);
        goto j199;
      }
    // end
    j218:;
    _get9 = jjtypeswitch_tmp1;
    // if 
      if (_get9 == 195) {
        _get10 = jjproto_target;
        _get11 = jjproto_targetjjtype;
        const struct ReturnValue _2 = dong_porf_js_microbench__ByteString_prototype_trim((i32)(_get10), _get11);
        jjlast_type = _2.type;
        _r199 = (f64)(_2.value);
        goto j199;
      }
    // end
    j219:;
    _r199 = 0;
  // end
  j199:;
  str = _r199;
  _get12 = jjlast_type;
  strjjtype = _get12;
  _get13 = str;
  // if 
    if ((f64)(i32_load(1, 0, (u32)(_get13))) == 0) {
      return 0;
    }
  // end
  j228:;
  _get14 = str;
  jjproto_target = _get14;
  _get15 = strjjtype;
  jjproto_targetjjtype = _get15;
  _get16 = strjjtype;
  jjtypeswitch_tmp1 = _get16;
  // block f64
  f64 _r229;
    _get17 = jjtypeswitch_tmp1;
    // if 
      if (_get17 == 33) {
        _get18 = jjproto_target;
        _get19 = jjproto_targetjjtype;
        const struct ReturnValue _3 = dong_porf_js_microbench__String_prototype_charCodeAt(_get18, _get19, 0, 1);
        jjlast_type = _3.type;
        _r229 = _3.value;
        goto j229;
      }
    // end
    j230:;
    _get20 = jjtypeswitch_tmp1;
    // if 
      if (_get20 == 67) {
        _get21 = jjproto_target;
        _get22 = jjproto_targetjjtype;
        const struct ReturnValue _4 = dong_porf_js_microbench__String_prototype_charCodeAt(_get21, _get22, 0, 1);
        jjlast_type = _4.type;
        _r229 = _4.value;
        goto j229;
      }
    // end
    j235:;
    _get23 = jjtypeswitch_tmp1;
    // if 
      if (_get23 == 195) {
        _get24 = jjproto_target;
        _get25 = jjproto_targetjjtype;
        const struct ReturnValue _5 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get24, _get25, 0, 1);
        jjlast_type = _5.type;
        _r229 = _5.value;
        goto j229;
      }
    // end
    j236:;
    _r229 = 0;
  // end
  j229:;
  first = _r229;
  _get26 = str;
  jjproto_target = _get26;
  _get27 = strjjtype;
  jjproto_targetjjtype = _get27;
  _get28 = strjjtype;
  jjtypeswitch_tmp1 = _get28;
  // block f64
  f64 _r238;
    _get29 = jjtypeswitch_tmp1;
    // if 
      if (_get29 == 33) {
        _get30 = jjproto_target;
        _get31 = jjproto_targetjjtype;
        const struct ReturnValue _6 = dong_porf_js_microbench__String_prototype_charCodeAt(_get30, _get31, 1, 1);
        jjlast_type = _6.type;
        _r238 = _6.value;
        goto j238;
      }
    // end
    j239:;
    _get32 = jjtypeswitch_tmp1;
    // if 
      if (_get32 == 67) {
        _get33 = jjproto_target;
        _get34 = jjproto_targetjjtype;
        const struct ReturnValue _7 = dong_porf_js_microbench__String_prototype_charCodeAt(_get33, _get34, 1, 1);
        jjlast_type = _7.type;
        _r238 = _7.value;
        goto j238;
      }
    // end
    j240:;
    _get35 = jjtypeswitch_tmp1;
    // if 
      if (_get35 == 195) {
        _get36 = jjproto_target;
        _get37 = jjproto_targetjjtype;
        const struct ReturnValue _8 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get36, _get37, 1, 1);
        jjlast_type = _8.type;
        _r238 = _8.value;
        goto j238;
      }
    // end
    j241:;
    _r238 = 0;
  // end
  j238:;
  second = _r238;
  _get38 = first;
  // if 
    if (_get38 == 48) {
      _get39 = second;
      logictmpi = _get39 == 120;
      _get40 = logictmpi;
      // if i32
      i32 _r243;
        if ((_get40) == 0) {
          _get41 = second;
          jjlast_type = 2;
          _r243 = _get41 == 88;
        } else {
          _get42 = logictmpi;
          jjlast_type = 2;
          _r243 = _get42;
        }
      // end
      j243:;
      // if 
        if ((_r243) != 0) {
          _get43 = str;
          _get44 = strjjtype;
          return dong_porf_js_microbench__Porffor_stn_int(_get43, _get44, 16, 1, 2, 1);
        }
      // end
      j244:;
      _get45 = second;
      logictmpi = _get45 == 111;
      _get46 = logictmpi;
      // if i32
      i32 _r260;
        if ((_get46) == 0) {
          _get47 = second;
          jjlast_type = 2;
          _r260 = _get47 == 79;
        } else {
          _get48 = logictmpi;
          jjlast_type = 2;
          _r260 = _get48;
        }
      // end
      j260:;
      // if 
        if ((_r260) != 0) {
          _get49 = str;
          _get50 = strjjtype;
          return dong_porf_js_microbench__Porffor_stn_int(_get49, _get50, 8, 1, 2, 1);
        }
      // end
      j261:;
      _get51 = second;
      logictmpi = _get51 == 98;
      _get52 = logictmpi;
      // if i32
      i32 _r262;
        if ((_get52) == 0) {
          _get53 = second;
          jjlast_type = 2;
          _r262 = _get53 == 66;
        } else {
          _get54 = logictmpi;
          jjlast_type = 2;
          _r262 = _get54;
        }
      // end
      j262:;
      // if 
        if ((_r262) != 0) {
          _get55 = str;
          _get56 = strjjtype;
          return dong_porf_js_microbench__Porffor_stn_int(_get55, _get56, 2, 1, 2, 1);
        }
      // end
      j263:;
    }
  // end
  j242:;
  i = 0;
  negative = 0;
  _get57 = first;
  // if 
    if (_get57 == 43) {
      i = 1;
    }
  // end
  j264:;
  _get58 = first;
  // if 
    if (_get58 == 45) {
      negative = 1;
      i = 1;
    }
  // end
  j265:;
  _get59 = i;
  _get60 = str;
  logictmpi = (_get59 + 8) == (f64)(i32_load(1, 0, (u32)(_get60)));
  _get61 = logictmpi;
  // if i32
  i32 _r266;
    if ((_get61) != 0) {
      _get62 = str;
      jjproto_target = _get62;
      _get63 = strjjtype;
      jjproto_targetjjtype = _get63;
      _get64 = strjjtype;
      jjtypeswitch_tmp1 = _get64;
      // block f64
      f64 _r267;
        _get65 = jjtypeswitch_tmp1;
        // if 
          if (_get65 == 33) {
            _get66 = jjproto_target;
            _get67 = jjproto_targetjjtype;
            _get68 = i;
            const struct ReturnValue _9 = dong_porf_js_microbench__String_prototype_charCodeAt(_get66, _get67, _get68, 1);
            jjlast_type = _9.type;
            _r267 = _9.value;
            goto j267;
          }
        // end
        j268:;
        _get69 = jjtypeswitch_tmp1;
        // if 
          if (_get69 == 67) {
            _get70 = jjproto_target;
            _get71 = jjproto_targetjjtype;
            _get72 = i;
            const struct ReturnValue _10 = dong_porf_js_microbench__String_prototype_charCodeAt(_get70, _get71, _get72, 1);
            jjlast_type = _10.type;
            _r267 = _10.value;
            goto j267;
          }
        // end
        j269:;
        _get73 = jjtypeswitch_tmp1;
        // if 
          if (_get73 == 195) {
            _get74 = jjproto_target;
            _get75 = jjproto_targetjjtype;
            _get76 = i;
            const struct ReturnValue _11 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get74, _get75, _get76, 1);
            jjlast_type = _11.type;
            _r267 = _11.value;
            goto j267;
          }
        // end
        j270:;
        _r267 = 0;
      // end
      j267:;
      jjlast_type = 2;
      _r266 = _r267 == 73;
    } else {
      _get77 = logictmpi;
      jjlast_type = 2;
      _r266 = _get77;
    }
  // end
  j266:;
  // if 
    if ((_r266) != 0) {
      _get78 = str;
      jjproto_target = _get78;
      _get79 = strjjtype;
      jjproto_targetjjtype = _get79;
      _get80 = strjjtype;
      jjtypeswitch_tmp1 = _get80;
      // block f64
      f64 _r272;
        _get81 = jjtypeswitch_tmp1;
        // if 
          if (_get81 == 33) {
            _get82 = jjproto_target;
            _get83 = jjproto_targetjjtype;
            _get84 = i;
            const struct ReturnValue _12 = dong_porf_js_microbench__String_prototype_charCodeAt(_get82, _get83, _get84 + 1, 1);
            jjlast_type = _12.type;
            _r272 = _12.value;
            goto j272;
          }
        // end
        j273:;
        _get85 = jjtypeswitch_tmp1;
        // if 
          if (_get85 == 67) {
            _get86 = jjproto_target;
            _get87 = jjproto_targetjjtype;
            _get88 = i;
            const struct ReturnValue _13 = dong_porf_js_microbench__String_prototype_charCodeAt(_get86, _get87, _get88 + 1, 1);
            jjlast_type = _13.type;
            _r272 = _13.value;
            goto j272;
          }
        // end
        j274:;
        _get89 = jjtypeswitch_tmp1;
        // if 
          if (_get89 == 195) {
            _get90 = jjproto_target;
            _get91 = jjproto_targetjjtype;
            _get92 = i;
            const struct ReturnValue _14 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get90, _get91, _get92 + 1, 1);
            jjlast_type = _14.type;
            _r272 = _14.value;
            goto j272;
          }
        // end
        j275:;
        _r272 = 0;
      // end
      j272:;
      logictmpi = _r272 == 110;
      _get93 = logictmpi;
      // if i32
      i32 _r276;
        if ((_get93) != 0) {
          _get94 = str;
          jjproto_target = _get94;
          _get95 = strjjtype;
          jjproto_targetjjtype = _get95;
          _get96 = strjjtype;
          jjtypeswitch_tmp1 = _get96;
          // block f64
          f64 _r277;
            _get97 = jjtypeswitch_tmp1;
            // if 
              if (_get97 == 33) {
                _get98 = jjproto_target;
                _get99 = jjproto_targetjjtype;
                _get100 = i;
                const struct ReturnValue _15 = dong_porf_js_microbench__String_prototype_charCodeAt(_get98, _get99, _get100 + 2, 1);
                jjlast_type = _15.type;
                _r277 = _15.value;
                goto j277;
              }
            // end
            j278:;
            _get101 = jjtypeswitch_tmp1;
            // if 
              if (_get101 == 67) {
                _get102 = jjproto_target;
                _get103 = jjproto_targetjjtype;
                _get104 = i;
                const struct ReturnValue _16 = dong_porf_js_microbench__String_prototype_charCodeAt(_get102, _get103, _get104 + 2, 1);
                jjlast_type = _16.type;
                _r277 = _16.value;
                goto j277;
              }
            // end
            j279:;
            _get105 = jjtypeswitch_tmp1;
            // if 
              if (_get105 == 195) {
                _get106 = jjproto_target;
                _get107 = jjproto_targetjjtype;
                _get108 = i;
                const struct ReturnValue _17 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get106, _get107, _get108 + 2, 1);
                jjlast_type = _17.type;
                _r277 = _17.value;
                goto j277;
              }
            // end
            j280:;
            _r277 = 0;
          // end
          j277:;
          jjlast_type = 2;
          _r276 = _r277 == 102;
        } else {
          _get109 = logictmpi;
          jjlast_type = 2;
          _r276 = _get109;
        }
      // end
      j276:;
      logictmpi = _r276;
      _get110 = logictmpi;
      jjlogicinner_tmp_int = _get110;
      _get111 = jjlast_type;
      jjtypeswitch_tmp1 = _get111;
      // block i32
      i32 _r281;
        _get112 = jjtypeswitch_tmp1;
        _get113 = jjtypeswitch_tmp1;
        // if 
          if (((_get112 == 67) | (_get113 == 195)) != 0) {
            _get114 = jjlogicinner_tmp_int;
            _r281 = i32_load(1, 0, _get114);
            goto j281;
          }
        // end
        j282:;
        _get115 = jjlogicinner_tmp_int;
        _r281 = _get115;
      // end
      j281:;
      // if i32
      i32 _r283;
        if ((_r281) != 0) {
          _get116 = str;
          jjproto_target = _get116;
          _get117 = strjjtype;
          jjproto_targetjjtype = _get117;
          _get118 = strjjtype;
          jjtypeswitch_tmp1 = _get118;
          // block f64
          f64 _r284;
            _get119 = jjtypeswitch_tmp1;
            // if 
              if (_get119 == 33) {
                _get120 = jjproto_target;
                _get121 = jjproto_targetjjtype;
                _get122 = i;
                const struct ReturnValue _18 = dong_porf_js_microbench__String_prototype_charCodeAt(_get120, _get121, _get122 + 3, 1);
                jjlast_type = _18.type;
                _r284 = _18.value;
                goto j284;
              }
            // end
            j285:;
            _get123 = jjtypeswitch_tmp1;
            // if 
              if (_get123 == 67) {
                _get124 = jjproto_target;
                _get125 = jjproto_targetjjtype;
                _get126 = i;
                const struct ReturnValue _19 = dong_porf_js_microbench__String_prototype_charCodeAt(_get124, _get125, _get126 + 3, 1);
                jjlast_type = _19.type;
                _r284 = _19.value;
                goto j284;
              }
            // end
            j286:;
            _get127 = jjtypeswitch_tmp1;
            // if 
              if (_get127 == 195) {
                _get128 = jjproto_target;
                _get129 = jjproto_targetjjtype;
                _get130 = i;
                const struct ReturnValue _20 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get128, _get129, _get130 + 3, 1);
                jjlast_type = _20.type;
                _r284 = _20.value;
                goto j284;
              }
            // end
            j287:;
            _r284 = 0;
          // end
          j284:;
          jjlast_type = 2;
          _r283 = _r284 == 105;
        } else {
          _get131 = logictmpi;
          _get132 = jjlast_type;
          jjlast_type = _get132;
          _r283 = _get131;
        }
      // end
      j283:;
      logictmpi = _r283;
      _get133 = logictmpi;
      jjlogicinner_tmp_int = _get133;
      _get134 = jjlast_type;
      jjtypeswitch_tmp1 = _get134;
      // block i32
      i32 _r288;
        _get135 = jjtypeswitch_tmp1;
        _get136 = jjtypeswitch_tmp1;
        // if 
          if (((_get135 == 67) | (_get136 == 195)) != 0) {
            _get137 = jjlogicinner_tmp_int;
            _r288 = i32_load(1, 0, _get137);
            goto j288;
          }
        // end
        j289:;
        _get138 = jjlogicinner_tmp_int;
        _r288 = _get138;
      // end
      j288:;
      // if i32
      i32 _r290;
        if ((_r288) != 0) {
          _get139 = str;
          jjproto_target = _get139;
          _get140 = strjjtype;
          jjproto_targetjjtype = _get140;
          _get141 = strjjtype;
          jjtypeswitch_tmp1 = _get141;
          // block f64
          f64 _r291;
            _get142 = jjtypeswitch_tmp1;
            // if 
              if (_get142 == 33) {
                _get143 = jjproto_target;
                _get144 = jjproto_targetjjtype;
                _get145 = i;
                const struct ReturnValue _21 = dong_porf_js_microbench__String_prototype_charCodeAt(_get143, _get144, _get145 + 4, 1);
                jjlast_type = _21.type;
                _r291 = _21.value;
                goto j291;
              }
            // end
            j292:;
            _get146 = jjtypeswitch_tmp1;
            // if 
              if (_get146 == 67) {
                _get147 = jjproto_target;
                _get148 = jjproto_targetjjtype;
                _get149 = i;
                const struct ReturnValue _22 = dong_porf_js_microbench__String_prototype_charCodeAt(_get147, _get148, _get149 + 4, 1);
                jjlast_type = _22.type;
                _r291 = _22.value;
                goto j291;
              }
            // end
            j293:;
            _get150 = jjtypeswitch_tmp1;
            // if 
              if (_get150 == 195) {
                _get151 = jjproto_target;
                _get152 = jjproto_targetjjtype;
                _get153 = i;
                const struct ReturnValue _23 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get151, _get152, _get153 + 4, 1);
                jjlast_type = _23.type;
                _r291 = _23.value;
                goto j291;
              }
            // end
            j294:;
            _r291 = 0;
          // end
          j291:;
          jjlast_type = 2;
          _r290 = _r291 == 110;
        } else {
          _get154 = logictmpi;
          _get155 = jjlast_type;
          jjlast_type = _get155;
          _r290 = _get154;
        }
      // end
      j290:;
      logictmpi = _r290;
      _get156 = logictmpi;
      jjlogicinner_tmp_int = _get156;
      _get157 = jjlast_type;
      jjtypeswitch_tmp1 = _get157;
      // block i32
      i32 _r295;
        _get158 = jjtypeswitch_tmp1;
        _get159 = jjtypeswitch_tmp1;
        // if 
          if (((_get158 == 67) | (_get159 == 195)) != 0) {
            _get160 = jjlogicinner_tmp_int;
            _r295 = i32_load(1, 0, _get160);
            goto j295;
          }
        // end
        j296:;
        _get161 = jjlogicinner_tmp_int;
        _r295 = _get161;
      // end
      j295:;
      // if i32
      i32 _r297;
        if ((_r295) != 0) {
          _get162 = str;
          jjproto_target = _get162;
          _get163 = strjjtype;
          jjproto_targetjjtype = _get163;
          _get164 = strjjtype;
          jjtypeswitch_tmp1 = _get164;
          // block f64
          f64 _r298;
            _get165 = jjtypeswitch_tmp1;
            // if 
              if (_get165 == 33) {
                _get166 = jjproto_target;
                _get167 = jjproto_targetjjtype;
                _get168 = i;
                const struct ReturnValue _24 = dong_porf_js_microbench__String_prototype_charCodeAt(_get166, _get167, _get168 + 5, 1);
                jjlast_type = _24.type;
                _r298 = _24.value;
                goto j298;
              }
            // end
            j299:;
            _get169 = jjtypeswitch_tmp1;
            // if 
              if (_get169 == 67) {
                _get170 = jjproto_target;
                _get171 = jjproto_targetjjtype;
                _get172 = i;
                const struct ReturnValue _25 = dong_porf_js_microbench__String_prototype_charCodeAt(_get170, _get171, _get172 + 5, 1);
                jjlast_type = _25.type;
                _r298 = _25.value;
                goto j298;
              }
            // end
            j300:;
            _get173 = jjtypeswitch_tmp1;
            // if 
              if (_get173 == 195) {
                _get174 = jjproto_target;
                _get175 = jjproto_targetjjtype;
                _get176 = i;
                const struct ReturnValue _26 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get174, _get175, _get176 + 5, 1);
                jjlast_type = _26.type;
                _r298 = _26.value;
                goto j298;
              }
            // end
            j301:;
            _r298 = 0;
          // end
          j298:;
          jjlast_type = 2;
          _r297 = _r298 == 105;
        } else {
          _get177 = logictmpi;
          _get178 = jjlast_type;
          jjlast_type = _get178;
          _r297 = _get177;
        }
      // end
      j297:;
      logictmpi = _r297;
      _get179 = logictmpi;
      jjlogicinner_tmp_int = _get179;
      _get180 = jjlast_type;
      jjtypeswitch_tmp1 = _get180;
      // block i32
      i32 _r302;
        _get181 = jjtypeswitch_tmp1;
        _get182 = jjtypeswitch_tmp1;
        // if 
          if (((_get181 == 67) | (_get182 == 195)) != 0) {
            _get183 = jjlogicinner_tmp_int;
            _r302 = i32_load(1, 0, _get183);
            goto j302;
          }
        // end
        j303:;
        _get184 = jjlogicinner_tmp_int;
        _r302 = _get184;
      // end
      j302:;
      // if i32
      i32 _r304;
        if ((_r302) != 0) {
          _get185 = str;
          jjproto_target = _get185;
          _get186 = strjjtype;
          jjproto_targetjjtype = _get186;
          _get187 = strjjtype;
          jjtypeswitch_tmp1 = _get187;
          // block f64
          f64 _r305;
            _get188 = jjtypeswitch_tmp1;
            // if 
              if (_get188 == 33) {
                _get189 = jjproto_target;
                _get190 = jjproto_targetjjtype;
                _get191 = i;
                const struct ReturnValue _27 = dong_porf_js_microbench__String_prototype_charCodeAt(_get189, _get190, _get191 + 6, 1);
                jjlast_type = _27.type;
                _r305 = _27.value;
                goto j305;
              }
            // end
            j306:;
            _get192 = jjtypeswitch_tmp1;
            // if 
              if (_get192 == 67) {
                _get193 = jjproto_target;
                _get194 = jjproto_targetjjtype;
                _get195 = i;
                const struct ReturnValue _28 = dong_porf_js_microbench__String_prototype_charCodeAt(_get193, _get194, _get195 + 6, 1);
                jjlast_type = _28.type;
                _r305 = _28.value;
                goto j305;
              }
            // end
            j307:;
            _get196 = jjtypeswitch_tmp1;
            // if 
              if (_get196 == 195) {
                _get197 = jjproto_target;
                _get198 = jjproto_targetjjtype;
                _get199 = i;
                const struct ReturnValue _29 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get197, _get198, _get199 + 6, 1);
                jjlast_type = _29.type;
                _r305 = _29.value;
                goto j305;
              }
            // end
            j308:;
            _r305 = 0;
          // end
          j305:;
          jjlast_type = 2;
          _r304 = _r305 == 116;
        } else {
          _get200 = logictmpi;
          _get201 = jjlast_type;
          jjlast_type = _get201;
          _r304 = _get200;
        }
      // end
      j304:;
      logictmpi = _r304;
      _get202 = logictmpi;
      jjlogicinner_tmp_int = _get202;
      _get203 = jjlast_type;
      jjtypeswitch_tmp1 = _get203;
      // block i32
      i32 _r309;
        _get204 = jjtypeswitch_tmp1;
        _get205 = jjtypeswitch_tmp1;
        // if 
          if (((_get204 == 67) | (_get205 == 195)) != 0) {
            _get206 = jjlogicinner_tmp_int;
            _r309 = i32_load(1, 0, _get206);
            goto j309;
          }
        // end
        j310:;
        _get207 = jjlogicinner_tmp_int;
        _r309 = _get207;
      // end
      j309:;
      // if i32
      i32 _r311;
        if ((_r309) != 0) {
          _get208 = str;
          jjproto_target = _get208;
          _get209 = strjjtype;
          jjproto_targetjjtype = _get209;
          _get210 = strjjtype;
          jjtypeswitch_tmp1 = _get210;
          // block f64
          f64 _r312;
            _get211 = jjtypeswitch_tmp1;
            // if 
              if (_get211 == 33) {
                _get212 = jjproto_target;
                _get213 = jjproto_targetjjtype;
                _get214 = i;
                const struct ReturnValue _30 = dong_porf_js_microbench__String_prototype_charCodeAt(_get212, _get213, _get214 + 7, 1);
                jjlast_type = _30.type;
                _r312 = _30.value;
                goto j312;
              }
            // end
            j313:;
            _get215 = jjtypeswitch_tmp1;
            // if 
              if (_get215 == 67) {
                _get216 = jjproto_target;
                _get217 = jjproto_targetjjtype;
                _get218 = i;
                const struct ReturnValue _31 = dong_porf_js_microbench__String_prototype_charCodeAt(_get216, _get217, _get218 + 7, 1);
                jjlast_type = _31.type;
                _r312 = _31.value;
                goto j312;
              }
            // end
            j314:;
            _get219 = jjtypeswitch_tmp1;
            // if 
              if (_get219 == 195) {
                _get220 = jjproto_target;
                _get221 = jjproto_targetjjtype;
                _get222 = i;
                const struct ReturnValue _32 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get220, _get221, _get222 + 7, 1);
                jjlast_type = _32.type;
                _r312 = _32.value;
                goto j312;
              }
            // end
            j315:;
            _r312 = 0;
          // end
          j312:;
          jjlast_type = 2;
          _r311 = _r312 == 121;
        } else {
          _get223 = logictmpi;
          _get224 = jjlast_type;
          jjlast_type = _get224;
          _r311 = _get223;
        }
      // end
      j311:;
      // if 
        if ((_r311) != 0) {
          n = Infinity;
          _get225 = negative;
          // if f64
          f64 _r317;
            if (((u32)(_get225)) != 0) {
              _get226 = n;
              jjlast_type = 1;
              _r317 = -_get226;
            } else {
              _get227 = n;
              jjlast_type = 1;
              _r317 = _get227;
            }
          // end
          j317:;
          return _r317;
        }
      // end
      j316:;
      return NaN;
    }
  // end
  j271:;
  _get228 = str;
  _get229 = strjjtype;
  _get230 = i;
  n = dong_porf_js_microbench__Porffor_stn_float(_get228, _get229, _get230, 1);
  _get231 = negative;
  // if 
    if (((u32)(_get231)) != 0) {
      _get232 = n;
      return -_get232;
    }
  // end
  j412:;
  _get233 = n;
  return _get233;
}

static struct ReturnValue dong_porf_js_microbench__Number_prototype_valueOf(f64 _this, i32 _thisjjtype) {
  f64 _get2;
  i32 _get1;
  i32 _get0;
  _get0 = _thisjjtype;
  _get1 = _thisjjtype;
  // if 
    if (((_get0 != 1) & (_get1 != 32)) != 0) {
    }
  // end
  j416:;
  _get2 = _this;
  return (struct ReturnValue){ _get2, 1 };
}

static struct ReturnValue dong_porf_js_microbench__Boolean_prototype_valueOf(f64 _this, i32 _thisjjtype) {
  f64 _get0;
  _get0 = _this;
  return (struct ReturnValue){ _get0, 2 };
}

static struct ReturnValue dong_porf_js_microbench__Object_prototype_valueOf(f64 _this, i32 _thisjjtype) {
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
          jjmember_prop_257 = 845;
          _get3 = obj;
          jjmember_obj_257 = _get3;
          _get4 = jjmember_obj_257;
          _get5 = jjmember_prop_257;
          const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_get_withHash((i32)(_get4), 7, (u32)(_get5), 195, -238740424, 1);
          jjlast_type = _0.type;
          _get6 = jjlast_type;
          ovrjjtype = _get6;
          ovr = _0.value;
          _get7 = ovrjjtype;
          logictmpi = (f64)(_get7) == 6;
          _get8 = logictmpi;
          // if i32
          i32 _r421;
            if ((_get8) != 0) {
              _get9 = ovr;
              jjlast_type = 2;
              _r421 = _get9 != 2;
            } else {
              _get10 = logictmpi;
              jjlast_type = 2;
              _r421 = _get10;
            }
          // end
          j421:;
          // if 
            if ((_r421) != 0) {
              _get11 = ovr;
              jjindirect_258_callee = _get11;
              _get12 = ovrjjtype;
              // if f64
              f64 _r423;
                if (_get12 == 6) {
                  _get13 = _this;
                  jjcall_val = _get13;
                  _get14 = jjcall_val;
                  _get15 = _thisjjtype;
                  jjcall_type = _get15;
                  _get16 = jjcall_type;
                  _get17 = jjindirect_258_callee;
                  jjlast_type = 0;
                  _r423 = 0;
                } else {
                  _r423 = 0;
                }
              // end
              j423:;
              _get18 = jjlast_type;
              return (struct ReturnValue){ _r423, _get18 };
              (void) 0;
            }
          // end
          j422:;
          _get19 = obj;
          entryPtr = (f64)(dong_porf_js_microbench__Porffor_object_lookup((i32)(_get19), 7, 845, 195, dong_porf_js_microbench__Porffor_object_hash(845, 195), 1));
          _get20 = entryPtr;
          // if 
            if (_get20 != -1) {
              _get21 = entryPtr;
              const struct ReturnValue _1 = dong_porf_js_microbench__Porffor_object_readValue((i32)(_get21), 1);
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
                  f64 _r426;
                    if (_get25 == 6) {
                      _get26 = _this;
                      jjcall_val = _get26;
                      _get27 = jjcall_val;
                      _get28 = _thisjjtype;
                      jjcall_type = _get28;
                      _get29 = jjcall_type;
                      _get30 = jjindirect_259_callee;
                      jjlast_type = 0;
                      _r426 = 0;
                    } else {
                      _r426 = 0;
                    }
                  // end
                  j426:;
                  _get31 = jjlast_type;
                  return (struct ReturnValue){ _r426, _get31 };
                  (void) 0;
                } else {
                  return (struct ReturnValue){ 0, 0 };
                  (void) 0;
                }
              // end
              j425:;
            }
          // end
          j424:;
        }
      // end
      j420:;
    }
  // end
  j419:;
  _get32 = _this;
  _get33 = _thisjjtype;
  return (struct ReturnValue){ _get32, _get33 };
}

static struct ReturnValue dong_porf_js_microbench__String_prototype_valueOf(i32 _this, i32 _thisjjtype) {
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
      j431:;
      _get4 = _this;
      _get5 = _thisjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString((f64)(_get4), _get5);
      _thisjjtype = _0.type;
      _this = (i32)(_0.value);
      _get6 = _thisjjtype;
      // if 
        if (_get6 == 195) {
          _get7 = _this;
          _this = dong_porf_js_microbench__Porffor_bytestringToString(_get7);
        }
      // end
      j432:;
    }
  // end
  j430:;
  _get8 = _this;
  return (struct ReturnValue){ _get8, 67 };
}

static struct ReturnValue dong_porf_js_microbench__Array_prototype_valueOf(f64 _this, i32 _thisjjtype) {
  f64 _get3;
  i32 _get2;
  f64 _get1;
  i32 _get0;
  _get0 = _thisjjtype;
  // if 
    if (_get0 != 72) {
      _get1 = _this;
      _get2 = _thisjjtype;
      _this = dong_porf_js_microbench__Array_from(_get1, _get2, 0, 0, 0, 0);
      _thisjjtype = 72;
    }
  // end
  j435:;
  _get3 = _this;
  return (struct ReturnValue){ _get3, 72 };
}

static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_valueOf(i32 _this, i32 _thisjjtype) {
  i32 _get0;
  _get0 = _this;
  return (struct ReturnValue){ _get0, 195 };
}

static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_toString(i32 _this, i32 _thisjjtype) {
  i32 _get0;
  _get0 = _this;
  return (struct ReturnValue){ _get0, 195 };
}

static struct ReturnValue dong_porf_js_microbench__ecma262_ToPrimitive_Number(f64 input, i32 inputjjtype) {
  i32 _get101;
  f64 _get100;
  i32 _get99;
  i32 _get98;
  f64 _get97;
  i32 _get96;
  f64 _get95;
  i32 _get94;
  i32 _get93;
  i32 _get92;
  f64 _get91;
  i32 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  f64 _get86;
  i32 _get85;
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
  f64 _r413;
    _get3 = jjtypeswitch_tmp1;
    // if 
      if (_get3 == 0) {
        jjlast_type = 0;
        _r413 = 0;
        goto j413;
      }
    // end
    j414:;
    _get4 = jjtypeswitch_tmp1;
    // if 
      if (_get4 == 1) {
        _get5 = jjproto_target;
        _get6 = jjproto_targetjjtype;
        const struct ReturnValue _0 = dong_porf_js_microbench__Number_prototype_valueOf(_get5, _get6);
        jjlast_type = _0.type;
        _r413 = _0.value;
        goto j413;
      }
    // end
    j415:;
    _get7 = jjtypeswitch_tmp1;
    // if 
      if (_get7 == 2) {
        _get8 = jjproto_target;
        _get9 = jjproto_targetjjtype;
        const struct ReturnValue _1 = dong_porf_js_microbench__Boolean_prototype_valueOf(_get8, _get9);
        jjlast_type = _1.type;
        _r413 = _1.value;
        goto j413;
      }
    // end
    j417:;
    _get10 = jjtypeswitch_tmp1;
    // if 
      if (_get10 == 7) {
        _get11 = jjproto_target;
        _get12 = jjproto_targetjjtype;
        const struct ReturnValue _2 = dong_porf_js_microbench__Object_prototype_valueOf(_get11, _get12);
        jjlast_type = _2.type;
        _r413 = _2.value;
        goto j413;
      }
    // end
    j418:;
    _get13 = jjtypeswitch_tmp1;
    // if 
      if (_get13 == 31) {
        _get14 = jjproto_target;
        _get15 = jjproto_targetjjtype;
        const struct ReturnValue _3 = dong_porf_js_microbench__Boolean_prototype_valueOf(_get14, _get15);
        jjlast_type = _3.type;
        _r413 = _3.value;
        goto j413;
      }
    // end
    j427:;
    _get16 = jjtypeswitch_tmp1;
    // if 
      if (_get16 == 32) {
        _get17 = jjproto_target;
        _get18 = jjproto_targetjjtype;
        const struct ReturnValue _4 = dong_porf_js_microbench__Number_prototype_valueOf(_get17, _get18);
        jjlast_type = _4.type;
        _r413 = _4.value;
        goto j413;
      }
    // end
    j428:;
    _get19 = jjtypeswitch_tmp1;
    // if 
      if (_get19 == 33) {
        _get20 = jjproto_target;
        _get21 = jjproto_targetjjtype;
        const struct ReturnValue _5 = dong_porf_js_microbench__String_prototype_valueOf((i32)(_get20), _get21);
        jjlast_type = _5.type;
        _r413 = (f64)(_5.value);
        goto j413;
      }
    // end
    j429:;
    _get22 = jjtypeswitch_tmp1;
    // if 
      if (_get22 == 67) {
        _get23 = jjproto_target;
        _get24 = jjproto_targetjjtype;
        const struct ReturnValue _6 = dong_porf_js_microbench__String_prototype_valueOf((i32)(_get23), _get24);
        jjlast_type = _6.type;
        _r413 = (f64)(_6.value);
        goto j413;
      }
    // end
    j433:;
    _get25 = jjtypeswitch_tmp1;
    // if 
      if (_get25 == 72) {
        _get26 = jjproto_target;
        _get27 = jjproto_targetjjtype;
        const struct ReturnValue _7 = dong_porf_js_microbench__Array_prototype_valueOf(_get26, _get27);
        jjlast_type = _7.type;
        _r413 = _7.value;
        goto j413;
      }
    // end
    j434:;
    _get28 = jjtypeswitch_tmp1;
    // if 
      if (_get28 == 195) {
        _get29 = jjproto_target;
        _get30 = jjproto_targetjjtype;
        const struct ReturnValue _8 = dong_porf_js_microbench__ByteString_prototype_valueOf((i32)(_get29), _get30);
        jjlast_type = _8.type;
        _r413 = (f64)(_8.value);
        goto j413;
      }
    // end
    j436:;
    _get31 = jjproto_target;
    _get32 = jjproto_targetjjtype;
    const struct ReturnValue _9 = dong_porf_js_microbench__Object_prototype_valueOf(_get31, _get32);
    jjlast_type = _9.type;
    _r413 = _9.value;
  // end
  j413:;
  value = _r413;
  _get33 = jjlast_type;
  valuejjtype = _get33;
  _get34 = value;
  jjlogicinner_tmp = _get34;
  _get35 = valuejjtype;
  jjtypeswitch_tmp1 = _get35;
  // block i32
  i32 _r437;
    _get36 = jjtypeswitch_tmp1;
    // if 
      if (_get36 == 0) {
        _r437 = 1;
        goto j437;
      }
    // end
    j438:;
    _get37 = jjtypeswitch_tmp1;
    // if 
      if (_get37 == 7) {
        _get38 = jjlogicinner_tmp;
        _r437 = _get38 == 0;
        goto j437;
      }
    // end
    j439:;
    _r437 = 0;
  // end
  j437:;
  logictmpi = (_r437) == 0;
  _get39 = logictmpi;
  // if i32
  i32 _r440;
    if ((_get39) != 0) {
      _get40 = value;
      _get41 = valuejjtype;
      jjlast_type = 2;
      _r440 = (f64)(dong_porf_js_microbench__Porffor_object_isObjectOrNull((i32)(_get40), _get41)) == 0;
    } else {
      _get42 = logictmpi;
      jjlast_type = 2;
      _r440 = _get42;
    }
  // end
  j440:;
  // if 
    if ((_r440) != 0) {
      _get43 = value;
      _get44 = valuejjtype;
      return (struct ReturnValue){ _get43, _get44 };
    }
  // end
  j441:;
  _get45 = input;
  jjproto_target = _get45;
  _get46 = inputjjtype;
  jjproto_targetjjtype = _get46;
  _get47 = inputjjtype;
  jjtypeswitch_tmp1 = _get47;
  // block f64
  f64 _r442;
    _get48 = jjtypeswitch_tmp1;
    // if 
      if (_get48 == 0) {
        jjlast_type = 0;
        _r442 = 0;
        goto j442;
      }
    // end
    j443:;
    _get49 = jjtypeswitch_tmp1;
    // if 
      if (_get49 == 1) {
        _get50 = jjproto_target;
        _get51 = jjproto_targetjjtype;
        const struct ReturnValue _10 = dong_porf_js_microbench__Number_prototype_toString(_get50, _get51, 0, 0);
        jjlast_type = _10.type;
        _r442 = _10.value;
        goto j442;
      }
    // end
    j444:;
    _get52 = jjtypeswitch_tmp1;
    // if 
      if (_get52 == 2) {
        _get53 = jjproto_target;
        _get54 = jjproto_targetjjtype;
        const struct ReturnValue _11 = dong_porf_js_microbench__Boolean_prototype_toString(_get53, _get54);
        jjlast_type = _11.type;
        _r442 = _11.value;
        goto j442;
      }
    // end
    j445:;
    _get55 = jjtypeswitch_tmp1;
    // if 
      if (_get55 == 6) {
        _get56 = jjproto_target;
        _get57 = jjproto_targetjjtype;
        const struct ReturnValue _12 = dong_porf_js_microbench__Function_prototype_toString(_get56, _get57);
        jjlast_type = _12.type;
        _r442 = _12.value;
        goto j442;
      }
    // end
    j446:;
    _get58 = jjtypeswitch_tmp1;
    // if 
      if (_get58 == 7) {
        _get59 = jjproto_target;
        _get60 = jjproto_targetjjtype;
        const struct ReturnValue _13 = dong_porf_js_microbench__Object_prototype_toString(_get59, _get60);
        jjlast_type = _13.type;
        _r442 = _13.value;
        goto j442;
      }
    // end
    j447:;
    _get61 = jjtypeswitch_tmp1;
    // if 
      if (_get61 == 31) {
        _get62 = jjproto_target;
        _get63 = jjproto_targetjjtype;
        const struct ReturnValue _14 = dong_porf_js_microbench__Boolean_prototype_toString(_get62, _get63);
        jjlast_type = _14.type;
        _r442 = _14.value;
        goto j442;
      }
    // end
    j448:;
    _get64 = jjtypeswitch_tmp1;
    // if 
      if (_get64 == 32) {
        _get65 = jjproto_target;
        _get66 = jjproto_targetjjtype;
        const struct ReturnValue _15 = dong_porf_js_microbench__Number_prototype_toString(_get65, _get66, 0, 0);
        jjlast_type = _15.type;
        _r442 = _15.value;
        goto j442;
      }
    // end
    j449:;
    _get67 = jjtypeswitch_tmp1;
    // if 
      if (_get67 == 33) {
        _get68 = jjproto_target;
        _get69 = jjproto_targetjjtype;
        const struct ReturnValue _16 = dong_porf_js_microbench__String_prototype_toString((i32)(_get68), _get69);
        jjlast_type = _16.type;
        _r442 = (f64)(_16.value);
        goto j442;
      }
    // end
    j450:;
    _get70 = jjtypeswitch_tmp1;
    // if 
      if (_get70 == 38) {
        _get71 = jjproto_target;
        _get72 = jjproto_targetjjtype;
        const struct ReturnValue _17 = dong_porf_js_microbench__TypeError_prototype_toString(_get71, _get72);
        jjlast_type = _17.type;
        _r442 = _17.value;
        goto j442;
      }
    // end
    j451:;
    _get73 = jjtypeswitch_tmp1;
    // if 
      if (_get73 == 40) {
        _get74 = jjproto_target;
        _get75 = jjproto_targetjjtype;
        const struct ReturnValue _18 = dong_porf_js_microbench__SyntaxError_prototype_toString(_get74, _get75);
        jjlast_type = _18.type;
        _r442 = _18.value;
        goto j442;
      }
    // end
    j452:;
    _get76 = jjtypeswitch_tmp1;
    // if 
      if (_get76 == 41) {
        _get77 = jjproto_target;
        _get78 = jjproto_targetjjtype;
        const struct ReturnValue _19 = dong_porf_js_microbench__RangeError_prototype_toString(_get77, _get78);
        jjlast_type = _19.type;
        _r442 = _19.value;
        goto j442;
      }
    // end
    j453:;
    _get79 = jjtypeswitch_tmp1;
    // if 
      if (_get79 == 67) {
        _get80 = jjproto_target;
        _get81 = jjproto_targetjjtype;
        const struct ReturnValue _20 = dong_porf_js_microbench__String_prototype_toString((i32)(_get80), _get81);
        jjlast_type = _20.type;
        _r442 = (f64)(_20.value);
        goto j442;
      }
    // end
    j454:;
    _get82 = jjtypeswitch_tmp1;
    // if 
      if (_get82 == 72) {
        _get83 = jjproto_target;
        _get84 = jjproto_targetjjtype;
        const struct ReturnValue _21 = dong_porf_js_microbench__Array_prototype_toString(_get83, _get84);
        jjlast_type = _21.type;
        _r442 = _21.value;
        goto j442;
      }
    // end
    j455:;
    _get85 = jjtypeswitch_tmp1;
    // if 
      if (_get85 == 195) {
        _get86 = jjproto_target;
        _get87 = jjproto_targetjjtype;
        const struct ReturnValue _22 = dong_porf_js_microbench__ByteString_prototype_toString((i32)(_get86), _get87);
        jjlast_type = _22.type;
        _r442 = (f64)(_22.value);
        goto j442;
      }
    // end
    j456:;
    _get88 = jjproto_target;
    _get89 = jjproto_targetjjtype;
    const struct ReturnValue _23 = dong_porf_js_microbench__Object_prototype_toString(_get88, _get89);
    jjlast_type = _23.type;
    _r442 = _23.value;
  // end
  j442:;
  value = _r442;
  _get90 = jjlast_type;
  valuejjtype = _get90;
  _get91 = value;
  jjlogicinner_tmp = _get91;
  _get92 = valuejjtype;
  jjtypeswitch_tmp1 = _get92;
  // block i32
  i32 _r457;
    _get93 = jjtypeswitch_tmp1;
    // if 
      if (_get93 == 0) {
        _r457 = 1;
        goto j457;
      }
    // end
    j458:;
    _get94 = jjtypeswitch_tmp1;
    // if 
      if (_get94 == 7) {
        _get95 = jjlogicinner_tmp;
        _r457 = _get95 == 0;
        goto j457;
      }
    // end
    j459:;
    _r457 = 0;
  // end
  j457:;
  logictmpi = (_r457) == 0;
  _get96 = logictmpi;
  // if i32
  i32 _r460;
    if ((_get96) != 0) {
      _get97 = value;
      _get98 = valuejjtype;
      jjlast_type = 2;
      _r460 = (f64)(dong_porf_js_microbench__Porffor_object_isObjectOrNull((i32)(_get97), _get98)) == 0;
    } else {
      _get99 = logictmpi;
      jjlast_type = 2;
      _r460 = _get99;
    }
  // end
  j460:;
  // if 
    if ((_r460) != 0) {
      _get100 = value;
      _get101 = valuejjtype;
      return (struct ReturnValue){ _get100, _get101 };
    }
  // end
  j461:;
  return (struct ReturnValue){ 0, 0 };
}

static f64 dong_porf_js_microbench__ecma262_ToNumber(f64 argument, i32 argumentjjtype) {
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
  j193:;
  _get2 = argumentjjtype;
  _get3 = argumentjjtype;
  // if 
    if ((((f64)(_get2) == 5) | ((f64)(_get3) == 4)) != 0) {
    }
  // end
  j194:;
  _get4 = argumentjjtype;
  // if 
    if ((f64)(_get4) == 0) {
      return NaN;
    }
  // end
  j195:;
  _get5 = argument;
  _get6 = argumentjjtype;
  _get7 = argument;
  _get8 = argumentjjtype;
  // if 
    if ((((_get5 == 0) & ((_get6 | 128) == (7 | 128))) | ((_get7 == 0) & ((_get8 | 128) == (2 | 128)))) != 0) {
      return 0;
    }
  // end
  j196:;
  _get9 = argument;
  _get10 = argumentjjtype;
  // if 
    if (((_get9 == 1) & ((_get10 | 128) == (2 | 128))) != 0) {
      return 1;
    }
  // end
  j197:;
  _get11 = argumentjjtype;
  // if 
    if ((f64)(_get11 | 128) == 195) {
      _get12 = argument;
      _get13 = argumentjjtype;
      return dong_porf_js_microbench__ecma262_StringToNumber(_get12, _get13);
    }
  // end
  j198:;
  _get14 = argument;
  _get15 = argumentjjtype;
  const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToPrimitive_Number(_get14, _get15);
  jjlast_type = _0.type;
  _get16 = jjlast_type;
  primValuejjtype = _get16;
  primValue = _0.value;
  _get17 = primValue;
  _get18 = primValuejjtype;
  return dong_porf_js_microbench__ecma262_ToNumber(_get17, _get18);
}

static f64 dong_porf_js_microbench__ecma262_ToIntegerOrInfinity(f64 argument, i32 argumentjjtype) {
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
  number = dong_porf_js_microbench__ecma262_ToNumber(_get0, _get1);
  _get2 = number;
  // if 
    if (((u32)(dong_porf_js_microbench__Number_isNaN(_get2))) != 0) {
      return 0;
    }
  // end
  j462:;
  _get3 = number;
  // if 
    if (dong_porf_js_microbench__Number_isFinite(_get3) == 0) {
      _get4 = number;
      return _get4;
    }
  // end
  j463:;
  _get5 = number;
  number = dong_porf_js_microbench__Math_trunc(_get5);
  _get6 = number;
  // if 
    if (_get6 == 0) {
      return 0;
    }
  // end
  j464:;
  _get7 = number;
  return _get7;
}

static f64 dong_porf_js_microbench_RangeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype) {
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
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString(_get2, _get3);
      jjlast_type = _0.type;
      _get4 = jjlast_type;
      messagejjtype = _get4;
      message = _0.value;
    }
  // end
  j466:;
  obj = (f64)(dong_porf_js_microbench__Porffor_malloc(8));
  _get5 = obj;
  _get6 = message;
  i32_store(0, 0, (i32)(_get5), (i32)(_get6));
  _get7 = obj;
  _get8 = messagejjtype;
  i32_store8(0, 4, (i32)(_get7), _get8);
  _get9 = obj;
  return _get9;
}

static f64 dong_porf_js_microbench__Array_from(f64 arg, i32 argjjtype, f64 mapFn, i32 mapFnjjtype, f64 thisArg, i32 thisArgjjtype) {
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
  j113:;
  _get1 = arg;
  jjlogicinner_tmp = _get1;
  _get2 = argjjtype;
  jjtypeswitch_tmp1 = _get2;
  // block i32
  i32 _r114;
    _get3 = jjtypeswitch_tmp1;
    // if 
      if (_get3 == 0) {
        _r114 = 1;
        goto j114;
      }
    // end
    j115:;
    _get4 = jjtypeswitch_tmp1;
    // if 
      if (_get4 == 7) {
        _get5 = jjlogicinner_tmp;
        _r114 = _get5 == 0;
        goto j114;
      }
    // end
    j116:;
    _r114 = 0;
  // end
  j114:;
  // if 
    if ((_r114) != 0) {
    }
  // end
  j117:;
  out = (f64)(dong_porf_js_microbench__Porffor_malloc(16384));
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
          j120:;
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
          j121:;
          _get23 = jjforof_base_pointer0;
          jjforof_length0 = i32_load(1, 0, _get23);
          // loop 
          j122:;
            // block 
              _get24 = jjforof_itertype0;
              jjtypeswitch_tmp1 = _get24;
              // block f64
              f64 _r124;
                _get25 = jjtypeswitch_tmp1;
                _get26 = jjtypeswitch_tmp1;
                // if 
                  if (((_get25 == 72) | (_get26 == 11)) != 0) {
                    _get27 = jjforof_length0;
                    if ((_get27) == 0) {
                      goto j123;
                    }
                    _get28 = jjforof_base_pointer0;
                    _get29 = jjforof_base_pointer0;
                    _get30 = jjforof_base_pointer0;
                    jjforof_base_pointer0 = _get30 + 9;
                    _get31 = jjforof_length0;
                    jjforof_length0 = _get31 - 1;
                    jjlast_type = i32_load8_u(0, 12, _get29);
                    _r124 = f64_load(0, 4, _get28);
                    goto j124;
                  }
                // end
                j125:;
                _get32 = jjtypeswitch_tmp1;
                // if 
                  if (_get32 == 67) {
                    _get33 = jjforof_length0;
                    if ((_get33) == 0) {
                      goto j123;
                    }
                    jjforof_allocd = dong_porf_js_microbench__Porffor_malloc(8);
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
                    _r124 = (f64)(_get39);
                    goto j124;
                  }
                // end
                j126:;
                _get40 = jjtypeswitch_tmp1;
                // if 
                  if (_get40 == 195) {
                    _get41 = jjforof_length0;
                    if ((_get41) == 0) {
                      goto j123;
                    }
                    jjforof_allocd = dong_porf_js_microbench__Porffor_malloc(8);
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
                    _r124 = (f64)(_get47);
                    goto j124;
                  }
                // end
                j127:;
              // end
              j124:;
              x = _r124;
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
              f64 _r128;
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
                  _r128 = 0;
                } else {
                  _r128 = 0;
                }
              // end
              j128:;
              f64_store(0, 4, 0, _r128);
              _get64 = jjmember_setter_ptr_tmp;
              _get65 = jjlast_type;
              i32_store8(0, 12, _get64, _get65);
              _get66 = i;
              i = _get66 + 1;
              goto j122;
            // end
            j123:;
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
          j129:;
          _get77 = jjforof_base_pointer0;
          jjforof_length0 = i32_load(1, 0, _get77);
          // loop 
          j130:;
            // block 
              _get78 = jjforof_itertype0;
              jjtypeswitch_tmp1 = _get78;
              // block f64
              f64 _r132;
                _get79 = jjtypeswitch_tmp1;
                _get80 = jjtypeswitch_tmp1;
                // if 
                  if (((_get79 == 72) | (_get80 == 11)) != 0) {
                    _get81 = jjforof_length0;
                    if ((_get81) == 0) {
                      goto j131;
                    }
                    _get82 = jjforof_base_pointer0;
                    _get83 = jjforof_base_pointer0;
                    _get84 = jjforof_base_pointer0;
                    jjforof_base_pointer0 = _get84 + 9;
                    _get85 = jjforof_length0;
                    jjforof_length0 = _get85 - 1;
                    jjlast_type = i32_load8_u(0, 12, _get83);
                    _r132 = f64_load(0, 4, _get82);
                    goto j132;
                  }
                // end
                j133:;
                _get86 = jjtypeswitch_tmp1;
                // if 
                  if (_get86 == 67) {
                    _get87 = jjforof_length0;
                    if ((_get87) == 0) {
                      goto j131;
                    }
                    jjforof_allocd = dong_porf_js_microbench__Porffor_malloc(8);
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
                    _r132 = (f64)(_get93);
                    goto j132;
                  }
                // end
                j134:;
                _get94 = jjtypeswitch_tmp1;
                // if 
                  if (_get94 == 195) {
                    _get95 = jjforof_length0;
                    if ((_get95) == 0) {
                      goto j131;
                    }
                    jjforof_allocd = dong_porf_js_microbench__Porffor_malloc(8);
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
                    _r132 = (f64)(_get101);
                    goto j132;
                  }
                // end
                j135:;
                _r132 = 0;
              // end
              j132:;
              x = _r132;
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
              goto j130;
            // end
            j131:;
          // end
        }
      // end
      j119:;
      _get112 = out;
      _get113 = i;
      i32_store(1, 0, (u32)(_get112), (u32)(_get113));
      _get114 = out;
      return _get114;
      (void) 0;
    }
  // end
  j118:;
  _get115 = arg;
  _get116 = argjjtype;
  // if 
    if ((dong_porf_js_microbench__Porffor_object_isObject((i32)(_get115), _get116)) != 0) {
      _get117 = argjjtype;
      // if f64
      f64 _r137;
        if ((f64)(_get117) == 7) {
          _get118 = arg;
          _get119 = argjjtype;
          jjlast_type = _get119;
          _r137 = _get118;
        } else {
          _get120 = arg;
          _get121 = argjjtype;
          const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_underlying(_get120, _get121);
          jjlast_type = _0.type;
          _get122 = jjlast_type;
          jjlast_type = _get122;
          _r137 = (f64)(_0.value);
        }
      // end
      j137:;
      obj = _r137;
      jjmember_prop_17 = 44;
      _get123 = obj;
      jjmember_obj_17 = _get123;
      _get124 = jjmember_obj_17;
      _get125 = jjmember_prop_17;
      const struct ReturnValue _1 = dong_porf_js_microbench__ecma262_ToPropertyKey(_get125, 195);
      jjswap = _1.type;
      _get126 = jjswap;
      const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_object_get((i32)(_get124), 7, (i32)(_1.value), _get126);
      jjlast_type = _2.type;
      _get127 = jjlast_type;
      len = dong_porf_js_microbench__ecma262_ToIntegerOrInfinity(_2.value, _get127);
      _get128 = len;
      // if 
        if (_get128 > 4294967295) {
          (void) 0;
        }
      // end
      j465:;
      _get129 = len;
      // if 
        if (_get129 < 0) {
          len = 0;
        }
      // end
      j467:;
      _get130 = mapFnjjtype;
      // if 
        if ((f64)(_get130) != 0) {
          _get131 = mapFnjjtype;
          // if 
            if ((f64)(_get131) != 6) {
              (void) 0;
            }
          // end
          j469:;
          i = 0;
          // loop 
          j470:;
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
                f64 _r472;
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
                    const struct ReturnValue _3 = dong_porf_js_microbench__ecma262_ToPropertyKey(_get148, 1);
                    jjswap = _3.type;
                    _get149 = jjswap;
                    const struct ReturnValue _4 = dong_porf_js_microbench__Porffor_object_get((i32)(_get147), 7, (i32)(_3.value), _get149);
                    jjlast_type = _4.type;
                    _get150 = jjlast_type;
                    _get151 = i;
                    _get152 = jjindirect_20_callee;
                    jjlast_type = 0;
                    _r472 = 0;
                  } else {
                    _r472 = 0;
                  }
                // end
                j472:;
                f64_store(0, 4, 0, _r472);
                _get153 = jjmember_setter_ptr_tmp;
                _get154 = jjlast_type;
                i32_store8(0, 12, _get153, _get154);
                _get155 = i;
                i = _get155 + 1;
                goto j470;
              }
            // end
            j471:;
          // end
        } else {
          i = 0;
          // loop 
          j473:;
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
                const struct ReturnValue _5 = dong_porf_js_microbench__ecma262_ToPropertyKey(_get166, 1);
                jjswap = _5.type;
                _get167 = jjswap;
                const struct ReturnValue _6 = dong_porf_js_microbench__Porffor_object_get((i32)(_get165), 7, (i32)(_5.value), _get167);
                jjlast_type = _6.type;
                f64_store(0, 4, _get162, _6.value);
                _get168 = jjmember_setter_ptr_tmp;
                _get169 = jjlast_type;
                i32_store8(0, 12, _get168, _get169);
                _get170 = i;
                i = _get170 + 1;
                goto j473;
              }
            // end
            j474:;
          // end
        }
      // end
      j468:;
      _get171 = out;
      _get172 = len;
      i32_store(1, 0, (u32)(_get171), (u32)(_get172));
      _get173 = out;
      return _get173;
      (void) 0;
    }
  // end
  j136:;
  _get174 = out;
  return _get174;
}

static f64 dong_porf_js_microbench__Porffor_bytestring_appendChar(f64 str, i32 strjjtype, f64 _char, i32 charjjtype) {
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

static struct ReturnValue dong_porf_js_microbench__Array_prototype_toString(f64 _this, i32 _thisjjtype) {
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
      _this = dong_porf_js_microbench__Array_from(_get1, _get2, 0, 0, 0, 0);
      _thisjjtype = 72;
    }
  // end
  j112:;
  out = (f64)(dong_porf_js_microbench__Porffor_malloc(16384));
  _get3 = _this;
  len = (f64)(i32_load(1, 0, (u32)(_get3)));
  i = 0;
  // loop 
  j475:;
    _get4 = i;
    _get5 = len;
    // if 
      if (_get4 < _get5) {
        _get6 = i;
        // if 
          if (_get6 > 0) {
            _get7 = out;
            (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get7, 195, 44, 1);
          }
        // end
        j477:;
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
        i32 _r478;
          if ((_get17) == 0) {
            _get18 = elementjjtype;
            _get19 = elementjjtype;
            jjlast_type = 2;
            _r478 = ((f64)(_get18) != 0) & ((f64)(_get19) != 7);
          } else {
            _get20 = logictmpi;
            jjlast_type = 2;
            _r478 = _get20;
          }
        // end
        j478:;
        // if 
          if ((_r478) != 0) {
            _get21 = out;
            _get22 = element;
            _get23 = elementjjtype;
            const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString(_get22, _get23);
            jjlast_type = _0.type;
            _get24 = jjlast_type;
            (void) dong_porf_js_microbench__Porffor_bytestring_appendStr(_get21, 195, _0.value, _get24);
          }
        // end
        j479:;
        goto j475;
      }
    // end
    j476:;
  // end
  _get25 = out;
  return (struct ReturnValue){ _get25, 195 };
}

static struct ReturnValue dong_porf_js_microbench__ecma262_ToPrimitive_String(f64 input, i32 inputjjtype) {
  i32 _get101;
  f64 _get100;
  i32 _get99;
  i32 _get98;
  f64 _get97;
  i32 _get96;
  f64 _get95;
  i32 _get94;
  i32 _get93;
  i32 _get92;
  f64 _get91;
  i32 _get90;
  i32 _get89;
  f64 _get88;
  i32 _get87;
  f64 _get86;
  i32 _get85;
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
  i32 _get59;
  i32 _get58;
  f64 _get57;
  i32 _get56;
  f64 _get55;
  i32 _get54;
  i32 _get53;
  f64 _get52;
  i32 _get51;
  f64 _get50;
  i32 _get49;
  i32 _get48;
  i32 _get47;
  f64 _get46;
  i32 _get45;
  i32 _get44;
  f64 _get43;
  i32 _get42;
  f64 _get41;
  i32 _get40;
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
  f64 _r31;
    _get3 = jjtypeswitch_tmp1;
    // if 
      if (_get3 == 0) {
        jjlast_type = 0;
        _r31 = 0;
        goto j31;
      }
    // end
    j32:;
    _get4 = jjtypeswitch_tmp1;
    // if 
      if (_get4 == 1) {
        _get5 = jjproto_target;
        _get6 = jjproto_targetjjtype;
        const struct ReturnValue _0 = dong_porf_js_microbench__Number_prototype_toString(_get5, _get6, 0, 0);
        jjlast_type = _0.type;
        _r31 = _0.value;
        goto j31;
      }
    // end
    j33:;
    _get7 = jjtypeswitch_tmp1;
    // if 
      if (_get7 == 2) {
        _get8 = jjproto_target;
        _get9 = jjproto_targetjjtype;
        const struct ReturnValue _1 = dong_porf_js_microbench__Boolean_prototype_toString(_get8, _get9);
        jjlast_type = _1.type;
        _r31 = _1.value;
        goto j31;
      }
    // end
    j34:;
    _get10 = jjtypeswitch_tmp1;
    // if 
      if (_get10 == 6) {
        _get11 = jjproto_target;
        _get12 = jjproto_targetjjtype;
        const struct ReturnValue _2 = dong_porf_js_microbench__Function_prototype_toString(_get11, _get12);
        jjlast_type = _2.type;
        _r31 = _2.value;
        goto j31;
      }
    // end
    j36:;
    _get13 = jjtypeswitch_tmp1;
    // if 
      if (_get13 == 7) {
        _get14 = jjproto_target;
        _get15 = jjproto_targetjjtype;
        const struct ReturnValue _3 = dong_porf_js_microbench__Object_prototype_toString(_get14, _get15);
        jjlast_type = _3.type;
        _r31 = _3.value;
        goto j31;
      }
    // end
    j40:;
    _get16 = jjtypeswitch_tmp1;
    // if 
      if (_get16 == 31) {
        _get17 = jjproto_target;
        _get18 = jjproto_targetjjtype;
        const struct ReturnValue _4 = dong_porf_js_microbench__Boolean_prototype_toString(_get17, _get18);
        jjlast_type = _4.type;
        _r31 = _4.value;
        goto j31;
      }
    // end
    j88:;
    _get19 = jjtypeswitch_tmp1;
    // if 
      if (_get19 == 32) {
        _get20 = jjproto_target;
        _get21 = jjproto_targetjjtype;
        const struct ReturnValue _5 = dong_porf_js_microbench__Number_prototype_toString(_get20, _get21, 0, 0);
        jjlast_type = _5.type;
        _r31 = _5.value;
        goto j31;
      }
    // end
    j89:;
    _get22 = jjtypeswitch_tmp1;
    // if 
      if (_get22 == 33) {
        _get23 = jjproto_target;
        _get24 = jjproto_targetjjtype;
        const struct ReturnValue _6 = dong_porf_js_microbench__String_prototype_toString((i32)(_get23), _get24);
        jjlast_type = _6.type;
        _r31 = (f64)(_6.value);
        goto j31;
      }
    // end
    j90:;
    _get25 = jjtypeswitch_tmp1;
    // if 
      if (_get25 == 38) {
        _get26 = jjproto_target;
        _get27 = jjproto_targetjjtype;
        const struct ReturnValue _7 = dong_porf_js_microbench__TypeError_prototype_toString(_get26, _get27);
        jjlast_type = _7.type;
        _r31 = _7.value;
        goto j31;
      }
    // end
    j95:;
    _get28 = jjtypeswitch_tmp1;
    // if 
      if (_get28 == 40) {
        _get29 = jjproto_target;
        _get30 = jjproto_targetjjtype;
        const struct ReturnValue _8 = dong_porf_js_microbench__SyntaxError_prototype_toString(_get29, _get30);
        jjlast_type = _8.type;
        _r31 = _8.value;
        goto j31;
      }
    // end
    j106:;
    _get31 = jjtypeswitch_tmp1;
    // if 
      if (_get31 == 41) {
        _get32 = jjproto_target;
        _get33 = jjproto_targetjjtype;
        const struct ReturnValue _9 = dong_porf_js_microbench__RangeError_prototype_toString(_get32, _get33);
        jjlast_type = _9.type;
        _r31 = _9.value;
        goto j31;
      }
    // end
    j108:;
    _get34 = jjtypeswitch_tmp1;
    // if 
      if (_get34 == 67) {
        _get35 = jjproto_target;
        _get36 = jjproto_targetjjtype;
        const struct ReturnValue _10 = dong_porf_js_microbench__String_prototype_toString((i32)(_get35), _get36);
        jjlast_type = _10.type;
        _r31 = (f64)(_10.value);
        goto j31;
      }
    // end
    j110:;
    _get37 = jjtypeswitch_tmp1;
    // if 
      if (_get37 == 72) {
        _get38 = jjproto_target;
        _get39 = jjproto_targetjjtype;
        const struct ReturnValue _11 = dong_porf_js_microbench__Array_prototype_toString(_get38, _get39);
        jjlast_type = _11.type;
        _r31 = _11.value;
        goto j31;
      }
    // end
    j111:;
    _get40 = jjtypeswitch_tmp1;
    // if 
      if (_get40 == 195) {
        _get41 = jjproto_target;
        _get42 = jjproto_targetjjtype;
        const struct ReturnValue _12 = dong_porf_js_microbench__ByteString_prototype_toString((i32)(_get41), _get42);
        jjlast_type = _12.type;
        _r31 = (f64)(_12.value);
        goto j31;
      }
    // end
    j480:;
    _get43 = jjproto_target;
    _get44 = jjproto_targetjjtype;
    const struct ReturnValue _13 = dong_porf_js_microbench__Object_prototype_toString(_get43, _get44);
    jjlast_type = _13.type;
    _r31 = _13.value;
  // end
  j31:;
  value = _r31;
  _get45 = jjlast_type;
  valuejjtype = _get45;
  _get46 = value;
  jjlogicinner_tmp = _get46;
  _get47 = valuejjtype;
  jjtypeswitch_tmp1 = _get47;
  // block i32
  i32 _r481;
    _get48 = jjtypeswitch_tmp1;
    // if 
      if (_get48 == 0) {
        _r481 = 1;
        goto j481;
      }
    // end
    j482:;
    _get49 = jjtypeswitch_tmp1;
    // if 
      if (_get49 == 7) {
        _get50 = jjlogicinner_tmp;
        _r481 = _get50 == 0;
        goto j481;
      }
    // end
    j483:;
    _r481 = 0;
  // end
  j481:;
  logictmpi = (_r481) == 0;
  _get51 = logictmpi;
  // if i32
  i32 _r484;
    if ((_get51) != 0) {
      _get52 = value;
      _get53 = valuejjtype;
      jjlast_type = 2;
      _r484 = (f64)(dong_porf_js_microbench__Porffor_object_isObjectOrNull((i32)(_get52), _get53)) == 0;
    } else {
      _get54 = logictmpi;
      jjlast_type = 2;
      _r484 = _get54;
    }
  // end
  j484:;
  // if 
    if ((_r484) != 0) {
      _get55 = value;
      _get56 = valuejjtype;
      return (struct ReturnValue){ _get55, _get56 };
    }
  // end
  j485:;
  _get57 = input;
  jjproto_target = _get57;
  _get58 = inputjjtype;
  jjproto_targetjjtype = _get58;
  _get59 = inputjjtype;
  jjtypeswitch_tmp1 = _get59;
  // block f64
  f64 _r486;
    _get60 = jjtypeswitch_tmp1;
    // if 
      if (_get60 == 0) {
        jjlast_type = 0;
        _r486 = 0;
        goto j486;
      }
    // end
    j487:;
    _get61 = jjtypeswitch_tmp1;
    // if 
      if (_get61 == 1) {
        _get62 = jjproto_target;
        _get63 = jjproto_targetjjtype;
        const struct ReturnValue _14 = dong_porf_js_microbench__Number_prototype_valueOf(_get62, _get63);
        jjlast_type = _14.type;
        _r486 = _14.value;
        goto j486;
      }
    // end
    j488:;
    _get64 = jjtypeswitch_tmp1;
    // if 
      if (_get64 == 2) {
        _get65 = jjproto_target;
        _get66 = jjproto_targetjjtype;
        const struct ReturnValue _15 = dong_porf_js_microbench__Boolean_prototype_valueOf(_get65, _get66);
        jjlast_type = _15.type;
        _r486 = _15.value;
        goto j486;
      }
    // end
    j489:;
    _get67 = jjtypeswitch_tmp1;
    // if 
      if (_get67 == 7) {
        _get68 = jjproto_target;
        _get69 = jjproto_targetjjtype;
        const struct ReturnValue _16 = dong_porf_js_microbench__Object_prototype_valueOf(_get68, _get69);
        jjlast_type = _16.type;
        _r486 = _16.value;
        goto j486;
      }
    // end
    j490:;
    _get70 = jjtypeswitch_tmp1;
    // if 
      if (_get70 == 31) {
        _get71 = jjproto_target;
        _get72 = jjproto_targetjjtype;
        const struct ReturnValue _17 = dong_porf_js_microbench__Boolean_prototype_valueOf(_get71, _get72);
        jjlast_type = _17.type;
        _r486 = _17.value;
        goto j486;
      }
    // end
    j491:;
    _get73 = jjtypeswitch_tmp1;
    // if 
      if (_get73 == 32) {
        _get74 = jjproto_target;
        _get75 = jjproto_targetjjtype;
        const struct ReturnValue _18 = dong_porf_js_microbench__Number_prototype_valueOf(_get74, _get75);
        jjlast_type = _18.type;
        _r486 = _18.value;
        goto j486;
      }
    // end
    j492:;
    _get76 = jjtypeswitch_tmp1;
    // if 
      if (_get76 == 33) {
        _get77 = jjproto_target;
        _get78 = jjproto_targetjjtype;
        const struct ReturnValue _19 = dong_porf_js_microbench__String_prototype_valueOf((i32)(_get77), _get78);
        jjlast_type = _19.type;
        _r486 = (f64)(_19.value);
        goto j486;
      }
    // end
    j493:;
    _get79 = jjtypeswitch_tmp1;
    // if 
      if (_get79 == 67) {
        _get80 = jjproto_target;
        _get81 = jjproto_targetjjtype;
        const struct ReturnValue _20 = dong_porf_js_microbench__String_prototype_valueOf((i32)(_get80), _get81);
        jjlast_type = _20.type;
        _r486 = (f64)(_20.value);
        goto j486;
      }
    // end
    j494:;
    _get82 = jjtypeswitch_tmp1;
    // if 
      if (_get82 == 72) {
        _get83 = jjproto_target;
        _get84 = jjproto_targetjjtype;
        const struct ReturnValue _21 = dong_porf_js_microbench__Array_prototype_valueOf(_get83, _get84);
        jjlast_type = _21.type;
        _r486 = _21.value;
        goto j486;
      }
    // end
    j495:;
    _get85 = jjtypeswitch_tmp1;
    // if 
      if (_get85 == 195) {
        _get86 = jjproto_target;
        _get87 = jjproto_targetjjtype;
        const struct ReturnValue _22 = dong_porf_js_microbench__ByteString_prototype_valueOf((i32)(_get86), _get87);
        jjlast_type = _22.type;
        _r486 = (f64)(_22.value);
        goto j486;
      }
    // end
    j496:;
    _get88 = jjproto_target;
    _get89 = jjproto_targetjjtype;
    const struct ReturnValue _23 = dong_porf_js_microbench__Object_prototype_valueOf(_get88, _get89);
    jjlast_type = _23.type;
    _r486 = _23.value;
  // end
  j486:;
  value = _r486;
  _get90 = jjlast_type;
  valuejjtype = _get90;
  _get91 = value;
  jjlogicinner_tmp = _get91;
  _get92 = valuejjtype;
  jjtypeswitch_tmp1 = _get92;
  // block i32
  i32 _r497;
    _get93 = jjtypeswitch_tmp1;
    // if 
      if (_get93 == 0) {
        _r497 = 1;
        goto j497;
      }
    // end
    j498:;
    _get94 = jjtypeswitch_tmp1;
    // if 
      if (_get94 == 7) {
        _get95 = jjlogicinner_tmp;
        _r497 = _get95 == 0;
        goto j497;
      }
    // end
    j499:;
    _r497 = 0;
  // end
  j497:;
  logictmpi = (_r497) == 0;
  _get96 = logictmpi;
  // if i32
  i32 _r500;
    if ((_get96) != 0) {
      _get97 = value;
      _get98 = valuejjtype;
      jjlast_type = 2;
      _r500 = (f64)(dong_porf_js_microbench__Porffor_object_isObjectOrNull((i32)(_get97), _get98)) == 0;
    } else {
      _get99 = logictmpi;
      jjlast_type = 2;
      _r500 = _get99;
    }
  // end
  j500:;
  // if 
    if ((_r500) != 0) {
      _get100 = value;
      _get101 = valuejjtype;
      return (struct ReturnValue){ _get100, _get101 };
    }
  // end
  j501:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_js_microbench__ecma262_ToString(f64 argument, i32 argumentjjtype) {
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
  j23:;
  _get3 = argumentjjtype;
  // if 
    if ((f64)(_get3) == 5) {
    }
  // end
  j24:;
  _get4 = argumentjjtype;
  // if 
    if ((f64)(_get4) == 0) {
      return (struct ReturnValue){ 145, 195 };
    }
  // end
  j25:;
  _get5 = argumentjjtype;
  _get6 = argument;
  // if 
    if ((((f64)(_get5) == 7) & (_get6 == 0)) != 0) {
      return (struct ReturnValue){ 160, 195 };
    }
  // end
  j26:;
  _get7 = argumentjjtype;
  // if 
    if ((f64)(_get7) == 2) {
      _get8 = argument;
      // if 
        if (_get8 == 1) {
          return (struct ReturnValue){ 170, 195 };
        }
      // end
      j28:;
      return (struct ReturnValue){ 180, 195 };
    }
  // end
  j27:;
  _get9 = argumentjjtype;
  // if 
    if ((f64)(_get9) == 1) {
      _get10 = argument;
      _get11 = argumentjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__Number_prototype_toString(_get10, _get11, 10, 1);
      jjlast_type = _0.type;
      _get12 = jjlast_type;
      return (struct ReturnValue){ _0.value, _get12 };
    }
  // end
  j29:;
  _get13 = argumentjjtype;
  // if 
    if ((f64)(_get13) == 33) {
      _get14 = argument;
      return (struct ReturnValue){ _get14, 67 };
    }
  // end
  j30:;
  _get15 = argument;
  _get16 = argumentjjtype;
  const struct ReturnValue _1 = dong_porf_js_microbench__ecma262_ToPrimitive_String(_get15, _get16);
  jjlast_type = _1.type;
  _get17 = jjlast_type;
  primValuejjtype = _get17;
  primValue = _1.value;
  _get18 = primValue;
  _get19 = primValuejjtype;
  const struct ReturnValue _2 = dong_porf_js_microbench__ecma262_ToString(_get18, _get19);
  jjlast_type = _2.type;
  _get20 = jjlast_type;
  return (struct ReturnValue){ _2.value, _get20 };
}

static f64 dong_porf_js_microbench_TypeError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype) {
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
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString(_get2, _get3);
      jjlast_type = _0.type;
      _get4 = jjlast_type;
      messagejjtype = _get4;
      message = _0.value;
    }
  // end
  j22:;
  obj = (f64)(dong_porf_js_microbench__Porffor_malloc(8));
  _get5 = obj;
  _get6 = message;
  i32_store(0, 0, (i32)(_get5), (i32)(_get6));
  _get7 = obj;
  _get8 = messagejjtype;
  i32_store8(0, 4, (i32)(_get7), _get8);
  _get9 = obj;
  return _get9;
}

static f64 dong_porf_js_microbench__Math_round(f64 l0) {
  f64 _get0;
  _get0 = l0;
  return round(_get0);
}

static struct ReturnValue dong_porf_js_microbench__Number_prototype_toString(f64 _this, i32 _thisjjtype, f64 radix, i32 radixjjtype) {
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
  j21:;
  _get2 = radixjjtype;
  // if 
    if ((f64)(_get2) != 1) {
      radix = 10;
      radixjjtype = 1;
    }
  // end
  j502:;
  _get3 = radix;
  radix = dong_porf_js_microbench__Math_trunc(_get3);
  radixjjtype = 1;
  _get4 = radix;
  logictmpi = _get4 < 2;
  _get5 = logictmpi;
  // if i32
  i32 _r503;
    if ((_get5) == 0) {
      _get6 = radix;
      jjlast_type = 2;
      _r503 = _get6 > 36;
    } else {
      _get7 = logictmpi;
      jjlast_type = 2;
      _r503 = _get7;
    }
  // end
  j503:;
  // if 
    if ((_r503) != 0) {
    }
  // end
  j504:;
  _get8 = _this;
  // if 
    if (dong_porf_js_microbench__Number_isFinite(_get8) == 0) {
      _get9 = _this;
      // if 
        if (((u32)(dong_porf_js_microbench__Number_isNaN(_get9))) != 0) {
          return (struct ReturnValue){ 1612, 195 };
        }
      // end
      j506:;
      _get10 = _this;
      // if 
        if (_get10 == Infinity) {
          return (struct ReturnValue){ 1621, 195 };
        }
      // end
      j507:;
      return (struct ReturnValue){ 1635, 195 };
    }
  // end
  j505:;
  _get11 = _this;
  // if 
    if (_get11 == 0) {
      return (struct ReturnValue){ 1650, 195 };
    }
  // end
  j508:;
  out = (f64)(dong_porf_js_microbench__Porffor_malloc(512));
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
  j509:;
  _get17 = _this;
  i = dong_porf_js_microbench__Math_trunc(_get17);
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
          j512:;
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
                i = dong_porf_js_microbench__Math_trunc(_get27 / _get28);
                _get29 = e;
                e = _get29 + 1;
                _get30 = trailing;
                // if 
                  if (((u32)(_get30)) != 0) {
                    _get31 = digit;
                    // if 
                      if (_get31 == 0) {
                        goto j512;
                      }
                    // end
                    j515:;
                    trailing = 0;
                  }
                // end
                j514:;
                _get32 = digits;
                _get33 = l;
                _get34 = digit;
                i32_store8(0, 4, (i32)((_get32 + _get33)), (i32)(_get34));
                _get35 = l;
                l = _get35 + 1;
                goto j512;
              }
            // end
            j513:;
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
          j516:;
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
                j518:;
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
                j519:;
                _get53 = outPtr;
                _get54 = outPtr;
                outPtr = _get54 + 1;
                _get55 = digit;
                i32_store8(0, 4, (i32)(_get53), (i32)(_get55));
                goto j516;
              }
            // end
            j517:;
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
          j520:;
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
                e = dong_porf_js_microbench__Math_trunc(_get69 / _get70);
                _get71 = l;
                l = _get71 + 1;
                goto j520;
              }
            // end
            j521:;
          // end
          _get72 = digits;
          _get73 = l;
          digitsPtr = _get72 + _get73;
          _get74 = outPtr;
          _get75 = l;
          endPtr = _get74 + _get75;
          // loop 
          j522:;
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
                j524:;
                _get83 = outPtr;
                _get84 = outPtr;
                outPtr = _get84 + 1;
                _get85 = digit;
                i32_store8(0, 4, (i32)(_get83), (i32)(_get85));
                goto j522;
              }
            // end
            j523:;
          // end
          _get86 = out;
          _get87 = outPtr;
          _get88 = out;
          i32_store(1, 0, (u32)(_get86), (u32)((_get87 - _get88)));
          _get89 = out;
          return (struct ReturnValue){ _get89, 195 };
        }
      // end
      j511:;
      _get90 = _this;
      // if 
        if (_get90 < 0.000001) {
          _get91 = _this;
          decimal = _get91;
          e = 1;
          // loop 
          j526:;
            // if 
              if ((1) != 0) {
                _get92 = decimal;
                _get93 = radix;
                decimal = _get92 * _get93;
                _get94 = decimal;
                intPart = dong_porf_js_microbench__Math_trunc(_get94);
                _get95 = intPart;
                // if 
                  if (_get95 > 0) {
                    _get96 = decimal;
                    _get97 = intPart;
                    // if 
                      if ((_get96 - _get97) < 1e-10) {
                        goto j527;
                      }
                    // end
                    j529:;
                  } else {
                    _get98 = e;
                    e = _get98 + 1;
                  }
                // end
                j528:;
                goto j526;
              }
            // end
            j527:;
          // end
          // loop 
          j530:;
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
                decimal = dong_porf_js_microbench__Math_trunc(_get106 / _get107);
                _get108 = digits;
                _get109 = l;
                _get110 = digit;
                i32_store8(0, 4, (i32)((_get108 + _get109)), (i32)(_get110));
                _get111 = l;
                l = _get111 + 1;
                goto j530;
              }
            // end
            j531:;
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
          j532:;
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
                j534:;
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
                j535:;
                _get129 = outPtr;
                _get130 = outPtr;
                outPtr = _get130 + 1;
                _get131 = digit;
                i32_store8(0, 4, (i32)(_get129), (i32)(_get131));
                goto j532;
              }
            // end
            j533:;
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
          j536:;
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
                e = dong_porf_js_microbench__Math_trunc(_get145 / _get146);
                _get147 = l;
                l = _get147 + 1;
                goto j536;
              }
            // end
            j537:;
          // end
          _get148 = digits;
          _get149 = l;
          digitsPtr = _get148 + _get149;
          _get150 = outPtr;
          _get151 = l;
          endPtr = _get150 + _get151;
          // loop 
          j538:;
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
                j540:;
                _get159 = outPtr;
                _get160 = outPtr;
                outPtr = _get160 + 1;
                _get161 = digit;
                i32_store8(0, 4, (i32)(_get159), (i32)(_get161));
                goto j538;
              }
            // end
            j539:;
          // end
          _get162 = out;
          _get163 = outPtr;
          _get164 = out;
          i32_store(1, 0, (u32)(_get162), (u32)((_get163 - _get164)));
          _get165 = out;
          return (struct ReturnValue){ _get165, 195 };
        }
      // end
      j525:;
    }
  // end
  j510:;
  _get166 = i;
  // if 
    if (_get166 == 0) {
      _get167 = digits;
      i32_store8(0, 4, (i32)(_get167), 0);
      l = 1;
    } else {
      // loop 
      j542:;
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
            i = dong_porf_js_microbench__Math_trunc(_get177 / _get178);
            _get179 = l;
            l = _get179 + 1;
            goto j542;
          }
        // end
        j543:;
      // end
    }
  // end
  j541:;
  _get180 = digits;
  _get181 = l;
  digitsPtr = _get180 + _get181;
  _get182 = outPtr;
  _get183 = l;
  endPtr = _get182 + _get183;
  // loop 
  j544:;
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
        j546:;
        _get191 = outPtr;
        _get192 = outPtr;
        outPtr = _get192 + 1;
        _get193 = digit;
        i32_store8(0, 4, (i32)(_get191), (i32)(_get193));
        goto j544;
      }
    // end
    j545:;
  // end
  _get194 = _this;
  _get195 = _this;
  decimal = _get194 - dong_porf_js_microbench__Math_trunc(_get195);
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
      j548:;
        _get201 = j;
        _get202 = decimalDigits;
        // if 
          if (_get201 < _get202) {
            _get203 = decimal;
            _get204 = radix;
            decimal = _get203 * _get204;
            _get205 = j;
            j = _get205 + 1;
            goto j548;
          }
        // end
        j549:;
      // end
      _get206 = decimal;
      decimal = dong_porf_js_microbench__Math_round(_get206);
      l = 0;
      trailing = 1;
      // loop 
      j550:;
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
            decimal = dong_porf_js_microbench__Math_trunc(_get214 / _get215);
            _get216 = trailing;
            // if 
              if (((u32)(_get216)) != 0) {
                _get217 = digit;
                // if 
                  if (_get217 == 0) {
                    goto j550;
                  }
                // end
                j553:;
                trailing = 0;
              }
            // end
            j552:;
            _get218 = digits;
            _get219 = l;
            _get220 = digit;
            i32_store8(0, 4, (i32)((_get218 + _get219)), (i32)(_get220));
            _get221 = l;
            l = _get221 + 1;
            goto j550;
          }
        // end
        j551:;
      // end
      _get222 = digits;
      _get223 = l;
      digitsPtr = _get222 + _get223;
      _get224 = outPtr;
      _get225 = l;
      endPtr = _get224 + _get225;
      // loop 
      j554:;
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
            j556:;
            _get233 = outPtr;
            _get234 = outPtr;
            outPtr = _get234 + 1;
            _get235 = digit;
            i32_store8(0, 4, (i32)(_get233), (i32)(_get235));
            goto j554;
          }
        // end
        j555:;
      // end
    }
  // end
  j547:;
  _get236 = out;
  _get237 = outPtr;
  _get238 = out;
  i32_store(1, 0, (u32)(_get236), (u32)((_get237 - _get238)));
  _get239 = out;
  return (struct ReturnValue){ _get239, 195 };
}

static struct ReturnValue dong_porf_js_microbench__Porffor_object_underlying(f64 _obj, i32 _objjjtype) {
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
  j4:;
  _get2 = _objjjtype;
  // if 
    if (_get2 > 5) {
      // if 
        if ((dong_porf_js_microbench_jjporfjjunderlyingStorejjglbl_inited) == 0) {
          dong_porf_js_microbench_jjporfjjunderlyingStore = 0;
          dong_porf_js_microbench_jjporfjjunderlyingStorejjglbl_inited = 1;
        }
      // end
      j6:;
      // if 
        if ((dong_porf_js_microbench_jjporfjjunderlyingStore) == 0) {
          dong_porf_js_microbench_jjporfjjunderlyingStore = dong_porf_js_microbench__Porffor_malloc(16384);
        }
      // end
      j7:;
      underlyingLength = i32_load(0, 0, dong_porf_js_microbench_jjporfjjunderlyingStore);
      _get3 = underlyingLength;
      end = _get3 * 12;
      i = 0;
      // loop 
      j8:;
        _get4 = i;
        _get5 = end;
        // if 
          if (_get4 < _get5) {
            _get6 = i;
            _get7 = _obj;
            // if 
              if (f64_load(0, 4, dong_porf_js_microbench_jjporfjjunderlyingStore + _get6) == _get7) {
                _get8 = i;
                return (struct ReturnValue){ i32_load(0, 12, dong_porf_js_microbench_jjporfjjunderlyingStore + _get8), 7 };
              }
            // end
            j10:;
            _get9 = i;
            i = _get9 + 12;
            goto j8;
          }
        // end
        j9:;
      // end
      _get10 = _obj;
      obj = (u32)(_get10);
      underlying = dong_porf_js_microbench__Porffor_malloc(16384);
      _get11 = _objjjtype;
      // if 
        if (_get11 == 6) {
          _get12 = underlying;
          _get13 = obj;
          dong_porf_js_microbench__Porffor_object_fastAdd(_get12, 7, 44, 195, (f64)(dong_porf_js_microbench__Porffor_funcLut_length(_get13)), 1, 2, 1);
          _get14 = underlying;
          _get15 = obj;
          dong_porf_js_microbench__Porffor_object_fastAdd(_get14, 7, 56, 195, (f64)(dong_porf_js_microbench__Porffor_funcLut_name(_get15)), 195, 2, 1);
          _get16 = _obj;
          _get17 = _objjjtype;
          // if 
            if (((i32)(dong_porf_js_microbench__ecma262_IsConstructor(_get16, _get17))) != 0) {
              proto = dong_porf_js_microbench__Porffor_malloc(16384);
              _get18 = underlying;
              _get19 = proto;
              dong_porf_js_microbench__Porffor_object_fastAdd(_get18, 7, 66, 195, (f64)(_get19), 7, 8, 1);
              _get20 = proto;
              _get21 = _obj;
              _get22 = _objjjtype;
              dong_porf_js_microbench__Porffor_object_fastAdd(_get20, 7, 81, 195, _get21, _get22, 10, 1);
            }
          // end
          j17:;
        }
      // end
      j11:;
      _get23 = _objjjtype;
      // if 
        if (_get23 == 72) {
          _get24 = obj;
          len = i32_load(0, 0, _get24);
          _get25 = underlying;
          _get26 = len;
          dong_porf_js_microbench__Porffor_object_fastAdd(_get25, 7, 44, 195, (f64)(_get26), 1, 8, 1);
          i = 0;
          // loop 
          j19:;
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
                const struct ReturnValue _0 = dong_porf_js_microbench__Number_prototype_toString((f64)(_get34), 1, 0, 0);
                jjlast_type = _0.type;
                _get35 = jjlast_type;
                _get36 = x;
                _get37 = xjjtype;
                dong_porf_js_microbench__Porffor_object_fastAdd(_get33, 7, (i32)(_0.value), _get35, _get36, _get37, 14, 1);
                _get38 = i;
                i = _get38 + 1;
                goto j19;
              }
            // end
            j20:;
          // end
        }
      // end
      j18:;
      _get39 = _objjjtype;
      _get40 = _objjjtype;
      // if 
        if (((_get39 == 67) | (_get40 == 33)) != 0) {
          _get41 = obj;
          len = i32_load(1, 0, _get41);
          _get42 = underlying;
          _get43 = len;
          dong_porf_js_microbench__Porffor_object_fastAdd(_get42, 7, 44, 195, (f64)(_get43), 1, 0, 1);
          i = 0;
          // loop 
          j558:;
            _get44 = i;
            _get45 = len;
            // if 
              if (_get44 < _get45) {
                _get46 = underlying;
                _get47 = i;
                const struct ReturnValue _1 = dong_porf_js_microbench__Number_prototype_toString((f64)(_get47), 1, 0, 0);
                jjlast_type = _1.type;
                _get48 = jjlast_type;
                _get49 = i;
                jjmember_prop_0 = _get49;
                _get50 = obj;
                jjmember_obj_0 = _get50;
                jjmember_allocd = dong_porf_js_microbench__Porffor_malloc(8);
                _get51 = jjmember_allocd;
                i32_store(0, 0, _get51, 1);
                _get52 = jjmember_allocd;
                _get53 = jjmember_prop_0;
                _get54 = jjmember_obj_0;
                i32_store16(0, 4, _get52, i32_load16_u(0, 4, (_get53 * 2) + _get54));
                _get55 = jjmember_allocd;
                jjlast_type = 67;
                dong_porf_js_microbench__Porffor_object_fastAdd(_get46, 7, (i32)(_1.value), _get48, (f64)(_get55), 67, 4, 1);
                _get56 = i;
                i = _get56 + 1;
                goto j558;
              }
            // end
            j559:;
          // end
          _get57 = _objjjtype;
          // if 
            if (_get57 == 67) {
              _get58 = obj;
              i32_store8(0, 2, _get58, 1);
            }
          // end
          j560:;
        }
      // end
      j557:;
      _get59 = _objjjtype;
      // if 
        if (_get59 == 195) {
          _get60 = obj;
          len = i32_load(1, 0, _get60);
          _get61 = underlying;
          _get62 = len;
          dong_porf_js_microbench__Porffor_object_fastAdd(_get61, 7, 44, 195, (f64)(_get62), 1, 0, 1);
          i = 0;
          // loop 
          j562:;
            _get63 = i;
            _get64 = len;
            // if 
              if (_get63 < _get64) {
                _get65 = underlying;
                _get66 = i;
                const struct ReturnValue _2 = dong_porf_js_microbench__Number_prototype_toString((f64)(_get66), 1, 0, 0);
                jjlast_type = _2.type;
                _get67 = jjlast_type;
                _get68 = i;
                jjmember_prop_1 = _get68;
                _get69 = obj;
                jjmember_obj_1 = _get69;
                jjmember_allocd = dong_porf_js_microbench__Porffor_malloc(8);
                _get70 = jjmember_allocd;
                i32_store(0, 0, _get70, 1);
                _get71 = jjmember_allocd;
                _get72 = jjmember_prop_1;
                _get73 = jjmember_obj_1;
                i32_store8(0, 4, _get71, i32_load8_u(0, 4, _get72 + _get73));
                _get74 = jjmember_allocd;
                jjlast_type = 195;
                dong_porf_js_microbench__Porffor_object_fastAdd(_get65, 7, (i32)(_2.value), _get67, (f64)(_get74), 195, 4, 1);
                _get75 = i;
                i = _get75 + 1;
                goto j562;
              }
            // end
            j563:;
          // end
          _get76 = obj;
          i32_store8(0, 2, _get76, 1);
        }
      // end
      j561:;
      _get77 = underlyingLength;
      i32_store(0, 0, dong_porf_js_microbench_jjporfjjunderlyingStore, _get77 + 1);
      _get78 = underlyingLength;
      _get79 = _obj;
      f64_store(0, 4, dong_porf_js_microbench_jjporfjjunderlyingStore + (_get78 * 12), _get79);
      _get80 = underlyingLength;
      _get81 = underlying;
      i32_store(0, 12, dong_porf_js_microbench_jjporfjjunderlyingStore + (_get80 * 12), _get81);
      _get82 = underlying;
      return (struct ReturnValue){ _get82, 7 };
    }
  // end
  j5:;
  _get83 = _obj;
  _get84 = _objjjtype;
  return (struct ReturnValue){ (u32)(_get83), _get84 };
}

static void dong_porf_js_microbench__Porffor_object_expr_init(i32 obj, i32 objjjtype, i32 key, i32 keyjjtype, f64 value, i32 valuejjtype) {
  i32 _get31;
  i32 _get30;
  f64 _get29;
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
  f64 _get17;
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
  i32 jjlast_type = 0;
  i32 hash = 0;
  i32 entryPtr = 0;
  i32 size = 0;

  _get0 = objjjtype;
  // if 
    if (_get0 != 7) {
      _get1 = obj;
      _get2 = objjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get1), _get2);
      jjlast_type = _0.type;
      _get3 = jjlast_type;
      objjjtype = _get3;
      obj = _0.value;
    }
  // end
  j3:;
  _get4 = key;
  _get5 = keyjjtype;
  hash = dong_porf_js_microbench__Porffor_object_hash(_get4, _get5);
  _get6 = obj;
  _get7 = objjjtype;
  _get8 = key;
  _get9 = keyjjtype;
  _get10 = hash;
  entryPtr = dong_porf_js_microbench__Porffor_object_lookup(_get6, _get7, _get8, _get9, _get10, 1);
  _get11 = entryPtr;
  // if 
    if (_get11 == -1) {
      _get12 = hash;
      // if 
        if (_get12 == 593337848) {
          _get13 = key;
          _get14 = keyjjtype;
          // if 
            if ((dong_porf_js_microbench__Porffor_strcmp(_get13, _get14, 579, 195)) != 0) {
              _get15 = obj;
              _get16 = objjjtype;
              _get17 = value;
              _get18 = valuejjtype;
              dong_porf_js_microbench__Porffor_object_setPrototype(_get15, _get16, (i32)(_get17), _get18);
              return;
            }
          // end
          j566:;
        }
      // end
      j565:;
      _get19 = obj;
      size = i32_load16_u(0, 0, _get19);
      _get20 = obj;
      _get21 = size;
      i32_store16(0, 0, _get20, _get21 + 1);
      _get22 = obj;
      _get23 = size;
      entryPtr = (_get22 + 8) + (_get23 * 18);
      _get24 = entryPtr;
      _get25 = key;
      _get26 = keyjjtype;
      _get27 = hash;
      dong_porf_js_microbench__Porffor_object_writeKey(_get24, 1, _get25, _get26, _get27, 1);
    }
  // end
  j564:;
  _get28 = entryPtr;
  _get29 = value;
  f64_store(0, 8, _get28, _get29);
  _get30 = entryPtr;
  _get31 = valuejjtype;
  i32_store16(0, 16, _get30, 14 + (_get31 << 8));
  return;
}

static struct ReturnValue dong_porf_js_microbench__Number_prototype_toFixed(f64 _this, i32 _thisjjtype, f64 fractionDigits, i32 fractionDigitsjjtype) {
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
  f64 _get7;
  i32 _get6;
  f64 _get5;
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  i32 _get0;
  i32 logictmpi = 0;
  i32 jjlast_type = 0;
  f64 out = 0;
  f64 outPtr = 0;
  f64 i = 0;
  f64 digits = 0;
  f64 l = 0;
  f64 jjmath_a = 0;
  f64 jjmath_b = 0;
  f64 digitsPtr = 0;
  f64 endPtr = 0;
  f64 digit = 0;
  f64 decimal = 0;
  f64 j = 0;

  _get0 = _thisjjtype;
  _get1 = _thisjjtype;
  // if 
    if (((_get0 != 1) & (_get1 != 32)) != 0) {
    }
  // end
  j574:;
  _get2 = fractionDigits;
  fractionDigits = dong_porf_js_microbench__Math_trunc(_get2);
  _get3 = fractionDigits;
  logictmpi = _get3 < 0;
  _get4 = logictmpi;
  // if i32
  i32 _r575;
    if ((_get4) == 0) {
      _get5 = fractionDigits;
      jjlast_type = 2;
      _r575 = _get5 > 100;
    } else {
      _get6 = logictmpi;
      jjlast_type = 2;
      _r575 = _get6;
    }
  // end
  j575:;
  // if 
    if ((_r575) != 0) {
    }
  // end
  j576:;
  _get7 = _this;
  // if 
    if (dong_porf_js_microbench__Number_isFinite(_get7) == 0) {
      _get8 = _this;
      // if 
        if (((u32)(dong_porf_js_microbench__Number_isNaN(_get8))) != 0) {
          return (struct ReturnValue){ 1612, 195 };
        }
      // end
      j578:;
      _get9 = _this;
      // if 
        if (_get9 == Infinity) {
          return (struct ReturnValue){ 1621, 195 };
        }
      // end
      j579:;
      return (struct ReturnValue){ 1635, 195 };
    }
  // end
  j577:;
  out = (f64)(dong_porf_js_microbench__Porffor_malloc(512));
  _get10 = out;
  outPtr = _get10;
  _get11 = _this;
  // if 
    if (_get11 < 0) {
      _get12 = _this;
      _this = -_get12;
      _get13 = outPtr;
      _get14 = outPtr;
      outPtr = _get14 + 1;
      i32_store8(0, 4, (i32)(_get13), 45);
    }
  // end
  j580:;
  _get15 = _this;
  i = dong_porf_js_microbench__Math_trunc(_get15);
  digits = 0;
  l = 0;
  _get16 = i;
  // if 
    if (_get16 == 0) {
      _get17 = digits;
      i32_store8(0, 4, (i32)(_get17), 0);
      l = 1;
    } else {
      // loop 
      j582:;
        _get18 = i;
        // if 
          if (_get18 > 0) {
            _get19 = digits;
            _get20 = l;
            _get21 = i;
            jjmath_a = _get21;
            _get22 = jjmath_a;
            jjmath_b = 10;
            _get23 = jjmath_b;
            _get24 = jjmath_a;
            _get25 = jjmath_b;
            i32_store8(0, 4, (i32)((_get19 + _get20)), (i32)((_get22 - (_get23 * trunc((_get24 / _get25))))));
            _get26 = i;
            i = dong_porf_js_microbench__Math_trunc(_get26 / 10);
            _get27 = l;
            l = _get27 + 1;
            goto j582;
          }
        // end
        j583:;
      // end
    }
  // end
  j581:;
  _get28 = digits;
  _get29 = l;
  digitsPtr = _get28 + _get29;
  _get30 = outPtr;
  _get31 = l;
  endPtr = _get30 + _get31;
  // loop 
  j584:;
    _get32 = outPtr;
    _get33 = endPtr;
    // if 
      if (_get32 < _get33) {
        _get34 = digitsPtr;
        digitsPtr = _get34 - 1;
        _get35 = digitsPtr;
        digit = (f64)(i32_load8_u(0, 4, (i32)(_get35)));
        _get36 = digit;
        // if 
          if (_get36 < 10) {
            _get37 = digit;
            digit = _get37 + 48;
          } else {
            _get38 = digit;
            digit = _get38 + 87;
          }
        // end
        j586:;
        _get39 = outPtr;
        _get40 = outPtr;
        outPtr = _get40 + 1;
        _get41 = digit;
        i32_store8(0, 4, (i32)(_get39), (i32)(_get41));
        goto j584;
      }
    // end
    j585:;
  // end
  _get42 = _this;
  _get43 = _this;
  decimal = _get42 - dong_porf_js_microbench__Math_trunc(_get43);
  _get44 = fractionDigits;
  // if 
    if (_get44 > 0) {
      _get45 = outPtr;
      _get46 = outPtr;
      outPtr = _get46 + 1;
      i32_store8(0, 4, (i32)(_get45), 46);
      _get47 = decimal;
      decimal = _get47 + 1;
      j = 0;
      // loop 
      j588:;
        _get48 = j;
        _get49 = fractionDigits;
        // if 
          if (_get48 < _get49) {
            _get50 = decimal;
            decimal = _get50 * 10;
            _get51 = j;
            j = _get51 + 1;
            goto j588;
          }
        // end
        j589:;
      // end
      _get52 = decimal;
      decimal = dong_porf_js_microbench__Math_round(_get52);
      l = 0;
      // loop 
      j590:;
        _get53 = decimal;
        // if 
          if (_get53 > 1) {
            _get54 = decimal;
            jjmath_a = _get54;
            _get55 = jjmath_a;
            jjmath_b = 10;
            _get56 = jjmath_b;
            _get57 = jjmath_a;
            _get58 = jjmath_b;
            digit = _get55 - (_get56 * trunc((_get57 / _get58)));
            _get59 = decimal;
            decimal = dong_porf_js_microbench__Math_trunc(_get59 / 10);
            _get60 = digits;
            _get61 = l;
            _get62 = digit;
            i32_store8(0, 4, (i32)((_get60 + _get61)), (i32)(_get62));
            _get63 = l;
            l = _get63 + 1;
            goto j590;
          }
        // end
        j591:;
      // end
      _get64 = digits;
      _get65 = l;
      digitsPtr = _get64 + _get65;
      _get66 = outPtr;
      _get67 = l;
      endPtr = _get66 + _get67;
      // loop 
      j592:;
        _get68 = outPtr;
        _get69 = endPtr;
        // if 
          if (_get68 < _get69) {
            _get70 = digitsPtr;
            digitsPtr = _get70 - 1;
            _get71 = digitsPtr;
            digit = (f64)(i32_load8_u(0, 4, (i32)(_get71)));
            _get72 = digit;
            // if 
              if (_get72 < 10) {
                _get73 = digit;
                digit = _get73 + 48;
              } else {
                _get74 = digit;
                digit = _get74 + 87;
              }
            // end
            j594:;
            _get75 = outPtr;
            _get76 = outPtr;
            outPtr = _get76 + 1;
            _get77 = digit;
            i32_store8(0, 4, (i32)(_get75), (i32)(_get77));
            goto j592;
          }
        // end
        j593:;
      // end
    }
  // end
  j587:;
  _get78 = out;
  _get79 = outPtr;
  _get80 = out;
  i32_store(1, 0, (u32)(_get78), (u32)((_get79 - _get80)));
  _get81 = out;
  return (struct ReturnValue){ _get81, 195 };
}

static struct ReturnValue dong_porf_js_microbench__String_fromCharCode(f64 codes, i32 codesjjtype) {
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

  out = (f64)(dong_porf_js_microbench__Porffor_malloc(16384));
  _get0 = codes;
  len = (f64)(i32_load(1, 0, (u32)(_get0)));
  _get1 = out;
  _get2 = len;
  i32_store(1, 0, (u32)(_get1), (u32)(_get2));
  bytestringable = 1;
  i = 0;
  // loop 
  j623:;
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
        v = dong_porf_js_microbench__ecma262_ToIntegerOrInfinity(f64_load(0, 4, _get9), _get11);
        _get12 = v;
        // if 
          if (_get12 > 255) {
            bytestringable = 0;
          }
        // end
        j625:;
        _get13 = out;
        _get14 = i;
        _get15 = v;
        i32_store16(0, 4, (i32)((_get13 + (_get14 * 2))), (i32)(_get15));
        _get16 = i;
        i = _get16 + 1;
        goto j623;
      }
    // end
    j624:;
  // end
  _get17 = bytestringable;
  // if 
    if (((u32)(_get17)) != 0) {
      _get18 = out;
      out2 = _get18;
      i = 0;
      // loop 
      j627:;
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
            goto j627;
          }
        // end
        j628:;
      // end
      _get26 = out2;
      return (struct ReturnValue){ _get26, 195 };
    }
  // end
  j626:;
  _get27 = out;
  return (struct ReturnValue){ _get27, 67 };
}

static struct ReturnValue dong_porf_js_microbench_utf8AppendCodePoint(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 out, i32 outjjtype, f64 cp, i32 cpjjtype) {
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
      f64 _r622;
        _get1 = out;
        __tmpop_left = _get1;
        _get2 = __tmpop_left;
        _get3 = cp;
        f64_store(0, 4, 65536, _get3);
        _get4 = cpjjtype;
        i32_store8(0, 12, 65536, _get4);
        i32_store(1, 0, 65536, 1);
        const struct ReturnValue _0 = dong_porf_js_microbench__String_fromCharCode(65536, 72);
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
            const struct ReturnValue _1 = dong_porf_js_microbench__Porffor_concatStrings(_get8, _get9, _get10, _get11);
            jjlast_type = _1.type;
            _r622 = _1.value;
            goto j622;
          }
        // end
        j629:;
        jjlast_type = 1;
        _r622 = _get2 + _get5;
      // end
      j622:;
      jjreturn = _r622;
      _get12 = jjlast_type;
      jjreturnjjtype = _get12;
      _get13 = jjnewtarget;
      // if 
        if (((u32)(_get13)) != 0) {
          _get14 = jjreturn;
          _get15 = jjreturnjjtype;
          // if 
            if ((dong_porf_js_microbench__Porffor_object_isObject((i32)(_get14), _get15)) == 0) {
              _get16 = jjthis;
              _get17 = jjthisjjtype;
              return (struct ReturnValue){ _get16, _get17 };
            }
          // end
          j631:;
        }
      // end
      j630:;
      _get18 = jjreturn;
      _get19 = jjreturnjjtype;
      return (struct ReturnValue){ _get18, _get19 };
    }
  // end
  j621:;
  _get20 = cp;
  // if 
    if ((f64)(_get20 < 2048) != 0) {
      // block f64
      f64 _r633;
        _get21 = out;
        __tmpop_left = _get21;
        _get22 = __tmpop_left;
        jjbitwise_left = 192;
        _get23 = jjbitwise_left;
        _get24 = jjbitwise_left;
        // if i32
        i32 _r634;
          if ((_get23 - _get24) == 0) {
            _get25 = jjbitwise_left;
            _r634 = (i32)((i64)(_get25));
          } else {
            _r634 = 0;
          }
        // end
        j634:;
        _get26 = cp;
        jjbitwise_left = _get26;
        _get27 = jjbitwise_left;
        _get28 = jjbitwise_left;
        // if i32
        i32 _r635;
          if ((_get27 - _get28) == 0) {
            _get29 = jjbitwise_left;
            _r635 = (i32)((i64)(_get29));
          } else {
            _r635 = 0;
          }
        // end
        j635:;
        jjbitwise_right = 6;
        _get30 = jjbitwise_right;
        _get31 = jjbitwise_right;
        // if i32
        i32 _r636;
          if ((_get30 - _get31) == 0) {
            _get32 = jjbitwise_right;
            _r636 = (i32)((i64)(_get32));
          } else {
            _r636 = 0;
          }
        // end
        j636:;
        jjbitwise_right = (f64)(_r635 >> _r636);
        _get33 = jjbitwise_right;
        _get34 = jjbitwise_right;
        // if i32
        i32 _r637;
          if ((_get33 - _get34) == 0) {
            _get35 = jjbitwise_right;
            _r637 = (i32)((i64)(_get35));
          } else {
            _r637 = 0;
          }
        // end
        j637:;
        f64_store(0, 4, 81920, (f64)(_r634 | _r637));
        i32_store8(0, 12, 81920, 1);
        jjbitwise_left = 128;
        _get36 = jjbitwise_left;
        _get37 = jjbitwise_left;
        // if i32
        i32 _r638;
          if ((_get36 - _get37) == 0) {
            _get38 = jjbitwise_left;
            _r638 = (i32)((i64)(_get38));
          } else {
            _r638 = 0;
          }
        // end
        j638:;
        _get39 = cp;
        jjbitwise_left = _get39;
        _get40 = jjbitwise_left;
        _get41 = jjbitwise_left;
        // if i32
        i32 _r639;
          if ((_get40 - _get41) == 0) {
            _get42 = jjbitwise_left;
            _r639 = (i32)((i64)(_get42));
          } else {
            _r639 = 0;
          }
        // end
        j639:;
        jjbitwise_right = 63;
        _get43 = jjbitwise_right;
        _get44 = jjbitwise_right;
        // if i32
        i32 _r640;
          if ((_get43 - _get44) == 0) {
            _get45 = jjbitwise_right;
            _r640 = (i32)((i64)(_get45));
          } else {
            _r640 = 0;
          }
        // end
        j640:;
        jjbitwise_right = (f64)(_r639 & _r640);
        _get46 = jjbitwise_right;
        _get47 = jjbitwise_right;
        // if i32
        i32 _r641;
          if ((_get46 - _get47) == 0) {
            _get48 = jjbitwise_right;
            _r641 = (i32)((i64)(_get48));
          } else {
            _r641 = 0;
          }
        // end
        j641:;
        f64_store(0, 13, 81920, (f64)(_r638 | _r641));
        i32_store8(0, 21, 81920, 1);
        i32_store(1, 0, 81920, 2);
        const struct ReturnValue _2 = dong_porf_js_microbench__String_fromCharCode(81920, 72);
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
            const struct ReturnValue _3 = dong_porf_js_microbench__Porffor_concatStrings(_get52, _get53, _get54, _get55);
            jjlast_type = _3.type;
            _r633 = _3.value;
            goto j633;
          }
        // end
        j642:;
        jjlast_type = 1;
        _r633 = _get22 + _get49;
      // end
      j633:;
      jjreturn = _r633;
      _get56 = jjlast_type;
      jjreturnjjtype = _get56;
      _get57 = jjnewtarget;
      // if 
        if (((u32)(_get57)) != 0) {
          _get58 = jjreturn;
          _get59 = jjreturnjjtype;
          // if 
            if ((dong_porf_js_microbench__Porffor_object_isObject((i32)(_get58), _get59)) == 0) {
              _get60 = jjthis;
              _get61 = jjthisjjtype;
              return (struct ReturnValue){ _get60, _get61 };
            }
          // end
          j644:;
        }
      // end
      j643:;
      _get62 = jjreturn;
      _get63 = jjreturnjjtype;
      return (struct ReturnValue){ _get62, _get63 };
    }
  // end
  j632:;
  _get64 = cp;
  // if 
    if ((f64)(_get64 < 65536) != 0) {
      // block f64
      f64 _r646;
        _get65 = out;
        __tmpop_left = _get65;
        _get66 = __tmpop_left;
        jjbitwise_left = 224;
        _get67 = jjbitwise_left;
        _get68 = jjbitwise_left;
        // if i32
        i32 _r647;
          if ((_get67 - _get68) == 0) {
            _get69 = jjbitwise_left;
            _r647 = (i32)((i64)(_get69));
          } else {
            _r647 = 0;
          }
        // end
        j647:;
        _get70 = cp;
        jjbitwise_left = _get70;
        _get71 = jjbitwise_left;
        _get72 = jjbitwise_left;
        // if i32
        i32 _r648;
          if ((_get71 - _get72) == 0) {
            _get73 = jjbitwise_left;
            _r648 = (i32)((i64)(_get73));
          } else {
            _r648 = 0;
          }
        // end
        j648:;
        jjbitwise_right = 12;
        _get74 = jjbitwise_right;
        _get75 = jjbitwise_right;
        // if i32
        i32 _r649;
          if ((_get74 - _get75) == 0) {
            _get76 = jjbitwise_right;
            _r649 = (i32)((i64)(_get76));
          } else {
            _r649 = 0;
          }
        // end
        j649:;
        jjbitwise_right = (f64)(_r648 >> _r649);
        _get77 = jjbitwise_right;
        _get78 = jjbitwise_right;
        // if i32
        i32 _r650;
          if ((_get77 - _get78) == 0) {
            _get79 = jjbitwise_right;
            _r650 = (i32)((i64)(_get79));
          } else {
            _r650 = 0;
          }
        // end
        j650:;
        f64_store(0, 4, 98304, (f64)(_r647 | _r650));
        i32_store8(0, 12, 98304, 1);
        jjbitwise_left = 128;
        _get80 = jjbitwise_left;
        _get81 = jjbitwise_left;
        // if i32
        i32 _r651;
          if ((_get80 - _get81) == 0) {
            _get82 = jjbitwise_left;
            _r651 = (i32)((i64)(_get82));
          } else {
            _r651 = 0;
          }
        // end
        j651:;
        _get83 = cp;
        jjbitwise_left = _get83;
        _get84 = jjbitwise_left;
        _get85 = jjbitwise_left;
        // if i32
        i32 _r652;
          if ((_get84 - _get85) == 0) {
            _get86 = jjbitwise_left;
            _r652 = (i32)((i64)(_get86));
          } else {
            _r652 = 0;
          }
        // end
        j652:;
        jjbitwise_right = 6;
        _get87 = jjbitwise_right;
        _get88 = jjbitwise_right;
        // if i32
        i32 _r653;
          if ((_get87 - _get88) == 0) {
            _get89 = jjbitwise_right;
            _r653 = (i32)((i64)(_get89));
          } else {
            _r653 = 0;
          }
        // end
        j653:;
        jjbitwise_left = (f64)(_r652 >> _r653);
        _get90 = jjbitwise_left;
        _get91 = jjbitwise_left;
        // if i32
        i32 _r654;
          if ((_get90 - _get91) == 0) {
            _get92 = jjbitwise_left;
            _r654 = (i32)((i64)(_get92));
          } else {
            _r654 = 0;
          }
        // end
        j654:;
        jjbitwise_right = 63;
        _get93 = jjbitwise_right;
        _get94 = jjbitwise_right;
        // if i32
        i32 _r655;
          if ((_get93 - _get94) == 0) {
            _get95 = jjbitwise_right;
            _r655 = (i32)((i64)(_get95));
          } else {
            _r655 = 0;
          }
        // end
        j655:;
        jjbitwise_right = (f64)(_r654 & _r655);
        _get96 = jjbitwise_right;
        _get97 = jjbitwise_right;
        // if i32
        i32 _r656;
          if ((_get96 - _get97) == 0) {
            _get98 = jjbitwise_right;
            _r656 = (i32)((i64)(_get98));
          } else {
            _r656 = 0;
          }
        // end
        j656:;
        f64_store(0, 13, 98304, (f64)(_r651 | _r656));
        i32_store8(0, 21, 98304, 1);
        jjbitwise_left = 128;
        _get99 = jjbitwise_left;
        _get100 = jjbitwise_left;
        // if i32
        i32 _r657;
          if ((_get99 - _get100) == 0) {
            _get101 = jjbitwise_left;
            _r657 = (i32)((i64)(_get101));
          } else {
            _r657 = 0;
          }
        // end
        j657:;
        _get102 = cp;
        jjbitwise_left = _get102;
        _get103 = jjbitwise_left;
        _get104 = jjbitwise_left;
        // if i32
        i32 _r658;
          if ((_get103 - _get104) == 0) {
            _get105 = jjbitwise_left;
            _r658 = (i32)((i64)(_get105));
          } else {
            _r658 = 0;
          }
        // end
        j658:;
        jjbitwise_right = 63;
        _get106 = jjbitwise_right;
        _get107 = jjbitwise_right;
        // if i32
        i32 _r659;
          if ((_get106 - _get107) == 0) {
            _get108 = jjbitwise_right;
            _r659 = (i32)((i64)(_get108));
          } else {
            _r659 = 0;
          }
        // end
        j659:;
        jjbitwise_right = (f64)(_r658 & _r659);
        _get109 = jjbitwise_right;
        _get110 = jjbitwise_right;
        // if i32
        i32 _r660;
          if ((_get109 - _get110) == 0) {
            _get111 = jjbitwise_right;
            _r660 = (i32)((i64)(_get111));
          } else {
            _r660 = 0;
          }
        // end
        j660:;
        f64_store(0, 22, 98304, (f64)(_r657 | _r660));
        i32_store8(0, 30, 98304, 1);
        i32_store(1, 0, 98304, 3);
        const struct ReturnValue _4 = dong_porf_js_microbench__String_fromCharCode(98304, 72);
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
            const struct ReturnValue _5 = dong_porf_js_microbench__Porffor_concatStrings(_get115, _get116, _get117, _get118);
            jjlast_type = _5.type;
            _r646 = _5.value;
            goto j646;
          }
        // end
        j661:;
        jjlast_type = 1;
        _r646 = _get66 + _get112;
      // end
      j646:;
      jjreturn = _r646;
      _get119 = jjlast_type;
      jjreturnjjtype = _get119;
      _get120 = jjnewtarget;
      // if 
        if (((u32)(_get120)) != 0) {
          _get121 = jjreturn;
          _get122 = jjreturnjjtype;
          // if 
            if ((dong_porf_js_microbench__Porffor_object_isObject((i32)(_get121), _get122)) == 0) {
              _get123 = jjthis;
              _get124 = jjthisjjtype;
              return (struct ReturnValue){ _get123, _get124 };
            }
          // end
          j663:;
        }
      // end
      j662:;
      _get125 = jjreturn;
      _get126 = jjreturnjjtype;
      return (struct ReturnValue){ _get125, _get126 };
    }
  // end
  j645:;
  // block f64
  f64 _r664;
    _get127 = out;
    __tmpop_left = _get127;
    _get128 = __tmpop_left;
    jjbitwise_left = 240;
    _get129 = jjbitwise_left;
    _get130 = jjbitwise_left;
    // if i32
    i32 _r665;
      if ((_get129 - _get130) == 0) {
        _get131 = jjbitwise_left;
        _r665 = (i32)((i64)(_get131));
      } else {
        _r665 = 0;
      }
    // end
    j665:;
    _get132 = cp;
    jjbitwise_left = _get132;
    _get133 = jjbitwise_left;
    _get134 = jjbitwise_left;
    // if i32
    i32 _r666;
      if ((_get133 - _get134) == 0) {
        _get135 = jjbitwise_left;
        _r666 = (i32)((i64)(_get135));
      } else {
        _r666 = 0;
      }
    // end
    j666:;
    jjbitwise_right = 18;
    _get136 = jjbitwise_right;
    _get137 = jjbitwise_right;
    // if i32
    i32 _r667;
      if ((_get136 - _get137) == 0) {
        _get138 = jjbitwise_right;
        _r667 = (i32)((i64)(_get138));
      } else {
        _r667 = 0;
      }
    // end
    j667:;
    jjbitwise_right = (f64)(_r666 >> _r667);
    _get139 = jjbitwise_right;
    _get140 = jjbitwise_right;
    // if i32
    i32 _r668;
      if ((_get139 - _get140) == 0) {
        _get141 = jjbitwise_right;
        _r668 = (i32)((i64)(_get141));
      } else {
        _r668 = 0;
      }
    // end
    j668:;
    f64_store(0, 4, 114688, (f64)(_r665 | _r668));
    i32_store8(0, 12, 114688, 1);
    jjbitwise_left = 128;
    _get142 = jjbitwise_left;
    _get143 = jjbitwise_left;
    // if i32
    i32 _r669;
      if ((_get142 - _get143) == 0) {
        _get144 = jjbitwise_left;
        _r669 = (i32)((i64)(_get144));
      } else {
        _r669 = 0;
      }
    // end
    j669:;
    _get145 = cp;
    jjbitwise_left = _get145;
    _get146 = jjbitwise_left;
    _get147 = jjbitwise_left;
    // if i32
    i32 _r670;
      if ((_get146 - _get147) == 0) {
        _get148 = jjbitwise_left;
        _r670 = (i32)((i64)(_get148));
      } else {
        _r670 = 0;
      }
    // end
    j670:;
    jjbitwise_right = 12;
    _get149 = jjbitwise_right;
    _get150 = jjbitwise_right;
    // if i32
    i32 _r671;
      if ((_get149 - _get150) == 0) {
        _get151 = jjbitwise_right;
        _r671 = (i32)((i64)(_get151));
      } else {
        _r671 = 0;
      }
    // end
    j671:;
    jjbitwise_left = (f64)(_r670 >> _r671);
    _get152 = jjbitwise_left;
    _get153 = jjbitwise_left;
    // if i32
    i32 _r672;
      if ((_get152 - _get153) == 0) {
        _get154 = jjbitwise_left;
        _r672 = (i32)((i64)(_get154));
      } else {
        _r672 = 0;
      }
    // end
    j672:;
    jjbitwise_right = 63;
    _get155 = jjbitwise_right;
    _get156 = jjbitwise_right;
    // if i32
    i32 _r673;
      if ((_get155 - _get156) == 0) {
        _get157 = jjbitwise_right;
        _r673 = (i32)((i64)(_get157));
      } else {
        _r673 = 0;
      }
    // end
    j673:;
    jjbitwise_right = (f64)(_r672 & _r673);
    _get158 = jjbitwise_right;
    _get159 = jjbitwise_right;
    // if i32
    i32 _r674;
      if ((_get158 - _get159) == 0) {
        _get160 = jjbitwise_right;
        _r674 = (i32)((i64)(_get160));
      } else {
        _r674 = 0;
      }
    // end
    j674:;
    f64_store(0, 13, 114688, (f64)(_r669 | _r674));
    i32_store8(0, 21, 114688, 1);
    jjbitwise_left = 128;
    _get161 = jjbitwise_left;
    _get162 = jjbitwise_left;
    // if i32
    i32 _r675;
      if ((_get161 - _get162) == 0) {
        _get163 = jjbitwise_left;
        _r675 = (i32)((i64)(_get163));
      } else {
        _r675 = 0;
      }
    // end
    j675:;
    _get164 = cp;
    jjbitwise_left = _get164;
    _get165 = jjbitwise_left;
    _get166 = jjbitwise_left;
    // if i32
    i32 _r676;
      if ((_get165 - _get166) == 0) {
        _get167 = jjbitwise_left;
        _r676 = (i32)((i64)(_get167));
      } else {
        _r676 = 0;
      }
    // end
    j676:;
    jjbitwise_right = 6;
    _get168 = jjbitwise_right;
    _get169 = jjbitwise_right;
    // if i32
    i32 _r677;
      if ((_get168 - _get169) == 0) {
        _get170 = jjbitwise_right;
        _r677 = (i32)((i64)(_get170));
      } else {
        _r677 = 0;
      }
    // end
    j677:;
    jjbitwise_left = (f64)(_r676 >> _r677);
    _get171 = jjbitwise_left;
    _get172 = jjbitwise_left;
    // if i32
    i32 _r678;
      if ((_get171 - _get172) == 0) {
        _get173 = jjbitwise_left;
        _r678 = (i32)((i64)(_get173));
      } else {
        _r678 = 0;
      }
    // end
    j678:;
    jjbitwise_right = 63;
    _get174 = jjbitwise_right;
    _get175 = jjbitwise_right;
    // if i32
    i32 _r679;
      if ((_get174 - _get175) == 0) {
        _get176 = jjbitwise_right;
        _r679 = (i32)((i64)(_get176));
      } else {
        _r679 = 0;
      }
    // end
    j679:;
    jjbitwise_right = (f64)(_r678 & _r679);
    _get177 = jjbitwise_right;
    _get178 = jjbitwise_right;
    // if i32
    i32 _r680;
      if ((_get177 - _get178) == 0) {
        _get179 = jjbitwise_right;
        _r680 = (i32)((i64)(_get179));
      } else {
        _r680 = 0;
      }
    // end
    j680:;
    f64_store(0, 22, 114688, (f64)(_r675 | _r680));
    i32_store8(0, 30, 114688, 1);
    jjbitwise_left = 128;
    _get180 = jjbitwise_left;
    _get181 = jjbitwise_left;
    // if i32
    i32 _r681;
      if ((_get180 - _get181) == 0) {
        _get182 = jjbitwise_left;
        _r681 = (i32)((i64)(_get182));
      } else {
        _r681 = 0;
      }
    // end
    j681:;
    _get183 = cp;
    jjbitwise_left = _get183;
    _get184 = jjbitwise_left;
    _get185 = jjbitwise_left;
    // if i32
    i32 _r682;
      if ((_get184 - _get185) == 0) {
        _get186 = jjbitwise_left;
        _r682 = (i32)((i64)(_get186));
      } else {
        _r682 = 0;
      }
    // end
    j682:;
    jjbitwise_right = 63;
    _get187 = jjbitwise_right;
    _get188 = jjbitwise_right;
    // if i32
    i32 _r683;
      if ((_get187 - _get188) == 0) {
        _get189 = jjbitwise_right;
        _r683 = (i32)((i64)(_get189));
      } else {
        _r683 = 0;
      }
    // end
    j683:;
    jjbitwise_right = (f64)(_r682 & _r683);
    _get190 = jjbitwise_right;
    _get191 = jjbitwise_right;
    // if i32
    i32 _r684;
      if ((_get190 - _get191) == 0) {
        _get192 = jjbitwise_right;
        _r684 = (i32)((i64)(_get192));
      } else {
        _r684 = 0;
      }
    // end
    j684:;
    f64_store(0, 31, 114688, (f64)(_r681 | _r684));
    i32_store8(0, 39, 114688, 1);
    i32_store(1, 0, 114688, 4);
    const struct ReturnValue _6 = dong_porf_js_microbench__String_fromCharCode(114688, 72);
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
        const struct ReturnValue _7 = dong_porf_js_microbench__Porffor_concatStrings(_get196, _get197, _get198, _get199);
        jjlast_type = _7.type;
        _r664 = _7.value;
        goto j664;
      }
    // end
    j685:;
    jjlast_type = 1;
    _r664 = _get128 + _get193;
  // end
  j664:;
  jjreturn = _r664;
  _get200 = jjlast_type;
  jjreturnjjtype = _get200;
  _get201 = jjnewtarget;
  // if 
    if (((u32)(_get201)) != 0) {
      _get202 = jjreturn;
      _get203 = jjreturnjjtype;
      // if 
        if ((dong_porf_js_microbench__Porffor_object_isObject((i32)(_get202), _get203)) == 0) {
          _get204 = jjthis;
          _get205 = jjthisjjtype;
          return (struct ReturnValue){ _get204, _get205 };
        }
      // end
      j687:;
    }
  // end
  j686:;
  _get206 = jjreturn;
  _get207 = jjreturnjjtype;
  return (struct ReturnValue){ _get206, _get207 };
}

static struct ReturnValue dong_porf_js_microbench_toUtf8(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 s, i32 sjjtype) {
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
  f64 jjmember_obj_35 = 0;
  f64 jjmember_prop_35 = 0;
  f64 cp = 0;
  i32 cpjjtype = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  f64 jjindirect_36_callee = 0;
  f64 jjindirect_36_caller = 0;
  i32 jjindirect_36_callerjjtype = 0;
  f64 jjmember_obj_37 = 0;
  f64 jjmember_prop_37 = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 logictmpi = 0;
  i32 jjlogicinner_tmp_int = 0;
  f64 next = 0;
  i32 nextjjtype = 0;
  f64 jjindirect_38_callee = 0;
  f64 jjindirect_38_caller = 0;
  i32 jjindirect_38_callerjjtype = 0;
  f64 jjmember_obj_39 = 0;
  f64 jjmember_prop_39 = 0;
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
  f64 _r595;
    if ((_get1 & 64) != 0) {
      _get2 = jjlength_tmp;
      jjlast_type = 1;
      _r595 = (f64)(i32_load(1, 0, _get2));
    } else {
      jjmember_prop_35 = 44;
      _get3 = s;
      jjmember_obj_35 = _get3;
      _get4 = sjjtype;
      // if f64
      f64 _r596;
        if (_get4 == 0) {
          _r596 = 0;
        } else {
          _get5 = jjmember_obj_35;
          _get6 = sjjtype;
          _get7 = jjmember_prop_35;
          const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_get_withHash((i32)(_get5), _get6, (u32)(_get7), 195, -2086110260, 1);
          jjlast_type = _0.type;
          _r596 = _0.value;
        }
      // end
      j596:;
      _r595 = _r596;
    }
  // end
  j595:;
  len = _r595;
  _get8 = jjlast_type;
  lenjjtype = _get8;
  // loop 
  j597:;
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
        f64 _r599;
          _get14 = jjtypeswitch_tmp1;
          // if 
            if (_get14 == 33) {
              _get15 = jjproto_target;
              _get16 = jjproto_targetjjtype;
              _get17 = i;
              _get18 = ijjtype;
              const struct ReturnValue _1 = dong_porf_js_microbench__String_prototype_charCodeAt(_get15, _get16, _get17, _get18);
              jjlast_type = _1.type;
              _r599 = _1.value;
              goto j599;
            }
          // end
          j600:;
          _get19 = jjtypeswitch_tmp1;
          // if 
            if (_get19 == 67) {
              _get20 = jjproto_target;
              _get21 = jjproto_targetjjtype;
              _get22 = i;
              _get23 = ijjtype;
              const struct ReturnValue _2 = dong_porf_js_microbench__String_prototype_charCodeAt(_get20, _get21, _get22, _get23);
              jjlast_type = _2.type;
              _r599 = _2.value;
              goto j599;
            }
          // end
          j601:;
          _get24 = jjtypeswitch_tmp1;
          // if 
            if (_get24 == 195) {
              _get25 = jjproto_target;
              _get26 = jjproto_targetjjtype;
              _get27 = i;
              _get28 = ijjtype;
              const struct ReturnValue _3 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get25, _get26, _get27, _get28);
              jjlast_type = _3.type;
              _r599 = _3.value;
              goto j599;
            }
          // end
          j602:;
          jjmember_prop_37 = 1939;
          _get29 = s;
          jjindirect_36_caller = _get29;
          _get30 = jjindirect_36_caller;
          _get31 = sjjtype;
          jjindirect_36_callerjjtype = _get31;
          jjmember_obj_37 = _get30;
          _get32 = jjindirect_36_callerjjtype;
          // if f64
          f64 _r603;
            if (_get32 == 0) {
              _r603 = 0;
            } else {
              _get33 = jjmember_obj_37;
              _get34 = jjindirect_36_callerjjtype;
              _get35 = jjmember_prop_37;
              const struct ReturnValue _4 = dong_porf_js_microbench__Porffor_object_get_withHash((i32)(_get33), _get34, (u32)(_get35), 195, -1592872053, 1);
              jjlast_type = _4.type;
              _r603 = _4.value;
            }
          // end
          j603:;
          jjindirect_36_callee = _r603;
          _get36 = jjlast_type;
          // if f64
          f64 _r604;
            if (_get36 == 6) {
              _get37 = jjindirect_36_caller;
              _get38 = jjindirect_36_callerjjtype;
              _get39 = i;
              _get40 = ijjtype;
              _get41 = jjindirect_36_callee;
              jjlast_type = 0;
              _r604 = 0;
            } else {
              _r604 = 0;
            }
          // end
          j604:;
          _r599 = _r604;
        // end
        j599:;
        cp = _r599;
        _get42 = jjlast_type;
        cpjjtype = _get42;
        _get43 = cp;
        logictmpi = _get43 >= 55296;
        _get44 = logictmpi;
        // if i32
        i32 _r605;
          if (_get44 != 0) {
            _get45 = cp;
            jjlast_type = 2;
            _r605 = _get45 <= 56319;
          } else {
            _get46 = logictmpi;
            jjlast_type = 2;
            _r605 = _get46;
          }
        // end
        j605:;
        logictmpi = _r605;
        _get47 = logictmpi;
        jjlogicinner_tmp_int = _get47;
        _get48 = jjlast_type;
        jjtypeswitch_tmp1 = _get48;
        // block i32
        i32 _r606;
          _get49 = jjtypeswitch_tmp1;
          _get50 = jjtypeswitch_tmp1;
          // if 
            if (((_get49 == 67) | (_get50 == 195)) != 0) {
              _get51 = jjlogicinner_tmp_int;
              _r606 = i32_load(1, 0, _get51);
              goto j606;
            }
          // end
          j607:;
          _get52 = jjtypeswitch_tmp1;
          _get53 = jjtypeswitch_tmp1;
          // if 
            if (((_get52 == 31) | (_get53 == 32)) != 0) {
              _r606 = 1;
              goto j606;
            }
          // end
          j608:;
          _get54 = jjlogicinner_tmp_int;
          _r606 = _get54 != 0;
        // end
        j606:;
        // if i32
        i32 _r609;
          if ((_r606) != 0) {
            _get55 = i;
            _get56 = len;
            jjlast_type = 2;
            _r609 = (_get55 + 1) < _get56;
          } else {
            _get57 = logictmpi;
            _get58 = jjlast_type;
            jjlast_type = _get58;
            _r609 = _get57;
          }
        // end
        j609:;
        // if 
          if ((f64)(_r609) != 0) {
            _get59 = s;
            jjproto_target = _get59;
            _get60 = sjjtype;
            jjproto_targetjjtype = _get60;
            _get61 = sjjtype;
            jjtypeswitch_tmp1 = _get61;
            // block f64
            f64 _r611;
              _get62 = jjtypeswitch_tmp1;
              // if 
                if (_get62 == 33) {
                  _get63 = jjproto_target;
                  _get64 = jjproto_targetjjtype;
                  _get65 = i;
                  const struct ReturnValue _5 = dong_porf_js_microbench__String_prototype_charCodeAt(_get63, _get64, _get65 + 1, 1);
                  jjlast_type = _5.type;
                  _r611 = _5.value;
                  goto j611;
                }
              // end
              j612:;
              _get66 = jjtypeswitch_tmp1;
              // if 
                if (_get66 == 67) {
                  _get67 = jjproto_target;
                  _get68 = jjproto_targetjjtype;
                  _get69 = i;
                  const struct ReturnValue _6 = dong_porf_js_microbench__String_prototype_charCodeAt(_get67, _get68, _get69 + 1, 1);
                  jjlast_type = _6.type;
                  _r611 = _6.value;
                  goto j611;
                }
              // end
              j613:;
              _get70 = jjtypeswitch_tmp1;
              // if 
                if (_get70 == 195) {
                  _get71 = jjproto_target;
                  _get72 = jjproto_targetjjtype;
                  _get73 = i;
                  const struct ReturnValue _7 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get71, _get72, _get73 + 1, 1);
                  jjlast_type = _7.type;
                  _r611 = _7.value;
                  goto j611;
                }
              // end
              j614:;
              jjmember_prop_39 = 1939;
              _get74 = s;
              jjindirect_38_caller = _get74;
              _get75 = jjindirect_38_caller;
              _get76 = sjjtype;
              jjindirect_38_callerjjtype = _get76;
              jjmember_obj_39 = _get75;
              _get77 = jjindirect_38_callerjjtype;
              // if f64
              f64 _r615;
                if (_get77 == 0) {
                  _r615 = 0;
                } else {
                  _get78 = jjmember_obj_39;
                  _get79 = jjindirect_38_callerjjtype;
                  _get80 = jjmember_prop_39;
                  const struct ReturnValue _8 = dong_porf_js_microbench__Porffor_object_get_withHash((i32)(_get78), _get79, (u32)(_get80), 195, -1592872053, 1);
                  jjlast_type = _8.type;
                  _r615 = _8.value;
                }
              // end
              j615:;
              jjindirect_38_callee = _r615;
              _get81 = jjlast_type;
              // if f64
              f64 _r616;
                if (_get81 == 6) {
                  _get82 = jjindirect_38_caller;
                  _get83 = jjindirect_38_callerjjtype;
                  _get84 = i;
                  _get85 = jjindirect_38_callee;
                  jjlast_type = 0;
                  _r616 = 0;
                } else {
                  _r616 = 0;
                }
              // end
              j616:;
              _r611 = _r616;
            // end
            j611:;
            next = _r611;
            _get86 = jjlast_type;
            nextjjtype = _get86;
            _get87 = next;
            logictmpi = _get87 >= 56320;
            _get88 = logictmpi;
            // if i32
            i32 _r617;
              if (_get88 != 0) {
                _get89 = next;
                jjlast_type = 2;
                _r617 = _get89 <= 57343;
              } else {
                _get90 = logictmpi;
                jjlast_type = 2;
                _r617 = _get90;
              }
            // end
            j617:;
            // if 
              if ((f64)(_r617) != 0) {
                _get91 = cp;
                jjbitwise_left = _get91 - 55296;
                _get92 = jjbitwise_left;
                _get93 = jjbitwise_left;
                // if i32
                i32 _r619;
                  if ((_get92 - _get93) == 0) {
                    _get94 = jjbitwise_left;
                    _r619 = (i32)((i64)(_get94));
                  } else {
                    _r619 = 0;
                  }
                // end
                j619:;
                jjbitwise_right = 10;
                _get95 = jjbitwise_right;
                _get96 = jjbitwise_right;
                // if i32
                i32 _r620;
                  if ((_get95 - _get96) == 0) {
                    _get97 = jjbitwise_right;
                    _r620 = (i32)((i64)(_get97));
                  } else {
                    _r620 = 0;
                  }
                // end
                j620:;
                _get98 = next;
                cp = (65536 + (f64)(_r619 << _r620)) + (_get98 - 56320);
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
            j618:;
          }
        // end
        j610:;
        _get102 = out;
        _get103 = outjjtype;
        _get104 = cp;
        _get105 = cpjjtype;
        const struct ReturnValue _9 = dong_porf_js_microbench_utf8AppendCodePoint(0, 0, 0, 0, _get102, _get103, _get104, _get105);
        jjlast_type = _9.type;
        _get106 = jjlast_type;
        outjjtype = _get106;
        out = _9.value;
        _get107 = i;
        i = _get107 + 1;
        _get108 = i;
        ijjtype = 1;
        (void) _get108;
        goto j597;
      }
    // end
    j598:;
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
        if ((dong_porf_js_microbench__Porffor_object_isObject((i32)(_get111), _get112)) == 0) {
          _get113 = jjthis;
          _get114 = jjthisjjtype;
          return (struct ReturnValue){ _get113, _get114 };
        }
      // end
      j689:;
    }
  // end
  j688:;
  _get115 = jjreturn;
  _get116 = jjreturnjjtype;
  return (struct ReturnValue){ _get115, _get116 };
}

static struct ReturnValue dong_porf_js_microbench_benchLog(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 msg, i32 msgjjtype) {
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = msg;
  _get1 = msgjjtype;
  const struct ReturnValue _0 = dong_porf_js_microbench_toUtf8(0, 0, 0, 0, _get0, _get1);
  (void) _0.type;
  __porf_import_dong_bench_log(_0.value);
  _get2 = jjnewtarget;
  // if 
    if (((u32)(_get2)) != 0) {
      _get3 = jjthis;
      _get4 = jjthisjjtype;
      return (struct ReturnValue){ _get3, _get4 };
    }
  // end
  j690:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_js_microbench_bench_finish(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get16;
  f64 _get15;
  f64 _get14;
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
  f64 _get0;
  f64 elapsed = 0;
  i32 elapsedjjtype = 0;
  f64 nsPerIter = 0;
  i32 nsPerIterjjtype = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  f64 jjindirect_33_callee = 0;
  f64 jjindirect_33_caller = 0;
  i32 jjindirect_33_callerjjtype = 0;
  f64 jjmember_obj_34 = 0;
  f64 jjmember_prop_34 = 0;
  i32 jjlast_type = 0;
  f64 line = 0;
  i32 linejjtype = 0;

  elapsed = __porf_import_dong_time_now() - dong_porf_js_microbench_bench_start;
  elapsedjjtype = 1;
  _get0 = elapsed;
  jjproto_target = (_get0 * 1000000) / dong_porf_js_microbench_bench_iterations;
  jjproto_targetjjtype = 1;
  _get1 = jjproto_target;
  _get2 = jjproto_targetjjtype;
  const struct ReturnValue _0 = dong_porf_js_microbench__Number_prototype_toFixed(_get1, _get2, 1, 1);
  jjlast_type = _0.type;
  _get3 = jjlast_type;
  nsPerIterjjtype = _get3;
  nsPerIter = _0.value;
  const struct ReturnValue _1 = dong_porf_js_microbench__Porffor_concatStrings(1881, 195, dong_porf_js_microbench_bench_name, dong_porf_js_microbench_bench_namejjtype);
  jjlast_type = _1.type;
  _get4 = jjlast_type;
  const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_concatStrings(_1.value, _get4, 312, 195);
  jjlast_type = _2.type;
  _get5 = elapsed;
  const struct ReturnValue _3 = dong_porf_js_microbench__Porffor_concatStrings(_2.value, 195, _get5, 1);
  jjlast_type = _3.type;
  _get6 = jjlast_type;
  const struct ReturnValue _4 = dong_porf_js_microbench__Porffor_concatStrings(_3.value, _get6, 1895, 195);
  jjlast_type = _4.type;
  const struct ReturnValue _5 = dong_porf_js_microbench__Porffor_concatStrings(_4.value, 195, dong_porf_js_microbench_bench_iterations, dong_porf_js_microbench_bench_iterationsjjtype);
  jjlast_type = _5.type;
  _get7 = jjlast_type;
  const struct ReturnValue _6 = dong_porf_js_microbench__Porffor_concatStrings(_5.value, _get7, 1905, 195);
  jjlast_type = _6.type;
  _get8 = nsPerIter;
  _get9 = nsPerIterjjtype;
  const struct ReturnValue _7 = dong_porf_js_microbench__Porffor_concatStrings(_6.value, 195, _get8, _get9);
  jjlast_type = _7.type;
  _get10 = jjlast_type;
  const struct ReturnValue _8 = dong_porf_js_microbench__Porffor_concatStrings(_7.value, _get10, 1924, 195);
  jjlast_type = _8.type;
  _get11 = jjlast_type;
  linejjtype = _get11;
  line = _8.value;
  _get12 = line;
  _get13 = linejjtype;
  const struct ReturnValue _9 = dong_porf_js_microbench_benchLog(0, 0, 0, 0, _get12, _get13);
  jjlast_type = _9.type;
  (void) _9.value;
  _get14 = jjnewtarget;
  // if 
    if (((u32)(_get14)) != 0) {
      _get15 = jjthis;
      _get16 = jjthisjjtype;
      return (struct ReturnValue){ _get15, _get16 };
    }
  // end
  j691:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_js_microbench_noop(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype) {
  i32 _get2;
  f64 _get1;
  f64 _get0;
  _get0 = jjnewtarget;
  // if 
    if (((u32)(_get0)) != 0) {
      _get1 = jjthis;
      _get2 = jjthisjjtype;
      return (struct ReturnValue){ _get1, _get2 };
    }
  // end
  j694:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_js_microbench_skipWhitespace() {
  i32 _get24;
  i32 _get23;
  f64 _get22;
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
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  f64 c = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  i32 jjlast_type = 0;
  i32 logictmpi = 0;
  i32 jjlogicinner_tmp_int = 0;
  i32 jjtypeswitch_tmp1 = 0;

  // loop 
  j699:;
    // if 
      if (dong_porf_js_microbench_jjporfjjpos < dong_porf_js_microbench_jjporfjjlen) {
        jjproto_target = dong_porf_js_microbench_jjporfjjtext;
        jjproto_targetjjtype = 195;
        _get0 = jjproto_target;
        _get1 = jjproto_targetjjtype;
        const struct ReturnValue _0 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get0, _get1, dong_porf_js_microbench_jjporfjjpos, 1);
        jjlast_type = _0.type;
        c = _0.value;
        _get2 = c;
        // if 
          if (_get2 > 32) {
            goto j700;
          }
        // end
        j701:;
        _get3 = c;
        logictmpi = _get3 == 32;
        _get4 = logictmpi;
        // if i32
        i32 _r702;
          if ((_get4) == 0) {
            _get5 = c;
            jjlast_type = 2;
            _r702 = _get5 == 9;
          } else {
            _get6 = logictmpi;
            jjlast_type = 2;
            _r702 = _get6;
          }
        // end
        j702:;
        logictmpi = _r702;
        _get7 = logictmpi;
        jjlogicinner_tmp_int = _get7;
        _get8 = jjlast_type;
        jjtypeswitch_tmp1 = _get8;
        // block i32
        i32 _r703;
          _get9 = jjtypeswitch_tmp1;
          _get10 = jjtypeswitch_tmp1;
          // if 
            if (((_get9 == 67) | (_get10 == 195)) != 0) {
              _get11 = jjlogicinner_tmp_int;
              _r703 = (i32_load(1, 0, _get11)) == 0;
              goto j703;
            }
          // end
          j704:;
          _get12 = jjlogicinner_tmp_int;
          _r703 = (_get12) == 0;
        // end
        j703:;
        // if i32
        i32 _r705;
          if ((_r703) != 0) {
            _get13 = c;
            jjlast_type = 2;
            _r705 = _get13 == 10;
          } else {
            _get14 = logictmpi;
            _get15 = jjlast_type;
            jjlast_type = _get15;
            _r705 = _get14;
          }
        // end
        j705:;
        logictmpi = _r705;
        _get16 = logictmpi;
        jjlogicinner_tmp_int = _get16;
        _get17 = jjlast_type;
        jjtypeswitch_tmp1 = _get17;
        // block i32
        i32 _r706;
          _get18 = jjtypeswitch_tmp1;
          _get19 = jjtypeswitch_tmp1;
          // if 
            if (((_get18 == 67) | (_get19 == 195)) != 0) {
              _get20 = jjlogicinner_tmp_int;
              _r706 = (i32_load(1, 0, _get20)) == 0;
              goto j706;
            }
          // end
          j707:;
          _get21 = jjlogicinner_tmp_int;
          _r706 = (_get21) == 0;
        // end
        j706:;
        // if i32
        i32 _r708;
          if ((_r706) != 0) {
            _get22 = c;
            jjlast_type = 2;
            _r708 = _get22 == 13;
          } else {
            _get23 = logictmpi;
            _get24 = jjlast_type;
            jjlast_type = _get24;
            _r708 = _get23;
          }
        // end
        j708:;
        // if 
          if ((_r708) != 0) {
            dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
          } else {
            goto j700;
          }
        // end
        j709:;
        goto j699;
      }
    // end
    j700:;
  // end
  return (struct ReturnValue){ 0, 0 };
}

static f64 dong_porf_js_microbench_SyntaxError(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 message, i32 messagejjtype) {
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
      const struct ReturnValue _0 = dong_porf_js_microbench__ecma262_ToString(_get2, _get3);
      jjlast_type = _0.type;
      _get4 = jjlast_type;
      messagejjtype = _get4;
      message = _0.value;
    }
  // end
  j711:;
  obj = (f64)(dong_porf_js_microbench__Porffor_malloc(8));
  _get5 = obj;
  _get6 = message;
  i32_store(0, 0, (i32)(_get5), (i32)(_get6));
  _get7 = obj;
  _get8 = messagejjtype;
  i32_store8(0, 4, (i32)(_get7), _get8);
  _get9 = obj;
  return _get9;
}

static f64 dong_porf_js_microbench__Porffor_array_fastPush(f64 arr, i32 arrjjtype, f64 el, i32 eljjtype) {
  f64 _get12;
  f64 _get11;
  f64 _get10;
  f64 _get9;
  i32 _get8;
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 len = 0;
  i32 jjmember_setter_ptr_tmp = 0;
  f64 jjmember_obj_89 = 0;
  f64 jjmember_prop_89 = 0;

  _get0 = arr;
  len = (f64)(i32_load(1, 0, (u32)(_get0)));
  _get1 = arr;
  jjmember_obj_89 = _get1;
  _get2 = len;
  jjmember_prop_89 = _get2;
  _get3 = jjmember_obj_89;
  _get4 = jjmember_prop_89;
  jjmember_setter_ptr_tmp = (u32)(_get3) + ((u32)(_get4) * 9);
  _get5 = jjmember_setter_ptr_tmp;
  _get6 = el;
  f64_store(0, 4, _get5, _get6);
  _get7 = jjmember_setter_ptr_tmp;
  _get8 = eljjtype;
  i32_store8(0, 12, _get7, _get8);
  _get9 = arr;
  _get10 = len;
  len = _get10 + 1;
  _get11 = len;
  i32_store(1, 0, (u32)(_get9), (u32)(_get11));
  _get12 = len;
  return _get12;
}

static struct ReturnValue dong_porf_js_microbench__Porffor_object_accessorSet(i32 entryPtr, i32 entryPtrjjtype) {
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 out = 0;

  _get0 = entryPtr;
  out = i32_load(0, 12, _get0);
  _get1 = out;
  // if 
    if ((_get1) == 0) {
      return (struct ReturnValue){ 0, 0 };
    }
  // end
  j809:;
  _get2 = out;
  return (struct ReturnValue){ _get2, 6 };
}

static i32 dong_porf_js_microbench__Porffor_object_isInextensible(i32 obj, i32 objjjtype) {
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
      const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get1), _get2);
      jjlast_type = _0.type;
      _get3 = jjlast_type;
      objjjtype = _get3;
      obj = _0.value;
      _get4 = objjjtype;
      // if 
        if (_get4 != 7) {
          return 0;
        }
      // end
      j812:;
    }
  // end
  j811:;
  _get5 = obj;
  return i32_load8_u(0, 2, _get5) & 1;
}

static struct ReturnValue dong_porf_js_microbench__Porffor_object_set(i32 _obj, i32 _objjjtype, i32 key, i32 keyjjtype, f64 value, i32 valuejjtype) {
  i32 _get123;
  f64 _get122;
  i32 _get121;
  i32 _get120;
  i32 _get119;
  f64 _get118;
  i32 _get117;
  i32 _get116;
  i32 _get115;
  f64 _get114;
  i32 _get113;
  i32 _get112;
  f64 _get111;
  i32 _get110;
  i32 _get109;
  f64 _get108;
  i32 _get107;
  i32 _get106;
  i32 _get105;
  i32 _get104;
  i32 _get103;
  i32 _get102;
  i32 _get101;
  i32 _get100;
  f64 _get99;
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
  f64 _get84;
  i32 _get83;
  i32 _get82;
  i32 _get81;
  f64 _get80;
  i32 _get79;
  i32 _get78;
  f64 _get77;
  i32 _get76;
  i32 _get75;
  i32 _get74;
  i32 _get73;
  i32 _get72;
  i32 _get71;
  i32 _get70;
  i32 _get69;
  f64 _get68;
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
  f64 _get25;
  i32 _get24;
  f64 _get23;
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
  f64 _get7;
  i32 _get6;
  i32 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  i32 obj = 0;
  i32 objjjtype = 0;
  i32 jjlast_type = 0;
  i32 hash = 0;
  i32 entryPtr = 0;
  i32 flags = 0;
  i32 proto = 0;
  i32 protojjtype = 0;
  i32 jjlogicinner_tmp = 0;
  i32 jjtypeswitch_tmp1 = 0;
  i32 lastProto = 0;
  i32 lastProtojjtype = 0;
  i32 tail = 0;
  i32 set = 0;
  i32 jjcall_val = 0;
  i32 jjcall_type = 0;
  i32 jjindirect_4_callee = 0;
  i32 jjswap = 0;
  i32 size = 0;
  i32 jjindirect_5_callee = 0;

  _get0 = _obj;
  obj = _get0;
  _get1 = _objjjtype;
  objjjtype = _get1;
  _get2 = objjjtype;
  // if 
    if (_get2 != 7) {
      _get3 = obj;
      _get4 = objjjtype;
      const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get3), _get4);
      jjlast_type = _0.type;
      _get5 = jjlast_type;
      objjjtype = _get5;
      obj = _0.value;
      _get6 = objjjtype;
      // if 
        if (_get6 != 7) {
          _get7 = value;
          _get8 = valuejjtype;
          return (struct ReturnValue){ _get7, _get8 };
        }
      // end
      j789:;
    }
  // end
  j788:;
  _get9 = obj;
  // if 
    if ((_get9) == 0) {
    }
  // end
  j790:;
  _get10 = key;
  _get11 = keyjjtype;
  hash = dong_porf_js_microbench__Porffor_object_hash(_get10, _get11);
  _get12 = obj;
  _get13 = objjjtype;
  _get14 = key;
  _get15 = keyjjtype;
  _get16 = hash;
  entryPtr = dong_porf_js_microbench__Porffor_object_lookup(_get12, _get13, _get14, _get15, _get16, 1);
  _get17 = entryPtr;
  // if 
    if (_get17 == -1) {
      _get18 = hash;
      // if 
        if (_get18 == 593337848) {
          _get19 = key;
          _get20 = keyjjtype;
          // if 
            if ((dong_porf_js_microbench__Porffor_strcmp(_get19, _get20, 579, 195)) != 0) {
              _get21 = obj;
              _get22 = objjjtype;
              _get23 = value;
              _get24 = valuejjtype;
              dong_porf_js_microbench__Porffor_object_setPrototype(_get21, _get22, (i32)(_get23), _get24);
              _get25 = value;
              _get26 = valuejjtype;
              return (struct ReturnValue){ _get25, _get26 };
            }
          // end
          j793:;
        }
      // end
      j792:;
      _get27 = obj;
      _get28 = objjjtype;
      const struct ReturnValue _1 = dong_porf_js_microbench__Porffor_object_getPrototype(_get27, _get28);
      jjlast_type = _1.type;
      _get29 = jjlast_type;
      protojjtype = _get29;
      proto = _1.value;
      _get30 = proto;
      jjlogicinner_tmp = _get30;
      _get31 = protojjtype;
      jjtypeswitch_tmp1 = _get31;
      // block i32
      i32 _r794;
        _get32 = jjtypeswitch_tmp1;
        // if 
          if ((_get32) == 0) {
            _r794 = 1;
            goto j794;
          }
        // end
        j795:;
        _get33 = jjtypeswitch_tmp1;
        // if 
          if (_get33 == 7) {
            _get34 = jjlogicinner_tmp;
            _r794 = (_get34) == 0;
            goto j794;
          }
        // end
        j796:;
        _r794 = 0;
      // end
      j794:;
      // if 
        if ((_r794) == 0) {
          _get35 = protojjtype;
          // if 
            if (_get35 != 7) {
              _get36 = proto;
              _get37 = protojjtype;
              const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get36), _get37);
              jjlast_type = _2.type;
              _get38 = jjlast_type;
              protojjtype = _get38;
              proto = _2.value;
            }
          // end
          j798:;
          _get39 = proto;
          lastProto = _get39;
          _get40 = protojjtype;
          lastProtojjtype = _get40;
          // loop 
          j799:;
            // if 
              if ((1) != 0) {
                _get41 = proto;
                _get42 = protojjtype;
                _get43 = key;
                _get44 = keyjjtype;
                _get45 = hash;
                entryPtr = dong_porf_js_microbench__Porffor_object_lookup(_get41, _get42, _get43, _get44, _get45, 1);
                _get46 = entryPtr;
                // if 
                  if (_get46 != -1) {
                    goto j800;
                  }
                // end
                j801:;
                _get47 = proto;
                _get48 = protojjtype;
                const struct ReturnValue _3 = dong_porf_js_microbench__Porffor_object_getPrototype(_get47, _get48);
                jjlast_type = _3.type;
                _get49 = jjlast_type;
                protojjtype = _get49;
                proto = _3.value;
                _get50 = protojjtype;
                // if 
                  if (_get50 != 7) {
                    _get51 = proto;
                    _get52 = protojjtype;
                    const struct ReturnValue _4 = dong_porf_js_microbench__Porffor_object_underlying((f64)(_get51), _get52);
                    jjlast_type = _4.type;
                    _get53 = jjlast_type;
                    protojjtype = _get53;
                    proto = _4.value;
                  }
                // end
                j802:;
                _get54 = proto;
                jjlogicinner_tmp = _get54;
                _get55 = protojjtype;
                jjtypeswitch_tmp1 = _get55;
                // block i32
                i32 _r803;
                  _get56 = jjtypeswitch_tmp1;
                  // if 
                    if ((_get56) == 0) {
                      _r803 = 1;
                      goto j803;
                    }
                  // end
                  j804:;
                  _get57 = jjtypeswitch_tmp1;
                  // if 
                    if (_get57 == 7) {
                      _get58 = jjlogicinner_tmp;
                      _r803 = (_get58) == 0;
                      goto j803;
                    }
                  // end
                  j805:;
                  _r803 = 0;
                // end
                j803:;
                _get59 = proto;
                _get60 = lastProto;
                // if 
                  if ((_r803 | (_get59 == _get60)) != 0) {
                    goto j800;
                  }
                // end
                j806:;
                _get61 = proto;
                lastProto = _get61;
                _get62 = protojjtype;
                lastProtojjtype = _get62;
                goto j799;
              }
            // end
            j800:;
          // end
          _get63 = entryPtr;
          // if 
            if (_get63 != -1) {
              _get64 = entryPtr;
              tail = i32_load16_u(0, 16, _get64);
              _get65 = tail;
              // if 
                if ((_get65 & 1) != 0) {
                  _get66 = entryPtr;
                  const struct ReturnValue _5 = dong_porf_js_microbench__Porffor_object_accessorSet(_get66, 1);
                  jjlast_type = _5.type;
                  set = _5.value;
                  _get67 = set;
                  // if 
                    if ((_get67) == 0) {
                      _get68 = value;
                      _get69 = valuejjtype;
                      return (struct ReturnValue){ _get68, _get69 };
                    }
                  // end
                  j810:;
                  _get70 = set;
                  jjindirect_4_callee = _get70;
                  jjswap = 0;
                  _get71 = jjswap;
                  _get72 = _obj;
                  jjcall_val = _get72;
                  _get73 = jjcall_val;
                  _get74 = _objjjtype;
                  jjcall_type = _get74;
                  _get75 = jjcall_type;
                  jjswap = _get75;
                  _get76 = jjswap;
                  _get77 = value;
                  _get78 = valuejjtype;
                  _get79 = jjindirect_4_callee;
                  jjlast_type = 0;
                  (void) (i32)(0);
                  _get80 = value;
                  _get81 = valuejjtype;
                  return (struct ReturnValue){ _get80, _get81 };
                  (void) 0;
                }
              // end
              j808:;
            }
          // end
          j807:;
        }
      // end
      j797:;
      _get82 = obj;
      _get83 = objjjtype;
      // if 
        if ((dong_porf_js_microbench__Porffor_object_isInextensible(_get82, _get83)) != 0) {
          _get84 = value;
          _get85 = valuejjtype;
          return (struct ReturnValue){ _get84, _get85 };
          (void) 0;
        }
      // end
      j813:;
      _get86 = obj;
      size = i32_load16_u(0, 0, _get86);
      _get87 = obj;
      _get88 = size;
      i32_store16(0, 0, _get87, _get88 + 1);
      _get89 = obj;
      _get90 = size;
      entryPtr = (_get89 + 8) + (_get90 * 18);
      _get91 = entryPtr;
      _get92 = key;
      _get93 = keyjjtype;
      _get94 = hash;
      dong_porf_js_microbench__Porffor_object_writeKey(_get91, 1, _get92, _get93, _get94, 1);
      flags = 14;
    } else {
      _get95 = entryPtr;
      tail = i32_load16_u(0, 16, _get95);
      _get96 = tail;
      // if 
        if ((_get96 & 1) != 0) {
          _get97 = entryPtr;
          const struct ReturnValue _6 = dong_porf_js_microbench__Porffor_object_accessorSet(_get97, 1);
          jjlast_type = _6.type;
          set = _6.value;
          _get98 = set;
          // if 
            if ((_get98) == 0) {
              _get99 = value;
              _get100 = valuejjtype;
              return (struct ReturnValue){ _get99, _get100 };
              (void) 0;
            }
          // end
          j815:;
          _get101 = set;
          jjindirect_5_callee = _get101;
          jjswap = 0;
          _get102 = jjswap;
          _get103 = _obj;
          jjcall_val = _get103;
          _get104 = jjcall_val;
          _get105 = _objjjtype;
          jjcall_type = _get105;
          _get106 = jjcall_type;
          jjswap = _get106;
          _get107 = jjswap;
          _get108 = value;
          _get109 = valuejjtype;
          _get110 = jjindirect_5_callee;
          jjlast_type = 0;
          (void) (i32)(0);
          _get111 = value;
          _get112 = valuejjtype;
          return (struct ReturnValue){ _get111, _get112 };
          (void) 0;
        }
      // end
      j814:;
      _get113 = tail;
      // if 
        if ((_get113 & 8) == 0) {
          _get114 = value;
          _get115 = valuejjtype;
          return (struct ReturnValue){ _get114, _get115 };
          (void) 0;
        }
      // end
      j816:;
      _get116 = tail;
      flags = _get116 & 255;
    }
  // end
  j791:;
  _get117 = entryPtr;
  _get118 = value;
  f64_store(0, 8, _get117, _get118);
  _get119 = entryPtr;
  _get120 = flags;
  _get121 = valuejjtype;
  i32_store16(0, 16, _get119, _get120 + (_get121 << 8));
  _get122 = value;
  _get123 = valuejjtype;
  return (struct ReturnValue){ _get122, _get123 };
}

static struct ReturnValue dong_porf_js_microbench__ByteString_prototype_slice(i32 _this, i32 _thisjjtype, i32 start, i32 startjjtype, i32 end, i32 endjjtype) {
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

  _get0 = _this;
  len = i32_load(1, 0, _get0);
  _get1 = endjjtype;
  // if 
    if ((_get1) == 0) {
      _get2 = len;
      end = _get2;
    }
  // end
  j840:;
  _get3 = start;
  // if 
    if (_get3 < 0) {
      _get4 = len;
      _get5 = start;
      start = _get4 + _get5;
      _get6 = start;
      // if 
        if (_get6 < 0) {
          start = 0;
        }
      // end
      j842:;
    }
  // end
  j841:;
  _get7 = start;
  _get8 = len;
  // if 
    if (_get7 > _get8) {
      _get9 = len;
      start = _get9;
    }
  // end
  j843:;
  _get10 = end;
  // if 
    if (_get10 < 0) {
      _get11 = len;
      _get12 = end;
      end = _get11 + _get12;
      _get13 = end;
      // if 
        if (_get13 < 0) {
          end = 0;
        }
      // end
      j845:;
    }
  // end
  j844:;
  _get14 = end;
  _get15 = len;
  // if 
    if (_get14 > _get15) {
      _get16 = len;
      end = _get16;
    }
  // end
  j846:;
  _get17 = end;
  _get18 = start;
  out = dong_porf_js_microbench__Porffor_malloc(6 + (_get17 - _get18));
  _get19 = start;
  _get20 = end;
  // if 
    if (_get19 > _get20) {
      _get21 = out;
      return (struct ReturnValue){ _get21, 195 };
    }
  // end
  j847:;
  _get22 = out;
  outPtr = _get22;
  _get23 = _this;
  thisPtr = _get23;
  _get24 = thisPtr;
  _get25 = end;
  thisPtrEnd = _get24 + _get25;
  _get26 = thisPtr;
  _get27 = start;
  thisPtr = _get26 + _get27;
  // loop 
  j848:;
    _get28 = thisPtr;
    _get29 = thisPtrEnd;
    // if 
      if (_get28 < _get29) {
        _get30 = outPtr;
        _get31 = outPtr;
        outPtr = _get31 + 1;
        _get32 = thisPtr;
        _get33 = thisPtr;
        thisPtr = _get33 + 1;
        i32_store8(0, 4, _get30, i32_load8_u(0, 4, _get32));
        goto j848;
      }
    // end
    j849:;
  // end
  _get34 = out;
  _get35 = end;
  _get36 = start;
  i32_store(1, 0, _get34, _get35 - _get36);
  _get37 = out;
  return (struct ReturnValue){ _get37, 195 };
}

static struct ReturnValue dong_porf_js_microbench_parseValue() {
  i32 _get245;
  f64 _get244;
  i32 _get243;
  i32 _get242;
  f64 _get241;
  i32 _get240;
  i32 _get239;
  i32 _get238;
  i32 _get237;
  i32 _get236;
  i32 _get235;
  i32 _get234;
  f64 _get233;
  i32 _get232;
  f64 _get231;
  i32 _get230;
  i32 _get229;
  f64 _get228;
  i32 _get227;
  i32 _get226;
  i32 _get225;
  i32 _get224;
  i32 _get223;
  i32 _get222;
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
  f64 _get211;
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
  f64 _get200;
  i32 _get199;
  f64 _get198;
  i32 _get197;
  f64 _get196;
  f64 _get195;
  f64 _get194;
  i32 _get193;
  f64 _get192;
  i32 _get191;
  f64 _get190;
  i32 _get189;
  i32 _get188;
  f64 _get187;
  i32 _get186;
  f64 _get185;
  i32 _get184;
  i32 _get183;
  f64 _get182;
  i32 _get181;
  f64 _get180;
  f64 _get179;
  i32 _get178;
  i32 _get177;
  i32 _get176;
  f64 _get175;
  f64 _get174;
  i32 _get173;
  i32 _get172;
  i32 _get171;
  f64 _get170;
  i32 _get169;
  i32 _get168;
  i32 _get167;
  i32 _get166;
  f64 _get165;
  i32 _get164;
  i32 _get163;
  f64 _get162;
  i32 _get161;
  i32 _get160;
  f64 _get159;
  i32 _get158;
  f64 _get157;
  f64 _get156;
  f64 _get155;
  f64 _get154;
  i32 _get153;
  f64 _get152;
  i32 _get151;
  f64 _get150;
  f64 _get149;
  i32 _get148;
  i32 _get147;
  f64 _get146;
  i32 _get145;
  f64 _get144;
  f64 _get143;
  f64 _get142;
  i32 _get141;
  f64 _get140;
  i32 _get139;
  f64 _get138;
  f64 _get137;
  f64 _get136;
  f64 _get135;
  f64 _get134;
  f64 _get133;
  i32 _get132;
  f64 _get131;
  i32 _get130;
  f64 _get129;
  f64 _get128;
  f64 _get127;
  i32 _get126;
  f64 _get125;
  i32 _get124;
  f64 _get123;
  f64 _get122;
  f64 _get121;
  i32 _get120;
  f64 _get119;
  i32 _get118;
  f64 _get117;
  f64 _get116;
  f64 _get115;
  i32 _get114;
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
  i32 _get94;
  f64 _get93;
  f64 _get92;
  f64 _get91;
  f64 _get90;
  i32 _get89;
  f64 _get88;
  f64 _get87;
  i32 _get86;
  i32 _get85;
  i32 _get84;
  f64 _get83;
  i32 _get82;
  i32 _get81;
  i32 _get80;
  i32 _get79;
  i32 _get78;
  i32 _get77;
  i32 _get76;
  i32 _get75;
  i32 _get74;
  f64 _get73;
  i32 _get72;
  i32 _get71;
  i32 _get70;
  i32 _get69;
  i32 _get68;
  i32 _get67;
  i32 _get66;
  i32 _get65;
  i32 _get64;
  f64 _get63;
  i32 _get62;
  i32 _get61;
  i32 _get60;
  i32 _get59;
  i32 _get58;
  i32 _get57;
  i32 _get56;
  i32 _get55;
  f64 _get54;
  i32 _get53;
  f64 _get52;
  i32 _get51;
  i32 _get50;
  i32 _get49;
  f64 _get48;
  i32 _get47;
  i32 _get46;
  i32 _get45;
  i32 _get44;
  i32 _get43;
  i32 _get42;
  i32 _get41;
  i32 _get40;
  i32 _get39;
  f64 _get38;
  i32 _get37;
  i32 _get36;
  i32 _get35;
  i32 _get34;
  i32 _get33;
  i32 _get32;
  i32 _get31;
  i32 _get30;
  f64 _get29;
  i32 _get28;
  f64 _get27;
  i32 _get26;
  i32 _get25;
  i32 _get24;
  f64 _get23;
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
  i32 _get5;
  f64 _get4;
  i32 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  i32 jjlast_type = 0;
  f64 c = 0;
  f64 jjproto_target = 0;
  i32 jjproto_targetjjtype = 0;
  i32 logictmpi = 0;
  i32 jjlogicinner_tmp_int = 0;
  i32 jjtypeswitch_tmp1 = 0;
  f64 out = 0;
  f64 ch = 0;
  f64 esc = 0;
  f64 unicode = 0;
  f64 i = 0;
  f64 hex = 0;
  f64 arr = 0;
  f64 next = 0;
  f64 obj = 0;
  i32 objjjtype = 0;
  f64 key = 0;
  i32 keyjjtype = 0;
  f64 value = 0;
  i32 valuejjtype = 0;
  i32 jjmember_setter_ptr_tmp = 0;
  f64 jjmember_obj_183 = 0;
  f64 jjmember_prop_183 = 0;
  i32 jjswap = 0;
  f64 start = 0;

  const struct ReturnValue _0 = dong_porf_js_microbench_skipWhitespace();
  jjlast_type = _0.type;
  (void) _0.value;
  // if 
    if (dong_porf_js_microbench_jjporfjjpos >= dong_porf_js_microbench_jjporfjjlen) {
    }
  // end
  j710:;
  jjproto_target = dong_porf_js_microbench_jjporfjjtext;
  jjproto_targetjjtype = 195;
  _get0 = jjproto_target;
  _get1 = jjproto_targetjjtype;
  const struct ReturnValue _1 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get0, _get1, dong_porf_js_microbench_jjporfjjpos, 1);
  jjlast_type = _1.type;
  c = _1.value;
  _get2 = c;
  // if 
    if (_get2 == 110) {
      logictmpi = (dong_porf_js_microbench_jjporfjjpos + 4) <= dong_porf_js_microbench_jjporfjjlen;
      _get3 = logictmpi;
      // if i32
      i32 _r713;
        if ((_get3) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get4 = jjproto_target;
          _get5 = jjproto_targetjjtype;
          const struct ReturnValue _2 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get4, _get5, dong_porf_js_microbench_jjporfjjpos + 1, 1);
          jjlast_type = _2.type;
          jjlast_type = 2;
          _r713 = _2.value == 117;
        } else {
          _get6 = logictmpi;
          jjlast_type = 2;
          _r713 = _get6;
        }
      // end
      j713:;
      logictmpi = _r713;
      _get7 = logictmpi;
      jjlogicinner_tmp_int = _get7;
      _get8 = jjlast_type;
      jjtypeswitch_tmp1 = _get8;
      // block i32
      i32 _r714;
        _get9 = jjtypeswitch_tmp1;
        _get10 = jjtypeswitch_tmp1;
        // if 
          if (((_get9 == 67) | (_get10 == 195)) != 0) {
            _get11 = jjlogicinner_tmp_int;
            _r714 = i32_load(1, 0, _get11);
            goto j714;
          }
        // end
        j715:;
        _get12 = jjlogicinner_tmp_int;
        _r714 = _get12;
      // end
      j714:;
      // if i32
      i32 _r716;
        if ((_r714) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get13 = jjproto_target;
          _get14 = jjproto_targetjjtype;
          const struct ReturnValue _3 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get13, _get14, dong_porf_js_microbench_jjporfjjpos + 2, 1);
          jjlast_type = _3.type;
          jjlast_type = 2;
          _r716 = _3.value == 108;
        } else {
          _get15 = logictmpi;
          _get16 = jjlast_type;
          jjlast_type = _get16;
          _r716 = _get15;
        }
      // end
      j716:;
      logictmpi = _r716;
      _get17 = logictmpi;
      jjlogicinner_tmp_int = _get17;
      _get18 = jjlast_type;
      jjtypeswitch_tmp1 = _get18;
      // block i32
      i32 _r717;
        _get19 = jjtypeswitch_tmp1;
        _get20 = jjtypeswitch_tmp1;
        // if 
          if (((_get19 == 67) | (_get20 == 195)) != 0) {
            _get21 = jjlogicinner_tmp_int;
            _r717 = i32_load(1, 0, _get21);
            goto j717;
          }
        // end
        j718:;
        _get22 = jjlogicinner_tmp_int;
        _r717 = _get22;
      // end
      j717:;
      // if i32
      i32 _r719;
        if ((_r717) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get23 = jjproto_target;
          _get24 = jjproto_targetjjtype;
          const struct ReturnValue _4 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get23, _get24, dong_porf_js_microbench_jjporfjjpos + 3, 1);
          jjlast_type = _4.type;
          jjlast_type = 2;
          _r719 = _4.value == 108;
        } else {
          _get25 = logictmpi;
          _get26 = jjlast_type;
          jjlast_type = _get26;
          _r719 = _get25;
        }
      // end
      j719:;
      // if 
        if ((_r719) != 0) {
          dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 4;
          return (struct ReturnValue){ 0, 7 };
        }
      // end
      j720:;
    }
  // end
  j712:;
  _get27 = c;
  // if 
    if (_get27 == 116) {
      logictmpi = (dong_porf_js_microbench_jjporfjjpos + 4) <= dong_porf_js_microbench_jjporfjjlen;
      _get28 = logictmpi;
      // if i32
      i32 _r722;
        if ((_get28) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get29 = jjproto_target;
          _get30 = jjproto_targetjjtype;
          const struct ReturnValue _5 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get29, _get30, dong_porf_js_microbench_jjporfjjpos + 1, 1);
          jjlast_type = _5.type;
          jjlast_type = 2;
          _r722 = _5.value == 114;
        } else {
          _get31 = logictmpi;
          jjlast_type = 2;
          _r722 = _get31;
        }
      // end
      j722:;
      logictmpi = _r722;
      _get32 = logictmpi;
      jjlogicinner_tmp_int = _get32;
      _get33 = jjlast_type;
      jjtypeswitch_tmp1 = _get33;
      // block i32
      i32 _r723;
        _get34 = jjtypeswitch_tmp1;
        _get35 = jjtypeswitch_tmp1;
        // if 
          if (((_get34 == 67) | (_get35 == 195)) != 0) {
            _get36 = jjlogicinner_tmp_int;
            _r723 = i32_load(1, 0, _get36);
            goto j723;
          }
        // end
        j724:;
        _get37 = jjlogicinner_tmp_int;
        _r723 = _get37;
      // end
      j723:;
      // if i32
      i32 _r725;
        if ((_r723) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get38 = jjproto_target;
          _get39 = jjproto_targetjjtype;
          const struct ReturnValue _6 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get38, _get39, dong_porf_js_microbench_jjporfjjpos + 2, 1);
          jjlast_type = _6.type;
          jjlast_type = 2;
          _r725 = _6.value == 117;
        } else {
          _get40 = logictmpi;
          _get41 = jjlast_type;
          jjlast_type = _get41;
          _r725 = _get40;
        }
      // end
      j725:;
      logictmpi = _r725;
      _get42 = logictmpi;
      jjlogicinner_tmp_int = _get42;
      _get43 = jjlast_type;
      jjtypeswitch_tmp1 = _get43;
      // block i32
      i32 _r726;
        _get44 = jjtypeswitch_tmp1;
        _get45 = jjtypeswitch_tmp1;
        // if 
          if (((_get44 == 67) | (_get45 == 195)) != 0) {
            _get46 = jjlogicinner_tmp_int;
            _r726 = i32_load(1, 0, _get46);
            goto j726;
          }
        // end
        j727:;
        _get47 = jjlogicinner_tmp_int;
        _r726 = _get47;
      // end
      j726:;
      // if i32
      i32 _r728;
        if ((_r726) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get48 = jjproto_target;
          _get49 = jjproto_targetjjtype;
          const struct ReturnValue _7 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get48, _get49, dong_porf_js_microbench_jjporfjjpos + 3, 1);
          jjlast_type = _7.type;
          jjlast_type = 2;
          _r728 = _7.value == 101;
        } else {
          _get50 = logictmpi;
          _get51 = jjlast_type;
          jjlast_type = _get51;
          _r728 = _get50;
        }
      // end
      j728:;
      // if 
        if ((_r728) != 0) {
          dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 4;
          return (struct ReturnValue){ 1, 2 };
        }
      // end
      j729:;
    }
  // end
  j721:;
  _get52 = c;
  // if 
    if (_get52 == 102) {
      logictmpi = (dong_porf_js_microbench_jjporfjjpos + 5) <= dong_porf_js_microbench_jjporfjjlen;
      _get53 = logictmpi;
      // if i32
      i32 _r731;
        if ((_get53) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get54 = jjproto_target;
          _get55 = jjproto_targetjjtype;
          const struct ReturnValue _8 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get54, _get55, dong_porf_js_microbench_jjporfjjpos + 1, 1);
          jjlast_type = _8.type;
          jjlast_type = 2;
          _r731 = _8.value == 97;
        } else {
          _get56 = logictmpi;
          jjlast_type = 2;
          _r731 = _get56;
        }
      // end
      j731:;
      logictmpi = _r731;
      _get57 = logictmpi;
      jjlogicinner_tmp_int = _get57;
      _get58 = jjlast_type;
      jjtypeswitch_tmp1 = _get58;
      // block i32
      i32 _r732;
        _get59 = jjtypeswitch_tmp1;
        _get60 = jjtypeswitch_tmp1;
        // if 
          if (((_get59 == 67) | (_get60 == 195)) != 0) {
            _get61 = jjlogicinner_tmp_int;
            _r732 = i32_load(1, 0, _get61);
            goto j732;
          }
        // end
        j733:;
        _get62 = jjlogicinner_tmp_int;
        _r732 = _get62;
      // end
      j732:;
      // if i32
      i32 _r734;
        if ((_r732) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get63 = jjproto_target;
          _get64 = jjproto_targetjjtype;
          const struct ReturnValue _9 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get63, _get64, dong_porf_js_microbench_jjporfjjpos + 2, 1);
          jjlast_type = _9.type;
          jjlast_type = 2;
          _r734 = _9.value == 108;
        } else {
          _get65 = logictmpi;
          _get66 = jjlast_type;
          jjlast_type = _get66;
          _r734 = _get65;
        }
      // end
      j734:;
      logictmpi = _r734;
      _get67 = logictmpi;
      jjlogicinner_tmp_int = _get67;
      _get68 = jjlast_type;
      jjtypeswitch_tmp1 = _get68;
      // block i32
      i32 _r735;
        _get69 = jjtypeswitch_tmp1;
        _get70 = jjtypeswitch_tmp1;
        // if 
          if (((_get69 == 67) | (_get70 == 195)) != 0) {
            _get71 = jjlogicinner_tmp_int;
            _r735 = i32_load(1, 0, _get71);
            goto j735;
          }
        // end
        j736:;
        _get72 = jjlogicinner_tmp_int;
        _r735 = _get72;
      // end
      j735:;
      // if i32
      i32 _r737;
        if ((_r735) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get73 = jjproto_target;
          _get74 = jjproto_targetjjtype;
          const struct ReturnValue _10 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get73, _get74, dong_porf_js_microbench_jjporfjjpos + 3, 1);
          jjlast_type = _10.type;
          jjlast_type = 2;
          _r737 = _10.value == 115;
        } else {
          _get75 = logictmpi;
          _get76 = jjlast_type;
          jjlast_type = _get76;
          _r737 = _get75;
        }
      // end
      j737:;
      logictmpi = _r737;
      _get77 = logictmpi;
      jjlogicinner_tmp_int = _get77;
      _get78 = jjlast_type;
      jjtypeswitch_tmp1 = _get78;
      // block i32
      i32 _r738;
        _get79 = jjtypeswitch_tmp1;
        _get80 = jjtypeswitch_tmp1;
        // if 
          if (((_get79 == 67) | (_get80 == 195)) != 0) {
            _get81 = jjlogicinner_tmp_int;
            _r738 = i32_load(1, 0, _get81);
            goto j738;
          }
        // end
        j739:;
        _get82 = jjlogicinner_tmp_int;
        _r738 = _get82;
      // end
      j738:;
      // if i32
      i32 _r740;
        if ((_r738) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get83 = jjproto_target;
          _get84 = jjproto_targetjjtype;
          const struct ReturnValue _11 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get83, _get84, dong_porf_js_microbench_jjporfjjpos + 4, 1);
          jjlast_type = _11.type;
          jjlast_type = 2;
          _r740 = _11.value == 101;
        } else {
          _get85 = logictmpi;
          _get86 = jjlast_type;
          jjlast_type = _get86;
          _r740 = _get85;
        }
      // end
      j740:;
      // if 
        if ((_r740) != 0) {
          dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 5;
          return (struct ReturnValue){ 0, 2 };
        }
      // end
      j741:;
    }
  // end
  j730:;
  _get87 = c;
  // if 
    if (_get87 == 34) {
      dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
      out = (f64)(dong_porf_js_microbench__Porffor_malloc(16384));
      // loop 
      j743:;
        // if 
          if (dong_porf_js_microbench_jjporfjjpos < dong_porf_js_microbench_jjporfjjlen) {
            jjproto_target = dong_porf_js_microbench_jjporfjjtext;
            jjproto_targetjjtype = 195;
            _get88 = jjproto_target;
            _get89 = jjproto_targetjjtype;
            const struct ReturnValue _12 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get88, _get89, dong_porf_js_microbench_jjporfjjpos, 1);
            jjlast_type = _12.type;
            ch = _12.value;
            _get90 = ch;
            // if 
              if (_get90 == 34) {
                dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
                _get91 = out;
                return (struct ReturnValue){ _get91, 195 };
              }
            // end
            j745:;
            _get92 = ch;
            // if 
              if (_get92 == 92) {
                dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
                // if 
                  if (dong_porf_js_microbench_jjporfjjpos >= dong_porf_js_microbench_jjporfjjlen) {
                  }
                // end
                j747:;
                jjproto_target = dong_porf_js_microbench_jjporfjjtext;
                jjproto_targetjjtype = 195;
                _get93 = jjproto_target;
                _get94 = jjproto_targetjjtype;
                dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
                const struct ReturnValue _13 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get93, _get94, dong_porf_js_microbench_jjporfjjpos, 1);
                jjlast_type = _13.type;
                esc = _13.value;
                _get95 = esc;
                // if 
                  if (_get95 == 34) {
                    _get96 = out;
                    (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get96, 195, 34, 1);
                  } else {
                    _get97 = esc;
                    // if 
                      if (_get97 == 92) {
                        _get98 = out;
                        (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get98, 195, 92, 1);
                      } else {
                        _get99 = esc;
                        // if 
                          if (_get99 == 47) {
                            _get100 = out;
                            (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get100, 195, 47, 1);
                          } else {
                            _get101 = esc;
                            // if 
                              if (_get101 == 98) {
                                _get102 = out;
                                (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get102, 195, 8, 1);
                              } else {
                                _get103 = esc;
                                // if 
                                  if (_get103 == 102) {
                                    _get104 = out;
                                    (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get104, 195, 12, 1);
                                  } else {
                                    _get105 = esc;
                                    // if 
                                      if (_get105 == 110) {
                                        _get106 = out;
                                        (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get106, 195, 10, 1);
                                      } else {
                                        _get107 = esc;
                                        // if 
                                          if (_get107 == 114) {
                                            _get108 = out;
                                            (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get108, 195, 13, 1);
                                          } else {
                                            _get109 = esc;
                                            // if 
                                              if (_get109 == 116) {
                                                _get110 = out;
                                                (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get110, 195, 9, 1);
                                              } else {
                                                _get111 = esc;
                                                // if 
                                                  if (_get111 == 117) {
                                                    // if 
                                                      if ((dong_porf_js_microbench_jjporfjjpos + 4) >= dong_porf_js_microbench_jjporfjjlen) {
                                                      }
                                                    // end
                                                    j757:;
                                                    unicode = 0;
                                                    i = 0;
                                                    // loop 
                                                    j758:;
                                                      _get112 = i;
                                                      // if 
                                                        if (_get112 < 4) {
                                                          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
                                                          jjproto_targetjjtype = 195;
                                                          _get113 = jjproto_target;
                                                          _get114 = jjproto_targetjjtype;
                                                          _get115 = i;
                                                          const struct ReturnValue _14 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get113, _get114, dong_porf_js_microbench_jjporfjjpos + _get115, 1);
                                                          jjlast_type = _14.type;
                                                          hex = _14.value;
                                                          _get116 = unicode;
                                                          unicode = (f64)((i32)(_get116) << 4);
                                                          _get117 = hex;
                                                          logictmpi = _get117 >= 48;
                                                          _get118 = logictmpi;
                                                          // if i32
                                                          i32 _r760;
                                                            if ((_get118) != 0) {
                                                              _get119 = hex;
                                                              jjlast_type = 2;
                                                              _r760 = _get119 <= 57;
                                                            } else {
                                                              _get120 = logictmpi;
                                                              jjlast_type = 2;
                                                              _r760 = _get120;
                                                            }
                                                          // end
                                                          j760:;
                                                          // if 
                                                            if ((_r760) != 0) {
                                                              _get121 = unicode;
                                                              _get122 = hex;
                                                              unicode = (f64)((i32)(_get121) | (i32)((_get122 - 48)));
                                                            } else {
                                                              _get123 = hex;
                                                              logictmpi = _get123 >= 65;
                                                              _get124 = logictmpi;
                                                              // if i32
                                                              i32 _r762;
                                                                if ((_get124) != 0) {
                                                                  _get125 = hex;
                                                                  jjlast_type = 2;
                                                                  _r762 = _get125 <= 70;
                                                                } else {
                                                                  _get126 = logictmpi;
                                                                  jjlast_type = 2;
                                                                  _r762 = _get126;
                                                                }
                                                              // end
                                                              j762:;
                                                              // if 
                                                                if ((_r762) != 0) {
                                                                  _get127 = unicode;
                                                                  _get128 = hex;
                                                                  unicode = (f64)((i32)(_get127) | (i32)((_get128 - 55)));
                                                                } else {
                                                                  _get129 = hex;
                                                                  logictmpi = _get129 >= 97;
                                                                  _get130 = logictmpi;
                                                                  // if i32
                                                                  i32 _r764;
                                                                    if ((_get130) != 0) {
                                                                      _get131 = hex;
                                                                      jjlast_type = 2;
                                                                      _r764 = _get131 <= 102;
                                                                    } else {
                                                                      _get132 = logictmpi;
                                                                      jjlast_type = 2;
                                                                      _r764 = _get132;
                                                                    }
                                                                  // end
                                                                  j764:;
                                                                  // if 
                                                                    if ((_r764) != 0) {
                                                                      _get133 = unicode;
                                                                      _get134 = hex;
                                                                      unicode = (f64)((i32)(_get133) | (i32)((_get134 - 87)));
                                                                    } else {
                                                                    }
                                                                  // end
                                                                  j765:;
                                                                }
                                                              // end
                                                              j763:;
                                                            }
                                                          // end
                                                          j761:;
                                                          _get135 = i;
                                                          i = _get135 + 1;
                                                          goto j758;
                                                        }
                                                      // end
                                                      j759:;
                                                    // end
                                                    dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 4;
                                                    _get136 = out;
                                                    _get137 = unicode;
                                                    (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get136, 195, _get137, 1);
                                                  } else {
                                                  }
                                                // end
                                                j756:;
                                              }
                                            // end
                                            j755:;
                                          }
                                        // end
                                        j754:;
                                      }
                                    // end
                                    j753:;
                                  }
                                // end
                                j752:;
                              }
                            // end
                            j751:;
                          }
                        // end
                        j750:;
                      }
                    // end
                    j749:;
                  }
                // end
                j748:;
              } else {
                _get138 = ch;
                logictmpi = _get138 >= 0;
                _get139 = logictmpi;
                // if i32
                i32 _r766;
                  if ((_get139) != 0) {
                    _get140 = ch;
                    jjlast_type = 2;
                    _r766 = _get140 <= 31;
                  } else {
                    _get141 = logictmpi;
                    jjlast_type = 2;
                    _r766 = _get141;
                  }
                // end
                j766:;
                // if 
                  if ((_r766) != 0) {
                  }
                // end
                j767:;
                _get142 = out;
                _get143 = ch;
                (void) dong_porf_js_microbench__Porffor_bytestring_appendChar(_get142, 195, _get143, 1);
                dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
              }
            // end
            j746:;
            goto j743;
          }
        // end
        j744:;
      // end
    }
  // end
  j742:;
  _get144 = c;
  // if 
    if (_get144 == 91) {
      dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
      arr = (f64)(dong_porf_js_microbench__Porffor_malloc(16384));
      const struct ReturnValue _15 = dong_porf_js_microbench_skipWhitespace();
      jjlast_type = _15.type;
      (void) _15.value;
      logictmpi = dong_porf_js_microbench_jjporfjjpos < dong_porf_js_microbench_jjporfjjlen;
      _get145 = logictmpi;
      // if i32
      i32 _r769;
        if ((_get145) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get146 = jjproto_target;
          _get147 = jjproto_targetjjtype;
          const struct ReturnValue _16 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get146, _get147, dong_porf_js_microbench_jjporfjjpos, 1);
          jjlast_type = _16.type;
          jjlast_type = 2;
          _r769 = _16.value == 93;
        } else {
          _get148 = logictmpi;
          jjlast_type = 2;
          _r769 = _get148;
        }
      // end
      j769:;
      // if 
        if ((_r769) != 0) {
          dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
          _get149 = arr;
          return (struct ReturnValue){ _get149, 72 };
        }
      // end
      j770:;
      // loop 
      j771:;
        // if 
          if ((1) != 0) {
            _get150 = arr;
            const struct ReturnValue _17 = dong_porf_js_microbench_parseValue();
            jjlast_type = _17.type;
            _get151 = jjlast_type;
            (void) dong_porf_js_microbench__Porffor_array_fastPush(_get150, 72, _17.value, _get151);
            const struct ReturnValue _18 = dong_porf_js_microbench_skipWhitespace();
            jjlast_type = _18.type;
            (void) _18.value;
            // if 
              if (dong_porf_js_microbench_jjporfjjpos >= dong_porf_js_microbench_jjporfjjlen) {
              }
            // end
            j773:;
            jjproto_target = dong_porf_js_microbench_jjporfjjtext;
            jjproto_targetjjtype = 195;
            _get152 = jjproto_target;
            _get153 = jjproto_targetjjtype;
            const struct ReturnValue _19 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get152, _get153, dong_porf_js_microbench_jjporfjjpos, 1);
            jjlast_type = _19.type;
            next = _19.value;
            _get154 = next;
            // if 
              if (_get154 == 93) {
                dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
                goto j772;
              }
            // end
            j774:;
            _get155 = next;
            // if 
              if (_get155 == 44) {
                dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
                goto j771;
              }
            // end
            j775:;
            goto j771;
          }
        // end
        j772:;
      // end
      _get156 = arr;
      return (struct ReturnValue){ _get156, 72 };
    }
  // end
  j768:;
  _get157 = c;
  // if 
    if (_get157 == 123) {
      dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
      obj = (f64)(dong_porf_js_microbench__Porffor_malloc(16384));
      objjjtype = 7;
      const struct ReturnValue _20 = dong_porf_js_microbench_skipWhitespace();
      jjlast_type = _20.type;
      (void) _20.value;
      logictmpi = dong_porf_js_microbench_jjporfjjpos < dong_porf_js_microbench_jjporfjjlen;
      _get158 = logictmpi;
      // if i32
      i32 _r777;
        if ((_get158) != 0) {
          jjproto_target = dong_porf_js_microbench_jjporfjjtext;
          jjproto_targetjjtype = 195;
          _get159 = jjproto_target;
          _get160 = jjproto_targetjjtype;
          const struct ReturnValue _21 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get159, _get160, dong_porf_js_microbench_jjporfjjpos, 1);
          jjlast_type = _21.type;
          jjlast_type = 2;
          _r777 = _21.value == 125;
        } else {
          _get161 = logictmpi;
          jjlast_type = 2;
          _r777 = _get161;
        }
      // end
      j777:;
      // if 
        if ((_r777) != 0) {
          dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
          _get162 = obj;
          _get163 = objjjtype;
          return (struct ReturnValue){ _get162, _get163 };
        }
      // end
      j778:;
      // loop 
      j779:;
        // if 
          if ((1) != 0) {
            const struct ReturnValue _22 = dong_porf_js_microbench_skipWhitespace();
            jjlast_type = _22.type;
            (void) _22.value;
            logictmpi = dong_porf_js_microbench_jjporfjjpos >= dong_porf_js_microbench_jjporfjjlen;
            _get164 = logictmpi;
            // if i32
            i32 _r781;
              if ((_get164) == 0) {
                jjproto_target = dong_porf_js_microbench_jjporfjjtext;
                jjproto_targetjjtype = 195;
                _get165 = jjproto_target;
                _get166 = jjproto_targetjjtype;
                const struct ReturnValue _23 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get165, _get166, dong_porf_js_microbench_jjporfjjpos, 1);
                jjlast_type = _23.type;
                jjlast_type = 2;
                _r781 = _23.value != 34;
              } else {
                _get167 = logictmpi;
                jjlast_type = 2;
                _r781 = _get167;
              }
            // end
            j781:;
            // if 
              if ((_r781) != 0) {
              }
            // end
            j782:;
            const struct ReturnValue _24 = dong_porf_js_microbench_parseValue();
            jjlast_type = _24.type;
            _get168 = jjlast_type;
            keyjjtype = _get168;
            key = _24.value;
            const struct ReturnValue _25 = dong_porf_js_microbench_skipWhitespace();
            jjlast_type = _25.type;
            (void) _25.value;
            logictmpi = dong_porf_js_microbench_jjporfjjpos >= dong_porf_js_microbench_jjporfjjlen;
            _get169 = logictmpi;
            // if i32
            i32 _r783;
              if ((_get169) == 0) {
                jjproto_target = dong_porf_js_microbench_jjporfjjtext;
                jjproto_targetjjtype = 195;
                _get170 = jjproto_target;
                _get171 = jjproto_targetjjtype;
                const struct ReturnValue _26 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get170, _get171, dong_porf_js_microbench_jjporfjjpos, 1);
                jjlast_type = _26.type;
                jjlast_type = 2;
                _r783 = _26.value != 58;
              } else {
                _get172 = logictmpi;
                jjlast_type = 2;
                _r783 = _get172;
              }
            // end
            j783:;
            // if 
              if ((_r783) != 0) {
              }
            // end
            j784:;
            dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
            const struct ReturnValue _27 = dong_porf_js_microbench_parseValue();
            jjlast_type = _27.type;
            _get173 = jjlast_type;
            valuejjtype = _get173;
            value = _27.value;
            _get174 = obj;
            jjmember_obj_183 = _get174;
            _get175 = key;
            jjmember_prop_183 = _get175;
            _get176 = objjjtype;
            jjtypeswitch_tmp1 = _get176;
            // block 
              _get177 = jjtypeswitch_tmp1;
              // if 
                if (_get177 == 0) {
                  goto j785;
                }
              // end
              j786:;
              _get178 = jjtypeswitch_tmp1;
              // if 
                if (_get178 == 72) {
                  _get179 = jjmember_obj_183;
                  _get180 = jjmember_prop_183;
                  jjmember_setter_ptr_tmp = (u32)(_get179) + ((u32)(_get180) * 9);
                  _get181 = jjmember_setter_ptr_tmp;
                  _get182 = value;
                  f64_store(0, 4, _get181, _get182);
                  _get183 = jjmember_setter_ptr_tmp;
                  _get184 = valuejjtype;
                  i32_store8(0, 12, _get183, _get184);
                  goto j785;
                }
              // end
              j787:;
              _get185 = jjmember_obj_183;
              _get186 = objjjtype;
              _get187 = jjmember_prop_183;
              _get188 = keyjjtype;
              const struct ReturnValue _28 = dong_porf_js_microbench__ecma262_ToPropertyKey(_get187, _get188);
              jjswap = _28.type;
              _get189 = jjswap;
              _get190 = value;
              _get191 = valuejjtype;
              const struct ReturnValue _29 = dong_porf_js_microbench__Porffor_object_set((i32)(_get185), _get186, (i32)(_28.value), _get189, _get190, _get191);
              (void) _29.type;
              (void) _29.value;
            // end
            j785:;
            const struct ReturnValue _30 = dong_porf_js_microbench_skipWhitespace();
            jjlast_type = _30.type;
            (void) _30.value;
            // if 
              if (dong_porf_js_microbench_jjporfjjpos >= dong_porf_js_microbench_jjporfjjlen) {
              }
            // end
            j817:;
            jjproto_target = dong_porf_js_microbench_jjporfjjtext;
            jjproto_targetjjtype = 195;
            _get192 = jjproto_target;
            _get193 = jjproto_targetjjtype;
            const struct ReturnValue _31 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get192, _get193, dong_porf_js_microbench_jjporfjjpos, 1);
            jjlast_type = _31.type;
            next = _31.value;
            _get194 = next;
            // if 
              if (_get194 == 125) {
                dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
                goto j780;
              }
            // end
            j818:;
            _get195 = next;
            // if 
              if (_get195 == 44) {
                dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
                goto j779;
              }
            // end
            j819:;
            goto j779;
          }
        // end
        j780:;
      // end
      _get196 = obj;
      _get197 = objjjtype;
      return (struct ReturnValue){ _get196, _get197 };
    }
  // end
  j776:;
  _get198 = c;
  logictmpi = _get198 >= 48;
  _get199 = logictmpi;
  // if i32
  i32 _r820;
    if ((_get199) != 0) {
      _get200 = c;
      jjlast_type = 2;
      _r820 = _get200 <= 57;
    } else {
      _get201 = logictmpi;
      jjlast_type = 2;
      _r820 = _get201;
    }
  // end
  j820:;
  logictmpi = _r820;
  _get202 = logictmpi;
  jjlogicinner_tmp_int = _get202;
  _get203 = jjlast_type;
  jjtypeswitch_tmp1 = _get203;
  // block i32
  i32 _r821;
    _get204 = jjtypeswitch_tmp1;
    _get205 = jjtypeswitch_tmp1;
    // if 
      if (((_get204 == 67) | (_get205 == 195)) != 0) {
        _get206 = jjlogicinner_tmp_int;
        _r821 = (i32_load(1, 0, _get206)) == 0;
        goto j821;
      }
    // end
    j822:;
    _get207 = jjlogicinner_tmp_int;
    _r821 = (_get207) == 0;
  // end
  j821:;
  // if i32
  i32 _r823;
    if ((_r821) != 0) {
      _get208 = c;
      jjlast_type = 2;
      _r823 = _get208 == 45;
    } else {
      _get209 = logictmpi;
      _get210 = jjlast_type;
      jjlast_type = _get210;
      _r823 = _get209;
    }
  // end
  j823:;
  // if 
    if ((_r823) != 0) {
      start = dong_porf_js_microbench_jjporfjjpos;
      _get211 = c;
      // if 
        if (_get211 == 45) {
          dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
        }
      // end
      j825:;
      // loop 
      j826:;
        // if 
          if (dong_porf_js_microbench_jjporfjjpos < dong_porf_js_microbench_jjporfjjlen) {
            jjproto_target = dong_porf_js_microbench_jjporfjjtext;
            jjproto_targetjjtype = 195;
            _get212 = jjproto_target;
            _get213 = jjproto_targetjjtype;
            const struct ReturnValue _32 = dong_porf_js_microbench__ByteString_prototype_charCodeAt(_get212, _get213, dong_porf_js_microbench_jjporfjjpos, 1);
            jjlast_type = _32.type;
            ch = _32.value;
            _get214 = ch;
            logictmpi = _get214 >= 48;
            _get215 = logictmpi;
            // if i32
            i32 _r828;
              if ((_get215) != 0) {
                _get216 = ch;
                jjlast_type = 2;
                _r828 = _get216 <= 57;
              } else {
                _get217 = logictmpi;
                jjlast_type = 2;
                _r828 = _get217;
              }
            // end
            j828:;
            // if 
              if ((_r828) != 0) {
                dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
              } else {
                _get218 = ch;
                logictmpi = _get218 == 46;
                _get219 = logictmpi;
                // if i32
                i32 _r830;
                  if ((_get219) == 0) {
                    _get220 = ch;
                    jjlast_type = 2;
                    _r830 = _get220 == 101;
                  } else {
                    _get221 = logictmpi;
                    jjlast_type = 2;
                    _r830 = _get221;
                  }
                // end
                j830:;
                logictmpi = _r830;
                _get222 = logictmpi;
                jjlogicinner_tmp_int = _get222;
                _get223 = jjlast_type;
                jjtypeswitch_tmp1 = _get223;
                // block i32
                i32 _r831;
                  _get224 = jjtypeswitch_tmp1;
                  _get225 = jjtypeswitch_tmp1;
                  // if 
                    if (((_get224 == 67) | (_get225 == 195)) != 0) {
                      _get226 = jjlogicinner_tmp_int;
                      _r831 = (i32_load(1, 0, _get226)) == 0;
                      goto j831;
                    }
                  // end
                  j832:;
                  _get227 = jjlogicinner_tmp_int;
                  _r831 = (_get227) == 0;
                // end
                j831:;
                // if i32
                i32 _r833;
                  if ((_r831) != 0) {
                    _get228 = ch;
                    jjlast_type = 2;
                    _r833 = _get228 == 69;
                  } else {
                    _get229 = logictmpi;
                    _get230 = jjlast_type;
                    jjlast_type = _get230;
                    _r833 = _get229;
                  }
                // end
                j833:;
                // if 
                  if ((_r833) != 0) {
                    dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
                  } else {
                    _get231 = ch;
                    logictmpi = _get231 == 43;
                    _get232 = logictmpi;
                    // if i32
                    i32 _r835;
                      if ((_get232) == 0) {
                        _get233 = ch;
                        jjlast_type = 2;
                        _r835 = _get233 == 45;
                      } else {
                        _get234 = logictmpi;
                        jjlast_type = 2;
                        _r835 = _get234;
                      }
                    // end
                    j835:;
                    logictmpi = _r835;
                    _get235 = logictmpi;
                    jjlogicinner_tmp_int = _get235;
                    _get236 = jjlast_type;
                    jjtypeswitch_tmp1 = _get236;
                    // block i32
                    i32 _r836;
                      _get237 = jjtypeswitch_tmp1;
                      _get238 = jjtypeswitch_tmp1;
                      // if 
                        if (((_get237 == 67) | (_get238 == 195)) != 0) {
                          _get239 = jjlogicinner_tmp_int;
                          _r836 = i32_load(1, 0, _get239);
                          goto j836;
                        }
                      // end
                      j837:;
                      _get240 = jjlogicinner_tmp_int;
                      _r836 = _get240;
                    // end
                    j836:;
                    // if i32
                    i32 _r838;
                      if ((_r836) != 0) {
                        _get241 = start;
                        jjlast_type = 2;
                        _r838 = dong_porf_js_microbench_jjporfjjpos > (_get241 + 1);
                      } else {
                        _get242 = logictmpi;
                        _get243 = jjlast_type;
                        jjlast_type = _get243;
                        _r838 = _get242;
                      }
                    // end
                    j838:;
                    // if 
                      if ((_r838) != 0) {
                        dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos + 1;
                      } else {
                        goto j827;
                      }
                    // end
                    j839:;
                  }
                // end
                j834:;
              }
            // end
            j829:;
            goto j826;
          }
        // end
        j827:;
      // end
      _get244 = start;
      const struct ReturnValue _33 = dong_porf_js_microbench__ByteString_prototype_slice((i32)(dong_porf_js_microbench_jjporfjjtext), 195, (i32)(_get244), 1, (i32)(dong_porf_js_microbench_jjporfjjpos), 1);
      jjlast_type = _33.type;
      _get245 = jjlast_type;
      return (struct ReturnValue){ dong_porf_js_microbench__ecma262_StringToNumber((f64)(_33.value), _get245), 1 };
    }
  // end
  j824:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_js_microbench__JSON_parse(f64 _, i32 _jjtype) {
  i32 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = _;
  dong_porf_js_microbench_jjporfjjtext = _get0;
  dong_porf_js_microbench_jjporfjjpos = 0;
  dong_porf_js_microbench_jjporfjjlen = (f64)(i32_load(1, 0, (u32)(dong_porf_js_microbench_jjporfjjtext)));
  const struct ReturnValue _0 = dong_porf_js_microbench_parseValue();
  jjlast_type = _0.type;
  _get1 = jjlast_type;
  return (struct ReturnValue){ _0.value, _get1 };
}

static struct ReturnValue dong_porf_js_microbench_dongLog(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 msg, i32 msgjjtype) {
  i32 _get4;
  f64 _get3;
  f64 _get2;
  i32 _get1;
  f64 _get0;
  i32 jjlast_type = 0;

  _get0 = msg;
  _get1 = msgjjtype;
  const struct ReturnValue _0 = dong_porf_js_microbench_toUtf8(0, 0, 0, 0, _get0, _get1);
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
  j854:;
  return (struct ReturnValue){ 0, 0 };
}

int dong_porf_js_microbench_main() {
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
  f64 _get25;
  i32 _get24;
  f64 _get23;
  i32 _get22;
  i32 _get21;
  f64 _get20;
  f64 _get19;
  f64 _get18;
  f64 _get17;
  i32 _get16;
  f64 _get15;
  i32 _get14;
  f64 _get13;
  i32 _get12;
  i32 _get11;
  f64 _get10;
  f64 _get9;
  f64 _get8;
  f64 _get7;
  f64 _get6;
  f64 _get5;
  i32 _get4;
  i32 _get3;
  i32 _get2;
  i32 _get1;
  i32 _get0;
  dong_porf_js_microbench__porf_init();

  i32 jjobjectexpr_29 = 0;
  f64 jjmember_obj_30 = 0;
  f64 jjmember_prop_30 = 0;
  i32 jjlast_type = 0;
  f64 jjmember_obj_31 = 0;
  f64 jjmember_prop_31 = 0;
  f64 __tmpop_left = 0;
  f64 __tmpop_right = 0;
  f64 jjmember_obj_32 = 0;
  f64 jjmember_prop_32 = 0;
  i32 jjobjectexpr_44 = 0;

  dong_porf_js_microbench_METRIC_OFFSET_WIDTH = 0;
  dong_porf_js_microbench_METRIC_OFFSET_WIDTHjjtype = 1;
  dong_porf_js_microbench_METRIC_OFFSET_HEIGHT = 1;
  dong_porf_js_microbench_METRIC_OFFSET_HEIGHTjjtype = 1;
  dong_porf_js_microbench_METRIC_OFFSET_TOP = 2;
  dong_porf_js_microbench_METRIC_OFFSET_TOPjjtype = 1;
  dong_porf_js_microbench_METRIC_OFFSET_LEFT = 3;
  dong_porf_js_microbench_METRIC_OFFSET_LEFTjjtype = 1;
  dong_porf_js_microbench_METRIC_CLIENT_WIDTH = 4;
  dong_porf_js_microbench_METRIC_CLIENT_WIDTHjjtype = 1;
  dong_porf_js_microbench_METRIC_CLIENT_HEIGHT = 5;
  dong_porf_js_microbench_METRIC_CLIENT_HEIGHTjjtype = 1;
  dong_porf_js_microbench_METRIC_SCROLL_WIDTH = 6;
  dong_porf_js_microbench_METRIC_SCROLL_WIDTHjjtype = 1;
  dong_porf_js_microbench_METRIC_SCROLL_HEIGHT = 7;
  dong_porf_js_microbench_METRIC_SCROLL_HEIGHTjjtype = 1;
  dong_porf_js_microbench_bench_name = 0;
  dong_porf_js_microbench_bench_namejjtype = 195;
  dong_porf_js_microbench_bench_iterations = 0;
  dong_porf_js_microbench_bench_iterationsjjtype = 1;
  dong_porf_js_microbench_bench_start = 0;
  dong_porf_js_microbench_bench_startjjtype = 1;
  dong_porf_js_microbench_bench_name = 16;
  dong_porf_js_microbench_bench_namejjtype = 195;
  (void) dong_porf_js_microbench_bench_name;
  dong_porf_js_microbench_bench_iterations = 1000000;
  dong_porf_js_microbench_bench_iterationsjjtype = 1;
  (void) dong_porf_js_microbench_bench_iterations;
  dong_porf_js_microbench_bench_start = __porf_import_dong_time_now();
  _get0 = jjlast_type;
  dong_porf_js_microbench_bench_startjjtype = _get0;
  (void) dong_porf_js_microbench_bench_start;
  dong_porf_js_microbench_i = 0;
  dong_porf_js_microbench_ijjtype = 1;
  dong_porf_js_microbench_obj_x = 0;
  dong_porf_js_microbench_obj_xjjtype = 1;
  // loop 
  j0:;
    // if 
      if (dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations) {
        jjobjectexpr_29 = dong_porf_js_microbench__Porffor_malloc(16384);
        _get1 = jjobjectexpr_29;
        _get2 = jjobjectexpr_29;
        dong_porf_js_microbench__Porffor_object_expr_init(_get2, 7, 37, 195, 1, 1);
        _get3 = jjobjectexpr_29;
        dong_porf_js_microbench__Porffor_object_expr_init(_get3, 7, 1657, 195, 2, 1);
        _get4 = jjobjectexpr_29;
        dong_porf_js_microbench__Porffor_object_expr_init(_get4, 7, 1664, 195, 3, 1);
        dong_porf_js_microbench_obj = (f64)(_get1);
        dong_porf_js_microbench_objjjtype = 7;
        // block f64
        f64 _r567;
          // block f64
          f64 _r568;
            jjmember_prop_30 = 37;
            jjmember_obj_30 = dong_porf_js_microbench_obj;
            // if f64
            f64 _r569;
              if (dong_porf_js_microbench_objjjtype == 0) {
                _r569 = 0;
              } else {
                _get5 = jjmember_obj_30;
                _get6 = jjmember_prop_30;
                const struct ReturnValue _0 = dong_porf_js_microbench__Porffor_object_get_withHash((i32)(_get5), dong_porf_js_microbench_objjjtype, (u32)(_get6), 195, -155065054, 1);
                jjlast_type = _0.type;
                _r569 = _0.value;
              }
            // end
            j569:;
            __tmpop_left = _r569;
            _get7 = __tmpop_left;
            jjmember_prop_31 = 1657;
            jjmember_obj_31 = dong_porf_js_microbench_obj;
            // if f64
            f64 _r570;
              if (dong_porf_js_microbench_objjjtype == 0) {
                _r570 = 0;
              } else {
                _get8 = jjmember_obj_31;
                _get9 = jjmember_prop_31;
                const struct ReturnValue _1 = dong_porf_js_microbench__Porffor_object_get_withHash((i32)(_get8), dong_porf_js_microbench_objjjtype, (u32)(_get9), 195, 309633745, 1);
                jjlast_type = _1.type;
                _r570 = _1.value;
              }
            // end
            j570:;
            __tmpop_right = _r570;
            _get10 = __tmpop_right;
            _get11 = jjlast_type;
            _get12 = jjlast_type;
            // if 
              if ((((_get11 | 128) == 195) | ((_get12 | 128) == 195)) != 0) {
                _get13 = __tmpop_left;
                _get14 = jjlast_type;
                _get15 = __tmpop_right;
                _get16 = jjlast_type;
                const struct ReturnValue _2 = dong_porf_js_microbench__Porffor_concatStrings(_get13, _get14, _get15, _get16);
                jjlast_type = _2.type;
                _r568 = _2.value;
                goto j568;
              }
            // end
            j571:;
            jjlast_type = 1;
            _r568 = _get7 + _get10;
          // end
          j568:;
          __tmpop_left = _r568;
          _get17 = __tmpop_left;
          jjmember_prop_32 = 1664;
          jjmember_obj_32 = dong_porf_js_microbench_obj;
          // if f64
          f64 _r572;
            if (dong_porf_js_microbench_objjjtype == 0) {
              _r572 = 0;
            } else {
              _get18 = jjmember_obj_32;
              _get19 = jjmember_prop_32;
              const struct ReturnValue _3 = dong_porf_js_microbench__Porffor_object_get_withHash((i32)(_get18), dong_porf_js_microbench_objjjtype, (u32)(_get19), 195, -1854417746, 1);
              jjlast_type = _3.type;
              _r572 = _3.value;
            }
          // end
          j572:;
          __tmpop_right = _r572;
          _get20 = __tmpop_right;
          _get21 = jjlast_type;
          _get22 = jjlast_type;
          // if 
            if ((((_get21 | 128) == 195) | ((_get22 | 128) == 195)) != 0) {
              _get23 = __tmpop_left;
              _get24 = jjlast_type;
              _get25 = __tmpop_right;
              _get26 = jjlast_type;
              const struct ReturnValue _4 = dong_porf_js_microbench__Porffor_concatStrings(_get23, _get24, _get25, _get26);
              jjlast_type = _4.type;
              _r567 = _4.value;
              goto j567;
            }
          // end
          j573:;
          jjlast_type = 1;
          _r567 = _get17 + _get20;
        // end
        j567:;
        dong_porf_js_microbench_obj_x = _r567;
        _get27 = jjlast_type;
        dong_porf_js_microbench_obj_xjjtype = _get27;
        (void) dong_porf_js_microbench_obj_x;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        goto j0;
      }
    // end
    j1:;
  // end
  const struct ReturnValue _5 = dong_porf_js_microbench_bench_finish(0, 0, 0, 0);
  jjlast_type = _5.type;
  (void) _5.value;
  dong_porf_js_microbench_bench_name = 2021;
  dong_porf_js_microbench_bench_namejjtype = 195;
  (void) dong_porf_js_microbench_bench_name;
  dong_porf_js_microbench_bench_iterations = 200000;
  dong_porf_js_microbench_bench_iterationsjjtype = 1;
  (void) dong_porf_js_microbench_bench_iterations;
  dong_porf_js_microbench_bench_start = __porf_import_dong_time_now();
  _get28 = jjlast_type;
  dong_porf_js_microbench_bench_startjjtype = _get28;
  (void) dong_porf_js_microbench_bench_start;
  dong_porf_js_microbench_i = 0;
  dong_porf_js_microbench_ijjtype = 1;
  (void) dong_porf_js_microbench_i;
  // loop 
  j692:;
    // if 
      if (dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations) {
        const struct ReturnValue _6 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _6.type;
        (void) _6.value;
        const struct ReturnValue _7 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _7.type;
        (void) _7.value;
        const struct ReturnValue _8 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _8.type;
        (void) _8.value;
        const struct ReturnValue _9 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _9.type;
        (void) _9.value;
        const struct ReturnValue _10 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _10.type;
        (void) _10.value;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        if (!(dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations)) {
          goto j693;
        }
        const struct ReturnValue _11 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _11.type;
        (void) _11.value;
        const struct ReturnValue _12 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _12.type;
        (void) _12.value;
        const struct ReturnValue _13 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _13.type;
        (void) _13.value;
        const struct ReturnValue _14 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _14.type;
        (void) _14.value;
        const struct ReturnValue _15 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _15.type;
        (void) _15.value;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        if (!(dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations)) {
          goto j693;
        }
        const struct ReturnValue _16 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _16.type;
        (void) _16.value;
        const struct ReturnValue _17 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _17.type;
        (void) _17.value;
        const struct ReturnValue _18 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _18.type;
        (void) _18.value;
        const struct ReturnValue _19 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _19.type;
        (void) _19.value;
        const struct ReturnValue _20 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _20.type;
        (void) _20.value;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        if (!(dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations)) {
          goto j693;
        }
        const struct ReturnValue _21 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _21.type;
        (void) _21.value;
        const struct ReturnValue _22 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _22.type;
        (void) _22.value;
        const struct ReturnValue _23 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _23.type;
        (void) _23.value;
        const struct ReturnValue _24 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _24.type;
        (void) _24.value;
        const struct ReturnValue _25 = dong_porf_js_microbench_noop(0, 0, 0, 0);
        jjlast_type = _25.type;
        (void) _25.value;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        goto j692;
      }
    // end
    j693:;
  // end
  const struct ReturnValue _26 = dong_porf_js_microbench_bench_finish(0, 0, 0, 0);
  jjlast_type = _26.type;
  (void) _26.value;
  dong_porf_js_microbench_bench_name = 2040;
  dong_porf_js_microbench_bench_namejjtype = 195;
  (void) dong_porf_js_microbench_bench_name;
  dong_porf_js_microbench_bench_iterations = 100000;
  dong_porf_js_microbench_bench_iterationsjjtype = 1;
  (void) dong_porf_js_microbench_bench_iterations;
  dong_porf_js_microbench_bench_start = __porf_import_dong_time_now();
  _get29 = jjlast_type;
  dong_porf_js_microbench_bench_startjjtype = _get29;
  (void) dong_porf_js_microbench_bench_start;
  dong_porf_js_microbench_i = 0;
  dong_porf_js_microbench_ijjtype = 1;
  (void) dong_porf_js_microbench_i;
  // loop 
  j695:;
    // if 
      if (dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations) {
        jjobjectexpr_44 = dong_porf_js_microbench__Porffor_malloc(16384);
        _get30 = jjobjectexpr_44;
        _get31 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get31, 7, 2059, 195, 1, 1);
        _get32 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get32, 7, 2066, 195, 2, 1);
        _get33 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get33, 7, 2073, 195, 3, 1);
        dong_porf_js_microbench_o = (f64)(_get30);
        dong_porf_js_microbench_ojjtype = 7;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        if (!(dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations)) {
          goto j696;
        }
        jjobjectexpr_44 = dong_porf_js_microbench__Porffor_malloc(16384);
        _get34 = jjobjectexpr_44;
        _get35 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get35, 7, 2059, 195, 1, 1);
        _get36 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get36, 7, 2066, 195, 2, 1);
        _get37 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get37, 7, 2073, 195, 3, 1);
        dong_porf_js_microbench_o = (f64)(_get34);
        dong_porf_js_microbench_ojjtype = 7;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        if (!(dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations)) {
          goto j696;
        }
        jjobjectexpr_44 = dong_porf_js_microbench__Porffor_malloc(16384);
        _get38 = jjobjectexpr_44;
        _get39 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get39, 7, 2059, 195, 1, 1);
        _get40 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get40, 7, 2066, 195, 2, 1);
        _get41 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get41, 7, 2073, 195, 3, 1);
        dong_porf_js_microbench_o = (f64)(_get38);
        dong_porf_js_microbench_ojjtype = 7;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        if (!(dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations)) {
          goto j696;
        }
        jjobjectexpr_44 = dong_porf_js_microbench__Porffor_malloc(16384);
        _get42 = jjobjectexpr_44;
        _get43 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get43, 7, 2059, 195, 1, 1);
        _get44 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get44, 7, 2066, 195, 2, 1);
        _get45 = jjobjectexpr_44;
        dong_porf_js_microbench__Porffor_object_expr_init(_get45, 7, 2073, 195, 3, 1);
        dong_porf_js_microbench_o = (f64)(_get42);
        dong_porf_js_microbench_ojjtype = 7;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        goto j695;
      }
    // end
    j696:;
  // end
  const struct ReturnValue _27 = dong_porf_js_microbench_bench_finish(0, 0, 0, 0);
  jjlast_type = _27.type;
  (void) _27.value;
  dong_porf_js_microbench_bench_name = 2080;
  dong_porf_js_microbench_bench_namejjtype = 195;
  (void) dong_porf_js_microbench_bench_name;
  dong_porf_js_microbench_bench_iterations = 1000;
  dong_porf_js_microbench_bench_iterationsjjtype = 1;
  (void) dong_porf_js_microbench_bench_iterations;
  dong_porf_js_microbench_jsonStr = 2096;
  dong_porf_js_microbench_jsonStrjjtype = 195;
  dong_porf_js_microbench_bench_start = __porf_import_dong_time_now();
  _get46 = jjlast_type;
  dong_porf_js_microbench_bench_startjjtype = _get46;
  (void) dong_porf_js_microbench_bench_start;
  dong_porf_js_microbench_i = 0;
  dong_porf_js_microbench_ijjtype = 1;
  (void) dong_porf_js_microbench_i;
  // loop 
  j697:;
    // if 
      if (dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations) {
        const struct ReturnValue _28 = dong_porf_js_microbench__JSON_parse(dong_porf_js_microbench_jsonStr, dong_porf_js_microbench_jsonStrjjtype);
        jjlast_type = _28.type;
        _get47 = jjlast_type;
        dong_porf_js_microbench_parsedjjtype = _get47;
        dong_porf_js_microbench_parsed = _28.value;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        if (!(dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations)) {
          goto j698;
        }
        const struct ReturnValue _29 = dong_porf_js_microbench__JSON_parse(dong_porf_js_microbench_jsonStr, dong_porf_js_microbench_jsonStrjjtype);
        jjlast_type = _29.type;
        _get48 = jjlast_type;
        dong_porf_js_microbench_parsedjjtype = _get48;
        dong_porf_js_microbench_parsed = _29.value;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        if (!(dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations)) {
          goto j698;
        }
        const struct ReturnValue _30 = dong_porf_js_microbench__JSON_parse(dong_porf_js_microbench_jsonStr, dong_porf_js_microbench_jsonStrjjtype);
        jjlast_type = _30.type;
        _get49 = jjlast_type;
        dong_porf_js_microbench_parsedjjtype = _get49;
        dong_porf_js_microbench_parsed = _30.value;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        if (!(dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations)) {
          goto j698;
        }
        const struct ReturnValue _31 = dong_porf_js_microbench__JSON_parse(dong_porf_js_microbench_jsonStr, dong_porf_js_microbench_jsonStrjjtype);
        jjlast_type = _31.type;
        _get50 = jjlast_type;
        dong_porf_js_microbench_parsedjjtype = _get50;
        dong_porf_js_microbench_parsed = _31.value;
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        goto j697;
      }
    // end
    j698:;
  // end
  const struct ReturnValue _32 = dong_porf_js_microbench_bench_finish(0, 0, 0, 0);
  jjlast_type = _32.type;
  (void) _32.value;
  dong_porf_js_microbench_bench_name = 2511;
  dong_porf_js_microbench_bench_namejjtype = 195;
  (void) dong_porf_js_microbench_bench_name;
  dong_porf_js_microbench_bench_iterations = 10000;
  dong_porf_js_microbench_bench_iterationsjjtype = 1;
  (void) dong_porf_js_microbench_bench_iterations;
  dong_porf_js_microbench_bench_start = __porf_import_dong_time_now();
  _get51 = jjlast_type;
  dong_porf_js_microbench_bench_startjjtype = _get51;
  (void) dong_porf_js_microbench_bench_start;
  dong_porf_js_microbench_i = 0;
  dong_porf_js_microbench_ijjtype = 1;
  (void) dong_porf_js_microbench_i;
  // loop 
  j850:;
    // if 
      if (dong_porf_js_microbench_i < dong_porf_js_microbench_bench_iterations) {
        dong_porf_js_microbench_s = 0;
        dong_porf_js_microbench_sjjtype = 195;
        dong_porf_js_microbench_j = 0;
        dong_porf_js_microbench_jjjtype = 1;
        // loop 
        j852:;
          // if 
            if (dong_porf_js_microbench_j < 100) {
              const struct ReturnValue _33 = dong_porf_js_microbench__Porffor_concatStrings(dong_porf_js_microbench_s, dong_porf_js_microbench_sjjtype, 37, 195);
              jjlast_type = _33.type;
              _get52 = jjlast_type;
              dong_porf_js_microbench_sjjtype = _get52;
              dong_porf_js_microbench_s = _33.value;
              dong_porf_js_microbench_j = dong_porf_js_microbench_j + 1;
              dong_porf_js_microbench_jjjtype = 1;
              (void) dong_porf_js_microbench_j;
              if (!(dong_porf_js_microbench_j < 100)) {
                goto j853;
              }
              const struct ReturnValue _34 = dong_porf_js_microbench__Porffor_concatStrings(dong_porf_js_microbench_s, dong_porf_js_microbench_sjjtype, 37, 195);
              jjlast_type = _34.type;
              _get53 = jjlast_type;
              dong_porf_js_microbench_sjjtype = _get53;
              dong_porf_js_microbench_s = _34.value;
              dong_porf_js_microbench_j = dong_porf_js_microbench_j + 1;
              dong_porf_js_microbench_jjjtype = 1;
              (void) dong_porf_js_microbench_j;
              if (!(dong_porf_js_microbench_j < 100)) {
                goto j853;
              }
              const struct ReturnValue _35 = dong_porf_js_microbench__Porffor_concatStrings(dong_porf_js_microbench_s, dong_porf_js_microbench_sjjtype, 37, 195);
              jjlast_type = _35.type;
              _get54 = jjlast_type;
              dong_porf_js_microbench_sjjtype = _get54;
              dong_porf_js_microbench_s = _35.value;
              dong_porf_js_microbench_j = dong_porf_js_microbench_j + 1;
              dong_porf_js_microbench_jjjtype = 1;
              (void) dong_porf_js_microbench_j;
              if (!(dong_porf_js_microbench_j < 100)) {
                goto j853;
              }
              const struct ReturnValue _36 = dong_porf_js_microbench__Porffor_concatStrings(dong_porf_js_microbench_s, dong_porf_js_microbench_sjjtype, 37, 195);
              jjlast_type = _36.type;
              _get55 = jjlast_type;
              dong_porf_js_microbench_sjjtype = _get55;
              dong_porf_js_microbench_s = _36.value;
              dong_porf_js_microbench_j = dong_porf_js_microbench_j + 1;
              dong_porf_js_microbench_jjjtype = 1;
              (void) dong_porf_js_microbench_j;
              goto j852;
            }
          // end
          j853:;
        // end
        dong_porf_js_microbench_i = dong_porf_js_microbench_i + 1;
        dong_porf_js_microbench_ijjtype = 1;
        (void) dong_porf_js_microbench_i;
        goto j850;
      }
    // end
    j851:;
  // end
  const struct ReturnValue _37 = dong_porf_js_microbench_bench_finish(0, 0, 0, 0);
  jjlast_type = _37.type;
  (void) _37.value;
  const struct ReturnValue _38 = dong_porf_js_microbench_benchLog(0, 0, 0, 0, 2530, 195);
  jjlast_type = _38.type;
  (void) _38.value;
  const struct ReturnValue _39 = dong_porf_js_microbench_dongLog(0, 0, 0, 0, 2530, 195);
  jjlast_type = _39.type;
  _get56 = jjlast_type;

  return 0;
}

typedef struct {
  f64 dong_porf_js_microbench_METRIC_OFFSET_WIDTH;
  i32 dong_porf_js_microbench_METRIC_OFFSET_WIDTHjjtype;
  f64 dong_porf_js_microbench_METRIC_OFFSET_HEIGHT;
  i32 dong_porf_js_microbench_METRIC_OFFSET_HEIGHTjjtype;
  f64 dong_porf_js_microbench_METRIC_OFFSET_TOP;
  i32 dong_porf_js_microbench_METRIC_OFFSET_TOPjjtype;
  f64 dong_porf_js_microbench_METRIC_OFFSET_LEFT;
  i32 dong_porf_js_microbench_METRIC_OFFSET_LEFTjjtype;
  f64 dong_porf_js_microbench_METRIC_CLIENT_WIDTH;
  i32 dong_porf_js_microbench_METRIC_CLIENT_WIDTHjjtype;
  f64 dong_porf_js_microbench_METRIC_CLIENT_HEIGHT;
  i32 dong_porf_js_microbench_METRIC_CLIENT_HEIGHTjjtype;
  f64 dong_porf_js_microbench_METRIC_SCROLL_WIDTH;
  i32 dong_porf_js_microbench_METRIC_SCROLL_WIDTHjjtype;
  f64 dong_porf_js_microbench_METRIC_SCROLL_HEIGHT;
  i32 dong_porf_js_microbench_METRIC_SCROLL_HEIGHTjjtype;
  f64 dong_porf_js_microbench_bench_name;
  i32 dong_porf_js_microbench_bench_namejjtype;
  f64 dong_porf_js_microbench_bench_iterations;
  i32 dong_porf_js_microbench_bench_iterationsjjtype;
  f64 dong_porf_js_microbench_bench_start;
  i32 dong_porf_js_microbench_bench_startjjtype;
  f64 dong_porf_js_microbench_i;
  i32 dong_porf_js_microbench_ijjtype;
  f64 dong_porf_js_microbench_obj_x;
  i32 dong_porf_js_microbench_obj_xjjtype;
  f64 dong_porf_js_microbench_obj;
  i32 dong_porf_js_microbench_objjjtype;
  i32 dong_porf_js_microbench_jjporfjjcurrentPtr;
  i32 dong_porf_js_microbench_jjporfjjcurrentPtrjjglbl_inited;
  i32 dong_porf_js_microbench_jjporfjjendPtr;
  i32 dong_porf_js_microbench_jjporfjjendPtrjjglbl_inited;
  i32 dong_porf_js_microbench_jjporfjjunderlyingStore;
  i32 dong_porf_js_microbench_jjporfjjunderlyingStorejjglbl_inited;
  i32 dong_porf_js_microbench_jjporfjjgetptr___Object_prototype;
  i32 dong_porf_js_microbench_jjporfjjgetptr___Object_prototypejjglbl_inited;
  f64 dong_porf_js_microbench_o;
  i32 dong_porf_js_microbench_ojjtype;
  f64 dong_porf_js_microbench_jsonStr;
  i32 dong_porf_js_microbench_jsonStrjjtype;
  f64 dong_porf_js_microbench_parsed;
  i32 dong_porf_js_microbench_parsedjjtype;
  f64 dong_porf_js_microbench_jjporfjjtext;
  i32 dong_porf_js_microbench_jjporfjjtextjjglbl_inited;
  f64 dong_porf_js_microbench_jjporfjjpos;
  i32 dong_porf_js_microbench_jjporfjjposjjglbl_inited;
  f64 dong_porf_js_microbench_jjporfjjlen;
  i32 dong_porf_js_microbench_jjporfjjlenjjglbl_inited;
  f64 dong_porf_js_microbench_s;
  i32 dong_porf_js_microbench_sjjtype;
  f64 dong_porf_js_microbench_j;
  i32 dong_porf_js_microbench_jjjtype;
} dong_porf_js_microbench_state_t;

void dong_porf_js_microbench_state_capture(dong_porf_js_microbench_state_t* out) {
  out->dong_porf_js_microbench_METRIC_OFFSET_WIDTH = dong_porf_js_microbench_METRIC_OFFSET_WIDTH;
  out->dong_porf_js_microbench_METRIC_OFFSET_WIDTHjjtype = dong_porf_js_microbench_METRIC_OFFSET_WIDTHjjtype;
  out->dong_porf_js_microbench_METRIC_OFFSET_HEIGHT = dong_porf_js_microbench_METRIC_OFFSET_HEIGHT;
  out->dong_porf_js_microbench_METRIC_OFFSET_HEIGHTjjtype = dong_porf_js_microbench_METRIC_OFFSET_HEIGHTjjtype;
  out->dong_porf_js_microbench_METRIC_OFFSET_TOP = dong_porf_js_microbench_METRIC_OFFSET_TOP;
  out->dong_porf_js_microbench_METRIC_OFFSET_TOPjjtype = dong_porf_js_microbench_METRIC_OFFSET_TOPjjtype;
  out->dong_porf_js_microbench_METRIC_OFFSET_LEFT = dong_porf_js_microbench_METRIC_OFFSET_LEFT;
  out->dong_porf_js_microbench_METRIC_OFFSET_LEFTjjtype = dong_porf_js_microbench_METRIC_OFFSET_LEFTjjtype;
  out->dong_porf_js_microbench_METRIC_CLIENT_WIDTH = dong_porf_js_microbench_METRIC_CLIENT_WIDTH;
  out->dong_porf_js_microbench_METRIC_CLIENT_WIDTHjjtype = dong_porf_js_microbench_METRIC_CLIENT_WIDTHjjtype;
  out->dong_porf_js_microbench_METRIC_CLIENT_HEIGHT = dong_porf_js_microbench_METRIC_CLIENT_HEIGHT;
  out->dong_porf_js_microbench_METRIC_CLIENT_HEIGHTjjtype = dong_porf_js_microbench_METRIC_CLIENT_HEIGHTjjtype;
  out->dong_porf_js_microbench_METRIC_SCROLL_WIDTH = dong_porf_js_microbench_METRIC_SCROLL_WIDTH;
  out->dong_porf_js_microbench_METRIC_SCROLL_WIDTHjjtype = dong_porf_js_microbench_METRIC_SCROLL_WIDTHjjtype;
  out->dong_porf_js_microbench_METRIC_SCROLL_HEIGHT = dong_porf_js_microbench_METRIC_SCROLL_HEIGHT;
  out->dong_porf_js_microbench_METRIC_SCROLL_HEIGHTjjtype = dong_porf_js_microbench_METRIC_SCROLL_HEIGHTjjtype;
  out->dong_porf_js_microbench_bench_name = dong_porf_js_microbench_bench_name;
  out->dong_porf_js_microbench_bench_namejjtype = dong_porf_js_microbench_bench_namejjtype;
  out->dong_porf_js_microbench_bench_iterations = dong_porf_js_microbench_bench_iterations;
  out->dong_porf_js_microbench_bench_iterationsjjtype = dong_porf_js_microbench_bench_iterationsjjtype;
  out->dong_porf_js_microbench_bench_start = dong_porf_js_microbench_bench_start;
  out->dong_porf_js_microbench_bench_startjjtype = dong_porf_js_microbench_bench_startjjtype;
  out->dong_porf_js_microbench_i = dong_porf_js_microbench_i;
  out->dong_porf_js_microbench_ijjtype = dong_porf_js_microbench_ijjtype;
  out->dong_porf_js_microbench_obj_x = dong_porf_js_microbench_obj_x;
  out->dong_porf_js_microbench_obj_xjjtype = dong_porf_js_microbench_obj_xjjtype;
  out->dong_porf_js_microbench_obj = dong_porf_js_microbench_obj;
  out->dong_porf_js_microbench_objjjtype = dong_porf_js_microbench_objjjtype;
  out->dong_porf_js_microbench_jjporfjjcurrentPtr = dong_porf_js_microbench_jjporfjjcurrentPtr;
  out->dong_porf_js_microbench_jjporfjjcurrentPtrjjglbl_inited = dong_porf_js_microbench_jjporfjjcurrentPtrjjglbl_inited;
  out->dong_porf_js_microbench_jjporfjjendPtr = dong_porf_js_microbench_jjporfjjendPtr;
  out->dong_porf_js_microbench_jjporfjjendPtrjjglbl_inited = dong_porf_js_microbench_jjporfjjendPtrjjglbl_inited;
  out->dong_porf_js_microbench_jjporfjjunderlyingStore = dong_porf_js_microbench_jjporfjjunderlyingStore;
  out->dong_porf_js_microbench_jjporfjjunderlyingStorejjglbl_inited = dong_porf_js_microbench_jjporfjjunderlyingStorejjglbl_inited;
  out->dong_porf_js_microbench_jjporfjjgetptr___Object_prototype = dong_porf_js_microbench_jjporfjjgetptr___Object_prototype;
  out->dong_porf_js_microbench_jjporfjjgetptr___Object_prototypejjglbl_inited = dong_porf_js_microbench_jjporfjjgetptr___Object_prototypejjglbl_inited;
  out->dong_porf_js_microbench_o = dong_porf_js_microbench_o;
  out->dong_porf_js_microbench_ojjtype = dong_porf_js_microbench_ojjtype;
  out->dong_porf_js_microbench_jsonStr = dong_porf_js_microbench_jsonStr;
  out->dong_porf_js_microbench_jsonStrjjtype = dong_porf_js_microbench_jsonStrjjtype;
  out->dong_porf_js_microbench_parsed = dong_porf_js_microbench_parsed;
  out->dong_porf_js_microbench_parsedjjtype = dong_porf_js_microbench_parsedjjtype;
  out->dong_porf_js_microbench_jjporfjjtext = dong_porf_js_microbench_jjporfjjtext;
  out->dong_porf_js_microbench_jjporfjjtextjjglbl_inited = dong_porf_js_microbench_jjporfjjtextjjglbl_inited;
  out->dong_porf_js_microbench_jjporfjjpos = dong_porf_js_microbench_jjporfjjpos;
  out->dong_porf_js_microbench_jjporfjjposjjglbl_inited = dong_porf_js_microbench_jjporfjjposjjglbl_inited;
  out->dong_porf_js_microbench_jjporfjjlen = dong_porf_js_microbench_jjporfjjlen;
  out->dong_porf_js_microbench_jjporfjjlenjjglbl_inited = dong_porf_js_microbench_jjporfjjlenjjglbl_inited;
  out->dong_porf_js_microbench_s = dong_porf_js_microbench_s;
  out->dong_porf_js_microbench_sjjtype = dong_porf_js_microbench_sjjtype;
  out->dong_porf_js_microbench_j = dong_porf_js_microbench_j;
  out->dong_porf_js_microbench_jjjtype = dong_porf_js_microbench_jjjtype;
}

void dong_porf_js_microbench_state_apply(const dong_porf_js_microbench_state_t* in) {
  dong_porf_js_microbench_METRIC_OFFSET_WIDTH = in->dong_porf_js_microbench_METRIC_OFFSET_WIDTH;
  dong_porf_js_microbench_METRIC_OFFSET_WIDTHjjtype = in->dong_porf_js_microbench_METRIC_OFFSET_WIDTHjjtype;
  dong_porf_js_microbench_METRIC_OFFSET_HEIGHT = in->dong_porf_js_microbench_METRIC_OFFSET_HEIGHT;
  dong_porf_js_microbench_METRIC_OFFSET_HEIGHTjjtype = in->dong_porf_js_microbench_METRIC_OFFSET_HEIGHTjjtype;
  dong_porf_js_microbench_METRIC_OFFSET_TOP = in->dong_porf_js_microbench_METRIC_OFFSET_TOP;
  dong_porf_js_microbench_METRIC_OFFSET_TOPjjtype = in->dong_porf_js_microbench_METRIC_OFFSET_TOPjjtype;
  dong_porf_js_microbench_METRIC_OFFSET_LEFT = in->dong_porf_js_microbench_METRIC_OFFSET_LEFT;
  dong_porf_js_microbench_METRIC_OFFSET_LEFTjjtype = in->dong_porf_js_microbench_METRIC_OFFSET_LEFTjjtype;
  dong_porf_js_microbench_METRIC_CLIENT_WIDTH = in->dong_porf_js_microbench_METRIC_CLIENT_WIDTH;
  dong_porf_js_microbench_METRIC_CLIENT_WIDTHjjtype = in->dong_porf_js_microbench_METRIC_CLIENT_WIDTHjjtype;
  dong_porf_js_microbench_METRIC_CLIENT_HEIGHT = in->dong_porf_js_microbench_METRIC_CLIENT_HEIGHT;
  dong_porf_js_microbench_METRIC_CLIENT_HEIGHTjjtype = in->dong_porf_js_microbench_METRIC_CLIENT_HEIGHTjjtype;
  dong_porf_js_microbench_METRIC_SCROLL_WIDTH = in->dong_porf_js_microbench_METRIC_SCROLL_WIDTH;
  dong_porf_js_microbench_METRIC_SCROLL_WIDTHjjtype = in->dong_porf_js_microbench_METRIC_SCROLL_WIDTHjjtype;
  dong_porf_js_microbench_METRIC_SCROLL_HEIGHT = in->dong_porf_js_microbench_METRIC_SCROLL_HEIGHT;
  dong_porf_js_microbench_METRIC_SCROLL_HEIGHTjjtype = in->dong_porf_js_microbench_METRIC_SCROLL_HEIGHTjjtype;
  dong_porf_js_microbench_bench_name = in->dong_porf_js_microbench_bench_name;
  dong_porf_js_microbench_bench_namejjtype = in->dong_porf_js_microbench_bench_namejjtype;
  dong_porf_js_microbench_bench_iterations = in->dong_porf_js_microbench_bench_iterations;
  dong_porf_js_microbench_bench_iterationsjjtype = in->dong_porf_js_microbench_bench_iterationsjjtype;
  dong_porf_js_microbench_bench_start = in->dong_porf_js_microbench_bench_start;
  dong_porf_js_microbench_bench_startjjtype = in->dong_porf_js_microbench_bench_startjjtype;
  dong_porf_js_microbench_i = in->dong_porf_js_microbench_i;
  dong_porf_js_microbench_ijjtype = in->dong_porf_js_microbench_ijjtype;
  dong_porf_js_microbench_obj_x = in->dong_porf_js_microbench_obj_x;
  dong_porf_js_microbench_obj_xjjtype = in->dong_porf_js_microbench_obj_xjjtype;
  dong_porf_js_microbench_obj = in->dong_porf_js_microbench_obj;
  dong_porf_js_microbench_objjjtype = in->dong_porf_js_microbench_objjjtype;
  dong_porf_js_microbench_jjporfjjcurrentPtr = in->dong_porf_js_microbench_jjporfjjcurrentPtr;
  dong_porf_js_microbench_jjporfjjcurrentPtrjjglbl_inited = in->dong_porf_js_microbench_jjporfjjcurrentPtrjjglbl_inited;
  dong_porf_js_microbench_jjporfjjendPtr = in->dong_porf_js_microbench_jjporfjjendPtr;
  dong_porf_js_microbench_jjporfjjendPtrjjglbl_inited = in->dong_porf_js_microbench_jjporfjjendPtrjjglbl_inited;
  dong_porf_js_microbench_jjporfjjunderlyingStore = in->dong_porf_js_microbench_jjporfjjunderlyingStore;
  dong_porf_js_microbench_jjporfjjunderlyingStorejjglbl_inited = in->dong_porf_js_microbench_jjporfjjunderlyingStorejjglbl_inited;
  dong_porf_js_microbench_jjporfjjgetptr___Object_prototype = in->dong_porf_js_microbench_jjporfjjgetptr___Object_prototype;
  dong_porf_js_microbench_jjporfjjgetptr___Object_prototypejjglbl_inited = in->dong_porf_js_microbench_jjporfjjgetptr___Object_prototypejjglbl_inited;
  dong_porf_js_microbench_o = in->dong_porf_js_microbench_o;
  dong_porf_js_microbench_ojjtype = in->dong_porf_js_microbench_ojjtype;
  dong_porf_js_microbench_jsonStr = in->dong_porf_js_microbench_jsonStr;
  dong_porf_js_microbench_jsonStrjjtype = in->dong_porf_js_microbench_jsonStrjjtype;
  dong_porf_js_microbench_parsed = in->dong_porf_js_microbench_parsed;
  dong_porf_js_microbench_parsedjjtype = in->dong_porf_js_microbench_parsedjjtype;
  dong_porf_js_microbench_jjporfjjtext = in->dong_porf_js_microbench_jjporfjjtext;
  dong_porf_js_microbench_jjporfjjtextjjglbl_inited = in->dong_porf_js_microbench_jjporfjjtextjjglbl_inited;
  dong_porf_js_microbench_jjporfjjpos = in->dong_porf_js_microbench_jjporfjjpos;
  dong_porf_js_microbench_jjporfjjposjjglbl_inited = in->dong_porf_js_microbench_jjporfjjposjjglbl_inited;
  dong_porf_js_microbench_jjporfjjlen = in->dong_porf_js_microbench_jjporfjjlen;
  dong_porf_js_microbench_jjporfjjlenjjglbl_inited = in->dong_porf_js_microbench_jjporfjjlenjjglbl_inited;
  dong_porf_js_microbench_s = in->dong_porf_js_microbench_s;
  dong_porf_js_microbench_sjjtype = in->dong_porf_js_microbench_sjjtype;
  dong_porf_js_microbench_j = in->dong_porf_js_microbench_j;
  dong_porf_js_microbench_jjjtype = in->dong_porf_js_microbench_jjjtype;
}
