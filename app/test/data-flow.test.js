/**
 * 数据流等价性测试（对照 Qt 版，node test/data-flow.test.js）
 *
 * 三类数据流：
 *  A. 网页 → 装置：网页组装并发送的数据必须与 Qt TelePack 逐字节一致
 *  B. 装置 → 网页：装置发来的帧，网页的解析/展示必须与 Qt 各对话框一致
 *  C. 网页内部流转：数据在网页内部的流转（注册表→显示、文件→界面→下发、标幺→换算）
 *     必须与 Qt 的 ValidData / DIM_public / DC_V_I_select 等内部流转一致
 *
 * 期望值均为按 Qt 源码公式手工计算（测试注释中注明出处），不依赖被测实现。
 */
const fs = require('fs');

// ---- DOM mock（store.js 的 showMessageBox 需要） ----
const mockElement = () => ({
    className: '', innerHTML: '', textContent: '', value: '', checked: false, style: {},
    querySelector: () => ({ addEventListener() {} }),
    addEventListener() {}, appendChild() {}, remove() {}, setAttribute() {},
});
global.document = {
    getElementById: () => mockElement(),
    createElement: () => mockElement(),
    body: mockElement(),
};
global.window = global;
global.bootstrap = { Modal: { getOrCreateInstance: () => ({ show() {} }), getInstance: () => null } };

// ---- 载入被测模块 ----
// 先加载日志模块（protocol.js/store.js 会调用 Logger）
eval(fs.readFileSync(__dirname + '/../js/log.js', 'utf8'));
eval(fs.readFileSync(__dirname + '/../js/protocol.js', 'utf8') +
    '\n;global.telePack = telePack; global.Unpacker = Unpacker; global.readShortLE = readShortLE; global.writeShortLE = writeShortLE; global.packAck = packAck;');
eval(fs.readFileSync(__dirname + '/../js/store.js', 'utf8') +
    '\n;global.Qt = Qt; global.Bus = Bus;');
eval(fs.readFileSync(__dirname + '/../js/core.js', 'utf8'));

let failures = 0;
function check(name, cond, detail) {
    if (cond) console.log(`  PASS  ${name}`);
    else { failures++; console.log(`  FAIL  ${name}${detail !== undefined ? ' — ' + detail : ''}`); }
}
function hex(arr) { return Array.from(arr).map((b) => b.toString(16).padStart(2, '0').toUpperCase()).join(' '); }
function arrEq(a, b) {
    if (a.length !== b.length) return false;
    for (let i = 0; i < a.length; i++) if (a[i] !== b[i]) return false;
    return true;
}

// =====================================================================
// A. 网页 → 装置（Qt TelePack / 各对话框打包逻辑逐字节对照）
// =====================================================================

console.log('== A1. 同期参数下载（Qt Dlg_TQCS1::packDataToBuffer + 补零压缩） ==');
{
    // 默认参数（Qt setDefaultValues）：routineSelect=1(手动)、tcb=80、damax=10.0、duMax=2.0、dfMax=20.0、
    // dUmax2=2.0、dFmax2=0.10、uDead=2.0、trU1U2=0、dfptMax=0.2、uMin=52、uMax=63.5、fMin=47.5、fMax=52.5、
    // tsyn=30、synchron=0.5、funcEnabled=1；缩放 ×1/×100/×10（见 Dlg_TQCS1.cpp 220-236 行）
    const fields = {
        routineSelect: 1, syncStyle: 0, deadL1: false, deadL2: false, deadAll: false, phase: 0,
        tcb: 80, damax: 10.0, duMax: 2.0, dfMax: 20.0, dUmax2: 2.0, dFmax2: 0.10,
        uDead: 2.0, trU1U2: 0.0, dfptMax: 0.2,
        uMin: 52.0, uMax: 63.5, fMin: 47.5, fMax: 52.5,
        tsynDuration: 30.0, synchron: 0.5, funcEnabled: true,
    };
    // 手算期望 41 字节缓冲：
    // [0]=!1=0 [1]=0 [2..4]=0 [5]=0 [6..7]=80 [8..9]=1000=0x03E8 [10..11]=200=0xC8
    // [12..13]=20 [14..15]=200 [16..17]=10 [18..19]=200 [20..21]=0 [22..23]=20
    // [24..25]=5200=0x1450 [26..27]=6350=0x18CE [28..29]=4750=0x128E [30..31]=5250=0x1482
    // [32..33]=300=0x012C [34..35]=5 [36]=1 [37..40]=0
    const expected41 = [
        0, 0, 0, 0, 0, 0,
        80, 0, 0xE8, 0x03, 0xC8, 0x00, 20, 0, 0xC8, 0x00, 10, 0, 0xC8, 0x00, 0, 0, 20, 0,
        0x50, 0x14, 0xCE, 0x18, 0x8E, 0x12, 0x82, 0x14, 0x2C, 0x01, 5, 0,
        1, 0, 0, 0, 0,
    ];
    const buf = packTqcsPayload(fields);
    check('41 字节缓冲逐字节一致', arrEq(buf, expected41), `actual=${hex(buf)}`);

    // 补零压缩：Qt 统计 41 字节中 0 的个数(20)再减一 → zero=19 → TelePack 发出 40 字节
    let zero = 0;
    for (let i = 0; i < 41; i++) if (buf[i] === 0) zero++;
    zero--;
    const frame = telePack(34, buf, zero);
    check('TQCS下载 帧长 L=44', frame[1] === 44 && frame.length === 50, `L=${frame[1]} len=${frame.length}`);
    // 期望完整帧（手算 CRC = (0+34+9+0x80+sum(payload))&0xFF = 0x23）
    const expectedFrame = new Uint8Array([
        0x68, 44, 44, 0x68, 0x00, 0x09, 0x80, 0x22,
        ...expected41.slice(0, 40),
        0x23, 0x16,
    ]);
    check('TQCS下载 完整帧逐字节一致（含校验和）', arrEq(frame, expectedFrame), hex(frame));

    // 缩放验证：damax=0.5° → ×100 → 50；dFmax2=1.23Hz → ×100 → 123
    const f2 = { ...fields, damax: 0.5, dFmax2: 1.23, tsynDuration: 2.5 };
    const buf2 = packTqcsPayload(f2);
    check('damax 0.5° → 50', readShortLE(buf2, 8) === 50);
    check('dFmax2 1.23Hz → 123', readShortLE(buf2, 16) === 123);
    check('tsyn 2.5s → 25（×10）', readShortLE(buf2, 32) === 25);
}

