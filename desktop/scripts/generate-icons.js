const fs = require("fs");
const path = require("path");
const zlib = require("zlib");

const root = path.resolve(__dirname, "..");
const assetsDir = path.join(root, "assets");
const iconsetDir = path.join(assetsDir, "icon.iconset");

const logoRows = [
  "                    ..                     ",
  "                  .8888.                   ",
  "                 .888888.                  ",
  "                .88888888.                 ",
  "               .8888888888.                ",
  "              .888888888888.               ",
  "             .88888888888888.              ",
  "            .8888888888888888.             ",
  "           .888888P\"  \"Y888888.            ",
  "          .88888P'      `Y88888.           ",
  "         .88888P          Y88888.          ",
  "        .88888P            Y88888.         ",
  "       .88888P    .8888.    Y88888.        ",
  "      .88888P    .888888.    Y88888.       ",
  "     .88888P    .88888888.    Y88888.      ",
  "    .88888P    .8888888888.    Y88888.     ",
  "   .88888P    .888888888888.    Y88888.    ",
  "  .88888P    .88888888888888.    Y88888.   "
];

const logoPalette = [
  [116, 199, 236],
  [116, 199, 236],
  [116, 199, 236],
  [137, 180, 250],
  [137, 180, 250],
  [137, 180, 250],
  [180, 190, 254],
  [180, 190, 254],
  [180, 190, 254],
  [203, 166, 247],
  [203, 166, 247],
  [203, 166, 247],
  [245, 194, 231],
  [245, 194, 231],
  [245, 194, 231],
  [245, 194, 231],
  [245, 194, 231],
  [245, 194, 231]
];

fs.mkdirSync(assetsDir, { recursive: true });

function crcTable() {
  const table = new Uint32Array(256);
  for (let n = 0; n < 256; n++) {
    let c = n;
    for (let k = 0; k < 8; k++) {
      c = (c & 1) ? (0xedb88320 ^ (c >>> 1)) : (c >>> 1);
    }
    table[n] = c >>> 0;
  }
  return table;
}

const crcLookup = crcTable();

function crc32(buf) {
  let c = 0xffffffff;
  for (const byte of buf) c = crcLookup[(c ^ byte) & 0xff] ^ (c >>> 8);
  return (c ^ 0xffffffff) >>> 0;
}

function chunk(type, data) {
  const typeBuf = Buffer.from(type);
  const out = Buffer.alloc(12 + data.length);
  out.writeUInt32BE(data.length, 0);
  typeBuf.copy(out, 4);
  data.copy(out, 8);
  out.writeUInt32BE(crc32(Buffer.concat([typeBuf, data])), 8 + data.length);
  return out;
}

function clamp(v) {
  return Math.max(0, Math.min(255, Math.round(v)));
}

function mix(a, b, t) {
  return a + (b - a) * t;
}

function roundRectDistance(px, py, x, y, width, height, radius) {
  const cx = x + width / 2;
  const cy = y + height / 2;
  const qx = Math.abs(px - cx) - (width / 2 - radius);
  const qy = Math.abs(py - cy) - (height / 2 - radius);
  return Math.hypot(Math.max(qx, 0), Math.max(qy, 0)) + Math.min(Math.max(qx, qy), 0) - radius;
}

function blendPixel(raw, size, x, y, color, alpha) {
  if (x < 0 || y < 0 || x >= size || y >= size || alpha <= 0) return;

  const i = y * (size * 4 + 1) + 1 + x * 4;
  const srcA = Math.max(0, Math.min(1, alpha));
  const dstA = raw[i + 3] / 255;
  const outA = srcA + dstA * (1 - srcA);

  if (outA <= 0) return;

  raw[i] = clamp((color[0] * srcA + raw[i] * dstA * (1 - srcA)) / outA);
  raw[i + 1] = clamp((color[1] * srcA + raw[i + 1] * dstA * (1 - srcA)) / outA);
  raw[i + 2] = clamp((color[2] * srcA + raw[i + 2] * dstA * (1 - srcA)) / outA);
  raw[i + 3] = clamp(outA * 255);
}

function paintRoundedRect(raw, size, x, y, width, height, radius, color, opacity) {
  const minX = Math.max(0, Math.floor(x - 1));
  const minY = Math.max(0, Math.floor(y - 1));
  const maxX = Math.min(size - 1, Math.ceil(x + width + 1));
  const maxY = Math.min(size - 1, Math.ceil(y + height + 1));

  for (let py = minY; py <= maxY; py++) {
    for (let px = minX; px <= maxX; px++) {
      const distance = roundRectDistance(px + 0.5, py + 0.5, x, y, width, height, radius);
      const coverage = Math.max(0, Math.min(1, 0.5 - distance));
      if (coverage > 0) blendPixel(raw, size, px, py, color, opacity * coverage);
    }
  }
}

function fillIconBackground(raw, size) {
  const radius = size * 0.2;

  for (let y = 0; y < size; y++) {
    const row = y * (size * 4 + 1);
    raw[row] = 0;

    for (let x = 0; x < size; x++) {
      const distance = roundRectDistance(x + 0.5, y + 0.5, 0, 0, size, size, radius);
      const coverage = Math.max(0, Math.min(1, 0.5 - distance));
      if (coverage <= 0) continue;

      const t = y / Math.max(1, size - 1);
      const edge = distance > -size * 0.018 ? 1 : 0;
      const base = [
        mix(18, 9, t),
        mix(20, 12, t),
        mix(24, 16, t)
      ];

      blendPixel(raw, size, x, y, base, coverage);
      if (edge) blendPixel(raw, size, x, y, [52, 58, 68], coverage * 0.95);
    }
  }

  paintRoundedRect(raw, size, size * 0.055, size * 0.055, size * 0.89, size * 0.89, size * 0.15, [20, 23, 28], 0.42);
}

