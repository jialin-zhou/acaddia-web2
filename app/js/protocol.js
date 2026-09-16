/**
 * ACAD DIA 通信协议模块
 * 逐行移植自 Qt 版 TeleProcess.cpp（D:\...\acaddia-web2\public\pre-acaddia\cpp\TeleProcess.cpp）
 *
 * 帧格式（主站发送）:
 *   68 L L 68 | 站址(0) | F1(0x09) | F2(0x80) | 报文号 | 数据... | 校验和 | 16
 *   - L = 4 + 数据长度（数据块 = 站址+F1+F2+报文号+数据）
 *   - 校验和 = 数据块各字节累加和，取低 8 位
 *   - F1 = (Msg_cntr << 5) | 0x09，Qt 中 Msg_cntr 恒为 0，故 F1 恒为 0x09
 *
 * 从站响应:
 *   - 数据帧: E5 68 L L 68 站址 F1 F2 报文号 数据... 校验和 16（E5 前缀 + 完整帧）
 *   - 短应答: 单字节 E5
 *   - 主站收到数据帧后回单字节 A2
 */

const HEADER = 0x68;
const FOOTER = 0x16;
const ACK_SLAVE_TO_MASTER = 0xE5; // 从站→主站短应答
const ACK_MASTER_TO_SLAVE = 0xA2; // 主站→从站短应答
const F1_FIELD = 0x09; // (Msg_cntr << 5) | 0x09，Msg_cntr 恒为 0
const F2_FIELD = 0x80;

/**
 * 组帧。与 Qt TeleProcess::TelePack 算法完全一致：
 * - 从数据数组依次拷贝字节，直到遇到 0x00
 * - 每遇到一个 0x00：zero 计数减一，若仍 >=0 则在输出同位置补一个 0x00 并继续拷贝后续字节；
 *   否则终止（该 0 及之后的内容丢弃）
 * - 校验和只累加 while 循环里拷贝的字节（0x00 加不加无影响）
 *
 * @param {number} telegramNr 报文号
 * @param {number[]|Uint8Array} data 数据区（Qt 中为 ValidDataToPack / TelegramBlock.Data，
 *        必须含 0x00 终止符，与 Qt 用法一致）
 * @param {number} zero 补零参数（Qt 中为 zero_NR）
 * @param {number} [stationAddr=0] 站地址
 * @param {number} [f2=0x80] F2 字段
 * @returns {Uint8Array} 完整帧
 */
function telePack(telegramNr, data, zero, stationAddr = 0, f2 = F2_FIELD) {
    const F1 = F1_FIELD;
    let crc = (stationAddr + telegramNr + F1 + f2) & 0xFF;
    const src = data || [];
    const out = [];
    let idx = 0;
    for (;;) {
        while (idx < src.length && src[idx] !== 0) {
            crc = (crc + src[idx]) & 0xFF;
            out.push(src[idx]);
            idx++;
        }
        zero--;
        if (zero >= 0) {
            out.push(0);
            idx++; // 跳过源 0，继续拷贝其后的字节
            continue;
        }
        break;
    }
    const L = 4 + out.length;
    const frame = new Uint8Array(L + 6);
    frame[0] = HEADER; // 0x68
    frame[1] = L;
    frame[2] = L;
    frame[3] = HEADER; // 0x68
    frame[4] = stationAddr;
    frame[5] = F1;
    frame[6] = f2;
    frame[7] = telegramNr;
    frame.set(out, 8);
    frame[8 + out.length] = crc;
    frame[9 + out.length] = FOOTER; // 0x16
    return frame;
}

/**
 * 生成主站→从站的单字节应答 0xA2
 * （Qt: TXDATA1[0]=0xa2; TXDATA1[1]='\0'; SentData 只发 1 字节）
 * @returns {Uint8Array} [0xA2]
 */
function packAck() {
    return new Uint8Array([ACK_MASTER_TO_SLAVE]);
}

/**
 * 流式解帧器。
 * 接收侧处理参考 Vue 版 acadia-protocol.js 的 Unpacker（流式缓冲），
 * 语义与 Qt TeleUnpack 对齐：
 * - E5 → { type:'ack_e5' }
 * - 68 L L 68 ... 校验和 16 → { type:'data', dataBlock(含4字节头), payload, telegramNr, raw }
 * - 其他字节 → { type:'junk' }
 * （Qt 要求数据帧带 E5 前缀且一次只解析一帧；本实现把 E5 与帧分开输出、
 *   支持一缓冲多帧，行为等价且更健壮）
 */