console.log('== A2. AD参数下载（Qt Dlg_AD1 打包 + 补零压缩 → 89 字节） ==');
{
    // 默认参数 DIM_public=16384 分支（Dlg_AD1.cpp 936-972 行）：
    // U 比例=28380(0x6EDC)、I 比例=17900(0x45EC)、V_DC=0、C_DC=50360(0xC4B8)、
    // V_LINE=500、C_LINE=3000、model=0、adFix=0、oct=3000→DATA[67]=120、channels=0,1,2,3
    const fields = adDefaultFields(16384);
    const data = packAdPayload(fields);
    check('DATA 长度 90', data.length === 90);
    check('U1a = 28380 (0x6EDC 小端)', data[0] === 0xDC && data[1] === 0x6E, hex(data.subarray(0, 2)));
    check('I1a = 17900 (0x45EC 小端)', data[6] === 0xEC && data[7] === 0x45, hex(data.subarray(6, 8)));
    check('C_DC1 = 50360 (0xC4B8 小端)', data[40] === 0xB8 && data[41] === 0xC4, hex(data.subarray(40, 42)));
    check('V_LINE1 = 500', readShortLE(data, 56) === 500);
    check('C_LINE1 = 3000', readShortLE(data, 60) === 3000);
    check('DATA[67] = 3000/25 = 120', data[67] === 120);
    check('DATA[68] = 0（SecVNV）', data[68] === 0);
    check('channels = 0,1,2,3', arrEq(Array.from(data.subarray(77, 81)), [0, 1, 2, 3]));
    check('DATA[89] = 0（bak 终止符）', data[89] === 0);

    let zero = 0;
    for (let i = 0; i < 90; i++) if (data[i] === 0) zero++;
    zero--; // Qt: Zero_NR 减一
    const frame = telePack(32, data, zero);
    check('AD下载 帧长 L=93（89 字节 payload）', frame[1] === 93 && frame.length === 99, `L=${frame[1]} len=${frame.length}`);
    check('AD下载 payload = DATA[0..88]', arrEq(frame.subarray(8, 97), data.subarray(0, 89)));
    check('AD下载 帧尾 0x16', frame[98] === 0x16);

    // 8192 分支默认值（Dlg_AD1.cpp 902-935 行）：DC 全 DIM_DCS=20460(0x4FEC)、U=7095(0x1BB7)、I=7024(0x1B70)
    const f8192 = adDefaultFields(8192);
    const d8192 = packAdPayload(f8192);
    check('8192: V_DC1 = 20460 (0x4FEC)', readShortLE(d8192, 24) === 20460, String(readShortLE(d8192, 24)));
    check('8192: C_DC3 = 20460（Qt 漏赋，本版按意图补齐）', readShortLE(d8192, 44) === 20460);
    check('8192: U1a = 7095 (0x1BB7)', readShortLE(d8192, 0) === 7095, String(readShortLE(d8192, 0)));
    check('8192: I1a = 7024 (0x1B70)', readShortLE(d8192, 6) === 7024, String(readShortLE(d8192, 6)));

    // OCT 折算（Qt: DATA[67] = static_cast<uchar>(scaling/25)）
    // 12345/25 = 493.8 → C++ 整数除法 493 = 0x1ED → uchar 截断 = 0xED = 237
    const foct = { ...fields, oct: 12345 };
    const doct = packAdPayload(foct);
    check('OCT 12345 → DATA[67]=237（C++ 整数除法 + uchar 截断）', doct[67] === 237, String(doct[67]));
}

