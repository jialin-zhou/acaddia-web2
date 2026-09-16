/**
 * 全局状态与数据注册表
 * 对应 Qt 版 new1Dlg 的全局变量与各对话框的静态 ValidData 数组。
 * ValidData 数组均【含 4 字节头】（站址/F1/F2/报文号），与 Qt Telegarm_Array 一致，
 * 因此所有解析下标与 Qt 源码完全相同（如 ADCacul_ValidData[20+2j]）。
 */

const Qt = {
    // --- 标幺设置（DLg_DIMset，纯本地，不下发） ---
    DIM_public: 16384, // 4000H
    DIM_Line_Vot_style: 0, // 0=相/线电压标幺都显示100%；1=相电压100%、线电压按比例

    // --- 主界面显示相关 ---
    Current_Style: 1, // 来自 AD1 ValidData[55]（连接时读取）
    DC_V_I_select: [1, 1, 1, 1, 1, 1, 1, 1], // 连接时全置 1；AD 参数下载时更新

    // --- 通信状态机（new1Dlg） ---
    // Down 功能码（与 Qt 常量一致）：
    // 1000=离线 1001=在线连接 1005=主界面获取信息
    // 1028=TQCS应用/时间设置 1029=TQCS获取/时间获取 1030=AD获取 1031=AD应用/TQML应用
    // 1032=TQML获取 1090=ADAdjust应用 1198=角度获取 1277=通信报文获取
    Down: 1000,
    ErrorCounter: 0,
    Connect_Index: 0,
    RX_Msg_End: true,
    Msg_Len: 0,

    // --- ValidData 注册表（含 4 字节头） ---
    TQCS_ValidData: null, // 响应报文号 34 (0x22)
    AD1_ValidData: null, // 响应报文号 32 (0x20)
    ADCacul_ValidData: null, // 响应报文号 36 (0x24) 主界面AD计算值
    BaseAngle_ValidData: null, // 响应报文号 48 (0x30)
    Message_ValidData: null, // 响应报文号 50 (0x32)
    TimeSet_ValidData: null, // 响应报文号 28 (0x1C)
    ADAdjust_ValidData: null, // 响应报文号 41 (0x29)
    TQML_ValidData: null, // 响应报文号 46 (0x2E)
    MsgList_h_ValidData: null, // 响应报文号 2（告警列表带头，Qt 当前版本从不请求）
    MsgList_ValidData: [], // 响应报文号 3（告警列表无头，Msg_Len 累加）

    // --- 各对话框功能码（Qt 中用于守卫解析时机） ---
    TQCS_FUNCTION: null,
    AD1_FUNCTION: null,
    TQML_FUNCTION: null,

    // --- 串口状态 ---
    serial: {
        connected: false,
        portName: '',
        settings: null,
    },

    // --- 连接流程标志 ---
    connecting: false, // 正在执行在线连接序列
    fetching: false, // 正在执行主界面获取信息序列

    // 响应报文号 → 存储位置映射
    RESPONSE_MAP: {
        34: 'TQCS_ValidData',
        32: 'AD1_ValidData',
        36: 'ADCacul_ValidData',
        48: 'BaseAngle_ValidData',
        50: 'Message_ValidData',
        28: 'TimeSet_ValidData',
        41: 'ADAdjust_ValidData',
        39: 'ADAdjust_ValidData', // 通道校准应用响应（装置回 39 号帧 [通道号,0x55]，Qt 分发表无此号）
        46: 'TQML_ValidData',
        2: 'MsgList_h_ValidData',
        3: 'MsgList_ValidData',
    },

    /**
     * 按响应报文号保存数据帧（对应 new1Dlg::handleTimer11 的分发逻辑）
     * @param {object} frame { telegramNr, dataBlock }
     */
    saveResponse(frame) {
        const nr = frame.telegramNr;
        if (nr === 34) { this.TQCS_ValidData = frame.dataBlock; return true; }
        if (nr === 32) { this.AD1_ValidData = frame.dataBlock; return true; }
        if (nr === 36) { this.ADCacul_ValidData = frame.dataBlock; return true; }
        if (nr === 48) { this.BaseAngle_ValidData = frame.dataBlock; return true; }
        if (nr === 50) { this.Message_ValidData = frame.dataBlock; return true; }
        if (nr === 28) { this.TimeSet_ValidData = frame.dataBlock; return true; }
        if (nr === 41) { this.ADAdjust_ValidData = frame.dataBlock; return true; }
        if (nr === 39) {
            // 通道校准应用响应（装置实际行为）：[通道号, 状态]，Qt 分发表无 39
            this.ADAdjust_ValidData = frame.dataBlock;
            return true;
        }
        if (nr === 46) { this.TQML_ValidData = frame.dataBlock; return true; }
        if (nr === 2) {
            // PC请求告警信息(有Head)：无论 Telegarm_Array[13] 是否为 0 都置 RX_Msg_End=true（Qt 原逻辑）
            this.MsgList_h_ValidData = frame.dataBlock;
            this.RX_Msg_End = true;
            Logger.debug('store', '保存 2 号帧（告警列表带头），RX_Msg_End=true');
            return true;
        }
        if (nr === 3) {
            // 无head告警列表：按 Msg_Len 偏移累加拷贝（Qt: Telegarm_Array[len] → MsgList_ValidData[len]）
            const len = this.Msg_Len;
            for (let i = 0; i < frame.dataBlock.length; i++) {
                this.MsgList_ValidData[len + i] = frame.dataBlock[i];
            }
            this.Msg_Len = len + frame.dataBlock.length;
            if (frame.dataBlock[4] === 0) {
                this.RX_Msg_End = true; // 读完
                Logger.debug('store', `保存 3 号帧（告警列表第 ${len} 起，共 ${frame.dataBlock.length} 字节），读完 RX_Msg_End=true`);
            } else {
                this.RX_Msg_End = false;
                if (this.Down === 1001) this.Connect_Index = 4;
                else if (this.Down === 1005) this.Connect_Index = 2;
                Logger.debug('store', `保存 3 号帧（告警列表第 ${len} 起，共 ${frame.dataBlock.length} 字节），未完 RX_Msg_End=false Connect_Index=${this.Connect_Index}`);
            }
            return true;
        }
        return false; // 未知报文号（Qt: ErrorCounter++, Connect_Index=0）
    },

    /**
     * 离线/重置：清空数据（对应 new1Dlg::OnOffline）
     */
    clearData() {
        this.TQCS_ValidData = null;
        this.AD1_ValidData = null;
        this.ADCacul_ValidData = null;
        this.BaseAngle_ValidData = null;
        this.Message_ValidData = null;
        this.TimeSet_ValidData = null;
        this.ADAdjust_ValidData = null;
        this.TQML_ValidData = null;
        this.MsgList_h_ValidData = null;
        this.MsgList_ValidData = [];
        this.Msg_Len = 0;
        this.RX_Msg_End = true;
        this.Current_Style = 1;
        this.DC_V_I_select = [1, 1, 1, 1, 1, 1, 1, 1];
        this.DIM_public = 16384;
        this.DIM_Line_Vot_style = 0;
        this.ErrorCounter = 0;
        this.Connect_Index = 0;
    },
};

