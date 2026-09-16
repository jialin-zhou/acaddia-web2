/**
 * 协议字节级自测（node 运行：node test/protocol-test.js）
 * 对照 Qt TeleProcess::TelePack 手算期望帧逐一验证
 */
const fs = require('fs');
// 先加载日志模块（protocol.js 校验失败时会调用 Logger.warn）
eval(fs.readFileSync(__dirname + '/../js/log.js', 'utf8'));
eval(fs.readFileSync(__dirname + '/../js/protocol.js', 'utf8') +
    '\n;global.Unpacker = Unpacker; global.readShortLE = readShortLE; global.writeShortLE = writeShortLE;');

let failures = 0;

function hex(arr) { return Array.from(arr).map((b) => b.toString(16).padStart(2, '0').toUpperCase()).join(' '); }
function expect(name, actual, expected) {
    const a = hex(actual), e = hex(expected);
    if (a === e) {
        console.log(`  PASS  ${name}: ${a}`);
    } else {
        failures++;
        console.log(`  FAIL  ${name}\n    expect: ${e}\n    actual: ${a}`);
    }
}

console.log('== 1. 读取命令（空数据，zero=0）==');
// 35: CRC = 0 + 35 + 0x09 + 0x80 = 0xAC
expect('TQCS获取 35', telePack(35, [], 0), [0x68, 0x04, 0x04, 0x68, 0x00, 0x09, 0x80, 0x23, 0xAC, 0x16]);
// 33: CRC = 0 + 33 + 0x09 + 0x80 = 0xAA
expect('AD获取 33', telePack(33, [], 0), [0x68, 0x04, 0x04, 0x68, 0x00, 0x09, 0x80, 0x21, 0xAA, 0x16]);
// 37: CRC = 0 + 37 + 0x09 + 0x80 = 0xAE
expect('AD计算值 37', telePack(37, [], 0), [0x68, 0x04, 0x04, 0x68, 0x00, 0x09, 0x80, 0x25, 0xAE, 0x16]);
// 47: 0x2F → CRC = 0x2F+0x89 = 0xB8
expect('角度 47', telePack(47, [], 0), [0x68, 0x04, 0x04, 0x68, 0x00, 0x09, 0x80, 0x2F, 0xB8, 0x16]);
// 49: 0x31 → CRC = 0x31+0x89 = 0xBA
expect('通信报文 49', telePack(49, [], 0), [0x68, 0x04, 0x04, 0x68, 0x00, 0x09, 0x80, 0x31, 0xBA, 0x16]);
// 29: 0x1D → CRC = 0x1D+0x89 = 0xA6
expect('时间获取 29', telePack(29, [], 0), [0x68, 0x04, 0x04, 0x68, 0x00, 0x09, 0x80, 0x1D, 0xA6, 0x16]);

console.log('== 2. 同期参数下载（41 字节缓冲 → 40 字节 payload）==');
{
    const data = new Uint8Array(41);
    data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0; data[4] = 0; data[5] = 0;
    data[6] = 80; data[7] = 0; // tcb=80
    data[8] = 0xE8; data[9] = 0x03; // damax=10.0 → 1000
    data[10] = 0xC8; data[11] = 0; // duMax=2 → 200
    data[12] = 20; data[13] = 0; // dfMax=20
    data[14] = 0xC8; data[15] = 0; // dUmax2=2 → 200
    data[16] = 10; data[17] = 0; // dFmax2=0.10 → 10
    data[18] = 0xC8; data[19] = 0; // uDead=2
    data[20] = 0; data[21] = 0; // trU1U2=0
    data[22] = 20; data[23] = 0; // dfptMax=0.2 → 20
    data[24] = 0x50; data[25] = 0x14; // uMin=52 → 5200
    data[26] = 0xCE; data[27] = 0x18; // uMax=63.5 → 6350
    data[28] = 0x8E; data[29] = 0x12; // fMin=47.5 → 4750
    data[30] = 0x82; data[31] = 0x14; // fMax=52.5 → 5250
    data[32] = 0x2C; data[33] = 0x01; // tsyn=30 → 300
    data[34] = 5; data[35] = 0; // synchron=0.5 → 5
    data[36] = 1; // 功能使能
    // 37..40 = 0
    let zero = 0;
    for (let i = 0; i < 41; i++) if (data[i] === 0) zero++;
    zero--; // 4-1=3
    const frame = telePack(34, data, zero);
    if (frame.length !== 44 + 6) { failures++; console.log(`  FAIL  TQCS下载 长度=${frame.length} 期望 50`); }
    else console.log(`  PASS  TQCS下载 长度=50 (L=${frame[1]})`);
    if (frame[1] !== 44) { failures++; console.log(`  FAIL  TQCS下载 L=${frame[1]} 期望 44`); }
    else console.log(`  PASS  TQCS下载 L=44`);
    // payload 应为 data[0..39]
    let ok = true;
    for (let i = 0; i < 40; i++) if (frame[8 + i] !== data[i]) ok = false;
    if (!ok) { failures++; console.log(`  FAIL  TQCS下载 payload 与 DATA[0..39] 不一致`); }
    else console.log(`  PASS  TQCS下载 payload = DATA[0..39]`);
    // 校验和 = 0+34+9+0x80+sum(data[0..39])
    let crc = (0 + 34 + 9 + 0x80) & 0xFF;
    for (let i = 0; i < 40; i++) crc = (crc + data[i]) & 0xFF;
    expect('TQCS下载 校验和', frame.subarray(48, 49), [crc]);
    expect('TQCS下载 帧尾', frame.subarray(49, 50), [0x16]);
}