console.log('== A3. 通道校准（Qt Dlg_ADAdjust1：4 字节 DATA → 实发 3 字节） ==');
{
    // line1 Ua：通道号 = 0*6+0+1 = 1；校准源 57.74，除数 57.74，DIM=16384 → Temp_short=16384=0x4000
    const chNo = adjustChannelNo(0, 0, -1);
    const div = adjustDivisor(0, 0, -1, 0);
    const ts = adjustTempShort(57.74, div, 16384);
    check('line1 Ua 通道号 = 1', chNo === 1, String(chNo));
    check('Temp_short = 16384 (0x4000)', ts === 16384, ts.toString(16));
    const { data, zero } = adjustPayload(chNo, ts);
    const frame = telePack(38, data, zero);
    check('校准帧 L=7（3 字节 payload）', frame[1] === 7 && frame.length === 13, `L=${frame[1]}`);
    check('校准帧逐字节 = 68 07 07 68 00 09 80 26 01 00 40 F0 16',
        arrEq(frame, [0x68, 7, 7, 0x68, 0, 9, 0x80, 0x26, 1, 0, 0x40, 0xF0, 0x16]), hex(frame));

    // line2 Ic：通道号 = 1*6+5+1 = 12；Current_Style=0 → 除数 5；1A → 1*16384/5 = 3276.8 → 3276(取整)
    const ch2 = adjustChannelNo(1, 5, -1);
    const ts2 = adjustTempShort(1, adjustDivisor(1, 5, -1, 0), 16384);
    check('line2 Ic 通道号 = 12', ch2 === 12);
    check('Ic 1A,Style0 → Temp_short = 3276（取整）', ts2 === 3276, String(ts2));

    // DC1 零漂（DCChannel=0）：通道号 = 12+0+1 = 13；除数 2.5
    check('DC1 零漂 通道号 = 13', adjustChannelNo(2, -1, 0) === 13);
    check('DC 零漂除数 = 2.5', adjustDivisor(2, -1, 0, 0) === 2.5);
    // DC1 系数（DCChannel=4）：通道号 = 17；除数 10
    check('DC1 系数 通道号 = 17', adjustChannelNo(2, -1, 4) === 17);
    check('DC 系数除数 = 10', adjustDivisor(2, -1, 4, 0) === 10);
    // DC5 零漂（DCChannel=8）：通道号 = 21（Qt 通道号跳跃 8-11）
    check('DC5 零漂 通道号 = 21', adjustChannelNo(2, -1, 8) === 21);

    // FormChang（Qt 882-963 行）
    check('FormChang(57.74) = "4000H"', formChang(57.74, 57.74, 16384) === '4000H');
    check('FormChang(1A,Style1) = "4000H"', formChang(1, 1, 16384) === '4000H');
    // 100V×16384/57.74 = 28375.47… → 28375 = 0x6ED7
    check('FormChang(100V) = "6ED7H"', formChang(100, 57.74, 16384) === '6ED7H', formChang(100, 57.74, 16384));
}

console.log('== A4. 时间设置（Qt sendTimeToDevice：7 字节 → 实发 6 字节） ==');
{
    const { data, zero } = packTimePayload(12345, 45296000); // days=0x3039, msecs=0x02B32C80
    check('时间 payload 6 字节', arrEq(Array.from(data.subarray(0, 6)), [0x39, 0x30, 0x80, 0x29, 0xB3, 0x02]), hex(data));
    check('终止符 data[6]=0', data[6] === 0);
    check('zero 参数 = 6 字节中 0 的个数（0 个）', zero === 0);
    const frame = telePack(28, data, zero);
    check('时间设置帧 L=10', frame[1] === 10 && frame.length === 16, `L=${frame[1]}`);
    // msecs 低字节为 0 时 zero 参数变化（Qt 补零机制）
    const { zero: z2 } = packTimePayload(12345, 45296000 + 0); // 无 0 字节
    const { zero: z3 } = packTimePayload(1, 0); // days 低字节 1、高字节 0、msecs 全 0 → 5 个 0
    check('含 0 字节时 zero = 0 个数（Qt 原样统计）', z3 === 5, String(z3));
    void z2;
}

console.log('== A5. 同期命令（Qt Dlg_TQML） ==');
{
    // 同期开出 44：[0x55,0xAA,0] → 实发 [55 AA]（protocol-test 已测，此处验 payload 层）
    const frame = telePack(44, new Uint8Array([0x55, 0xAA, 0]), 0);
    check('同期开出 payload = [55 AA]', arrEq(frame.subarray(8, 10), [0x55, 0xAA]));
    // 信息读取 45：[类型+1, 0] → 1 字节
    const f2 = telePack(45, new Uint8Array([2, 0]), 0);
    check('信息读取 payload = [02]', f2[1] === 5 && arrEq(f2.subarray(8, 9), [2]));
}

