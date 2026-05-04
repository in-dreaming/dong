(() => {
  // node_modules/preact/dist/preact.module.js
  var n;
  var l;
  var u;
  var t;
  var i;
  var r;
  var o;
  var e;
  var f;
  var c;
  var s;
  var a;
  var h;
  var p = {};
  var v = [];
  var y = /acit|ex(?:s|g|n|p|$)|rph|grid|ows|mnc|ntw|ine[ch]|zoo|^ord|itera/i;
  var d = Array.isArray;
  function w(n2, l3) {
    for (var u4 in l3) n2[u4] = l3[u4];
    return n2;
  }
  function g(n2) {
    n2 && n2.parentNode && n2.parentNode.removeChild(n2);
  }
  function _(l3, u4, t3) {
    var i4, r3, o2, e3 = {};
    for (o2 in u4) "key" == o2 ? i4 = u4[o2] : "ref" == o2 ? r3 = u4[o2] : e3[o2] = u4[o2];
    if (arguments.length > 2 && (e3.children = arguments.length > 3 ? n.call(arguments, 2) : t3), "function" == typeof l3 && null != l3.defaultProps) for (o2 in l3.defaultProps) void 0 === e3[o2] && (e3[o2] = l3.defaultProps[o2]);
    return m(l3, e3, i4, r3, null);
  }
  function m(n2, t3, i4, r3, o2) {
    var e3 = { type: n2, props: t3, key: i4, ref: r3, __k: null, __: null, __b: 0, __e: null, __c: null, constructor: void 0, __v: null == o2 ? ++u : o2, __i: -1, __u: 0 };
    return null == o2 && null != l.vnode && l.vnode(e3), e3;
  }
  function b() {
    return { current: null };
  }
  function k(n2) {
    return n2.children;
  }
  function x(n2, l3) {
    this.props = n2, this.context = l3;
  }
  function S(n2, l3) {
    if (null == l3) return n2.__ ? S(n2.__, n2.__i + 1) : null;
    for (var u4; l3 < n2.__k.length; l3++) if (null != (u4 = n2.__k[l3]) && null != u4.__e) return u4.__e;
    return "function" == typeof n2.type ? S(n2) : null;
  }
  function C(n2) {
    if (n2.__P && n2.__d) {
      var u4 = n2.__v, t3 = u4.__e, i4 = [], r3 = [], o2 = w({}, u4);
      o2.__v = u4.__v + 1, l.vnode && l.vnode(o2), z(n2.__P, o2, u4, n2.__n, n2.__P.namespaceURI, 32 & u4.__u ? [t3] : null, i4, null == t3 ? S(u4) : t3, !!(32 & u4.__u), r3), o2.__v = u4.__v, o2.__.__k[o2.__i] = o2, V(i4, o2, r3), u4.__e = u4.__ = null, o2.__e != t3 && M(o2);
    }
  }
  function M(n2) {
    if (null != (n2 = n2.__) && null != n2.__c) return n2.__e = n2.__c.base = null, n2.__k.some(function(l3) {
      if (null != l3 && null != l3.__e) return n2.__e = n2.__c.base = l3.__e;
    }), M(n2);
  }
  function $(n2) {
    (!n2.__d && (n2.__d = true) && i.push(n2) && !I.__r++ || r != l.debounceRendering) && ((r = l.debounceRendering) || o)(I);
  }
  function I() {
    try {
      for (var n2, l3 = 1; i.length; ) i.length > l3 && i.sort(e), n2 = i.shift(), l3 = i.length, C(n2);
    } finally {
      i.length = I.__r = 0;
    }
  }
  function P(n2, l3, u4, t3, i4, r3, o2, e3, f4, c3, s3) {
    var a3, h3, y3, d3, w3, g4, _3, m3 = t3 && t3.__k || v, b2 = l3.length;
    for (f4 = A(u4, l3, m3, f4, b2), a3 = 0; a3 < b2; a3++) null != (y3 = u4.__k[a3]) && (h3 = -1 != y3.__i && m3[y3.__i] || p, y3.__i = a3, g4 = z(n2, y3, h3, i4, r3, o2, e3, f4, c3, s3), d3 = y3.__e, y3.ref && h3.ref != y3.ref && (h3.ref && D(h3.ref, null, y3), s3.push(y3.ref, y3.__c || d3, y3)), null == w3 && null != d3 && (w3 = d3), (_3 = !!(4 & y3.__u)) || h3.__k === y3.__k ? f4 = H(y3, f4, n2, _3) : "function" == typeof y3.type && void 0 !== g4 ? f4 = g4 : d3 && (f4 = d3.nextSibling), y3.__u &= -7);
    return u4.__e = w3, f4;
  }
  function A(n2, l3, u4, t3, i4) {
    var r3, o2, e3, f4, c3, s3 = u4.length, a3 = s3, h3 = 0;
    for (n2.__k = new Array(i4), r3 = 0; r3 < i4; r3++) null != (o2 = l3[r3]) && "boolean" != typeof o2 && "function" != typeof o2 ? ("string" == typeof o2 || "number" == typeof o2 || "bigint" == typeof o2 || o2.constructor == String ? o2 = n2.__k[r3] = m(null, o2, null, null, null) : d(o2) ? o2 = n2.__k[r3] = m(k, { children: o2 }, null, null, null) : void 0 === o2.constructor && o2.__b > 0 ? o2 = n2.__k[r3] = m(o2.type, o2.props, o2.key, o2.ref ? o2.ref : null, o2.__v) : n2.__k[r3] = o2, f4 = r3 + h3, o2.__ = n2, o2.__b = n2.__b + 1, e3 = null, -1 != (c3 = o2.__i = T(o2, u4, f4, a3)) && (a3--, (e3 = u4[c3]) && (e3.__u |= 2)), null == e3 || null == e3.__v ? (-1 == c3 && (i4 > s3 ? h3-- : i4 < s3 && h3++), "function" != typeof o2.type && (o2.__u |= 4)) : c3 != f4 && (c3 == f4 - 1 ? h3-- : c3 == f4 + 1 ? h3++ : (c3 > f4 ? h3-- : h3++, o2.__u |= 4))) : n2.__k[r3] = null;
    if (a3) for (r3 = 0; r3 < s3; r3++) null != (e3 = u4[r3]) && 0 == (2 & e3.__u) && (e3.__e == t3 && (t3 = S(e3)), E(e3, e3));
    return t3;
  }
  function H(n2, l3, u4, t3) {
    var i4, r3;
    if ("function" == typeof n2.type) {
      for (i4 = n2.__k, r3 = 0; i4 && r3 < i4.length; r3++) i4[r3] && (i4[r3].__ = n2, l3 = H(i4[r3], l3, u4, t3));
      return l3;
    }
    n2.__e != l3 && (t3 && (l3 && n2.type && !l3.parentNode && (l3 = S(n2)), u4.insertBefore(n2.__e, l3 || null)), l3 = n2.__e);
    do {
      l3 = l3 && l3.nextSibling;
    } while (null != l3 && 8 == l3.nodeType);
    return l3;
  }
  function L(n2, l3) {
    return l3 = l3 || [], null == n2 || "boolean" == typeof n2 || (d(n2) ? n2.some(function(n3) {
      L(n3, l3);
    }) : l3.push(n2)), l3;
  }
  function T(n2, l3, u4, t3) {
    var i4, r3, o2, e3 = n2.key, f4 = n2.type, c3 = l3[u4], s3 = null != c3 && 0 == (2 & c3.__u);
    if (null === c3 && null == e3 || s3 && e3 == c3.key && f4 == c3.type) return u4;
    if (t3 > (s3 ? 1 : 0)) {
      for (i4 = u4 - 1, r3 = u4 + 1; i4 >= 0 || r3 < l3.length; ) if (null != (c3 = l3[o2 = i4 >= 0 ? i4-- : r3++]) && 0 == (2 & c3.__u) && e3 == c3.key && f4 == c3.type) return o2;
    }
    return -1;
  }
  function j(n2, l3, u4) {
    "-" == l3[0] ? n2.setProperty(l3, null == u4 ? "" : u4) : n2[l3] = null == u4 ? "" : "number" != typeof u4 || y.test(l3) ? u4 : u4 + "px";
  }
  function F(n2, l3, u4, t3, i4) {
    var r3, o2;
    n: if ("style" == l3) if ("string" == typeof u4) n2.style.cssText = u4;
    else {
      if ("string" == typeof t3 && (n2.style.cssText = t3 = ""), t3) for (l3 in t3) u4 && l3 in u4 || j(n2.style, l3, "");
      if (u4) for (l3 in u4) t3 && u4[l3] == t3[l3] || j(n2.style, l3, u4[l3]);
    }
    else if ("o" == l3[0] && "n" == l3[1]) r3 = l3 != (l3 = l3.replace(f, "$1")), o2 = l3.toLowerCase(), l3 = o2 in n2 || "onFocusOut" == l3 || "onFocusIn" == l3 ? o2.slice(2) : l3.slice(2), n2.l || (n2.l = {}), n2.l[l3 + r3] = u4, u4 ? t3 ? u4.u = t3.u : (u4.u = c, n2.addEventListener(l3, r3 ? a : s, r3)) : n2.removeEventListener(l3, r3 ? a : s, r3);
    else {
      if ("http://www.w3.org/2000/svg" == i4) l3 = l3.replace(/xlink(H|:h)/, "h").replace(/sName$/, "s");
      else if ("width" != l3 && "height" != l3 && "href" != l3 && "list" != l3 && "form" != l3 && "tabIndex" != l3 && "download" != l3 && "rowSpan" != l3 && "colSpan" != l3 && "role" != l3 && "popover" != l3 && l3 in n2) try {
        n2[l3] = null == u4 ? "" : u4;
        break n;
      } catch (n3) {
      }
      "function" == typeof u4 || (null == u4 || false === u4 && "-" != l3[4] ? n2.removeAttribute(l3) : n2.setAttribute(l3, "popover" == l3 && 1 == u4 ? "" : u4));
    }
  }
  function O(n2) {
    return function(u4) {
      if (this.l) {
        var t3 = this.l[u4.type + n2];
        if (null == u4.t) u4.t = c++;
        else if (u4.t < t3.u) return;
        return t3(l.event ? l.event(u4) : u4);
      }
    };
  }
  function z(n2, u4, t3, i4, r3, o2, e3, f4, c3, s3) {
    var a3, h3, p2, y3, _3, m3, b2, S2, C2, M3, $2, I2, A4, H3, L2, T4 = u4.type;
    if (void 0 !== u4.constructor) return null;
    128 & t3.__u && (c3 = !!(32 & t3.__u), o2 = [f4 = u4.__e = t3.__e]), (a3 = l.__b) && a3(u4);
    n: if ("function" == typeof T4) try {
      if (S2 = u4.props, C2 = T4.prototype && T4.prototype.render, M3 = (a3 = T4.contextType) && i4[a3.__c], $2 = a3 ? M3 ? M3.props.value : a3.__ : i4, t3.__c ? b2 = (h3 = u4.__c = t3.__c).__ = h3.__E : (C2 ? u4.__c = h3 = new T4(S2, $2) : (u4.__c = h3 = new x(S2, $2), h3.constructor = T4, h3.render = G), M3 && M3.sub(h3), h3.state || (h3.state = {}), h3.__n = i4, p2 = h3.__d = true, h3.__h = [], h3._sb = []), C2 && null == h3.__s && (h3.__s = h3.state), C2 && null != T4.getDerivedStateFromProps && (h3.__s == h3.state && (h3.__s = w({}, h3.__s)), w(h3.__s, T4.getDerivedStateFromProps(S2, h3.__s))), y3 = h3.props, _3 = h3.state, h3.__v = u4, p2) C2 && null == T4.getDerivedStateFromProps && null != h3.componentWillMount && h3.componentWillMount(), C2 && null != h3.componentDidMount && h3.__h.push(h3.componentDidMount);
      else {
        if (C2 && null == T4.getDerivedStateFromProps && S2 !== y3 && null != h3.componentWillReceiveProps && h3.componentWillReceiveProps(S2, $2), u4.__v == t3.__v || !h3.__e && null != h3.shouldComponentUpdate && false === h3.shouldComponentUpdate(S2, h3.__s, $2)) {
          u4.__v != t3.__v && (h3.props = S2, h3.state = h3.__s, h3.__d = false), u4.__e = t3.__e, u4.__k = t3.__k, u4.__k.some(function(n3) {
            n3 && (n3.__ = u4);
          }), v.push.apply(h3.__h, h3._sb), h3._sb = [], h3.__h.length && e3.push(h3);
          break n;
        }
        null != h3.componentWillUpdate && h3.componentWillUpdate(S2, h3.__s, $2), C2 && null != h3.componentDidUpdate && h3.__h.push(function() {
          h3.componentDidUpdate(y3, _3, m3);
        });
      }
      if (h3.context = $2, h3.props = S2, h3.__P = n2, h3.__e = false, I2 = l.__r, A4 = 0, C2) h3.state = h3.__s, h3.__d = false, I2 && I2(u4), a3 = h3.render(h3.props, h3.state, h3.context), v.push.apply(h3.__h, h3._sb), h3._sb = [];
      else do {
        h3.__d = false, I2 && I2(u4), a3 = h3.render(h3.props, h3.state, h3.context), h3.state = h3.__s;
      } while (h3.__d && ++A4 < 25);
      h3.state = h3.__s, null != h3.getChildContext && (i4 = w(w({}, i4), h3.getChildContext())), C2 && !p2 && null != h3.getSnapshotBeforeUpdate && (m3 = h3.getSnapshotBeforeUpdate(y3, _3)), H3 = null != a3 && a3.type === k && null == a3.key ? q(a3.props.children) : a3, f4 = P(n2, d(H3) ? H3 : [H3], u4, t3, i4, r3, o2, e3, f4, c3, s3), h3.base = u4.__e, u4.__u &= -161, h3.__h.length && e3.push(h3), b2 && (h3.__E = h3.__ = null);
    } catch (n3) {
      if (u4.__v = null, c3 || null != o2) if (n3.then) {
        for (u4.__u |= c3 ? 160 : 128; f4 && 8 == f4.nodeType && f4.nextSibling; ) f4 = f4.nextSibling;
        o2[o2.indexOf(f4)] = null, u4.__e = f4;
      } else {
        for (L2 = o2.length; L2--; ) g(o2[L2]);
        N(u4);
      }
      else u4.__e = t3.__e, u4.__k = t3.__k, n3.then || N(u4);
      l.__e(n3, u4, t3);
    }
    else null == o2 && u4.__v == t3.__v ? (u4.__k = t3.__k, u4.__e = t3.__e) : f4 = u4.__e = B(t3.__e, u4, t3, i4, r3, o2, e3, c3, s3);
    return (a3 = l.diffed) && a3(u4), 128 & u4.__u ? void 0 : f4;
  }
  function N(n2) {
    n2 && (n2.__c && (n2.__c.__e = true), n2.__k && n2.__k.some(N));
  }
  function V(n2, u4, t3) {
    for (var i4 = 0; i4 < t3.length; i4++) D(t3[i4], t3[++i4], t3[++i4]);
    l.__c && l.__c(u4, n2), n2.some(function(u5) {
      try {
        n2 = u5.__h, u5.__h = [], n2.some(function(n3) {
          n3.call(u5);
        });
      } catch (n3) {
        l.__e(n3, u5.__v);
      }
    });
  }
  function q(n2) {
    return "object" != typeof n2 || null == n2 || n2.__b > 0 ? n2 : d(n2) ? n2.map(q) : w({}, n2);
  }
  function B(u4, t3, i4, r3, o2, e3, f4, c3, s3) {
    var a3, h3, v3, y3, w3, _3, m3, b2 = i4.props || p, k3 = t3.props, x3 = t3.type;
    if ("svg" == x3 ? o2 = "http://www.w3.org/2000/svg" : "math" == x3 ? o2 = "http://www.w3.org/1998/Math/MathML" : o2 || (o2 = "http://www.w3.org/1999/xhtml"), null != e3) {
      for (a3 = 0; a3 < e3.length; a3++) if ((w3 = e3[a3]) && "setAttribute" in w3 == !!x3 && (x3 ? w3.localName == x3 : 3 == w3.nodeType)) {
        u4 = w3, e3[a3] = null;
        break;
      }
    }
    if (null == u4) {
      if (null == x3) return document.createTextNode(k3);
      u4 = document.createElementNS(o2, x3, k3.is && k3), c3 && (l.__m && l.__m(t3, e3), c3 = false), e3 = null;
    }
    if (null == x3) b2 === k3 || c3 && u4.data == k3 || (u4.data = k3);
    else {
      if (e3 = e3 && n.call(u4.childNodes), !c3 && null != e3) for (b2 = {}, a3 = 0; a3 < u4.attributes.length; a3++) b2[(w3 = u4.attributes[a3]).name] = w3.value;
      for (a3 in b2) w3 = b2[a3], "dangerouslySetInnerHTML" == a3 ? v3 = w3 : "children" == a3 || a3 in k3 || "value" == a3 && "defaultValue" in k3 || "checked" == a3 && "defaultChecked" in k3 || F(u4, a3, null, w3, o2);
      for (a3 in k3) w3 = k3[a3], "children" == a3 ? y3 = w3 : "dangerouslySetInnerHTML" == a3 ? h3 = w3 : "value" == a3 ? _3 = w3 : "checked" == a3 ? m3 = w3 : c3 && "function" != typeof w3 || b2[a3] === w3 || F(u4, a3, w3, b2[a3], o2);
      if (h3) c3 || v3 && (h3.__html == v3.__html || h3.__html == u4.innerHTML) || (u4.innerHTML = h3.__html), t3.__k = [];
      else if (v3 && (u4.innerHTML = ""), P("template" == t3.type ? u4.content : u4, d(y3) ? y3 : [y3], t3, i4, r3, "foreignObject" == x3 ? "http://www.w3.org/1999/xhtml" : o2, e3, f4, e3 ? e3[0] : i4.__k && S(i4, 0), c3, s3), null != e3) for (a3 = e3.length; a3--; ) g(e3[a3]);
      c3 || (a3 = "value", "progress" == x3 && null == _3 ? u4.removeAttribute("value") : null != _3 && (_3 !== u4[a3] || "progress" == x3 && !_3 || "option" == x3 && _3 != b2[a3]) && F(u4, a3, _3, b2[a3], o2), a3 = "checked", null != m3 && m3 != u4[a3] && F(u4, a3, m3, b2[a3], o2));
    }
    return u4;
  }
  function D(n2, u4, t3) {
    try {
      if ("function" == typeof n2) {
        var i4 = "function" == typeof n2.__u;
        i4 && n2.__u(), i4 && null == u4 || (n2.__u = n2(u4));
      } else n2.current = u4;
    } catch (n3) {
      l.__e(n3, t3);
    }
  }
  function E(n2, u4, t3) {
    var i4, r3;
    if (l.unmount && l.unmount(n2), (i4 = n2.ref) && (i4.current && i4.current != n2.__e || D(i4, null, u4)), null != (i4 = n2.__c)) {
      if (i4.componentWillUnmount) try {
        i4.componentWillUnmount();
      } catch (n3) {
        l.__e(n3, u4);
      }
      i4.base = i4.__P = null;
    }
    if (i4 = n2.__k) for (r3 = 0; r3 < i4.length; r3++) i4[r3] && E(i4[r3], u4, t3 || "function" != typeof n2.type);
    t3 || g(n2.__e), n2.__c = n2.__ = n2.__e = void 0;
  }
  function G(n2, l3, u4) {
    return this.constructor(n2, u4);
  }
  function J(u4, t3, i4) {
    var r3, o2, e3, f4;
    t3 == document && (t3 = document.documentElement), l.__ && l.__(u4, t3), o2 = (r3 = "function" == typeof i4) ? null : i4 && i4.__k || t3.__k, e3 = [], f4 = [], z(t3, u4 = (!r3 && i4 || t3).__k = _(k, null, [u4]), o2 || p, p, t3.namespaceURI, !r3 && i4 ? [i4] : o2 ? null : t3.firstChild ? n.call(t3.childNodes) : null, e3, !r3 && i4 ? i4 : o2 ? o2.__e : t3.firstChild, r3, f4), V(e3, u4, f4);
  }
  n = v.slice, l = { __e: function(n2, l3, u4, t3) {
    for (var i4, r3, o2; l3 = l3.__; ) if ((i4 = l3.__c) && !i4.__) try {
      if ((r3 = i4.constructor) && null != r3.getDerivedStateFromError && (i4.setState(r3.getDerivedStateFromError(n2)), o2 = i4.__d), null != i4.componentDidCatch && (i4.componentDidCatch(n2, t3 || {}), o2 = i4.__d), o2) return i4.__E = i4;
    } catch (l4) {
      n2 = l4;
    }
    throw n2;
  } }, u = 0, t = function(n2) {
    return null != n2 && void 0 === n2.constructor;
  }, x.prototype.setState = function(n2, l3) {
    var u4;
    u4 = null != this.__s && this.__s != this.state ? this.__s : this.__s = w({}, this.state), "function" == typeof n2 && (n2 = n2(w({}, u4), this.props)), n2 && w(u4, n2), null != n2 && this.__v && (l3 && this._sb.push(l3), $(this));
  }, x.prototype.forceUpdate = function(n2) {
    this.__v && (this.__e = true, n2 && this.__h.push(n2), $(this));
  }, x.prototype.render = k, i = [], o = "function" == typeof Promise ? Promise.prototype.then.bind(Promise.resolve()) : setTimeout, e = function(n2, l3) {
    return n2.__v.__b - l3.__v.__b;
  }, I.__r = 0, f = /(PointerCapture)$|Capture$/i, c = 0, s = O(false), a = O(true), h = 0;

  // node_modules/preact/hooks/dist/hooks.module.js
  var t2;
  var r2;
  var u2;
  var i2;
  var f2 = [];
  var c2 = l;
  var e2 = c2.__b;
  var a2 = c2.__r;
  var v2 = c2.diffed;
  var l2 = c2.__c;
  var m2 = c2.unmount;
  var s2 = c2.__;
  function j2() {
    for (var n2; n2 = f2.shift(); ) {
      var t3 = n2.__H;
      if (n2.__P && t3) try {
        t3.__h.some(z2), t3.__h.some(B2), t3.__h = [];
      } catch (r3) {
        t3.__h = [], c2.__e(r3, n2.__v);
      }
    }
  }
  c2.__b = function(n2) {
    r2 = null, e2 && e2(n2);
  }, c2.__ = function(n2, t3) {
    n2 && t3.__k && t3.__k.__m && (n2.__m = t3.__k.__m), s2 && s2(n2, t3);
  }, c2.__r = function(n2) {
    a2 && a2(n2), t2 = 0;
    var i4 = (r2 = n2.__c).__H;
    i4 && (u2 === r2 ? (i4.__h = [], r2.__h = [], i4.__.some(function(n3) {
      n3.__N && (n3.__ = n3.__N), n3.u = n3.__N = void 0;
    })) : (i4.__h.some(z2), i4.__h.some(B2), i4.__h = [], t2 = 0)), u2 = r2;
  }, c2.diffed = function(n2) {
    v2 && v2(n2);
    var t3 = n2.__c;
    t3 && t3.__H && (t3.__H.__h.length && (1 !== f2.push(t3) && i2 === c2.requestAnimationFrame || ((i2 = c2.requestAnimationFrame) || w2)(j2)), t3.__H.__.some(function(n3) {
      n3.u && (n3.__H = n3.u), n3.u = void 0;
    })), u2 = r2 = null;
  }, c2.__c = function(n2, t3) {
    t3.some(function(n3) {
      try {
        n3.__h.some(z2), n3.__h = n3.__h.filter(function(n4) {
          return !n4.__ || B2(n4);
        });
      } catch (r3) {
        t3.some(function(n4) {
          n4.__h && (n4.__h = []);
        }), t3 = [], c2.__e(r3, n3.__v);
      }
    }), l2 && l2(n2, t3);
  }, c2.unmount = function(n2) {
    m2 && m2(n2);
    var t3, r3 = n2.__c;
    r3 && r3.__H && (r3.__H.__.some(function(n3) {
      try {
        z2(n3);
      } catch (n4) {
        t3 = n4;
      }
    }), r3.__H = void 0, t3 && c2.__e(t3, r3.__v));
  };
  var k2 = "function" == typeof requestAnimationFrame;
  function w2(n2) {
    var t3, r3 = function() {
      clearTimeout(u4), k2 && cancelAnimationFrame(t3), setTimeout(n2);
    }, u4 = setTimeout(r3, 35);
    k2 && (t3 = requestAnimationFrame(r3));
  }
  function z2(n2) {
    var t3 = r2, u4 = n2.__c;
    "function" == typeof u4 && (n2.__c = void 0, u4()), r2 = t3;
  }
  function B2(n2) {
    var t3 = r2;
    n2.__c = n2.__(), r2 = t3;
  }

  // node_modules/preact/compat/dist/compat.module.js
  function g3(n2, t3) {
    for (var e3 in t3) n2[e3] = t3[e3];
    return n2;
  }
  function E2(n2, t3) {
    for (var e3 in n2) if ("__source" !== e3 && !(e3 in t3)) return true;
    for (var r3 in t3) if ("__source" !== r3 && n2[r3] !== t3[r3]) return true;
    return false;
  }
  function M2(n2, t3) {
    this.props = n2, this.context = t3;
  }
  (M2.prototype = new x()).isPureReactComponent = true, M2.prototype.shouldComponentUpdate = function(n2, t3) {
    return E2(this.props, n2) || E2(this.state, t3);
  };
  var T3 = l.__b;
  l.__b = function(n2) {
    n2.type && n2.type.__f && n2.ref && (n2.props.ref = n2.ref, n2.ref = null), T3 && T3(n2);
  };
  var A3 = "undefined" != typeof Symbol && Symbol.for && Symbol.for("react.forward_ref") || 3911;
  var O2 = l.__e;
  l.__e = function(n2, t3, e3, r3) {
    if (n2.then) {
      for (var u4, o2 = t3; o2 = o2.__; ) if ((u4 = o2.__c) && u4.__c) return null == t3.__e && (t3.__e = e3.__e, t3.__k = e3.__k), u4.__c(n2, t3);
    }
    O2(n2, t3, e3, r3);
  };
  var U = l.unmount;
  function V2(n2, t3, e3) {
    return n2 && (n2.__c && n2.__c.__H && (n2.__c.__H.__.forEach(function(n3) {
      "function" == typeof n3.__c && n3.__c();
    }), n2.__c.__H = null), null != (n2 = g3({}, n2)).__c && (n2.__c.__P === e3 && (n2.__c.__P = t3), n2.__c.__e = true, n2.__c = null), n2.__k = n2.__k && n2.__k.map(function(n3) {
      return V2(n3, t3, e3);
    })), n2;
  }
  function W(n2, t3, e3) {
    return n2 && e3 && (n2.__v = null, n2.__k = n2.__k && n2.__k.map(function(n3) {
      return W(n3, t3, e3);
    }), n2.__c && n2.__c.__P === t3 && (n2.__e && e3.appendChild(n2.__e), n2.__c.__e = true, n2.__c.__P = e3)), n2;
  }
  function P3() {
    this.__u = 0, this.o = null, this.__b = null;
  }
  function j3(n2) {
    var t3 = n2.__ && n2.__.__c;
    return t3 && t3.__a && t3.__a(n2);
  }
  function B3() {
    this.i = null, this.l = null;
  }
  l.unmount = function(n2) {
    var t3 = n2.__c;
    t3 && (t3.__z = true), t3 && t3.__R && t3.__R(), t3 && 32 & n2.__u && (n2.type = null), U && U(n2);
  }, (P3.prototype = new x()).__c = function(n2, t3) {
    var e3 = t3.__c, r3 = this;
    null == r3.o && (r3.o = []), r3.o.push(e3);
    var u4 = j3(r3.__v), o2 = false, i4 = function() {
      o2 || r3.__z || (o2 = true, e3.__R = null, u4 ? u4(c3) : c3());
    };
    e3.__R = i4;
    var l3 = e3.__P;
    e3.__P = null;
    var c3 = function() {
      if (!--r3.__u) {
        if (r3.state.__a) {
          var n3 = r3.state.__a;
          r3.__v.__k[0] = W(n3, n3.__c.__P, n3.__c.__O);
        }
        var t4;
        for (r3.setState({ __a: r3.__b = null }); t4 = r3.o.pop(); ) t4.__P = l3, t4.forceUpdate();
      }
    };
    r3.__u++ || 32 & t3.__u || r3.setState({ __a: r3.__b = r3.__v.__k[0] }), n2.then(i4, i4);
  }, P3.prototype.componentWillUnmount = function() {
    this.o = [];
  }, P3.prototype.render = function(n2, e3) {
    if (this.__b) {
      if (this.__v.__k) {
        var r3 = document.createElement("div"), o2 = this.__v.__k[0].__c;
        this.__v.__k[0] = V2(this.__b, r3, o2.__O = o2.__P);
      }
      this.__b = null;
    }
    var i4 = e3.__a && _(k, null, n2.fallback);
    return i4 && (i4.__u &= -33), [_(k, null, e3.__a ? null : n2.children), i4];
  };
  var H2 = function(n2, t3, e3) {
    if (++e3[1] === e3[0] && n2.l.delete(t3), n2.props.revealOrder && ("t" !== n2.props.revealOrder[0] || !n2.l.size)) for (e3 = n2.i; e3; ) {
      for (; e3.length > 3; ) e3.pop()();
      if (e3[1] < e3[0]) break;
      n2.i = e3 = e3[2];
    }
  };
  (B3.prototype = new x()).__a = function(n2) {
    var t3 = this, e3 = j3(t3.__v), r3 = t3.l.get(n2);
    return r3[0]++, function(u4) {
      var o2 = function() {
        t3.props.revealOrder ? (r3.push(u4), H2(t3, n2, r3)) : u4();
      };
      e3 ? e3(o2) : o2();
    };
  }, B3.prototype.render = function(n2) {
    this.i = null, this.l = /* @__PURE__ */ new Map();
    var t3 = L(n2.children);
    n2.revealOrder && "b" === n2.revealOrder[0] && t3.reverse();
    for (var e3 = t3.length; e3--; ) this.l.set(t3[e3], this.i = [1, 0, this.i]);
    return n2.children;
  }, B3.prototype.componentDidUpdate = B3.prototype.componentDidMount = function() {
    var n2 = this;
    this.l.forEach(function(t3, e3) {
      H2(n2, e3, t3);
    });
  };
  var q3 = "undefined" != typeof Symbol && Symbol.for && Symbol.for("react.element") || 60103;
  var G2 = /^(?:accent|alignment|arabic|baseline|cap|clip(?!PathU)|color|dominant|fill|flood|font|glyph(?!R)|horiz|image(!S)|letter|lighting|marker(?!H|W|U)|overline|paint|pointer|shape|stop|strikethrough|stroke|text(?!L)|transform|underline|unicode|units|v|vector|vert|word|writing|x(?!C))[A-Z]/;
  var J2 = /^on(Ani|Tra|Tou|BeforeInp|Compo)/;
  var K2 = /[A-Z0-9]/g;
  var Q2 = "undefined" != typeof document;
  var X = function(n2) {
    return ("undefined" != typeof Symbol && "symbol" == typeof Symbol() ? /fil|che|rad/ : /fil|che|ra/).test(n2);
  };
  function nn(n2, t3, e3) {
    return null == t3.__k && (t3.textContent = ""), J(n2, t3), "function" == typeof e3 && e3(), n2 ? n2.__c : null;
  }
  x.prototype.isReactComponent = true, ["componentWillMount", "componentWillReceiveProps", "componentWillUpdate"].forEach(function(t3) {
    Object.defineProperty(x.prototype, t3, { configurable: true, get: function() {
      return this["UNSAFE_" + t3];
    }, set: function(n2) {
      Object.defineProperty(this, t3, { configurable: true, writable: true, value: n2 });
    } });
  });
  var en = l.event;
  l.event = function(n2) {
    return en && (n2 = en(n2)), n2.persist = function() {
    }, n2.isPropagationStopped = function() {
      return this.cancelBubble;
    }, n2.isDefaultPrevented = function() {
      return this.defaultPrevented;
    }, n2.nativeEvent = n2;
  };
  var rn;
  var un = { configurable: true, get: function() {
    return this.class;
  } };
  var on = l.vnode;
  l.vnode = function(n2) {
    "string" == typeof n2.type && function(n3) {
      var t3 = n3.props, e3 = n3.type, u4 = {}, o2 = -1 == e3.indexOf("-");
      for (var i4 in t3) {
        var l3 = t3[i4];
        if (!("value" === i4 && "defaultValue" in t3 && null == l3 || Q2 && "children" === i4 && "noscript" === e3 || "class" === i4 || "className" === i4)) {
          var c3 = i4.toLowerCase();
          "defaultValue" === i4 && "value" in t3 && null == t3.value ? i4 = "value" : "download" === i4 && true === l3 ? l3 = "" : "translate" === c3 && "no" === l3 ? l3 = false : "o" === c3[0] && "n" === c3[1] ? "ondoubleclick" === c3 ? i4 = "ondblclick" : "onchange" !== c3 || "input" !== e3 && "textarea" !== e3 || X(t3.type) ? "onfocus" === c3 ? i4 = "onfocusin" : "onblur" === c3 ? i4 = "onfocusout" : J2.test(i4) && (i4 = c3) : c3 = i4 = "oninput" : o2 && G2.test(i4) ? i4 = i4.replace(K2, "-$&").toLowerCase() : null === l3 && (l3 = void 0), "oninput" === c3 && u4[i4 = c3] && (i4 = "oninputCapture"), u4[i4] = l3;
        }
      }
      "select" == e3 && (u4.multiple && Array.isArray(u4.value) && (u4.value = L(t3.children).forEach(function(n4) {
        n4.props.selected = -1 != u4.value.indexOf(n4.props.value);
      })), null != u4.defaultValue && (u4.value = L(t3.children).forEach(function(n4) {
        n4.props.selected = u4.multiple ? -1 != u4.defaultValue.indexOf(n4.props.value) : u4.defaultValue == n4.props.value;
      }))), t3.class && !t3.className ? (u4.class = t3.class, Object.defineProperty(u4, "className", un)) : t3.className && (u4.class = u4.className = t3.className), n3.props = u4;
    }(n2), n2.$$typeof = q3, on && on(n2);
  };
  var ln = l.__r;
  l.__r = function(n2) {
    ln && ln(n2), rn = n2.__c;
  };
  var cn = l.diffed;
  l.diffed = function(n2) {
    cn && cn(n2);
    var t3 = n2.props, e3 = n2.__e;
    null != e3 && "textarea" === n2.type && "value" in t3 && t3.value !== e3.value && (e3.value = null == t3.value ? "" : t3.value), rn = null;
  };

  // src/polyfills.js
  if (typeof globalThis.requestAnimationFrame === "undefined") {
    globalThis.requestAnimationFrame = function(cb) {
      return setTimeout(function() {
        cb(performance.now());
      }, 16);
    };
    globalThis.cancelAnimationFrame = function(id) {
      clearTimeout(id);
    };
  }
  if (typeof globalThis.queueMicrotask === "undefined") {
    globalThis.queueMicrotask = function(fn) {
      Promise.resolve().then(fn);
    };
  }

  // node_modules/preact/jsx-runtime/dist/jsxRuntime.module.js
  var f3 = 0;
  var i3 = Array.isArray;
  function u3(e3, t3, n2, o2, i4, u4) {
    t3 || (t3 = {});
    var a3, c3, p2 = t3;
    if ("ref" in p2) for (c3 in p2 = {}, t3) "ref" == c3 ? a3 = t3[c3] : p2[c3] = t3[c3];
    var l3 = { type: e3, props: p2, key: n2, ref: a3, __k: null, __: null, __b: 0, __e: null, __c: null, constructor: void 0, __v: --f3, __i: -1, __u: 0, __source: i4, __self: u4 };
    if ("function" == typeof e3 && (a3 = e3.defaultProps)) for (c3 in a3) void 0 === p2[c3] && (p2[c3] = a3[c3]);
    return l.vnode && l.vnode(l3), l3;
  }

  // examples/todo-classic/main.jsx
  var TodoItem = class extends x {
    constructor(props) {
      super(props);
      this.handleToggle = this.handleToggle.bind(this);
      this.handleDelete = this.handleDelete.bind(this);
    }
    shouldComponentUpdate(nextProps) {
      return nextProps.todo !== this.props.todo;
    }
    handleToggle() {
      this.props.onToggle(this.props.todo.id);
    }
    handleDelete() {
      this.props.onDelete(this.props.todo.id);
    }
    render() {
      const { todo } = this.props;
      return /* @__PURE__ */ u3("div", { style: {
        display: "flex",
        alignItems: "center",
        padding: "12px 16px",
        backgroundColor: "#fff",
        borderRadius: "8px",
        marginBottom: "8px",
        boxShadow: "0 1px 3px rgba(0,0,0,0.1)"
      }, children: [
        /* @__PURE__ */ u3(
          "div",
          {
            onClick: this.handleToggle,
            style: {
              width: "24px",
              height: "24px",
              borderRadius: "50%",
              border: todo.done ? "none" : "2px solid #bdc3c7",
              backgroundColor: todo.done ? "#27ae60" : "transparent",
              marginRight: "12px",
              cursor: "pointer",
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
              flexShrink: "0"
            },
            children: todo.done && /* @__PURE__ */ u3("span", { style: { color: "#fff", fontSize: "14px", fontWeight: "bold" }, children: "V" })
          }
        ),
        /* @__PURE__ */ u3("span", { style: {
          flex: "1",
          fontSize: "16px",
          color: todo.done ? "#95a5a6" : "#2c3e50",
          textDecoration: todo.done ? "line-through" : "none"
        }, children: todo.text }),
        /* @__PURE__ */ u3(
          "button",
          {
            onClick: this.handleDelete,
            style: {
              backgroundColor: "transparent",
              border: "none",
              color: "#e74c3c",
              fontSize: "18px",
              cursor: "pointer",
              padding: "4px 8px",
              borderRadius: "4px"
            },
            children: "X"
          }
        )
      ] });
    }
  };
  var FilterBar = class extends x {
    render() {
      const { filter, onFilterChange, counts } = this.props;
      const filters = [
        { key: "all", label: "All (" + counts.total + ")" },
        { key: "active", label: "Active (" + counts.active + ")" },
        { key: "done", label: "Done (" + counts.done + ")" }
      ];
      return /* @__PURE__ */ u3("div", { style: {
        display: "flex",
        gap: "8px",
        marginBottom: "16px"
      }, children: filters.map((f4) => /* @__PURE__ */ u3(
        "button",
        {
          onClick: () => onFilterChange(f4.key),
          style: {
            padding: "8px 16px",
            backgroundColor: filter === f4.key ? "#3498db" : "#ecf0f1",
            color: filter === f4.key ? "#fff" : "#7f8c8d",
            border: "none",
            borderRadius: "6px",
            fontSize: "13px",
            cursor: "pointer",
            fontWeight: filter === f4.key ? "bold" : "normal"
          },
          children: f4.label
        },
        f4.key
      )) });
    }
  };
  var TodoApp = class extends x {
    constructor(props) {
      super(props);
      this.state = {
        todos: [
          { id: 1, text: "Learn Preact class components", done: true },
          { id: 2, text: "Compare bundle size with React", done: true },
          { id: 3, text: "Integrate Preact with Dong engine", done: false },
          { id: 4, text: "Ship the game UI", done: false }
        ],
        filter: "all",
        inputText: "",
        nextId: 5
      };
      this.inputRef = b();
      this.handleAdd = this.handleAdd.bind(this);
      this.handleToggle = this.handleToggle.bind(this);
      this.handleDelete = this.handleDelete.bind(this);
      this.handleFilterChange = this.handleFilterChange.bind(this);
      this.handleClearDone = this.handleClearDone.bind(this);
      this.handleInputChange = this.handleInputChange.bind(this);
      this.handleKeyDown = this.handleKeyDown.bind(this);
    }
    componentDidMount() {
      console.log("[TodoApp] mounted, " + this.state.todos.length + " items");
    }
    componentDidUpdate(prevProps, prevState) {
      if (prevState.todos.length !== this.state.todos.length) {
        console.log("[TodoApp] todo count changed: " + this.state.todos.length);
      }
    }
    handleInputChange(e3) {
      this.setState({ inputText: e3.target.value });
    }
    handleKeyDown(e3) {
      if (e3.key === "Enter") {
        this.handleAdd();
      }
    }
    handleAdd() {
      const text = this.state.inputText.trim();
      if (!text) return;
      this.setState((prev) => ({
        todos: [...prev.todos, { id: prev.nextId, text, done: false }],
        nextId: prev.nextId + 1,
        inputText: ""
      }));
    }
    handleToggle(id) {
      this.setState((prev) => ({
        todos: prev.todos.map(
          (t3) => t3.id === id ? { ...t3, done: !t3.done } : t3
        )
      }));
    }
    handleDelete(id) {
      this.setState((prev) => ({
        todos: prev.todos.filter((t3) => t3.id !== id)
      }));
    }
    handleFilterChange(filter) {
      this.setState({ filter });
    }
    handleClearDone() {
      this.setState((prev) => ({
        todos: prev.todos.filter((t3) => !t3.done)
      }));
    }
    getFilteredTodos() {
      const { todos, filter } = this.state;
      if (filter === "active") return todos.filter((t3) => !t3.done);
      if (filter === "done") return todos.filter((t3) => t3.done);
      return todos;
    }
    render() {
      const filtered = this.getFilteredTodos();
      const { todos, filter, inputText } = this.state;
      const counts = {
        total: todos.length,
        active: todos.filter((t3) => !t3.done).length,
        done: todos.filter((t3) => t3.done).length
      };
      return /* @__PURE__ */ u3("div", { style: {
        width: "100%",
        height: "100%",
        display: "flex",
        alignItems: "flex-start",
        justifyContent: "center",
        backgroundColor: "#f0f3f5",
        fontFamily: "sans-serif",
        paddingTop: "40px"
      }, children: /* @__PURE__ */ u3("div", { style: {
        width: "480px",
        padding: "24px"
      }, children: [
        /* @__PURE__ */ u3("h1", { style: {
          fontSize: "28px",
          color: "#2c3e50",
          marginBottom: "24px",
          textAlign: "center"
        }, children: "Preact Class Todo" }),
        /* @__PURE__ */ u3("div", { style: {
          display: "flex",
          gap: "8px",
          marginBottom: "20px"
        }, children: [
          /* @__PURE__ */ u3(
            "input",
            {
              ref: this.inputRef,
              value: inputText,
              onInput: this.handleInputChange,
              onKeyDown: this.handleKeyDown,
              placeholder: "Add a new task...",
              style: {
                flex: "1",
                padding: "12px 16px",
                fontSize: "15px",
                border: "2px solid #dcdde1",
                borderRadius: "8px",
                backgroundColor: "#fff"
              }
            }
          ),
          /* @__PURE__ */ u3(
            "button",
            {
              onClick: this.handleAdd,
              style: {
                padding: "12px 20px",
                backgroundColor: "#3498db",
                color: "#fff",
                border: "none",
                borderRadius: "8px",
                fontSize: "15px",
                cursor: "pointer",
                fontWeight: "bold"
              },
              children: "Add"
            }
          )
        ] }),
        /* @__PURE__ */ u3(
          FilterBar,
          {
            filter,
            onFilterChange: this.handleFilterChange,
            counts
          }
        ),
        /* @__PURE__ */ u3("div", { children: filtered.map((todo) => /* @__PURE__ */ u3(
          TodoItem,
          {
            todo,
            onToggle: this.handleToggle,
            onDelete: this.handleDelete
          },
          todo.id
        )) }),
        counts.done > 0 && /* @__PURE__ */ u3("div", { style: {
          marginTop: "16px",
          textAlign: "center"
        }, children: /* @__PURE__ */ u3(
          "button",
          {
            onClick: this.handleClearDone,
            style: {
              padding: "8px 16px",
              backgroundColor: "#e74c3c",
              color: "#fff",
              border: "none",
              borderRadius: "6px",
              fontSize: "13px",
              cursor: "pointer"
            },
            children: [
              "Clear done (",
              counts.done,
              ")"
            ]
          }
        ) })
      ] }) });
    }
  };
  nn(/* @__PURE__ */ u3(TodoApp, {}), document.getElementById("root"));
})();
