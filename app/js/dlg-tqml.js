/**
 * 同期命令设置对话框（Dlg_TQML）
 * - 同期开出（Apply）：44 号，payload = [0x55, 0xAA]
 * - 信息读取（Fetch）：45 号，payload = [m_nTQML_REC + 1]（0=接收到命令/1=发合闸命令/2=合闸时刻）
 * - 响应 46 号解析（recordType=1/2/3 数据，4=复归，5=未完成）
 *   日期：1984-01-01 + days（TQML 不減 1，与 TimeSet 不同，Qt 原样）
 */
const DlgTqml = {
    _rec: 0, // m_nTQML_REC

    open() {
        this._rec = 0;
        document.querySelectorAll('input[name="tqml-rec"]').forEach((r) => { r.checked = (r.value === '0'); });
        this.resetDataFields();
        openDlg('dlg-tqml');
    },

    close() {
        closeDlg('dlg-tqml');
    },

    /** Qt resetDataFields：日期=1000-10-10 10:10:10、标志全 false、数值全 0 */
    resetDataFields() {
        ['rec', 'sent', 'time'].forEach((g) => {
            $(`tqml-date-${g}`).value = '1000-10-10T10:10:10';
            $(`tqml-ms-${g}`).value = '0';
            $(`tqml-ok-${g}`).checked = false;
            ['u1', 'u2', 'f1', 'f2', 'f1dt', 'f2dt', 'du', 'df', 'dfdt', 'da'].forEach((f) => {
                $(`tqml-${f}-${g}`).value = '';
            });
        });
        $('tqml-reset').checked = false;
        $('tqml-unfinish').checked = false;
    },

    _groupName() {
        return this._rec === 0 ? 'rec' : (this._rec === 1 ? 'sent' : 'time');
    },

    /** 同期开出（44 号 [55 AA]） */
    async apply() {
        try {
            this.resetDataFields();
            // Qt: ValidDataToPack[0]=0x55,[1]=0xAA,[2]=0 → payload 2 字节
            await SerialManager.sendCommand({ nr: 44, data: new Uint8Array([0x55, 0xAA, 0]), zero: 0, expectNr: 46, expectE5: true, down: 1031 });
        } catch (e) {
            await showMessageBox('错误', '同期开出失败：' + (e.message || e));
        }
    },

    /** 信息读取（45 号，payload = 类型+1） */
    async fetch() {
        try {
            const type = this._rec + 1;
            const frame = await SerialManager.sendCommand({ nr: 45, data: new Uint8Array([type, 0]), zero: 0, expectNr: 46, down: 1032 });
            this.parse(frame.dataBlock);
        } catch (e) {
            await showMessageBox('错误', '信息读取失败：' + (e.message || e));
        }
    },

    /**
     * 解析响应 46（core.parseTqmlResponse，ValidData 含 4 字节头）
     */
    parse(v) {
        if (!v || v.length < 31) {
            showMessageBox('错误', '同期命令数据不完整');
            return;
        }
        const r = parseTqmlResponse(v);
        if (r.recordType === 4) {
            // 复位
            $('tqml-reset').checked = true;
            $('tqml-unfinish').checked = false;
            this.resetDataFields();
            return;
        }
        if (r.recordType === 5) {
            // 未完成
            $('tqml-unfinish').checked = true;
            $('tqml-reset').checked = false;
            this.resetDataFields();
            return;
        }
        if (r.recordType !== 1 && r.recordType !== 2 && r.recordType !== 3) {
            return;
        }
        this._rec = r.recordType - 1;
        document.querySelectorAll('input[name="tqml-rec"]').forEach((rb) => { rb.checked = (parseInt(rb.value, 10) === this._rec); });
        $('tqml-reset').checked = false;
        $('tqml-unfinish').checked = false;

        const g = this._groupName();
        // 日期：1984-01-01 + days（TQML 不减 1，Qt 原样）
        const totalSeconds = Math.floor(r.msecs / 1000);
        $(`tqml-date-${g}`).value =
            dateStrFromDays1984(r.days, false) +
            `T${String(Math.floor(totalSeconds / 3600) % 24).padStart(2, '0')}:${String(Math.floor((totalSeconds % 3600) / 60)).padStart(2, '0')}:${String(totalSeconds % 60).padStart(2, '0')}`;
        $(`tqml-ms-${g}`).value = String(r.msecs % 1000);

        // 字段显示格式：F1/F2/DF → 3 位小数；其余 → 2 位小数（Qt 159-195 行）
        const fields = [
            ['u1', 2], ['u2', 2],
            ['f1', 3], ['f2', 3],
            ['f1dt', 2], ['f2dt', 2],
            ['du', 2], ['df', 3],
            ['dfdt', 2], ['da', 2],
        ];
        fields.forEach(([name, digits]) => {
            $(`tqml-${name}-${g}`).value = r[name].toFixed(digits);
        });
        $(`tqml-ok-${g}`).checked = true;
    },
};

(function () {
    document.querySelectorAll('input[name="tqml-rec"]').forEach((r) => {
        r.addEventListener('change', () => {
            if (r.checked) DlgTqml._rec = parseInt(r.value, 10);
        });
    });
    $('tqml-apply').addEventListener('click', () => DlgTqml.apply());
    $('tqml-fetch').addEventListener('click', () => DlgTqml.fetch());
    $('tqml-cancel').addEventListener('click', () => DlgTqml.close());
})();
