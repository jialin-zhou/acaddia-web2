/**
 * 通信状态机模拟测试（node test/state-machine-test.js）
 * 用 mock 驱动 SerialManager 的连接/获取链路，验证发出的字节序列与 Qt new1Dlg 状态机一致
 */
const fs = require('fs');

// ---- DOM mock ----
const mockElement = () => ({
    className: '', innerHTML: '', textContent: '', value: '', style: {},
    querySelector: () => ({ addEventListener() {} }),
    addEventListener() {}, appendChild() {}, remove() {}, setAttribute() {},
});
global.document = {
    getElementById: () => mockElement(),
    createElement: () => mockElement(),
    body: mockElement(),
};
global.window = global;
global.navigator = { serial: null };
global.bootstrap = { Modal: { getOrCreateInstance: () => ({ show() {} }), getInstance: () => null } };

// ---- 载入模块 ----
// 先加载日志模块（protocol.js/store.js/serial.js 会调用 Logger）
eval(fs.readFileSync(__dirname + '/../js/log.js', 'utf8'));
eval(fs.readFileSync(__dirname + '/../js/protocol.js', 'utf8') +
    '\n;global.telePack = telePack; global.Unpacker = Unpacker; global.readShortLE = readShortLE; global.packAck = packAck;');
eval(fs.readFileSync(__dirname + '/../js/store.js', 'utf8') +
    '\n;global.Qt = Qt; global.Bus = Bus;');
eval(fs.readFileSync(__dirname + '/../js/serial.js', 'utf8') +
    '\n;global.SerialManager = SerialManager;');

let failures = 0;
function check(name, cond, detail) {
    if (cond) console.log(`  PASS  ${name}`);
    else { failures++; console.log(`  FAIL  ${name}${detail ? ' — ' + detail : ''}`); }
}
function hex(arr) { return Array.from(arr).map((b) => b.toString(16).padStart(2, '0').toUpperCase()).join(' '); }

// 截获写出的字节
const sent = [];
SerialManager.write = async function (data) {
    sent.push(new Uint8Array(data));
};

// 构造设备响应：E5 + 数据帧
function deviceResponse(nr, payload) {
    const frame = telePack(nr, new Uint8Array([...payload, 0]), 0);
    return new Uint8Array([0xE5, ...frame]);
}

// 事件记录
const events = [];
['connect-done', 'fetch-done'].forEach((ev) => Bus.on(ev, () => events.push(ev)));

