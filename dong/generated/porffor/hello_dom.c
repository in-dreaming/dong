// Porffor module: hello_dom
#include "dong_porf_runtime.h"
char* dong_porf_hello_dom_memory; u32 dong_porf_hello_dom_memory_pages = 1;


__attribute__((import_module(""), import_name("h")))
extern f64 __porf_import_dong_dom_getElementById(f64);

__attribute__((import_module(""), import_name("m")))
extern void __porf_import_dong_stage_0(f64);

__attribute__((import_module(""), import_name("n")))
extern void __porf_import_dong_stage_1(f64);

__attribute__((import_module(""), import_name("o")))
extern void __porf_import_dong_stage_2(f64);

__attribute__((import_module(""), import_name("q")))
extern void __porf_import_dong_commit_addEventListener();

__attribute__((import_module(""), import_name("p")))
extern void __porf_import_dong_commit_set_textContent();

__attribute__((import_module(""), import_name("e")))
extern void __porf_import_dong_print(f64);
static struct ReturnValue dong_porf_hello_dom_getElementById(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 id, i32 idjjtype);
static struct ReturnValue dong_porf_hello_dom_setTextContent(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 text, i32 textjjtype);
static struct ReturnValue dong_porf_hello_dom_addEventListener(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 type, i32 typejjtype, f64 handlerName, i32 handlerNamejjtype);
static struct ReturnValue dong_porf_hello_dom_dongLog(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 msg, i32 msgjjtype);
static i32 dong_porf_hello_dom___Porffor_object_isObject(i32 arg, i32 argjjtype);

f64 dong_porf_hello_dom_statusId = 0;
i32 dong_porf_hello_dom_statusIdjjtype = 0;
f64 dong_porf_hello_dom_btnId = 0;
i32 dong_porf_hello_dom_btnIdjjtype = 0;