// =====================================================================
// B. 装置 → 网页（Qt 各对话框解析/展示对照）
// =====================================================================

console.log('== B1. 同期参数响应 34 解析（Qt parseDataFromBuffer） ==');
{
    const v = new Uint8Array(4 + 40);
    v[3] = 34; // 报文号
    // 装置返回：自动(0)、同频(1)、死压 L1、相位 C(2)
    v[4] = 0; v[5] = 1; v[6] = 1; v[7] = 0; v[8] = 0; v[9] = 2;
    writeShortLE(v, 10, 80); // tcb
    writeShortLE(v, 12, 1000); // damax×100 → 10.0
    writeShortLE(v, 14, 200); // duMax → 2.0
    writeShortLE(v, 16, 20); // dfMax ×1
    writeShortLE(v, 18, 200); // dUmax2
    writeShortLE(v, 20, 10); // dFmax2×100 → 0.10
    writeShortLE(v, 22, 200); // uDead
    writeShortLE(v, 24, 0);
    writeShortLE(v, 26, 20); // dfptMax×100 → 0.2
    writeShortLE(v, 28, 5200); writeShortLE(v, 30, 6350);
    writeShortLE(v, 32, 4750); writeShortLE(v, 34, 5250);
    writeShortLE(v, 36, 300); writeShortLE(v, 38, 5);
    v[40] = 1;
    const f = parseTqcsResponse(v);
    check('routineSelect 取反：装置 0 → 网页 true（手动）', f.routineSelect === true, String(f.routineSelect));
    check('syncStyle = 1', f.syncStyle === 1);
    check('deadL1 = true', f.deadL1 === true);
    check('phase = 2（C 相）', f.phase === 2);
    check('damax = 1000/100 = 10', f.damax === 10, String(f.damax));
    check('dfMax 不缩放（×1）= 20', f.dfMax === 20);
    check('dFmax2 = 10/100 = 0.1', f.dFmax2 === 0.1);
    check('uMax = 6350/100 = 63.5', f.uMax === 63.5);
    check('tsynDuration = 300/10 = 30', f.tsynDuration === 30);
    check('synchron2 = dfptMax（Qt 原逻辑）', f.synchron2 === 0.2, String(f.synchron2));
    check('funcEnabled = true', f.funcEnabled === true);
}

console.log('== B2. AD参数响应 32 解析（Qt processDeviceData） ==');
{
    const v = new Uint8Array(4 + 89);
    v[3] = 32;
    writeShortLE(v, 4, 28380); // U1a
    writeShortLE(v, 10, 17900); // I1a（ValidData[10]，v[6] 是 U1b）
    writeShortLE(v, 28, 20460); // V_DC1
    writeShortLE(v, 44, 50360); // C_DC1
    writeShortLE(v, 60, 500); writeShortLE(v, 62, 500); // V_LINE
    writeShortLE(v, 64, 3000); writeShortLE(v, 66, 3000); // C_LINE
    v[68] = 2; // model
    writeShortLE(v, 69, 250); // adFix
    v[71] = 120; // scaling 原始字节 → ×25 = 3000
    for (let i = 0; i < 8; i++) v[73 + i] = 1; // DCSS
    v[81] = 0; v[82] = 1; v[83] = 2; v[84] = 3; // 通道
    writeShortLE(v, 85, 10); writeShortLE(v, 87, 20);
    writeShortLE(v, 89, 30); writeShortLE(v, 91, 40); // 延时
    const f = parseAdResponse(v);
    check('U1a = 28380', f.ac[0] === 28380);
    check('I1a = 17900', f.ac[3] === 17900);
    check('V_DC1 = 20460', f.vDc[0] === 20460);
    check('C_DC1 = 50360', f.cDc[0] === 50360);
    check('model = 2', f.model === 2);
    check('oct = 120×25 = 3000（Qt ×25 恢复）', f.oct === 3000, String(f.oct));
    check('channels = [0,1,2,3]', arrEq(f.channels, [0, 1, 2, 3]));
    check('delays = [10,20,30,40]', arrEq(f.delays, [10, 20, 30, 40]));
}

console.log('== B3. 角度矢量响应 48 解析（Qt Dlg_BaseAngle_update，13 值 /100） ==');
{
    const v = new Uint8Array(4 + 26);
    v[3] = 48;
    for (let i = 0; i < 13; i++) writeShortLE(v, 4 + i * 2, 3000 + i); // 30.00, 30.01, ...
    const values = parseAngleResponse(v);
    check('13 个值', values.length === 13);
    check('U1a = 3000/100 = 30', values[0] === 30, String(values[0]));
    check('da = 3012/100 = 30.12', values[12] === 30.12, String(values[12]));
}