console.log('== 连接序列（Down=1001：35→34, 33→32, 37→36, 47→48）==');
(async () => {
    Qt.clearData();
    await SerialManager.startConnect();
    check('第 1 条命令 = 35 号', sent.length === 1 && hex(sent[0]) === '68 04 04 68 00 09 80 23 AC 16', hex(sent[0] || []));

    // 34 响应（payload 55 字节，让 dataBlock[55] 可用于 Current_Style 检查）
    const p34 = new Uint8Array(55); p34[51] = 2;
    await SerialManager.processBatch(deviceResponse(34, p34));
    check('收到 34 → 回 A2', hex(sent[1]) === 'A2', hex(sent[1] || []));
    check('收到 34 → 发 33 号', sent.length === 3 && hex(sent[2]) === '68 04 04 68 00 09 80 21 AA 16', hex(sent[2] || []));

    // 32 响应（payload 89 字节）
    const p32 = new Uint8Array(89);
    for (let i = 0; i < 89; i++) p32[i] = (i * 5 + 3) & 0xFF;
    await SerialManager.processBatch(deviceResponse(32, p32));
    check('收到 32 → 发 37 号', sent.length === 5 && hex(sent[4]) === '68 04 04 68 00 09 80 25 AE 16', hex(sent[4] || []));
    check('AD1 数据已保存（含 4 字节头，长度 93）', Qt.AD1_ValidData && Qt.AD1_ValidData.length === 93);

    // 36 响应
    const p36 = new Uint8Array(80);
    await SerialManager.processBatch(deviceResponse(36, p36));
    check('收到 36 → 发 47 号', sent.length === 7 && hex(sent[6]) === '68 04 04 68 00 09 80 2F B8 16', hex(sent[6] || []));

    // 48 响应 → 连接完成
    const p48 = new Uint8Array(26);
    await SerialManager.processBatch(deviceResponse(48, p48));
    check('收到 48 → 连接完成（connect-done）', events.includes('connect-done'));
    check('Current_Style = AD1_ValidData[55]', Qt.Current_Style === 2, `Current_Style=${Qt.Current_Style}`);
    check('DC_V_I_select 全 1', Qt.DC_V_I_select.every((v) => v === 1));

    console.log('== 主界面获取序列（Down=1005：37→36, 47→48, 47→48, 49→50）==');
    sent.length = 0;
    events.length = 0;
    await SerialManager.startFetch();
    check('第 1 条命令 = 37 号', sent.length === 1 && hex(sent[0]) === '68 04 04 68 00 09 80 25 AE 16', hex(sent[0] || []));

    await SerialManager.processBatch(deviceResponse(36, p36));
    check('收到 36 → 发 47 号', sent.length === 3 && hex(sent[2]) === '68 04 04 68 00 09 80 2F B8 16', hex(sent[2] || []));

    await SerialManager.processBatch(deviceResponse(48, p48));
    check('第一次 48 → 再发 47 号', sent.length === 5 && hex(sent[4]) === '68 04 04 68 00 09 80 2F B8 16', hex(sent[4] || []));

    await SerialManager.processBatch(deviceResponse(48, p48));
    check('第二次 48 → 发 49 号', sent.length === 7 && hex(sent[6]) === '68 04 04 68 00 09 80 31 BA 16', hex(sent[6] || []));

    const p50 = new Uint8Array(64);
    await SerialManager.processBatch(deviceResponse(50, p50));
    check('收到 50 → 获取完成（fetch-done）', events.includes('fetch-done'));

    console.log('== 连接期裸 E5（无后续帧）→ 错误计数并重发 35 ==');
    sent.length = 0;
    Qt.clearData();
    await SerialManager.startConnect();
    sent.length = 0;
    await SerialManager.processBatch(new Uint8Array([0xE5]));
    check('ErrorCounter=1', Qt.ErrorCounter === 1);
    check('重发 35 号', sent.length === 1 && hex(sent[0]) === '68 04 04 68 00 09 80 23 AC 16', hex(sent[0] || []));

    console.log('== 连接期未知报文号 → 错误计数并重发 35 ==');
    sent.length = 0;
    await SerialManager.processBatch(deviceResponse(99, new Uint8Array([1, 2, 3])));
    check('ErrorCounter 累加', Qt.ErrorCounter === 2);
    check('未知帧不回 A2（直接重发 35）', sent.length === 1 && hex(sent[0]) === '68 04 04 68 00 09 80 23 AC 16', hex(sent[0] || []));

    console.log('== 获取流程：49 后收到裸 E5 → 设备无数据帧可回、获取结束（对照 Qt：链路自然结束 + 999ms 刷新界面） ==');
    sent.length = 0;
    events.length = 0;
    Qt.clearData();
    // 复位状态机标志（clearData 不重置 connecting/fetching，前一组连接测试会遗留 connecting=true）
    Qt.connecting = false;
    Qt.fetching = false;
    await SerialManager.startFetch();
    await SerialManager.processBatch(deviceResponse(36, p36));
    await SerialManager.processBatch(deviceResponse(48, p48));
    await SerialManager.processBatch(deviceResponse(48, p48));
    check('第二次 48 → 发 49 号', sent.length === 7 && hex(sent[6]) === '68 04 04 68 00 09 80 31 BA 16', hex(sent[6] || []));
    await SerialManager.processBatch(new Uint8Array([0xE5]));
    check('E5 → 获取完成（fetch-done）', events.includes('fetch-done'));
    check('E5 后不再发送新命令（不再重发 37）', sent.length === 7, `实际发送 ${sent.length} 次`);
    check('fetching 已复位', Qt.fetching === false);
    check('RX_Msg_End=true', Qt.RX_Msg_End === true);

    console.log('== 获取流程：3 号告警列表帧（未完 → 续发 47 继续读，读完 → 结束） ==');
    sent.length = 0;
    events.length = 0;
    Qt.clearData();
    Qt.connecting = false;
    Qt.fetching = false;
    await SerialManager.startFetch();
    await SerialManager.processBatch(deviceResponse(36, p36));
    await SerialManager.processBatch(deviceResponse(48, p48)); // idx2 → 发 47
    sent.length = 0;
    await SerialManager.processBatch(deviceResponse(3, new Uint8Array([5, 1, 2])));
    check('3 号帧未完 → 续发 47（Qt 继续读列表）', sent.length === 2 && hex(sent[1]) === '68 04 04 68 00 09 80 2F B8 16', hex(sent[1] || []));
    // 手工构造 payload 首字节为 0 的 3 号帧（dataBlock[4]==0 → 读完）。
    // 不能用 deviceResponse：telePack 的补零压缩会把首 0 吞掉导致 dataBlock 无 payload。
    // L=5（4 字节头 + 1 字节 payload），校验和 = 00+09+80+03+00 = 0x8C
    await SerialManager.processBatch(new Uint8Array([0xE5, 0x68, 0x05, 0x05, 0x68, 0x00, 0x09, 0x80, 0x03, 0x00, 0x8C, 0x16]));
    check('3 号帧读完 → 获取完成（fetch-done）', events.includes('fetch-done'));

    console.log('== 获取流程：设备无响应 → 超时重试计错误，5 次后报通信错误（不再无限循环） ==');
    sent.length = 0;
    events.length = 0;
    Qt.clearData();
    Qt.connecting = false;
    Qt.fetching = false;
    const savedTimeout = SerialManager.COMM_TIMEOUT;
    SerialManager.COMM_TIMEOUT = 20;
    await SerialManager.startFetch();
    await new Promise((r) => setTimeout(r, 30 * 5 + 150));
    const sent37 = sent.filter((x) => hex(x) === '68 04 04 68 00 09 80 25 AE 16').length;
    check('超时重发 37 共 5 次', sent37 === 5, `实际 ${sent37} 次`);
    check('错误计数达到 5', Qt.ErrorCounter === 5, `ErrorCounter=${Qt.ErrorCounter}`);
    check('获取已停止（fetching=false）', Qt.fetching === false);
    SerialManager.COMM_TIMEOUT = savedTimeout;

    console.log('== 通道校准：39 号帧登记 + 校准命令按 39 号帧成功解析 ==');
    Qt.connecting = false;
    Qt.fetching = false;
    Qt.serial.connected = true;
    sent.length = 0;
    await SerialManager.processBatch(deviceResponse(39, new Uint8Array([0x16, 0x55])));
    check('39 号帧被登记（回 A2，不报未知帧）', sent.length === 1 && hex(sent[0]) === 'A2', hex(sent[0] || []));
    check('39 号帧存入 ADAdjust_ValidData', Qt.ADAdjust_ValidData && Qt.ADAdjust_ValidData[4] === 0x16 && Qt.ADAdjust_ValidData[5] === 0x55);

    let calResult = null;
    SerialManager.sendCommand({ nr: 38, data: new Uint8Array([0x16, 0, 0, 0]), zero: 0, expectNrs: [39, 41], expectE5: true, down: 1090 })
        .then((r) => { calResult = r; }, (e) => { calResult = 'rejected:' + e.message; });
    await SerialManager.processBatch(deviceResponse(39, new Uint8Array([0x16, 0x55])));
    check('校准命令收到 39 号帧 → 成功解析', calResult && calResult.type === 'data' && calResult.payload[1] === 0x55, String(calResult && calResult.type));

    console.log('== sendCommand：残留 timer 不误杀后续命令 ==');
    Qt.connecting = false;
    Qt.fetching = false;
    Qt.serial.connected = true;
    let aDone = null, bDone = null;
    // A 的超时 80ms（快速完成后应清除其 timer）；B 的超时 400ms（远晚于 A 的 80ms 点）
    SerialManager.sendCommand({ nr: 33, expectNr: 32, down: 1030, timeout: 80 })
        .then(() => { aDone = true; }, () => { aDone = false; });
    await SerialManager.processBatch(deviceResponse(32, new Uint8Array(89)));
    check('命令 A 正常完成', aDone === true, `aDone=${aDone}`);
    // A 完成后立即发 B；若 A 的残留 timer 未清除，会在 t≈80ms 时误杀 B（B 自身超时 400ms）
    SerialManager.sendCommand({ nr: 33, expectNr: 32, down: 1030, timeout: 400 })
        .then(() => { bDone = true; }, () => { bDone = false; });
    await new Promise((r) => setTimeout(r, 200));
    check('超过 A 的超时点后 B 仍待决（未被残留 timer 误杀）', SerialManager.pendingCommand !== null && SerialManager.pendingCommand.nr === 33, `pending=${SerialManager.pendingCommand && SerialManager.pendingCommand.nr}`);
    await SerialManager.processBatch(deviceResponse(32, new Uint8Array(89)));
    check('命令 B 正常完成', bDone === true, `bDone=${bDone}`);

    console.log(failures === 0 ? '\n全部通过 ✓' : `\n${failures} 项失败 ✗`);
    process.exit(failures === 0 ? 0 : 1);
})().catch((e) => { console.error(e); process.exit(1); });
