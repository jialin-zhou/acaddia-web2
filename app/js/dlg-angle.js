/**
 * 角度矢量对话框（Dlg_BaseAngle）
 * 发送 47 号获取角度；响应 48 号，13 个 u16 小端 /100
 * （Qt 发送后立即解析；本版等待响应帧到达后解析，字节交互一致）
 */
const DlgAngle = {
    open() {
        openDlg('dlg-angle');
    },

    close() {
        closeDlg('dlg-angle');
    },

    async fetch() {
        try {
            const frame = await SerialManager.sendCommand({ nr: 47, data: [], zero: 0, expectNr: 48, down: 1198 });
            this.parse(frame.dataBlock);
        } catch (e) {
            await showMessageBox('错误', '获取角度失败：' + (e.message || e));
        }
    },

    /**
     * 解析（core.parseAngleResponse，ValidData 含 4 字节头，与 Qt 下标一致）：
     * 13 个 u16 小端 /100，显示 QString::number(float) 默认格式
     */
    parse(v) {
        if (!v || v.length < 30) {
            showMessageBox('错误', '角度数据不完整');
            return;
        }
        const values = parseAngleResponse(v);
        const names = ['ang-l1-ua', 'ang-l1-ub', 'ang-l1-uc', 'ang-l1-ia', 'ang-l1-ib', 'ang-l1-ic',
            'ang-l2-ua', 'ang-l2-ub', 'ang-l2-uc', 'ang-l2-ia', 'ang-l2-ib', 'ang-l2-ic'];
        for (let i = 0; i < 12; i++) {
            $(names[i]).value = String(values[i]);
        }
        $('ang-da').value = String(values[12]);
    },
};

(function () {
    $('ang-fetch').addEventListener('click', () => DlgAngle.fetch());
    $('ang-exit').addEventListener('click', () => DlgAngle.close());
})();