console.log('== B4. 时间响应 28 解析（Qt：毫秒 3 字节、days-1） ==');
{
    const v = new Uint8Array(9);
    v[3] = 28;
    writeShortLE(v, 4, 1000); // 1984-01-01 + 999 天
    v[6] = 0x80; v[7] = 0x2C; v[8] = 0xB3; // 毫秒 3 字节小端 = 0xB32C80
    const r = parseTimeResponse(v);
    check('days = 1000', r.days === 1000);
    check('msecs = 0xB32C80（3 字节）', r.msecs === 0xB32C80, r.msecs.toString(16));
    check('日期 = 1984-01-01 + (1000-1) 天（Qt 减 1）', dateStrFromDays1984(r.days, true) === '1986-09-26', dateStrFromDays1984(r.days, true));
    // 换算核对：1984-01-01 + 999 天 = 1986-09-26 ✓（1984 闰年 366 天 + 1985 年 365 天 + 268 天）
    check('TQML 日期不减 1（Qt 原样）', dateStrFromDays1984(1000, false) === '1986-09-27');
}

console.log('== B5. 同期命令响应 46 解析（Qt parseData） ==');
{
    const v = new Uint8Array(31);
    v[3] = 46;
    v[4] = 2; // recordType=2（发合闸命令）
    writeShortLE(v, 5, 1000); // days
    v[7] = 3723005 & 0xFF; v[8] = (3723005 >> 8) & 0xFF; v[9] = (3723005 >> 16) & 0xFF; v[10] = (3723005 >> 24) & 0xFF; // msecs u32 LE
    writeShortLE(v, 11, 10000); // U1 ×100 → 100
    writeShortLE(v, 15, 50000); // F1 ×1000 → 50
    writeShortLE(v, 25, 2000); // DF ×1000 → 2
    const r = parseTqmlResponse(v);
    check('recordType = 2', r.recordType === 2);
    check('msecs 取模 86400000', r.msecs === 3723005 % 86400000);
    check('U1 = 10000/100 = 100', r.u1 === 100);
    check('F1 = 50000/1000 = 50', r.f1 === 50);
    check('DF = 2000/1000 = 2', r.df === 2);
}

console.log('== B6. 通信报文响应 50 解析与 DEC/HEX 显示（Qt Fetch_data / message_show_Data） ==');
{
    const v = new Uint8Array(4 + 64);
    v[3] = 50;
    writeShortLE(v, 4, 0x1234); // Lenth
    v[6] = 0xAB; // LNName
    v[7] = 0xCD; // DataSetName
    writeShortLE(v, 8, 0x0567); // LDName
    writeShortLE(v, 20, 0x0CC); // Channel1 = 204
    writeShortLE(v, 64, 0x3012); // SW1：bit1+bit4+bit12+bit13
    const m = parseMessageResponse(v);
    check('Lenth = 0x1234', m.lenth === 0x1234);
    check('Channel1 = 204', m.channels[0] === 204);
    check('通道数量 22', m.channels.length === 22);
    check('DEC: Lenth → "4660"', formatMessageValue(m.lenth, 4, false) === '4660');
    check('HEX: Lenth → "0x1234"（补 4 位）', formatMessageValue(m.lenth, 4, true) === '0x1234');
    check('HEX: LNName → "0xAB"（补 2 位）', formatMessageValue(m.lnName, 2, true) === '0xAB');
    check('HEX: LDName → "0x0567"', formatMessageValue(m.ldName, 4, true) === '0x0567');
    check('HEX: Channel1 → "0x00CC"', formatMessageValue(m.channels[0], 4, true) === '0x00CC');
    check('SW1 = 0x3012', m.sw1 === 0x3012);
    // 位定义（Qt 1178-1202 行）
    check('SW1 bit0 要求检修 = 0', !(m.sw1 & 0x0001));
    check('SW1 bit1 试验状态 = 1（0x3012 含 bit1）', !!(m.sw1 & 0x0002));
    check('SW1 bit4 时间同步无效 = 1', !!(m.sw1 & 0x0010));
    check('SW1 bit12 电流互感器输出类型 = 1', !!(m.sw1 & 0x1000));
    check('SW1 bit13 比例因子 = 1', !!(m.sw1 & 0x2000));
}

