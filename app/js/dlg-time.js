/**
 * 时间设置对话框（Dlg_TimeSet）
 * - 获取时间：29 → 28（天数 u16 LE，毫秒仅 3 字节 LE，日期 = 1984-01-01 + (days-1)）
 * - 设置时间：28（天数 u16 LE + 毫秒 u32 LE，days = daysTo(1984-01-01)+1），仅手动模式且用户修改过
 * - 未勾选 set data/time 时每秒自动刷新系统时间（Qt: 1000ms 定时器）
 */
const DlgTime = {
    _timer: null,
    _userModified: false,

    open() {
        $('time-manual').checked = false;
        this._userModified = false;
        this._syncToSystemTime();
        this._startTimer();
        openDlg('dlg-time');
    },

    close() {
        this._stopTimer();
        closeDlg('dlg-time');
    },

    _startTimer() {
        this._stopTimer();
        this._timer = setInterval(() => {
            if (!$('time-manual').checked) {
                this._syncToSystemTime();
            }
        }, 1000);
    },

    _stopTimer() {
        if (this._timer) { clearInterval(this._timer); this._timer = null; }
    },

    _syncToSystemTime() {
        const now = new Date();
        $('time-date').value = `${now.getFullYear()}-${String(now.getMonth() + 1).padStart(2, '0')}-${String(now.getDate()).padStart(2, '0')}`;
        $('time-time').value = `${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}:${String(now.getSeconds()).padStart(2, '0')}`;
    },

    _daysSince1984(dateStr) {
        // 返回 1984-01-01 到 dateStr 的天数（QDate.daysTo）
        const [y, m, d] = dateStr.split('-').map(Number);
        const a = Date.UTC(1984, 0, 1);
        const b = Date.UTC(y, m - 1, d);
        return Math.round((b - a) / 86400000);
    },

    _msecsSinceMidnight(timeStr) {
        const [h, m, s] = timeStr.split(':').map(Number);
        return ((h * 3600 + m * 60 + s) * 1000) >>> 0;
    },

    /** 获取时间（对应 Qt Fetch：29 号） */
    async fetch() {
        try {
            const frame = await SerialManager.sendCommand({ nr: 29, data: [], zero: 0, expectNr: 28, down: 1029 });
            this.parse(frame.dataBlock);
        } catch (e) {
            await showMessageBox('错误', '获取时间失败：' + (e.message || e));
        }
    },

    /**
     * 解析响应 28（core.parseTimeResponse）：
     * days = V[4]|V[5]<<8；date = 1984-01-01 + (days-1)；
     * 毫秒 3 字节小端（Qt 发送 4 字节/接收 3 字节的不对称原样保留）
     */
    parse(v) {
        if (!v || v.length < 9) {
            showMessageBox('错误', '时间数据不完整');
            return;
        }
        const r = parseTimeResponse(v);
        $('time-date').value = dateStrFromDays1984(r.days, true);
        const totalSeconds = Math.floor(r.msecs / 1000);
        $('time-time').value = `${String(Math.floor(totalSeconds / 3600) % 24).padStart(2, '0')}:${String(Math.floor((totalSeconds % 3600) / 60)).padStart(2, '0')}:${String(totalSeconds % 60).padStart(2, '0')}`;
        this._userModified = false;
    },

    /** 设置时间（对应 Qt sendTimeToDevice：28 号） */
    async set() {
        if (!$('time-manual').checked || !this._userModified) {
            await showMessageBox('提示', '请先勾选 set data/time 并修改日期/时间');
            return;
        }
        const dateStr = $('time-date').value;
        const timeStr = $('time-time').value;
        if (!dateStr || !timeStr) return;

        const days = this._daysSince1984(dateStr) + 1; // Qt: daysTo + 1
        const msecs = this._msecsSinceMidnight(timeStr);

        // 6 字节 payload + 1 字节 0 终止符；zero = 6 字节中 0 的个数（Qt 补零压缩原样）
        const { data, zero } = packTimePayload(days, msecs);

        try {
            // Qt: 设备可能回 28 数据帧或 E5，两者都视为成功
            await SerialManager.sendCommand({ nr: 28, data, zero, expectNr: 28, expectE5: true, down: 1028 });
            this._userModified = false;
        } catch (e) {
            await showMessageBox('错误', '设置时间失败：' + (e.message || e));
        }
    },
};

(function () {
    // 模态框关闭（含右上角 X / ESC）时停止秒级刷新定时器
    $('dlg-time').addEventListener('hidden.bs.modal', () => DlgTime._stopTimer());
    $('time-manual').addEventListener('change', () => {
        if ($('time-manual').checked) {
            // 进入手动模式：停止自动刷新（Qt 行为）
        } else {
            DlgTime._syncToSystemTime();
        }
    });
    $('time-date').addEventListener('change', () => { DlgTime._userModified = true; });
    $('time-time').addEventListener('change', () => { DlgTime._userModified = true; });
    $('time-get').addEventListener('click', () => DlgTime.fetch());
    $('time-set').addEventListener('click', () => DlgTime.set());
    $('time-exit').addEventListener('click', () => DlgTime.close());
})();