class Unpacker {
    constructor() {
        this.buffer = new Uint8Array(0);
    }

    addData(newData) {
        const b = new Uint8Array(this.buffer.length + newData.length);
        b.set(this.buffer);
        b.set(newData, this.buffer.length);
        this.buffer = b;
    }

    unpack() {
        const frames = [];
        let consumed = 0;
        let pos = 0;
        const buf = this.buffer;

        while (pos < buf.length) {
            const b0 = buf[pos];

            if (b0 === ACK_SLAVE_TO_MASTER) {
                if (pos > consumed) {
                    frames.push({ type: 'junk', raw: buf.subarray(consumed, pos) });
                }
                frames.push({ type: 'ack_e5', raw: buf.subarray(pos, pos + 1) });
                pos++;
                consumed = pos;
                continue;
            }

            // 0xA2（主站→从站应答回显）：Qt TeleUnpack 置 ACK2
            if (b0 === ACK_MASTER_TO_SLAVE) {
                if (pos > consumed) {
                    frames.push({ type: 'junk', raw: buf.subarray(consumed, pos) });
                }
                frames.push({ type: 'ack_a2', raw: buf.subarray(pos, pos + 1) });
                pos++;
                consumed = pos;
                continue;
            }

            if (b0 === HEADER) {
                // 需要至少 4 字节确认头部 68 L L 68
                if (buf.length < pos + 4) break;
                if (buf[pos + 3] !== HEADER) { pos++; continue; }
                const L = buf[pos + 1];
                if (L !== buf[pos + 2]) { pos++; continue; }
                const frameLen = 4 + L + 2; // 头4 + 数据块L + 校验和 + 帧尾
                if (buf.length < pos + frameLen) break;

                const frame = buf.subarray(pos, pos + frameLen);
                if (frame[frame.length - 1] !== FOOTER) { pos++; continue; }

                const dataBlock = frame.subarray(4, 4 + L);
                const receivedCs = frame[frame.length - 2];
                let calc = 0;
                for (let i = 0; i < dataBlock.length; i++) calc = (calc + dataBlock[i]) & 0xFF;

                if (calc === receivedCs) {
                    if (pos > consumed) {
                        frames.push({ type: 'junk', raw: buf.subarray(consumed, pos) });
                    }
                    frames.push({
                        type: 'data',
                        stationAddr: dataBlock[0],
                        funcCode1: dataBlock[1],
                        funcCode2: dataBlock[2],
                        telegramNr: dataBlock[3],
                        dataBlock: dataBlock.slice(), // 含 4 字节头，对应 Qt Telegarm_Array/ValidData
                        payload: dataBlock.subarray(4),
                        raw: frame.slice(),
                    });
                    pos += frameLen;
                    consumed = pos;
                    continue;
                }
                // 校验失败：跳过这个 68 继续搜
                Logger.warn('protocol', '数据帧校验和不符，跳过该 68 继续搜索', {
                    nr: dataBlock[3],
                    calc: '0x' + calc.toString(16),
                    received: '0x' + receivedCs.toString(16),
                    hex: Logger.hex(frame),
                });
                pos++;
                continue;
            }

            // 既不是 E5 也不是 68：junk 字节，继续搜
            pos++;
        }

        if (pos > consumed && pos === buf.length) {
            frames.push({ type: 'junk', raw: buf.subarray(consumed, pos) });
            consumed = pos;
        }
        if (consumed > 0) {
            this.buffer = buf.slice(consumed);
        }
        return frames;
    }

    reset() {
        this.buffer = new Uint8Array(0);
    }
}

/**
 * 小端读取工具（与 Qt 各对话框 readShort 一致：低字节在前）
 */
function readShortLE(arr, offset) {
    return (arr[offset] | (arr[offset + 1] << 8)) & 0xFFFF;
}
function readInt24LE(arr, offset) {
    return (arr[offset] | (arr[offset + 1] << 8) | (arr[offset + 2] << 16)) & 0xFFFFFF;
}
function readInt32LE(arr, offset) {
    return (arr[offset] | (arr[offset + 1] << 8) | (arr[offset + 2] << 16) | (arr[offset + 3] << 24)) >>> 0;
}
function writeShortLE(arr, offset, value) {
    arr[offset] = value & 0xFF;
    arr[offset + 1] = (value >> 8) & 0xFF;
}
