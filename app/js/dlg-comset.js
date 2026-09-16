/**
 * 串口设置对话框（Dlg_ComSet）
 * 参数选项与 Qt 一致：波特率 4800/9600/19200/38400，校验 NONE/ODD/EVEN，数据位 8/7/6，停止位 1/2
 */
const DlgComSet = {
    _pickedPort: null,

    open() {
        if (Qt.serial.settings) {
            $('com-baud').value = String(Qt.serial.settings.baudRate);
            $('com-parity').value = Qt.serial.settings.parity;
            $('com-databits').value = String(Qt.serial.settings.dataBits);
            $('com-stopbits').value = String(Qt.serial.settings.stopBits);
        }
        this._pickedPort = null;
        $('com-port-label').textContent = Qt.serial.connected
            ? ('当前串口：' + Qt.serial.portName)
            : '未选择串口';
        openDlg('dlg-comset');
    },

    close() {
        closeDlg('dlg-comset');
    },

    async pickPort() {
        try {
            this._pickedPort = await SerialManager.requestPort();
            const info = this._pickedPort.getInfo && this._pickedPort.getInfo();
            $('com-port-label').textContent = '已选择串口：' +
                (info && info.usbVendorId ? `VID:${info.usbVendorId.toString(16)}, PID:${info.usbProductId.toString(16)}` : '串口');
        } catch (e) {
            if (e && e.name === 'NotFoundError') {
                Logger.debug('dlg-comset', '用户取消了串口选择（NotFoundError）');
            } else {
                Logger.error('dlg-comset', '选择串口失败', e);
            }
        }
    },

    async apply() {
        const settings = {
            baudRate: parseInt($('com-baud').value, 10),
            dataBits: parseInt($('com-databits').value, 10),
            stopBits: parseInt($('com-stopbits').value, 10),
            parity: $('com-parity').value,
            flowControl: 'none',
            bufferSize: 255,
        };
        const port = this._pickedPort || SerialManager.port;
        if (!port) {
            await showMessageBox('错误', '请先点击"获取串口"选择串口');
            return;
        }
        try {
            // Qt: 关闭已打开串口 → 按新参数重新打开（不重新加载数据）
            await SerialManager.connect(port, settings);
            await showMessageBox('串口设置', `${settings.baudRate},${settings.parity[0]},${settings.dataBits},${settings.stopBits}`);
            this.close();
        } catch (e) {
            await showMessageBox('错误', '无法打开串口：' + (e.message || e));
        }
    },
};

(function () {
    $('com-getport').addEventListener('click', () => DlgComSet.pickPort());
    $('com-ok').addEventListener('click', () => DlgComSet.apply());
    $('com-cancel').addEventListener('click', () => DlgComSet.close());
})();
