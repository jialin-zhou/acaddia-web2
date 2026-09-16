/**
 * 同期参数对话框（Dlg_TQCS1）——DOM 薄层
 * 打包/解析逻辑在 core.js（packTqcsPayload / parseTqcsResponse）
 * - Fetch：35 → 34；Apply：34（41 字节缓冲 → 实发 40 字节，Qt 原逻辑）
 */
const DlgTqcs = {
    open() {
        this.setDefaults();
        openDlg('dlg-tqcs');
    },

    close() {
        closeDlg('dlg-tqcs');
    },

    /** Qt setDefaultValues */
    setDefaults() {
        document.querySelectorAll('input[name="tqcs-routine"]').forEach((r) => { r.checked = (r.value === '1'); });
        document.querySelectorAll('input[name="tqcs-syncstyle"]').forEach((r) => { r.checked = (r.value === '0'); });
        document.querySelectorAll('input[name="tqcs-phase"]').forEach((r) => { r.checked = (r.value === '0'); });
        $('tqcs-dead-l1').checked = false;
        $('tqcs-dead-l2').checked = false;
        $('tqcs-dead-all').checked = false;
        $('tqcs-func-enab').checked = true;
        $('tqcs-tcb').value = 80;
        $('tqcs-damax').value = 10.0;
        $('tqcs-dumax').value = 2.0;
        $('tqcs-dumax2').value = 2.0;
        $('tqcs-dfmax').value = 20.0;
        $('tqcs-dfmax2').value = 0.10;
        $('tqcs-udead').value = 2.0;
        $('tqcs-tru1u2').value = 0.0;
        $('tqcs-dfptmax').value = 0.2;
        $('tqcs-synchron2').value = 0.2;
        $('tqcs-umin').value = 52.0;
        $('tqcs-umax').value = 63.5;
        $('tqcs-fmin').value = 47.50;
        $('tqcs-fmax').value = 52.50;
        $('tqcs-tsyn').value = 30.0;
        $('tqcs-synchron').value = 0.5;
        this.updateControls();
    },

    /** 控件可用性（Qt 162-172 行） */
    updateControls() {
        const enabled = $('tqcs-func-enab').checked;
        const manual = document.querySelector('input[name="tqcs-routine"]:checked').value === '1';
        document.querySelectorAll('#dlg-tqcs .modal-body input[type="number"]:not([readonly])').forEach((i) => {
            i.disabled = !enabled;
        });
        document.querySelectorAll('input[name="tqcs-syncstyle"]').forEach((r) => { r.disabled = !(enabled && manual); });
        document.querySelectorAll('input[name="tqcs-routine"], input[name="tqcs-phase"]').forEach((r) => { r.disabled = !enabled; });
        ['tqcs-dead-l1', 'tqcs-dead-l2', 'tqcs-dead-all'].forEach((id) => { $(id).disabled = !enabled; });
    },

    /** 从 UI 取值 */
    readFields() {
        const num = (id, def) => {
            const v = parseFloat($(id).value);
            return isNaN(v) ? def : v;
        };
        return {
            routineSelect: document.querySelector('input[name="tqcs-routine"]:checked').value === '1' ? 1 : 0,
            syncStyle: document.querySelector('input[name="tqcs-syncstyle"]:checked').value === '1' ? 1 : 0,
            deadL1: $('tqcs-dead-l1').checked,
            deadL2: $('tqcs-dead-l2').checked,
            deadAll: $('tqcs-dead-all').checked,
            phase: parseInt(document.querySelector('input[name="tqcs-phase"]:checked').value, 10),
            tcb: num('tqcs-tcb', 80),
            damax: num('tqcs-damax', 10),
            duMax: num('tqcs-dumax', 2),
            dfMax: num('tqcs-dfmax', 20),
            dUmax2: num('tqcs-dumax2', 2),
            dFmax2: num('tqcs-dfmax2', 0.1),
            uDead: num('tqcs-udead', 2),
            trU1U2: num('tqcs-tru1u2', 0),
            dfptMax: num('tqcs-dfptmax', 0.2),
            uMin: num('tqcs-umin', 52),
            uMax: num('tqcs-umax', 63.5),
            fMin: num('tqcs-fmin', 47.5),
            fMax: num('tqcs-fmax', 52.5),
            tsynDuration: num('tqcs-tsyn', 30),
            synchron: num('tqcs-synchron', 0.5),
            funcEnabled: $('tqcs-func-enab').checked,
        };
    },

    /** Apply（34 号，40 字节 payload） */
    async apply() {
        try {
            const data = packTqcsPayload(this.readFields());
            let zero = 0;
            for (let i = 0; i < 41; i++) if (data[i] === 0) zero++;
            zero--; // Qt: zero_NR 减一后传给 Fun_Connect
            await SerialManager.sendCommand({ nr: 34, data, zero, expectNr: 34, expectE5: true, down: 1028 });
        } catch (e) {
            await showMessageBox('错误', '同期参数下载失败：' + (e.message || e));
        }
    },

    /** Fetch（35 → 34） */
    async fetch() {
        try {
            const frame = await SerialManager.sendCommand({ nr: 35, data: [], zero: 0, expectNr: 34, down: 1029 });
            this.writeFields(parseTqcsResponse(frame.dataBlock));
        } catch (e) {
            await showMessageBox('错误', '同期参数获取失败：' + (e.message || e));
        }
    },

    /** 把解析结果写回 UI */
    writeFields(f) {
        if (!f) return;
        document.querySelectorAll('input[name="tqcs-routine"]').forEach((r) => { r.checked = (r.value === (f.routineSelect ? '0' : '1')); });
        document.querySelectorAll('input[name="tqcs-syncstyle"]').forEach((r) => { r.checked = (r.value === String(f.syncStyle)); });
        $('tqcs-dead-l1').checked = f.deadL1;
        $('tqcs-dead-l2').checked = f.deadL2;
        $('tqcs-dead-all').checked = f.deadAll;
        document.querySelectorAll('input[name="tqcs-phase"]').forEach((r) => { r.checked = (parseInt(r.value, 10) === f.phase); });
        $('tqcs-tcb').value = String(f.tcb);
        $('tqcs-damax').value = String(f.damax);
        $('tqcs-dumax').value = String(f.duMax);
        $('tqcs-dfmax').value = String(f.dfMax);
        $('tqcs-dumax2').value = String(f.dUmax2);
        $('tqcs-dfmax2').value = String(f.dFmax2);
        $('tqcs-udead').value = String(f.uDead);
        $('tqcs-tru1u2').value = String(f.trU1U2);
        $('tqcs-dfptmax').value = String(f.dfptMax);
        $('tqcs-synchron2').value = String(f.synchron2);
        $('tqcs-umin').value = String(f.uMin);
        $('tqcs-umax').value = String(f.uMax);
        $('tqcs-fmin').value = String(f.fMin);
        $('tqcs-fmax').value = String(f.fMax);
        $('tqcs-tsyn').value = String(f.tsynDuration);
        $('tqcs-synchron').value = String(f.synchron);
        $('tqcs-func-enab').checked = f.funcEnabled;
        this.updateControls();
    },
};

(function () {
    $('tqcs-func-enab').addEventListener('change', () => DlgTqcs.updateControls());
    document.querySelectorAll('input[name="tqcs-routine"]').forEach((r) => {
        r.addEventListener('change', () => DlgTqcs.updateControls());
    });
    $('tqcs-apply').addEventListener('click', () => DlgTqcs.apply());
    $('tqcs-fetch').addEventListener('click', () => DlgTqcs.fetch());
    $('tqcs-default').addEventListener('click', () => DlgTqcs.setDefaults());
    $('tqcs-cancel').addEventListener('click', () => DlgTqcs.close());
})();
