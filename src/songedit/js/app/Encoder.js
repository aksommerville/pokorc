/* Encoder.js
 */
 
export class Encoder {
  constructor() {
    this.dst = new Uint8Array(1024);
    this.dstc = 0;
  }
  
  finish() {
    const out = new Uint8Array(this.dstc);
    out.set(new Uint8Array(this.dst.buffer, 0, this.dstc));
    return out.buffer;
  }
  
  require(c) {
    if (c < 1) return;
    let na = this.dstc + c;
    if (na <= this.dst.length) return;
    na = (na + 1024) & ~1023;
    const newBuffer = new ArrayBuffer(na);
    const newView = new Uint8Array(newBuffer);
    newView.set(this.dst);
    this.dst = newView;
  }
  
  raw(src) {
    if (src instanceof ArrayBuffer) {
      this.require(src.byteLength);
      this.dst.set(new Uint8Array(src), this.dstc);
      this.dstc += src.byteLength;
      return src.byteLength;
    }
    if (ArrayBuffer.isView(src)) {
      if (!src.byteOffset && (src.byteLength === src.buffer.byteLength)) {
        return this.raw(src.buffer);
      }
      const tmpBuffer = new ArrayBuffer(src.byteLength);
      const tmpView = new Uint8Array(tmpBuffer);
      tmpView.set(src);
      return this.raw(tmpBuffer);
    }
    if (typeof(src) === "string") {
      return this.raw(new TextEncoder("utf-8").encode(src));
    }
    throw new Error("Inappropriate source object for Encoder.raw(), should be string or ArrayBuffer");
  }
  
  u8(src) {
    this.require(1);
    this.dst[this.dstc++] = src;
  }
  
  u16be(src) {
    this.require(2);
    this.dst[this.dstc++] = src >> 8;
    this.dst[this.dstc++] = src;
  }
  
  u32be(src) {
    this.require(4);
    this.dst[this.dstc++] = src >> 24;
    this.dst[this.dstc++] = src >> 16;
    this.dst[this.dstc++] = src >> 8;
    this.dst[this.dstc++] = src;
  }
  
  vlq(src) {
    if ((typeof(src) !== "number") || (src < 0) || (src > 0x0fffffff)) {
      throw new Error(`Encoder.vlq() requires number, got ${src}`);
    }
    this.require(4);
    if (src >= 0x200000) {
      this.dst[this.dstc++] = 0x80 | (src >> 21);
      this.dst[this.dstc++] = 0x80 | ((src >> 14) & 0x7f);
      this.dst[this.dstc++] = 0x80 | ((src >> 7) & 0x7f);
      this.dst[this.dstc++] = src & 0x7f;
      return 4;
    } else if (src >= 0x4000) {
      this.dst[this.dstc++] = 0x80 | (src >> 14);
      this.dst[this.dstc++] = 0x80 | ((src >> 7) & 0x7f);
      this.dst[this.dstc++] = src & 0x7f;
      return 3;
    } else if (src >= 0x80) {
      this.dst[this.dstc++] = 0x80 | (src >> 7);
      this.dst[this.dstc++] = src & 0x7f;
      return 2;
    } else {
      this.dst[this.dstc++] = src;
      return 1;
    }
  }
  
  /* Capture (p=dstc) before encoding the chunk, then call one of these to insert the length.
   */
  u32belen(p) {
    const len = this.dstc - p;
    this.require(4);
    this.dst.set(new Uint8Array(this.dst.buffer, p, len), p + 4);
    this.dst[p++] = len >> 24;
    this.dst[p++] = len >> 16;
    this.dst[p++] = len >> 8;
    this.dst[p++] = len;
    this.dstc += 4;
  }
  vlqlen(p) {
    const len = this.dstc - p;
    this.require(4);
    const tmpView = new Uint8Array(len);
    tmpView.set(new Uint8Array(this.dst.buffer, p, len));
    this.dstc = p;
    this.vlq(len);
    this.raw(tmpView);
  }
}
