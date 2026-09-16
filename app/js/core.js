/**
 * 纯数据核心层（无 DOM）：所有与 Qt 对应的打包/解析/显示计算
 * 逐行对照 Qt 各对话框源码移植：
 *  - Dlg_TQCS1::packDataToBuffer / parseDataFromBuffer
 *  - Dlg_AD1::onpushButtonAd1ApplyClicked / processDeviceData / onpushButton_3Ad1DefaultClicked
 *  - Dlg_ADAdjust1::onButtonApplyClicked / FormChang
 *  - Dlg_TQML::parseData
 *  - Dlg_TimeSet::sendTimeToDevice / calculateDateTimeFromDeviceData
 *  - Dlg_Message::Fetch_data / message_show_Data
 *  - Dlg_BaseAngle::Dlg_BaseAngle_update
 *  - Dlg_ZJM::updateData / processMeasurementRow
 */

// ==================== 同期参数（Dlg_TQCS1） ====================

/**
 * 打包 41 字节缓冲（Qt packDataToBuffer）
 * 所有数值字段使用 Qt 的缩放倍数并取整（×100 / ×10 / ×1）
 */
function packTqcsPayload(f) {
    const buf = new Uint8Array(41);
    buf[0] = !f.routineSelect; // 注意取反（Qt 原逻辑）
    buf[1] = f.syncStyle;
    buf[2] = f.deadL1 ? 1 : 0;
    buf[3] = f.deadL2 ? 1 : 0;
    buf[4] = f.deadAll ? 1 : 0;
    buf[5] = f.phase;
    writeShortLE(buf, 6, Math.trunc(f.tcb * 1));
    writeShortLE(buf, 8, Math.trunc(f.damax * 100));
    writeShortLE(buf, 10, Math.trunc(f.duMax * 100));
    writeShortLE(buf, 12, Math.trunc(f.dfMax * 1));
    writeShortLE(buf, 14, Math.trunc(f.dUmax2 * 100));
    writeShortLE(buf, 16, Math.trunc(f.dFmax2 * 100));
    writeShortLE(buf, 18, Math.trunc(f.uDead * 100));
    writeShortLE(buf, 20, Math.trunc(f.trU1U2 * 100));
    writeShortLE(buf, 22, Math.trunc(f.dfptMax * 100));
    writeShortLE(buf, 24, Math.trunc(f.uMin * 100));
    writeShortLE(buf, 26, Math.trunc(f.uMax * 100));
    writeShortLE(buf, 28, Math.trunc(f.fMin * 100));
    writeShortLE(buf, 30, Math.trunc(f.fMax * 100));
    writeShortLE(buf, 32, Math.trunc(f.tsynDuration * 10));
    writeShortLE(buf, 34, Math.trunc(f.synchron * 10));
    buf[36] = f.funcEnabled ? 1 : 0;
    // buf[37..40] = 0（Qt memset(dataBuffer+37, 0, 4)）
    return buf;
}

/**
 * 解析响应 34（ValidData 含 4 字节头，下标与 Qt 一致）
 * 返回字段对象；synchron2 = dfptMax（Qt 原逻辑）
 */
function parseTqcsResponse(v) {
    return {
        routineSelect: !v[4],
        syncStyle: v[5],
        deadL1: !!v[6],
        deadL2: !!v[7],
        deadAll: !!v[8],
        phase: v[9],
        tcb: readShortLE(v, 10) / 1,
        damax: readShortLE(v, 12) / 100,
        duMax: readShortLE(v, 14) / 100,
        dfMax: readShortLE(v, 16) / 1,
        dUmax2: readShortLE(v, 18) / 100,
        dFmax2: readShortLE(v, 20) / 100,
        uDead: readShortLE(v, 22) / 100,
        trU1U2: readShortLE(v, 24) / 100,
        dfptMax: readShortLE(v, 26) / 100,
        synchron2: readShortLE(v, 26) / 100, // = dfptMax
        uMin: readShortLE(v, 28) / 100,
        uMax: readShortLE(v, 30) / 100,
        fMin: readShortLE(v, 32) / 100,
        fMax: readShortLE(v, 34) / 100,
        tsynDuration: readShortLE(v, 36) / 10,
        synchron: readShortLE(v, 38) / 10,
        funcEnabled: !!v[40],
    };
}

// ==================== AD 参数（Dlg_AD1） ====================

