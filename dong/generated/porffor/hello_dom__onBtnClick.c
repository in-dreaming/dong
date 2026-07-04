// Porffor module: hello_dom__onBtnClick
#include "dong_porf_runtime.h"
char* dong_porf_hello_dom__onBtnClick_memory; u32 dong_porf_hello_dom__onBtnClick_memory_pages = 1;


__attribute__((import_module(""), import_name("h")))
extern f64 __porf_import_dong_dom_getElementById(f64);

__attribute__((import_module(""), import_name("m")))
extern void __porf_import_dong_stage_0(f64);

__attribute__((import_module(""), import_name("n")))
extern void __porf_import_dong_stage_1(f64);

__attribute__((import_module(""), import_name("p")))
extern void __porf_import_dong_commit_set_textContent();
static struct ReturnValue dong_porf_hello_dom__onBtnClick_getElementById(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 id, i32 idjjtype);
static struct ReturnValue dong_porf_hello_dom__onBtnClick_setTextContent(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 text, i32 textjjtype);
static i32 dong_porf_hello_dom__onBtnClick___Porffor_object_isObject(i32 arg, i32 argjjtype);

f64 dong_porf_hello_dom__onBtnClick_statusId = 0;
i32 dong_porf_hello_dom__onBtnClick_statusIdjjtype = 0;

static i32 dong_porf_hello_dom__onBtnClick___Porffor_object_isObject(i32 arg, i32 argjjtype) {
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

static struct ReturnValue dong_porf_hello_dom__onBtnClick_getElementById(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 id, i32 idjjtype) {
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
        if ((dong_porf_hello_dom__onBtnClick___Porffor_object_isObject((i32)(_get2), _get3)) == 0) {
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

static struct ReturnValue dong_porf_hello_dom__onBtnClick_setTextContent(f64 jjnewtarget, i32 jjnewtargetjjtype, f64 jjthis, i32 jjthisjjtype, f64 nodeId, i32 nodeIdjjtype, f64 text, i32 textjjtype) {
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
  j2:;
  return (struct ReturnValue){ 0, 0 };
}

int dong_porf_hello_dom__onBtnClick_main() {
  i32 _get1;
  i32 _get0;
  dong_porf_hello_dom__onBtnClick_memory = calloc(1, dong_porf_hello_dom__onBtnClick_memory_pages * 65536);

  dong_porf_hello_dom__onBtnClick_memory[16]=(u8)6;dong_porf_hello_dom__onBtnClick_memory[20]=(u8)115;dong_porf_hello_dom__onBtnClick_memory[21]=(u8)116;dong_porf_hello_dom__onBtnClick_memory[22]=(u8)97;dong_porf_hello_dom__onBtnClick_memory[23]=(u8)116;dong_porf_hello_dom__onBtnClick_memory[24]=(u8)117;dong_porf_hello_dom__onBtnClick_memory[25]=(u8)115;
  dong_porf_hello_dom__onBtnClick_memory[28]=(u8)7;dong_porf_hello_dom__onBtnClick_memory[32]=(u8)99;dong_porf_hello_dom__onBtnClick_memory[33]=(u8)108;dong_porf_hello_dom__onBtnClick_memory[34]=(u8)105;dong_porf_hello_dom__onBtnClick_memory[35]=(u8)99;dong_porf_hello_dom__onBtnClick_memory[36]=(u8)107;dong_porf_hello_dom__onBtnClick_memory[37]=(u8)101;dong_porf_hello_dom__onBtnClick_memory[38]=(u8)100;

  i32 jjlast_type = 0;

  const struct ReturnValue _0 = dong_porf_hello_dom__onBtnClick_getElementById(0, 0, 0, 0, 16, 195);
  jjlast_type = _0.type;
  _get0 = jjlast_type;
  dong_porf_hello_dom__onBtnClick_statusIdjjtype = _get0;
  dong_porf_hello_dom__onBtnClick_statusId = _0.value;
  const struct ReturnValue _1 = dong_porf_hello_dom__onBtnClick_setTextContent(0, 0, 0, 0, dong_porf_hello_dom__onBtnClick_statusId, dong_porf_hello_dom__onBtnClick_statusIdjjtype, 28, 195);
  jjlast_type = _1.type;
  _get1 = jjlast_type;

  return 0;
}