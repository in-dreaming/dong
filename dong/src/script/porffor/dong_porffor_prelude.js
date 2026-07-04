// Porffor prelude — flat helpers only (no object method calls).
// Multi-arg host calls use single-arg stage + commit imports (Porffor 2c limitation).
// String channel: T16 pull-based host→wasm + UTF-8 toUtf8 for wasm→host.

function utf8AppendCodePoint(out, cp) {
  if (cp < 128) {
    return out + String.fromCharCode(cp);
  }
  if (cp < 2048) {
    return out + String.fromCharCode(192 | (cp >> 6), 128 | (cp & 63));
  }
  if (cp < 65536) {
    return out + String.fromCharCode(224 | (cp >> 12), 128 | ((cp >> 6) & 63), 128 | (cp & 63));
  }
  return out + String.fromCharCode(240 | (cp >> 18), 128 | ((cp >> 12) & 63), 128 | ((cp >> 6) & 63), 128 | (cp & 63));
}

function toUtf8(s) {
  var out = "";
  var i = 0;
  var len = s.length;
  while (i < len) {
    var cp = s.charCodeAt(i);
    if (cp >= 55296 && cp <= 56319 && i + 1 < len) {
      var next = s.charCodeAt(i + 1);
      if (next >= 56320 && next <= 57343) {
        cp = 65536 + ((cp - 55296) << 10) + (next - 56320);
        i = i + 1;
      }
    }
    out = utf8AppendCodePoint(out, cp);
    i = i + 1;
  }
  return out;
}

function toBytes(s) {
  return toUtf8(s);
}

function pullHostString() {
  var len = dong_str_len();
  if (len === 0) {
    return "";
  }
  var out = "";
  var i = 0;
  var b0 = 0;
  var b1 = 0;
  var b2 = 0;
  var b3 = 0;
  var cp = 0;
  while (i < len) {
    b0 = dong_str_byte_at(i);
    if (b0 < 0) {
      break;
    }
    if (b0 < 128) {
      out = out + String.fromCharCode(b0);
      i = i + 1;
    } else if ((b0 & 224) === 192) {
      if (i + 1 >= len) {
        break;
      }
      b1 = dong_str_byte_at(i + 1);
      cp = ((b0 & 31) << 6) | (b1 & 63);
      out = out + String.fromCharCode(cp);
      i = i + 2;
    } else if ((b0 & 240) === 224) {
      if (i + 2 >= len) {
        break;
      }
      b1 = dong_str_byte_at(i + 1);
      b2 = dong_str_byte_at(i + 2);
      cp = ((b0 & 15) << 12) | ((b1 & 63) << 6) | (b2 & 63);
      out = out + String.fromCharCode(cp);
      i = i + 3;
    } else if ((b0 & 248) === 240) {
      if (i + 3 >= len) {
        break;
      }
      b1 = dong_str_byte_at(i + 1);
      b2 = dong_str_byte_at(i + 2);
      b3 = dong_str_byte_at(i + 3);
      cp = ((b0 & 7) << 18) | ((b1 & 63) << 12) | ((b2 & 63) << 6) | (b3 & 63);
      if (cp > 65535) {
        cp = cp - 65536;
        out = out + String.fromCharCode(55296 + (cp >> 10), 56320 + (cp & 1023));
      } else {
        out = out + String.fromCharCode(cp);
      }
      i = i + 4;
    } else {
      i = i + 1;
    }
  }
  return out;
}

function getElementById(id) {
  return dong_dom_getElementById(toUtf8(id));
}

function setTextContent(nodeId, text) {
  dong_stage_0(nodeId);
  dong_stage_1(toUtf8(text));
  dong_commit_set_textContent();
}

function getTextContent(nodeId) {
  dong_dom_get_textContent(nodeId);
  return pullHostString();
}

function addEventListener(nodeId, type, handlerName) {
  dong_stage_0(nodeId);
  dong_stage_1(toUtf8(type));
  dong_stage_2(toUtf8(handlerName));
  dong_commit_addEventListener();
}

function dongLog(msg) {
  dong_print(toUtf8(msg));
}

function benchLog(msg) {
  dong_bench_log(toUtf8(msg));
}

function setTimeout(handlerName, ms) {
  dong_stage_0(ms);
  dong_stage_1(toUtf8(handlerName));
  dong_commit_setTimeout();
}

function dongStateSetNum(slot, v) {
  dong_state_set_num(slot, v);
}

function dongStateGetNum(slot) {
  return dong_state_get_num(slot);
}