const AD_DEFAULTS = {
    DIM_US: 7095,  // 0x1BB7
    DIM_IS: 7024,  // 0x1B70
    DIM_DCS: 20460, // 0x4FEC
    DIM_U: 28380, // 0x6EDC
    DIM_I: 17900, // 0x45E8
};

/**
 * 打包 90 字节 DATA（Qt onpushButtonAd1ApplyClicked）
 * f: { ac[12], vDc[8], cDc[8], vLine1, vLine2, cLine1, cLine2,
 *      model, adFix, oct, dcss[8], channels[4](已-1), delays[4] }
 */
function packAdPayload(f) {
    const d = new Uint8Array(90);
    for (let i = 0; i < 12; i++) writeShortLE(d, i * 2, f.ac[i]);
    for (let i = 0; i < 8; i++) {
        writeShortLE(d, 24 + i * 2, f.vDc[i]);
        writeShortLE(d, 40 + i * 2, f.cDc[i]);
    }
    writeShortLE(d, 56, f.vLine1);
    writeShortLE(d, 58, f.vLine2);
    writeShortLE(d, 60, f.cLine1);
    writeShortLE(d, 62, f.cLine2);
    d[64] = f.model;
    writeShortLE(d, 65, f.adFix); // m_Delay
    d[67] = Math.floor(f.oct / 25) & 0xFF; // C++ 整数除法 + uchar 截断
    d[68] = f.secVnv != null ? f.secVnv : 0; // m_nAD1_SecVNV
    for (let i = 0; i < 8; i++) d[69 + i] = f.dcss[i];
    for (let i = 0; i < 4; i++) d[77 + i] = f.channels[i]; // currentText-1
    writeShortLE(d, 81, f.delays[0]);
    writeShortLE(d, 83, f.delays[1]);
    writeShortLE(d, 85, f.delays[2]);
    writeShortLE(d, 87, f.delays[3]);
    d[89] = 0; // bak
    return d;
}

/**
 * 默认参数 89 字节（Qt onpushButton_3Ad1DefaultClicked）
 * 注意：Qt 8192 分支漏赋 C_DC3（未初始化），此处按意图补齐 DIM_DCS（见 README）
 */
function adDefaultFields(dimPublic) {
    let vDc, cDc, uScale, iScale;
    if (dimPublic === 8192) {
        vDc = AD_DEFAULTS.DIM_DCS; cDc = AD_DEFAULTS.DIM_DCS;
        uScale = AD_DEFAULTS.DIM_US; iScale = AD_DEFAULTS.DIM_IS;
    } else {
        vDc = 0; cDc = 50360;
        uScale = AD_DEFAULTS.DIM_U; iScale = AD_DEFAULTS.DIM_I;
    }
    const ac = [];
    for (let i = 0; i < 12; i++) {
        const isU = i < 3 || (i >= 6 && i < 9);
        ac.push(isU ? uScale : iScale);
    }
    return {
        ac,
        vDc: [vDc, vDc, vDc, vDc, vDc, vDc, vDc, vDc],
        cDc: [cDc, cDc, cDc, cDc, cDc, cDc, cDc, cDc],
        vLine1: 500, vLine2: 500, cLine1: 3000, cLine2: 3000,
        model: 0, secVnv: 0, adFix: 0, oct: 3000,
        dcss: [1, 1, 1, 1, 1, 1, 1, 1],
        channels: [0, 1, 2, 3],
        delays: [0, 0, 0, 0],
    };
}

/**
 * 解析响应 32（ValidData 含 4 字节头，下标与 Qt processDeviceData 一致）
 * 返回字段对象（oct 已 ×25 恢复，通道未 +1——Qt 存的是 currentText-1 值）
 */
function parseAdResponse(v) {
    const ac = [];
    for (let i = 0; i < 12; i++) ac.push(readShortLE(v, 4 + i * 2));
    const vDc = [], cDc = [];
    for (let i = 0; i < 8; i++) {
        vDc.push(readShortLE(v, 28 + i * 2));
        cDc.push(readShortLE(v, 44 + i * 2));
    }
    return {
        ac,
        vDc,
        cDc,
        vLine1: readShortLE(v, 60),
        vLine2: readShortLE(v, 62),
        cLine1: readShortLE(v, 64),
        cLine2: readShortLE(v, 66),
        model: v[68],
        secVnv: v[72], // m_nAD1_SecVNV
        adFix: readShortLE(v, 69),
        oct: v[71] * 25, // ×25 恢复
        dcss: Array.from(v.slice(73, 81)),
        channels: Array.from(v.slice(81, 85)),
        delays: [readShortLE(v, 85), readShortLE(v, 87), readShortLE(v, 89), readShortLE(v, 91)],
    };
}