function charMetrics(ch, cellW, cellH) {
  if (ch === ".") return [cellW * 0.42, cellH * 0.26, Math.min(cellW, cellH) * 0.25];
  if (ch === "'" || ch === "`") return [cellW * 0.34, cellH * 0.54, Math.min(cellW, cellH) * 0.18];
  if (ch === "\"") return [cellW * 0.48, cellH * 0.5, Math.min(cellW, cellH) * 0.18];
  if (ch === "P" || ch === "Y") return [cellW * 0.74, cellH * 0.64, Math.min(cellW, cellH) * 0.22];
  return [cellW * 0.68, cellH * 0.62, Math.min(cellW, cellH) * 0.26];
}

function paintAsciiLogo(raw, size) {
  const cols = Math.max(...logoRows.map((row) => row.length));
  const boxW = size * 0.72;
  const boxH = size * 0.55;
  const boxX = (size - boxW) / 2;
  const boxY = size * 0.2;
  const cellW = boxW / cols;
  const cellH = boxH / logoRows.length;

  for (let rowIndex = 0; rowIndex < logoRows.length; rowIndex++) {
    const row = logoRows[rowIndex];
    const color = logoPalette[rowIndex];
    const glow = [
      mix(color[0], 255, 0.18),
      mix(color[1], 255, 0.18),
      mix(color[2], 255, 0.18)
    ];

    for (let col = 0; col < row.length; col++) {
      const ch = row[col];
      if (ch === " ") continue;

      const [w, h, r] = charMetrics(ch, cellW, cellH);
      const x = boxX + col * cellW + (cellW - w) / 2;
      const y = boxY + rowIndex * cellH + (cellH - h) / 2;

      paintRoundedRect(raw, size, x - cellW * 0.18, y - cellH * 0.2, w + cellW * 0.36, h + cellH * 0.4, r, glow, 0.11);
      paintRoundedRect(raw, size, x, y, w, h, r, color, 0.94);
    }
  }
}

function iconPng(size) {
  const raw = Buffer.alloc((size * 4 + 1) * size);

  fillIconBackground(raw, size);
  paintAsciiLogo(raw, size);

  const header = Buffer.alloc(13);
  header.writeUInt32BE(size, 0);
  header.writeUInt32BE(size, 4);
  header[8] = 8;
  header[9] = 6;
  header[10] = 0;
  header[11] = 0;
  header[12] = 0;

  return Buffer.concat([
    Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]),
    chunk("IHDR", header),
    chunk("IDAT", zlib.deflateSync(raw, { level: 9 })),
    chunk("IEND", Buffer.alloc(0))
  ]);
}

function writePng(name, size) {
  const file = path.join(iconsetDir, name);
  fs.writeFileSync(file, iconPng(size));
}

fs.rmSync(iconsetDir, { recursive: true, force: true });
fs.mkdirSync(iconsetDir, { recursive: true });

writePng("icon_16x16.png", 16);
writePng("icon_16x16@2x.png", 32);
writePng("icon_32x32.png", 32);
writePng("icon_32x32@2x.png", 64);
writePng("icon_128x128.png", 128);
writePng("icon_128x128@2x.png", 256);
writePng("icon_256x256.png", 256);
writePng("icon_256x256@2x.png", 512);
writePng("icon_512x512.png", 512);
writePng("icon_512x512@2x.png", 1024);

const png1024 = iconPng(1024);
fs.writeFileSync(path.join(assetsDir, "icon.png"), png1024);

const icoImages = [16, 32, 48, 256].map((size) => iconPng(size));
const icoHeader = Buffer.alloc(6 + icoImages.length * 16);
icoHeader.writeUInt16LE(0, 0);
icoHeader.writeUInt16LE(1, 2);
icoHeader.writeUInt16LE(icoImages.length, 4);

let offset = icoHeader.length;
icoImages.forEach((image, index) => {
  const size = [16, 32, 48, 256][index];
  const entry = 6 + index * 16;
  icoHeader[entry] = size === 256 ? 0 : size;
  icoHeader[entry + 1] = size === 256 ? 0 : size;
  icoHeader[entry + 2] = 0;
  icoHeader[entry + 3] = 0;
  icoHeader.writeUInt16LE(1, entry + 4);
  icoHeader.writeUInt16LE(32, entry + 6);
  icoHeader.writeUInt32LE(image.length, entry + 8);
  icoHeader.writeUInt32LE(offset, entry + 12);
  offset += image.length;
});
fs.writeFileSync(path.join(assetsDir, "icon.ico"), Buffer.concat([icoHeader, ...icoImages]));

const icnsChunks = [
  ["icp4", iconPng(16)],
  ["icp5", iconPng(32)],
  ["icp6", iconPng(64)],
  ["ic07", iconPng(128)],
  ["ic08", iconPng(256)],
  ["ic09", iconPng(512)],
  ["ic10", png1024]
].map(([type, data]) => {
  const header = Buffer.alloc(8);
  header.write(type, 0, "ascii");
  header.writeUInt32BE(data.length + 8, 4);
  return Buffer.concat([header, data]);
});

const icnsSize = 8 + icnsChunks.reduce((sum, item) => sum + item.length, 0);
const icnsHeader = Buffer.alloc(8);
icnsHeader.write("icns", 0, "ascii");
icnsHeader.writeUInt32BE(icnsSize, 4);
fs.writeFileSync(path.join(assetsDir, "icon.icns"), Buffer.concat([icnsHeader, ...icnsChunks]));

console.log("Generated YukiShell icons from the terminal ASCII mark.");