console.log('== 3. AD参数下载（90 字节 → 89 字节 payload）==');
{
    const data = new Uint8Array(90);
    for (let i = 0; i < 89; i++) data[i] = (i * 7 + 1) & 0xFF; // 无 0
    data[89] = 0;
    let zero = 0;
    for (let i = 0; i < 90; i++) if (data[i] === 0) zero++;
    zero--; // 0
    const frame = telePack(32, data, zero);
    if (frame[1] !== 93) { failures++; console.log(`  FAIL  AD下载 L=${frame[1]} 期望 93`); }
    else console.log(`  PASS  AD下载 L=93`);
    let ok = true;
    for (let i = 0; i < 89; i++) if (frame[8 + i] !== data[i]) ok = false;
    if (!ok) { failures++; console.log(`  FAIL  AD下载 payload 与 DATA[0..88] 不一致`); }
    else console.log(`  PASS  AD下载 payload = DATA[0..88] (89 字节)`);
    let crc = (0 + 32 + 9 + 0x80) & 0xFF;
    for (let i = 0; i < 89; i++) crc = (crc + data[i]) & 0xFF;
    expect('AD下载 校验和', frame.subarray(97, 98), [crc]);
}

console.log('== 4. 通道校准（4 字节 → 3 字节 payload）==');
{
    // 通道 Ua line1: 通道号 1, 值 57.74*16384/57.74=16384=0x4000
    const data = new Uint8Array([1, 0x00, 0x40, 0]);
    let zero = 0;
    for (let i = 0; i < 4; i++) if (data[i] === 0) zero++;
    zero--; // 0
    const frame = telePack(38, data, zero);
    if (frame[1] !== 7) { failures++; console.log(`  FAIL  校准 L=${frame[1]} 期望 7`); }
    else console.log(`  PASS  校准 L=7`);
    expect('校准 payload', frame.subarray(8, 11), [1, 0x00, 0x40]);
    // CRC = 0+38+9+0x80+1+0+0x40 = 0x26+0x89+0x41 = 0xF0
    expect('校准 校验和', frame.subarray(11, 12), [0xF0]);
}

console.log('== 5. 同期开出（[55 AA 00] → [55 AA]）==');
{
    const frame = telePack(44, new Uint8Array([0x55, 0xAA, 0]), 0);
    if (frame[1] !== 6) { failures++; console.log(`  FAIL  TQML开出 L=${frame[1]} 期望 6`); }
    else console.log(`  PASS  TQML开出 L=6`);
    expect('TQML开出 payload', frame.subarray(8, 10), [0x55, 0xAA]);
    // CRC = 0+44+9+0x80+0x55+0xAA = 0x2C+0x89+0xFF = 0x1B4 & FF = 0xB4
    expect('TQML开出 校验和', frame.subarray(10, 11), [0xB4]);
}

console.log('== 6. 同期读取（[类型 00] → 1 字节）==');
{
    const frame = telePack(45, new Uint8Array([2, 0]), 0);
    if (frame[1] !== 5) { failures++; console.log(`  FAIL  TQML读取 L=${frame[1]} 期望 5`); }
    else console.log(`  PASS  TQML读取 L=5`);
    expect('TQML读取 payload', frame.subarray(8, 9), [2]);
}