// ==================== 通道校准（Dlg_ADAdjust1） ====================

/**
 * 通道号（Qt onButtonApplyClicked）：AC = line*6+acChannel+1；DC = 12+dcChannel+1
 */
function adjustChannelNo(line, acChannel, dcChannel) {
    if (line === 0 || line === 1) return (line * 6) + acChannel + 1;
    return 12 + dcChannel + 1;
}

/**
 * 换算除数（Qt 公式）：
 * AC 电压 57.74；AC 电流 Current_Style==0→5，==1→1；DC 零漂 2.5；DC 系数 10
 */
function adjustDivisor(line, acChannel, dcChannel, currentStyle) {
    if (line === 0 || line === 1) {
        if (acChannel <= 2) return 57.74;
        return (currentStyle === 0) ? 5 : 1;
    }
    if (dcChannel === 0 || dcChannel === 1 || dcChannel === 2 || dcChannel === 3 ||
        dcChannel === 8 || dcChannel === 9 || dcChannel === 10 || dcChannel === 11) {
        return 2.5; // 零漂电压
    }
    return 10; // 系数电流
}

/**
 * Temp_short（Qt: Temp_float × DIM_public / divisor 取整）
 */
function adjustTempShort(value, divisor, dimPublic) {
    return Math.trunc((value * dimPublic) / divisor) & 0xFFFF;
}

/**
 * FormChang：大写十六进制 + "H"
 */
function formChang(value, divisor, dimPublic) {
    const t = Math.trunc((value * dimPublic) / divisor);
    return t.toString(16).toUpperCase() + 'H';
}

/**
 * 校准 4 字节 DATA + 补零参数（Qt: DATA[3]=0, zero_NR=0 个数-1 → 实发 3 字节）
 */
function adjustPayload(channelNo, tempShort) {
    const data = new Uint8Array([channelNo & 0xFF, tempShort & 0xFF, (tempShort >> 8) & 0xFF, 0]);
    let zero = 0;
    for (let i = 0; i < 4; i++) if (data[i] === 0) zero++;
    zero--;
    return { data, zero };
}

// ==================== 同期命令（Dlg_TQML） ====================

/**
 * 解析响应 46（ValidData 含 4 字节头）
 * 日期：1984-01-01 + days（TQML 不減 1，Qt 原样）
 */
function parseTqmlResponse(v) {
    const fields = [
        { name: 'u1', off: 11, scale: 100 },
        { name: 'u2', off: 13, scale: 100 },
        { name: 'f1', off: 15, scale: 1000 },
        { name: 'f2', off: 17, scale: 1000 },
        { name: 'f1dt', off: 19, scale: 100 },
        { name: 'f2dt', off: 21, scale: 100 },
        { name: 'du', off: 23, scale: 100 },
        { name: 'df', off: 25, scale: 1000 },
        { name: 'dfdt', off: 27, scale: 100 },
        { name: 'da', off: 29, scale: 100 },
    ];
    const values = {};
    fields.forEach((f) => { values[f.name] = readShortLE(v, f.off) / f.scale; });
    return {
        recordType: v[4],
        days: readShortLE(v, 5),
        msecs: readInt32LE(v, 7) % 86400000,
        ...values,
    };
}

// ==================== 时间设置（Dlg_TimeSet） ====================

/**
 * 打包时间设置 7 字节（Qt sendTimeToDevice：6 字节数据 + 1 字节 0 终止符）
 */
function packTimePayload(days, msecs) {
    const data = new Uint8Array(7);
    data[0] = days & 0xFF;
    data[1] = (days >> 8) & 0xFF;
    data[2] = msecs & 0xFF;
    data[3] = (msecs >> 8) & 0xFF;
    data[4] = (msecs >> 16) & 0xFF;
    data[5] = (msecs >> 24) & 0xFF;
    data[6] = 0;
    let zero = 0;
    for (let i = 0; i < 6; i++) if (data[i] === 0) zero++;
    return { data, zero };
}

/**
 * 解析响应 28（ValidData 含 4 字节头）
 * 毫秒只读 3 字节小端（Qt 发送 4 字节/接收 3 字节的不对称原样保留）
 * 日期 = 1984-01-01 + (days - 1)
 */
