/**
 * AD参数对话框（Dlg_AD1）——DOM 薄层
 * 打包/解析/默认值逻辑在 core.js（packAdPayload / parseAdResponse / adDefaultFields）
 * - Fetch：33 → 32；Apply：32（90 字节 DATA → 实发 89 字节，Qt 原逻辑）
 * - DC 源选择在下载时写回 Qt.DC_V_I_select（Qt 发送路径行为；解析路径不回写）
 */
const DlgAd = {
    _acFields: ['u1a', 'u1b', 'u1c', 'i1a', 'i1b', 'i1c', 'u2a', 'u2b', 'u2c', 'i2a', 'i2b', 'i2c'],
    _secVnv: 0, // 对应 Qt m_nAD1_SecVNV（界面无输入，获取后保留设备值）

    open() {
        this.loadFromData();
        openDlg('dlg-ad');
    },

    close() {
        closeDlg('dlg-ad');
    },

    /** 从 Qt.AD1_DATA（93 字节工作区）刷新界面 */
    loadFromData() {
        const d = Qt.AD1_DATA;
        if (!d) return;
        this._acFields.forEach((f, i) => { $(`ad-${f}`).value = String(readShortLE(d, i * 2)); });
        for (let i = 0; i < 8; i++) {
            $(`ad-dc-v${i + 1}`).value = String(readShortLE(d, 24 + i * 2));
            $(`ad-dc-c${i + 1}`).value = String(readShortLE(d, 40 + i * 2));
        }
        $('ad-vline1').value = String(readShortLE(d, 56));
        $('ad-vline2').value = String(readShortLE(d, 58));
        $('ad-cline1').value = String(readShortLE(d, 60));
        $('ad-cline2').value = String(readShortLE(d, 62));
        $('ad-model').value = String(d[64]);
        $('ad-adfix').value = String(readShortLE(d, 65));
        $('ad-oct').value = String(d[67] * 25);
        for (let i = 0; i < 8; i++) {
            document.querySelectorAll(`input[name="ad-dcss${i + 1}"]`).forEach((r) => {
                r.checked = (parseInt(r.value, 10) === d[69 + i]);
            });
        }
        ['ia', 'ib', 'ic', 'ix'].forEach((ch, i) => { $(`ad-ch-${ch}`).value = String(d[77 + i] + 1); });
        $('ad-dela').value = String(readShortLE(d, 81));
        $('ad-delb').value = String(readShortLE(d, 83));
        $('ad-delc').value = String(readShortLE(d, 85));
        $('ad-delx').value = String(readShortLE(d, 87));
    },

    /** 从 UI 取值（Qt onpushButtonAd1ApplyClicked 的 UI 读取） */
    readFields() {
        const num = (id, def) => {
            const v = parseInt($(id).value, 10);
            return isNaN(v) ? def : v;
        };
        const ac = this._acFields.map((f) => num(`ad-${f}`, 0));
        const vDc = [], cDc = [];
        for (let i = 0; i < 8; i++) {
            vDc.push(num(`ad-dc-v${i + 1}`, 0));
            cDc.push(num(`ad-dc-c${i + 1}`, 0));
        }
        const dcss = [];
        for (let i = 0; i < 8; i++) {
            const sel = document.querySelector(`input[name="ad-dcss${i + 1}"]:checked`);
            dcss.push(sel ? parseInt(sel.value, 10) : 1);
        }
        const channels = ['ia', 'ib', 'ic', 'ix'].map((ch, i) => num(`ad-ch-${ch}`, i + 1) - 1);
        return {
            ac,
            vDc,
            cDc,
            vLine1: num('ad-vline1', 500),
            vLine2: num('ad-vline2', 500),
            cLine1: num('ad-cline1', 3000),
            cLine2: num('ad-cline2', 3000),
            model: num('ad-model', 0),
            secVnv: this._secVnv != null ? this._secVnv : 0,
            adFix: num('ad-adfix', 0),
            oct: num('ad-oct', 3000),
            dcss,
            channels,
            delays: [num('ad-dela', 0), num('ad-delb', 0), num('ad-delc', 0), num('ad-delx', 0)],
        };
    },

    /** 默认参数（Qt onpushButton_3Ad1DefaultClicked） */
    applyDefaults() {
        const fields = adDefaultFields(Qt.DIM_public);
        this._secVnv = fields.secVnv;
        const data = packAdPayload(fields);
        Qt.AD1_DATA.set(data.subarray(0, 89), 0);
        Qt.AD1_DATA[89] = 0; Qt.AD1_DATA[90] = 0; Qt.AD1_DATA[91] = 0; Qt.AD1_DATA[92] = 0;
        this.loadFromData();
    },

    /** 下载参数（32 号，89 字节 payload） */
    async apply() {
        try {
            const fields = this.readFields();
            const data = packAdPayload(fields);
            let zero = 0;
            for (let i = 0; i < 90; i++) if (data[i] === 0) zero++;
            zero--; // Qt: Zero_NR 减一
            await SerialManager.sendCommand({ nr: 32, data, zero, expectNr: 32, expectE5: true, down: 1031 });
            // 下载成功：更新工作区 + DC_V_I_select（Qt 发送路径写 pParent->DC_V_I_select）
            Qt.AD1_DATA.set(data.subarray(0, 89), 0);
            Qt.AD1_DATA[89] = 0; Qt.AD1_DATA[90] = 0; Qt.AD1_DATA[91] = 0; Qt.AD1_DATA[92] = 0;
            for (let i = 0; i < 8; i++) Qt.DC_V_I_select[i] = fields.dcss[i];
            await showMessageBox('下载提示', '下载成功');
        } catch (e) {
            await showMessageBox('错误', '下载参数失败：' + (e.message || e));
        }
    },

    /** 获取参数（33 → 32） */
    async fetch() {
        try {
            const frame = await SerialManager.sendCommand({ nr: 33, data: [], zero: 0, expectNr: 32, down: 1030 });
            const fields = parseAdResponse(frame.dataBlock);
            this._secVnv = fields.secVnv; // Qt: m_nAD1_SecVNV 从设备值保留
            const data = packAdPayload(fields);
            Qt.AD1_DATA.set(data.subarray(0, 89), 0);
            Qt.AD1_DATA[89] = 0; Qt.AD1_DATA[90] = 0; Qt.AD1_DATA[91] = 0; Qt.AD1_DATA[92] = 0;
            this.loadFromData();
            // Qt 解析路径不回写 DC_V_I_select（保持连接后的默认值/上次下载值）
        } catch (e) {
            await showMessageBox('错误', '获取参数失败：' + (e.message || e));
        }
    },
};

