/**
 * 通信报文对话框（Dlg_Message）
 * 发送 49 号读取；响应 50 号（64 字节数据区，全部小端 u16 / 单字节）
 * 初始状态：DataInvalid=1、Interpolation=1（Qt 构造）
 * HEX/DEC 单选为换算显示，不重新取数
 */
const DlgMessage = {
    _lastData: null,

    open() {
        openDlg('dlg-message');
    },

    close() {
        closeDlg('dlg-message');
    },

    async fetch() {
        try {
            const frame = await SerialManager.sendCommand({ nr: 49, data: [], zero: 0, expectNr: 50, down: 1277 });
            this._lastData = frame.dataBlock;
            this.render();
        } catch (e) {
            await showMessageBox('错误', '读取通信报文失败：' + (e.message || e));
        }
    },

    /**
     * 解析响应 50（core.parseMessageResponse，ValidData 含 4 字节头）
     * HEX 显示：Lenth/LDName 及以后字段补 4 位、LNName/DataSetName 补 2 位（Qt message_show_Data）
     */
    render() {
        const v = this._lastData;
        if (!v || v.length < 68) {
            showMessageBox('错误', '通信报文数据不完整');
            return;
        }
        const isHex = document.querySelector('input[name="msg-radix"]:checked').value === 'hex';
        const m = parseMessageResponse(v);

        $('msg-lenth').value = formatMessageValue(m.lenth, 4, isHex);
        $('msg-lnname').value = formatMessageValue(m.lnName, 2, isHex);
        $('msg-datasetname').value = formatMessageValue(m.dataSetName, 2, isHex);
        $('msg-ldname').value = formatMessageValue(m.ldName, 4, isHex);
        $('msg-artg').value = formatMessageValue(m.artg, 4, isHex);
        $('msg-nartg').value = formatMessageValue(m.nartg, 4, isHex);
        $('msg-vrtg').value = formatMessageValue(m.vrtg, 4, isHex);
        $('msg-tdr').value = formatMessageValue(m.tdr, 4, isHex);
        $('msg-smpctr').value = formatMessageValue(m.smpCtr, 4, isHex);

        // Channel1..22
        const chInputs = document.querySelectorAll('#msg-channels input[type="text"]');
        for (let i = 0; i < 22; i++) {
            chInputs[i].value = formatMessageValue(m.channels[i], 4, isHex);
        }

        // StatusWord1/2
        $('msg-sw1').value = formatMessageValue(m.sw1, 4, isHex);
        $('msg-sw2').value = formatMessageValue(m.sw2, 4, isHex);

        // StatusWord1 位定义（Qt 1178-1202 行）
        $('msg-bit0').checked = !!(m.sw1 & 0x0001); // 要求检修
        $('msg-bit1').checked = !!(m.sw1 & 0x0002); // 试验状态
        $('msg-bit2').checked = !!(m.sw1 & 0x0004); // 唤醒时间数据无效
        $('msg-bit3').checked = !!(m.sw1 & 0x0008); // 适用于插值法
        $('msg-bit4').checked = !!(m.sw1 & 0x0010); // 时间同步无效
        $('msg-bit12').checked = !!(m.sw1 & 0x1000); // 电流互感器输出类型
        $('msg-bit13').checked = !!(m.sw1 & 0x2000); // 比例因子
    },
};

(function () {
    // 22 个通道：禁用复选框（文本为序号）+ 数值框
    const box = $('msg-channels');
    for (let i = 1; i <= 22; i++) {
        const label = document.createElement('label');
        const cb = document.createElement('input');
        cb.type = 'checkbox';
        cb.className = 'form-check-input';
        cb.disabled = true;
        label.appendChild(cb);
        label.appendChild(document.createTextNode(String(i)));
        const input = document.createElement('input');
        input.type = 'text';
        input.className = 'form-control form-control-sm';
        input.readOnly = true;
        box.appendChild(label);
        box.appendChild(input);
    }

    // HEX/DEC 切换 → 换算显示
    document.querySelectorAll('input[name="msg-radix"]').forEach((r) => {
        r.addEventListener('change', () => { if (DlgMessage._lastData) DlgMessage.render(); });
    });

    $('msg-fetch').addEventListener('click', () => DlgMessage.fetch());
    $('msg-return').addEventListener('click', () => DlgMessage.close());
})();