function parseTimeResponse(v) {
    return {
        days: readShortLE(v, 4),
        msecs: readInt24LE(v, 6),
    };
}

// ==================== 角度矢量（Dlg_BaseAngle） ====================

/**
 * 解析响应 48：13 个 u16 小端 / 100（下标与 Qt 一致）
 */
function parseAngleResponse(v) {
    const values = [];
    for (let i = 0; i < 13; i++) {
        values.push(readShortLE(v, 4 + i * 2) / 100);
    }
    return values; // [U1a,U1b,U1c,I1a,I1b,I1c,U2a,U2b,U2c,I2a,I2b,I2c,da]
}

// ==================== 通信报文（Dlg_Message） ====================

/**
 * 解析响应 50（ValidData 含 4 字节头，下标与 Qt Fetch_data 一致）
 */
function parseMessageResponse(v) {
    const channels = [];
    for (let i = 0; i < 22; i++) channels.push(readShortLE(v, 20 + i * 2));
    return {
        lenth: readShortLE(v, 4),
        lnName: v[6],
        dataSetName: v[7],
        ldName: readShortLE(v, 8),
        artg: readShortLE(v, 10),
        nartg: readShortLE(v, 12),
        vrtg: readShortLE(v, 14),
        tdr: readShortLE(v, 16),
        smpCtr: readShortLE(v, 18),
        channels,
        sw1: readShortLE(v, 64),
        sw2: readShortLE(v, 66),
    };
}

/**
 * 显示格式（Qt message_show_Data）：DEC 十进制；HEX "0x"+大写十六进制，
 * Lenth/LDName 及以后字段补齐 4 位，LNName/DataSetName 补齐 2 位
 */
function formatMessageValue(val, digits, isHex) {
    return isHex ? '0x' + val.toString(16).toUpperCase().padStart(digits, '0') : String(val);
}

// ==================== 主界面显示（Dlg_ZJM::updateData / processMeasurementRow） ====================

const MAIN_ITEM_NAMES = ['Ua', 'Ub', 'Uc', 'Uab', 'Ubc', 'Uca', 'Ia', 'Ib', 'Ic', 'P', 'Q', 'S', 'f', 'Cos phi'];
const MAIN_DC_NAMES = ['DC1', 'DC2', 'DC3', 'DC4', 'DC5', 'DC6', 'DC7', 'DC8'];

/**
 * 单行测量数据计算（Qt processMeasurementRow）
 * @returns { channelName, item, second, percent, count }
 */
function computeMeasurementRow(channelName, itemIndex, rawData, state) {
    const countStr = String(rawData);
    let normalizedValue = rawData;
    // Line1 使用固定 16384（4000H，与装置满量程一致），Line2 使用 DIM_public。
    // Qt 原逻辑 Line1 固定除 16899（历史遗留常数），在满量程 4000H 的装置上所有读数偏低 3.05%
    // （实测 57.74V 显示 55.98V、28.87V 显示 27.99V），本版按 16384 修正（见 README 已知差异）
    normalizedValue /= (channelName === 'Line1') ? 16384.0 : state.dimPublic;

    let physicalValue = normalizedValue;
    let unit = '';
    let isNegative = false;
    let absRawData = rawData;

    // P, Q, Cos phi 可能为负（补码）
    if (itemIndex === 9 || itemIndex === 10 || itemIndex === 13) {
        if (absRawData > 32767) {
            isNegative = true;
            absRawData = (~absRawData + 1) & 0xFFFF;
            normalizedValue = absRawData / state.dimPublic;
        }
    }

    switch (itemIndex) {
        case 0: case 1: case 2:
            physicalValue *= 57.74; unit = 'V'; break;
        case 3: case 4: case 5:
            physicalValue *= (state.dimLineVotStyle === 1) ? 57.74 : 100.0; unit = 'V'; break;
        case 6: case 7: case 8:
            physicalValue *= (state.currentStyle === 0) ? 5.0 : 1.0; unit = 'A'; break;
        case 9: case 10: case 11: {
            const pnv = absRawData / state.dimPublic;
            physicalValue = pnv * ((state.currentStyle === 0) ? 865.5 : 173.1);
            if (itemIndex !== 11 && isNegative) physicalValue *= -1; // S 恒正
            unit = itemIndex === 9 ? 'W' : (itemIndex === 10 ? 'Var' : 'VA');
            break;
        }
        case 12:
            physicalValue *= 50.0; unit = 'Hz'; break;
        case 13:
            physicalValue = absRawData / state.dimPublic;
            if (isNegative) physicalValue *= -1;
            unit = ''; break;
    }

    return {
        channel: channelName,
        item: MAIN_ITEM_NAMES[itemIndex],
        second: (physicalValue.toFixed(4) + ' ' + unit).trim(),
        percent: (normalizedValue * 100.0).toFixed(4) + '%',
        count: countStr,
    };
}