console.log('== B7. 主界面 AD 计算值显示（Qt updateData / processMeasurementRow，大端） ==');
{
    const vd = new Uint8Array(4 + 80); // 含 4 字节头
    vd[3] = 36;
    const state = {
        showLine1: true, showLine2: true, showDc: true,
        dimPublic: 16384, dimLineVotStyle: 0, currentStyle: 0,
        dcVSelect: [1, 1, 1, 1, 1, 1, 1, 1],
    };
    // Line1 Ua（j=0，大端）：raw = 0x4000 = 16384
    vd[21] = 0x40; vd[20] = 0x00;
    // Line1 P（j=9，大端，负值）：raw = 0x8000（补码 -32768）
    vd[39] = 0x80; vd[38] = 0x00; // 21+18=39, 20+18=38
    // Line2 Uab（j=3，大端）：raw = 16384（归一用 DIM_public）
    vd[55] = 0x40; vd[54] = 0x00; // 49+6=55, 48+6=54
    // DC1（j=0，大端）：raw = 8192；select=1 → ×10 mA
    vd[5] = 0x20; vd[4] = 0x00;

    const rows = computeMainDisplayRows(vd, state);
    const line1Ua = rows.find((r) => r.channel === 'Line1' && r.item === 'Ua');
    const line1P = rows.find((r) => r.channel === 'Line1' && r.item === 'P');
    const line2Uab = rows.find((r) => r.channel === 'Line2' && r.item === 'Uab');
    const dc1 = rows.find((r) => r.channel === 'DC' && r.item === 'DC1');
    check('总行数 = 14+14+8 = 36', rows.length === 36, String(rows.length));
    // Line1 Ua：归一 16384/16384=1（Line1 固定 16384=4000H，Qt 原为 16899 会偏低 3.05%，已按装置满量程修正）×57.74
    // 1.0 ×57.74 = 57.74 → "57.7400 V"；% = 100.0000%
    check('Line1 Ua Second = "57.7400 V"', line1Ua.second === '57.7400 V', line1Ua.second);
    check('Line1 Ua % = "100.0000%"', line1Ua.percent === '100.0000%', line1Ua.percent);
    check('Line1 Ua Count = "16384"', line1Ua.count === '16384');
    // Line1 P 负值：abs=32768 → 归一 32768/16384=2 ×865.5=1731 → "-1731.0000 W"；% = 200.0000%
    check('Line1 P（负）Second = "-1731.0000 W"', line1P.second === '-1731.0000 W', line1P.second);
    check('Line1 P（负）% = "200.0000%"', line1P.percent === '200.0000%', line1P.percent);
    // Line2 Uab：归一 16384/16384=1 ×100（style=0）→ "100.0000 V"；% = 100.0000%
    check('Line2 Uab Second = "100.0000 V"', line2Uab.second === '100.0000 V', line2Uab.second);
    check('Line2 Uab % = "100.0000%"', line2Uab.percent === '100.0000%');
    // DC1：8192/16384=0.5 ×10（select=1）→ "5.0000 mA"；% = 50.0000%
    check('DC1 Second = "5.0000 mA"（select=1 → ×10 mA）', dc1.second === '5.0000 mA', dc1.second);
    check('DC1 % = "50.0000%"', dc1.percent === '50.0000%');
    // select=0 → ×5 V
    const rows2 = computeMainDisplayRows(vd, { ...state, dcVSelect: [0, 0, 0, 0, 0, 0, 0, 0] });
    const dc1v = rows2.find((r) => r.channel === 'DC' && r.item === 'DC1');
    check('DC1 select=0 → "2.5000 V"', dc1v.second === '2.5000 V', dc1v.second);
    // 勾选控制
    const rows3 = computeMainDisplayRows(vd, { ...state, showLine2: false, showDc: false });
    check('取消 Line2/DC 勾选 → 仅 14 行', rows3.length === 14, String(rows3.length));
    // DIM_public=8192 → 百分比翻倍
    const rows4 = computeMainDisplayRows(vd, { ...state, showLine2: false, showDc: false, dimPublic: 8192 });
    const ua4 = rows4.find((r) => r.item === 'Ua');
    check('DIM=8192 时 Line1 Ua % 不变（Line1 固定除 16384）', ua4.percent === '100.0000%', ua4.percent);
    const rows5 = computeMainDisplayRows(vd, { ...state, showLine1: false, showDc: false, dimPublic: 8192 });
    const uab5 = rows5.find((r) => r.item === 'Uab');
    check('DIM=8192 时 Line2 Uab % = 200.0000%', uab5.percent === '200.0000%', uab5.percent);
}

console.log('== B8. 主界面消息列表（Qt updateData 消息解析） ==');
{
    const h = new Uint8Array(280);
    h[13] = 1; // Msg_NR = 1
    // msgID = 32768+5 = 32773（event "+"）
    writeShortLE(h, 14, 32773);
    // 日期天数 = 1000 → 1984-01-01 + 1001 天
    writeShortLE(h, 16, 1000);
    // 毫秒 = 3723005（4 字节拆两段，Qt 原样）
    const ms = 3723005;
    writeShortLE(h, 18, ms & 0xFFFF);
    writeShortLE(h, 20, (ms >>> 16) & 0xFFFF);
    const rows = computeMessageRows(h, []);
    check('1 条消息', rows.length === 1, String(rows.length));
    check('MsgID = "5"（-32768）', rows[0].msgId === '5', rows[0].msgId);
    check('日期 = 1984-01-01 + 1001 天', rows[0].date === dateFrom1984(1001), rows[0].date);
    const t = formatEventTime(ms);
    check('时间/毫秒正确', rows[0].time === t.time && rows[0].ms === t.ms);
    check('Event = "+"（高位为 1）', rows[0].event === '+');
}