// --- 极简事件总线 ---
const Bus = {
    _handlers: {},
    on(event, fn) {
        (this._handlers[event] = this._handlers[event] || []).push(fn);
    },
    off(event, fn) {
        const list = this._handlers[event];
        if (list) {
            const i = list.indexOf(fn);
            if (i >= 0) list.splice(i, 1);
        }
    },
    emit(event, payload) {
        (this._handlers[event] || []).slice().forEach((fn) => {
            try { fn(payload); } catch (e) { Logger.error('bus', `事件处理异常 [${event}]`, e); }
        });
    },
};

// --- 通用小工具 ---
function $(id) { return document.getElementById(id); }

/**
 * 模拟 Qt QMessageBox（基于 Bootstrap 模态框 #msgbox-modal）
 */
function showMessageBox(title, text) {
    return new Promise((resolve) => {
        const el = document.getElementById('msgbox-modal');
        el.querySelector('.modal-title').textContent = title;
        el.querySelector('.modal-body').textContent = text;
        el.addEventListener('hidden.bs.modal', () => resolve(), { once: true });
        bootstrap.Modal.getOrCreateInstance(el).show();
    });
}

/**
 * 与 Qt QDate(1984,1,1).addDays(days) 一致的日期计算
 */
function dateFrom1984(daysElapsed) {
    const d = new Date(Date.UTC(1984, 0, 1));
    d.setUTCDate(d.getUTCDate() + daysElapsed);
    const y = d.getUTCFullYear();
    const m = String(d.getUTCMonth() + 1).padStart(2, '0');
    const day = String(d.getUTCDate()).padStart(2, '0');
    return `${y}-${m}-${day}`;
}

/**
 * 当天毫秒数 → "HH:MM:SS" 与剩余毫秒（对应 Dlg_ZJM::formatEventTime）
 * @returns {{time: string, ms: number}}
 */
function formatEventTime(msSinceMidnight) {
    const totalSeconds = Math.floor(msSinceMidnight / 1000);
    const ms = msSinceMidnight % 1000;
    const h = String(Math.floor(totalSeconds / 3600) % 24).padStart(2, '0');
    const m = String(Math.floor((totalSeconds % 3600) / 60)).padStart(2, '0');
    const s = String(totalSeconds % 60).padStart(2, '0');
    return { time: `${h}:${m}:${s}`, ms };
}