/**
 * 主界面 AD 计算值全部行（Qt updateData）
 * vd = ADCacul_ValidData（含 4 字节头，下标 20/48/4 直接沿用，大端）
 */
function computeMainDisplayRows(vd, state) {
    const rows = [];
    if (!vd) return rows;
    if (state.showLine1) {
        for (let j = 0; j < 14; j++) {
            const raw = (vd[21 + 2 * j] << 8) | vd[20 + 2 * j];
            rows.push(computeMeasurementRow('Line1', j, raw, state));
        }
    }
    if (state.showLine2) {
        for (let j = 0; j < 14; j++) {
            const raw = (vd[49 + 2 * j] << 8) | vd[48 + 2 * j];
            rows.push(computeMeasurementRow('Line2', j, raw, state));
        }
    }
    if (state.showDc) {
        for (let j = 0; j < 8; j++) {
            const raw = (vd[5 + 2 * j] << 8) | vd[4 + 2 * j];
            const normalized = raw / state.dimPublic;
            const physical = normalized * ((state.dcVSelect[j] === 0) ? 5.0 : 10.0);
            const unit = (state.dcVSelect[j] === 0) ? 'V' : 'mA';
            rows.push({
                channel: 'DC',
                item: MAIN_DC_NAMES[j],
                second: (physical.toFixed(4) + ' ' + unit).trim(),
                percent: (normalized * 100.0).toFixed(4) + '%',
                count: String(raw),
            });
        }
    }
    return rows;
}

/**
 * 主界面消息列表行（Qt updateData 中 Down==1005 分支）
 * h = MsgList_h_ValidData, b = MsgList_ValidData
 */
function computeMessageRows(h, b) {
    const rows = [];
    if (!h) return rows;
    const msgNr = h[13] & 0x7F;
    for (let k = 0; k < msgNr; k++) {
        const msgID = (h[15 + 8 * k] << 8) + h[14 + 8 * k];
        rows.push({
            msgId: String(msgID - 32768),
            date: dateFrom1984(((h[17 + 8 * k] << 8) + h[16 + 8 * k]) + 1),
            time: formatEventTime((((h[21 + 8 * k] << 8) + h[20 + 8 * k]) << 16) + ((h[19 + 8 * k] << 8) + h[18 + 8 * k])).time,
            ms: formatEventTime((((h[21 + 8 * k] << 8) + h[20 + 8 * k]) << 16) + ((h[19 + 8 * k] << 8) + h[18 + 8 * k])).ms,
            event: (msgID > 32767) ? '+' : '-',
        });
    }
    if (b && b.length > 4) {
        const count = b[4] - 128;
        for (let kk = 0; kk < count; kk++) {
            const msgID = (h[6 + 8 * kk] << 8) + b[5 + 8 * kk];
            rows.push({
                msgId: String(msgID - 32768),
                date: dateFrom1984(((h[8 + 8 * kk] << 8) + h[7 + 8 * kk]) + 1),
                time: formatEventTime((((h[12 + 8 * kk] << 8) + h[11 + 8 * kk]) << 16) + ((h[10 + 8 * kk] << 8) + h[9 + 8 * kk])).time,
                ms: formatEventTime((((h[12 + 8 * kk] << 8) + h[11 + 8 * kk]) << 16) + ((h[10 + 8 * kk] << 8) + h[9 + 8 * kk])).ms,
                event: (msgID > 32767) ? '+' : '-',
            });
        }
    }
    return rows;
}

/**
 * 日期换算（Qt QDate(1984,1,1).addDays(n)）
 */
function dateStrFromDays1984(daysElapsed, minusOne) {
    const d = new Date(Date.UTC(1984, 0, 1));
    d.setUTCDate(d.getUTCDate() + (minusOne ? daysElapsed - 1 : daysElapsed));
    const y = d.getUTCFullYear();
    const m = String(d.getUTCMonth() + 1).padStart(2, '0');
    const day = String(d.getUTCDate()).padStart(2, '0');
    return `${y}-${m}-${day}`;
}
