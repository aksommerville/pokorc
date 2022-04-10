/* Decoder.js
 */
 
export class Decoder {

  constructor(src) {
    if (src instanceof Uint8Array) {
      this.src = src;
    } else if (src instanceof ArrayBuffer) {
      this.src = new Uint8Array(src);
    } else if (ArrayBuffer.isView(src)) {
      this.src = new Uint8Array(src.buffer, src.byteOffset, src.byteLength);
    } else if (src instanceof Array) {
      this.src = new Uint8Array(src);
    } else if (!src) {
      this.src = new Uint8Array([]);
    } else {
      throw new Error(`Inappropriate source for Decoder: ${typeof src} ${src}`);
    }
    this.srcp = 0;
  }
  
  remaining() {
    return this.src.length - this.srcp;
  }
  
  require(c) {
    if (this.srcp <= this.src.length - c) return;
    throw new Error(`require ${c} more bytes at ${this.srcp}/${this.src.length}`);
  }
  
  unread(c) {
    if (c > this.srcp) throw new Error(`Request to unread ${c} at ${this.srcp}/${this.src.length}`);
    this.srcp -= c;
  }
  
  u8() {
    this.require(1);
    return this.src[this.srcp++];
  }
  
  u16be() {
    this.require(2);
    const v = (this.src[this.srcp] << 8) | this.src[this.srcp + 1];
    this.srcp += 2;
    return v;
  }
  
  u32be() {
    this.require(4);
    const v = (this.src[this.srcp] << 24) | (this.src[this.srcp + 1] << 16) | (this.src[this.srcp + 2] << 8) | this.src[this.srcp + 3];
    this.srcp += 4;
    return v;
  }
  
  vlq() {
    const p0 = this.srcp;
    try {
      let v = 0, c = 0;
      while (true) {
        this.require(1);
        const sub = this.src[this.srcp++];
        v <<= 7;
        v |= sub & 0x7f;
        if (!(sub & 0x80)) break;
        if (++c > 4) throw new Error(`VLQ at ${p0}/${this.src.length} unterminated after 4 bytes`);
      }
      return v;
    } catch (e) {
      this.srcp = p0;
      throw e;
    }
  }
  
  binary(c) {
    this.require(c);
    const dst = new ArrayBuffer(c);
    const dstView = new Uint8Array(dst);
    const srcView = new Uint8Array(this.src.buffer, this.src.byteOffset + this.srcp, c);
    dstView.set(srcView);
    this.srcp += c;
    return dst;
  }
  
  string(c) {
    this.require(c);
    const srcArray = new Uint8Array(this.src.buffer, this.src.byteOffset + this.srcp, c);
    const string = new TextDecoder("utf-8").decode(srcArray);
    this.srcp += c;
    return string;
  }
  
  // binary() with a leading length
  u32belen() {
    const len = this.u32be();
    return this.binary(len);
  }
  vlqlen() {
    const len = this.vlq();
    return this.binary(len);
  }
}