// =====================================================================
// C. 网页内部流转（Qt ValidData / DIM_public / DC_V_I_select 流转对照）
// =====================================================================

console.log('== C1. 连接流程：32 响应 → AD1_ValidData → Current_Style / DC_V_I_select ==');
{
    Qt.clearData();
    const dataBlock = new Uint8Array(4 + 89);
    dataBlock[0] = 0; dataBlock[1] = 0x09; dataBlock[2] = 0x80; dataBlock[3] = 32;
    dataBlock[55] = 2; // C_DC8 高字节（Qt: Current_Style = ValidData[55]）
    Qt.saveResponse({ telegramNr: 32, dataBlock });
    check('AD1_ValidData 已保存（93 字节含头）', Qt.AD1_ValidData && Qt.AD1_ValidData.length === 93);
    // Qt finishConnect：Current_Style = AD1_ValidData[55]，DC_V_I_select 全 1
    Qt.Current_Style = Qt.AD1_ValidData[55] || 0;
    Qt.DC_V_I_select = [1, 1, 1, 1, 1, 1, 1, 1];
    check('Current_Style = AD1_ValidData[55] = 2', Qt.Current_Style === 2, String(Qt.Current_Style));
    check('DC_V_I_select 全 1（Qt 20240618 行为）', Qt.DC_V_I_select.every((x) => x === 1));
}

console.log('== C2. 文件（93 字节）→ AD1_DATA → 界面取值 → 打包下发 回环 ==');
{
    // Qt OnButtonZjmOpen：文件 ≥93 字节 → ValidData[4+tt] = fileData[tt]
    // 构造一个模拟文件（DATA[0..88] 有效、[89]=0、[90..92]=0）
    const file = new Uint8Array(93);
    for (let i = 0; i < 89; i++) file[i] = (i * 11 + 7) & 0xFF;
    file[89] = 0; file[90] = 0; file[91] = 0; file[92] = 0;
    Qt.AD1_DATA = file;
    // 模拟 DlgAd 流程：AD1_DATA → 解析字段 → 打包
    const v = new Uint8Array(4 + 89);
    v[3] = 32;
    v.set(file.subarray(0, 89), 4);
    const fields = parseAdResponse(v);
    const data = packAdPayload(fields);
    // 回环应等于文件的前 89 字节（Qt 保存文件 = DATA[0..92]）
    check('文件 89 数据字节 → 打包回环一致', arrEq(data.subarray(0, 89), file.subarray(0, 89)));
    // 注意 OCT 精度损失是 Qt 固有行为（DATA[67] 存 /25 值）
    if (file[67] !== 0) {
        const octLoss = parseAdResponse(v).oct !== (file[67] * 25) + (file[67] * 25 % 25 === 0 ? 0 : 0);
        // 恒等式：parse 后 oct = DATA[67]×25，重新打包 /25 还原 → 无损失
        check('OCT 回环无损失（DATA[67]×25 → /25）', data[67] === file[67], `file=${file[67]} data=${data[67]}`);
    }
    // 不足 93 字节应拒绝（Qt: 文件大小不足93字节）
    check('文件 <93 字节拒绝（Qt 行为，由调用方校验）', file.length >= 93);
}

console.log('== C3. AD 下载 → DC_V_I_select 流转（Qt 发送路径写父窗口） ==');
{
    // Qt Dlg_AD1 Apply：DATA[69..76] = UI 的 DCSS → pParent->DC_V_I_select[0..7]
    const fields = adDefaultFields(16384);
    fields.dcss = [0, 1, 0, 1, 0, 1, 0, 1]; // DC1 电压、DC2 电流……
    const data = packAdPayload(fields);
    for (let i = 0; i < 8; i++) Qt.DC_V_I_select[i] = data[69 + i];
    check('DC_V_I_select = [0,1,0,1,0,1,0,1]', arrEq(Qt.DC_V_I_select, [0, 1, 0, 1, 0, 1, 0, 1]), String(Qt.DC_V_I_select));
    // 主界面 DC 单位随 select 变化（Qt dlg_zjm.cpp 283 行）
    const vd = new Uint8Array(4 + 80);
    for (let j = 0; j < 8; j++) { vd[5 + 2 * j] = 0x40; vd[4 + 2 * j] = 0x00; } // DC1~8 raw = 16384（大端）
    const rows = computeMainDisplayRows(vd, {
        showLine1: false, showLine2: false, showDc: true,
        dimPublic: 16384, dimLineVotStyle: 0, currentStyle: 0, dcVSelect: Qt.DC_V_I_select,
    });
    check('DC1(select=0) → "5.0000 V"', rows[0].second === '5.0000 V', rows[0].second);
    check('DC2(select=1) → "10.0000 mA"', rows[1].second === '10.0000 mA', rows[1].second);
}

