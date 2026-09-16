/**
 * 通道校准对话框（Dlg_ADAdjust1）
 * - Apply：38 号，DATA = [通道号+1, Temp_short低, Temp_short高, 0] → 实际发 3 字节（Qt 补零压缩）
 * - 通道号：AC = line*6 + ACChannel + 1；DC = 12 + DCChannel + 1
 * - Temp_short 换算：AC电压 ×DIM_public/57.74；AC电流 Current_Style==0 →/5，==1 →/1；
 *   DC零漂(0-3,8-11) ×DIM_public/2.5；DC系数(4-7,12-15) ×DIM_public/10
 * - 编辑校准源后 理想值 = 大写十六进制 + "H"（FormChang）
 * - DC 校准值：Qt 原代码 Temp_float 未初始化（UB）；本版按界面填写值下发——
 *   零漂校准默认 0mA（校零）、系数校准默认 20mA（装置直流量板卡 4-20mA，标称 20mA），均可编辑（见 README）
 * - 装置对 38 号校准命令回 39 号帧 [通道号, 0x55]（Qt 分发表无 39 号，Qt 原版同样无法识别）；
 *   本版期待 39（兼容 41），状态字节 0x55 判成功，解决"校准实际成功但界面报失败"的假象
 */
const DlgAdjust = {
    _channel: 0, // 0=line1 1=line2 2=DC
    _acChannel: 0, // 0..5 或 -1
    _dcChannel: 0, // 0..15 或 -1
    _dcValues: null, // DC 校准值（16 项：零漂 0-7 默认 0mA，系数 8-15 默认 20mA）

    // Line1/Line2 每行：{source, corr, orig, calc, ideal, ok}
    _lines: [
        [makeRow(), makeRow(), makeRow(), makeRow(), makeRow(), makeRow()],
        [makeRow(), makeRow(), makeRow(), makeRow(), makeRow(), makeRow()],
    ],

    _acNames: ['Ua', 'Ub', 'Uc', 'Ia', 'Ib', 'Ic'],

    open() {
        // Qt 构造：ACChannel=-1, Channel=-1, DCChannel=-1 → 然后 Channel=0, control()
        this._channel = 0;
        this._acChannel = 0;
        this._dcChannel = -1;
        // DC 校准值默认：零漂 0mA、系数 20mA（装置直流量板卡 4-20mA 标称）
        this._dcValues = [0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 20, 20, 20, 20, 20, 20];
        document.querySelectorAll('input[name="adj-line"]').forEach((r) => { r.checked = (r.value === '0'); });
        this.control();
        this.renderAC();
        this.renderDC();
        openDlg('dlg-adjust');
    },

    close() {
        closeDlg('dlg-adjust');
    },

    /** control()：line1/line2 → AC 表格，DC → DC 表格 */
    control() {
        $('adj-ac-group').style.display = (this._channel === 2) ? 'none' : '';
        $('adj-dc-group').style.display = (this._channel === 2) ? '' : 'none';
    },

    renderAC() {
        const tbody = $('tbl-adjust-ac').querySelector('tbody');
        tbody.innerHTML = '';
        const line = this._lines[this._channel];
        line.forEach((row, i) => {
            const tr = tbody.insertRow();
            // 通道号（单选）
            const td0 = tr.insertCell();
            td0.innerHTML = `<label class="form-check-label d-flex align-items-center gap-1 justify-content-center"><input class="form-check-input" type="radio" name="adj-acch" value="${i}" ${this._acChannel === i ? 'checked' : ''}> ${this._acNames[i]}</label>`;
            // 校准源
            const td1 = tr.insertCell();
            td1.innerHTML = `<input type="number" step="any" class="adj-src form-control form-control-sm" data-row="${i}" value="${row.source}">`;
            // 修正系数 / 原系数 / 计算值
            tr.insertCell().textContent = row.corr;
            tr.insertCell().textContent = row.orig;
            tr.insertCell().textContent = row.calc;
            // 理想值
            tr.insertCell().textContent = row.ideal;
            // 校准状态
            const td6 = tr.insertCell();
            td6.innerHTML = `<input class="form-check-input" type="checkbox" disabled ${row.ok ? 'checked' : ''}>`;
        });
    },

    renderDC() {
        const tbody = $('tbl-adjust-dc').querySelector('tbody');
        tbody.innerHTML = '';
        for (let i = 1; i <= 8; i++) {
            const tr = tbody.insertRow();
            tr.insertCell().textContent = `DC${i}`;
            // 零漂校准选择（DCChannel = i-1）+ 校准值（默认 0mA）
            const td1 = tr.insertCell();
            td1.innerHTML = `<input class="form-check-input" type="radio" name="adj-dczd" value="${i - 1}" ${this._dcChannel === i - 1 ? 'checked' : ''}>`;
            const td2 = tr.insertCell();
            td2.innerHTML = `<input type="number" step="any" class="adj-dcval form-control form-control-sm" data-ch="${i - 1}" value="${this._dcValues[i - 1]}">`;
            // 系数校准选择（DCChannel = 8+i-1）+ 校准值（默认 20mA）
            const td3 = tr.insertCell();
            td3.innerHTML = `<input class="form-check-input" type="radio" name="adj-dcxs" value="${8 + i - 1}" ${this._dcChannel === 8 + i - 1 ? 'checked' : ''}>`;
            const td4 = tr.insertCell();
            td4.innerHTML = `<input type="number" step="any" class="adj-dcval form-control form-control-sm" data-ch="${8 + i - 1}" value="${this._dcValues[8 + i - 1]}">`;
        }
    },

    /** 当前选中通道的除数（core: Qt 公式） */
    _divisor() {
        return adjustDivisor(this._channel, this._acChannel, this._dcChannel, Qt.Current_Style);
    },

    /** FormChang：大写十六进制 + "H"（core） */
    formChang(value) {
        return formChang(value, this._divisor(), Qt.DIM_public);
    },

    /** 通道校准（Apply，38 号，实发 3 字节） */
    async apply() {
        let channelNo;
        let tempShort;
        if (this._channel === 0 || this._channel === 1) {
            channelNo = adjustChannelNo(this._channel, this._acChannel, -1);
            const source = parseFloat(this._lines[this._channel][this._acChannel].source) || 0;
            tempShort = adjustTempShort(source, this._divisor(), Qt.DIM_public);
            // Qt: 清 OK 标志
            this._lines[this._channel][this._acChannel].ok = false;
        } else {
            channelNo = adjustChannelNo(2, -1, this._dcChannel);
            // Qt 原代码此处 Temp_float 未初始化（UB）；本版按界面填写的校准值下发
            // （零漂默认 0mA、系数默认 20mA，可在表格中修改）
            const val = this._dcValues[this._dcChannel] != null ? this._dcValues[this._dcChannel]
                : (this._dcChannel < 8 ? 0 : 20);
            tempShort = adjustTempShort(val, this._divisor(), Qt.DIM_public);
        }
        const { data, zero } = adjustPayload(channelNo, tempShort);
        Logger.info('dlg-adjust', `通道校准 Apply：通道号 ${channelNo}，校准值 ${(tempShort / Qt.DIM_public * this._divisor()).toFixed(3)}`);

        try {
            // 装置对 38 号校准命令回 39 号帧 [通道号, 0x55]（Qt 分发表无 39）；
            // 兼容期待 41（Qt 假定的回读响应号）
            const resp = await SerialManager.sendCommand({ nr: 38, data, zero, expectNrs: [39, 41], expectE5: true, down: 1090 });
            const okE5 = resp && resp.type === 'ack_e5';
            const okData = resp && resp.type === 'data' && resp.payload && resp.payload[1] === 0x55;
            if (!okE5 && !okData) {
                throw new Error('装置返回状态异常（期望 E5 或 39 号帧状态 0x55）');
            }
            if (this._channel === 0 || this._channel === 1) {
                this._lines[this._channel][this._acChannel].ok = true;
                this.renderAC();
            }
            Logger.info('dlg-adjust', '通道校准成功（装置已确认）');
            await showMessageBox('提示', '通道校准成功');
        } catch (e) {
            Logger.error('dlg-adjust', '通道校准失败', e);
            await showMessageBox('错误', '通道校准失败：' + (e.message || e));
        }
    },
};