static i32 dong_porf_hello_dom___Porffor_object_isObject(i32 arg, i32 argjjtype) {
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

static struct ReturnValue dong_porf_hello_dom_getElementById(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 id, i32 idjjtype) {
  i32 _get7;
  f64 _get6;
  i32 _get5;
  f64 _get4;
  i32 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  f64 jjreturn = 0;
  i32 jjreturnjjtype = 0;

  _get0 = id;
  jjreturn = __porf_import_dong_dom_getElementById(_get0);
  jjreturnjjtype = 1;
  _get1 = jjnewtarget;
  // if 
    if (((u32)(_get1)) != 0) {
      _get2 = jjreturn;
      _get3 = jjreturnjjtype;
      // if 
        if ((dong_porf_hello_dom___Porffor_object_isObject((i32)(_get2), _get3)) == 0) {
          _get4 = jjthis;
          _get5 = jjthisjjtype;
          return (struct ReturnValue){ _get4, _get5 };
        }
      // end
      j1:;
    }
  // end
  j0:;
  _get6 = jjreturn;
  _get7 = jjreturnjjtype;
  return (struct ReturnValue){ _get6, _get7 };
}

static struct ReturnValue dong_porf_hello_dom_addEventListener(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 type, i32 typejjtype, f64 handlerName, i32 handlerNamejjtype) {
  i32 _get5;
  f64 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  _get0 = nodeId;
  __porf_import_dong_stage_0(_get0);
  _get1 = type;
  __porf_import_dong_stage_1(_get1);
  _get2 = handlerName;
  __porf_import_dong_stage_2(_get2);
  __porf_import_dong_commit_addEventListener();
  _get3 = jjnewtarget;
  // if 
    if (((u32)(_get3)) != 0) {
      _get4 = jjthis;
      _get5 = jjthisjjtype;
      return (struct ReturnValue){ _get4, _get5 };
    }
  // end
  j2:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_hello_dom_setTextContent(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 text, i32 textjjtype) {
  i32 _get4;
  f64 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  _get0 = nodeId;
  __porf_import_dong_stage_0(_get0);
  _get1 = text;
  __porf_import_dong_stage_1(_get1);
  __porf_import_dong_commit_set_textContent();
  _get2 = jjnewtarget;
  // if 
    if (((u32)(_get2)) != 0) {
      _get3 = jjthis;
      _get4 = jjthisjjtype;
      return (struct ReturnValue){ _get3, _get4 };
    }
  // end
  j3:;
  return (struct ReturnValue){ 0, 0 };
}

static struct ReturnValue dong_porf_hello_dom_dongLog(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 msg, i32 msgjjtype) {
  i32 _get3;
  f64 _get2;
  f64 _get1;
  f64 _get0;
  _get0 = msg;
  __porf_import_dong_print(_get0);
  _get1 = jjnewtarget;
  // if 
    if (((u32)(_get1)) != 0) {
      _get2 = jjthis;
      _get3 = jjthisjjtype;
      return (struct ReturnValue){ _get2, _get3 };
    }
  // end
  j4:;
  return (struct ReturnValue){ 0, 0 };
}

int dong_porf_hello_dom_main() {
  i32 _get2;
  i32 _get1;
  i32 _get0;
  dong_porf_hello_dom_memory = calloc(1, dong_porf_hello_dom_memory_pages * 65536);

  dong_porf_hello_dom_memory[16]=(u8)6;dong_porf_hello_dom_memory[20]=(u8)115;dong_porf_hello_dom_memory[21]=(u8)116;dong_porf_hello_dom_memory[22]=(u8)97;dong_porf_hello_dom_memory[23]=(u8)116;dong_porf_hello_dom_memory[24]=(u8)117;dong_porf_hello_dom_memory[25]=(u8)115;
  dong_porf_hello_dom_memory[28]=(u8)3;dong_porf_hello_dom_memory[32]=(u8)98;dong_porf_hello_dom_memory[33]=(u8)116;dong_porf_hello_dom_memory[34]=(u8)110;
  dong_porf_hello_dom_memory[37]=(u8)5;dong_porf_hello_dom_memory[41]=(u8)99;dong_porf_hello_dom_memory[42]=(u8)108;dong_porf_hello_dom_memory[43]=(u8)105;dong_porf_hello_dom_memory[44]=(u8)99;dong_porf_hello_dom_memory[45]=(u8)107;
  dong_porf_hello_dom_memory[48]=(u8)10;dong_porf_hello_dom_memory[52]=(u8)111;dong_porf_hello_dom_memory[53]=(u8)110;dong_porf_hello_dom_memory[54]=(u8)66;dong_porf_hello_dom_memory[55]=(u8)116;dong_porf_hello_dom_memory[56]=(u8)110;dong_porf_hello_dom_memory[57]=(u8)67;dong_porf_hello_dom_memory[58]=(u8)108;dong_porf_hello_dom_memory[59]=(u8)105;dong_porf_hello_dom_memory[60]=(u8)99;dong_porf_hello_dom_memory[61]=(u8)107;
  dong_porf_hello_dom_memory[64]=(u8)5;dong_porf_hello_dom_memory[68]=(u8)114;dong_porf_hello_dom_memory[69]=(u8)101;dong_porf_hello_dom_memory[70]=(u8)97;dong_porf_hello_dom_memory[71]=(u8)100;dong_porf_hello_dom_memory[72]=(u8)121;
  dong_porf_hello_dom_memory[75]=(u8)16;dong_porf_hello_dom_memory[79]=(u8)104;dong_porf_hello_dom_memory[80]=(u8)101;dong_porf_hello_dom_memory[81]=(u8)108;dong_porf_hello_dom_memory[82]=(u8)108;dong_porf_hello_dom_memory[83]=(u8)111;dong_porf_hello_dom_memory[84]=(u8)95;dong_porf_hello_dom_memory[85]=(u8)100;dong_porf_hello_dom_memory[86]=(u8)111;dong_porf_hello_dom_memory[87]=(u8)109;dong_porf_hello_dom_memory[88]=(u8)32;dong_porf_hello_dom_memory[89]=(u8)108;dong_porf_hello_dom_memory[90]=(u8)111;dong_porf_hello_dom_memory[91]=(u8)97;dong_porf_hello_dom_memory[92]=(u8)100;dong_porf_hello_dom_memory[93]=(u8)101;dong_porf_hello_dom_memory[94]=(u8)100;

  i32 jjlast_type = 0;

  const struct ReturnValue _0 = dong_porf_hello_dom_getElementById(0, 0, 0, 0, 16, 195);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  dong_porf_hello_dom_statusIdjjtype = _get0;
  dong_porf_hello_dom_statusId = _0.value;
  const struct ReturnValue _1 = dong_porf_hello_dom_getElementById(0, 0, 0, 0, 28, 195);
  jjlast_type = _1.type;
  _get1 = jjlast_type;
  dong_porf_hello_dom_btnIdjjtype = _get1;
  dong_porf_hello_dom_btnId = _1.value;
  const struct ReturnValue _2 = dong_porf_hello_dom_addEventListener(0, 0, 0, 0, dong_porf_hello_dom_btnId, dong_porf_hello_dom_btnIdjjtype, 37, 195, 48, 195);
  jjlast_type = _2.type;
  (void) _2.value;
  const struct ReturnValue _3 = dong_porf_hello_dom_setTextContent(0, 0, 0, 0, dong_porf_hello_dom_statusId, dong_porf_hello_dom_statusIdjjtype, 64, 195);
  jjlast_type = _3.type;
  (void) _3.value;
  const struct ReturnValue _4 = dong_porf_hello_dom_dongLog(0, 0, 0, 0, 75, 195);
  jjlast_type = _4.type;
  _get2 = jjlast_type;

  return 0;
}