console.log('== 7. 时间设置（7 字节 → 6 字节 payload）==');
{
    // days=12345=0x3039, msecs=45296000=0x02B32C80
    const data = new Uint8Array([0x39, 0x30, 0x80, 0x2C, 0xB3, 0x02, 0]);
    const frame = telePack(28, data, 0);
    if (frame[1] !== 10) { failures++; console.log(`  FAIL  时间设置 L=${frame[1]} 期望 10`); }
    else console.log(`  PASS  时间设置 L=10`);
    expect('时间设置 payload', frame.subarray(8, 14), [0x39, 0x30, 0x80, 0x2C, 0xB3, 0x02]);
}

console.log('== 8. 解帧器 ==');
{
    const u = new Unpacker();
    // 设备响应：E5 + 34 号数据帧（含 4 字节头 + 40 字节数据）
    const payload = [];
    for (let i = 0; i < 40; i++) payload.push((i * 3 + 5) & 0xFF);
    const frame = telePack(34, new Uint8Array([...payload, 0]), 0);
    const stream = new Uint8Array([0xE5, ...frame]);
    u.addData(stream);
    const frames = u.unpack();
    const types = frames.map((f) => f.type);
    expect('解帧 类型序列', types, ['ack_e5', 'data']);
    const dataFrame = frames[1];
    if (dataFrame.telegramNr !== 34) { failures++; console.log(`  FAIL  解帧 报文号=${dataFrame.telegramNr}`); }
    else console.log(`  PASS  解帧 报文号=34`);
    if (dataFrame.dataBlock.length !== 44) { failures++; console.log(`  FAIL  解帧 dataBlock 长度=${dataFrame.dataBlock.length}`); }
    else console.log(`  PASS  解帧 dataBlock 长度=44（含 4 字节头）`);
    // dataBlock = [站址, F1, F2, 报文号, ...payload]
    let ok = dataFrame.dataBlock[0] === 0 && dataFrame.dataBlock[1] === 0x09 &&
        dataFrame.dataBlock[2] === 0x80 && dataFrame.dataBlock[3] === 34;
    for (let i = 0; i < 40; i++) if (dataFrame.dataBlock[4 + i] !== payload[i]) ok = false;
    if (!ok) { failures++; console.log(`  FAIL  解帧 dataBlock 内容不符`); }
    else console.log(`  PASS  解帧 dataBlock 内容 = 4 字节头 + payload`);

    // 校验错误帧被丢弃
    const bad = telePack(34, new Uint8Array([1, 2, 0]), 0);
    bad[bad.length - 2] ^= 0xFF; // 破坏校验和
    u.reset();
    u.addData(new Uint8Array([0x68, ...bad.subarray(0)]));
    const badFrames = u.unpack();
    if (badFrames.length !== 0 && badFrames[0].type !== 'data') {
        console.log(`  PASS  校验错误帧被丢弃`);
    } else { failures++; console.log(`  FAIL  校验错误帧未被丢弃: ${JSON.stringify(badFrames)}`); }

    // 裸 E5
    u.reset();
    u.addData(new Uint8Array([0xE5]));
    const ack = u.unpack();
    if (ack.length === 1 && ack[0].type === 'ack_e5') console.log(`  PASS  裸 E5 → ack_e5`);
    else { failures++; console.log(`  FAIL  裸 E5 解析错误`); }

    // 裸 68 帧（无 E5 前缀）也能解析（比 Qt 更宽容，行为等价）
    u.reset();
    u.addData(new Uint8Array([...frame]));
    const f2 = u.unpack();
    if (f2.length === 1 && f2[0].type === 'data' && f2[0].telegramNr === 34) console.log(`  PASS  无 E5 前缀的 68 帧可解析`);
    else { failures++; console.log(`  FAIL  无前缀帧解析错误`); }

    // 0xA2 → ack_a2
    u.reset();
    u.addData(new Uint8Array([0xA2]));
    const a2 = u.unpack();
    if (a2.length === 1 && a2[0].type === 'ack_a2') console.log(`  PASS  0xA2 → ack_a2`);
    else { failures++; console.log(`  FAIL  0xA2 解析错误`); }

    // E5 + 帧 + 裸 E5 同一批次（serial.js processBatch 会跳过帧前缀 E5、裸 E5 按 ACK 处理）
    u.reset();
    u.addData(new Uint8Array([0xE5, ...frame, 0xE5]));
    const mix = u.unpack();
    const mixTypes = mix.map((f) => f.type).join(',');
    if (mixTypes === 'ack_e5,data,ack_e5') console.log(`  PASS  混合批次解析序列: ${mixTypes}`);
    else { failures++; console.log(`  FAIL  混合批次解析序列: ${mixTypes}`); }
}

console.log(failures === 0 ? '\n全部通过 ✓' : `\n${failures} 项失败 ✗`);
process.exit(failures === 0 ? 0 : 1);