(function () {
    // 通道选择下拉 1..22
    ['ia', 'ib', 'ic', 'ix'].forEach((ch) => {
        const sel = $(`ad-ch-${ch}`);
        for (let i = 1; i <= 22; i++) {
            const opt = document.createElement('option');
            opt.value = String(i);
            opt.textContent = String(i);
            sel.appendChild(opt);
        }
    });

    // DC1..DC8 网格（2×4，Bootstrap 卡片样式）
    const grid = $('ad-dc-grid');
    for (let i = 1; i <= 8; i++) {
        const cell = document.createElement('div');
        cell.className = 'col';
        cell.innerHTML = `
            <div class="dc-cell">
                <div class="dc-cell-title">DC${i}</div>
                <div class="dc-row"><span>V</span><input type="number" class="form-control form-control-sm" id="ad-dc-v${i}"><label class="form-check"><input class="form-check-input" type="radio" name="ad-dcss${i}" value="0"> Voltage</label></div>
                <div class="dc-row"><span>C</span><input type="number" class="form-control form-control-sm" id="ad-dc-c${i}"><label class="form-check"><input class="form-check-input" type="radio" name="ad-dcss${i}" value="1" checked> Current</label></div>
            </div>`;
        grid.appendChild(cell);
    }

    $('ad-apply').addEventListener('click', () => DlgAd.apply());
    $('ad-fetch').addEventListener('click', () => DlgAd.fetch());
    $('ad-default').addEventListener('click', () => {
        DlgAd.applyDefaults();
        showMessageBox('提示', '默认参数已恢复');
    });
    $('ad-exit').addEventListener('click', () => DlgAd.close());
})();
