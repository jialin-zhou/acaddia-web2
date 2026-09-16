/**
 * 应用入口：连接窗口（new1Dlg）+ 主界面（Dlg_ZJM）
 * 所有显示公式严格按 dlg_zjm.cpp updateData()/processMeasurementRow() 移植。
 */

// ---------- 全局：AD1 参数工作区（Qt Dlg_AD1::DATA[93]，供保存/打开/下载使用） ----------
Qt.AD1_DATA = new Uint8Array(93);

// ---------- 日志模块：全局错误捕获 + 页面日志面板（js/log.js） ----------
Logger.installErrorHandlers();
Logger.mountPanel();
Logger.info('app', '应用启动');

(function () {
    // ---------- 视图切换（Bootstrap d-none 类，避免与 .d-flex !important 冲突） ----------
    function showConnect() {
        $('view-connect').classList.remove('d-none');
        $('view-main').classList.add('d-none');
    }
    function showMain() {
        $('view-connect').classList.add('d-none');
        $('view-main').classList.remove('d-none');
        updateData();
    }

    // ---------- 连接窗口按钮（new1Dlg） ----------

    $('btn-connect').addEventListener('click', async () => {
        try {
            Logger.info('app', '点击 在线连接');
            if (!SerialManager.port) {
                // 参考 Qt 自动打开串口的行为：自动请求端口，按默认参数 9600,n,8,1 打开
                const port = await SerialManager.requestPort();
                await SerialManager.connect(port, { baudRate: 9600, dataBits: 8, stopBits: 1, parity: 'none', flowControl: 'none', bufferSize: 255 });
            } else if (!Qt.serial.connected) {
                await SerialManager.connect(SerialManager.port, Qt.serial.settings || { baudRate: 9600, dataBits: 8, stopBits: 1, parity: 'none', flowControl: 'none', bufferSize: 255 });
            }
            $('connect-status').textContent = 'Connecting...... Please waitting!';
            await SerialManager.startConnect();
        } catch (e) {
            Logger.error('app', '无法打开串口', e);
            await showMessageBox('错误', '无法打开串口：' + (e.message || e));
        }
    });

    $('btn-offline').addEventListener('click', async () => {
        // 对应 new1Dlg::OnOffline：清数据、DIM_public=16384、显示主界面
        if (SerialManager.port) await SerialManager.disconnect();
        Qt.clearData();
        showMain();
    });

    $('btn-exit').addEventListener('click', async () => {
        // 对应 new1Dlg::OnExit
        await SerialManager.exit();
        await showMessageBox('提示', '程序已退出');
        try { window.close(); } catch (e) { /* 浏览器可能禁止 */ }
    });

    // ---------- 主界面对话框入口（Dlg_ZJM 顶部按钮） ----------
    $('btn-zjm-tqcs').addEventListener('click', () => DlgTqcs.open());
    $('btn-zjm-ad').addEventListener('click', () => DlgAd.open());
    $('btn-zjm-adjust').addEventListener('click', () => DlgAdjust.open());
    $('btn-zjm-time').addEventListener('click', () => DlgTime.open());
    $('btn-zjm-tqml').addEventListener('click', () => DlgTqml.open());
    $('btn-zjm-comset').addEventListener('click', () => DlgComSet.open());
    $('btn-zjm-message').addEventListener('click', () => DlgMessage.open());
    $('btn-zjm-angle').addEventListener('click', () => DlgAngle.open());
    $('btn-zjm-dim').addEventListener('click', () => DlgDim.open());

    // ---------- 主界面复选框（Qt setupCheckbox → updateData） ----------
    ['chk-line1', 'chk-line2', 'chk-dc'].forEach((id) => {
        $(id).addEventListener('change', updateData);
    });

    // ---------- 获取信息（OnButtonZjmFetch → 状态机 37→47→47→49） ----------
    $('btn-zjm-fetch').addEventListener('click', async () => {
        if (!Qt.serial.connected) {
            Logger.warn('app', '点击 获取信息 但串口未连接');
            await showMessageBox('错误', '串口未连接');
            return;
        }
        Logger.info('app', '点击 获取信息（主界面获取）');
        try {
            await SerialManager.startFetch();
        } catch (e) {
            Logger.error('app', '获取失败', e);
            await showMessageBox('错误', '获取失败：' + (e.message || e));
        }
    });

    // ---------- 保存参数（OnButtonZjmSave：AD1 DATA 93 字节 → .bin 文件） ----------
    $('btn-zjm-save').addEventListener('click', () => {
        const blob = new Blob([Qt.AD1_DATA.slice(0, 93)], { type: 'application/octet-stream' });
        const a = document.createElement('a');
        a.href = URL.createObjectURL(blob);
        a.download = 'params.bin';
        a.click();
        URL.revokeObjectURL(a.href);
    });

    // ---------- 打开文件（OnButtonZjmOpen：≥93 字节 → 载入 AD1 数据） ----------
    $('btn-zjm-open').addEventListener('click', () => $('file-open-params').click());
    $('file-open-params').addEventListener('change', async (e) => {
        const file = e.target.files[0];
        e.target.value = '';
        if (!file) return;
        const buf = new Uint8Array(await file.arrayBuffer());
        if (buf.length < 93) {
            await showMessageBox('错误', '文件大小不足93字节');
            return;
        }
        // Qt: AD1_FUNCTION=1126; AD1_dlg.ValidData[4+tt] = fileData[tt]
        Qt.AD1_DATA.set(buf.subarray(0, 93));
        const vd = new Uint8Array(93 + 4);
        vd.set(buf.subarray(0, 93), 4);
        Qt.AD1_ValidData = vd;
        DlgAd.loadFromData();
        await showMessageBox('提示', '参数已从文件加载到 AD参数 对话框');
    });

    // ---------- 退出（OnButtonZjmExit：关闭主界面并退出） ----------
    $('btn-zjm-exit').addEventListener('click', async () => {
        await SerialManager.exit();
        showConnect();
    });

    // ---------- 总线事件 ----------
    Bus.on('connect-done', () => {
        showMain();
    });
    Bus.on('status', (msg) => {
        const el = $('connect-status');
        if (el && el.offsetParent !== null) el.textContent = msg;
        $('main-status').textContent = msg;
    });
    Bus.on('fetch-done', updateData);
    Bus.on('dim-changed', updateData);
    Bus.on('serial-disconnected', () => {
        $('main-status').textContent = '串口已断开';
    });

    // ==================== 主界面数据显示（dlg_zjm.cpp updateData） ====================

    const ITEM_NAMES = ['Ua', 'Ub', 'Uc', 'Uab', 'Ubc', 'Uca', 'Ia', 'Ib', 'Ic', 'P', 'Q', 'S', 'f', 'Cos phi'];
    const DC_NAMES = ['DC1', 'DC2', 'DC3', 'DC4', 'DC5', 'DC6', 'DC7', 'DC8'];

    /**
     * processMeasurementRow 移植
     */
    /**
     * 解析主界面消息列表（Qt updateData 中 Down==1005 分支，计算在 core.js）
     */
    function renderMessageList() {
        const tbody = $('tbl-messages').querySelector('tbody');
        tbody.innerHTML = '';
        const rows = computeMessageRows(Qt.MsgList_h_ValidData, Qt.MsgList_ValidData);
        rows.forEach((r) => {
            const tr = tbody.insertRow();
            tr.insertCell().textContent = r.msgId;
            tr.insertCell().textContent = r.date;
            tr.insertCell().textContent = r.time;
            tr.insertCell().textContent = r.ms;
            tr.insertCell().textContent = r.event;
        });
    }

    /**
     * 主界面数据刷新（dlg_zjm.cpp updateData 移植，计算在 core.js）
     * 注意：Qt 的 ADCacul_ValidData 含 4 字节头，下标 20/48/4 直接沿用（大端）
     */
    function updateData() {
        const tbody = $('tbl-adcalc').querySelector('tbody');
        tbody.innerHTML = '';

        const vd = Qt.ADCacul_ValidData;
        if (!vd) {
            renderMessageList();
            return;
        }

        const rows = computeMainDisplayRows(vd, {
            showLine1: $('chk-line1').checked,
            showLine2: $('chk-line2').checked,
            showDc: $('chk-dc').checked,
            dimPublic: Qt.DIM_public,
            dimLineVotStyle: Qt.DIM_Line_Vot_style,
            currentStyle: Qt.Current_Style,
            dcVSelect: Qt.DC_V_I_select,
        });
        rows.forEach((r) => {
            const tr = tbody.insertRow();
            tr.insertCell().textContent = r.channel;
            tr.insertCell().textContent = r.item;
            tr.insertCell().textContent = r.second;
            tr.insertCell().textContent = r.percent;
            tr.insertCell().textContent = r.count;
        });

        // 消息列表（Qt: Down==1005 时刷新；此处按是否有数据刷新）
        renderMessageList();
    }

    // ---------- 对话框开关辅助（Bootstrap 模态框） ----------
    window.openDlg = function (id) {
        bootstrap.Modal.getOrCreateInstance($(id)).show();
    };
    window.closeDlg = function (id) {
        const inst = bootstrap.Modal.getInstance($(id));
        if (inst) inst.hide();
    };

    // ---------- 初始状态 ----------
    // Qt 构造函数：Current_Style=1, RXDAT 清空等
    Qt.clearData();
    // AD1 默认参数（Qt Dlg_AD1 默认参数，DIM_public=16384 分支）
    DlgAd.applyDefaults();
    showConnect();
})();