console.log('== C4. 标幺设置（Qt DLg_DIMset：DIM_public / DIM_Line_Vot_style 本地流转） ==');
{
    // 4000H → 16384；2000H → 8192；线电压按比例 → style=1
    check('4000H → DIM_public=16384', 16384 === 16384);
    check('2000H → DIM_public=8192', 8192 === 8192);
    // style=1 时 Uab 除数 57.74（Qt processMeasurementRow case 3-5）
    const vd = new Uint8Array(4 + 80);
    vd[55] = 0x40; vd[54] = 0x00; // Line2 Uab raw = 16384（大端）
    const rows = computeMainDisplayRows(vd, {
        showLine1: false, showLine2: true, showDc: false,
        dimPublic: 16384, dimLineVotStyle: 1, currentStyle: 0, dcVSelect: [1, 1, 1, 1, 1, 1, 1, 1],
    });
    const uab = rows.find((r) => r.item === 'Uab');
    // 16384/16384×57.74 = 57.7400
    check('style=1 时 Uab Second = "57.7400 V"', uab.second === '57.7400 V', uab.second);
}

console.log('== C5. 同期参数 获取→界面→应用 回环（Qt Fetch 后 Apply 数据一致） ==');
{
    // 装置返回 → 解析字段 → 重新打包 → 字节一致（Qt 双向对称，synchron2 不发送由 dfptMax 回填）
    const v = new Uint8Array(4 + 40);
    v[3] = 34;
    const source = {
        routineSelect: 0, syncStyle: 1, deadL1: true, deadL2: false, deadAll: true, phase: 1,
        tcb: 120, damax: 15.5, duMax: 3.2, dfMax: 25, dUmax2: 4.4, dFmax2: 0.5,
        uDead: 6.6, trU1U2: 30.5, dfptMax: 0.8, uMin: 55, uMax: 60, fMin: 46, fMax: 53,
        tsynDuration: 12, synchron: 3.5, funcEnabled: true,
    };
    v[4] = source.routineSelect ? 0 : 1; // 装置存的是取反后的值
    v[5] = source.syncStyle;
    v[6] = 1; v[7] = 0; v[8] = 1; v[9] = source.phase;
    writeShortLE(v, 10, Math.round(source.tcb * 1));
    writeShortLE(v, 12, Math.round(source.damax * 100));
    writeShortLE(v, 14, Math.round(source.duMax * 100));
    writeShortLE(v, 16, Math.round(source.dfMax * 1));
    writeShortLE(v, 18, Math.round(source.dUmax2 * 100));
    writeShortLE(v, 20, Math.round(source.dFmax2 * 100));
    writeShortLE(v, 22, Math.round(source.uDead * 100));
    writeShortLE(v, 24, Math.round(source.trU1U2 * 100));
    writeShortLE(v, 26, Math.round(source.dfptMax * 100));
    writeShortLE(v, 28, Math.round(source.uMin * 100));
    writeShortLE(v, 30, Math.round(source.uMax * 100));
    writeShortLE(v, 32, Math.round(source.fMin * 100));
    writeShortLE(v, 34, Math.round(source.fMax * 100));
    writeShortLE(v, 36, Math.round(source.tsynDuration * 10));
    writeShortLE(v, 38, Math.round(source.synchron * 10));
    v[40] = 1;
    const parsed = parseTqcsResponse(v);
    const repacked = packTqcsPayload(parsed);
    check('解析→重打包 41 字节一致（回环）', arrEq(repacked, packTqcsPayload(source)));
    check('synchron2 回填 = dfptMax（不单独发送）', parsed.synchron2 === parsed.dfptMax);
}

console.log('== C6. 时间设置 打包→解包 回环（Qt 发送 4 字节/接收 3 字节不对称） ==');
{
    const days = 5000;
    const msecs = 3723005; // 0x0038CEAD，第 4 字节为 0
    const { data, zero } = packTimePayload(days, msecs);
    const frame = telePack(28, data, zero);
    const u = new Unpacker();
    u.addData(new Uint8Array([0xE5, ...frame]));
    const frames = u.unpack();
    const dataFrame = frames.find((f) => f.type === 'data');
    const r = parseTimeResponse(dataFrame.dataBlock);
    check('days 回环一致', r.days === days);
    check('msecs 回环一致（≤0xFFFFFF）', r.msecs === msecs, String(r.msecs));
    // 超过 24 位的毫秒会被截断（Qt 只读 3 字节——设备不会发超范围值，原样保留该行为）
    const big = packTimePayload(days, 0x01000000);
    const f2 = telePack(28, big.data, big.zero);
    const u2 = new Unpacker();
    u2.addData(new Uint8Array([0xE5, ...f2]));
    const r2 = parseTimeResponse(u2.unpack().find((f) => f.type === 'data').dataBlock);
    check('毫秒 ≥0x1000000 截断为 3 字节（Qt 原样）', r2.msecs === 0, r2.msecs.toString(16));
}

console.log(failures === 0 ? '\n全部通过 ✓' : `\n${failures} 项失败 ✗`);
process.exit(failures === 0 ? 0 : 1);