function makeRow() {
    return { source: 0, corr: '0H', orig: '0H', calc: '0H', ideal: '4000H', ok: false };
}

(function () {
    // 初始化 Line1/Line2 校准源默认值（.ui 默认：U 行 57.74、I 行 1）
    [0, 1].forEach((l) => {
        DlgAdjust._lines[l].forEach((row, i) => {
            row.source = (i <= 2) ? 57.74 : 1;
        });
    });

    // Select 切换
    document.querySelectorAll('input[name="adj-line"]').forEach((r) => {
        r.addEventListener('change', () => {
            if (!r.checked) return;
            DlgAdjust._channel = parseInt(r.value, 10);
            if (DlgAdjust._channel === 2) {
                DlgAdjust._dcChannel = 0;
                DlgAdjust._acChannel = -1;
            } else {
                DlgAdjust._acChannel = 0;
                DlgAdjust._dcChannel = -1;
            }
            DlgAdjust.control();
            DlgAdjust.renderAC();
            DlgAdjust.renderDC();
        });
    });

    // AC 通道单选（表格内动态生成，事件委托）
    $('tbl-adjust-ac').addEventListener('change', (e) => {
        const t = e.target;
        if (t.name === 'adj-acch' && t.checked) {
            DlgAdjust._acChannel = parseInt(t.value, 10);
        }
        if (t.classList.contains('adj-src')) {
            const row = parseInt(t.dataset.row, 10);
            // Qt: 仅当前选中通道编辑后更新理想值
            if (row === DlgAdjust._acChannel && DlgAdjust._channel !== 2) {
                const val = parseFloat(t.value) || 0;
                DlgAdjust._lines[DlgAdjust._channel][row].source = val;
                DlgAdjust._lines[DlgAdjust._channel][row].ideal = DlgAdjust.formChang(val);
                DlgAdjust.renderAC();
            } else {
                DlgAdjust._lines[DlgAdjust._channel][row].source = parseFloat(t.value) || 0;
            }
        }
    });

    // DC 通道单选（事件委托）
    $('tbl-adjust-dc').addEventListener('change', (e) => {
        const t = e.target;
        if ((t.name === 'adj-dczd' || t.name === 'adj-dcxs') && t.checked) {
            DlgAdjust._dcChannel = parseInt(t.value, 10);
        }
    });

    // DC 校准值编辑（事件委托）
    $('tbl-adjust-dc').addEventListener('input', (e) => {
        const t = e.target;
        if (t.classList.contains('adj-dcval')) {
            const ch = parseInt(t.dataset.ch, 10);
            const val = parseFloat(t.value);
            DlgAdjust._dcValues[ch] = isNaN(val) ? 0 : val;
        }
    });

    $('adj-apply').addEventListener('click', () => DlgAdjust.apply());
    $('adj-exit').addEventListener('click', () => DlgAdjust.close());
})();
